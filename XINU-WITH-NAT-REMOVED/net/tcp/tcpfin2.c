/* tcpfin2.c  -  tcpfin2 */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  tcpfin2  -  Handle an incoming segment in the FIN-WAIT2 state
 *------------------------------------------------------------------------
 */
int32	tcpfin2(
	  struct tcb	*ptcb,		/* Ptr to a TCB			*/
	  struct ip	*pip,		/* Ptr to IP (not used)		*/
	  struct tcp	*ptcp		/* Ptr to TCP (not used)	*/
	)
{
	tcpdata (ptcb, pip, ptcp);

	/* If our FIN has been ACKed, move to TIME-WAIT	*/
	/*            and schedule the TCB to expire	*/

	if (ptcb->tcb_flags & TCBF_FINSEEN
	    && SEQ_CMP(ptcb->tcb_rnext, ptcb->tcb_rfin) > 0) {
		tcptmset (TCP_MSL << 1, ptcb, TCBC_EXPIRE);
		ptcb->tcb_state = TCB_TWAIT;
	}

	tcpxmit (ptcb);

	return OK;
}
