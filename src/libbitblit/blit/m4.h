/*                        Copyright (c) 1988 Bellcore
 *                            All Rights Reserved
 *       Permission is granted to copy or use this program, EXCEPT that it
 *       may not be sold for profit, the copyright notice must be reproduced
 *       on copies, and credit should be given to Bellcore where it is due.
 *       BELLCORE MAKES NO WARRANTY AND ACCEPTS NO LIABILITY FOR THIS PROGRAM.
 */
/*	$Header: m4.h,v 4.1 88/06/21 13:19:12 bianchi Exp $
	$Source: /tmp/mgrsrc/src/blit/RCS/m4.h,v $
*/
static char	h_m4_[] = "$Source: /tmp/mgrsrc/src/blit/RCS/m4.h,v $$Revision: 4.1 $";

/*
 * asm argument modifiers:  need to use m4  as cpp doesn't substitute
 * into quoted strings
 */

define(INCR, $1`@+')
define(DECR, $1`@-')
define(IMM, `#'$1)
define(IND, $1`@')
define(T_SRC,d0)
define(T_DST,d1)
