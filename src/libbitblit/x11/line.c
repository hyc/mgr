/*                        Copyright (c) 1988 Bellcore
 *                            All Rights Reserved
 *       Permission is granted to copy or use this program, EXCEPT that it
 *       may not be sold for profit, the copyright notice must be reproduced
 *       on copies, and credit should be given to Bellcore where it is due.
 *       BELLCORE MAKES NO WARRANTY AND ACCEPTS NO LIABILITY FOR THIS PROGRAM.
 */
/*	$Header: /files/src/linuxmgr/src/libbitblit/stub_lib/RCS/line.c,v 4.3 1994/01/28 21:01:13 broman Stab $
	$Source: /files/src/linuxmgr/src/libbitblit/stub_lib/RCS/line.c,v $
*/
static char	RCSid_[] = "$Source: /files/src/linuxmgr/src/libbitblit/stub_lib/RCS/line.c,v $$Revision: 4.3 $";

/*  Draw a line stub */

#include "bitmap.h"

void bit_line(dest, x0, y0, x1, y1, func)
BITMAP *dest;
int x0, y0, x1, y1;
int func;
   {
	xdinfo *xd;
	GC gc = DefaultGC(bit_xinfo.d, bit_xinfo.s);
	int fg, bg;

	fg = GETFCOLOR(func);
	bg = GETBCOLOR(func);
	if (bit_xinfo.fakemap) {
		fg = bit_colors[fg];
		bg = bit_colors[bg];
	}
	xd = dest->deviceinfo;
	XSetState(bit_xinfo.d, gc, fg, bg, bit_ops[func&0xf], AllPlanes);
	XDrawLine(bit_xinfo.d, xd->d, gc, x0, y0, x1, y1);
   }
