/*{{{}}}*/
/*{{{  #includes*/
#include <mgr/mgr.h>
#include <stdio.h>
/*}}}  */

/*{{{  m_getwindowsize*/
int m_getwindowsize(int *width, int *height)
{
  int w, h, dummy;

  _m_ttyset();
  m_getinfo(G_COORDS);
  m_gets(m_linebuf);
  _m_ttyreset();
  if( sscanf(m_linebuf,"%d %d %d %d",&dummy,&dummy,&w,&h) < 4)
    return( -1);
  if( width)
    *width = w;
  if( height)
    *height = h;
  return( 0);
}
/*}}}  */
