/* tcprecv.c  -  tcp_recv */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  tcp_recv  -  Read from a TCP connection
 *------------------------------------------------------------------------
 */
int32	tcp_recv(
	  int32		index,		/* TCP endpoint index		*/
	  char		*data,		/* Ptr to buffer		*/
	  int32		len		/* Size of buffer		*/
	)
{
	struct	tcb	*ptcb;		/* Ptr to a TCB			*/
	int32		i;		/* Iterates through data	*/
	int32		j;		/* Counter used during copy	*/
	int32		curlen;		/* Amount of data available	*/
	pid32		child;		/* Process ID of child		*/

	/* Obtain pointer to the TCB for the pseudo device */

	ptcb = &tcbtab[index];

	/* Obtain exclusive access to TCB table */

	wait (Tcp.tcpmutex);

	if (ptcb->tcb_state == TCB_FREE) {
		signal (Tcp.tcpmutex);
		return SYSERR;
	}

	/* Obtain exclusive access to the specific TCB */

	wait (ptcb->tcb_mutex);
	signal (Tcp.tcpmutex);

	tcbref (ptcb);

	/* Interpret semantics according to TCB state */

	if (ptcb->tcb_state == TCB_LISTEN) {

		/* Read on a passive socket acts like accept() */

		while (ptcb->tcb_qlen == 0
		       && ptcb->tcb_state != TCB_CLOSED) {
			ptcb->tcb_readers++;
			signal (ptcb->tcb_mutex);
			wait (ptcb->tcb_rblock);
			wait (ptcb->tcb_mutex);
		}

		/* Recheck state to see why we resumed */

		if (ptcb->tcb_state == TCB_CLOSED) {
			child = SYSERR;
		} else {
			ptcb->tcb_qlen--;
			child = mqrecv (ptcb->tcb_lq);
		}
		tcbunref (ptcb);
		signal (ptcb->tcb_mutex);
		if (child == SYSERR){
			return SYSERR;
		} else {
			*(int *)data = child;
		}
		return OK;
	}

	/* Check closed sockets, pushes, etc. etc. */

	i = 0;
	while (i < len) {
		
		
		/* Handle read after FIN */

		if ((ptcb->tcb_flags & TCBF_FINSEEN)
		    && ptcb->tcb_rbseq == ptcb->tcb_rfin)
			break;
		
		

		if (ptcb->tcb_flags & TCBF_RDDONE
		    && ptcb->tcb_rblen == 0)
			break;

		/* A test is needed here: if another reader has already	*/
		/* read the data for which we are waiting or we hit a	*/
		/* FIN, the situation is the same as the above		*/
		/* condition.						*/

		if (ptcb->tcb_rblen == 0) {
			ptcb->tcb_readers++;
			signal (ptcb->tcb_mutex);
			wait (ptcb->tcb_rblock);
			wait (ptcb->tcb_mutex);
		}

		/* Compute current data length */
		curlen = min(len - i, ptcb->tcb_rblen);
		if (ptcb->tcb_flags & TCBF_RPUSHOK)
			curlen = min (curlen,
				      ptcb->tcb_rpush - ptcb->tcb_rbseq);

		/* Copy data to caller's buffer */
		
		for (j = 0; j < curlen; j++) {
			data[i++] = ptcb->tcb_rbuf[(ptcb->tcb_rbdata + j)
				% ptcb->tcb_rbsize];
		}
		ptcb->tcb_rbdata = (ptcb->tcb_rbdata + curlen)
				% ptcb->tcb_rbsize;
		ptcb->tcb_rbseq += curlen;
		ptcb->tcb_rblen -= curlen;

		/* If data was pushed, reset PUSH flag */

		if (ptcb->tcb_flags & TCBF_RPUSHOK
		    && ptcb->tcb_rbseq == ptcb->tcb_rpush) {
			ptcb->tcb_flags &= ~TCBF_RPUSHOK;
			break;
		}
	}

	tcbunref (ptcb);
	signal (ptcb->tcb_mutex);

	return i;
}
