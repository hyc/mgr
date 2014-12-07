#include "bitmap.h"
#include <mgr/share.h>
#include <stdlib.h>

void (*display_close)(BITMAP *);

/*{{{  bit_destroy -- destroy a bitmap, free up space*/
void bit_destroy(bitmap) BITMAP *bitmap;
{
  if (bitmap==(BITMAP*)0) return;

  if (IS_PRIMARY(bitmap))
  {
    xdinfo *xd = bitmap->deviceinfo;
    /* destroy bitmap cache for primary bitmaps */
    if (bitmap->cache)
	{
      bit_destroy(bitmap->cache);
      bitmap->cache=NULL;
	}
    if (IS_MEMORY(bitmap))
    {
#ifdef MOVIE
      log_destroy(bitmap);
#endif
	  if (bitmap->data) {
        free(bitmap->data);
        bitmap->data=(DATA*)0;
	  } else {
	    XFreePixmap(bit_xinfo.d, xd->d);
	  }
	}
    else if (IS_SCREEN(bitmap))
	{
#ifdef MOVIE
      log_destroy(bitmap);
#endif
      display_close(bitmap);
	}
    /* dont free static bitmaps */
    if (IS_STATIC(bitmap)) return;
  }
  free(bitmap);
}
/*}}}  */
