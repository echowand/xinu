/* tcpclose.c  -  tcpclose */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  tcpclose  -  close a TCP connection
 *------------------------------------------------------------------------
 */
int32	tcp_close(
	  int32 index			/* index of endpoint	*/
	)
{
	int32	child;			/* Child process for server	*/
	struct	tcb *ptcb;		/* Ptr to TCB			*/


	/* Obtain pointer to TCB for this device */

	ptcb = &tcbtab[index];

	/* Obtain exclusive access and insure connection is open */

	wait (ptcb->tcb_mutex);
	if (ptcb->tcb_state < TCB_CLOSED
	    || ptcb->tcb_flags & TCBF_WRDONE) {
		signal (ptcb->tcb_mutex);
		return SYSERR;
	}

	/* For an endpoint in the LISTEN state, close waiting children	*/

	if (ptcb->tcb_state == TCB_LISTEN) {
		while (ptcb->tcb_qlen--) {
			child = mqrecv (ptcb->tcb_qlen);

			/* Perhaps the child should abort rather than	*/
			/* 	close cleanly ... 			*/
			tcp_close (child);
		}
		ptcb->tcb_state = TCB_CLOSED;
		tcbunref (ptcb);
		return OK;
	}

	/* Connection is still opening, so stop the retransmission */

	if (ptcb->tcb_state == TCB_SYNSENT) {

		/* There must be an outstanding segment */

		tcptmdel (ptcb, TCBC_RTO);
		ptcb->tcb_state = TCB_CLOSED;
		tcbunref (ptcb);
		return OK;
	}

	/* Handle processes waiting to read or write */

	ptcb->tcb_flags |= TCBF_RDDONE|TCBF_WRDONE;
	ptcb->tcb_sfin = ptcb->tcb_suna + ptcb->tcb_sblen;

	tcpwake (ptcb, TCPW_READERS|TCPW_WRITERS);

	mqsend (Tcp.tcpcmdq, TCBCMD(ptcb, TCBC_SEND));

	/* mqunref is not called here, because did not call mqref	*/
	/*	in mqsend						*/
	signal (ptcb->tcb_mutex);

	return OK;
}
