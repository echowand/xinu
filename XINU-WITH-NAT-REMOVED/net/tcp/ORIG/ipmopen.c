/* ipmopen.c - ipmopen */

#include <conf.h>
#include <kernel.h>
#include <ip.h>

/*------------------------------------------------------------------------
 *  ipmopen - open the master IP device - register an IP protocol
 *------------------------------------------------------------------------
 */
int ipmopen(struct devsw *pdev, void *arg2, void *arg3) {

	u_char proto = (int)arg2;	/* ip protocol number */
	int i;				/* int index */

	/* Lock the IP control structure */

	wait(Ip.devmutex);

	/* Check if protocol already registered and return */

	for (i = 0; i < Nip; i++) {
		if (ipdevtab[i].ipd_proto == proto) {
			signal(Ip.devmutex);
			return SYSERR;
		}
	}

	/* Look for an empty slot in the IP protocol table */

	for (i = 0; i < Nip; i++)
		if (ipdevtab[i].ipd_proto == SYSERR)
			break;

	/* If no slot is found, unlock and return */

	if (i == Nip) {
		signal (Ip.devmutex);
		return SYSERR;
	}

	/* Register the protocol in the IP slave device table */

	ipdevtab[i].ipd_proto = proto;

	/* Unlock the IP control device */

	signal(Ip.devmutex);

	/* Return the slave device number */

	return ipdevtab[i].ipd_dvnum;
}
