/*	$Header: send.c,v 4.1 88/06/21 14:02:46 bianchi Exp $
	$Source: /tmp/mgrsrc/demo/msg/RCS/send.c,v $
*/
static char	RCSid_[] = "$Source: /tmp/mgrsrc/demo/msg/RCS/send.c,v $$Revision: 4.1 $";

/*                        Copyright (c) 1987 Bellcore
 *                            All Rights Reserved
 *       Permission is granted to copy or use this program, EXCEPT that it
 *       may not be sold for profit, the copyright notice must be reproduced
 *       on copies, and credit should be given to Bellcore where it is due.
 *       BELLCORE MAKES NO WARRANTY AND ACCEPTS NO LIABILITY FOR THIS PROGRAM.
 */
#include <mgr/mgr.h>

/* sent a message to a server */

main(argc,argv)
int argc;
char **argv;
   {
   char *dot, *index();

   ckmgrterm( *argv );

   if (argc < 2)
      exit (0);

   if (dot=index(argv[1],'.'))
      *dot = '\0';

   m_setup( M_FLUSH );
   m_sendto( atoi(argv[1]), "F $" );
   }
