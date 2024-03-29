/* net.h */

#define NETSTK		8192 		/* stack size for network setup */
#define NETPRIO		500    		/* network startup priority 	*/

/* Constants used in the networking code */

#define	ETH_ARP     0x0806		/* Ethernet type for ARP	*/
#define	ETH_IP      0x0800		/* Ethernet type for IP		*/
#define	ETH_IPv6    0x86DD		/* Ethernet type for IPv6	*/

/* Format of an Ethernet packet carrying IPv4 and UDP */

#pragma pack(2)
struct	netpacket	{
	byte	net_ethdst[ETH_ADDR_LEN];/* Ethernet dest. MAC address	*/
	byte	net_ethsrc[ETH_ADDR_LEN];/* Ethernet source MAC address	*/
	uint16	net_ethtype;		/* Ethernet type field		*/
	byte	net_ipvh;		/* IP version and hdr length	*/
	byte	net_iptos;		/* IP type of service		*/
	uint16	net_iplen;		/* IP total packet length	*/
	uint16	net_ipid;		/* IP datagram ID		*/
	uint16	net_ipfrag;		/* IP flags & fragment offset	*/
	byte	net_ipttl;		/* IP time-to-live		*/
	byte	net_ipproto;		/* IP protocol (actually type)	*/
	uint16	net_ipcksum;		/* IP checksum			*/
	uint32	net_ipsrc;		/* IP source address		*/
	uint32	net_ipdst;		/* IP destination address	*/
	union {
	 struct {
	  uint16 	net_udpsport;	/* UDP source protocol port	*/
	  uint16	net_udpdport;	/* UDP destination protocol port*/
	  uint16	net_udplen;	/* UDP total length		*/
	  uint16	net_udpcksum;	/* UDP checksum			*/
	  byte		net_udpdata[1500-28];/* UDP payload (1500-above) */
	 };
		/* NOTE - THIS IS FAKE AND NEEDS TO BE UDPATED */
	 	/* Attemp to update....*/
	 struct {
	  uint16 	net_tcpsport;	/* TCP source protocol port	*/
	  uint16	net_tcpdport;	/* TCP destination protocol port*/
	  uint32	net_tcpseq;	/* Sequence number		*/
	  uint32	net_tcpack;	/* Acknowledgment		*/
	  uint16	net_tcpcode;	/* Segment type (TCPF_* above)	*/
	  uint16	net_tcpwindow;	/* Advertised window size	*/
	  uint16	net_tcpcksum;	/* Segment checksum		*/
	  uint16	net_tcpurgptr;	/* Urgent pointer		*/
	  byte		net_tcpdata[1500-42];/* TCP payload (1500-above)*/
	 };
	 struct {
	  byte		net_ictype;	/* ICMP message type		*/
	  byte		net_iccode;	/* ICMP code field (0 for ping)	*/
	  uint16	net_iccksum;	/* ICMP message checksum	*/
	  uint16	net_icident; 	/* ICMP identifier		*/
	  uint16	net_icseq;	/* ICMP sequence number		*/
	  byte		net_icdata[1500-28];/* ICMP payload (1500-above)*/
	 };
	};
	int16	net_iface;		/* Interface over which packet	*/
					/*  arrived (placed here beyond	*/
					/*  the actual packet so we can	*/
					/*  pass the packet address to	*/
					/*  write			*/
};
#pragma pack()

#define	PACKLEN	sizeof(struct netpacket)

extern	bpid32	netbufpool;		/* ID of net packet buffer pool	*/

extern	bool8	host;			/* is this system a host?	*/
extern	int32	bingid;			/* Bing ID in use		*/
