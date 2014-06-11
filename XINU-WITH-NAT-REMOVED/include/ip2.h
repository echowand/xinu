/* ip2.h - additional definintions used by TCP */

#define IP_ALEN		sizeof(IPaddr)		/* IP address len	*/

#define IP_DEFTTL	255			/* IP default TTL	*/

#define IPADDR_BCAST	0xffffffff		/* IP broadcast address	*/
#define IPADDR_ANY	0x0			/* IP address any	*/

#define IPF_DF		0x4000			/* don't fragment mask	*/
#define IPF_MF		0x2000			/* more fragments mask	*/
#define IP_OFFMASK	0x1fff			/* fragment offset mask	*/

#define IP_HLEN(pip)	( (0xf & (pip)->ip_vhlen) << 2) /* IP hdr length*/
						/*    measured in bytes	*/
/* macro to get IP fragment offset */
#define IP_OFF(pip)	((net2hs((pip)->ip_offset) & IP_OFFMASK) << 3)

/* macro to check if an IP address is a multicast address */
#define IP_ISMULTI(x)	((x & 0x000000f0) == 0x000000e0)

struct ip {					/* IP header 		*/
	byte		ip_vhlen;		/* IP version & hdr len	*/
	byte		ip_tos;			/* IP type-of-service	*/
	uint16		ip_len;			/* IP Total length	*/
	uint16		ip_id;			/* IP identification	*/
	uint16		ip_offset;		/* IP flags & frag offst*/
	byte		ip_ttl;			/* IP time to live	*/
	byte		ip_proto;		/* IP protocol number	*/
	uint16		ip_cksum;		/* IP header checksum	*/
	IPaddr		ip_src;			/* IP source address	*/
	IPaddr		ip_dst;			/* IP destination addr	*/
	byte		ip_data[1];		/* start of IP data	*/
};

/* ether.h - Ethernet definitions */

/* Ethernet definitions and constants */

#define	EP_MAXMULTI	10	/* multicast address table size		*/

#define	EP_MINLEN	60	/* minimum packet length		*/
#define	EP_DLEN		1500	/* length of data field ep		*/
#define	EP_HLEN		14	/* size of Ethernet header		*/
#define	EP_ALEN		6	/* number of octets in physical address	*/
#define	EP_CRC		4	/* ether CRC (trailer)			*/
#define	EP_MAXLEN	(EP_HLEN+EP_DLEN)

#define	EP_RETRY	3	/* number of times to retry xmit errors	*/
#define	EP_BRC	"\377\377\377\377\377\377"/* Ethernet broadcast address	*/
#define EP_RTO 		300	/* time out in seconds for reads	*/

#define	EPT_LOOP	0x0060	/* type: Loopback			*/
#define	EPT_PUP		0x0200	/* type: Xerox PUP			*/
#define	EPT_IP		0x0800	/* type: Internet Protocol		*/
#define	EPT_ARP		0x0806	/* type: ARP				*/
#define	EPT_RARP	0x8035	/* type: Reverse ARP			*/

#define EPT_OTHERMASK	0xF000	/* mask for othernet ether type 	*/

struct	eh {			/* ethernet header			*/
	Eaddr eh_dst;		/* destination host address		*/
	Eaddr eh_src;		/* source host address			*/
	unsigned short eh_type;	/* Ethernet packet type (see below)	*/
};

struct	ep {			/* complete structure of Ethernet packet*/
	struct	eh ep_eh;	/* the ethernet header			*/
	char	ep_data[EP_DLEN];	/* data in the packet		*/
};

/* these allow us to pretend it's all one big happy structure */
#define ep_next		ep_ehx.ehx_next
#define	ep_dst		ep_eh.eh_dst
#define	ep_src		ep_eh.eh_src
#define	ep_type		ep_eh.eh_type
