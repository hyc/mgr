/* Xlib interface */

#include "bitmap.h"
#include <stdlib.h>
#include <stdio.h>
#include <X11/Xutil.h>

xinfo bit_xinfo;

extern void (*_bit_freedisplay)(BITMAP *);
extern void (*_bit_freepixmap)(xdinfo *);

static void _bit_destroy(BITMAP *map)
{
  XFreePixmap(bit_xinfo.d, bit_xinfo.p);
  XDestroyWindow(bit_xinfo.d, bit_xinfo.w);
  XCloseDisplay(bit_xinfo.d);
}

static void _bit_freepix(xdinfo *xd)
{
  XFreePixmap(bit_xinfo.d, xd->d);
}

static int _bit_errhandl(Display *d, XErrorEvent *err)
{
	char errbuf[1024];
	XGetErrorText(d, err->error_code, errbuf, sizeof(errbuf));
	fprintf(stderr,"Err on req #%lu code %d: %s\n", err->serial, err->error_code, errbuf);
	fprintf(stderr,"req code %d / %d, resource %lx\n",
		err->request_code, err->minor_code, err->resourceid);
	return 0;
}

/* setup the display */

DATA *
bit_initscreen(char *name, int *width, int *height, unsigned char *depth,
	void **devi)
{
   XVisualInfo xvi;
   XSetWindowAttributes attrs;

   bit_xinfo.d = XOpenDisplay(name);
   if (bit_xinfo.d == NULL) return BIT_NULL;
   bit_xinfo.s = DefaultScreen(bit_xinfo.d);
   bit_xinfo.fd = ConnectionNumber(bit_xinfo.d);

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

   /* Pixmap for our window contents */
   {
   XWindowAttributes wa;
   XGetWindowAttributes(bit_xinfo.d, bit_xinfo.w, &wa);
   bit_xinfo.depth = wa.depth;
   bit_xinfo.p = XCreatePixmap(bit_xinfo.d, bit_xinfo.w, wa.width, wa.height, wa.depth);
   bit_xinfo.gc = XCreateGC(bit_xinfo.d, bit_xinfo.w, 0, NULL);
   XCopyGC(bit_xinfo.d, DefaultGC(bit_xinfo.d, bit_xinfo.s), 0xffffff, bit_xinfo.gc);
   }

   XSetErrorHandler(_bit_errhandl);

   /* Hide the X cursor */
   {
	Cursor invis;
	Pixmap bmnull;
	XColor black;
	static char empty[] = {0,0,0,0,0,0,0,0};
	black.red = black.green = black.blue = 0;
	bmnull = XCreateBitmapFromData(bit_xinfo.d, bit_xinfo.w, empty, 8, 8);
	invis = XCreatePixmapCursor(bit_xinfo.d, bmnull, bmnull, &black, &black, 0, 0);
	XDefineCursor(bit_xinfo.d, bit_xinfo.w, invis);
	XFreeCursor(bit_xinfo.d, invis);
   }
   XSelectInput(bit_xinfo.d, bit_xinfo.w, ExposureMask | KeyPressMask|ButtonPressMask|ButtonReleaseMask|PointerMotionMask);
   XMapWindow(bit_xinfo.d, bit_xinfo.w);
   XFlush(bit_xinfo.d);

   *width = 1024;
   *height = 768;
   *depth = 8;

   _bit_freedisplay = _bit_destroy;
   _bit_freepixmap = _bit_freepix;
   return NULL;
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
	img = XGetImage(bit_xinfo.d, xd->d, x+bp->x0, y+bp->y0, 1, 1, AllPlanes, XYPixmap);
	if (!img)
		return 0;
	ret = XGetPixel(img, 0, 0);
	XDestroyImage(img);
	return ret;
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

