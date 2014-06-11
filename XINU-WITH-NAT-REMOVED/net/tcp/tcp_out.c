/* tcp_out.c  -  tpc_out */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  tcp_out  -  TCP output process that executes scheduled events
 *------------------------------------------------------------------------
 */
process	tcp_out(void)
{
	struct	tcb	*ptcb;		/* Ptr to a TCB			*/
	int32	msg;			/* Message from a message queue	*/

	/* Continually wait for a message and perform the command */

	while (1) {

		/* Extract next message */

		msg = mqrecv (Tcp.tcpcmdq);

		/* Extract the TCB pointer from the message */

		ptcb = TCBCMD_TCB(msg);

		/* Obtain exclusive use of TCP */

		wait (Tcp.tcpmutex);

		/* Obtain exclusive use of the TCB */

		wait (ptcb->tcb_mutex);

		/* Insure TCB has remained active */

		if (ptcb->tcb_state <= TCB_CLOSED) {
			tcbunref (ptcb);
			signal (ptcb->tcb_mutex);
			signal (Tcp.tcpmutex);
			continue;
		}
		signal (Tcp.tcpmutex);

		/* Perform the command */

		switch (TCBCMD_CMD(msg)) {

		/* Send data */

		case TCBC_SEND:
/*DEBUG*/ kprintf("tcp_out: Command SEND\n");
			tcpxmit (ptcb);
			break;

		/* Send a delayed ACK */

		case TCBC_DELACK:
/*DEBUG*/ kprintf("tcp_out: Command DELAYED ACK\n");
			tcpack (ptcb, FALSE);
			break;

		/* Retransmission Timer expired */

		case TCBC_RTO:
/*DEBUG*/ kprintf("tcp_out: Command RTO\n");
			ptcb->tcb_cwnd = ptcb->tcb_mss;
			ptcb->tcb_ssthresh = max(ptcb->tcb_snext
						 - ptcb->tcb_suna,
						 ptcb->tcb_mss);
			ptcb->tcb_snext = ptcb->tcb_suna;
			ptcb->tcb_dupacks = 0;
			ptcb->tcb_rto <<= 1;
			if (++ptcb->tcb_rtocount > TCP_MAXRTO)
				tcpabort (ptcb);
			else
				tcpxmit (ptcb);
			break;

		/* TCB has expired, so mark it closed */

		case TCBC_EXPIRE:
/*DEBUG*/ kprintf("tcp_out: Command TCB EXPIRE\n");
			ptcb->tcb_state = TCB_CLOSED;
			tcbunref (ptcb);
			break;

		/* Unknown command (should not happen) */

		default:
			break;
		}

		/* Command has been handled, so reduce reference count	*/

		tcbunref (ptcb);

		/* Release TCP mutex while waiting for next message	*/

		signal (ptcb->tcb_mutex);
	}

	/* Will never reach this point */

	return SYSERR;
}
