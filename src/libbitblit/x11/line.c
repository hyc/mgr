/* Copyright (c) 2014 Howard Chu, hyc@symas.com
 *                            All Rights Reserved
 *       Permission is granted to copy or use this program, EXCEPT that it
 *       may not be sold for profit, the copyright notice must be reproduced
 *       on copies, and credit should be given to the author where it is due.
 *       THE AUTHOR MAKES NO WARRANTY AND ACCEPTS NO LIABILITY FOR THIS PROGRAM.
 */

/* Draw a line */

#include "bitmap.h"

void bit_line(dest, x0, y0, x1, y1, func)
BITMAP *dest;
int x0, y0, x1, y1;
int func;
   {
	xdinfo *xd;
	int fg, bg;

	fg = GETFCOLOR(func);
	bg = GETBCOLOR(func);
	if (bit_xinfo.fakemap) {
		fg = bit_colors[fg];
		bg = bit_colors[bg];
	}
	xd = dest->deviceinfo;
	if (!xd->d)
		bit_pixmap(dest);
	XSetState(bit_xinfo.d, bit_xinfo.gc, fg, bg, bit_ops[func&0xf], AllPlanes);
	x0 += dest->x0;
	y0 += dest->y0;
	x1 += dest->x0;
	y1 += dest->y0;
	XDrawLine(bit_xinfo.d, xd->d, bit_xinfo.gc, x0, y0, x1, y1);
	if (IS_SCREEN(dest)) {
	  XDrawLine(bit_xinfo.d, bit_xinfo.w, bit_xinfo.gc, x0, y0, x1, y1);
	}
   }
