#include <mgr/mgr.h>
#include <stdio.h>

int m_getpalette(unsigned int color, unsigned int *r, unsigned int *g,
			unsigned int *b, unsigned int *maxi)
{
  unsigned int c2;

  if( color > 255 || !r || !g || !b || !maxi)
    return( -1);
  _m_ttyset();
  m_getcolorrgb( color);
  m_flush();
  m_gets(m_linebuf);
  _m_ttyreset();
  return (sscanf(m_linebuf,"COLOR %u %u %u %u %u",&c2,r,g,b,maxi) == 4
          && c2 == color? 0: -1);
}
