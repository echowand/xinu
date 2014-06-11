#include <conf.h>
#include <kernel.h>
#include <ip.h>
#include <tcp.h>
#include <tcb.h>

extern unsigned int ctr1000;

int tcprto (struct tcb *ptcb)
{
	int x, r = min ((int)ctr1000 - ptcb->tcb_rtttime, 1);
	/* Use signed integer math for the above calculation to handle
	 * overflow.  Bound our lower measurement at 1ms, since this is
	 * our clock granularity */

	if (ptcb->tcb_srtt == 0) {
		/* SRTT is stored SRTT*8, and RTTVAR as RTTVAR*4. As
		 * this assignment is to be RTTVAR = R/2, we use *2. */
		ptcb->tcb_srtt = r << 3;
		ptcb->tcb_rttvar = r << 1;
	} else {
		x = (ptcb->tcb_srtt >> 3) - r;
		if (x < 0)
			x = -x;
		ptcb->tcb_rttvar += -(ptcb->tcb_rttvar << 2) + x;
		ptcb->tcb_srtt += -(ptcb->tcb_srtt >> 3) + r;
	}
	ptcb->tcb_rto = (ptcb->tcb_srtt >> 3) + ptcb->tcb_rttvar;

	return ptcb->tcb_rto;
}
