/* Copyright (c) 2014 Howard Chu, hyc@symas.com
 *                            All Rights Reserved
 *       Permission is granted to copy or use this program, EXCEPT that it
 *       may not be sold for profit, the copyright notice must be reproduced
 *       on copies, and credit should be given to the author where it is due.
 *       THE AUTHOR MAKES NO WARRANTY AND ACCEPTS NO LIABILITY FOR THIS PROGRAM.
 */

#include "bitmap.h"

int bit_colors[256];

/* return the color index in the color lookup table of the foreground */
unsigned int fg_color_idx(void)
{ return 1; }

void
getpalette(BITMAP *bp, unsigned int index, unsigned int *red,
	unsigned int *green, unsigned int *blue, unsigned int *maxi)
{
	if (bit_xinfo.fakemap) {
		int pixel;
		if (index > 255) return;
		pixel = bit_colors[index];
		*blue = pixel & 0xff;
		pixel >>= 8;
		*green = pixel & 0xff;
		pixel >>= 8;
		*red = pixel & 0xff;
		*maxi = 255;
	} else {
	Colormap cm = DefaultColormap(bit_xinfo.d, bit_xinfo.s);
	XColor xc;

	xc.pixel = index;
	XQueryColor(bit_xinfo.d, cm, &xc);
	*red = xc.red;
	*green = xc.green;
	*blue = xc.blue;
	*maxi = 65535;	/* max intensity per color */
	}
}

void
setpalette(BITMAP *bp, unsigned int index, unsigned int red,
	unsigned int green, unsigned int blue, unsigned int max)
{
	if (bit_xinfo.fakemap) {
		int tmp, pixel;
		if (index > 255) return;
		pixel = red * 255 /max;
		tmp = green * 255 / max;
		pixel <<= 8;
		pixel |= tmp;
		tmp = blue * 255 / max;
		pixel <<= 8;
		pixel |= tmp;
		bit_colors[index] = pixel;
	} else {
	XColor xc;

	xc.pixel = index;
	xc.red = red * 65535 / max;
	xc.green = green * 65535 / max;
	xc.blue = blue * 65535 / max;
	xc.flags = DoRed | DoGreen | DoBlue;
	XStoreColor(bit_xinfo.d, DefaultColormap(bit_xinfo.d, bit_xinfo.s), &xc);
	}
}
