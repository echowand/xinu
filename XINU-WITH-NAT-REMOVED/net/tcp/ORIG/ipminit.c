/* ipminit.c - ipminit */

#include <conf.h>
#include <kernel.h>
#include <mark.h>
#include <bufpool.h>
#include <mq.h>
#include <netdefs.h>
#include <ether.h>
#include <netif.h>
#include <net.h>
#include <ip.h>
#include <ipif.h>
#include <arp.h>
#include <iproute.h>
#include <stdio.h>

/* Definition of global Ipif */

struct ipif ipif[NIF];

/* Definition of global Ip */

struct ipcontrol Ip;

/*------------------------------------------------------------------------
 *  ipminit - initialize the IP master device
 *------------------------------------------------------------------------
 */
int ipminit() {
	int i;

	/* Initialize ARP */

        arpinit();

	/* Initiliazine routing table */

        rtinit();

	/* Initialize IP network interfaces */

	for (i = 0; i < NIF; i++) {
		switch (i) {

		case NI_LOCAL:

			/* Set local ip: 127.0.0.1/8 */

			ipif[i].ipif_addr = hl2net(0x7f000001);
			ipif[i].ipif_mask = hl2net(0xff000000);
			break;

		default:
			
			/* Set not-initialized ifs to 0.0.0.0/32 */

			ipif[i].ipif_addr = IPADDR_ANY;
			ipif[i].ipif_mask = hl2net(0xffffffff);
			break;
		}

		/* Compute network and broadcast addresses */

		ipif[i].ipif_net = ipif[i].ipif_addr 
				& ipif[i].ipif_mask;
		ipif[i].ipif_bcast = ipif[i].ipif_addr 
				| ~ipif[i].ipif_mask;
	}

	/* Initialize IP message queues */

	Ip.fastipmq = mqcreate(IP_FQSIZE);
	Ip.slowipmq = mqcreate(IP_SQSIZE);

	/* Create IP device semaphore */

	Ip.devmutex = screate(1);

	/* Enable IP forwarding */

	Ip.forward = 1;

	/* Initialize medium and large buffer pools */

	Ip.mbpool = mkpool(IP_MEDBUFSZ, IP_NMEDBUF);
	Ip.lbpool = mkpool(IP_LRGBUFSZ, IP_NLRGBUF);

	/* Initialize fragment coalece (reasm) table */

	Ip.nfrags = 0;
	for (i = 0; i < IPFC_N; i++)
		ipfctab[i].head = NULL;

	/* Create and start fastip and slowip */

	resume(create((int *)fastip, 512, 90, "fastip", 0, NULL));
	resume(create((int *)slowip, 512, 80, "slowip", 0, NULL));

	return OK;
}
