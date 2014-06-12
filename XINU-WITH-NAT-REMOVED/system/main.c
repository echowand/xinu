/*  main.c  - main */

#include <xinu.h>
#include <stdio.h>

char	bigbuf[100000];

#define	FILENAME	"atom2.boot"

int32	main(void)
{
	uint32	retval;
	int32	i, j;
	char	*p;


	kprintf("\n\n###########################################################\n\n");

	kprintf("...starting the network\n");
	netstart();

	kprintf("\n...trying tftp\n");

	retval = tftp(FILENAME, bigbuf, sizeof(bigbuf));
	kprintf("GOT BACK %d\n", retval);
	p = bigbuf;

	/*for  (i=0; i<200; i++) {
		kprintf("%07o",((char *)p) - bigbuf);
		for (j=0; j<4; j++) {
			kprintf(" %02x",(*p++)& 0xff);
			kprintf("%02x",(*p++)& 0xff);
			kprintf("%02x",(*p++)& 0xff);
			kprintf("%02x",(*p++)& 0xff);
		}
		kprintf("\n");
	}*/


	kprintf("\n...creating a shell\n");
	recvclr();
	resume(create(shell, 8192, 50, "shell", 1, CONSOLE));

	/* Wait for shell to exit and recreate it */

	while (TRUE) {
		retval = receive();
		sleepms(200);
		kprintf("\n\n\rMain process recreating shell\n\n\r");
		resume(create(shell, 4096, 20, "shell", 1, CONSOLE));
	}
	return OK;
}
