#include <conf.h>
#include <kernel.h>
#include <timer.h>
#include <ether.h>
#include <ip.h>
#include <tcp.h>
#include <tcb.h>

void tcpack (struct tcb *ptcb, int force)
{
	struct ep *pep; 
	struct ip *pip;
	struct tcp *ptcp;

	if (!force) {
		if (!(ptcb->tcb_flags & TCBF_NEEDACK))
			return;
		if (!(ptcb->tcb_flags & TCBF_ACKPEND)) {
			ptcb->tcb_flags |= TCBF_ACKPEND;
			tcptmset (TCP_ACKDELAY, ptcb, TCBC_DELACK);
			return;
		}
		/* Otherwise there's an ACK pending, or we're forcing
		 * an ACK, so go ahead and do it */
	}

	if (ptcb->tcb_flags & TCBF_ACKPEND)
		tcptmdel (ptcb, TCBC_DELACK);

	pep = tcpalloc (ptcb, 0);
	pip = (struct ip *)pep->ep_data;
	ptcp = (struct tcp *)((char *)pip + IP_HLEN(pip));

	ptcp->tcp_seq = ptcb->tcb_snext;
	tcph2net (ptcp);
	ptcp->tcp_cksum = tcpcksum (pip, ptcp);

	ptcb->tcb_flags &= ~(TCBF_NEEDACK | TCBF_ACKPEND);

	ipsend (pep);
}
