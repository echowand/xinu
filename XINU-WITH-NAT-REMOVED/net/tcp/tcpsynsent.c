/* tcpsynsent.c  -  tcpsynsent */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  tcpsynsent  -  Handle an incoming segment in the SYN-SENT state
 *------------------------------------------------------------------------
 */
int32	tcpsynsent(
	  struct tcb	*ptcb,		/* Ptr to a TCB			*/
	  struct ip	*pip,		/* Ptr to IP			*/
	  struct tcp	*ptcp		/* Ptr to TCP			*/
	)
{
	/* Must receive a SYN or reset connection */

	if (!(ptcp->tcp_code & TCPF_SYN)) {
		tcpreset (pip, ptcp);
		return SYSERR;
	}
        
	/*
	/* Move to SYN-RECEIVED state */

	//ptcb->tcb_state = TCB_SYNRCVD;

	/* Set up parameters, such as the window size */
        
	ptcb->tcb_rnext = ptcb->tcb_rbseq = ++ptcp->tcp_seq;
	ptcb->tcb_rwnd = ptcb->tcb_ssthresh = ptcp->tcp_window;
	ptcp->tcp_code &= ~TCPF_SYN;
         
	/** Send an ACK */
        

	tcbref(ptcb);
	
	ptcb->tcb_state = TCB_ESTD;
	
	/* Move past SYN */

        ptcb->tcb_suna++;
	
	tcpdata(ptcb, pip, ptcp);
	tcpack(ptcb, TRUE);

        /*if (tcpdata (ptcb, pip, ptcp)) {
                tcpack (ptcb, TRUE);
        }
	*/

        /* Permit reading */

        if (ptcb->tcb_readers) {
                ptcb->tcb_readers--;
                signal (ptcb->tcb_rblock);
        }

/*DEBUG*///kprintf("Should be established here\n");

	return OK;
}
