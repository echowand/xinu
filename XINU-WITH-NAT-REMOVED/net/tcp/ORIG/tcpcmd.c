#include <conf.h>
#include <kernel.h>
#include <mq.h>
#include <ip.h>
#include <tcp.h>
#include <tcb.h>

/*
 * tcpcmd - The root of the command and control plane
 */
PROCESS tcpcmd ()
{
	struct tcb *ptcb;
	int msg;

	while (1) {
		msg = mqrecv (Tcp.tcpcmdq);

		ptcb = TCBCMD_TCB(msg);

		wait (Tcp.tcpmutex);
		wait (ptcb->tcb_mutex);
		if (ptcb->tcb_state <= TCB_CLOSED) {
			tcbunref (ptcb);
			signal (ptcb->tcb_mutex);
			signal (Tcp.tcpmutex);
			continue;
		}
		signal (Tcp.tcpmutex);

		switch (TCBCMD_CMD(msg)) {
		case TCBC_SEND:
			tcpxmit (ptcb);
			break;
		case TCBC_DELACK:
			tcpack (ptcb, FALSE);
			break;
		case TCBC_RTO:
			/* XXX */
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
		case TCBC_EXPIRE:
			ptcb->tcb_state = TCB_CLOSED;
			tcbunref (ptcb);
			break;
		default:
			break;
		}

		tcbunref (ptcb);	/* For the command */
		signal (ptcb->tcb_mutex);
	}

	return SYSERR;
}
