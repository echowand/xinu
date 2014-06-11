#include <conf.h>
#include <kernel.h>
#include <mark.h>
#include <bufpool.h>
#include <netdefs.h>
#include <ether.h>
#include <ip.h>
#include <tcp.h>
#include <tcb.h>
#include <stdio.h>

/*
 * tcpclassify - The root of the data plane
 */
PROCESS tcpclassify ()
{
	int i, partial, complete, entry;
	struct ep *pep;
	struct ip *pip;
	struct tcp *ptcp;
	u_short len;

	while (1) {
		if (read (Tcp.tcpdev, &pep, sizeof(pep)) == SYSERR)
			return SYSERR;
		pip = (struct ip *)pep->ep_data;
		ptcp = (struct tcp *)((char *)pip + IP_HLEN(pip));
		len = net2hs (pip->ip_len);

		/* Reject broadcast or multicast packets out of hand */
		if ((pep->ep_flags & EPF_BCAST) || IP_ISMULTI(pip->ip_dst)) {
			freebuf (pep);
			continue;
		}

		/* Validate header lengths and the TCP checksum */
		if (len < IP_HLEN(pip) + TCP_MINHLEN
		    || len < IP_HLEN(pip) + TCP_NETHLEN(ptcp)
		    || tcpcksum (pip, ptcp) != 0) {
			freebuf (pep);
			continue;
		}

		tcpnet2h (ptcp);

		partial = complete = -1;
		wait (Tcp.tcpmutex);
		for (i = 0; i < Ntcp; i++) {
			switch (tcbtab[i].tcb_state) {
			case TCB_FREE:
				continue;
			case TCB_LISTEN:
				/* Accept this only if the current entry
				 * matches and is more specific */
				if (((tcbtab[i].tcb_lip == IPADDR_ANY
				      && partial == -1)
				     || pip->ip_dst == tcbtab[i].tcb_lip)
				    && ptcp->tcp_dport == tcbtab[i].tcb_lport)
					partial = i;
				continue;
			default:
				if (pip->ip_src == tcbtab[i].tcb_rip
				    && pip->ip_dst == tcbtab[i].tcb_lip
				    && ptcp->tcp_sport == tcbtab[i].tcb_rport
				    && ptcp->tcp_dport == tcbtab[i].tcb_lport) {
					complete = i;
					break;
				}
				continue;
			}
		}
		if (complete != -1)
			entry = complete;
		else if (partial != -1)
			entry = partial;
		else {
			signal (Tcp.tcpmutex);
			tcpreset (pip, ptcp);
			freebuf (pep);
			continue;
		}

		wait (tcbtab[entry].tcb_mutex);
		signal (Tcp.tcpmutex);

		tcpinput (&tcbtab[entry], pip, ptcp);

		signal (tcbtab[entry].tcb_mutex);

		freebuf (pep);
	}

	return SYSERR;
}
