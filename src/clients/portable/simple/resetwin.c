#include <stdio.h>
#include <string.h>

#ifdef OLDMGR
#include <term.h>
#define BIT_SRC B_SRC
#else
#include <mgr/mgr.h>
#endif

extern int m_envcount;

int
main( ) {
  int i, fg;

  if( ! is_mgr_term()) {
    fprintf( stderr, "resetwin:  only works on MGR terminals.\n");
    exit( 1);
  }

  m_setup( 0);
  m_envcount = 5;              /* fake it, because we're doing repairs */
  m_popall();
    
  m_selectwin( 0);

  m_clearevent( ACCEPT);
  m_clearevent( ACTIVATE);
  m_clearevent( BUTTON_1);
  m_clearevent( BUTTON_1U);
  m_clearevent( BUTTON_2);
  m_clearevent( BUTTON_2U);
  m_clearevent( COVERED);
  m_clearevent( DEACTIVATE);
  m_clearevent( DESTROY);
  m_clearevent( MOVE);
  m_clearevent( NOTIFY);
  m_clearevent( PASTE);
  m_clearevent( REDRAW);
  m_clearevent( RESHAPE);
  m_clearevent( SNARFED);
  m_clearevent( UNCOVERED);

  m_nomenu();
  m_nomenu2();

  for( i = 0; i < 20; i += 1)
    m_clearmenu( i);
  for( i = 1; i <= 50; i += 1)
    m_bitdestroy( i );
  for( i = 1; i <= 25; i += 1)
    m_destroywin( i);

  m_setmode( M_BACKGROUND);
  m_clearmode( M_ABS);
  m_clearmode( M_AUTOEXPOSE);
  m_clearmode( M_NOINPUT);
  m_clearmode( M_NOWRAP);
  m_clearmode( M_OVERSTRIKE);
  m_clearmode( M_SNARFHARD);
  m_clearmode( M_SNARFLINES);
  m_clearmode( M_SNARFTABS);
  m_clearmode( M_STACK);
  m_clearmode( M_STANDOUT);
  m_clearmode( M_WOB);
  m_clearmode( M_DUPKEY);

  m_setcursor( CS_BLOCK);
  m_font( 0);
  m_func( BIT_SRC);
#if defined(__linux__) || defined(__i386__)
  fg = (m_getdepth() == 8)? 255: 63; /* mono VGA uses 63 */
#else
  fg = 255;
#endif
  m_fgbgcolor( fg, 0);
  m_fcolor( fg);
  m_bcolor( 0);
  m_textreset();
  m_clear();

  m_flush();
  m_setecho();
  m_setnoraw();
  exit(0);
}
