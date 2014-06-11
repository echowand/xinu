#include <conf.h>
#include <kernel.h>
#include <ip.h>
#include <tcp.h>
#include <tcb.h>

int tcpxmit (struct tcb *ptcb)
{
	int len, offset, pipe, code, codelen, sent = 0;
	tcpseq seq;

	while (1) {
		len = tcpnextseg (ptcb, &offset);
		seq = ptcb->tcb_suna + offset;
		code = codelen = 0;

		/* Code bits */
		if (ptcb->tcb_state <= TCB_SYNRCVD
		    && SEQ_CMP (ptcb->tcb_snext, ptcb->tcb_ssyn) <= 0) {
			codelen++;
			code |= TCPF_SYN;
		}
		if (ptcb->tcb_flags & TCBF_WRDONE
		    && seq + len == ptcb->tcb_sfin
		    && SEQ_CMP (ptcb->tcb_snext, ptcb->tcb_sfin) <= 0) {
			codelen++;
			code |= TCPF_FIN;
			if (ptcb->tcb_state == TCB_ESTD
			    || ptcb->tcb_state == TCB_SYNRCVD)
				ptcb->tcb_state = TCB_FIN1;
			else if (ptcb->tcb_state == TCB_CWAIT)
				ptcb->tcb_state = TCB_LASTACK;
		}

		if (ptcb->tcb_flags & TCBF_SPUSHOK
		    && SEQ_CMP(seq, ptcb->tcb_spush) < 0
		    && SEQ_CMP(seq + len, ptcb->tcb_spush) >= 0) {
			len = min (len, ptcb->tcb_spush - seq);
			code |= TCPF_PSH;
		}

		if (ptcb->tcb_state <= TCB_SYNRCVD)
			pipe = 0;
		else
			pipe = ptcb->tcb_snext - ptcb->tcb_suna;

		/* If we've gotten to here with no data, check if we need to
		 * ACK and simply return otherwise ... this lets us call
		 * tcpxmit at will. */
		if (len + codelen == 0
		    || pipe + len + codelen >= ptcb->tcb_cwnd) {
			if (sent == 0)
				tcpack (ptcb, FALSE);
			return OK;
		}

		tcpsendseg (ptcb, offset, len, code);

		if (SEQ_CMP(ptcb->tcb_snext, seq + len + codelen) < 0)
			ptcb->tcb_snext = seq + len + codelen;

		sent = 1;
	}

	return OK;
}
