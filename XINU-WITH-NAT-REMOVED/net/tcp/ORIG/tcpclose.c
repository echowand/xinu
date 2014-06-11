#include <conf.h>
#include <kernel.h>
#include <mq.h>
#include <ip.h>
#include <tcp.h>
#include <tcb.h>

int tcpclose (struct devsw *pdev)
{
	int child;
	struct tcb *ptcb = &tcbtab[pdev->dvminor];

	wait (ptcb->tcb_mutex);
	if (ptcb->tcb_state < TCB_CLOSED
	    || ptcb->tcb_flags & TCBF_WRDONE) {
		signal (ptcb->tcb_mutex);
		return SYSERR;
	}

	if (ptcb->tcb_state == TCB_LISTEN) {
		while (ptcb->tcb_qlen--) {
			child = mqrecv (ptcb->tcb_qlen);
			/* XXX: This should probably abort, rather than
			 * close cleanly ... ? */
			close (child);
		}
		ptcb->tcb_state = TCB_CLOSED;
		tcbunref (ptcb);
		return OK;
	}

	if (ptcb->tcb_state == TCB_SYNSENT) {
		/* There must be an outstanding segment */
		tcptmdel (ptcb, TCBC_RTO);
		ptcb->tcb_state = TCB_CLOSED;
		tcbunref (ptcb);
		return OK;
	}

	ptcb->tcb_flags |= TCBF_RDDONE|TCBF_WRDONE;
	ptcb->tcb_sfin = ptcb->tcb_suna + ptcb->tcb_sblen;

	tcpwake (ptcb, TCPW_READERS|TCPW_WRITERS);

	mqsend (Tcp.tcpcmdq, TCBCMD(ptcb, TCBC_SEND));

	/* We don't unref here, because we would ref for the mqsend */
	signal (ptcb->tcb_mutex);

	return OK;
}
