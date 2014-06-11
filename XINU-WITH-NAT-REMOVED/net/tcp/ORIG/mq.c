#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <mq.h>
#include <stdio.h>

#define MQ_FREE		0
#define MQ_ALLOC	1
#define MQ_DISABLE	2
#define MQ_CLEAR	3

struct mqentry mqtab[NMQ];
int mqsem;

/*
 * Semaphore semantics are a little bizarre here ... mqcreate and
 * mqdelete take the semaphore so that mqcreate does not have to
 * disable interrupts while it loops.  However, interrupts are
 * disabled at any point where an item's MQ_FREE/MQ_ALLOC status is
 * changed, and so disabling interrupts is sufficient to send or
 * receive a message.
 *
 * In short, take mqsem if you wish to add or remove a queue, and make
 * sure to disable interrupts for the status change itself; for a
 * send or receive, interrupts must be disabled for all modifications.
 */

void mqinit ()
{
	int i;
	for (i = 0; i < NMQ; i++) {
		mqtab[i].mq_state = MQ_FREE;
		mqtab[i].mq_cookie = 0;
	}
	mqsem = screate (1);
}

int mqcreate (int qlen)
{
	int i;
	STATWORD ps;

	wait (mqsem);
	for (i = 0; i < NMQ; i++) {
		if (mqtab[i].mq_state != MQ_FREE)
			continue;
		if ((mqtab[i].mq_sem = screate (0)) == SYSERR) {
			signal (mqsem);
			return SYSERR;
		}
		if ((mqtab[i].mq_msgs = (int *)getmem (qlen * sizeof(int)))
		    == (int *)SYSERR) {
			sdelete(mqtab[i].mq_sem);
			signal (mqsem);
			return SYSERR;
		}
		mqtab[i].mq_qlen = qlen;
		mqtab[i].mq_first = 0;
		mqtab[i].mq_count = 0;
		mqtab[i].mq_rcvrs = 0;
		mqtab[i].mq_cookie++;
		disable(ps);
		mqtab[i].mq_state = MQ_ALLOC;
		restore(ps);
		signal (mqsem);
		return i;
	}
	signal (mqsem);
	return SYSERR;
}

int mqdelete (int mq)
{
	struct mqentry *mqp;
	STATWORD ps;

	mqp = &mqtab[mq];
	wait (mqsem);
	if (!MQVALID(mq) || mqp->mq_state != MQ_ALLOC) {
		signal (mqsem);
		return SYSERR;
	}

	disable(ps);
	mqp->mq_state = MQ_FREE;
	restore(ps);
	sdelete (mqp->mq_sem);
	freemem ((struct mblock *)mqp->mq_msgs, mqp->mq_qlen * sizeof(int));
	signal (mqsem);
	return OK;
}

int mqsend (int mq, int msg)
{
	struct mqentry *mqp;
	STATWORD ps;

	mqp = &mqtab[mq];
	disable(ps);
	if (!MQVALID(mq) || mqp->mq_state != MQ_ALLOC
	    || mqp->mq_count == mqp->mq_qlen) {
		restore(ps);
		return SYSERR;
	}

	mqp->mq_msgs[(mqp->mq_first + mqp->mq_count) % mqp->mq_qlen] = msg;
	mqp->mq_count++;
	signal (mqp->mq_sem);

	restore(ps);
	return OK;
}

int mqrecv (int mq)
{
	struct mqentry *mqp;
	int msg, cookie;
	STATWORD ps;

	mqp = &mqtab[mq];
	disable(ps);
	if (!MQVALID(mq) || mqp->mq_state != MQ_ALLOC) {
		restore(ps);
		return SYSERR;
	}

	cookie = mqp->mq_cookie;
	mqp->mq_rcvrs++;
	if (wait (mqp->mq_sem) == DELETED
	    || cookie != mqp->mq_cookie /* became disabled */) {
		restore(ps);
		return SYSERR;
	}
	mqp->mq_rcvrs--;
	msg = mqp->mq_msgs[mqp->mq_first];
	mqp->mq_first = (mqp->mq_first + 1) % mqp->mq_qlen;
	mqp->mq_count--;
	restore(ps);
	return msg;
}

int mqpoll (int mq)
{
	int val;
	STATWORD ps;

	disable(ps);
	if (!MQVALID(mq) || mqtab[mq].mq_state != MQ_ALLOC
	    || mqtab[mq].mq_count == 0) {
		restore(ps);
		return SYSERR;
	}

	val = mqrecv (mq);
	restore(ps);

	return val;
}

int mqdisable (int mq)
{
	struct mqentry *mqp;
	STATWORD ps;

	mqp = &mqtab[mq];
	wait (mqsem);
	disable(ps);
	if (!MQVALID(mq) || mqp->mq_state != MQ_ALLOC) {
		restore(ps);
		signal (mqsem);
		return SYSERR;
	}
	mqp->mq_cookie++;
	mqp->mq_state = MQ_DISABLE;
	sreset(mqp->mq_sem, 0);
	mqp->mq_rcvrs = 0;

	restore(ps);
	signal (mqsem);
	return OK;
}

int mqenable (int mq)
{
	STATWORD ps;
	int rv = SYSERR;

	wait (mqsem);
	disable(ps);
	if (MQVALID(mq) && mqtab[mq].mq_state == MQ_DISABLE) {
		mqtab[mq].mq_state = MQ_ALLOC;
		rv = OK;
	}
	restore(ps);
	signal (mqsem);
	return rv;
}

int mqclear (int mq, int (*func)(int))
{
	struct mqentry *mqp;
	STATWORD ps;
	int i;

	mqp = &mqtab[mq];
	wait (mqsem);
	disable(ps);
	if (!MQVALID(mq) || mqp->mq_state != MQ_DISABLE) {
		restore(ps);
		signal (mqsem);
		return SYSERR;
	}
	mqp->mq_state = MQ_CLEAR;
	restore(ps);
	signal (mqsem);

	for (i = 0; i < mqp->mq_count; i++) {
		(*func)(mqp->mq_msgs[(mqp->mq_first + i) % mqp->mq_qlen]);
	}
	mqp->mq_count = 0;

	disable(ps);
	mqp->mq_state = MQ_DISABLE;
	restore(ps);

	return OK;
}
