/*                        Copyright (c) 1988 Bellcore
 *                            All Rights Reserved
 *       Permission is granted to copy or use this program, EXCEPT that it
 *       may not be sold for profit, the copyright notice must be reproduced
 *       on copies, and credit should be given to Bellcore where it is due.
 *       BELLCORE MAKES NO WARRANTY AND ACCEPTS NO LIABILITY FOR THIS PROGRAM.
 */
/*	$Header: /files/src/linuxmgr/src/libbitblit/stub_lib/RCS/pixel.c,v 4.3 1994/01/28 21:01:13 broman Stab $
	$Source: /files/src/linuxmgr/src/libbitblit/stub_lib/RCS/pixel.c,v $
*/
static char	RCSid_[] = "$Source: /files/src/linuxmgr/src/libbitblit/stub_lib/RCS/pixel.c,v $$Revision: 4.3 $";

/* draw a point  stub */

#include "bitmap.h"

int
bit_point(map, dx, dy, func)
BITMAP *map;
register int dx, dy;
int func;
   {
	xdinfo *xd;
	int fg;

	fg = GETFCOLOR(func);
	if (bit_xinfo.fakemap) {
		fg = bit_colors[fg];
	}
	xd = map->deviceinfo;
	XSetForeground(bit_xinfo.d, bit_xinfo.gc, fg);
	XSetFunction(bit_xinfo.d, bit_xinfo.gc, bit_ops[func&0xf]);
	XDrawPoint(bit_xinfo.d, xd->d, bit_xinfo.gc, dx, dy);
	if (IS_SCREEN(map))
		XDrawPoint(bit_xinfo.d, bit_xinfo.w, bit_xinfo.gc, dx, dy);
		
	return 0;
   }
