/*{{{}}}*/
/*{{{  #includes*/
#include "screen.h"
/*}}}  */

/*{{{  bit_open.c*/
BITMAP *bit_open(char *name)
{
  BITMAP *result;

  result = pr_open(name);
# ifdef MOVIE
  if(result) log_open(result);
# endif
  return result;
}
/*}}}  */
