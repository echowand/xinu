struct ipif {					/* ip interface struct	*/
	IPaddr ipif_addr;			/* if IP address	*/
	IPaddr ipif_mask;			/* if IP mask		*/
	IPaddr ipif_net;			/* if IP network 	*/
	IPaddr ipif_bcast;			/* if IP net broadcast	*/
};

extern struct ipif ipif[];			/* IP interface tbl decl*/
