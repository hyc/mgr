/*                        Copyright (c) 1987 Bellcore
 *                            All Rights Reserved
 *       Permission is granted to copy or use this program, EXCEPT that it
 *       may not be sold for profit, the copyright notice must be reproduced
 *       on copies, and credit should be given to Bellcore where it is due.
 *       BELLCORE MAKES NO WARRANTY AND ACCEPTS NO LIABILITY FOR THIS PROGRAM.
 */
/*	$Header: /files/src/linuxmgr/src/libbitblit/stub_lib/RCS/blit.c,v 4.3 1994/01/28 21:01:13 broman Stab $
	$Source: /files/src/linuxmgr/src/libbitblit/stub_lib/RCS/blit.c,v $
*/
static char	RCSid_[] = "$Source: /files/src/linuxmgr/src/libbitblit/stub_lib/RCS/blit.c,v $$Revision: 4.3 $";

/*  stub bitblit code */

#include "bitmap.h"

/*
 *  General memory-to-memory rasterop
 */

mem_rop(dest, dx, dy, width, height, func, source, sx, sy)
int sx, sy, dx, dy;		/* properly clipped source and dest */
int width, height;		/* rectangle to be transferred */
BITMAP *source, *dest;		/* bit map pointers */
int func;			/* rasterop function */
   {
   }
