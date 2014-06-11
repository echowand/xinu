
/* tcpwake.c  -  tcpwake */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  tcpwake  -  Awaken up readers, writers, or both and return count
 *------------------------------------------------------------------------
 */
int32	tcpwake(
	  struct tcb	*ptcb,		/* Ptr to a TCB			*/
	  int32		cond		/* Condition			*/
	)
{
	int32	awakened = 0;

	/* Check condition for readers and awaken */

	if (cond & TCPW_READERS) {
		awakened += ptcb->tcb_readers;
		while (ptcb->tcb_readers--)
			signal (ptcb->tcb_rblock);
	}

	/* Check condition for writers and awaken */

	if (cond & TCPW_WRITERS) {
		awakened += ptcb->tcb_writers;
		while (ptcb->tcb_writers--)
			signal (ptcb->tcb_wblock);
	}

	/* Return count */

	return awakened;
}
