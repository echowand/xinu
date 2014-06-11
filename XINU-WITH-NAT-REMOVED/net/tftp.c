/* tftp.c - tftp, tftp_send1 */

#include <xinu.h>
#include <string.h>

/*------------------------------------------------------------------------
 *
 * tftp_send1 - internal function to send one outgoing request (RRQ or
 *		ACK) message during a read sequence and get a matching
 *		response (ignoring duplicates and nonsense packets)
 *
 *------------------------------------------------------------------------
 */

status	tftp_send1 (
	 int32	sock,			/* UDP socket to use		*/
	 uint32	remip,			/* Remote IP address		*/
	 uint16	*remport,		/* Remote port to use/set	*/
	 struct tftp_msg *msg,		/* Pointer to outgoing message	*/
	 int32	mlen,			/* Size of ougoing message	*/
	 struct tftp_msg *inmsg,	/* Pointer to buffer for	*/
					/*	an incoming message	*/
	 uint16	expected		/* Block number expected	*/
	)
{
	int32	ret;			/* Return value	for udp_send	*/
	int32	n;			/* Number of bytes in response	*/
	uint32	tmp;			/* Holds IP address on receive	*/

	/* Send the outgoing message */

	ret = udp_sendto(sock, remip, *remport, (char *) msg, mlen);

	if (ret == SYSERR) {
		return SYSERR;
	}

	/* Repeatedly read incoming messages, discarding packets that	*/
	/*	are not valid or do not match the expected block number	*/
	/*	(i.e., duplicates of previous packets)			*/

	while(1) {

		/* Read next incoming message */

		n = udp_recvaddr(sock, &tmp, remport, (char *)inmsg,
				sizeof(struct tftp_msg), TFTP_WAIT);
		if (n == SYSERR) {
			return SYSERR;
		} else if (n == TIMEOUT) {
			return TIMEOUT;
		}

		if (n < 4) {	/* too small to be a valid packet */
			continue;
		}

		/* If Error came back, give up */
		
		if (ntohs(inmsg->tf_opcode) == TFTP_ERROR) {
			kprintf("\nTFTP Error %d, %s\n",
					ntohs(inmsg->tf_ercode),
					inmsg->tf_ermsg );
			return SYSERR;
		}

		/* If data packet matches expected block, return */

		if ( (ntohs(inmsg->tf_opcode) == TFTP_DATA) &&
				(ntohs(inmsg->tf_dblk) == expected)) {
			return n;
		}

		/* As this point, incoming message is either invalid or	*/
		/*	a duplicate, so ignore and try again		*/
	}
}


/*------------------------------------------------------------------------
 *
 * tftp - use TFTP to download a specified file from the default server
 *
 *------------------------------------------------------------------------
 */
status	tftp (
	 char	*filename,	/* Name of the file to download		*/
	 char	*buf,		/* Buffer to hold the file		*/
	 int32	buflen		/* Size of the buffer			*/
	)
{
	uint32	serverip;	/* IP address of server in binary	*/
	int32	nlen;		/* Length of file name			*/
	uint16	localport;	/* Local UDP port to use		*/
	uint16	remport=TFTP_PORT;/* Local UDP port to use		*/
	int32	sock;		/* socket descriptor to use		*/
	int32	iface;		/* Interface used to reach the server	*/
	uint16	expected = 1;	/* Next data block we expect		*/
	int32	i;		/* Loop index				*/
	int32	n;		/* Number of octets in incoming message	*/
	int32	ret;		/* Return value				*/
	int32	filesiz;	/* Total size of downloaded file	*/
	struct	tftp_msg outmsg;/* Outgoing message			*/
	int32	mlen;		/* Length of outgoing mesage		*/
	struct	tftp_msg inmsg;	/* Buffer for response message		*/
	int32	dlen;		/* Size of data in a response		*/
	uint32	now;		/* Current time (used to generate a	*/
				/*   "random" port number		*/

	/* check args */
	
	if (dot2ip(TFTP_SERVER, &serverip) == SYSERR) {
		return SYSERR;
	}

	nlen = strnlen(filename, TFTP_MAXNAM+1);
	if ( (nlen <= 0) || (nlen > TFTP_MAXNAM) ) {
		return SYSERR;
	}

	/* Find the interface to use */

	iface = ip_route(serverip);
	if (iface == SYSERR) {
		return SYSERR;
	}

	/* Generate a local port */

	if (getutime(&now) == SYSERR) {
		return SYSERR;
	}
	localport = (uint16) ( 50000 + (now & 0xfff) );

	kprintf("Using local port %u\n", 0xffff & localport);

	/* Register a UDP socket */

	sock = udp_register(iface, serverip, 0, localport);
	if (sock == SYSERR) {
		return SYSERR;
	}

	/* Clear the message buffer */

	memset((char*)&outmsg, NULLCH, sizeof(outmsg));

	/* Initialize the total file size to zero */

	filesiz = 0;

	/* Form the first message and compute length (a Read Request)	*/

	outmsg.tf_opcode = htons(TFTP_RRQ);
	strncpy(outmsg.tf_filemode, filename, nlen+1);
	/* set mode to 'octet' */
	strncpy(outmsg.tf_filemode+nlen+1, "octet", sizeof("octet")+1);

	/* length is 2 opcode octets, name length, "octet" length,	*/
	/*		and two NULLs					*/

	mlen = nlen + strnlen("octet", 6) + 4;

	/* Repeatedly send the next request and get a response,		*/
	/*	retransmitting a request up to TFTP_MAXRETRIES times	*/

	while(1) {
		for (i=0; i < TFTP_MAXRETRIES; i++) {
			n = tftp_send1(sock, serverip, &remport, &outmsg,
					mlen, &inmsg, expected);
			if (n > 0) {
				break;
			} else if (n == SYSERR) {
				udp_release(sock);
				return SYSERR;
			} else if (n == TIMEOUT) {
				continue;

			}
		}
		if (i >= TFTP_MAXRETRIES) {
			udp_release(sock);
			return SYSERR;
		}

		/* Compute size of data in the message */

		dlen = n - sizeof(inmsg.tf_opcode) - sizeof(inmsg.tf_dblk);

		/* Move the contents of this block into the file buffer	*/

		for (i=0; i<dlen; i++) {
			filesiz++;
			if (filesiz > buflen) {
				udp_release(sock);
				return SYSERR;
			}
			*buf++ = inmsg.tf_data[i];
		}

		/* Form an ACK */

		outmsg.tf_opcode = htons(TFTP_ACK);
		outmsg.tf_ablk = htons(expected);
		mlen = sizeof(outmsg.tf_opcode) + sizeof(outmsg.tf_ablk);

		/* If this was the last packet, send final ACK */

		if (dlen < 512) {
			ret = udp_sendto(sock, serverip, remport,
						(char *) &outmsg, mlen);
			if (ret == SYSERR) {
				return SYSERR;
			}
			return filesiz;
		}

		/* Move to next block and continue */

		expected++;
	}
}
