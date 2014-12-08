/* Copyright (c) 2014 Howard Chu, hyc@symas.com
 *                            All Rights Reserved
 *       Permission is granted to copy or use this program, EXCEPT that it
 *       may not be sold for profit, the copyright notice must be reproduced
 *       on copies, and credit should be given to the author where it is due.
 *       THE AUTHOR MAKES NO WARRANTY AND ACCEPTS NO LIABILITY FOR THIS PROGRAM.
 */

/* draw a point */

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
	if (!xd->d)
		bit_pixmap(map);
	XSetForeground(bit_xinfo.d, bit_xinfo.gc, fg);
	XSetFunction(bit_xinfo.d, bit_xinfo.gc, bit_ops[func&0xf]);
	dx += map->x0;
	dy += map->y0;
	XDrawPoint(bit_xinfo.d, xd->d, bit_xinfo.gc, dx, dy);
	if (IS_SCREEN(map))
		XDrawPoint(bit_xinfo.d, bit_xinfo.w, bit_xinfo.gc, dx, dy);
		
	return 0;
   }
