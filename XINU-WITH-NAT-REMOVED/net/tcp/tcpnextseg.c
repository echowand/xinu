/* tcpnextseg.c  -  tcpnextseg */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  tcpnextseg  -  Compute the offset for the next segment
 *------------------------------------------------------------------------
 */
int32	tcpnextseg(
	  struct tcb	*ptcb,		/* Ptr to a TCB			*/
	  int32		*offset		/* Offset returned to caller	*/
	)
{
	*offset = ptcb->tcb_snext - ptcb->tcb_suna;

	if (ptcb->tcb_sblen > *offset)
		return (min (ptcb->tcb_mss, ptcb->tcb_sblen - *offset));

	return 0;
}
