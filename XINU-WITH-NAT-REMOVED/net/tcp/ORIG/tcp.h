typedef long	tcpseq;

#define SEQ_CMP(x, y)	((x) - (y))

#define TCPP_ECHO	7
#define TCPP_DISCARD	9

#define TCP_HLEN(ptcp)	(((ptcp)->tcp_code >> 10) & 0x3c)
#define TCP_NETHLEN(ptcp) ((net2hs((ptcp)->tcp_code) >> 10) & 0x3c)
#define TCP_MINHLEN	20
#define TCP_DATALEN(pip, ptcp) (net2hs((pip)->ip_len) - IP_HLEN(pip) - TCP_HLEN(ptcp))
#define TCPF_FIN	0x01
#define TCPF_SYN	0x02
#define TCPF_RST	0x04
#define TCPF_PSH	0x08
#define TCPF_ACK	0x10
#define TCPF_URG	0x20

#define TCP_ETHMSS	1460

#define TCP_ACKDELAY	200
#define TCP_MAXRTO	5

#define TCP_MSL		120000

/*
 * SEQCMP - sequence space comparator
 * This handles sequence-space wraparound.  Overflow/underflow make
 * the result below correct (-, 0, +) for any a, b in the sequence
 * space.  A result of < 0 implies that a < b, > 0 that a > b, and 0
 * that a == b.
 */
struct tcp {
	u_short		tcp_sport;
	u_short		tcp_dport;
	tcpseq		tcp_seq;
	tcpseq		tcp_ack;
	u_short		tcp_code;
	u_short		tcp_window;
	u_short		tcp_cksum;
	u_short		tcp_urgptr;
	u_char		tcp_data[1];
};

struct tcpcontrol {
	int		tcpmutex;	/* TCP state synch mutex	*/
	int		tcpdev;		/* IP device for TCP		*/
	u_short		tcpnextport;	/* Next port to try on open()	*/
	int		tcpcmdq;	/* TCP Command & Control queue	*/
};

extern struct tcpcontrol Tcp;
