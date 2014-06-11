/* tcpsend.c  -  tcp_send */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  tcp_send  -  Write data to a TCP connection
 *------------------------------------------------------------------------
 */
int32	tcp_send(
	  int 		index,		/* Ptr to device table entry	*/
	  char		*data,		/* Buffer of data to write	*/
	  int32		len		/* Count of chars in the buffer	*/
	)
{
	struct	tcb	*ptcb;		/* Ptr to TCB for the device	*/
	int32		i;		/* counts bytes of data		*/
	int32		j;		/* used during copy		*/
	int32		curlen;		/* bytes that can be copied	*/

	/* Obtain a pointer to the TCB entry for the device */

	ptcb = &tcbtab[index];

	/* Obtain exclusive use of the TCB table */

	wait (Tcp.tcpmutex);
	if (ptcb->tcb_state <= TCB_LISTEN) {
		/* Connection not yet established */
		signal (Tcp.tcpmutex);
		return SYSERR;
	}

	/* Obtain exclusive use of the specific TCB */

	wait (ptcb->tcb_mutex);
	signal (Tcp.tcpmutex);

	i = 0;

	/* Loop until all data has been written */

	
	while (i < len) {
	
		if (ptcb->tcb_flags & TCBF_WRDONE)
			break;

		while (ptcb->tcb_sblen == ptcb->tcb_sbsize
		       && ptcb->tcb_state != TCB_CLOSED) {
			ptcb->tcb_writers++;
			signal (ptcb->tcb_mutex);
			wait (ptcb->tcb_wblock);
			wait (ptcb->tcb_mutex);
		}
		if (ptcb->tcb_state == TCB_CLOSED) {
			break;
		}

		/* Calculate size of dtaa that can be copied */

		curlen = min (ptcb->tcb_sbsize - ptcb->tcb_sblen, len - i);

		/* Copy data */

		for (j = 0; j < curlen; j++) {
			ptcb->tcb_sbuf[(ptcb->tcb_sbdata
					+ ptcb->tcb_sblen
					+ j) % ptcb->tcb_sbsize] = data[i++];
		}
		ptcb->tcb_sblen += curlen;
	}

	/* If data was deposited, push it (push on write) */

	if (i > 0) {
		ptcb->tcb_flags |= TCBF_SPUSHOK;
		ptcb->tcb_spush = ptcb->tcb_suna + ptcb->tcb_sblen;

		tcbref (ptcb);

		/* send a message to the output process */

		mqsend (Tcp.tcpcmdq, TCBCMD(ptcb, TCBC_SEND));
		kprintf("tcpsend: message sent\r\n");
	}
	signal (ptcb->tcb_mutex);

	return i;
}
