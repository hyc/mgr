/*{{{}}}*/
/*{{{  #includes*/
#include <mgr/mgr.h>
#include <stdio.h>
/*}}}  */

/*{{{  m_getfontid*/
int m_getfontid(void)
{
  int dummy,id;

  _m_ttyset();
  m_getinfo(G_FONT);
  m_gets(m_linebuf);
  _m_ttyreset();
  return(sscanf(m_linebuf,"%d %d %d",&dummy,&dummy,&id)==3 ? id : -1);
}
/*}}}  */
