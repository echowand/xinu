/* ipfrag.c - ipfrag */

#include <conf.h>
#include <kernel.h>
#include <ether.h>
#include <netif.h>
#include <net.h>
#include <netdefs.h>
#include <ip.h>
#include <ipopt.h>
#include <stdio.h>

/*------------------------------------------------------------------------
 *  ipfrag - fragment an IP packet
 *------------------------------------------------------------------------
 */
void ipfrag(struct ep *pep, struct ip *pip) {

	struct ep *fragep;	/* pointer to fragment frame 	 */
	struct ip *fragip;	/* pointer to fragment IP header */
	int remaining;		/* remainging octets to fragment */
	int len;		/* fragment len			 */
	int offset;		/* current fragmentation offset  */
	int ipoff;		/* real fragment offset		 */
	int hlen;		/* fragment header len 		 */
	int origoff;		/* original IP frag offset 	 */

	/* Initialize loop */

	remaining = net2hs(pip->ip_len) - IP_HLEN(pip);
	offset = IP_HLEN(pip);
	origoff = (net2hs(pip->ip_offset) & IP_OFFMASK) << 3;

	/* Crate fragments until all data in the packet 
	   to be fragmented is sent 			*/

	while (remaining) {

		/* Get a new network buffer for another fragment */

		fragep = (struct ep *)getbuf(Net.netpool);

		/* Copy MAC header, Xinu extended header and IP header */

		blkcopy(fragep, pep, sizeof(struct ehx)
				    + sizeof(struct eh)
				    + sizeof(struct ip));

		/* Set fragip to the beginning of (fragment) IP packet */

		fragip = (struct ip *)fragep->ep_data;

		/* Copy all IP options for the first fragement, 
		   and call optfrag() to handle subsequent fragments */

		if (offset == IP_HLEN(pip))
			blkcopy(fragip->ip_data, pip->ip_data,
				IP_HLEN(pip) - IP_MINHLEN);
		else
			optfrag(pip, fragip);

		/* Set len to effective data length that can be sent 
		   in this fragment */

		len = nif[pep->ep_outif].ni_mtu - IP_HLEN(fragip);
		len = min(len, remaining);
		
		/* If this is not the last fragment, the length should 
		   be a multiple of 8 octets */

		if (len != remaining)
			len &= 0xfff8;

		/* Recalculate remaining octets */

		remaining -= len;
		hlen = IP_HLEN(fragip);

		/* Set the frame length and IP length */

		fragep->ep_len = sizeof(struct eh)+ hlen + len;
		fragip->ip_len = hs2net(len + hlen);

		/* Calculate IP offset and set more fragments if needed */

		ipoff = (origoff + offset - IP_HLEN(pip)) >> 3;
		if (remaining || net2hs(pip->ip_offset) & IPF_MF)
                                        ipoff |= IPF_MF;
		fragip->ip_offset = hs2net(ipoff);

		/* Calculate IP checksum */

		fragip->ip_cksum = 0;
		fragip->ip_cksum = cksum((char *)fragip, hlen);

		/* Copy fragment IP data */

		blkcopy((char *)fragip + hlen, 
				(char *)pip + offset, len);

		/* Increment offset */

		offset += len;

		/* Send fragment to the interface driver */

		ipdeliver(fragep, fragip);
	}
}

