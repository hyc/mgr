#include "screen.h"

/*
    Return the nonnegative integer value of the pixel at x,y in bitmap bp,
    or zero if x,y is outside the bitmap.
    For monochrome bitmaps, the value is either 0 or 1.
    For color bitmaps, all bits of the pixel value are returned.
    If the background color is not zero, then testing ON vs. OFF
    would require a comparison against this background color,
    something that is not available to this function.
 */

int bit_on( bp, x, y ) register BITMAP	*bp; int x, y;
{

	if( x < 0 || x >= BIT_WIDE(bp) || y < 0 ||  y >= BIT_HIGH(bp) )
		return  0;
	if( BIT_DEPTH(bp) == 1) {
		/* actually same as color case, but optimized by hand */
		DATA *dp = BIT_DATA(bp) + y * BIT_LINE(bp) + (x >> LOGBITS);
		DATA pix;

#ifdef BANKED
		if( IS_SCREEN( bp))
			dp = getdatap( dp, BIT_DATA(bp), NULL);
#endif
#if DOFLIP
		pix = GETMSB( *dp, (x & BITS));
#else
		pix = GETLSB( *dp, BITS - (x & BITS));
#endif
		return pix & 1;
	} else { /* color */
		unsigned int bitx = (x * BIT_DEPTH(bp));
		DATA *dp = BIT_DATA(bp) + y * BIT_LINE(bp) + (bitx >> LOGBITS);
		DATA pix;

#ifdef BANKED
		if( IS_SCREEN( bp))
			dp = getdatap( dp, BIT_DATA(bp), NULL);
#endif
#if DOFLIP
		pix = GETMSB( *dp, (bitx & BITS));
#else
		pix = GETLSB( *dp, BITS + 1 - BIT_DEPTH(bp) - (bitx & BITS));
#endif
		return pix & ~(((DATA)~0) << BIT_DEPTH(bp));
	}
}
