/*{{{}}}*/
/*{{{  #includes*/
#include "screen.h"
/*}}}  */

/*{{{  bit_destroy -- destroy a bitmap, free up space*/
void bit_destroy(bitmap) BITMAP *bitmap;
{
#ifdef MOVIE
  if(bitmap) log_destroy(bitmap);
#endif
  pr_destroy(bitmap);
}
/*}}}  */
