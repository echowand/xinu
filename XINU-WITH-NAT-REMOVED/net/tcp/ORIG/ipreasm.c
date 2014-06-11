/* ipreasm.c - ipreassemble, ipfcadd, ipfccoallesce, ipfcassemble, 
	       ipfctimeout, ipfcalloc, ipfcclearlru, ipfcclear,
	       ipeq, ipprintfc */

#include <conf.h>
#include <kernel.h>
#include <mark.h>
#include <bufpool.h>
#include <netdefs.h>
#include <ether.h>
#include <netif.h>
#include <net.h>
#include <ip.h>
#include <ipopt.h>
#include <icmp.h>
#include <mq.h>
#include <timer.h>
#include <stdio.h>

extern int ctr1000;

/* fragment cache */

struct ipfc ipfctab[IPFC_N];

/*------------------------------------------------------------------------
 *  ipreassemble - reassmbles IP fragments received from slowip
 *------------------------------------------------------------------------
 */
void ipreassemble(struct ep *pep, struct ip *pip) {
	int i, lru, lruts;
	struct ip *iphole;

	/* Clear least recently used fragment if the cache if full */

	if (Ip.nfrags >= IPFC_FRAGMAX)
		ipfcclearlru();

	/* Increment the cache fragment counter */

	Ip.nfrags++;

	pep->ep_next = NULL;

	/* Determine which packet the fragment belongs to */

	for (i = 0; i < IPFC_N; i++) {
		if (ipfctab[i].head != NULL && 
		    (ipeq(pip, (struct ip*) ipfctab[i].head->ep_data))) {

			/* Add fragment to the packet chain */

			ipfcadd (&ipfctab[i], pep, pip);

			/* Assemble pkt if all the frgts have arrived */

			if (ipfctab[i].hole != NULL) {
				iphole = (struct ip *) 
						ipfctab[i].hole->ep_data;
				if (!net2hs(iphole->ip_offset & IPF_MF))
					ipfcassemble(&ipfctab[i]);
				ipfctab[i].rcntts = ctr1000;
				break;
			}
		}
	}

	/* Create a new entry if this if the first fragment */

	if (i == IPFC_N) {
		lruts = ctr1000 + 1;
		lru = 0;
		
		/* Determine a free slot and/or calculate lru */

		for (i = 0; i < IPFC_N; i++) {
			if (ipfctab[i].rcntts < lruts) {
				lru = i;
				lruts = ipfctab[lru].rcntts;
			}
			if (ipfctab[i].head == NULL) {
				ipfcalloc(&ipfctab[i], pep);
				break;
			}
		}

		/* If a free slot is not found, replace the lru */

		if (i == IPFC_N)
			ipfcalloc(&ipfctab[lru], pep);
	}
	
	return;
}

/*------------------------------------------------------------------------
 *  ipfcadd - add a fragment to slot in fragment cache
 *------------------------------------------------------------------------
 */
int ipfcadd(struct ipfc *pfc, struct ep *pep, struct ip *pip) {
	struct ep *cur, *prev;
	struct ip *curip, *nextip, *holeip;

	/* Determine where the first hole in the packet is */
	
	if (pfc->hole == NULL) {

		cur = pfc->head;
		prev = NULL;

	} else {

		holeip = (struct ip *) pfc->hole->ep_data;
		if (IP_OFF(holeip) + net2hs(holeip->ip_len)
		    >= IP_OFF(pip) + net2hs(pip->ip_len)) {

			/* If it doesn't bring anything new, free it. */

			freebuf(pep);
			return OK;
		}
		cur = pfc->hole->ep_next;
		prev = pfc->hole;

	}

	/* Determine where the fragment fits in the chain */

	while (cur != NULL) {
		curip = (struct ip*) cur->ep_data;
		if (IP_OFF(pip) + net2hs(pip->ip_len) < 
		    IP_OFF(curip) + net2hs(curip->ip_len)) {
			pep->ep_next = cur;
			if (prev == NULL)
				pfc->head = pep;
			else
				prev->ep_next = pep;

			/* Group fragments data together */

			ipfccoallesce (pep, pip, cur, curip);

			if (prev != NULL)
				
				/* Group fragments data */

				ipfccoallesce(prev, 
					      (struct ip*)prev->ep_data,
					      pep, 
					      pip);
			break;
		}
		prev = cur;
		cur = cur->ep_next;
	}

	/* Add the fragment on the tail of the chain and try to group
	   fragment data */

	if (cur == NULL) {
		if (prev == NULL)
			pfc->head = pep;
		else {
			prev->ep_next = pep;
			ipfccoallesce(prev, 
				      (struct ip*)prev->ep_data, 
				      pep, 
				      pip);
		}
	}
	
	/* Recalculate the hole position */

	if (IP_OFF((struct ip*)pfc->head->ep_data) != 0) {
		pfc->hole = NULL;
	} else {
		cur = pfc->hole;
		if (cur == NULL)
			cur = pfc->head;
		curip = (struct ip *)cur->ep_data;
		while (cur->ep_next != NULL) {
			nextip = (struct ip *)cur->ep_next->ep_data;
			if (IP_OFF(curip) + net2hs(curip->ip_len)
			    <= IP_OFF(nextip))
				break;
			cur = cur->ep_next;
			curip = nextip;
		}
		pfc->hole = cur;
	}
	return OK;
}

