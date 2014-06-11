/* tcpreset.c  -  tcpreset */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  tcpreset  -  Send a TCP RESET segment
 *------------------------------------------------------------------------
 */
int32	tcpreset(
	  struct ip	*pip,		/* Ptr to old IP datagram hdr	*/
	  struct tcp	*ptcp		/* Ptr to TCP in old packet	*/
	)
{
	struct	netpacket *pkt;		/* ptr to network packet	*/
	struct	ip	*pnewip;	/* ptr to IP in new packet	*/
	struct	tcp	*pnewtcp;	/* ptr to TCP in new packet	*/
	int32		iface;		/* Interface to use		*/
	uint32		lip;		/* Local IP address to use	*/
	uint32		rip;		/* Remote IP address (reply)	*/
	int32		len;		/* Length of the TCP data	*/

	/* Did we already send a RESET? */

	if (ptcp->tcp_code & TCPF_RST)
		return SYSERR;

	/* Allocate a buffer */

	pkt = (struct netpacket *)getbuf(netbufpool);

	if ((int32)pkt == SYSERR) {
		return SYSERR;
	}

	/* Compute length of TCP data (needed for ACK) */

	len = pip->ip_len - IP_HDR_LEN - TCP_HLEN(ptcp);

	/* Obtain remote IP address */

	rip = pip->ip_src;
	lip = pip->ip_dst;

	/* Set interface in packet buffer */

	iface = ip_route(rip);
	if (iface == SYSERR) {
		return SYSERR;
	}
	pkt->net_iface = iface;

	/* Create TCP packet in pkt */

	memcpy((char *)pkt->net_ethsrc,if_tab[iface].if_macucast,
							ETH_ADDR_LEN);
	pkt->net_ethtype = 0x0800;	/* Type is IP */

	/* Fill in IP header */

	pkt->net_ipvh = 0x45;		/* IP version and hdr length	*/
	pkt->net_iptos = 0x00;		/* Type of service		*/
	pkt->net_iplen= IP_HDR_LEN + TCP_HLEN(ptcp);
					/* total datagram length	*/
	pkt->net_ipid = ident++;	/* datagram gets next IDENT	*/
	pkt->net_ipfrag = 0x0000;	/* IP flags & fragment offset	*/
	pkt->net_ipttl = 0xff;		/* IP time-to-live		*/
	pkt->net_ipproto = IP_TCP;	/* datagram carries TCP		*/
	pkt->net_ipcksum = 0x0000;	/* initial checksum		*/
	pkt->net_ipsrc = lip;		/* IP source address		*/
	pkt->net_ipdst = rip;		/* IP destination address	*/

	/* Use vol2 structures */

	pnewip = (struct ip *)&pkt->net_ipvh;
	pnewtcp = (struct tcp *)&pkt->net_tcpsport;

	/* Fill in TCP header */

	pnewtcp->tcp_sport = ptcp->tcp_dport;
	pnewtcp->tcp_dport = ptcp->tcp_sport;
	if (ptcp->tcp_code & TCPF_ACK) {
		pnewtcp->tcp_seq = ptcp->tcp_ack;
	} else {
		pnewtcp->tcp_seq = 0;
	}
	pnewtcp->tcp_ack = ptcp->tcp_seq + len;
	if (ptcp->tcp_code & (TCPF_SYN|TCPF_FIN)) {
		pnewtcp->tcp_ack++;
	}
	pnewtcp->tcp_code = TCP_HDR_LEN << 10;
	pnewtcp->tcp_code |= TCPF_ACK | TCPF_RST;
	pnewtcp->tcp_window = 0;
	pnewtcp->tcp_cksum = 0;
	pnewtcp->tcp_urgptr = 0;

	/* Call ip_send to send the datagram */

	ip_send(pkt);
	return OK;
}
