/* tcp_in.c  -  tcp_in */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  tcp_in  -  process an incoming TCP segment
 *------------------------------------------------------------------------
 */
void	tcp_in(
	  struct netpacket *ptr		/* Ptr to arriving packet	*/
	)
{
	int32	i;			/* Iterates through TCBs	*/
	int32	partial;		/* -1 or slot of partial match	*/
	int32	complete;		/* -1 or slot of full match	*/
	byte	ebcast[] = {0xff,0xff,0xff,0xff,0xff,0xff};
	int32	entry;
	struct	ep	*pep;		/* Ptr to Ethernet header	*/
	struct	ip	*pip;		/* Ptr to IP header		*/
	struct	tcp	*ptcp;		/* Ptr to TCP segment		*/
	uint16	len;			/* Length of the segment	*/
	struct	tcb	*ptcb;		/* Ptr to TCB entry		*/

	/* Get pointers to Ethernet header, IP header and TCP header */

	pep = (struct ep *) ptr;
	pip = (struct ip *)pep->ep_data;
	ptcp = (struct tcp *)((char *)pip + IP_HDR_LEN);
	len = pip->ip_len;

	tcp_ntoh(ptcp);

	/* Reject broadcast or multicast packets out of hand */

	if ((memcmp(pep->ep_dst,ebcast,ETH_ADDR_LEN)== 0) ||
				IP_ISMULTI(pip->ip_dst)) {
		freebuf ((char *)ptr);
		return;
	}
/*DEBUG*/ kprintf("\n\nIN:\n");pdumph(ptr);

	/* Validate header lengths */

	if (len < (IP_HDR_LEN + TCP_HLEN(ptcp)) ) {
		freebuf ((char *)ptr);
		return;
	}

	partial = complete = -1;

	/* Insure exclusive use */

	wait (Tcp.tcpmutex);
	for (i = 0; i < Ntcp; i++) {
		ptcb = &tcbtab[i];
		if (ptcb->tcb_state == TCB_FREE) {

			/* Skip unused emtries */
			continue;
		} else if (ptcb->tcb_state == TCB_LISTEN) {

			/* Check partial match */

			/* Accept only if the current entry	*/
			/* matches and is more specific		*/

			if (((ptcb->tcb_lip == IPADDR_ANY
			      && partial == -1)
			     || pip->ip_dst == ptcb->tcb_lip)
			    && ptcp->tcp_dport == ptcb->tcb_lport) {
				partial = i;
			}
			continue;
		} else {

			/* Check for exact match */

			if (pip->ip_src == tcbtab[i].tcb_rip
			    && pip->ip_dst == ptcb->tcb_lip
			    && ptcp->tcp_sport == ptcb->tcb_rport
			    && ptcp->tcp_dport == ptcb->tcb_lport) {
				complete = i;
				break;
			}
			continue;
		}
	}

	/* See if full match found, partial match found, or none */

	if (complete != -1) {
		entry = complete;
	} else if (partial != -1) {
		entry = partial;
	} else {

		/* No match - send a reset and drop the packet */

		signal (Tcp.tcpmutex);
		tcpreset (pip, ptcp);
		freebuf ((char *)ptr);
		return;
	}

	/* Wait on mutex for TCB and release global mutex */

	wait (tcbtab[entry].tcb_mutex);
	signal (Tcp.tcpmutex);

	/* Process the segment according to the state of the TCB */

	tcpdisp (&tcbtab[entry], pip, ptcp);
	signal (tcbtab[entry].tcb_mutex);
	freebuf ((char *)ptr);

	return;
}
