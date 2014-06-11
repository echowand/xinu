/* tcptwait.c  -  tcptwait */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  tcptwait  -  Handle an incoming segment in the TIME-WAIT state
 *------------------------------------------------------------------------
 */
int32	tcptwait(
	  struct tcb	*ptcb,		/* Ptr to a TCB			*/
	  struct ip	*pip,		/* Ptr to IP (not used)		*/
	  struct tcp	*ptcp		/* Ptr to TCP (not used)	*/
	)
{
	return OK;
}
