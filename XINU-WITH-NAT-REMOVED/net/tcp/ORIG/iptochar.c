/* iptochar.c - iptochar */

#include <conf.h>
#include <kernel.h>
#include <ip.h>
#include <stdio.h>

/*------------------------------------------------------------------------
 *  iptochar - convert IP address from binary to a dotted decimal string
 *------------------------------------------------------------------------
 */
char *iptochar(IPaddr ip, char *dest) {
        sprintf(dest, "%d.%d.%d.%d",
		(ip>>24)&0xff,
		(ip>>16)&0xff,
		(ip>> 8)&0xff,
		(ip    )&0xff);
        return(dest);
}

