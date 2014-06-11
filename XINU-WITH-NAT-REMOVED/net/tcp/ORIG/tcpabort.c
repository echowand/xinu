#include <conf.h>
#include <kernel.h>
#include <ip.h>
#include <tcp.h>
#include <tcb.h>

void tcpabort (struct tcb *ptcb)
{
	ptcb->tcb_state = TCB_CLOSED;
	tcpwake (ptcb, TCPW_READERS|TCPW_WRITERS);
	if (ptcb->tcb_state > TCB_SYNRCVD)
		tcbunref (ptcb);
}
