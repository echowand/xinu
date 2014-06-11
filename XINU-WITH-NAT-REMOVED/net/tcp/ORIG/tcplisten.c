#include <conf.h>
#include <kernel.h>
#include <mq.h>
#include <ip.h>
#include <tcp.h>
#include <tcb.h>
#include <stdio.h>

int tcplisten (struct tcb *ptcb, struct ip *pip, struct tcp *ptcp)
{
	struct tcb *pnewtcb;
	int i;

	if ((ptcp->tcp_code & TCPF_SYN) == 0)
		return SYSERR;

	wait (Tcp.tcpmutex);
	for (i = 0; i < Ntcp; i++) {
		if (tcbtab[i].tcb_state == TCB_FREE)
			break;
	}
	if (i == Ntcp) {
		signal (Tcp.tcpmutex);
		/* XXX: What? */
		return SYSERR;
	}
	pnewtcb = &tcbtab[i];
	tcbclear (pnewtcb);
	pnewtcb->tcb_rbuf = (char *)getmem (65535);
	if (pnewtcb->tcb_rbuf == (char *)SYSERR) {
		signal (Tcp.tcpmutex);
		return SYSERR;
	}
	pnewtcb->tcb_rbsize = 65535;
	pnewtcb->tcb_sbuf = (char *)getmem (65535);
	if (pnewtcb->tcb_sbuf == (char *)SYSERR) {
		freemem ((struct mblock *)pnewtcb->tcb_rbuf, 65535);
		signal (Tcp.tcpmutex);
		return SYSERR;
	}
	pnewtcb->tcb_sbsize = 65535;
	pnewtcb->tcb_state = TCB_SYNRCVD;
	tcbref (pnewtcb);
	wait (pnewtcb->tcb_mutex);
	if (mqsend (ptcb->tcb_lq, i) == SYSERR) {
		tcbunref (pnewtcb);
		signal (pnewtcb->tcb_mutex);
		signal (Tcp.tcpmutex);
		/* XXX: What? */
		return SYSERR;
	}
	signal (Tcp.tcpmutex);

	ptcb->tcb_qlen++;
	if (ptcb->tcb_readers) {
		ptcb->tcb_readers--;
		signal (ptcb->tcb_rblock);
	}

	pnewtcb->tcb_lip = pip->ip_dst;
	pnewtcb->tcb_lport = ptcp->tcp_dport;
	pnewtcb->tcb_rip = pip->ip_src;
	pnewtcb->tcb_rport = ptcp->tcp_sport;

	pnewtcb->tcb_rnext = pnewtcb->tcb_rbseq = ptcp->tcp_seq + 1;
	pnewtcb->tcb_rwnd = pnewtcb->tcb_ssthresh = ptcp->tcp_window;
	pnewtcb->tcb_snext = pnewtcb->tcb_suna = pnewtcb->tcb_ssyn = 1; /* XXX */

	tcpdata (pnewtcb, pip, ptcp);

	/* XXX: Can this deadlock? */
	tcpxmit (pnewtcb);

	signal (pnewtcb->tcb_mutex);

	return OK;
}
