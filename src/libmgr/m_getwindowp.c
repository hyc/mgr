/*{{{}}}*/
/*{{{  #includes*/
#include <mgr/mgr.h>
#include <stdio.h>
/*}}}  */

/*{{{  m_getwindowposition*/
int m_getwindowposition(int *xpos, int *ypos)
{
  int x, y;

  _m_ttyset();
  m_getinfo(G_COORDS);
  m_gets(m_linebuf);
  _m_ttyreset();
  if( sscanf(m_linebuf,"%d %d",&x,&y) < 2)
    return( -1);
  if( xpos)
    *xpos = x;
  if( ypos)
    *ypos = y;
  return( 0);
}
/*}}}  */
