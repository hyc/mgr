/* Copyright (c) 2014 Howard Chu, hyc@symas.com
 *                            All Rights Reserved
 *       Permission is granted to copy or use this program, EXCEPT that it
 *       may not be sold for profit, the copyright notice must be reproduced
 *       on copies, and credit should be given to the author where it is due.
 *       THE AUTHOR MAKES NO WARRANTY AND ACCEPTS NO LIABILITY FOR THIS PROGRAM.
 */
#include <X11/Xlib.h>

typedef struct xinfo {
	Display *d;
	int s;
	int fd;
	Window w;
	Pixmap p;
	Colormap c;
	GC gc;
	int depth;
	int fakemap;
} xinfo;

extern xinfo bit_xinfo;

typedef struct xdinfo {
	Drawable d;
} xdinfo;

