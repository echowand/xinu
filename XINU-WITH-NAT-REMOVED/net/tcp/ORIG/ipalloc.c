/* ipalloc.c - _GETBUF _ipgetbuf _ipalloc */

#include <conf.h>
#include <kernel.h>
#include <io.h>
#include <netdefs.h>
#include <ether.h>
#include <netif.h>
#include <net.h>
#include <ip.h>
#include <ipif.h>
#include <iproute.h>
#include <stdio.h>

#define _GETBUF(block) (block ? (getbuf) : (nbgetbuf))


/*------------------------------------------------------------------------
 * _ipgetbuf - choose a buffer pool size and allocate a buffer
 *------------------------------------------------------------------------
 */
inline struct ep * _ipgetbuf(int buflen, int block)
{
	if (buflen <= IP_SMLBUFSZ)
		return(struct ep *)_GETBUF(block)(Net.netpool);
	else if (buflen <= IP_MEDBUFSZ)
		return(struct ep *)_GETBUF(block)(Ip.mbpool);
	else if (buflen <= IP_LRGBUFSZ)
		return(struct ep *)_GETBUF(block)(Ip.lbpool);
	else
		return(NULL);
}

/*------------------------------------------------------------------------
 * _ipalloc - allocate a new buffer and fill in the basic components
 *------------------------------------------------------------------------
 */
struct ep *_ipalloc(int ipdev, int len, IPaddr src, IPaddr dst, int block)
{
	struct devsw *pdev;
	struct ipdev *pipd;
	struct ep *pep;
	struct ip *pip;
	struct route *prt;

	if (isbaddev(ipdev) || (pdev = &devtab[ipdev])->dvminor > Nip
	    || ipdev != (pipd = &ipdevtab[pdev->dvminor])->ipd_dvnum)
		return(NULL);

	/* Get a new buffer */
	
	pep = _ipgetbuf(len + IP_MINHLEN + sizeof(struct eh)
			+ sizeof(struct ehx), block);
	if (pep == (struct ep *)SYSERR || pep == NULL)
		return(NULL);

	/* Fill in the components of the IP structure */
	
	pip = (struct ip *)pep->ep_data;
	pip->ip_ver = 4;
	pip->ip_hlen = IP_MINHLEN >> 2;
	pep->ep_inif = NI_LOCAL;
	pep->ep_outif = NI_LOCAL;
	pep->ep_len = IP_HLEN(pip) + len + sizeof(pep->ep_eh);
	pep->ep_type = EPT_IP;
	pep->ep_flags = 0;
	pip->ip_tos = pipd->ipd_tos;
	pip->ip_len = hs2net(IP_HLEN(pip) + len);
	pip->ip_id = Ip.fragid++;
	pip->ip_offset = 0;
	pip->ip_ttl = pipd->ipd_ttl;
	pip->ip_proto = pipd->ipd_proto;

	pip->ip_src = src;
	pip->ip_dst = dst;

	/* If destination is the IPADDR_ANY, return the buffer */
	
	if (dst == IPADDR_ANY)
		return pep;

	/* If the source address is IPADDR_ANY, determine a source addr */
	
	if (src == IPADDR_ANY) {
		
		/* Perform a routing lookup to determine the interface the
		   packet will be delivered on */
		
		if ((prt = rtget(dst)) == NULL) {
			
			/* If the lookup failed, pick the IP address of
			   the primary interface */
			
			pip->ip_src = ipif[NI_PRIMARY].ipif_addr;
			
		} else {

			/* If the lookup succeed, pick the IP address of 
			   the if the packet will be delivered on */
			
			pip->ip_src = ipif[prt->rt_ifnum].ipif_addr;
			rtfree(prt);
			
		}
	}
	
	return pep;
}
