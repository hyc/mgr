/*
 * is_mgr_term - returns 1 (true) if tty is an MGR window, 0 else.
 */

#include <string.h>

#ifdef PROBE
#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>
#include <mgr/mgr.h>

/*
 * returns 1 if input on fd is available in ms milliseconds,
 * nonpositive otherwise.
 */
static int
waitmillisecs( ms, fd)
int ms, fd;
{
fd_set set;
struct timeval tv;

  tv.tv_sec = ms / 1000;
  tv.tv_usec = (ms % 1000) * 1000;
  FD_ZERO( &set);
  FD_SET( fd, &set);
  return select( fd + 1, &set, NULL, NULL, &tv);
}

static char cookie[ 14] = "# Needs MGR!\n"; /* in case it's sent to a shell */
static char line[ 128];
#endif	/* PROBE */


extern int
is_mgr_term(void)
{
  char *t, *getenv();
  int pred;

  t = getenv( "TERM");
  if( t)
    return( strcmp( t, "mgr") == 0 || strncmp( t, "mgr-", 4) == 0);
  if( getenv( "DISPLAY"))
    return 0;

  pred = 0;

#ifdef PROBE
  /* without env info, we probe with an m_sendme( cookie) call. */
  m_setup( M_MODEOK);
  m_ttyset();
  m_sendme( cookie);
  m_flush();
  if( waitmillisecs( 700, fileno( m_termin)) > 0) {
    m_gets( line);
    if( memcmp( cookie, line, strlen( cookie) + 1) == 0)
	pred = 1;
  }
  m_ttyreset();
#endif

  return pred;
}
