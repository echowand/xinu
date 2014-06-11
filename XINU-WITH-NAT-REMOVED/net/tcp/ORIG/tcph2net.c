#include <conf.h>
#include <kernel.h>
#include <netdefs.h>
#include <ip.h>
#include <tcp.h>
#include <tcb.h>

void tcph2net (struct tcp *ptcp)
{
	ptcp->tcp_seq = hl2net (ptcp->tcp_seq);
	ptcp->tcp_ack = hl2net (ptcp->tcp_ack);
	ptcp->tcp_code = hs2net (ptcp->tcp_code);
	ptcp->tcp_window = hs2net (ptcp->tcp_window);
	ptcp->tcp_urgptr = hs2net (ptcp->tcp_urgptr);
}
