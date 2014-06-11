/* ipread.c - ipread */

#include <conf.h>
#include <kernel.h>
#include <mq.h>
#include <ether.h>
#include <ip.h>
#include <stdio.h>

/*------------------------------------------------------------------------
 *  ipread - read from IP slave device the pointer of the next packet
 *------------------------------------------------------------------------
 */
int ipread(struct devsw *pdev, char *data, int len) {
	struct ep *pep;		/* pointer to packet */
	struct ipdev *pipd;	/* pointer to entry in IP dev table */

	/* Set pipd to the Xinu device block */

	pipd = (struct ipdev *)pdev->dvioblk;

	/* Check the requested length */

	if (len != sizeof(struct ep *))
		return SYSERR;

	/* Read the device associated message queue */

	pep = (struct ep *)mqrecv (pipd->ipd_mq);
	if (pep == (struct ep *)SYSERR)
		return SYSERR;

	/* Copy the packet pointer and return */

	*(struct ep **)data = pep;
	return OK;
}
