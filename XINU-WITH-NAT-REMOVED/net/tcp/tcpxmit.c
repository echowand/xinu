/* tcpxmit.c  -  tcpxmit */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  tcpxmit  -  Send a segment if needed
 *------------------------------------------------------------------------
 */
int32	tcpxmit(
	  struct tcb	*ptcb		/* Ptr to a TCB			*/
	)
{
	int32		len;		/* length of segment to send	*/
	int32		offset;		/* Offset of first data byte	*/
	int32		pipe;		/*				*/
	int32		code;		/* Code bits			*/
	int32		codelen;	/* Number of code bits set	*/
	int32		sent;		/* Has anything been sent?	*/
	tcpseq		seq;		/* sequence number		*/

	sent = 0;

	/* Send as many segments as we can */

	while (1) {

		/* Calculate length and sequence for next segment */

		len = tcpnextseg (ptcb, &offset);
		seq = ptcb->tcb_suna + offset;
		code = codelen = 0;

		/* The Following handles each code bit */

		/* SYN */

		if (ptcb->tcb_state <= TCB_SYNRCVD
		    && SEQ_CMP (ptcb->tcb_snext, ptcb->tcb_ssyn) <= 0) {
			codelen++;
			code |= TCPF_SYN;
		}

		/* FIN */

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

		/* PUSH */

		if (ptcb->tcb_flags & TCBF_SPUSHOK
		    && SEQ_CMP(seq, ptcb->tcb_spush) < 0
		    && SEQ_CMP(seq + len, ptcb->tcb_spush) >= 0) {
			len = min (len, ptcb->tcb_spush - seq);
			code |= TCPF_PSH;
		}

		if (ptcb->tcb_state <= TCB_SYNRCVD) {
			pipe = 0;
		} else {
			pipe = ptcb->tcb_snext - ptcb->tcb_suna;
		}

		/* If we reach this point with no data, check to see	*/
		/* whether an ACK is needed.  If not, simply return	*/
		/* without taking action.  Avoiding action in case of	*/
		/* no data and no ACK allows tcpxmit to be called at	*/
		/* will without requiring a caller to check conditions.	*/

		if ( ( (len + codelen) == 0 )
		    || ( (pipe + len + codelen) >= ptcb->tcb_cwnd ) ) {
			if (sent == 0) {
				tcpack (ptcb, FALSE);
			}
			return OK;
		}

		/* Send a segment */

		tcpsendseg (ptcb, offset, len, code);

		if (SEQ_CMP(ptcb->tcb_snext, seq + len + codelen) < 0) {
			ptcb->tcb_snext = seq + len + codelen;

		}
		sent = 1;
	}

	return OK;
}
