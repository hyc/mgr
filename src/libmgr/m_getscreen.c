/*{{{}}}*/
/*{{{  #includes*/
#include <mgr/mgr.h>
#include <ctype.h>
#include <stdio.h>
/*}}}  */

/*{{{  m_getscreensize*/
int m_getscreensize(int *width, int *height, int *depth)
{
  int w, h, d, dummy;
  char *p=m_linebuf;

  _m_ttyset();
  m_getinfo(G_SYSTEM);
  m_gets(m_linebuf);
  _m_ttyreset();
  while (*p && !isdigit(*p)) p++;
  if( sscanf(p,"%d %d %d %d",&w,&h,&dummy,&d) < 4)
    return( -1);
  if( width)
    *width = w;
  if( height)
    *height = h;
  if( depth)
    *depth = d;
  return ( 0);
}
/*}}}  */
