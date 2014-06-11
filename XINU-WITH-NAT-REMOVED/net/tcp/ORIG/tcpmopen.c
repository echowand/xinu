#include <conf.h>
#include <kernel.h>
#include <mq.h>
#include <netdefs.h>
#include <ip.h>
#include <ipif.h>
#include <iproute.h>
#include <dns.h>
#include <tcp.h>
#include <tcb.h>

static int checktuple (IPaddr lip, u_short lport, IPaddr rip, u_short rport)
{
	int i;
	for (i = 0; i < Ntcp; i++) {
		if (tcbtab[i].tcb_state != TCB_FREE
		    && tcbtab[i].tcb_lip == lip
		    && tcbtab[i].tcb_rip == rip
		    && tcbtab[i].tcb_lport == lport
		    && tcbtab[i].tcb_rport == rport)
			return SYSERR;
	}
	return OK;
}

int tcpmopen (struct devsw *pdev, int arg1, int arg2)
{
	char *spec = (char *)arg1;
	struct route *prt;
	struct tcb *ptcb;
	IPaddr ip, lip;
	u_short port;
	int i, active, state;

	if (cspecparse (spec, &ip, &port, &active) == SYSERR)
		return SYSERR;

	wait (Tcp.tcpmutex);
	for (i = 0; i < Ntcp; i++) {
		if (tcbtab[i].tcb_state == TCB_FREE)
			break;
	}
	if (i == Ntcp) {
		signal (Tcp.tcpmutex);
		return SYSERR;
	}
	ptcb = &tcbtab[i];
	tcbclear (ptcb);

	if (spec[0] == 'a') {
		prt = rtget (ip);
		if (prt == NULL) {
			signal (Tcp.tcpmutex);
			return SYSERR;
		}
		lip = ipif[prt->rt_ifnum].ipif_addr;
		rtfree (prt);
		ptcb->tcb_rbuf = (char *)getmem (65535);
		if (ptcb->tcb_rbuf == (char *)SYSERR) {
			signal (Tcp.tcpmutex);
			return SYSERR;
		}
		ptcb->tcb_rbsize = 65535;
		ptcb->tcb_sbuf = (char *)getmem (65535);
		if (ptcb->tcb_sbuf == (char *)SYSERR) {
			freemem ((struct mblock *)ptcb->tcb_rbuf, 65535);
			signal (Tcp.tcpmutex);
			return SYSERR;
		}
		ptcb->tcb_sbsize = 65535;
		/* This will always succeed, we check Ntcp + 1 numbers */
		for (i = 0; i < Ntcp + 1; i++) {
				if (checktuple (lip, Tcp.tcpnextport,
						ip, port) == OK)
					break;
			Tcp.tcpnextport++;
			if (Tcp.tcpnextport > 2048)
				Tcp.tcpnextport = 1024;
		}

		ptcb->tcb_lip = lip;
		ptcb->tcb_lport = hs2net (Tcp.tcpnextport++);
		ptcb->tcb_rip = ip;
		ptcb->tcb_rport = hs2net (port);
		ptcb->tcb_snext = ptcb->tcb_suna = ptcb->tcb_ssyn = 1;	/* XXX */
		ptcb->tcb_state = TCB_SYNSENT;
		wait (ptcb->tcb_mutex);
		tcbref (ptcb);
		signal (Tcp.tcpmutex);

		tcbref (ptcb);
		mqsend (Tcp.tcpcmdq, TCBCMD(ptcb, TCBC_SEND));
		while (ptcb->tcb_state != TCB_CLOSED
		       && ptcb->tcb_state != TCB_ESTD) {
			ptcb->tcb_readers++;
			signal (ptcb->tcb_mutex);
			wait (ptcb->tcb_rblock);
			wait (ptcb->tcb_mutex);
		}
		if ((state = ptcb->tcb_state) == TCB_CLOSED)
			tcbunref (ptcb);
		signal (ptcb->tcb_mutex);
		return (state == TCB_ESTD ? ptcb->tcb_dvnum : SYSERR);
	} else if (spec[0] == 'p') {
		for (i = 0; i < Ntcp; i++) {
			if (tcbtab[i].tcb_state == TCB_LISTEN
			    && tcbtab[i].tcb_lip == ip
			    && tcbtab[i].tcb_lport == port) {
				signal (Tcp.tcpmutex);
				return SYSERR;
			}
		}
		ptcb->tcb_lip = ip;
		ptcb->tcb_lport = hs2net (port);
		ptcb->tcb_state = TCB_LISTEN;
		tcbref (ptcb);
		signal (Tcp.tcpmutex);
	} else {
		signal (Tcp.tcpmutex);
		return SYSERR;
	}


	return ptcb->tcb_dvnum;
}
