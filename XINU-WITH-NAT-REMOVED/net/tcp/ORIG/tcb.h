#define TCB_FREE	-1
#define TCB_CLOSED	0
#define TCB_LISTEN	1
#define TCB_SYNSENT	2
#define TCB_SYNRCVD	3
#define TCB_ESTD	4
#define TCB_FIN1	5
#define TCB_FIN2	6
#define TCB_CWAIT	7
#define TCB_CLOSING	8
#define TCB_LASTACK	9
#define TCB_TWAIT	10

#define TCBF_FINSEEN	0x1
#define TCBF_RDDONE	0x2
#define TCBF_WRDONE	0x4
#define TCBF_NEEDACK	0x8
#define TCBF_RPUSHOK	0x10
#define TCBF_SPUSHOK	0x20
#define TCBF_ACKPEND	0x40
#define TCBF_RTTPEND	0x80

#define TCBCMD(ptcb, cmd)	((((ptcb) - tcbtab) << 16) | (cmd))
#define TCBCMD_TCB(msg)		(&tcbtab[((msg) >> 16) & 0xffff])
#define TCBCMD_CMD(msg)		((msg) & 0xffff)

#define TCBC_SEND	1
#define TCBC_DELACK	2
#define TCBC_RTO	3
#define TCBC_EXPIRE	4

#define TCPW_READERS	0x1
#define TCPW_WRITERS	0x2

struct tcb {
	/* Device information */
	int		tcb_dvnum;	/* Xinu device number		*/

	/* Allocated resources and precious state */
	int		tcb_state;	/* Connection state in the FSM	*/
	int		tcb_mutex;	/* Metadata exclusion		*/
	int		tcb_rblock;	/* Reader block			*/
	int		tcb_wblock;	/* Writer block			*/
	int		tcb_lq;		/* Waiting connection queue	*/
	int		tcb_ref;	/* Reference count		*/

	/* Connection identifying information */
	IPaddr		tcb_lip;	/* Local IP address		*/
	IPaddr		tcb_rip;	/* Remote host IP address	*/
	u_short		tcb_lport;	/* Local port number		*/
	u_short		tcb_rport;	/* Remote port number		*/

	/* Miscellaneous information */
	int		tcb_flags;	/* Flags			*/
	int		tcb_qlen;	/* # of tcbs in the listen queue*/
	int		tcb_mss;	/* Maximum segment size		*/

	/* Receiver-side state */
	tcpseq		tcb_rnext;	/* RCV.NXT from RFC793		*/
	u_short		tcb_rwnd;	/* RCV.WND			*/
	tcpseq		tcb_rfin;	/* Receiver-side FIN seqno	*/
	tcpseq		tcb_rpush;	/* Received PSH seqno		*/
	int		tcb_rbsize;	/* Receive buffer size		*/
	int		tcb_rbdata;	/* First data byte in buffer	*/
	tcpseq		tcb_rbseq;	/* Receive buffer 1st seqno	*/
	int		tcb_rblen;	/* Length of data in buffer	*/
	char *		tcb_rbuf;	/* Receive buffer		*/
	int		tcb_readers;	/* # of waiting readers		*/

	/* Sender-side state */
	tcpseq		tcb_suna;	/* SND.UNA from RFC793		*/
	tcpseq		tcb_snext;	/* SND.NXT			*/
	tcpseq		tcb_ssyn;	/* Outgoing SYN seqno		*/
	tcpseq		tcb_sfin;	/* Outgoing FIN seqno		*/
	tcpseq		tcb_spush;	/* Send PSH seqno		*/
	tcpseq		tcb_rttseq;	/* Seqno of RTT measurement	*/
	int		tcb_rtttime;	/* Time of RTT measurement	*/
	int		tcb_cwnd;	/* JK88 cwnd			*/
	int		tcb_ssthresh;	/* JK88 ssthresh		*/
	int		tcb_dupacks;	/* Duplicate acks for suna	*/
	int		tcb_srtt;	/* JK88 sa			*/
	int		tcb_rttvar;	/* JK88 sv			*/
	int		tcb_rto;	/* RTO in ms			*/
	int		tcb_rtocount;	/* Successive RTOs		*/
	int		tcb_sbsize;
	int		tcb_sbdata;
	int		tcb_sblen;
	char *		tcb_sbuf;
	int		tcb_writers;
};

extern struct tcb tcbtab[Ntcp];

int tcplisten (struct tcb *ptcb, struct ip *pip, struct tcp *ptcp);
int tcpsynsent (struct tcb *ptcb, struct ip *pip, struct tcp *ptcp);
int tcpsynrcvd (struct tcb *ptcb, struct ip *pip, struct tcp *ptcp);
int tcpestd (struct tcb *ptcb, struct ip *pip, struct tcp *ptcp);
int tcpcwait (struct tcb *ptcb, struct ip *pip, struct tcp *ptcp);
int tcpfin1 (struct tcb *ptcb, struct ip *pip, struct tcp *ptcp);
int tcpfin2 (struct tcb *ptcb, struct ip *pip, struct tcp *ptcp);
int tcpclosing (struct tcb *ptcb, struct ip *pip, struct tcp *ptcp);
int tcplastack (struct tcb *ptcb, struct ip *pip, struct tcp *ptcp);
int tcptwait (struct tcb *ptcb, struct ip *pip, struct tcp *ptcp);

PROCESS tcpclassify ();
PROCESS tcpcmd ();

void tcbclear (struct tcb *ptcb);
void tcbref (struct tcb *ptcb);
void tcbunref (struct tcb *ptcb);
void tcpabort (struct tcb *ptcb);
int tcpwake (struct tcb *ptcb, int cond);

void tcptmset (int delay, struct tcb *ptcb, int message);
void tcptmdel (struct tcb *ptcb, int message);

int tcpnextseg (struct tcb *ptcb, int *offset);
int tcpupdate (struct tcb *ptcb, struct ip *pip, struct tcp *ptcp);
int tcprto (struct tcb *ptcb);
struct ep *tcpalloc (struct tcb *ptcb, int len);
u_short tcpcksum (struct ip *pip, struct tcp *ptcp);
void tcpnet2h (struct tcp *ptcp);
void tcph2net (struct tcp *ptcp);
void tcpinput (struct tcb *ptcb, struct ip *pip, struct tcp *ptcp);
int tcpreset (struct ip *pip, struct tcp *ptcp);
void tcpack (struct tcb *ptcb, int force);
int tcpxmit (struct tcb *ptcb);
void tcpsendseg (struct tcb *ptcb, int offset, int len, int code);
int tcpdata (struct tcb *ptcb, struct ip *pip, struct tcp *ptcp);
