#include <conf.h>
#include <kernel.h>
#include <netdefs.h>
#include <ip.h>
#include <tcp.h>
#include <tcb.h>

int tcpdata (struct tcb *ptcb, struct ip *pip, struct tcp *ptcp)
{
	int datalen, codelen, offset, i, j;
	tcpseq endseq;
	char *data;

	datalen = TCP_DATALEN(pip, ptcp);
	codelen = offset = 0;

	if (ptcp->tcp_code & TCPF_FIN) {
		codelen++;
		ptcb->tcb_flags |= TCBF_FINSEEN;
		ptcb->tcb_rfin = ptcp->tcp_seq + datalen;
	}

	if (datalen == 0 && codelen == 0)
		return 0;

	/* If the reader can't read this, RST it */
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

	/* If the sender busted our window, shame on them */
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

	data = (char *)ptcp + TCP_HLEN(ptcp) + offset;
	i = 0;
	j = ptcb->tcb_rbdata + ptcp->tcp_seq - ptcb->tcb_rbseq + offset;
	while (i < datalen) {
		if (j >= ptcb->tcb_rbsize)
			j %= ptcb->tcb_rbsize;
		ptcb->tcb_rbuf[j++] = data[i++];
	}

	endseq = ptcp->tcp_seq + offset + datalen;
	if (SEQ_CMP (ptcp->tcp_seq + offset,
		     ptcb->tcb_rbseq + ptcb->tcb_rblen) <= 0) {
		/* in order */
		if (endseq - ptcb->tcb_rbseq >= ptcb->tcb_rblen) {
			ptcb->tcb_rblen = endseq - ptcb->tcb_rbseq;
			ptcb->tcb_rnext = endseq + codelen;
		}
	} else {
		/* XXX: Deal with out-of-order segments */
	}

	if (ptcp->tcp_code & TCPF_PSH) {
		ptcb->tcb_flags |= TCBF_RPUSHOK;
		ptcb->tcb_rpush = endseq;
	}

	if (datalen || codelen) {
		ptcb->tcb_flags |= TCBF_NEEDACK;
		if (ptcb->tcb_readers) {
			ptcb->tcb_readers--;
			signal (ptcb->tcb_rblock);
		}
	}

	return datalen + codelen;
}
