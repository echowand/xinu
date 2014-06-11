/* tftp.h - definitions for a TFTP client */

#define	TFTP_PORT	69		/* UDP protocol port to use	*/



#define	TFTP_SERVER	"128.10.3.31"	/* TFTP server at Purdue CS	*/

#define	TFTP_MAXNAM	64		/* Max length of a file name	*/

#define	TFTP_MAXDATA	512		/* Max size of a data packet	*/

#define	TFTP_MAXRETRIES	3		/* Number of retranmissions	*/

#define	TFTP_WAIT	5000		/* Time to wait for reply (ms)	*/

/* TFTP opcodes */

#define	TFTP_RRQ	1		/* Read request */
#define	TFTP_WRQ	2		/* Write request */
#define	TFTP_DATA	3		/* Data packet */
#define	TFTP_ACK	4		/* ACK packet */
#define	TFTP_ERROR	5		/* Error indicator */


/* Format of a TFTP message (items following opcode depend on msg type)	*/

struct	tftp_msg {
	uint16		tf_opcode;	/* One of the opcodes above	*/
	union {

	 /* Items in a RRQ or WRQ message */

	 struct	{
	  char	tf_filemode[TFTP_MAXNAM+10]; /* file name and mode	*/
	 };

	 /* Items in a Data packet */

	 struct {
	  uint16	tf_dblk;	/* Block number of this data	*/
	  char		tf_data[TFTP_MAXDATA]; /* Actual data		*/
	 };

	 /* Items in an ACK packet */

	 struct {
	  uint16	tf_ablk;	/* Block number being acked	*/
	 };

	/* Items in an Error packet */

	 struct {
	  uint16	tf_ercode;	/* Integer error code		*/
	  char		tf_ermsg[TFTP_MAXDATA]; /* Error message	*/
	 };
	};
};
