/* tcptimer.c  -  tcptmset, tcptmdel */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  tcptmset  -  Set a timed TCP event
 *------------------------------------------------------------------------
 */
void	tcptmset(
	  int32		delay,		/* Delay in milliseconds	*/
	  struct tcb	*ptcb,		/* ptr to TCB for the event	*/
	  int32		message		/* Event message		*/
	)
{
	/* Increment reference count for TCB */

	tcbref (ptcb);

	/* Set the event */

	tmset (delay, Tcp.tcpcmdq, TCBCMD(ptcb, message));
	return;
}

/*------------------------------------------------------------------------
 *  tcptmdel  -  Delete a timed TCP event
 *------------------------------------------------------------------------
 */
void	tcptmdel(
	  struct tcb	*ptcb,		/* ptr to TCB for the event	*/
	  int32		message		/* Event to delete		*/
	)
{
	/* Delete the event */

	if (tmdel (Tcp.tcpcmdq, TCBCMD(ptcb, message)) == OK) {

		/* Decrement the refernce count */

		tcbunref (ptcb);
	}
	return;
}
