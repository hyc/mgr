/*                        Copyright (c) 1987 Bellcore
 *                            All Rights Reserved
 *       Permission is granted to copy or use this program, EXCEPT that it
 *       may not be sold for profit, the copyright notice must be reproduced
 *       on copies, and credit should be given to Bellcore where it is due.
 *       BELLCORE MAKES NO WARRANTY AND ACCEPTS NO LIABILITY FOR THIS PROGRAM.
 */
/*	$Header: /files/src/linuxmgr/src/libbitblit/stub_lib/RCS/bitmap.c,v 4.3 1994/01/28 21:01:13 broman Stab $
	$Source: /files/src/linuxmgr/src/libbitblit/stub_lib/RCS/bitmap.c,v $
*/
static char	RCSid_[] = "$Source: /files/src/linuxmgr/src/libbitblit/stub_lib/RCS/bitmap.c,v $$Revision: 4.3 $";

/*  generic bitblit code routines*/

#include "bitmap.h"
#include <stdlib.h>
#include <X11/Xutil.h>

xinfo bit_xinfo;

/* open the display */

BITMAP *
bit_open(name)
char *name;			/* name of frame buffer */
{
   BITMAP *result;
   xdinfo *xd;
   XVisualInfo xvi;
   XSetWindowAttributes attrs;

   if ((result = (BITMAP *) malloc(sizeof(BITMAP)+sizeof(xdinfo))) == (BITMAP *) 0)
      return (BIT_NULL);

	/* do what you need to do here to initialize the display */

   bit_xinfo.d = XOpenDisplay(name);
   if (bit_xinfo.d == NULL) {
   	free(result);
	return BIT_NULL;
   }
   bit_xinfo.s = DefaultScreen(bit_xinfo.d);

   /* Look for a PseudoColor visual. If none exists, fake it */
   if (XMatchVisualInfo(bit_xinfo.d, bit_xinfo.s, 24, PseudoColor, &xvi)) {
   	bit_xinfo.c = XCreateColormap(bit_xinfo.d, RootWindow(bit_xinfo.d, bit_xinfo.s),xvi.visual, AllocAll);
   	attrs.colormap = bit_xinfo.c;
   	bit_xinfo.w = XCreateWindow(bit_xinfo.d, RootWindow(bit_xinfo.d, bit_xinfo.s),
   	0, 0, 1024, 768, 1, 24, InputOutput, xvi.visual, CWColormap, &attrs);
	bit_xinfo.fakemap = 0;
   } else {
    bit_xinfo.fakemap = 1;
   	bit_xinfo.w = XCreateSimpleWindow(bit_xinfo.d, RootWindow(bit_xinfo.d, bit_xinfo.s),
   	0, 0, 1024, 768, 1, BlackPixel(bit_xinfo.d, bit_xinfo.s),
	BlackPixel(bit_xinfo.d, bit_xinfo.s));
   }
   XSelectInput(bit_xinfo.d, bit_xinfo.w, ExposureMask | KeyPressMask);
   XMapWindow(bit_xinfo.d, bit_xinfo.w);
   XFlush(bit_xinfo.d);

   result->deviceinfo = result+1;
   result->primary = result;
   result->data = 0;
   result->x0 = 0,
   result->y0 = 0,
   result->wide = 1024;
   result->high = 768;
   result->type = _SCREEN;
   result->depth = 8 /* DefaultDepth(bit_xinfo.d, bit_xinfo.s) */;
   xd = result->deviceinfo;
   xd->d = bit_xinfo.w;
   return (result);
}

/* destroy a bitmap, free up space (might need special code for the display) */

void
bit_destroy(bitmap)
BITMAP *bitmap;
{
   if (bitmap == (BITMAP *) 0)
      return;
   if (IS_PRIMARY(bitmap)) {
   	  xdinfo *xd = bitmap->deviceinfo;
      if (IS_MEMORY(bitmap)) {
	    XFreePixmap(bit_xinfo.d, xd->d);
	  	free(bitmap->data);
	  }
	  else if (IS_SCREEN(bitmap)) {
	    XDestroyWindow(bit_xinfo.d, xd->d);
		XCloseDisplay(bit_xinfo.d);
	  }
   }
   free(bitmap);
}

/* create a bitmap as a sub-rectangle of another bitmap */

BITMAP *
bit_create(map, x, y, wide, high)
BITMAP *map;
int x, y, wide, high;
{
   register BITMAP *result;
   xdinfo *xd0, *xd1;

   if (x + wide > map->wide)
      wide = map->wide - x;
   if (y + high > map->high)
      high = map->high - y;
   if (wide < 1 || high < 1)
      return (BIT_NULL);

   if ((result = (BITMAP *) malloc(sizeof(BITMAP)+sizeof(xdinfo))) == (BITMAP *) 0)
      return (BIT_NULL);

   result->data = map->data;
   result->x0 = map->x0 + x;
   result->y0 = map->y0 + y;
   result->wide = wide;
   result->high = high;
   result->primary = map->primary;
   result->type = map->type;
   result->deviceinfo = result+1;
   if (map->deviceinfo) {
	 xd0 = map->deviceinfo;
     xd1 = result->deviceinfo;
     xd1->d = xd0->d;
   }
   return (result);
}

/* allocate space for, and create a memory bitmap */

BITMAP *
bit_alloc(wide, high, data, depth)
unsigned short wide, high;
DATA *data;
unsigned char depth;
{
   register BITMAP *result;
   xdinfo *xd;

   if ((result = (BITMAP *) malloc(sizeof(BITMAP)+sizeof(xdinfo))) == (BITMAP *) 0)
      return (result);

   result->x0 = 0;
   result->y0 = 0;
   result->high = high;
   result->wide = wide;
   result->depth = depth;
   result->deviceinfo = result+1;
   xd = result->deviceinfo;
   xd->d = 0;

   if (data != (DATA *) 0)
      result->data = data;
   else {
      xd->d = XCreatePixmap(bit_xinfo.d, bit_xinfo.s, wide, high, depth);
      return ((BITMAP *) 0);
   }

   result->primary = result;
   result->type = _MEMORY;
   return (result);
}

void bit_textscreen() {}
void bit_grafscreen() {}

int bit_on(bp, x, y)
	register BITMAP *bp;
	int x, y;
{
	XImage *img;
	xdinfo *xd;
	int ret;

	if (x < 0 || x >= BIT_WIDE(bp) || y < 0 || y >= BIT_HIGH(bp))
		return 0;
	xd = bp->deviceinfo;
	img = XGetImage(bit_xinfo.d, xd->d, x, y, 1, 1, AllPlanes, XYPixmap);
	if (!img)
		return 0;
	ret = XGetPixel(img, 0, 0);
	XDestroyImage(img);
	return ret;
}

int bit_size(int wide, int high, unsigned char depth)
{
	return BIT_Size(wide,high,depth);
}

/* shrink an 8-bit map to 1-bit. */
BITMAP *
bit_shrink(src_map, bg_color)
BITMAP *src_map;
int bg_color;	/* bg color is 0, everything else is 1 */
{
	BITMAP *map = src_map->primary;
	BITMAP *result = bit_alloc(BIT_WIDE(map),BIT_HIGH(map),NULL,1);
	return result;
}

