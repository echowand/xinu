#include <conf.h>
#include <kernel.h>
#include <timer.h>
#include <netdefs.h>
#include <ip.h>
#include <tcp.h>
#include <tcb.h>

int tcpupdate (struct tcb *ptcb, struct ip *pip, struct tcp *ptcp)
{
	int datalen, adjust, acklen;

	if (ptcb->tcb_state <= TCB_SYNRCVD
	    || !(ptcp->tcp_code & TCPF_ACK))
		return OK;

	acklen = SEQ_CMP(ptcp->tcp_ack, ptcb->tcb_suna);
	if (acklen < 0) {
		/* Stale ACK */
		return OK;
	} if (acklen == 0) {
		/* Potentially duplicate ACK */
		if (TCP_DATALEN(pip, ptcp) == 0
		    && ptcp->tcp_window == ptcb->tcb_rwnd)
			ptcb->tcb_dupacks++;
	} if (acklen > 0) {
		datalen = min (ptcb->tcb_sblen,
			       ptcp->tcp_ack - ptcb->tcb_suna);
		ptcb->tcb_sbdata += datalen;
		ptcb->tcb_sbdata %= ptcb->tcb_sbsize;
		ptcb->tcb_sblen -= datalen;
		ptcb->tcb_suna = ptcp->tcp_ack;
		if (ptcb->tcb_cwnd < ptcb->tcb_ssthresh)
			adjust = ptcb->tcb_mss;
		else
			adjust = ptcb->tcb_mss
				* ptcb->tcb_mss
				/ ptcb->tcb_cwnd; /* XXX */
		ptcb->tcb_cwnd += adjust ? adjust : 1;
		ptcb->tcb_dupacks = 0;
		ptcb->tcb_rtocount = 0;
	}

	/* Update the receive window */
	ptcb->tcb_rwnd = ptcp->tcp_window;

	/* Check for RTT measurement */
	if (ptcb->tcb_flags && TCBF_RTTPEND
	    && SEQ_CMP(ptcp->tcp_ack, ptcb->tcb_rttseq) >= 0) {
		ptcb->tcb_flags &= ~TCBF_RTTPEND;
		tcprto (ptcb);
	}

	/* Correct the RTO timer */
	if (acklen) {
		tcptmdel (ptcb, TCBC_RTO);
		if (ptcb->tcb_suna != ptcb->tcb_snext)
			tcptmset (ptcb->tcb_rto, ptcb, TCBC_RTO);
	}

	return OK;
}