/*------------------------------------------------------------------------
 *  ipfccoallesce - coallesce (group) fragment data
 *------------------------------------------------------------------------
 */
void ipfccoallesce(struct ep *pep1, struct ip *pip1,
		   struct ep *pep2, struct ip *pip2) {
	int nlen;
	u_short offset;
	nlen = net2hs(pip1->ip_len) + 
	       net2hs(pip2->ip_len) - 
	       IP_HLEN(pip2);

	/* Check if two fragments can be grouped together and copy the
	   second fragment's data into the free part of the first pep */

	if (IP_OFF(pip1) <= IP_OFF(pip2)
	    && IP_OFF(pip1) + net2hs(pip1->ip_len)
	      - IP_HLEN(pip1) >= IP_OFF(pip2)
	    && sizeof(struct ehx) + sizeof(struct eh) + nlen <= 
		IP_SMLBUFSZ) {
		
		blkcopy(pep1->ep_data + IP_HLEN(pip1) + 
			IP_OFF(pip2) - IP_OFF(pip1),
			pep2->ep_data + IP_HLEN(pip2),
			net2hs(pip2->ip_len) - IP_HLEN(pip2));
		pip1->ip_len = hs2net(nlen);
		offset = net2hs(pip1->ip_offset);
		offset &= ~(IPF_MF);
		offset |= net2hs(pip2->ip_offset) & IPF_MF;
		pip1->ip_offset = hs2net(offset);
		pip1->ip_cksum = 0;
		pip1->ip_cksum = cksum(pip1, IP_HLEN(pip1));
		pep1->ep_len = nlen + sizeof(struct eh);
		pep1->ep_next = pep2->ep_next;
		Ip.nfrags--;
		freebuf(pep2);
	}
}

/*------------------------------------------------------------------------
 *  ipfcassemble - assembles a packet from its fragments and delivers it
 *------------------------------------------------------------------------
 */
void ipfcassemble(struct ipfc *pfc) {
	struct ep *cur, *npep;
	struct ip *curip, *npip, *headip, *holeip;
	int len;

	headip = (struct ip*)pfc->head->ep_data;
	holeip = (struct ip*)pfc->hole->ep_data;
	
	len = IP_OFF(holeip) + net2hs(holeip->ip_len) - IP_HLEN(holeip);

	/* Get buffer for the new packet */

	npep = _ipgetbuf(len + IP_HLEN(headip) +
		sizeof(struct eh) + sizeof(struct ehx), FALSE);
	if (npep == NULL || npep == (struct ep*)SYSERR) {
		ipfcclear(pfc);
		return;
	}
	
	/* Copy header from the first fragment into the new buffer */

	blkcopy(npep, pfc->head, sizeof(struct eh) + sizeof(struct ehx)
		+ net2hs(headip->ip_len));
	npip = (struct ip*) npep->ep_data;
	cur = pfc->head->ep_next;

	/* Copy fragments contents in new buffer */

	while (cur != NULL) {
		curip = (struct ip*)cur->ep_data;
		blkcopy (npep->ep_data + IP_HLEN(npip) + IP_OFF(curip),
			(u_char*)curip + IP_HLEN(curip),
			net2hs(curip->ip_len) - IP_HLEN(curip));
		cur = cur->ep_next;
	}

	/* Clear the fragment cache slot */

	ipfcclear(pfc);

	/* Fix-up new packet and deliver */

	npip->ip_len = hs2net(IP_HLEN(npip) + len);
	npip->ip_offset = 0;
	npip->ip_cksum = 0;
	npip->ip_cksum = cksum(npip, IP_HLEN(npip));
	npep->ep_next = NULL;
	npep->ep_len = IP_HLEN(npip) + len + sizeof(struct eh);
	ipdeliver(npep, npip);
}

