/* tcp.h - definitions used by TCP, including header length in-lines	*/

typedef	int32	tcpseq;			/* TCP sequence number type	*/

/************************************************************************/
/* SEQCMP - an in-line function to perform sequence space comparison	*/
/*									*/
/* Comparison must handle sequence-space wraparound.  Overflow and	*/
/* underflow make it possible to perform the comparison using standard	*/
/* subtraction: for any a, b in the sequence space, a result of < 0	*/
/* means that a < b, a result of > 0 means that a > b, and a result of	*/
/* 0 means that a == b.							*/
/************************************************************************/

#define SEQ_CMP(x, y)	((x) - (y))

/* In-lines for minimum or maximum of two values */

#define	max(a,b)	( (a) > (b) ? (a) : (b) )
#define	min(a,b)	( (a) < (b) ? (a) : (b) )

/* Well-known TCP port numbers */

#define TCPP_ECHO	7
#define TCPP_DISCARD	9

#define	TCP_HDR_LEN	20		/* minimum TCP header size	*/

/* In-line functions to extract header lengths */

#define TCP_HLEN(ptcp)	(((ptcp)->tcp_code >> 10) & 0x3c)
#define TCP_NETHLEN(ptcp) ((ntohs((ptcp)->tcp_code) >> 10) & 0x3c)
#define TCP_MINHLEN	20
#define TCP_DATALEN(pip, ptcp) (pip->ip_len-IP_HDR_LEN-TCP_HLEN(ptcp))

/* Values for TCP code field (flags) */

#define TCPF_FIN	0x01
#define TCPF_SYN	0x02
#define TCPF_RST	0x04
#define TCPF_PSH	0x08
#define TCPF_ACK	0x10
#define TCPF_URG	0x20

/* Default maximum segment size */

#define TCP_ETHMSS	1460

#define TCP_ACKDELAY	200		/* ACK delay in millissconds	*/

#define TCP_MAXRTO	5		/* Maximum round-trip timeout	*/

#define TCP_MSL		120000		/* Maximum Segment Lifetime 	*/

/* Constants for the TCP state entry */

#define TCP_SLOTS       2               /* num. of open TCP endpoints   */
#define TCP_QSIZ        20               /* packets enqueued per endpoint*/

/* Constants for the state of an entry */

#define TCP_FREE        0               /* entry is unused              */
#define TCP_USED        1               /* entry is being used          */
#define TCP_RECV        2               /* entry has a process waiting  */

#define TCP_ANYIF       -2              /* Register an endpoint for any */
                                        /*   interface on the machine   */

/* TCP header format */

struct tcp {
	uint16		tcp_sport;	/* Source port number		*/
	uint16		tcp_dport;	/* Destination port number	*/
	tcpseq		tcp_seq;	/* Sequence number		*/
	tcpseq		tcp_ack;	/* Acknowledgment		*/
	uint16		tcp_code;	/* Segment type (TCPF_* above)	*/
	uint16		tcp_window;	/* Advertised window size	*/
	uint16		tcp_cksum;	/* Segment checksum		*/
	uint16		tcp_urgptr;	/* Urgent pointer		*/
	byte		tcp_data[1];	/* Start of data area		*/
};

/* Miscellaneous values used by TCP across all connections */

struct tcpcontrol {
	int32		tcpmutex;	/* TCP state synch mutex	*/
	int32		tcpdev;		/* IP device for TCP		*/
	uint16		tcpnextport;	/* Next port to try on open()	*/
	int32		tcpcmdq;	/* TCP Command & Control queue	*/
};

extern struct tcpcontrol Tcp;



