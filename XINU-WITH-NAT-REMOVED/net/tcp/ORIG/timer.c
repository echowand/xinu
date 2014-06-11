#include <conf.h>
#include <kernel.h>
#include <timer.h>
#include <mq.h>
#include <stdio.h>

int tmpending;
int *tmnext;

struct tmentry tmtab[NTIMERS];
int tmfree;
int tmhead;
int tmsem;
int tmlock;

void tminit ()
{
	int i;
	tmpending = 0;
	tmnext = NULL;
	tmfree = 0;
	for (i = 0; i < NTIMERS - 1; i++)
		tmtab[i].tm_next = i + 1;
	tmtab[i].tm_next = BADTIMER;
	tmhead = BADTIMER;
	tmsem = screate (0);
	tmlock = screate (1);
	resume (create ((int *)timer, TMSTK, TMPRIO, TMNAME, 0, 0));
}

PROCESS timer ()
{
	int tmp;
	STATWORD ps;

	while (1) {
		wait (tmsem);
		wait (tmlock);
		while (tmhead != BADTIMER && tmtab[tmhead].tm_remain <= 0) {
			mqsend (tmtab[tmhead].tm_mq, tmtab[tmhead].tm_msg);
			tmp = tmhead;
			tmhead = tmtab[tmhead].tm_next;
			tmtab[tmp].tm_next = tmfree;
			tmfree = tmp;
		}
		if (tmhead != BADTIMER) {
			disable(ps);
			tmpending = TRUE;
			tmnext = &tmtab[tmhead].tm_remain;
			restore(ps);
		}
		signal (tmlock);
	}
}

INTPROC tmfire ()
{
	tmpending = FALSE;
	signal (tmsem);
	return 0;
}

int tmset (int delay, int mq, int msg)
{
	int tment, cur, prev;
	STATWORD ps;

	wait (tmlock);
	if (tmfree == BADTIMER) {
		signal (tmlock);
		return SYSERR;
	}
	tment = tmfree;
	tmfree = tmtab[tmfree].tm_next;

	tmtab[tment].tm_mq = mq;
	tmtab[tment].tm_msg = msg;
	tmtab[tment].tm_next = BADTIMER;

	prev = BADTIMER;
	cur = tmhead;
	disable(ps);
	while (cur != BADTIMER) {
		if (tmtab[cur].tm_remain > delay) {
			tmtab[cur].tm_remain -= delay;
			break;
		}
		delay -= tmtab[cur].tm_remain;
		prev = cur;
		cur = tmtab[cur].tm_next;
	}
	if (prev == BADTIMER)
		tmhead = tment;
	else
		tmtab[prev].tm_next = tment;
	tmtab[tment].tm_next = cur;
	tmtab[tment].tm_remain = delay;

	tmpending = TRUE;
	tmnext = &tmtab[tmhead].tm_remain;
	restore(ps);
	signal (tmlock);
	return OK;
}

int tmdel (int mq, int msg)
{
	int cur, prev, next, found;
	STATWORD ps;

	prev = BADTIMER;
	cur = tmhead;
	found = 0;
	wait (tmlock);
	while (cur != BADTIMER) {
		if (tmtab[cur].tm_mq == mq && tmtab[cur].tm_msg == msg) {
			if (prev == BADTIMER)
				tmhead = tmtab[cur].tm_next;
			else
				tmtab[prev].tm_next = tmtab[cur].tm_next;
			if ((next = tmtab[cur].tm_next) != BADTIMER) {
				disable(ps);
				tmtab[next].tm_remain += tmtab[cur].tm_remain;
				restore(ps);
			}
			tmtab[cur].tm_next = tmfree;
			tmfree = cur;
			found = 1;
			break;
		}
		prev = cur;
		cur = tmtab[cur].tm_next;
	}

	if (!found) {
		signal (tmlock);
		return SYSERR;
	}

	disable(ps);
	if (tmhead == BADTIMER) {
		tmpending = FALSE;
	} else {
		tmpending = TRUE;
		tmnext = &tmtab[tmhead].tm_remain;
	}
	restore(ps);
	signal (tmlock);
	return OK;
}
