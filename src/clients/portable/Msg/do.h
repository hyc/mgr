/*	$Header: do.h,v 4.1 88/06/21 14:02:42 bianchi Exp $
	$Source: /tmp/mgrsrc/demo/msg/RCS/do.h,v $
*/
static char	h_do_[] = "$Source: /tmp/mgrsrc/demo/msg/RCS/do.h,v $$Revision: 4.1 $";

/*                        Copyright (c) 1987 Bellcore
 *                            All Rights Reserved
 *       Permission is granted to copy or use this program, EXCEPT that it
 *       may not be sold for profit, the copyright notice must be reproduced
 *       on copies, and credit should be given to Bellcore where it is due.
 *       BELLCORE MAKES NO WARRANTY AND ACCEPTS NO LIABILITY FOR THIS PROGRAM.
 */
/* simple server-client message protocol */

/* server messages */

#define S_HI	'H'		/* announce existence  (broadcast)*/
#define S_BYE	'B'		/* announce demise     (broadcast)*/
#define S_REPLY	'R'		/* reply to message    (point-to-point) */

/* types of server replies */

#define R_WHO	'?'		/* server identification */
#define R_RCVD	'r'		/* received command */
#define R_DO	'.'		/* working on command */
#define R_DONE	'd'		/* command completed */
#define R_HOST	'h'		/* rejected, no remote requests accepted */
#define R_NET	'n'		/* Can't get file for remote request */
#define R_BAD	'x'		/* command rejected */
#define R_UNKWN	'u'		/* unknown command */

/* client messages */

#define C_WHO	'?'		/* who is out there  (broadcast) */
#define C_DO	'F'		/* do 'file' command (next arg is file name) */

#define C_SNARF	'$'		/* use snarf buffer for file name */
#define HOST	':'		/* file name prefixed with hostname */
