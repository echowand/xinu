/* tcpsendseg.c  -  tcpsendseg */

#include <xinu.h>

extern	uint32	ctr1000;

/*------------------------------------------------------------------------
 *  tcpsendseg  -  Send a TCP segment for a specified TCB
 *------------------------------------------------------------------------
 */
void	tcpsendseg(
	  struct tcb	*ptcb,		/* Ptr to a TCB			*/
	  int32		offset,		/* Offset for data to send	*/
	  int32		len,		/* Length of TCP data		*/
	  int32		code		/* Code value to use		*/
	)
{
	struct ep	*pep;		/* ptr to a packet		*/
	struct ip 	*pip;		/* ptr to IP datagram in packet	*/
	struct tcp	*ptcp;		/* ptr to TCP segment in packet	*/
	int		i;		/* counts data bytes during copy*/
	char		*data;		/* ptr to data being copied	*/

	struct netpacket *pkt;		/* ptr for new formant		*/

	/* Allocate a network buffer */

	pep = tcpalloc (ptcb, len);

	/* Set up for use the Vol2 structures */

	pip = (struct ip *)&pep->ep_data;
	ptcp = (struct tcp *)((char *)pip + IP_HDR_LEN);

	if ((int32)pep == SYSERR) {
		return;
	}
	pkt = (struct netpacket *) pep;

	ptcp->tcp_seq = ptcb->tcb_suna + offset;
	ptcp->tcp_code |= code;

	data = ((char *)ptcp + TCP_HLEN(ptcp));
	for (i = 0; i < len; i++) {
		data[i] = ptcb->tcb_sbuf[(ptcb->tcb_sbdata + offset + i)
			% ptcb->tcb_sbsize];
	}

	if (ptcb->tcb_suna == ptcb->tcb_snext) {/* No outstanding data */
		tcptmset (ptcb->tcb_rto, ptcb, TCBC_RTO);
	}

	if (ptcb->tcb_flags & TCBF_ACKPEND) {
		tcptmdel (ptcb, TCBC_DELACK);
	}
	ptcb->tcb_flags &= ~(TCBF_NEEDACK | TCBF_ACKPEND);

	if (!(ptcb->tcb_flags & TCBF_RTTPEND) && len) {
		ptcb->tcb_flags |= TCBF_RTTPEND;
		ptcb->tcb_rttseq = ptcp->tcp_seq;
		ptcb->tcb_rtttime = (int)ctr1000;
	}
	ip_send (pkt);
	return;
}
