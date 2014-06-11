#include <conf.h>
#include <kernel.h>
#include <ip.h>
#include <tcp.h>
#include <tcb.h>

int tcpsynrcvd (struct tcb *ptcb, struct ip *pip, struct tcp *ptcp)
{
	if (!(ptcp->tcp_code & TCPF_ACK)
	    || ptcp->tcp_seq != ptcb->tcb_rnext)
		return SYSERR;

	tcbref (ptcb);			/* For the system, since now
					 * we're synchronized */
	ptcb->tcb_state = TCB_ESTD;

	ptcb->tcb_suna++;
	if (tcpdata (ptcb, pip, ptcp))
		tcpxmit (ptcb);

	if (ptcb->tcb_readers) {
		ptcb->tcb_readers--;
		signal (ptcb->tcb_rblock);
	}

	return OK;
}
