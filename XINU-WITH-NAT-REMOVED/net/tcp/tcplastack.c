/* tcplastack.c  -  tcplastack */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  tcplastack  -  Handle the final ACK for the FIN
 *------------------------------------------------------------------------
 */
int32	tcplastack(
	  struct tcb	*ptcb,		/* Ptr to a TCB			*/
	  struct ip	*pip,		/* Ptr to IP (not used)		*/
	  struct tcp	*ptcp		/* Ptr to TCP (not used)	*/
	)
{
	/* See if this ACK covers the FIN */
	if (SEQ_CMP(ptcb->tcb_suna, ptcb->tcb_sfin) > 0) {

		/* There must be an RTO pending at this point */

		tcptmdel (ptcb, TCBC_RTO);

		/* Mark the TCB closed */

		ptcb->tcb_state = TCB_CLOSED;

		/* Reduce the reference count */

		tcbunref (ptcb);
	}
	return OK;
}
