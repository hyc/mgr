/*{{{}}}*/
/*{{{  #includes*/
#include <mgr/mgr.h>
#include <ctype.h>
#include <stdio.h>
/*}}}  */

/*{{{  m_getfontname*/
char *m_getfontname(void)
{
  char *p;

  _m_ttyset();
  m_getinfo(G_FONT);
  m_gets(m_linebuf);
  _m_ttyreset();
  p=m_linebuf+strlen(m_linebuf);
  if (p!=m_linebuf)
  {
    *(--p)='\0';
    while (p>m_linebuf && *p!=' ') p--;
    return (*p==' ' ? p+1 : (char*)0);
  } else return (char*)0;
}
/*}}}  */
