#include <conf.h>
#include <kernel.h>
#include <ip.h>
#include <tcp.h>
#include <tcb.h>

int tcpnextseg (struct tcb *ptcb, int *offset)
{
	*offset = ptcb->tcb_snext - ptcb->tcb_suna;

	if (ptcb->tcb_sblen > *offset)
		return (min (ptcb->tcb_mss, ptcb->tcb_sblen - *offset));

	return 0;
}
