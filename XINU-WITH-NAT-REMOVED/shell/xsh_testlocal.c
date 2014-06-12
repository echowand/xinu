/* xsh_test.c - xsh_test */

#include <xinu.h>
#include <stdio.h>
#include <string.h>

/*------------------------------------------------------------------------
 * xsh_testlocal - shell command to test TCP
 *------------------------------------------------------------------------
 */
shellcmd xsh_testlocal(int nargs, char *args[])
{
	int32	fd;			/* descriptor for connection	*/
	char	str[] =			/* value for open		*/
	    "a:128.10.19.20:80";
	int32	retval;			/* Return from read		*/
	char	url[100];		/* URL to send			*/
	char	*pch;			/* walks url string		*/
	int32	i;			/* Counts bytes on output	*/
	char	buff[512];		/* Input buffer			*/
	int32	len;			/* Length of url		*/
	/* For argument '--help', emit help about the 'tcp' command	*/

	if (nargs == 2 && strncmp(args[1], "--help", 7) == 0) {
		printf("Use: %s\n\n", args[0]);
		printf("Description:\n");
		printf("\tTests TCP\n");
		printf("Options:\n");
		printf("\t--help\t display this help and exit\n");
		return 0;
	}

	if (nargs != 1) {
		fprintf(stderr, "%s\n", args[0]);
		fprintf(stderr, "Try '%s --help' for more information\n",
				args[0]);
		return 1;
	}

	kprintf ("\nForming TCP connection to %s...\n", str);
	fd = tcp_register(str);

	if (fd == SYSERR) {
		kprintf("%s: TCP open failed\n", args[0]);
		return 1;
	}

	strncpy(url, "GET /homes/comer/test.html \r\n\r\n", 100);

	kprintf ("TCP open succeeded\n");
	for (pch = url; *pch != NULLCH;) {
			pch++;
	}
	len = pch - url;
	kprintf("writing %s, %d bytes to %d\n", url, len, fd);
	tcp_send(fd, url, len);
	kprintf("reading 512 bytes\n");
	retval = tcp_recv(fd, buff, 512);
	kprintf ("read returns %d\n", retval);
	if (retval != SYSERR) {
		kprintf("Got back: ");
		for (i=0; i<retval; i++) {
			kprintf("%c", buff[i]);
		}
		kprintf("<----END\n");
	}
	kprintf("reading again\n");
	retval = tcp_recv(fd, buff, 512);
	kprintf ("read returns %d\n", retval); 

	kprintf ("Closing...\n");
	retval = tcp_close (fd);
	kprintf ("Close returns %d\n", retval);
	kprintf ("\nFinished\n");
	return 0;
}
