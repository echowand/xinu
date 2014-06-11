/* iproute.c - iproute */

#include <conf.h>
#include <kernel.h>
#include <mark.h>
#include <bufpool.h>
#include <netif.h>
#include <ether.h>
#include <ip.h>
#include <ipif.h>
#include <mq.h>

/*------------------------------------------------------------------------
 *  iproute - determine next hop destination for an IP packet
 *------------------------------------------------------------------------
 */
int iproute(struct ep *pep, struct ip *pip) {

	/* Handle broadcast, multicast packets generated locally */

	if (pep->ep_inif == NI_LOCAL && (pip->ip_dst == IPADDR_BCAST
					 || IP_ISMULTI(pip->ip_dst))) {
		if (pep->ep_outif == NI_LOCAL) {
			freebuf(pep);
			return SYSERR;
		}
		pep->ep_nexthop = pip->ip_dst;
		return OK;
	}

	/* If there is no configured address on the incoming
	   interface, set the packet to be delivered locally */

	if (ipif[pep->ep_inif].ipif_addr == IPADDR_ANY) {
		pep->ep_outif = NI_LOCAL;
		return OK;
	}

	/* Handle incoming broadcast */

	if ((pip->ip_dst == IPADDR_BCAST 
	     && pep->ep_inif != NI_LOCAL)
	     || (pip->ip_dst == ipif[pep->ep_inif].ipif_bcast)) {
		pep->ep_flags |= EPF_BCAST;
		pep->ep_outif = NI_LOCAL;
		return OK;
	}

	/* Determine next hop for a uniscast destination */

	if (iplookup(pep) == SYSERR) {
		pep->ep_flags = EPF_DESTUN;
		if (mqsend(Ip.slowipmq, (int)pep) == SYSERR)
			freebuf(pep);
		return SYSERR;
	}

	return OK;
}
