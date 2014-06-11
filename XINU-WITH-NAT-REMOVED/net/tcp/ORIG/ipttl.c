/* ipttl.c - ipttl */

#include <conf.h>
#include <kernel.h>
#include <ip.h>

/*------------------------------------------------------------------------
 *  ipttl - decrement the TTL field, update cheksum, return new TTL
 *------------------------------------------------------------------------
 */
inline int ipttl(struct ip *pip) {
	unsigned int	adjust;	/* checksum adjustment */
	unsigned short	*pttl;	/* pointer to TTL field in datagram */

	/* If TTL is already zero, return without further decrement */

	if (pip->ip_ttl == 0)
		return 0;

	/* Decrement the TTL and adjust the IP checksum */

	adjust = pip->ip_cksum;
	pttl = (unsigned short *) &pip->ip_ttl;
	adjust += *pttl;
	pip->ip_ttl--;
	adjust -= *pttl;
	adjust = (adjust & 0xffff) + (adjust >> 16);
	pip->ip_cksum = adjust;
	
	/* Return the new TTL */

	return pip->ip_ttl;
}
