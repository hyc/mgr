/*{{{}}}*/
/*{{{  #includes*/
#include "screen.h"
/*}}}  */

/*{{{  bit_alloc -- allocate space for, and create a memory bitmap*/
BITMAP *bit_alloc(int wide, int high, DATA *data, unsigned char depth)
{
  register BITMAP *result;

#ifdef DEBUG
  if (wide<=0 || high <=0 || !(depth==8 || depth==1))
  {
    fprintf(stderr,"bit_alloc boo-boo %d x %d x %d\r\n",wide,high,depth);
    return(NULL);
  }
#endif

  if(data)
    result=mem_point(wide,high,depth,data);
  else
    result=mem_create(wide,high,depth);

#ifdef MOVIE
  if(result) log_alloc(result);
#endif

  return (result);
}
/*}}}  */
