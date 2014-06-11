/* iplookup.c - iplookup */

#include <conf.h>
#include <kernel.h>
#include <ether.h>
#include <ip.h>
#include <ipif.h>
#include <iproute.h>

/*------------------------------------------------------------------------
 *  iplookup - lookup a destination in the routing table
 *------------------------------------------------------------------------
 */
int iplookup(struct ep *pep) {
        struct route *prt;	/* routing table entry */

	/* Check if pep is valid */

        if (pep == NULL) return SYSERR;

	/* Perform an IP table lookup for ip_dst */

        prt = rtget(((struct ip *)(pep->ep_data))->ip_dst);

	/* Check if a match was found */

        if (prt) {	/* a match was found */

		/* Set the outgoing interface for the packet */

                pep->ep_outif = prt->rt_ifnum;

		/* If pkt will be delivered on a local network, 
		   set nexthop to ip_dst, otherwise set it to rt_gw */

                if (prt->rt_gw == ipif[pep->ep_outif].ipif_addr)
                        pep->ep_nexthop = 
				((struct ip *)(pep->ep_data))->ip_dst;
                else
                        pep->ep_nexthop = prt->rt_gw;

		/* Set the routed flag for the packet */

                pep->ep_flags = pep->ep_flags | EPF_ROUTED;

		/* free the routing table entry and return */
		
                rtfree(prt);
		return OK;
        }

	/* Match was not found - return error */

        return SYSERR;
}
