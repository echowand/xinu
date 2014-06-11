#include <conf.h>
#include <kernel.h>
#include <mark.h>
#include <bufpool.h>
#include <ip.h>
#include <tcp.h>
#include <tcb.h>

int (*tcpstatesw[]) (struct tcb *ptcb, struct ip *pip, struct tcp *ptcp) = {
	NULL,				/* CLOSED			*/
	tcplisten,			/* LISTEN			*/
	tcpsynsent,			/* SYN SENT			*/
	tcpsynrcvd,			/* SYN RCVD			*/
	tcpestd,			/* ESTABLISHED 			*/
	tcpfin1,			/* FIN WAIT-1			*/
	tcpfin2,			/* FIN WAIT-2			*/
	tcpcwait,			/* CLOSE WAIT			*/
	tcpclosing,			/* CLOSING			*/
	tcplastack,			/* LAST ACK			*/
	tcptwait,			/* TIME WAIT			*/
};

void tcpinput (struct tcb *ptcb, struct ip *pip, struct tcp *ptcp)
{
	if (ptcp->tcp_code & TCPF_RST) {
		if (ptcb->tcb_state == TCB_LISTEN)
			return;
		else if (ptcb->tcb_state == TCB_SYNSENT
			 && ptcp->tcp_ack == ptcb->tcb_snext)
			tcpabort (ptcb);
		else if (ptcp->tcp_seq >= ptcb->tcb_rnext
		    && ptcp->tcp_seq <= ptcb->tcb_rnext + ptcb->tcb_rwnd)
			tcpabort (ptcb);
		return;
	}

	if (ptcp->tcp_ack < ptcb->tcb_suna
	    || ptcp->tcp_ack > ptcb->tcb_snext) {
		if (ptcb->tcb_state <= TCB_SYNRCVD)
			tcpreset (pip, ptcp);
		else
			tcpack (ptcb, TRUE);
		return;
	}

	if (ptcp->tcp_code & TCPF_SYN && ptcb->tcb_state > TCB_SYNRCVD) {
		/* XXX: is it the _right_ SYN? */
		ptcp->tcp_code &= ~TCPF_SYN;
		ptcp->tcp_seq++;
	}

	tcpupdate (ptcb, pip, ptcp);

	(tcpstatesw[ptcb->tcb_state]) (ptcb, pip, ptcp);

	return;
}
