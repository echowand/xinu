/* ipsend.c - ipsend */

#include <conf.h>
#include <kernel.h>
#include <mq.h>
#include <io.h>
#include <netdefs.h>
#include <netif.h>
#include <ether.h>
#include <ip.h>
#include <ipif.h>
#include <iproute.h>
#include <stdio.h>

/*------------------------------------------------------------------------
 *  ipsend - send an IP packet (called by upper layers)
 *------------------------------------------------------------------------
 */
int ipsend(struct ep *pep) {

	struct ip *pip;		/* pointer to IP packet header */
	struct route *prt;	/* pointer to an IP table entry */

	/* Set pip to the start of the IP packet */

	pip = (struct ip *)pep->ep_data;

	/* Check if the IP destination is valid */

	if (pip->ip_dst == IPADDR_ANY) {
		freebuf(pep);
		return SYSERR;
	}

	/* Fill in source IP address if caller didn't specify it */

	if (pip->ip_src == IPADDR_ANY) {

		/* Lookup the source IP in the routing table */

		if ((prt = rtget(pip->ip_src)) == NULL) { /* failed */

			/* Set IP source to outgoing's if address for
			   broadcast packets, NI_PRIMARY's otherwise */

			if (pip->ip_dst == IPADDR_BCAST)
				pip->ip_src=ipif[pep->ep_outif].ipif_addr;
			else
				pip->ip_src=ipif[NI_PRIMARY].ipif_addr;

		} else { /* lookup succeded */
			pip->ip_src = ipif[prt->rt_ifnum].ipif_addr;
			rtfree(prt);
		}
	}

	/* Compute IP checksum */

	pip->ip_cksum = 0;
	pip->ip_cksum = cksum(pip, IP_HLEN(pip));

	/* Send packet to fastip */

	if (mqsend(Ip.fastipmq, (int) pep) == SYSERR)
		freebuf(pep);

	/* Return OK even if prev call failed (best effort delivery) */

	return OK;
}
