#include <conf.h>
#include <kernel.h>
#include <ip.h>
#include <tcp.h>
#include <tcb.h>

int tcptwait (struct tcb *ptcb, struct ip *pip, struct tcp *ptcp)
{
	return OK;
}
