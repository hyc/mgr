/*{{{}}}*/
/*{{{  #includes*/
#include <sys/types.h>
#include <stdlib.h>
#include <fcntl.h>
#ifdef USE_X11
#include "../x11/bitmap.h"
#include <X11/Xutil.h>
#endif
#include <mgr/bitblit.h>
#include <mgr/share.h>
/*}}}  */

/*{{{  bit_alloc -- allocate space for, and create a memory bitmap*/
BITMAP *bit_alloc(int wide, int high, DATA *data, unsigned char depth)
{
  register BITMAP *result;
#ifdef USE_X11
  xdinfo *xd;
#endif

#ifdef DEBUG
  if (wide<=0 || high <=0 || !(depth==8 || depth==1))
  {
    fprintf(stderr,"bit_alloc boo-boo %d x %d x %d\r\n",wide,high,depth);
    return(NULL);
  }
#endif
#ifdef USE_X11
  if ((result=(BITMAP*)malloc(sizeof(BITMAP)+sizeof(xdinfo)))==(BITMAP*)0) return (result);
  result->deviceinfo = result+1;
  xd = result->deviceinfo;
  xd->d = 0;
#else
  if ((result=(BITMAP*)malloc(sizeof(BITMAP)))==(BITMAP*)0) return (result);
  result->deviceinfo = NULL;
#endif

  result->x0=0;
  result->y0=0;
  result->high=high;
  result->wide=wide;
  result->depth=depth;
  result->cache=NULL;
  result->color=0;

  if (data != (DATA *) 0)
  {
    result->data = data;
    /* convert from external to internal format (if required) */
#ifdef MOVIE
    log_alloc(result);
#endif
  }
  else
  {
#ifdef USE_X11
	xd->d = XCreatePixmap(bit_xinfo.d, bit_xinfo.w, wide, high, bit_xinfo.depth);
	result->data = NULL;
#else
    register int size=bit_size(wide,high,depth);

    if ((result->data = (DATA *) malloc(size)) == (DATA *) 0)
    {
      free(result);
      return ((BITMAP *) 0);
    }
#endif
#ifdef MOVIE
  log_alloc(result);
#endif
  }

  result->primary = result;
  result->type = _MEMORY;
  result->id = 0;	/* assign elsewhere? */
  return (result);
}
/*}}}  */
