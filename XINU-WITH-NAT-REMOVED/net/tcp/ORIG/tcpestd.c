#include <conf.h>
#include <kernel.h>
#include <ip.h>
#include <tcp.h>
#include <tcb.h>

int tcpestd (struct tcb *ptcb, struct ip *pip, struct tcp *ptcp)
{
	tcpdata (ptcb, pip, ptcp);

	if (ptcb->tcb_flags & TCBF_FINSEEN
	    && SEQ_CMP(ptcb->tcb_rnext, ptcb->tcb_rfin) > 0)
		ptcb->tcb_state = TCB_CWAIT;

	tcpxmit (ptcb);

	return OK;
}
