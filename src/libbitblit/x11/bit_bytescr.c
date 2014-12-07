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
