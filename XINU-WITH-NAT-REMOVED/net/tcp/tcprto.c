/* tcprto.c  -  tcprto */

#include <xinu.h>

extern	uint32	ctr1000;

/*------------------------------------------------------------------------
 *  tcprto  -  Compute round-trip timeout
 *------------------------------------------------------------------------
 */
int32	tcprto(
	  struct tcb	*ptcb		/* Ptr to a TCB			*/
	)
{
	int32		x;		/* Increment			*/
	int32		r;		/* round trip			*/

	r = min ((int)ctr1000 - ptcb->tcb_rtttime, 1);

	/* Use signed integer arithmentic for the above calculation to	*/
	/* handle overflow.  We must bound our lower measurement at	*/
	/* 1 millisecond because that's the clock granularity.		*/

	if (ptcb->tcb_srtt == 0) {

		/* Scaling for powers of two: the smothed round-trip	*/
		/* time, SRTT, is stored as the	actual SRTT times 8,	*/
		/* and RTTVAR is stored as RTTVAR times 4. Because the	*/
		/* following assignment RTTVAR = R/2, we use * 2.	*/

		ptcb->tcb_srtt = r << 3;
		ptcb->tcb_rttvar = r << 1;

	} else {
		x = (ptcb->tcb_srtt >> 3) - r;
		if (x < 0) {
			x = -x;
		}
		ptcb->tcb_rttvar += -(ptcb->tcb_rttvar << 2) + x;
		ptcb->tcb_srtt += -(ptcb->tcb_srtt >> 3) + r;
	}
	ptcb->tcb_rto = (ptcb->tcb_srtt >> 3) + ptcb->tcb_rttvar;

	return ptcb->tcb_rto;
}
