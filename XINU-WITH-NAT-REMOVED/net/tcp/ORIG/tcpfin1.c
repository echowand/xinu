#include <conf.h>
#include <kernel.h>
#include <ip.h>
#include <tcp.h>
#include <tcb.h>

int tcpfin1 (struct tcb *ptcb, struct ip *pip, struct tcp *ptcp)
{
	int rfin, ack;

	rfin = ack = 0;

	tcpdata (ptcb, pip, ptcp);

	if (ptcb->tcb_flags & TCBF_FINSEEN &&
	    SEQ_CMP(ptcb->tcb_rnext, ptcb->tcb_rfin) > 0)
		rfin = 1;

	if (SEQ_CMP(ptcb->tcb_suna, ptcb->tcb_sfin) > 0)
		ack = 1;

	if (rfin && ack) {
		tcptmset (TCP_MSL << 1, ptcb, TCBC_EXPIRE);
		ptcb->tcb_state = TCB_TWAIT;
	} else if (rfin)
		ptcb->tcb_state = TCB_CLOSING;
	else if (ack)
		ptcb->tcb_state = TCB_FIN2;

	tcpxmit (ptcb);

	return OK;
}
