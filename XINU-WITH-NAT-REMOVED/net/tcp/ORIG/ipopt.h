#define IPOPT_EOL       0x00			/* End of opetions list	*/
#define IPOPT_NOP       0x01			/* No operation		*/
#define IPOPT_RR        0x07			/* Record route option	*/
#define IPOPT_TS        0x44			/* Time stamp option	*/
#define IPOPT_SEC       0x82			/* Security option	*/
#define IPOPT_LSRR      0x83			/* Loose src route opt	*/
#define IPOPT_SSRR      0x89			/* Strict src route opt	*/
#define IPOPT_SID       0x88			/* Stream identifier opt*/

#define IPOPT_TS_FLG    0x0f			/* tstamp opt flag 	*/
#define IPOPT_TS_OFLW   0xf0			/* tstamp opt overflow	*/

#define IPOPT_TS_ONLY   0x00			/* ts - ts only		*/
#define IPOPT_TS_SETA   0x01			/* ts - ts and address	*/
#define IPOPT_TS_SETT   0x03			/* ts - prespec address */

int ipopt(struct ep *pep, struct ip *pip);
int optlsrr (struct ep *pep, struct ip *pip, u_char *opt);
int optssrr (struct ep *pep, struct ip *pip, u_char *opt);
int optts (struct ep *pep, struct ip *pip, u_char *opt, IPaddr ourip);
int optrr (struct ep *pep, struct ip *pip, u_char *opt);
int optlocal (IPaddr addr);
void optfrag (struct ip *pip, struct ip *pipfrag);
