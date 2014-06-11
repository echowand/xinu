#include <conf.h>
#include <kernel.h>
#include <netdefs.h>
#include <ip.h>
#include <tcp.h>
#include <tcb.h>

void tcpnet2h (struct tcp *ptcp)
{
	ptcp->tcp_seq = net2hl (ptcp->tcp_seq);
	ptcp->tcp_ack = net2hl (ptcp->tcp_ack);
	ptcp->tcp_code = net2hs (ptcp->tcp_code);
	ptcp->tcp_window = net2hs (ptcp->tcp_window);
	ptcp->tcp_urgptr = net2hs (ptcp->tcp_urgptr);
}
