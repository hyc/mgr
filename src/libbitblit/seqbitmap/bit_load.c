/*{{{}}}*/
/*{{{  #includes*/
#include "screen.h"
#include <mgr/mgr.h>
#include <stdlib.h>
#include <stdio.h>
/*}}}  */

/*{{{  bit_load*/
BITMAP *bit_load(int w, int h, unsigned char d, int size, unsigned char *src)
{
  /*{{{  variables*/
  BITMAP *bp;
  unsigned char *dst;
  int portable_size1=B_SIZE8(w,1,d);
  int bitmap_size1=bit_linesize(w,d);
  int height=h>(size/portable_size1) ? (size/portable_size1) : h;
  int y;
  /*}}}  */

  if ((bp=bit_alloc(w,h,0,d))==(BITMAP*)0) return (BITMAP*)0;
#ifdef MOVIE
  SET_DIRTY(bp);
#endif
  dst=(unsigned char *)BIT_DATA(bp);
  for (y=0; y<height; y++)
  {
    memcpy(dst,src,portable_size1);
    src+=portable_size1;
    dst+=bitmap_size1;
  }
  return bp;
}
/*}}}  */
