/*{{{}}}*/
/*{{{  #includes*/
#include <mgr/mgr.h>
#include <stdio.h>
/*}}}  */

/*{{{  m_getfontsize*/
int m_getfontsize(int *width, int *height)
{
  int w, h;

  _m_ttyset();
  m_getinfo(G_FONT);
  m_gets(m_linebuf);
  _m_ttyreset();
  if( sscanf(m_linebuf,"%d %d",&w,&h) < 2)
    return( -1);
  if( width)
    *width = w;
  if( height)
    *height = h;
  return( 0);
}
/*}}}  */
