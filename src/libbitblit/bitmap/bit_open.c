/*{{{}}}*/
/*{{{  #includes*/
#ifdef USE_X11
#include "../x11/bitmap.h"
#include <X11/Xutil.h>
#endif
#include <mgr/bitblit.h>
#include <mgr/share.h>
#include <stdlib.h>
/*}}}  */

/*{{{  bit_open.c*/
BITMAP *bit_open(char *name)
{
  BITMAP *result = (BITMAP*)0;

#ifdef USE_X11
  if ((result=malloc(sizeof(BITMAP)+sizeof(xdinfo)))==(BITMAP*)0) return (BITMAP*)0;
  result->deviceinfo = result+1;
#else
  if ((result=malloc(sizeof(BITMAP)))==(BITMAP*)0) return (BITMAP*)0;
#endif
  result->primary = result;
  result->data = bit_initscreen(name,&(result->wide),&(result->high),
				&(result->depth),&(result->deviceinfo));
#ifdef USE_X11
  {
    xdinfo *xd = result->deviceinfo;
	xd->d = bit_xinfo.p;
  }
#endif
  result->x0 = 0;
  result->y0 = 0;
  result->type = _SCREEN;
  result->cache = 0;
  result->color = 0;
  result->id = 0;	/* set elsewhere? */
# ifdef MOVIE
  log_open(result);
# endif
  return result;
}
/*}}}  */
