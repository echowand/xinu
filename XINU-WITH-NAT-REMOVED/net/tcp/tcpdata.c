/* tcpdata.c  -  tcpdata */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  tcpdata  -  Handle an incoming segment that contains data
 *------------------------------------------------------------------------
 */
int32	tcpdata(
	  struct tcb	*ptcb,		/* Ptr to a TCB			*/
	  struct ip	*pip,		/* Ptr to IP (not used)		*/
	  struct tcp	*ptcp		/* Ptr to TCP (not used)	*/
	)
{
	int32	datalen;		/* Data length			*/
	int32	codelen;		/* Count 1 sequence for FIN	*/
	int32	offset;			/* Offset in segment of new	*/
					/*   data (i.e., data not	*/
					/*   received earlier		*/
	int32	i, j;			/* counter and index used	*/
					/*   during data copy		*/
	tcpseq	endseq;			/* Ending sequence number after	*/
					/*   new data that arrived	*/
	char	*data;			/* Ptr used during data copy	*/

	/* Compute the segment data size */

	datalen = TCP_DATALEN(pip, ptcp);
	codelen = offset = 0;

	/* See if FIN has arived */

	if (ptcp->tcp_code & TCPF_FIN) {
		codelen++;
		ptcb->tcb_flags |= TCBF_FINSEEN;
		ptcb->tcb_rfin = ptcp->tcp_seq + datalen;
	}

	/* If no data, we're finished */

	if (datalen == 0 && codelen == 0)
		return 0;

	/* If the reader cannot read this, RST it */

	if (ptcb->tcb_flags & TCBF_RDDONE && datalen) {
		tcpreset (pip, ptcp);
		return SYSERR;
	}

	/* If this segment is too old, drop it */

	if (SEQ_CMP (ptcp->tcp_seq + datalen + codelen,
		     ptcb->tcb_rbseq) <= 0) {
		ptcb->tcb_flags |= TCBF_NEEDACK;
		return SYSERR;
	}

	/* If the sender exceeded our window, shame on them */

	if (SEQ_CMP (ptcp->tcp_seq + datalen + codelen,
		     ptcb->tcb_rbseq + ptcb->tcb_rbsize) > 0) {
		ptcb->tcb_flags |= TCBF_NEEDACK;
		return SYSERR;
	}

	/* Discard data already consumed by the reader */

	if (SEQ_CMP (ptcp->tcp_seq, ptcb->tcb_rbseq) < 0) {
		offset = ptcb->tcb_rbseq - ptcp->tcp_seq;
		datalen -= offset;
	}

	/* Move to start of new data in segment */

	data = (char *)ptcp + TCP_HLEN(ptcp) + offset;

	/* Copy data from segment to TCB */

	i = 0;
	j = ptcb->tcb_rbdata + ptcp->tcp_seq - ptcb->tcb_rbseq + offset;
	while (i < datalen) {
		if (j >= ptcb->tcb_rbsize)
			j %= ptcb->tcb_rbsize;
		ptcb->tcb_rbuf[j++] = data[i++];
	}

	/* compute the ending sequence number after the new data */

	endseq = ptcp->tcp_seq + offset + datalen;

	/* See if segment arrived in order */

	if (SEQ_CMP (ptcp->tcp_seq + offset,
		     ptcb->tcb_rbseq + ptcb->tcb_rblen) <= 0) {
		/* Yes, the data is in order */
		if (endseq - ptcb->tcb_rbseq >= ptcb->tcb_rblen) {
			ptcb->tcb_rblen = endseq - ptcb->tcb_rbseq;

			ptcb->tcb_rnext = endseq + codelen;
		}
	} else {
		/* We should deal with out-of-order segments */
	}

	/* See if new data should be pushed to applications */

	if (ptcp->tcp_code & TCPF_PSH) {
		ptcb->tcb_flags |= TCBF_RPUSHOK;
		ptcb->tcb_rpush = endseq;
	}

	/* If data or a FIN arrived, an ACK is needed */

	if (datalen || codelen) {
		ptcb->tcb_flags |= TCBF_NEEDACK;
		if (ptcb->tcb_readers) {
			ptcb->tcb_readers--;
			signal (ptcb->tcb_rblock);
		}
	}

	/* Return size of data available */

	return datalen + codelen;
}
