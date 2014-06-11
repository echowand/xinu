#define NTIMERS		100
#define BADTIMER	-1

#define TMSTK		512
#define TMPRIO		100
#define TMNAME		"timer"

struct tmentry {
	int tm_remain;
	int tm_mq;
	int tm_msg;
	int tm_next;
};

extern int tmpending;
extern int *tmnext;

extern struct tmentry tmtab[NTIMERS];
extern int tmfree;
extern int tmhead;

PROCESS timer ();
INTPROC tmfire ();

void tminit ();
int tmset (int delay, int mq, int msg);
int tmdel (int mq, int msg);
