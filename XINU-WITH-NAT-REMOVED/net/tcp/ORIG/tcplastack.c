#include <conf.h>
#include <kernel.h>
#include <ip.h>
#include <tcp.h>
#include <tcb.h>

int tcplastack (struct tcb *ptcb, struct ip *pip, struct tcp *ptcp)
{
	if (SEQ_CMP(ptcb->tcb_suna, ptcb->tcb_sfin) > 0) {
		/* There _is_ an RTO pending */
		tcptmdel (ptcb, TCBC_RTO);
		ptcb->tcb_state = TCB_CLOSED;
		tcbunref (ptcb);
	}

	return OK;
}
