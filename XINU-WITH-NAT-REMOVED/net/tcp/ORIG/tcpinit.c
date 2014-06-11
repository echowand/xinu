#include <conf.h>
#include <kernel.h>
#include <mq.h>
#include <ip.h>
#include <tcp.h>
#include <tcb.h>

struct tcb tcbtab[Ntcp];

int tcpinit (struct devsw *pdev)
{
	struct tcb *ptcb = &tcbtab[pdev->dvminor];

	ptcb->tcb_dvnum = pdev->dvnum;

	ptcb->tcb_mutex = screate (1);
	ptcb->tcb_rblock = screate (0);
	ptcb->tcb_wblock = screate (0);
	ptcb->tcb_lq = mqcreate (5);

	ptcb->tcb_ref = 0;

	ptcb->tcb_state = TCB_FREE;

	return OK;
}
