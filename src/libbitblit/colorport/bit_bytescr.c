#include <string.h>
#include "screen.h"

void bit_bytescroll(map,x,y,wide,high,delta)
     BITMAP *map;
     int x,y,wide,high,delta;
{
  long int byteswide = map->primary->wide;
  char *dst = ((char *)(map->data)) + y*byteswide + x;
  char *src = dst + delta*byteswide;
  long int ncount = high - delta;

# ifdef MOVIE
  log_bytescroll(map,x,y,wide,high,delta);
# endif

  while (ncount--) {
#ifndef BANKED
    memcpy( dst, src, wide);
#else /* banked vga frame buffer */
    DATA *srcb = getdata( src, BIT_DATA(map), wide);
    unsigned int left, wideb = wide;
    DATA *dstb = getdatap( dst, BIT_DATA(map), &wideb);
    memcpy( dstb, srcb, wideb);
    left = wide - wideb;
    if( left) {
      dstb = getdatap( dst+wideb, BIT_DATA(map), NULL);
      memcpy( dstb, srcb+wideb, left);
    }
#endif
    dst += byteswide;
    src += byteswide;
  }
}
