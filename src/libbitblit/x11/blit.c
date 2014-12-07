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
	XSetState(bit_xinfo.d, bit_xinfo.gc, fg, bg, bit_ops[func&0xf], AllPlanes);
	if (src) {
	  sx += src->x0;
	  sy += src->y0;
	  xds = src->deviceinfo;
	  if (xds) {
		  d = xds->d;
	  } else {
		XImage img;
		img.width = BIT_WIDE(src->primary);
		img.height = BIT_HIGH(src->primary);
		img.xoffset = 0;
		img.format = (src->depth == 1) ? XYBitmap : XYPixmap;
		img.data = src->data;
		img.byte_order =  MSBFirst;
		img.bitmap_bit_order = MSBFirst;
		img.bitmap_unit = 32;
		img.bitmap_pad = 8;
		img.depth = src->depth;
		img.bits_per_pixel = src->depth;
		img.bytes_per_line = 0;
		img.red_mask = 0xff0000;
		img.green_mask = 0xff00;
		img.blue_mask = 0xff;
		img.obdata = 0;
		XInitImage(&img);
		XPutImage(bit_xinfo.d, xdd->d, bit_xinfo.gc, &img, sx, sy, dx, dy, width, height);
		goto done;
	  }
	} else {
	  d = xdd->d;
	  /* Doing a clear op, fill to BG color */
	  if (!(func & 0xf) && IS_SCREEN(dst)) {
		XSetWindowBackground(bit_xinfo.d, bit_xinfo.w, bg);
		XClearArea(bit_xinfo.d, bit_xinfo.w, dx, dy, width, height, 0);
/*		XSetState(bit_xinfo.d, bit_xinfo.gc, bg, bg, GXset, AllPlanes);
		XFillRectangle(bit_xinfo.d, d, bit_xinfo.gc, dx, dy, width, height); */
		return;
	  }
	}
	XCopyArea(bit_xinfo.d, d, xdd->d, bit_xinfo.gc, sx, sy, width, height, dx, dy);
done:
	if (IS_SCREEN(dst)) {
		XSetState(bit_xinfo.d, bit_xinfo.gc, fg, bg, GXcopy, AllPlanes);
		XCopyArea(bit_xinfo.d, xdd->d, bit_xinfo.w, bit_xinfo.gc, dx, dy, width, height, dx, dy);
	}
}
