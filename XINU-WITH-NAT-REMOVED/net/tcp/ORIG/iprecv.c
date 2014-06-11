/* iprecv.c - iprecv */

#include <conf.h>
#include <kernel.h>
#include <mq.h>
#include <netdefs.h>
#include <ether.h>
#include <ip.h>
#include <stdio.h>

/* Warning: the following code is executed during a hardware interrupt */

/*------------------------------------------------------------------------
 *  iprecv - handle an incoming IP packet (called from a device driver)
 *------------------------------------------------------------------------
 */
void iprecv (struct ep *pep) {
	struct ip *pip = (struct ip *)pep->ep_data;

	/* Check basic validity */

	if (pep->ep_ehx.ehx_len < sizeof(struct eh) + IP_MINHLEN
	    || pep->ep_ehx.ehx_len < sizeof(struct eh) + 
				     net2hs(pip->ip_len)
	    || pip->ip_ver != 4 || IP_HLEN(pip) < IP_MINHLEN) {
		freebuf(pep);
		return;
	}

	/* Enqueue for the fastip process */

	if (mqsend (Ip.fastipmq, (int)pep) == SYSERR)
		freebuf (pep);
}
