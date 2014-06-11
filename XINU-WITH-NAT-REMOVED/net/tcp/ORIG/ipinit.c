/* ipinit.c - ipinit */

#include <conf.h>
#include <kernel.h>
#include <mq.h>
#include <ip.h>

/* Definition of global ipdevtab */

struct ipdev ipdevtab[Nip];

/*------------------------------------------------------------------------
 *  ipinit - initialize an IP slave device
 *------------------------------------------------------------------------
 */
int ipinit(struct devsw *pdev) {

	struct ipdev *pipd; /* pointer to the ipdevtab entry */
	
	pipd = &ipdevtab[pdev->dvminor];

	/* Initialize ipdevtab entry fields */

	pipd->ipd_proto = SYSERR;
	pipd->ipd_dvnum = pdev->dvnum;
	pipd->ipd_mq = mqcreate(5);
	pipd->ipd_tos = 0;
	pipd->ipd_ttl = IP_DEFTTL;
	pdev->dvioblk = (char *)pipd;

	return OK;
}
