/* tcpack.c  -  tcpack */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  tcpack  -  Send or schedule transmission of a TCP Acknowledgement
 *------------------------------------------------------------------------
 */
void	tcpack(
	  struct tcb	*ptcb,		/* Ptr to the TCB		*/
	  int32		force		/* Should we force transmission?*/
	)
{
	struct ep	*pep; 		/* Ptr to Ethernet packet	*/
	struct ip	*pip;		/* Ptr to IP datagram		*/
	struct tcp	*ptcp;		/* Ptr to TCP segment		*/

	/* If not forcing transmission, schedule an event */

	if (!force) {
		if (!(ptcb->tcb_flags & TCBF_NEEDACK)) {
			return;
		}
		if (!(ptcb->tcb_flags & TCBF_ACKPEND)) {
			ptcb->tcb_flags |= TCBF_ACKPEND;
			tcptmset (TCP_ACKDELAY, ptcb, TCBC_DELACK);
			return;
		}
	}

	/* If we reach this point, an ACK is pending, or the caller	*/
	/*   has specified forcing an ACK, so go send it now		*/

	if (ptcb->tcb_flags & TCBF_ACKPEND) {
		tcptmdel (ptcb, TCBC_DELACK);
	}

	/* Allocate a datagram buffer and fill in IP header */

	pep = tcpalloc (ptcb, 0);
	pip = (struct ip *)pep->ep_data;
	ptcp = (struct tcp *)((char *)pip + IP_HDR_LEN);

	/* Fill in TCP header */

	ptcp->tcp_seq = ptcb->tcb_snext;

	ptcb->tcb_flags &= ~(TCBF_NEEDACK | TCBF_ACKPEND);

	ip_send ((struct netpacket *)pep);
	return;
}
