#include <conf.h>
#include <kernel.h>
#include <ip.h>
#include <tcp.h>
#include <tcb.h>


int tcpclosing (struct tcb *ptcb, struct ip *pip, struct tcp *ptcp)
{
	if (SEQ_CMP (ptcb->tcb_suna, ptcb->tcb_sfin) > 0) {
		tcptmset (TCP_MSL << 1, ptcb, TCBC_EXPIRE);
		ptcb->tcb_state = TCB_TWAIT;
	}
	return OK;
}
