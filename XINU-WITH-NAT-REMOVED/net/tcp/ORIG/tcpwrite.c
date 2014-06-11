#include <conf.h>
#include <kernel.h>
#include <mq.h>
#include <ip.h>
#include <tcp.h>
#include <tcb.h>

int tcpwrite (struct devsw *pdev, char *data, int len)
{
	struct tcb *ptcb = &tcbtab[pdev->dvminor];
	int i, j, curlen;

	wait (Tcp.tcpmutex);
	if (ptcb->tcb_state <= TCB_LISTEN) {
		signal (Tcp.tcpmutex);
		return SYSERR;
	}
	wait (ptcb->tcb_mutex);
	signal (Tcp.tcpmutex);

	i = 0;
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
		if (ptcb->tcb_state == TCB_CLOSED)
			break;
		curlen = min (ptcb->tcb_sbsize - ptcb->tcb_sblen, len - i);
		for (j = 0; j < curlen; j++) {
			ptcb->tcb_sbuf[(ptcb->tcb_sbdata
					+ ptcb->tcb_sblen
					+ j) % ptcb->tcb_sbsize] = data[i++];
		}
		ptcb->tcb_sblen += curlen;
	}

	if (i > 0) {
		ptcb->tcb_flags |= TCBF_SPUSHOK;
		ptcb->tcb_spush = ptcb->tcb_suna + ptcb->tcb_sblen;

		tcbref (ptcb);
		mqsend (Tcp.tcpcmdq, TCBCMD(ptcb, TCBC_SEND));
	}

	signal (ptcb->tcb_mutex);

	return i;
}
