#ifndef lint
static char rcsid[] =
    "@(#) $Header: cheapmoon.c,v 1.5 88/08/26 22:29:32 jef Exp $ (LBL)";
#endif

/*
** Copyright (C) 1988 by Jef Poskanzer and Craig Leres.
**
** Permission to use, copy, modify, and distribute this software and its
** documentation for any purpose and without fee is hereby granted, provided
** that the above copyright notice appear in all copies and that both that
** copyright notice and this permission notice appear in supporting
** documentation.  This software is provided "as is" without express or
** implied warranty.
*/


#include "smallmoon.xbm" 


void getbitmap(w, h, bits, cx, cy, r)
	int *w, *h;
	char **bits;
	int *cx, *cy, *r;
{
	*w = verycheapmoon_width;
	*h = verycheapmoon_height;
	*bits = verycheapmoon_bits;
	*cx = 288;
        *cy = 191;
	*r = 190;
}
