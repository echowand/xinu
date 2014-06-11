#include <conf.h>
#include <kernel.h>
#include <timer.h>
#include <ip.h>
#include <tcp.h>
#include <tcb.h>

void tcptmset (int delay, struct tcb *ptcb, int message)
{
	tcbref (ptcb);
	tmset (delay, Tcp.tcpcmdq, TCBCMD(ptcb, message));
}

void tcptmdel (struct tcb *ptcb, int message)
{
	if (tmdel (Tcp.tcpcmdq, TCBCMD(ptcb, message)) == OK)
		tcbunref (ptcb);
}
