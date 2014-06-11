/* iproute - ip routing table implementation definitions */

#define RT_NROUTES      128	/* max number of routes			*/
#define RT_MAXIPSTR     16      /* max length of an IP address string   */
#define RT_TTLINF       999     /* infinity ttl			 	*/
#define RT_TTLGC	120     /* garbage collection timer	     	*/
#define RT_DFLT_METRIC	1	/* default metric 			*/
#define RT_METRICINF    16      /* infinity metric		      	*/
#define RT_IPFULL       0xFFFFFFFF /* full 1s 32-bit mask 		*/

#define RTF_NULL	0x0000	/* */
#define RTF_CHANGED     0x0001	/* */
#define RTF_NOTADV      0x0002	/* */

struct route {
	int     rt_iself;	/* */
	int     rt_inext;	/* */
	int     rt_iprefix;	/* */

	IPaddr	rt_net;		/* network address for the route	*/
	IPaddr	rt_mask;	/* mask for the route			*/
	IPaddr	rt_gw;		/* nexthop address			*/
	u_int	rt_ifnum;	/* output interface for the route	*/
	u_int   rt_metric;	/* RIP route distance metric 		*/
	u_int	rt_ttl;		/* RIP time to live (in seconds) 	*/
	u_short rt_tag;		/* the RIP tag for the route	    	*/
	u_short rt_flags;	/* the change flag			*/
};

struct rttable {		/* routing table structure		*/
	struct route rtb_table[RT_NROUTES];   /* route entries table 	*/
	u_int rtb_trie[2*RT_NROUTES];	      /* route table trie 	*/
	struct route *rtb_prefix[RT_NROUTES]; /* */
	
	int rtb_ifree;		/* */
	int rtb_irlist;		/* */
	
	int rtb_rcount;		/* */
	u_int rtb_nfree;	/* */
	u_int rtb_plast;	/* */
};

struct rtinfo {
	int    ri_rsem;		/* */
	int    ri_wsem;		/* */
	int    ri_refsem;
	
	int    ri_havegc;	/* do we have a route garbage collector */

	struct rttable *ri_lookup;	          /* the lookup table  */
	struct rttable *ri_working;     	  /* the working table */
};

extern struct rtinfo Route;

#define RT_IEND -1

#define RT_PTR(prtable, index) (&(prtable->rtb_table[index]))
#define RT_IPTR(ptr) (ptr==NULL) ? RT_IEND:ptr->rt_iself

#define RT_PHEAD(prtable) (prtable->rtb_irlist==RT_IEND) ? NULL:(&(prtable->rtb_table[prtable->rtb_irlist]))

#define RT_PNEXT(prtable, prt)   ((prt->rt_inext==RT_IEND) ? NULL:(&(prtable->rtb_table[prt->rt_inext])))
#define RT_PNPREFIX(prtable, prt) ((prt->rt_iprefix==RT_IEND) ? NULL:(&(prtable->rtb_table[prt->rt_iprefix])))
#define RT_PFREE(prtable, prt) {prt->rt_inext=prtable->rtb_ifree; prtable->rtb_ifree=prt->rt_iself;}

/* trie construction macros */

#define RT_BITEXTRACT(integer, skip, nbits) \
	(((integer) << (skip)) >> (32-(nbits)))

#define RT_BUILD_LEAF(branch, skip, pindex) \
	((((((branch << 5) + skip) << 1) + 1) << 21) + pindex)

#define RT_BUILD_NODE(branch, skip, index) \
	((((branch << 5) + skip) << 22) + index)

#define RT_ISPREFIXOF(prt, mrt) \
	((net2hl(mrt->rt_mask) > net2hl(prt->rt_mask)) && \
	 (net2hl(prt->rt_net) == net2hl((prt->rt_mask & mrt->rt_net))))

void rtinit (void);
int rtadd (IPaddr prefix, IPaddr mask, IPaddr gateway, u_int ifnum, 
	u_int metric, u_int ttl, u_int tag, u_int notadv);
int rtaddnl (IPaddr prefix, IPaddr mask, IPaddr gateway, u_int ifnum, 
	u_int metric, u_int ttl, u_int tag, u_int notadv);
int rtdel (IPaddr, IPaddr);
int rtdelnb (IPaddr prefix, IPaddr mask);
struct route *rtget (IPaddr);
struct route *rtlookup (struct rttable *, IPaddr);
int rtfree (struct route *);
void rttrie ();
void rtcommit ();
void rtdump (int stdout);
void rtprint(int stdout, struct route *prt);
void trieprint(int stdout, struct rttable *prtable);
int rtroute (int, int, int, int, char**);
