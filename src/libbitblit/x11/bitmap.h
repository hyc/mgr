#include <X11/Xlib.h>

/* header file stub */

#ifndef Min
#define Min(x,y)	((x)>(y)?y:x)
#endif

typedef struct xinfo {
	Display *d;
	int s;
	Window w;
	Colormap c;
	int fakemap;
} xinfo;

extern xinfo bit_xinfo;
extern const unsigned char bit_ops[16];
extern int bit_colors[256];

typedef struct xdinfo {
	Drawable d;
} xdinfo;

#define	DATA	unsigned char

#include <mgr/bitblit.h>

/* bit mask for bitmap data padding */

#define BITS	15L

/* BIT_SIZE(map) == the number of chars needed to store the data of the bitmap.
   Usually used with malloc(3).
*/

#define BIT_SIZE(m)     BIT_Size(BIT_WIDE(m), BIT_HIGH(m), BIT_DEPTH(m)) /* bytes */

/* BIT_Size(wide,high,depth) = the number of chars needed to store the data of a
   bitmap of the given number of bits wide high and deep.
	   Typical usage:
		char	bitbuffer[ Bit_Size(16,16,1) ];
*/

#define BIT_Size(wide,high,d)     ((d)*((wide+BITS)&~BITS)*high>>3) /* bytes */

#define BIT_LINE(x)	((x->primary->wide+BITS)>>4) /* short aligned (shorts) */
