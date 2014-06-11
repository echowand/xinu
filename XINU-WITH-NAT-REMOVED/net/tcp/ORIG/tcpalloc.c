#include <conf.h>
#include <kernel.h>
#include <ether.h>
#include <ip.h>
#include <tcp.h>
#include <tcb.h>

struct ep *tcpalloc (struct tcb *ptcb, int len)
{
	struct ep *pep;
	struct ip *pip;
	struct tcp *ptcp;

	pep = ipalloc (Tcp.tcpdev, TCP_MINHLEN + len, ptcb->tcb_lip, ptcb->tcb_rip);

	pip = (struct ip *)pep->ep_data;
	ptcp = (struct tcp *)((char *)pip + IP_HLEN(pip));

	ptcp->tcp_sport = ptcb->tcb_lport;
	ptcp->tcp_dport = ptcb->tcb_rport;

	ptcp->tcp_code = (TCP_MINHLEN << 10);
	if (ptcb->tcb_state > TCB_SYNSENT) {
		ptcp->tcp_code |= TCPF_ACK;
		ptcp->tcp_ack = ptcb->tcb_rnext;
	}

	ptcp->tcp_window = ptcb->tcb_rbsize - ptcb->tcb_rblen;
	/* XXX: Silly Window Avoidance goes here */
	ptcp->tcp_cksum = 0;
	ptcp->tcp_urgptr = 0;

	return pep;
}
