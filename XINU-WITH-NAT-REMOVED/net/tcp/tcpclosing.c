/* tcpclosing.c  -  tcpclosing */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  tcpclosing  -  Schedule expiration on a TCB that is closing
 *------------------------------------------------------------------------
 */
int32	tcpclosing(
	  struct tcb	*ptcb,		/* Ptr to a TCB			*/
	  struct ip	*pip,		/* Ptr to IP (not used)		*/
	  struct tcp	*ptcp		/* Ptr to TCP (not used)	*/
	)
{
	/* If FIN is within range, schedule TCB expiration */
	kprintf("closing here\n\r");

	if (SEQ_CMP (ptcb->tcb_suna, ptcb->tcb_sfin) > 0) {
		tcptmset (TCP_MSL << 1, ptcb, TCBC_EXPIRE);
		ptcb->tcb_state = TCB_TWAIT;
			}
	return OK;
}
