#include <ctype.h>
#include <stdio.h>
#include <mgr/mgr.h>

/* m_getdepth inquires the server how many bits deep the screen is */
int m_getdepth( void)
{
  int d, dummy;
  char *p = m_linebuf;

  _m_ttyset();
  m_getinfo( G_SYSTEM);
  m_gets( m_linebuf);
  _m_ttyreset();
  while( *p && *p == ' ')
    p++;
  while( *p && *p != ' ')
    p++;		/* skip the hostname field */
  return( sscanf( p, "%d %d %d %d", &dummy, &dummy, &dummy, &d) == 4? d: -1);
  /* failure return of -1 for old servers that don't report the depth */
}
