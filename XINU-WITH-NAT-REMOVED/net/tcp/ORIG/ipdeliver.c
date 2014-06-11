/* ipdeliver.c - ipdeliver */

#include <conf.h>
#include <kernel.h>
#include <mark.h>
#include <bufpool.h>
#include <mq.h>
#include <ether.h>
#include <netif.h>
#include <net.h>
#include <ip.h>
#include <ipif.h>
#include <arp.h>
#include <icmp.h>

/*------------------------------------------------------------------------
 *  ipdeliver - forward an IP packet to a next hop or to the local stack
 *------------------------------------------------------------------------
 */
void ipdeliver(struct ep *pep, struct ip *pip) {

	int i;		/* index in the registered IP protocols table */
	int ret;	/* arplookup return code  */
	int proto;	/* local copy of the IP protocol number */

	/* If packet is incoming, deliver to the local protocol stack */

	if (pep->ep_outif == NI_LOCAL) { /* local interface => incoming */

		proto = pip->ip_proto;

		/* Handle incoming ICMP packets */

		if (proto == IPP_ICMP) {

			/* For an ICMP error, record the protocol	*/
			/*   number of original packet			*/

			if ((proto = icmpgetproto(pep)) == SYSERR) {
				freebuf(pep);
				return;
			}
		}

		/* Use the registered IP protocol device table	*/
		/* to demultiplex the packet			*/

		for (i = 0; i < Nip; i++) {

			/* If protocol matches, deposit packet on the 
			   associated protocol's message queue */

			if (ipdevtab[i].ipd_proto == proto) {
				if (mqsend(ipdevtab[i].ipd_mq, (int)pep)
						== SYSERR)
					freebuf(pep);
				break;
			}
		}

		/* Protocol unreachable: send to slowip for ICMP error */

		if (i == Nip) {
			pep->ep_flags |= EPF_PROTOUN;
			if (mqsend(Ip.slowipmq, (int)pep) == SYSERR)
				freebuf(pep);
			return;
		}
	} else {  /* packet is outgoing; forward to next hop */

		/* Do not forward incoming broadcast packets */

		if (pep->ep_inif != NI_LOCAL
		    && pip->ip_dst == ipif[pep->ep_outif].ipif_bcast) {
			freebuf(pep);
			return;
		}

		/* If TTL has expired, send to slowip for ICMP error */

		if (pep->ep_inif != NI_LOCAL && ipttl(pip) == 0) {
			if (mqsend(Ip.slowipmq, (int)pep) == SYSERR)
				freebuf(pep);
			return;
		}

		/* If ICMP redirect needed, send packet to slowip */

		if (!(pep->ep_flags & (EPF_REDIRECT | EPF_SRCROUTE))
		    && pep->ep_inif != NI_LOCAL
		    && pep->ep_inif == pep->ep_outif
		    && (pip->ip_src & ipif[pep->ep_outif].ipif_mask)
				== ipif[pep->ep_outif].ipif_net) {
			pep->ep_flags |= EPF_REDIRECT;
			if (mqsend(Ip.slowipmq, (int)pep) == SYSERR)
				freebuf(pep);
			return;
		}

		/* Send packet to ARP for address resolution */

		pep->ep_flags |= EPF_ARPDONE;
		ret = arplookup(pep->ep_nexthop, pep->ep_outif,
				 pep->ep_eh.eh_dst, pep, Ip.fastipmq);

		/* Take action, depending on ARP lookup */

		if (ret == ARP_FAIL) {		/* lookup failed */
			freebuf(pep);
			return;
		} else if (ret == ARP_MISS) {	/* ARP cache miss */
			return;
		}

		/* ARP lookup succeeded - send packet to if driver */

		ni_out(pep);
	}
}
