/*
 * mnew - run any command line in a new mgr window
 *
 * usage:  mnew [options...] 'shell command line'
 * options are, with n being a decimal nbr:
 *	-p	preserve piped stdin, dont read from window tty
 *	-s	use user's SHELL from environment, otherwise /bin/sh
 *	-f n	use font nbr n in the new window, otherwise use same font
 *	-x n	place window horizontally at n
 *	-y n	place window vertically at n
 *	-w n	make window width n, otherwise like parent
 *	-h n	make window height n, otherwise like parent
 *	-t tty	use prepared half window tty device, dont open a new one.
 * windows tend to be placed above and to the right of the parent,
 * but wrap around to the opposite end of the screen when the edge is hit.
 * No new window is created if MGR not running, i.e. not in the TERM env var.
 *
 * Vincent Broman, broman@nosc.mil, 14 march 1996
 */

#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

#include <mgr/mgr.h>

#define MINW 16

static void die();


extern int
main( argc, argv)
int argc;
char *argv[];
{
  char *s, *getenv(), ptyname[ 32];
  char *shell = "/bin/sh";
  char *command_line = 0;
  int chuck_stdin = 1;
  int preopened = 0;
  int f = -1;	/* -1 means unknown/unset */
  int x = -1;
  int y = -1;
  int w = -1;
  int h = -1;
  int xx, yy;
  int last;
  int displaywid, displayht, borderwid = 5;

  ptyname[0] = 0;
  for( argc--,argv++; argc > 0 && **argv == '-'; argc--,argv++) {
    switch( argv[0][1]) {
    case 'p':		/* preserve stdin for pipe input */
      chuck_stdin = 0;
      break;
    case 's':		/* use user's shell */
      s = getenv( "SHELL");
      if( s)
	shell = s;
      break;
    case 't':		/* Use the named tty for a preopened window */
      s = argv[0][2]? argv[0] + 2: argv[1];
      if( argv[0][2] == 0)
	argc--,argv++;
      if( s) {
	strncpy( ptyname, s, sizeof( ptyname));
	ptyname[ sizeof( ptyname) - 1] = 0;
	if( access( ptyname, R_OK| W_OK) == 0)
	  preopened = 1;
	else
	  die( "named pty file is not accessible");
      }
      break;
    case 'f':		/* set font nbr in new window */
      s = argv[0][2]? argv[0] + 2: argv[1];
      if( argv[0][2] == 0)
	argc--,argv++;
      if( sscanf( s, "%d", &f) != 1)
	f = -1;
      break;
    case 'x':		/* set x coord of new window */
      s = argv[0][2]? argv[0] + 2: argv[1];
      if( argv[0][2] == 0)
	argc--,argv++;
      if( sscanf( s, "%d", &x) != 1)
	x = -1;
      break;
    case 'y':		/* set y coord of new window */
      s = argv[0][2]? argv[0] + 2: argv[1];
      if( argv[0][2] == 0)
	argc--,argv++;
      if( sscanf( s, "%d", &y) != 1)
	y = -1;
      break;
    case 'w':		/* set w coord of new window */
      s = argv[0][2]? argv[0] + 2: argv[1];
      if( argv[0][2] == 0)
	argc--,argv++;
      if( sscanf( s, "%d", &w) != 1)
	w = -1;
      break;
    case 'h':		/* set h coord of new window */
      s = argv[0][2]? argv[0] + 2: argv[1];
      if( argv[0][2] == 0)
	argc--,argv++;
      if( sscanf( s, "%d", &h) != 1)
	h = -1;
      break;
    default:
      die( "unrecognized option argument");
    }
  }
  if( argc <= 0 || *argv == 0 || **argv == 0)
    die( "no command line string supplied");
  else if( argc == 1)
    command_line = *argv;
  else {
    /* multiple arguments already parsed by the shell */
    char *normals = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789/._-:,";
    int len;
    char **av;

    len = 1;
    for( av = argv; *av; av += 1)
      len += strlen( *av) + 3;
    command_line = malloc( len);
    if( command_line == 0)
      die( "no more memory");
    s = command_line;
    for( av = argv; *av; av += 1) {
      len = strlen( *av);
      if( strspn( *av, normals) < len) {
	/* Magic chars.
	 * this quoting fails if the string contains apostrophes
	 * or chars not quoted by apostrophes.
	 */
	sprintf( s, "'%s' ", *av);
	s += len + 3;
      } else {
	/* no magic chars */
	sprintf( s, "%s ", *av);
	s += len + 1;
      }
    }
  }

  if( is_mgr_term()) {
    /* open a new window with pty for the program */
    m_setup( M_MODEOK);

    m_ttyset();
    if( x < 0 || y < 0 || w < MINW || h < MINW || f < 0) {
      borderwid = m_getbordersize();
      m_getscreensize( &displaywid, &displayht, (int *)0);
    }

    /* actual min win size depends on font size */
    if( w < MINW && h < MINW) {
      m_getwindowsize( &w, &h);
      w += 2 * borderwid;
      h += 2 * borderwid;
    } else if( w < MINW) {
      m_getwindowsize( &w, (int *)0);
      w += 2 * borderwid;
    } else if( h < MINW) {
      m_getwindowsize( (int *)0, &h);
      h += 2 * borderwid;
    }
    if( w > displaywid)
      w = displaywid;
    if( h > displayht)
      h = displayht;

    if( x < 0 || y < 0) {
      m_getwindowposition( &xx, &yy);
      /* offset the new window NE from the old */
      if( x < 0) {
	x = xx + 32;
	if( x + w > displaywid)
	  x = 0;
      }
      if( y < 0) {
	y = yy - 32;
	if( y < 0 || y + h > displayht)
	  y = displayht - h;
      }
    }

    if( f < 0)
      f = m_getfontid();

    if( !preopened) {
      m_halfwinft( x, y, w, h, f);
      m_flush();
      m_gets( ptyname);	/* hope nothing interferes with this xchg */
    }
    m_ttyreset();

    last = strlen( ptyname) - 1;
    if( ptyname[ last] == '\n') ptyname[ last--] = 0;
    if( last < 4)
      die( "mgr did not create a new window.");
    if( access( ptyname, R_OK| W_OK) < 0)
      die( "cannot open the pty for the new window.");

    if( chuck_stdin)
      close( 0);
    close( 1);
    close( 2);
    fclose( m_termin);  m_termin = NULL;
    fclose( m_termout);  m_termout = NULL;

    if( fork() > 0)
      _exit( 0);

    /* get our own process group and controlling terminal */
    setsid();

    if( chuck_stdin)
      (void) open( ptyname, O_RDONLY);	/* stdin */
    (void) open( ptyname, O_WRONLY);	/* stdout */
    (void) open( ptyname, O_WRONLY);	/* stderr */

    if( preopened) {
      m_setup( M_MODEOK);
      m_ttyset();
      m_shapewindow( x, y, w, h);
      m_font( f);
      m_flush();
      m_ttyreset();
      fclose( m_termin);  m_termin = NULL;
      fclose( m_termout);  m_termout = NULL;
    }
  }

  execl( shell, shell, "-c", command_line, (char *)0);
  die( "cannot exec the program.");
  return( 2); /* for lint */
}


static void die( s)
char *s;
{

  fprintf( stderr, "mnew: %s\n", s);
  fflush( stderr);
  sleep( 1);
  exit( 1);
}
