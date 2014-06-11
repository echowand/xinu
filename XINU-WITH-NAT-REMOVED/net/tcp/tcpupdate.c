/* tcpupdate.c  -  tcpupdate */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  tcpupdate  -  Update the receive window and other parameters
 *------------------------------------------------------------------------
 */
int32	tcpupdate(
	  struct tcb	*ptcb,		/* Ptr to a TCB			*/
	  struct ip	*pip,		/* Ptr to IP (not used)		*/
	  struct tcp	*ptcp		/* Ptr to TCP (not used)	*/
	)
{
	int32		datalen;	/* Amount of data		*/
	int32		adjust;		/* Adjusted window size		*/
	int32		acklen;		/* Compate ACK to data sent	*/

	/* If starting a connection, nothing to be done */

	if (ptcb->tcb_state <= TCB_SYNRCVD
	    || !(ptcp->tcp_code & TCPF_ACK)) {
		return OK;
	}

	/* Check ACK against data sent */

	acklen = SEQ_CMP(ptcp->tcp_ack, ptcb->tcb_suna);
	if (acklen < 0) {
		/* Stale ACK */
		return OK;
	} else if (acklen == 0) {
		/* Potentially duplicate ACK */
		if (TCP_DATALEN(pip, ptcp) == 0
		    && ptcp->tcp_window == ptcb->tcb_rwnd)
			ptcb->tcb_dupacks++;
	} else if (acklen > 0) {
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
				/ ptcb->tcb_cwnd;
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
