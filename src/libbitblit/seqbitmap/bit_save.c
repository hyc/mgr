/*{{{}}}*/
/*{{{  #includes*/
#include "screen.h"
#include <mgr/mgr.h>
#include <stdlib.h>
#include <stdio.h>
/*}}}  */

/*{{{  bit_save -- convert bitmap data to external format*/
void *bit_save(BITMAP *bp)
{
  int portable_size=B_SIZE8(BIT_WIDE(bp),1,BIT_DEPTH(bp));
  int bitmap_size=bit_linesize(BIT_WIDE(bp),BIT_DEPTH(bp));
  int i;
  char *r,*map;
  char *s=(char*)BIT_DATA(bp);

  if ((r=map=malloc(BIT_HIGH(bp)*portable_size))==(void*)0) return (void*)0;
  for (i=0; i<BIT_HIGH(bp); i++,r+=portable_size,s+=bitmap_size) memcpy(r,s,portable_size);
  return (void*)map;
}
/*}}}  */
