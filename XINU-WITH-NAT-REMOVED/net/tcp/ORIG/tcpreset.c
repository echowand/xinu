#include <conf.h>
#include <kernel.h>
#include <netdefs.h>
#include <ether.h>
#include <ip.h>
#include <tcp.h>
#include <tcb.h>

int tcpreset (struct ip *pip, struct tcp *ptcp)
{
	struct ep *pep;
	struct ip *pnewip;
	struct tcp *pnewtcp;

	if (ptcp->tcp_code & TCPF_RST)
		return SYSERR;

	pep = ipalloc (Tcp.tcpdev, TCP_MINHLEN, pip->ip_dst, pip->ip_src);
	pnewip = (struct ip *)pep->ep_data;
	pnewtcp = (struct tcp *)((char *)pnewip + IP_HLEN(pnewip));

	pnewtcp->tcp_sport = ptcp->tcp_dport;
	pnewtcp->tcp_dport = ptcp->tcp_sport;
	if (ptcp->tcp_code & TCPF_ACK)
		pnewtcp->tcp_seq = ptcp->tcp_ack;
	else
		pnewtcp->tcp_seq = 0;
	pnewtcp->tcp_ack = ptcp->tcp_seq + net2hs (pip->ip_len)
		- IP_HLEN(pip) - TCP_HLEN(ptcp);
	if (ptcp->tcp_code & (TCPF_SYN|TCPF_FIN))
		pnewtcp->tcp_ack++;
	pnewtcp->tcp_code = TCP_MINHLEN << 10;
	pnewtcp->tcp_code |= TCPF_ACK | TCPF_RST;
	pnewtcp->tcp_window = 0;
	pnewtcp->tcp_cksum = 0;
	pnewtcp->tcp_urgptr = 0;

	tcph2net (pnewtcp);
	pnewtcp->tcp_cksum = tcpcksum (pnewip, pnewtcp);
	ipsend (pep);

	return OK;
}
