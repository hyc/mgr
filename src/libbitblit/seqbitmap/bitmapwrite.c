/*{{{}}}*/
/*{{{  #includes*/
#include "screen.h"
#include <mgr/mgr.h>
#include <stdlib.h>
#include <stdio.h>
/*}}}  */

/*{{{  bitmapwrite*/
/*
	Write a bitmap.
	Given an open FILE pointer to an file and a pointer to a bitmap,
	write the header and the bitmap.  Return 0 on failure, positive on
	success.
*/
int bitmapwrite(FILE *fp, BITMAP *bp)
{
  char *datap;
  int w,h;
  unsigned char d;
  struct b_header head;
  int sizefile1; /* the size of 1 line of the bitmap as stored in a file, in bytes */
  int sizemem1;	/* the size of 1 line of the bitmap as stored in memory, in bytes */

  w = BIT_WIDE(bp);
  h = BIT_HIGH(bp);
  d = BIT_DEPTH(bp);
  B_PUTHDR8( &head, w, h, d );
  sizefile1 = B_SIZE8(w, 1, d);
  if( fwrite( (char *)&head, sizeof head, 1, fp ) != 1 ) return  0;
  sizemem1 = bit_linesize(w, d);
  datap = (char *)BIT_DATA(bp);
  while( h-- > 0 ) 
  {
    if( fwrite( datap, sizefile1, 1, fp ) != 1 )
    return  0;
    datap += sizemem1;
  }
  return(1);
}
/*}}}  */
