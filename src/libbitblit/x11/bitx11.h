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

