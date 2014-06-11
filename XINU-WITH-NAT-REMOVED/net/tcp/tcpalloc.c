/* tcpalloc.c  -  tcpalloc */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  tcpalloc  -  allocate a buffer for an IP datagram carrying TCP
 *------------------------------------------------------------------------
 */
struct ep *tcpalloc(
	  struct tcb	*ptcb,		/* Ptr to a TCB			*/
	  int32		len		/* Length of the TCP segment	*/
	)
{
	struct	ep	*pep;		/* Ptr to Ethernet packet	*/
	struct	ip	*pip;		/* Ptr to IP area of packet	*/
	struct	tcp	*ptcp;		/* Ptr to TCP area of packet	*/
	int32		iface;		/* Interface to use 		*/
	struct netpacket *pkt;		/* Ptr to packet for new struct	*/

	/* Allocate a buffer for a TCB */

	pep = (struct ep *) getbuf(netbufpool);
	pkt = (struct netpacket *) pep;

	pip = (struct ip *)pep->ep_data;
	ptcp = (struct tcp *)((char *)pip + IP_HDR_LEN);

	/* Set Ethernet type */

	/* Fill in Ethernet header */

	iface = ptcb->tcb_iface;
	memcpy((char *)pkt->net_ethsrc,if_tab[iface].if_macucast,
							ETH_ADDR_LEN);
	pkt->net_ethtype = 0x0800;	/* Type is IP */

	/* Fill in IP header */

	pkt->net_ipvh = 0x45;		/* IP version and hdr length	*/
	pkt->net_iptos = 0x00;		/* Type of service		*/
	pkt->net_iplen= IP_HDR_LEN + TCP_HDR_LEN + len;
					/* Total datagram length	*/
	pkt->net_ipid = ident++;	/* datagram gets next IDENT	*/
	pkt->net_ipfrag = 0x0000;	/* IP flags & fragment offset	*/
	pkt->net_ipttl = 0xff;		/* IP time-to-live		*/
	pkt->net_ipproto = IP_TCP;	/* datagram carries TCP		*/
	pkt->net_ipcksum = 0x0000;	/* initial checksum		*/
	pkt->net_ipsrc = ptcb->tcb_lip; /* IP source address		*/
	pkt->net_ipdst = ptcb->tcb_rip;	/* IP destination address	*/

	/* Set the TCP port fields in the segment */

	ptcp->tcp_sport = ptcb->tcb_lport;
	ptcp->tcp_dport = ptcb->tcb_rport;

	/* Set the code field (includes Hdr length */

	ptcp->tcp_code = (TCP_HDR_LEN << 10);

	if (ptcb->tcb_state > TCB_SYNSENT) {
		ptcp->tcp_code |= TCPF_ACK;
		ptcp->tcp_ack = ptcb->tcb_rnext;
	}

	/* Set the window advertisement */

	ptcp->tcp_window = ptcb->tcb_rbsize - ptcb->tcb_rblen;

	/* Silly Window Avoidance belongs here... */

	/* Initialize the checksum and urgent pointer */
	ptcp->tcp_cksum = 0;
	ptcp->tcp_urgptr = 0;

	/* Return entire packet to caller */

	return pep;
}
