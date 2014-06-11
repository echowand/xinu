typedef	unsigned long IPaddr;			/*  internet address	*/

#define IP_ALEN		sizeof(IPaddr)		/* IP address len	*/
#define IP_HLEN(pip)	((pip)->ip_hlen << 2)	/* IP header len	*/
#define IP_MINHLEN	20			/* IP min header len	*/

#define IP_DEFTTL	255			/* IP default TTL	*/

#define IPADDR_BCAST	0xffffffff		/* IP broadcast address	*/
#define IPADDR_ANY	0x0			/* IP address any	*/

/* IP Protocols */
#define IPP_ICMP	1			/* ICMP protocol number	*/
#define IPP_TCP		6			/* TCP protocol number	*/
#define IPP_UDP		17			/* UDP protocol number	*/

#define IPF_DF		0x4000			/* don't fragment mask	*/
#define IPF_MF		0x2000			/* more fragments mask	*/
#define IP_OFFMASK	0x1fff			/* fragment offset mask	*/

/* macro to get IP fragment offset */
#define IP_OFF(pip)	((net2hs((pip)->ip_offset) & IP_OFFMASK) << 3)

#define IPC_MJOIN       0			/* IP multicast join	*/
#define IPC_MLEAVE      1			/* IP multicast leave	*/

/* macro to check if an IP address is a multicast address */
#define IP_ISMULTI(x)	((x & 0x000000f0) == 0x000000e0)

#define IP_SMLBUFSZ	MAXNETBUF		/* small IP buffer size	*/
#define IP_NSMLBUF	NETBUFS			/* # of small buffers   */
#define IP_MEDBUFSZ	(8192 + sizeof (struct eh) + sizeof (struct ehx))
						/* med IP buffer size	*/
#define IP_NMEDBUF	8			/* # of medium buffers	*/
#define IP_LRGBUFSZ	(65536 + sizeof (struct eh) + sizeof (struct ehx))
						/* large IP buffer size	*/
#define IP_NLRGBUF	4			/* # of large buffers	*/

#define IPFC_N		10			/* max # of reasmbl pkts*/
#define IPFC_FRAGMAX	100			/* max # of frags	*/
#define IPFC_TIMEOUT	60000			/* reasselmbly to 60sec */

#define IP_FQSIZE	10			/* fast IP queue len	*/
#define IP_SQSIZE	10			/* slow IP queue len	*/

struct ip {					/* IP header 		*/
#if BYTE_ORDER == BIG_ENDIAN
	unsigned int	ip_ver:4;		/* IP version		*/
	unsigned int	ip_hlen:4;		/* IP header len	*/
#else
	unsigned int	ip_hlen:4;		/* IP header len	*/
	unsigned int	ip_ver:4;		/* IP version		*/
#endif
	u_char		ip_tos;			/* IP type-of-service	*/
	u_short		ip_len;			/* IP Total length	*/
	u_short		ip_id;			/* IP identification	*/
	u_short		ip_offset;		/* IP flags & frag offst*/
	u_char		ip_ttl;			/* IP time to live	*/
	u_char		ip_proto;		/* IP protocol number	*/
	u_short		ip_cksum;		/* IP header checksum	*/
	IPaddr		ip_src;			/* IP source address	*/
	IPaddr		ip_dst;			/* IP destination addr	*/
	u_char		ip_data[1];		/* start of IP data	*/
};

struct ipdev {					/* IP device table	*/
	int ipd_proto;				/* IP protocol number	*/
	int ipd_mq;				/* IP protocol queue	*/
	int ipd_dvnum;				/* protocol device #	*/
	u_char ipd_tos;				/* type of service	*/
	u_char ipd_ttl;				/* time to live		*/
};

struct ipcontrol {				/* IP control struct	*/
	int fastipmq;				/* fastip message queue */
	int slowipmq;				/* slowip message queue	*/
	int forward;				/* enable forwarding 	*/
	u_short fragid;				/* nxt fragment id	*/
	int devmutex;				/* IP device semaphore	*/
	int mbpool;				/* IP med buf pool	*/
	int lbpool;				/* IP large buf pool	*/
	int nfrags;				/* # of frag in stack	*/
};


struct ipfc {					/* IP frag ctrl struct	*/
	struct ep *head;			/* head of frag list	*/
	struct ep *hole;			/* frg before hole 	*/
	int firstts;				/* ts of first frag	*/
	int rcntts;	   			/* ts of most recent frg*/
};


extern struct ipdev ipdevtab[];			/* IP device table decl	*/
extern struct ipcontrol Ip;			/* IP control decl	*/
extern struct ipfc ipfctab[];			/* IP frg ctl table decl*/

char *iptochar (IPaddr, char*);			/* IP nr to str convert */
IPaddr chartoip (char*);			/* IP str to nr convert */

/* frontend macro to blocking _ipalloc */
#define ipalloc(ipdev, len, src, dst) _ipalloc (ipdev, len, src, dst, FALSE)

/* frontend macro to non-blocking _ipalloc */
#define nbipalloc(ipdev, len, src, dst) _ipalloc (ipdev, len, src, dst, TRUE)

struct ep *_ipalloc (int ipdev, int len, IPaddr src, IPaddr dst, int block);
void iprecv (struct ep *pep);
inline int ipttl (struct ip *pip);
int ipsend (struct ep *pep);
int iplookup(struct ep *pep);
int iproute (struct ep *pep, struct ip *pip);
void ipdeliver (struct ep *pep, struct ip *pip);
inline struct ep * _ipgetbuf (int buflen, int block);
void ipfrag(struct ep *pup, struct ip *pip);
void ipreassemble (struct ep *pep, struct ip *pip);
int ipprintfc (int stdout);
void ipfctimeout ();

/* reassembly prototypes */

void ipreassemble(struct ep *pep, struct ip *pip);
int ipfcadd(struct ipfc *pfc, struct ep *pep, struct ip *pip);
void ipfccoallesce(struct ep *pep1, struct ip *pip1,
                   struct ep *pep2, struct ip *pip2);
void ipfcassemble(struct ipfc *pfc);
void ipfctimeout();
void ipfcalloc(struct ipfc *pfc, struct ep *pep);
void ipfcclearlru();
void ipfcclear(struct ipfc *pfc);
int ipeq(struct ip *pip1, struct ip *pip2);
int ipprintfc(int stdout);

PROCESS fastip ();
PROCESS slowip ();
