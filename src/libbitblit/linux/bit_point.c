/*
 * Draw a point at position (dx, dy) in 'map' using bitblit function 'func'.
 */

/*{{{}}}*/
/*{{{  #includes*/
#include "screen.h"
#include <mgr/share.h>

#include "do.h"
/*}}}  */
#ifndef FB_AD /* for NEED_ADJUST code */
#define FB_AD(bp,pp) (pp)
#endif


int bit_point(map, dx, dy, func)
BITMAP *map;
register int dx, dy;
int func;
{
  register DATA *dst;
  int x, y, mask;

  if (dx < 0 || dy < 0 || dx >= BIT_WIDE(map) || dy >= BIT_HIGH(map))
	return(-1);
  x = BIT_X(map) + dx;
  y = BIT_Y(map) + dy;
#ifdef MOVIE
  log_point(map,x,y,func);
#endif
  mask = 0x80 >> (x & 7);
  dst = BIT_DATA(map) + y * BIT_LINE(map) + (x >> 3);
  _do_mask( FB_AD(map,dst), mask, OPCODE(func));
  return ((*FB_AD(map,dst)) & mask);
}
