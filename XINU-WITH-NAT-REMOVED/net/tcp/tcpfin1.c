/* tcpfin1.c  -  tcpfin1 */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  tcpfin1  -  Handle an incoming segment in the FIN1 state
 *------------------------------------------------------------------------
 */
int32	tcpfin1(
	  struct tcb	*ptcb,		/* Ptr to a TCB			*/
	  struct ip	*pip,		/* Ptr to IP (not used)		*/
	  struct tcp	*ptcp		/* Ptr to TCP (not used)	*/
	)
{
	int32	rfin;			/* Did we receive a FIN?	*/
	int32	ack;			/* Did we send an ACK		*/

	rfin = ack = 0;

	/* Handle data in the segment first */

	tcpdata (ptcb, pip, ptcp);

	/* Check whether FIN has been receives and ACK sent */

	if (ptcb->tcb_flags & TCBF_FINSEEN &&
	    SEQ_CMP(ptcb->tcb_rnext, ptcb->tcb_rfin) > 0)
		rfin = 1;

	if (SEQ_CMP(ptcb->tcb_suna, ptcb->tcb_sfin) > 0)
		ack = 1;

	if (rfin && ack) {

		/* FIN was received and ours was ACKed, so the	*/
		/*     connection is closed and TCB can expire	*/

		tcptmset (TCP_MSL << 1, ptcb, TCBC_EXPIRE);
		ptcb->tcb_state = TCB_TWAIT;
	} else if (rfin)

		/* FIN was received - connection is closing */

		ptcb->tcb_state = TCB_CLOSING;
	else if (ack)

		/* ACK arrived and ew move to FIN-WAIT2 */
		ptcb->tcb_state = TCB_FIN2;

	tcpxmit (ptcb);

	return OK;
}
