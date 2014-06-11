#include <conf.h>
#include <kernel.h>
#include <mq.h>
#include <ip.h>
#include <tcp.h>
#include <tcb.h>
#include <stdio.h>

struct tcpcontrol Tcp;

int tcpminit (struct devsw *pdev)
{
	int tcpdev;

	if ((tcpdev = open (IP, IPP_TCP, sizeof(int))) == SYSERR)
		return SYSERR;
	Tcp.tcpdev = tcpdev;
	Tcp.tcpmutex = screate (1);
	Tcp.tcpnextport = 1024;
	Tcp.tcpcmdq = mqcreate (10);

	resume (create ((int *)tcpclassify, 512, 80, "tcpclassify", 0, NULL));
	resume (create ((int *)tcpcmd, 512, 80, "tcpcmd", 0, NULL));

	return OK;
}
