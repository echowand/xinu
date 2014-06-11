/* tcpestd.c  -  tcpestd */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  tcpestd  -  Handle an arriving segment for the Established state
 *------------------------------------------------------------------------
 */
int32	tcpestd(
	  struct tcb	*ptcb,		/* Ptr to a TCB			*/
	  struct ip	*pip,		/* Ptr to IP (not used)		*/
	  struct tcp	*ptcp		/* Ptr to TCP (not used)	*/
	)
{
	/* First, process data in the segment */

	tcpdata (ptcb, pip, ptcp);

	/* If a FIN has been seen, move to Close-Wait */

	if (ptcb->tcb_flags & TCBF_FINSEEN
	    && SEQ_CMP(ptcb->tcb_rnext, ptcb->tcb_rfin) > 0) {
		ptcb->tcb_state = TCB_CWAIT;
	}

	/* Transmit a response immediately */

	tcpxmit (ptcb);

	return OK;
}
