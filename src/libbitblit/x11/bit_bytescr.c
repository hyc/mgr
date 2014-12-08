/* Copyright (c) 2014 Howard Chu, hyc@symas.com
 *                            All Rights Reserved
 *       Permission is granted to copy or use this program, EXCEPT that it
 *       may not be sold for profit, the copyright notice must be reproduced
 *       on copies, and credit should be given to the author where it is due.
 *       THE AUTHOR MAKES NO WARRANTY AND ACCEPTS NO LIABILITY FOR THIS PROGRAM.
 */

#include "bitmap.h"
#include <mgr/share.h>
#include <stdlib.h>

/*{{{  bit_bytescroll -- scroll a byte-aligned region */
void bit_bytescroll(map,x,y,wide,high,delta)
	BITMAP *map;
	int x,y,wide,high,delta;
{
	long byteswide = map->primary->wide;
	char *dst = ((char *)(map->data)) + y*byteswide + x;
	char *src = dst + delta*byteswide;
	long ncount = high - delta;

#ifdef MOVIE
	log_bytescroll(map,x,y,wide,high,delta);
#endif
	while (ncount--) {
		memcpy(dst, src, wide);
		dst += byteswide;
		src += byteswide;
	}
}
/*}}}  */
