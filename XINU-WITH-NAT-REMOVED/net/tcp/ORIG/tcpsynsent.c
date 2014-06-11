#include <conf.h>
#include <kernel.h>
#include <ip.h>
#include <tcp.h>
#include <tcb.h>

int tcpsynsent (struct tcb *ptcb, struct ip *pip, struct tcp *ptcp)
{
	if (!(ptcp->tcp_code & TCPF_SYN)) {
		tcpreset (pip, ptcp);
		return SYSERR;
	}

	ptcb->tcb_state = TCB_SYNRCVD;

	ptcb->tcb_rnext = ptcb->tcb_rbseq = ++ptcp->tcp_seq;
	ptcb->tcb_rwnd = ptcb->tcb_ssthresh = ptcp->tcp_window;
	ptcp->tcp_code &= ~TCPF_SYN;
	tcpdata (ptcb, pip, ptcp);
	tcpack (ptcb, TRUE);

	return OK;
}
