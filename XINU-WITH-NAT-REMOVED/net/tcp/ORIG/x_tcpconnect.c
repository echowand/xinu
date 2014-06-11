#include <conf.h>
#include <kernel.h>
#include <stdio.h>

COMMAND	x_tcpconnect (int stdin, int stdout, int stderr,
		      int nargs, char *args[])
{
	int fd;

	if (nargs != 2) {
		fprintf (stderr, "usage: tcpconnect <a|p><:host:port>\n");
	}
	fprintf (stdout, "Connecting...\n");
	fd = open (TCP, (int)args[1], 0);
	fprintf (stdout, "Done\n");
	sleep (3);
	fprintf (stdout, "Closing...\n");
	close (fd);
	fprintf (stdout, "Done\n");

	return OK;
}
