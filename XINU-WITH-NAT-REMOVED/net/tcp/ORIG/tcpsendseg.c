#include <conf.h>
#include <kernel.h>
#include <ether.h>
#include <ip.h>
#include <tcp.h>
#include <tcb.h>

extern unsigned int ctr1000;

void tcpsendseg (struct tcb *ptcb, int offset, int len, int code)
{
	struct ep *pep = tcpalloc (ptcb, len);
	struct ip *pip = (struct ip *)pep->ep_data;
	struct tcp *ptcp = (struct tcp *)((char *)pip + IP_HLEN(pip));
	int i;
	char *data;

	ptcp->tcp_seq = ptcb->tcb_suna + offset;
	ptcp->tcp_code |= code;

	data = ((char *)ptcp + TCP_HLEN(ptcp));
	for (i = 0; i < len; i++) {
		data[i] = ptcb->tcb_sbuf[(ptcb->tcb_sbdata + offset + i)
			% ptcb->tcb_sbsize];
	}

	if (ptcb->tcb_suna == ptcb->tcb_snext)	/* No outstanding data */
		tcptmset (ptcb->tcb_rto, ptcb, TCBC_RTO);

	tcph2net (ptcp);
	ptcp->tcp_cksum = tcpcksum (pip, ptcp);

	if (ptcb->tcb_flags & TCBF_ACKPEND)
		tcptmdel (ptcb, TCBC_DELACK);
	ptcb->tcb_flags &= ~(TCBF_NEEDACK | TCBF_ACKPEND);

	if (!(ptcb->tcb_flags & TCBF_RTTPEND) && len) {
		ptcb->tcb_flags |= TCBF_RTTPEND;
		ptcb->tcb_rttseq = ptcp->tcp_seq;
		ptcb->tcb_rtttime = (int)ctr1000;
	}

	ipsend (pep);
}
