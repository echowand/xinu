/* tcpsynrcvd.c  -  tcpsynrcvd */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  tcpsynrcvd  -  Handle an incoming segment in state SYN-RECEIVED
 *------------------------------------------------------------------------
 */
int32	tcpsynrcvd(
	  struct tcb	*ptcb,		/* Ptr to a TCB			*/
	  struct ip	*pip,		/* Ptr to IP (not used)		*/
	  struct tcp	*ptcp		/* Ptr to TCP (not used)	*/
	)
{
	/* Check for ACK or bas sequence number */

	if (!(ptcp->tcp_code & TCPF_ACK)
	    || ptcp->tcp_seq != ptcb->tcb_rnext) {
		return SYSERR;
	}

	/* The following increment of the refernce count is for	*/
	/*    the system because the TCB is now synchronized	*/

	tcbref (ptcb);

	/* Change state to ESTABLISHED */

	ptcb->tcb_state = TCB_ESTD;

/*DEBUG*///kprintf("Established here\n");

	/* Move past SYN */

	ptcb->tcb_suna++;
	if (tcpdata (ptcb, pip, ptcp)) {
		tcpxmit (ptcb);
	}

	/* Permit reading */

	if (ptcb->tcb_readers) {
		ptcb->tcb_readers--;
		signal (ptcb->tcb_rblock);
	}

	return OK;
}
