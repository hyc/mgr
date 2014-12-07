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

/* MGR bit ops are bit reversed from Xlib */

const unsigned char bit_ops[16] = {
	GXclear,		/* 0000 BIT_CLR */
	GXnor,			/* 0001 */
	GXandInverted,	/* 0010 BIT_NAND */
	GXcopyInverted,	/* 0011 */
	GXandReverse,	/* 0100 */
	GXinvert,		/* 0101 BIT_INV */
	GXxor,			/* 0110 BIT_XOR */
	GXnand,			/* 0111 */
	GXand,			/* 1000 BIT_AND */
	GXequiv,		/* 1001 */
	GXnoop,			/* 1010 BIT_DST */
	GXorInverted,	/* 1011 BIT_NOR */
	GXcopy,			/* 1100 BIT_SRC */
	GXorReverse,	/* 1101 */
	GXor,			/* 1110 BIT_OR */
	GXset			/* 1111 BIT_SET */
};

/*
 *  General memory-to-memory rasterop
 */

void bit_blit(dst, dx, dy, width, height, func, src, sx, sy)
int sx, sy, dx, dy;		/* properly clipped source and dest */
int width, height;		/* rectangle to be transferred */
BITMAP *src, *dst;		/* bit map pointers */
int func;			/* rasterop function */
{
	xdinfo *xds, *xdd;
	GC gc = DefaultGC(bit_xinfo.d, bit_xinfo.s);
	int fg, bg;
	Drawable d;

	if (!dst) return;

	fg = GETFCOLOR(func);
	bg = GETBCOLOR(func);
	if (bit_xinfo.fakemap) {
		fg = bit_colors[fg];
		bg = bit_colors[bg];
	}
	xdd = dst->deviceinfo;
	dx += dst->x0;
	dy += dst->y0;
	if (src) {
	  xds = src->deviceinfo;
	  if (xds) {
		  d = xds->d;
	  } else {
	  	XImage *img = XCreateImage(bit_xinfo.d, DefaultVisual(bit_xinfo.d,
		bit_xinfo.s), src->depth, src->depth == 1 ? XYBitmap : XYPixmap,
		0, src->data, BIT_WIDE(src), BIT_HIGH(src), 8, 0);
		XSetState(bit_xinfo.d, gc, fg, bg, bit_ops[func&0xf], AllPlanes);
		img->byte_order =  MSBFirst;
		img->bitmap_bit_order = MSBFirst;
		XPutImage(bit_xinfo.d, xdd->d, gc, img, sx, sy, dx, dy, width, height);
		img->data = NULL;
		XDestroyImage(img);
		return;
	  }
	} else {
	  d = xdd->d;
	  /* Doing a clear op, fill to BG color */
	  if (!(func & 0xf)) {
	    XSetWindowBackground(bit_xinfo.d, d, bg);
	  	XClearArea(bit_xinfo.d, d, dx, dy, width, height, 0);
		return;
	  }
	}
	XSetState(bit_xinfo.d, gc, fg, bg, bit_ops[func&0xf], AllPlanes);
	XCopyArea(bit_xinfo.d, d, xdd->d, gc, sx, sy, width, height, dx, dy);
}
