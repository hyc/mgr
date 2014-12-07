#include <X11/Xlib.h>

typedef struct xinfo {
	Display *d;
	int s;
	int fd;
	Window w;
	Colormap c;
	int fakemap;
} xinfo;

extern xinfo bit_xinfo;

typedef struct xdinfo {
	Drawable d;
} xdinfo;

