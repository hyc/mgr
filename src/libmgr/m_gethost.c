/*{{{}}}*/
/*{{{  #includes*/
#include <mgr/mgr.h>
#include <stdio.h>
/*}}}  */

/*{{{  m_gethostname*/
int m_gethostname(char *name, size_t len)
{
  char *p=m_linebuf;

  _m_ttyset();
  m_getinfo(G_SYSTEM);
  m_gets(m_linebuf);
  _m_ttyreset();
  while (len && *p!=' ' && (*name++=*p++)!='\0') --len;
  if (len) *name='\0';
  return (p!=m_linebuf ? 0 : -1);
}
/*}}}  */
