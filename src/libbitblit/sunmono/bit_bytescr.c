#include <string.h>

#include "screen.h"

#define BYTESWIDE(x)	((x->primary->wide+7)>>3)

void bit_bytescroll(map,x,y,wide,high,delta)
     BITMAP *map;
     int x,y,wide,high,delta;
{
  register char *dst = (unsigned char *) ((long)(map->data)+(y*BYTESWIDE(map) + (x>>3)));
  register char *src = dst + delta*BYTESWIDE(map);
  register long count = (wide+(x&7))>>3;
  register long skip = BYTESWIDE(map)-count;
  register long ncount = high - delta;

# ifdef MOVIE
  log_bytescroll(map,x,y,wide,high,delta);
# endif
  while (ncount--) {
    memcpy(dst,src,count);
    dst+=(count+skip);
    src+=(count+skip);
  }
}
