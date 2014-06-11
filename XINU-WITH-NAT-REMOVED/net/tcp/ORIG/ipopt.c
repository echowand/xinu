/* ipopt.c - ipopt */

#include <conf.h>
#include <kernel.h>
#include <netdefs.h>
#include <ether.h>
#include <ip.h>
#include <ipopt.h>
#include <icmp.h>
#include <stdio.h>

/*------------------------------------------------------------------------
 *  ipopt - process IP options
 *------------------------------------------------------------------------
 */
int ipopt(struct ep *pep, struct ip *pip) {
	u_char *prr   = NULL;	/* MUST - record route pointer	  */
	u_char *pts   = NULL;	/* MUST - timestamp		  */
	u_char *psec  = NULL;	/* OPTIONAL - security option     */
	u_char *plsrr = NULL;	/* MUST - loose source and record */
	u_char *pssrr = NULL;	/* MUST - strict source and rcord */
	u_char *psid  = NULL;	/* NO PROCESSING - stream id      */
	u_char *perr  = NULL;	/* error pointer		  */

	u_char *pop;		 /* pointer to start of options	  */
	u_char *peop;		 /* pointer to end of options 	  */
	u_char bogus;		 /* track if any option if bogus  */

	/* Initialize */

	pop  = ((u_char *) pip) + IP_MINHLEN;
	peop = ((u_char *) pip) + IP_HLEN(pip);
	bogus = FALSE;

	/* Process options and set associated pointers */

	while (pop<peop) {
		switch(*pop) {

		case IPOPT_EOL:
			pop=peop;
			break;

		case IPOPT_NOP:
			pop++;
			break;

		case IPOPT_RR:
			if (prr == NULL) prr = pop; 
			else { bogus = TRUE; perr = prr; }
			pop += *(pop+1);
			break;

		case IPOPT_TS:
			if (pts == NULL) pts = pop; 
			else { bogus = TRUE; perr = pts; }
			pop += *(pop+1);
			break;

		case IPOPT_SEC:
			if (psec == NULL) psec = pop; 
			else { bogus = TRUE; perr = psec; }
			pop += *(pop+1);
			break;

		case IPOPT_LSRR:
			if (plsrr == NULL) plsrr = pop;
			else { bogus = TRUE; perr = plsrr; }
			pop += *(pop+1);
			break;

		case IPOPT_SSRR:
			if (pssrr == NULL) pssrr = pop;
			else { bogus = TRUE; perr = pssrr; }
			pop += *(pop+1);
			break;

		case IPOPT_SID:
			if (psid == NULL) psid = pop;
			else { bogus = TRUE; perr = psid; }
			pop += *(pop+1);
			break;

		default: pop++;

		}
		
	}

	/* Send an ICMP error message if any of the option is bogus */

	if (bogus == TRUE) {
		icmpreply(ICMPT_PARAM, ICMPC_POINT,
			((int) perr - (int) pip), pep);
		return SYSERR;
	}

	/* Send an ICMP error message if both loose and strict 
	   source route and record options are present */

	if (plsrr !=NULL && pssrr != NULL) {
		icmpreply(ICMPT_PARAM, ICMPC_POINT,
			((int) plsrr - (int) pip), pep);

		return SYSERR;
	}

	/* Process individual options */

	if (plsrr != NULL)
		if (optlsrr(pep, pip, plsrr) == SYSERR)
			return SYSERR;
	if (pssrr != NULL) 
		if (optssrr(pep, pip, pssrr)==SYSERR) 
			return SYSERR;
	if (prr != NULL)
		if (optrr(pep, pip, prr)==SYSERR)
			return SYSERR;
	if (pts)
		if (optts(pep, pip, pts, IPADDR_ANY) == SYSERR) 
			return SYSERR;

	/* Recompute IP header checksum */

	pip->ip_cksum = 0; 
	pip->ip_cksum = (cksum(pip, IP_HLEN(pip)));

	/* Set options done flag */

	pep->ep_flags |= EPF_OPTSDONE;

	return OK;
}

