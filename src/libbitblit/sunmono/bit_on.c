#include "screen.h"

/*
    Return the nonnegative integer value of the pixel at x,y in bitmap bp,
    or zero if x,y is outside the bitmap.
    For monochrome bitmaps, the value is either 0 or 1.
 */

int bit_on( bp, x, y ) register BITMAP	*bp; int x, y;
{
	DATA pix, *dp;

	if( x < 0 || x >= BIT_WIDE(bp) || y < 0 ||  y >= BIT_HIGH(bp) )
		return  0;
	dp = BIT_DATA(bp) + y * BIT_LINE(bp) + (x >> LOGBITS);
#if DOFLIP
	pix = GETMSB( *dp, (x & BITS));
#else
	pix = GETLSB( *dp, BITS - (x & BITS));
#endif
	return pix & 1;
}
