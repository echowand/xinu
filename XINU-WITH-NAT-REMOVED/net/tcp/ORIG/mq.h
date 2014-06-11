#define NMQ	70

struct mqentry {
	int mq_state;
	int mq_first;
	int mq_count;
	int mq_sem;
	int mq_qlen;
	int mq_rcvrs;
	int mq_cookie;
	int *mq_msgs;
};

extern struct mqentry mqtab[NMQ];

void mqinit ();
int mqcreate (int qlen);
int mqdelete (int mq);
int mqsend (int mq, int msg);
int mqrecv (int mq);
int mqpoll (int mq);
int mqdisable (int mq);
int mqenable (int mq);
int mqclear (int mq, int (*func)(int));

#define MQVALID(mq) ((mq) >= 0 && (mq) < NMQ)