/*------------------------------------------------------------------------
 *  ipfctimeout - free fragmentation cache entries that have expired
 *------------------------------------------------------------------------
 */
void ipfctimeout() {
	int i;
	struct ipfc *pfc;

	/* Iterate through the fragmentation cache entries,
	   check if they expired and free them up */	

	for (i = 0; i < IPFC_N; i++) {
		if ((pfc = &ipfctab[i])->head != NULL && 
		    pfc->firstts - (ctr1000 - IPFC_TIMEOUT) < 0) {
			if (IP_OFF((struct ip*) pfc->head->ep_data) == 0)
				icmpreply(ICMPT_TIMEX, 
					  ICMPC_FRAG, 
					  0,
					  pfc->head);
				ipfcclear (pfc);
		}
	}
}


/*------------------------------------------------------------------------
 *  ipfcalloc - initializes a slot in the fragment cache
 *------------------------------------------------------------------------
 */
void ipfcalloc(struct ipfc *pfc, struct ep *pep) {
	ipfcclear(pfc);
	pfc->head = pep;
	pfc->hole = NULL;
	pfc->firstts = ctr1000;
	pfc->rcntts = ctr1000;
	tmset(IPFC_TIMEOUT + 5, Ip.slowipmq, NULL);
}

/*------------------------------------------------------------------------
 *  ipfcclearlru - determine and clear the lru slot from the frg cache
 *------------------------------------------------------------------------
 */
void ipfcclearlru() {
	int i, lru, lruts;

	lruts = ctr1000 + 1;
	lru = 0;

	/* Iterate through the fragment cache to determine the lru */

	for (i = 0; i < IPFC_N; i++) {
		if (ipfctab[i].head != NULL && 
		    ipfctab[i].rcntts - lruts < 0)
			lruts = ipfctab[lru = i].rcntts;
	}

	/* Clear the lru slot */

	ipfcclear(&ipfctab[lru]);
}

/*------------------------------------------------------------------------
 *  ipfcclear - clear a slot from the fragment cache
 *------------------------------------------------------------------------
 */
void ipfcclear(struct ipfc *pfc) {
	struct ep *cur, *next;
	cur = pfc->head;
	while (cur != NULL) {
		next = cur->ep_next;
		freebuf(cur);
		cur = next;
		Ip.nfrags--;
	}
	pfc->head = NULL;
}

/*------------------------------------------------------------------------
 *  ipeq - test if two packets are part of the same fragment
 *------------------------------------------------------------------------
 */
int ipeq(struct ip *pip1, struct ip *pip2) {
	return(pip1->ip_src == pip2->ip_src
		&& pip1->ip_dst == pip2->ip_dst
		&& pip1->ip_id == pip2->ip_id
		&& pip1->ip_proto == pip2->ip_proto);
}

/*------------------------------------------------------------------------
 *  ipprintfc - print the fragment cache
 *------------------------------------------------------------------------
 */
int ipprintfc(int stdout) {
        struct ep *cur;
        struct ip *curip;
        int i;
        struct ipfc *pfc;

	/* Iterate through the fragment cache and print its 
	   contents to stdout */

        for (i = 0; i < IPFC_N; i++) {
                pfc = &ipfctab[i];
                fprintf(stdout, 
			"\n[%d] head=0x%x, hole=0x%x, fts=%d, rts=%d\n",
                        i, 
			(u_int) pfc->head, 
			(u_int) pfc->hole,
                        pfc->firstts, 
			pfc->rcntts);
                cur = pfc->head;
                while (cur != NULL) {
                        curip = (struct ip*) cur->ep_data;
                        fprintf(stdout, 
				" [0x%x : %d-%d]", 
				(u_int) cur,
                                (int)IP_OFF(curip), 
				(int)(IP_OFF(curip)
                                	+ net2hs(curip->ip_len) 
					- IP_HLEN(curip)));
                        cur = cur->ep_next;
                }
        }
        fprintf (stdout, "\n");

        return OK;
}

