/* tcpcwait.c  -  tcpcwait */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  tcpcwait  -  perform null action when in wait state
 *------------------------------------------------------------------------
 */
int32	tcpcwait(
	  struct tcb	*ptcb,		/* Ptr to a TCB			*/
	  struct ip	*pip,		/* Ptr to IP (not used)		*/
	  struct tcp	*ptcp		/* Ptr to TCP (not used)	*/
	)
{
	return OK;
}
