#include <string.h>

#include "screen.h"

#ifndef FB_AD /* for NEED_ADJUST code */
#define FB_AD(bp,pp) (pp)
#endif

#define BYTESWIDE(x)	((x->primary->wide+7)>>3)

void bit_bytescroll(map,x,y,wide,high,delta) BITMAP *map; int x,y,wide,high,delta;
{
  register DATA *dst = BIT_DATA(map) + y*BYTESWIDE(map) + (x>>3);
  register DATA *src = dst + delta*BYTESWIDE(map);
  register long count = (wide+(x&7))>>3;
  register long skip = BYTESWIDE(map)-count;
  register long ncount = high - delta;

# ifdef MOVIE
  log_bytescroll(map,x,y,wide,high,delta);
# endif

  while (ncount--)
  {
    memcpy( FB_AD(map,dst), FB_AD(map,src), count);
    dst+=(count+skip);
    src+=(count+skip);
  }
}
/*{{{}}}*/
