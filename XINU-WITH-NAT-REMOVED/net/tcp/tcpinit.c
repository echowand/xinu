/* tcpinit.c  -  tcpinit */

#include <xinu.h>

struct	tcb	tcbtab[Ntcp];		/* The TCB table		*/
struct  tcpcontrol      Tcp;


/*------------------------------------------------------------------------
 *  tcpinit  -  Initialize a TCB for a TCP pseudo device
 *------------------------------------------------------------------------
 */
int32	tcpinit(void)
{
	struct	tcb	*ptcb;		/* Ptr to TCB entry 		*/
	int32		i;		/* Walk through endpoints	*/

	/* Initialize values in the Tcp strcuture */

        Tcp.tcpmutex = semcreate (1);
        Tcp.tcpnextport = 33000;

        /* Create a message queue for TCP */

        if (mqready == 0) {
                tminit();
                mqinit();
        }
        Tcp.tcpcmdq = mqcreate (10);

        resume (create ((int *)tcp_out, 512, 80, "tcp_out", 0, NULL));	


	for(i=0; i<Ntcp; i++) { 

		ptcb = &tcbtab[i];

		ptcb->tcb_iface = -1;

		/* Initialize TCB semaphores */

		ptcb->tcb_mutex  = semcreate (1);
		ptcb->tcb_rblock = semcreate (0);
		ptcb->tcb_wblock = semcreate (0);

		/* Create a message queue for the TCB */

		if (mqready == 0) {
			tminit();
			mqinit();
		}

		ptcb->tcb_lq = mqcreate (5);

		/* Start the refernce count at zero and mark the TCB free */

		ptcb->tcb_ref = 0;
		ptcb->tcb_state = TCB_FREE;
	}

	return OK;
}
