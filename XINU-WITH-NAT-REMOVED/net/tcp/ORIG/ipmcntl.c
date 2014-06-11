/* ipmcntl.c - ipmcntl */

#include <conf.h>
#include <kernel.h>
#include <netif.h>
#include <ip.h>
#include <iproute.h>

/*------------------------------------------------------------------------
 *  ipmcntl - IP master device control
 *------------------------------------------------------------------------
 */
int ipmcntl(struct devsw *pdev, int func, int arg) {

        int i;	/* index in the IP interface table */

	/* 
		IP device control functions:
			IPC_MJOIN  - join a multicast group
			IPC_MLEAVE - leave a multicast group 
	*/

        switch (func) {

        case IPC_MJOIN:

		/* Check if arg is valid IP multicast address */

        	if (!IP_ISMULTI(arg)) return SYSERR;

		/* Add the multicast route to the routing table */

                rtadd(arg, RT_IPFULL, arg, NI_LOCAL, 
			RT_DFLT_METRIC, RT_TTLINF, 0, TRUE);

		/* For each interface, register the hw mcast address */

                for (i=0;i<NIF;i++)
                        if (nif[i].ni_mcast!=NULL)
                                nif[i].ni_mcast(NI_MADD, nif[i].ni_dev, 
						arg);
                return OK;

        case IPC_MLEAVE:

		/* Check if arg is valid IP multicast address */

        	if (!IP_ISMULTI(arg)) return SYSERR;
		
		/* Remove the multicast route from the routing table */

                rtdel (arg, RT_IPFULL);

		/* For each interface, unregister the hw mcast addr */

                for (i=0;i<NIF;i++)
                        if (nif[i].ni_mcast!=NULL)
                                nif[i].ni_mcast(NI_MDEL, nif[i].ni_dev, 
						arg);
                return OK;
        }

        return SYSERR;
}

