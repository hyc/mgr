/* SUN dependent part of the generic color code for palettes */

#include "sun.h"

#ifdef PIXRECT
#include <sys/types.h>
#include <pixrect/pixrect_hs.h>
#endif

/* returns the color index in the color lookup table of the foreground */
unsigned int fg_color_idx( void){ return 255;}

/* rescale computes a color index from the range [0..newmax]
   corresponding to a color index supplied in the range [0..oldmax].
   0 maps to 0 and oldmax maps to newmax.
   The expanding rescaling produces nearly equispaced outputs,
   and contracting rescaling maps nearly equal intervals to a single output.
   A rescaling followed by the reverse rescaling is a projection mapping.
 */
unsigned int rescale( unsigned int col, unsigned int oldmax,
					unsigned int newmax) {
    unsigned int newcol;

    if( newmax >= oldmax)
	newcol = oldmax? (col*2*newmax + oldmax) / (2*oldmax): newmax;
    else
	newcol = (col*2*(newmax + 1) + newmax) / (2*(oldmax + 1));
    return newcol;
}

void
setpalette(BITMAP *bp, unsigned int index, unsigned int red,
		unsigned int green, unsigned int blue, unsigned int maxi)
{
#ifdef PIXRECT
    if( bp->deviceinfo) {
	struct pixrect *pixrect_screen = (struct pixrect *)bp->deviceinfo;
	unsigned char r = rescale( red, maxi, 255);
	unsigned char g = rescale( green, maxi, 255);
	unsigned char b = rescale( blue, maxi, 255);

	pr_putcolormap( pixrect_screen, index, 1, &r, &g, &b);
    }
#endif
}

void
getpalette(BITMAP *bp, unsigned int index, unsigned int *red,
		unsigned int *green, unsigned int *blue, unsigned int *maxi)
{
#ifdef PIXRECT
    if( bp->deviceinfo) {
	struct pixrect *pixrect_screen = (struct pixrect *)bp->deviceinfo;
	unsigned char r, g, b;

	pr_getcolormap( pixrect_screen, index, 1, &r, &g, &b);
	*red = r;
	*green = g;
	*blue = b;
	*maxi = 255;
	return;
    }
#endif
    *red = *green = *blue = 0;
    *maxi = 1;
}
