/*{{{}}}*/
/*{{{  #includes*/
#include <mgr/bitblit.h>
#include <mgr/share.h>
#include <stdlib.h>
/*}}}  */

/*{{{  bit_open.c*/
BITMAP *bit_open(char *name)
{
  BITMAP *result = (BITMAP*)0;

  if ((result=malloc(sizeof(BITMAP)))==(BITMAP*)0) return (BITMAP*)0;
  result->primary = result;
  result->data = bit_initscreen(name,&(result->wide),&(result->high),
				&(result->depth),&(result->deviceinfo));
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
