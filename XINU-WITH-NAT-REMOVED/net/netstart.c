/* netstart.c - netstart */

#include <xinu.h>
#include <stdio.h>

bool8	host;				/* TRUE if running a host	*/
int32	bingid;				/* User's bing ID		*/

#define	ROUTER_SUFFIX	0x00006464	/* router IP address on othernet*/
					/*   ENDS IN   .100.100		*/

/*------------------------------------------------------------------------
 * netstart - ask user for Bing ID and whether to run a host or router
 *		and set configuration variables accordingly
 *------------------------------------------------------------------------
 */

void	netstart (void)
{
	int32	iface;			/* interface index		*/
	struct	ifentry	*ifptr;		/* ptr to interface		*/
	uint32	ipaddr;			/* IP address			*/
	uint32	ipmask;			/* IP address mask		*/
	uint32	iprouter;		/* Defualt router address	*/

	/* Initialize network interfaces */

	kprintf("...initializing network stack\n");
	net_init();

	/* Delay because Ethernet driver doesn't work without it */

	sleepms(800);

	/* Obtain a host address on each active interface */

	for (iface=0; iface<NIFACES; iface++) {

		/* Primary interface *must* be up; others  are simply	*/
		/*	skipped if they are down			*/

		if (if_tab[iface].if_state != IF_UP) {
			if (iface == ifprime) {
				panic("Error: primary interface is down\n");
			}
			continue;
		}

		kprintf("...using dhcp to obtain an IP address");
		kprintf(" on interface %d\n", iface);

		ipaddr = getlocalip(iface);
		if (ipaddr == SYSERR) {

			/* Primary interface must have an address */

			if (iface == ifprime) {
				panic("Error: could not obtain an IP address");
			} else {
				if_tab[iface].if_state = IF_DOWN;
			}
		}
	}

	/* Give IP address on primary interface */

	ifptr = &if_tab[ifprime];
	ipaddr = ifptr->if_ipucast;
	ipmask = ifptr->if_ipmask;
	iprouter = ifptr->if_iprouter;

	kprintf("\nIP address is %d.%d.%d.%d   (0x%08x)\n\r",
		(ipaddr>>24)&0xff, (ipaddr>>16)&0xff, (ipaddr>>8)&0xff,
		ipaddr&0xff,ipaddr);


	kprintf("Subnet mask is %d.%d.%d.%d and router is %d.%d.%d.%d\n\r",
		(ipmask>>24)&0xff, (ipmask>>16)&0xff,
		(ipmask>> 8)&0xff,  ipmask&0xff,
		(iprouter>>24)&0xff, (iprouter>>16)&0xff,
		(iprouter>> 8)&0xff, iprouter&0xff);
	return;
}
