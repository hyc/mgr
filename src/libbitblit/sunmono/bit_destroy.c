/*{{{}}}*/
/*{{{  #includes*/
#include <mgr/share.h>
#include <stdlib.h>
#include "screen.h"
/*}}}  */

/*{{{  bit_destroy -- destroy a bitmap, free up space*/
void bit_destroy(bitmap) BITMAP *bitmap;
{
  if (bitmap==(BITMAP*)0) return;

  /* destroy bitmap cache for primary bitmaps */
  if (IS_PRIMARY(bitmap) && bitmap->cache)
  {
    bit_destroy(bitmap->cache);
    bitmap->cache=NULL;
  }

  if (IS_MEMORY(bitmap) && IS_PRIMARY(bitmap))
  {
#ifdef MOVIE
    log_destroy(bitmap);
#endif
    free(bitmap->data);
    bitmap->data=(DATA*)0;
  }
  else if (IS_SCREEN(bitmap) && IS_PRIMARY(bitmap))
  {
#ifdef MOVIE
    log_destroy(bitmap);
#endif
    display_close(bitmap);
  }

  /* dont free static bitmaps */
  if (IS_STATIC(bitmap) && IS_PRIMARY(bitmap)) return;
  free(bitmap);
}
/*}}}  */
