/*{{{}}}*/
/*{{{  #includes*/
#include "screen.h"
/*}}}  */

/*{{{  bit_create -- create a bitmap as a sub-rectangle of another bitmap*/
BITMAP *bit_create(map, x, y, wide, high) BITMAP *map; int x, y, wide, high;
{
  register BITMAP *result;

  result=pr_region(map,x,y,wide,high);

#ifdef MOVIE
  if(result) log_create(map);
#endif

  return (result);
}
/*}}}  */
