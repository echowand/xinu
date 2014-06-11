/* tcpdisp.c  -  tcpnull, tcpdisp */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  tcpnull -  Do nothing (entry in tcpstatesw when no action required)
 *------------------------------------------------------------------------
 */
int32	tcpnull(
	  struct tcb	*ptcb,		/* Ptr to a TCB			*/
	  struct ip	*pip,		/* Ptr to IP (not used)		*/
	  struct tcp	*ptcp		/* Ptr to TCP (not used)	*/
	)
{
	return OK;
}


int32 (*tcpstatesw[]) (struct tcb *ptcb,struct ip *pip,struct tcp *ptcp)
  = {
	tcpnull,			/* CLOSED			*/
	tcplisten,			/* LISTEN			*/
	tcpsynsent,			/* SYN SENT			*/
	tcpsynrcvd,			/* SYN RCVD			*/
	tcpestd,			/* ESTABLISHED 			*/
	tcpfin1,			/* FIN WAIT-1			*/
	tcpfin2,			/* FIN WAIT-2			*/
	tcpcwait,			/* CLOSE WAIT			*/
	tcpclosing,			/* CLOSING			*/
	tcplastack,			/* LAST ACK			*/
	tcptwait,			/* TIME WAIT			*/
};


/*------------------------------------------------------------------------
 *  tcpdisp  -  Use the current state of the TCB to dispatch segment
 *			processing to one of the input functions
 *------------------------------------------------------------------------
 */
void	tcpdisp(
	  struct tcb	*ptcb,		/* Ptr to a TCB			*/
	  struct ip	*pip,		/* Ptr to IP (not used)		*/
	  struct tcp	*ptcp		/* Ptr to TCP (not used)	*/
	)
{
	int32		state;		/* state of the TCB		*/
/*DEBUG*/// char *dnames[] = {"tcpnull","tcplisten","tcpsynsent","tcpsynrcvd",
/*DEBUG*///	"tcpestd","tcpfin1","tcpfin2","tcpcwait","tcpclosing",
/*DEBUG*///	"tcplastack","tcptwait"};

	/* Obtain the state from the TCB */

	state = ptcb->tcb_state;


	/* Handle a reset */

	if (ptcp->tcp_code & TCPF_RST) {
		if (state == TCB_LISTEN)
			return;
		else if (state == TCB_SYNSENT
			 && ptcp->tcp_ack == ptcb->tcb_snext)
			tcpabort (ptcb);
		else if (ptcp->tcp_seq >= ptcb->tcb_rnext
		    && ptcp->tcp_seq <= ptcb->tcb_rnext + ptcb->tcb_rwnd)
			tcpabort (ptcb);
		return;
	}

	/* Handle an incoming ACK */

	if ( ptcp->tcp_ack < ptcb->tcb_suna
	    || ptcp->tcp_ack > ptcb->tcb_snext) {
		if (state <= TCB_SYNRCVD) {
			tcpreset (pip, ptcp);
		} else {
			tcpack (ptcb, TRUE);
		}
		return; 
	}

	/* Handle SYN */

	if (ptcp->tcp_code & TCPF_SYN && state > TCB_SYNRCVD) {

		/* Is this the _right_ SYN? */

		ptcp->tcp_code &= ~TCPF_SYN;
		ptcp->tcp_seq++;
	}

	/* Handle SYN-ACK */
	/*if (ptcp->tcp_code & (TCPF_SYN | TCPF_ACK) && state==TCB_SYNSENT){

		tcpsynsent(ptcb, pip, ptcp);
		kprintf("synack here!!");
		return;
	}*/

	tcpupdate (ptcb, pip, ptcp);

	/* Dispatch processing according to the state of the TCB */
	/*DEBUG*/// kprintf("tcpdisp: dispatching to %s\n",dnames[state]);
	(tcpstatesw[state]) (ptcb, pip, ptcp);

	return;
}
