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
    memcpy( dst, src, wide);
    dst += byteswide;
    src += byteswide;
  }
}
