#include <conf.h>
#include <kernel.h>
#include <ip.h>
#include <tcp.h>
#include <tcb.h>

int tcpfin2 (struct tcb *ptcb, struct ip *pip, struct tcp *ptcp)
{
	tcpdata (ptcb, pip, ptcp);

	if (ptcb->tcb_flags & TCBF_FINSEEN
	    && SEQ_CMP(ptcb->tcb_rnext, ptcb->tcb_rfin) > 0) {
		tcptmset (TCP_MSL << 1, ptcb, TCBC_EXPIRE);
		ptcb->tcb_state = TCB_TWAIT;
	}

	tcpxmit (ptcb);

	return OK;
}
