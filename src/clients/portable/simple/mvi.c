/*
 * mvi - run the vi editor in a new mgr window
 */

#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <mgr/mgr.h>

static void die();

#ifdef DEBUG
static void
complain( char *s) {
    int fd = open( "/dev/console", O_WRONLY|O_NOCTTY);
    FILE *f = fdopen( fd, "w");
    fprintf(f,"%s\r\n",s);
    fclose(f);
}

static void
check_fg( char *ptyname) {
    int pg, fgpg, fgpg2, fd;

    fd = open( "/dev/tty", 0);
    if( fd < 0)  complain( "cant open /dev/tty");
    fgpg = tcgetpgrp( fd);
    if( fgpg < 0)  complain( "cant tcgetpg");
    pg = getpgrp();
    if(pg != fgpg)  complain( "pg not fg");
    close( fd);
    fd = open( ptyname, 0);
    fgpg2 = tcgetpgrp( fd);
    if( fgpg2 != fgpg)  complain( "ctty not pty");
    close( fd);
}
#endif /* DEBUG */


extern int
main( argc, argv)
int argc;
char *argv[];
{
  char ptyname[ 32];
  int i;
  int x, y, w, h, displaywid, borderwid;

  if( ! is_mgr_term())
    die( "only works on mgr terminals.");

  m_setup( M_MODEOK);

  m_ttyset();
  borderwid = m_getbordersize();
  m_getscreensize( &displaywid, (int *)0, (int *)0);
  m_getwindowsize( &w, &h);
  m_getwindowposition( &x, &y);
  w += 2 * borderwid;
  h += 2 * borderwid;
  /* offset the new window NE from the old */
  y -= 20;
  if( y < 0)  y = 0;
  x += 32;
  if( x + w > displaywid)
    x = displaywid - w;

  m_halfwinft( x, y, w, h, m_getfontid());
  m_flush();
  m_gets( ptyname);
  m_ttyreset();

  i = strlen( ptyname) - 1;
  if( ptyname[ i] == '\n') ptyname[ i--] = 0;
  if( i < 4)
    die( "mgr did not create a new window.");
  if( access( ptyname, R_OK| W_OK) < 0)
    die( "cannot open the pty for the new window.");

  close( 0);
  close( 1);
  close( 2);
  fclose( m_termin);  m_termin = NULL;
  fclose( m_termout);  m_termout = NULL;

  if( fork() > 0)
    _exit( 0);

  /* get our own process group and controlling terminal */
  setsid();

  (void) open( ptyname, O_RDONLY); /* stdin */
  (void) open( ptyname, O_WRONLY); /* stdout */
  (void) open( ptyname, O_WRONLY); /* stderr */

#ifdef DEBUG
  check_fg( ptyname);
#endif

  {
    char modeset[2];
    modeset[0] = ESC;
    modeset[1] = E_VI;
    write( 1, modeset, 2); /* set vi-mode in this window */
  }
  argv[0] += 1;		/* tell vi it is vi instead of mvi */
  execvp( "vi", argv);
  execvp( "elvis", argv);
  die( "cannot exec the visual editor program.");
  return( 2); /* for lint */
}


static void die( s)
char *s;
{

  fprintf( stderr, "mvi: %s\n", s);
  fflush( stderr);
  sleep( 1);
  exit( 1);
}
