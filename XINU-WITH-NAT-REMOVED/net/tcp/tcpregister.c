/* tcpmopen.c  -  checktuple, tcpmopen */

#include <xinu.h>
#include <string.h>

/*------------------------------------------------------------------------
 *  ckecktuple  -  Verify that a TCP connection is not already in use by
 *			checking (src IP, src port, Dst IP, Dst port)
 *------------------------------------------------------------------------
 */
static	int32	checktuple (
		  IPaddr	lip,	/* Local IP address		*/
		  uint16	lport,	/* Local TCP port number	*/
		  IPaddr	rip,	/* Remote IP address		*/
		  uint16	rport	/* Remote TCP port number	*/
		)
{
	int32	i;			/* Walks though TCBs		*/
	struct	tcb *ptcb;		/* Ptr to TCB entry 		*/

	/* Check each TCB that is open */

	for (i = 0; i < Ntcp; i++) {
		ptcb = &tcbtab[i];
		if (ptcb->tcb_state != TCB_FREE
		    && ptcb->tcb_lip == lip
		    && ptcb->tcb_rip == rip
		    && ptcb->tcb_lport == lport
		    && ptcb->tcb_rport == rport) {
			return SYSERR;
		}
	}
	return OK;
}

/*------------------------------------------------------------------------
 *  tcp_register  -  Open a TCP connection and receive the index of the 
 *  		     endpoint
 *			
 *------------------------------------------------------------------------
 */
int32	tcp_register(
	  int32	arg1			/* Argument from open		*/
	)
{
	char		*spec;		/* IP:port as a string		*/
	struct tcb	*ptcb;		/* Ptr to TCB			*/
	IPaddr		ip;		/* Remote IP address		*/
	IPaddr		lip;		/* Local IP address		*/
	uint16		port;		/* Port number			*/
	int32		i;		/* Walks through TCBs		*/
	int32		active;		/* Active or passive type?	*/ 
	int32		state;		/* Connection state		*/
	int32		iface;		/* Interface to use		*/
	struct ifentry	*ifptr;		/* ptr to interface entry	*/
	int32		index;		/* TCP endpoint index		*/

	/* Parse "X:machine:port" string and set variables, where	*/
	/*	X	- either 'a' or 'p' for "active" or "passive"	*/
	/*	machine	- an IP address in dotted decimal		*/
	/*	port	- a protocol port number			*/

	spec = (char *)arg1;
	if (tcpparse (spec, &ip, &port, &active) == SYSERR) {
		return SYSERR;
	}

	/* Obtain exclusive use, find free TCB, and clear it */

	wait (Tcp.tcpmutex);
	
	for (i = 0; i < Ntcp; i++) {
		if (tcbtab[i].tcb_state == TCB_FREE)
			break;
	}
	if (i >= Ntcp){
		signal (Tcp.tcpmutex);
		return SYSERR;
	}
	ptcb = &tcbtab[i];
	tcbclear (ptcb);
	index = i;
	/* Either set up active or passive endpoint */

	if (active) {

		/* Lookup up route */

		iface = ip_route(ip);

		if (iface == SYSERR) {
			signal (Tcp.tcpmutex);
			return SYSERR;
		}

		/* Record interface to use */

		ptcb->tcb_iface = iface;

		/* Obtain ptr to interface entry */
		ifptr = &if_tab[iface];

		/* Obtain local IP address from interface */

		lip = ifptr->if_ipucast;

		/* Allocate receive buffer and initialize ptrs */

		ptcb->tcb_rbuf = (char *)getmem (65535);
		if (ptcb->tcb_rbuf == (char *)SYSERR) {
			signal (Tcp.tcpmutex);
			return SYSERR;
		}
		ptcb->tcb_rbsize = 65535;
		ptcb->tcb_sbuf = (char *)getmem (65535);
		if (ptcb->tcb_sbuf == (char *)SYSERR) {
			freemem ((char *)ptcb->tcb_rbuf, 65535);
			signal (Tcp.tcpmutex);
			return SYSERR;
		}
		ptcb->tcb_sbsize = 65535;

		/* The following will always succeed because	*/
		/*   the iteration covers at least Ntcp+1 port	*/
		/*   numbers and there are only Ntcp slots	*/

		for (i = 0; i < Ntcp + 1; i++) {
			if (checktuple (lip, Tcp.tcpnextport,
					      ip, port) == OK) {
				break;
			}
			Tcp.tcpnextport++;
			if (Tcp.tcpnextport > 63000)
				Tcp.tcpnextport = 33000;
		}

		ptcb->tcb_lip = lip;

		/* Assign next local port */


		ptcb->tcb_lport = Tcp.tcpnextport++;
		if (Tcp.tcpnextport > 63000) {
			Tcp.tcpnextport = 33000;
		}
		ptcb->tcb_rip = ip;
		ptcb->tcb_rport = port;
		ptcb->tcb_snext = ptcb->tcb_suna = ptcb->tcb_ssyn = 1;
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
		if ((state = ptcb->tcb_state) == TCB_CLOSED) {
			tcbunref (ptcb);
		}
		signal (ptcb->tcb_mutex);
		
		return (state == TCB_ESTD ? index : SYSERR);

	} else {  /* Passive connection */
		for (i = 0; i < Ntcp; i++) {
			if (tcbtab[i].tcb_state == TCB_LISTEN
			    && tcbtab[i].tcb_lip == ip
			    && tcbtab[i].tcb_lport == port) {

				/* Duplicates prior connection */
 
				signal (Tcp.tcpmutex);
				return SYSERR;
			}
		}

		/* New passive endpoint - fill in TCB */

		ptcb->tcb_lip = ip;
		ptcb->tcb_lport = port;
		ptcb->tcb_state = TCB_LISTEN;
		tcbref (ptcb);
		signal (Tcp.tcpmutex);

		/* Return slave device to caller */

		return index;
	}
}
