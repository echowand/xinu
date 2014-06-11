#include <conf.h>
#include <kernel.h>
#include <ip.h>
#include <tcp.h>
#include <tcb.h>

int tcpwake (struct tcb *ptcb, int cond)
{
	int woken = 0;

	if (cond & TCPW_READERS) {
		woken += ptcb->tcb_readers;
		while (ptcb->tcb_readers--)
			signal (ptcb->tcb_rblock);
	}
	if (cond & TCPW_WRITERS) {
		woken += ptcb->tcb_writers;
		while (ptcb->tcb_writers--)
			signal (ptcb->tcb_wblock);
	}

	return woken;
}
