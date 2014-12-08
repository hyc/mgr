/*{{{}}}*/
/*{{{  #includes*/
#ifdef USE_X11
#include "../x11/bitmap.h"
#endif
#include <mgr/bitblit.h>
#include <mgr/share.h>
#include <stdlib.h>
/*}}}  */

/*{{{  bit_create -- create a bitmap as a sub-rectangle of another bitmap*/
BITMAP *bit_create(map, x, y, wide, high) BITMAP *map; int x, y, wide, high;
{
  register BITMAP *result;

  if (x + wide > map->wide) wide = map->wide - x;
  if (y + high > map->high) high = map->high - y;
  if (wide < 1 || high < 1) return (BITMAP*)0;

#ifdef USE_X11
  if ((result=(BITMAP*)malloc(sizeof(BITMAP)+sizeof(xdinfo)))==(BITMAP*)0) return (BITMAP*)0;
  if (map->deviceinfo) {
    xdinfo *xd0, *xd1;
    result->deviceinfo = result+1;
    xd0 = map->deviceinfo;
	xd1 = result->deviceinfo;
	xd1->d = xd0->d;
  } else {
    result->deviceinfo = NULL;
  }
#else
  if ((result=(BITMAP*)malloc(sizeof(BITMAP)))==(BITMAP*)0) return (BITMAP*)0;
  result->deviceinfo = map->deviceinfo;
#endif

  result->data = map->data;
  result->x0 = map->x0 + x;
  result->y0 = map->y0 + y;
  result->wide = wide;
  result->high = high;
  result->depth = map->depth;
  result->primary = map->primary;
  result->cache = NULL;
  result->color = 0;
#ifdef MOVIE
  log_create(map);
#endif
  result->id = map->id;
  result->type = map->type;
  return (result);
}
/*}}}  */
