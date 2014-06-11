#include <conf.h>
#include <kernel.h>
#include <netdefs.h>
#include <ip.h>
#include <tcp.h>
#include <tcb.h>

u_short tcpcksum (struct ip *pip, struct tcp *ptcp)
{
	unsigned int sum = 0;
	u_short *ptr, len;
	int i;

	ptr = (u_short *)&pip->ip_src;
	for (i = 0; i < 4; i++)
		sum += *ptr++;
	sum += (hs2net (pip->ip_proto));
	len = net2hs (pip->ip_len) - IP_HLEN (pip);
	sum += hs2net (len);
	if (len % 2) {
		ptcp->tcp_data[len - TCP_MINHLEN] = 0;
		len++;
	}
	sum += ~cksum (ptcp, len) & 0xffff;
	sum = (sum & 0xffff) + (sum >> 16);
	return ~((sum & 0xffff) + (sum >> 16)) & 0xffff;
}
