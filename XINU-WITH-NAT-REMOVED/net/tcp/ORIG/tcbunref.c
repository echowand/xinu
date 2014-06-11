#include <conf.h>
#include <kernel.h>
#include <ip.h>
#include <tcp.h>
#include <tcb.h>

void tcbunref (struct tcb *ptcb)
{
	if (--ptcb->tcb_ref == 0) {
		freemem ((struct mblock *)ptcb->tcb_rbuf, ptcb->tcb_rbsize);
		freemem ((struct mblock *)ptcb->tcb_sbuf, ptcb->tcb_sbsize);
		ptcb->tcb_state = TCB_FREE;
	}
}
