/* tcpabort.c - tcpabort */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  tcpabort  -  Close a TCB, releasing waiting processes 
 *------------------------------------------------------------------------
 */
void	tcpabort(
	  struct tcb	*ptcb		/* Ptr to the TCB		*/
	)
{
	/* Change state to closed */

	ptcb->tcb_state = TCB_CLOSED;

	/* Release any processes that are waiting to read or write */

	tcpwake (ptcb, TCPW_READERS|TCPW_WRITERS);

	/* If connection in progress, decrement reference count */

	if (ptcb->tcb_state > TCB_SYNRCVD)
		tcbunref (ptcb);

	return;
}
