/* tcp.c -- tcp_ntoh, tcp_hton, tcp_cksum */

#include <xinu.h>

//struct	tcpentry tcptab[TCP_SLOTS];		/* table of TCP endpts	*/

/*------------------------------------------------------------------------
 * tcp_ntoh - convert UDP header fields from net to host byte order
 *------------------------------------------------------------------------
 */
void 	tcp_ntoh2(
	  struct netpacket *pktptr
	)
{
	return;
}

/*------------------------------------------------------------------------
 * tcp_hton - convert packet header fields from host to net byte order
 *------------------------------------------------------------------------
 */
void 	tcp_hton2(
	  struct netpacket *pktptr
	)
{
	return;
}


/*------------------------------------------------------------------------
 * tcp_cksum - compute a TCP checksum
 *------------------------------------------------------------------------
 */
uint16 	tcp_cksum(
	  char	*buffer,
	  int32	len
	)
{
	return (uint16) 0;
}
