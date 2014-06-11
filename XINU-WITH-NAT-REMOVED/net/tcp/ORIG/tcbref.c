#include <conf.h>
#include <kernel.h>
#include <ip.h>
#include <tcp.h>
#include <tcb.h>

void tcbref (struct tcb *ptcb)
{
	ptcb->tcb_ref++;
}
