#include "screen.h"

int
bit_point(map, x, y, func)
register BITMAP *map;			/* destination maskmap */	
int x, y;				/* point coordinates */
int func;				/* set, clear, or invert  + color */
{
#ifndef NOCLIP
   if (x<0 || x>BIT_WIDE(map) || y<0 || y>BIT_HIGH(map))
      return(0);
#endif

#ifdef MOVIE
  log_point(map,x,y,func);
#endif

#ifdef INVERT
   func = rop_invert(func);
#endif

   return pr_vector(map,x,y,x,y,PIXRECT_OP(func),0);
}
