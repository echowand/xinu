#include <conf.h>
#include <kernel.h>
#include <netdefs.h>
#include <ip.h>
#include <tcp.h>
#include <tcb.h>
#include <stdio.h>

PROCESS x_tcpstat (int stdin, int stdout, int stderr, int argc, char *argv[])
{
	int i;
	char addr[16];
	struct tcb *ptcb;

	printf ("\tLocal addr\t\tRemote addr\tState\tRefs\n");
	printf ("------------------------------------------------------------\n");
	for (i = 0; i < Ntcp; i++) {
		ptcb = &tcbtab[i];
		wait (Tcp.tcpmutex);
		if (ptcb->tcb_state == TCB_FREE) {
			signal (Tcp.tcpmutex);
			continue;
		}
		wait (ptcb->tcb_mutex);
		signal (Tcp.tcpmutex);
		if (ptcb->tcb_state == TCB_CLOSED) {
			printf ("\t\t\t\t\t\tCLOSED\t%d\n", ptcb->tcb_ref);
			signal (ptcb->tcb_mutex);
			continue;
		}
		iptochar (ptcb->tcb_lip, addr);
		printf ("%15s:%d\t", addr, net2hs (ptcb->tcb_lport));
		iptochar (ptcb->tcb_rip, addr);
		printf ("%15s:%d\t", addr, net2hs (ptcb->tcb_rport));
		switch (ptcb->tcb_state) {
		case TCB_LISTEN:
			printf ("LISTEN\t");
			break;
		case TCB_SYNSENT:
			printf ("SYNSENT\t");
			break;
		case TCB_SYNRCVD:
			printf ("SYNRCVD\t");
			break;
		case TCB_ESTD:
			printf ("ESTD\t");
			break;
		case TCB_FIN1:
			printf ("FIN1\t");
			break;
		case TCB_FIN2:
			printf ("FIN2\t");
			break;
		case TCB_CWAIT:
			printf ("CWAIT\t");
			break;
		case TCB_CLOSING:
			printf ("CLOSING\t");
			break;
		case TCB_LASTACK:
			printf ("LASTACK\t");
			break;
		case TCB_TWAIT:
			printf ("TWAIT\t");
			break;
		}
		printf ("%d\n", ptcb->tcb_ref);
		signal (ptcb->tcb_mutex);
	}
	return OK;
}
