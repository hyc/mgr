/*{{{}}}*/
/*{{{  #includes*/
#include <mgr/mgr.h>
#include <stdio.h>
/*}}}  */

/*{{{  m_setup*/
int m_setup(int flags)
{
  m_flags = flags;

  if (!(m_flags&M_DEBUG)) 
  {
    m_termout = fopen(M_DEVICEOUT,"w");
    m_termin = fopen(M_DEVICEIN,"r");
  }

  if (m_termin == NULL || m_termout == NULL) m_flags |= M_DEBUG;

  if (m_flags&M_DEBUG) 
  {
    m_termin = stdin;
    m_termout = stdout;
  }
  return(m_flags);
}
/*}}}  */
