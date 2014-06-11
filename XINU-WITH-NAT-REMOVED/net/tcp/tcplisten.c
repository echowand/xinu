/* tcplisten.c  -  tcplisten */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  tcplisten  -  handle a segment in the LISTEN state
 *------------------------------------------------------------------------
 */
int32	tcplisten(
	  struct tcb	*ptcb,		/* Ptr to a TCB			*/
	  struct ip	*pip,		/* Ptr to IP (not used)		*/
	  struct tcp	*ptcp		/* Ptr to TCP (not used)	*/
	)
{
	struct	tcb	*pnewtcb;	/* Ptr to TCB for new connection*/
	int32	i;			/* Walks through TCP devices	*/

	/* When listening, incoming segment must be a SYN */

	if ((ptcp->tcp_code & TCPF_SYN) == 0) {
		return SYSERR;
	}

	/* Obtain exclusive use */

	wait (Tcp.tcpmutex);

	/* Find a free TCP pseudo-device for this connection */

	for (i = 0; i < Ntcp; i++) {
		if (tcbtab[i].tcb_state == TCB_FREE)
			break;
	}
	if (i == Ntcp) {

		/* No free devices */

		signal (Tcp.tcpmutex);
		return SYSERR;
	}

	/* Obtain pointer to the device for the new connection	*/
	/*	and initialize the TCB				*/

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
		freemem ((char *)pnewtcb->tcb_rbuf, 65535);
		signal (Tcp.tcpmutex);
		return SYSERR;
	}
	pnewtcb->tcb_sbsize = 65535;

	/* New connection is in SYN-Received State */

	pnewtcb->tcb_state = TCB_SYNRCVD;
	tcbref (pnewtcb);
	wait (pnewtcb->tcb_mutex);
	if (mqsend (ptcb->tcb_lq, i) == SYSERR) {
		tcbunref (pnewtcb);
		signal (pnewtcb->tcb_mutex);
		signal (Tcp.tcpmutex);
		return SYSERR;
	}
	signal (Tcp.tcpmutex);

	ptcb->tcb_qlen++;
	if (ptcb->tcb_readers) {
		ptcb->tcb_readers--;
		signal (ptcb->tcb_rblock);
	}

	/* Copy data from SYN segment to TCB */

	pnewtcb->tcb_lip = pip->ip_dst;
	pnewtcb->tcb_lport = ptcp->tcp_dport;
	pnewtcb->tcb_rip = pip->ip_src;
	pnewtcb->tcb_rport = ptcp->tcp_sport;

	/* Record sequence number and window size */

	pnewtcb->tcb_rnext = pnewtcb->tcb_rbseq = ptcp->tcp_seq + 1;
	pnewtcb->tcb_rwnd = pnewtcb->tcb_ssthresh = ptcp->tcp_window;
	pnewtcb->tcb_snext = pnewtcb->tcb_suna = pnewtcb->tcb_ssyn = 1;

	/* Handle any data in the segment (unexpected, but required) */

	tcpdata (pnewtcb, pip, ptcp);

	/* Can this deadlock? */
	tcpxmit (pnewtcb);

	signal (pnewtcb->tcb_mutex);

	return OK;
}
