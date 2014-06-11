#include <conf.h>
#include <kernel.h>
#include <mq.h>
#include <ip.h>
#include <tcp.h>
#include <tcb.h>

int tcpread (struct devsw *pdev, char *data, int len)
{
	struct tcb *ptcb = &tcbtab[pdev->dvminor];
	int i, j, curlen, child;

	wait (Tcp.tcpmutex);

	if (ptcb->tcb_state == TCB_FREE) {
		signal (Tcp.tcpmutex);
		return SYSERR;
	}

	wait (ptcb->tcb_mutex);
	signal (Tcp.tcpmutex);

	tcbref (ptcb);

	if (ptcb->tcb_state == TCB_LISTEN) {
		/* read on a passive socket is more like accept() */
		while (ptcb->tcb_qlen == 0
		       && ptcb->tcb_state != TCB_CLOSED) {
			ptcb->tcb_readers++;
			signal (ptcb->tcb_mutex);
			wait (ptcb->tcb_rblock);
			wait (ptcb->tcb_mutex);
		}
		if (ptcb->tcb_state == TCB_CLOSED) {
			child = SYSERR;
		} else {
			ptcb->tcb_qlen--;
			child = mqrecv (ptcb->tcb_lq);
		}
		tcbunref (ptcb);
		signal (ptcb->tcb_mutex);
		if (child == SYSERR)
			return SYSERR;
		else
			*(int *)data = tcbtab[child].tcb_dvnum;
		return OK;
	}

	/* XXX: closed sockets, pushes, etc. etc. */

	i = 0;
	while (i < len) {
		if ((ptcb->tcb_flags & TCBF_FINSEEN)
		    && ptcb->tcb_rbseq == ptcb->tcb_rfin)
			break;

		if (ptcb->tcb_flags & TCBF_RDDONE
		    && ptcb->tcb_rblen == 0)
			break;

		/* This must be an if ... if another reader gets the
		 * data or we hit FIN, it falls through to the above
		 * condition */
		if (ptcb->tcb_rblen == 0) {
			ptcb->tcb_readers++;
			signal (ptcb->tcb_mutex);
			wait (ptcb->tcb_rblock);
			wait (ptcb->tcb_mutex);
		}

		curlen = min(len - i, ptcb->tcb_rblen);
		if (ptcb->tcb_flags & TCBF_RPUSHOK)
			curlen = min (curlen,
				      ptcb->tcb_rpush - ptcb->tcb_rbseq);
		for (j = 0; j < curlen; j++) {
			data[i++] = ptcb->tcb_rbuf[(ptcb->tcb_rbdata + j)
				% ptcb->tcb_rbsize];
		}
		ptcb->tcb_rbdata = (ptcb->tcb_rbdata + curlen)
			% ptcb->tcb_rbsize;
		ptcb->tcb_rbseq += curlen;
		ptcb->tcb_rblen -= curlen;
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
