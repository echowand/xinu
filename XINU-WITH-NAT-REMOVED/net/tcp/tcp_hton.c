

/* tcp_hton.c  -  tcp_hton, tcp_ntoh */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  tcp_hton  -  convert TCP fields from host byte order to network order
 *------------------------------------------------------------------------
 */
void	tcp_hton(
	  struct tcp	*ptcp		/* Pointer to a TCP segment	*/
	)
{
	ptcp->tcp_sport = htons (ptcp->tcp_sport);
	ptcp->tcp_dport = htons (ptcp->tcp_dport);
	ptcp->tcp_seq = htonl (ptcp->tcp_seq);
	ptcp->tcp_ack = htonl (ptcp->tcp_ack);
	ptcp->tcp_code = htons (ptcp->tcp_code);
	ptcp->tcp_window = htons (ptcp->tcp_window);
	ptcp->tcp_urgptr = htons (ptcp->tcp_urgptr);
	return;
}


/*------------------------------------------------------------------------
 *  tcp_ntoh  -  convert TCP fields from network byte order to host order
 *------------------------------------------------------------------------
 */
void	tcp_ntoh(
	  struct tcp	*ptcp		/* Pointer to a TCP segment	*/
	)
{
	ptcp->tcp_sport = ntohs (ptcp->tcp_sport);
	ptcp->tcp_dport = ntohs (ptcp->tcp_dport);
	ptcp->tcp_seq = ntohl (ptcp->tcp_seq);
	ptcp->tcp_ack = ntohl (ptcp->tcp_ack);
	ptcp->tcp_code = ntohs (ptcp->tcp_code);
	ptcp->tcp_window = ntohs (ptcp->tcp_window);
	ptcp->tcp_urgptr = ntohs (ptcp->tcp_urgptr);
	return;
}
