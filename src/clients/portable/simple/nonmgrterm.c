/*
 * nonmgrterm - returns 0 (true) if tty is not an MGR window.
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
int
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

static char cookie[ 5] = "*Z!\n";
static char line[ 10];
#endif	/* PROBE */


extern int
main()
{
  char *t, *getenv();
  int pred;

  t = getenv( "TERM");
  if( t)
    return( strcmp( t, "mgr") == 0 || strncmp( t, "mgr-", 4) == 0);

  pred = 0;

#ifdef PROBE
  /* without a TERM set, we probe with an m_sendme( cookie) call. */
  m_setup( M_MODEOK);
  m_ttyset();
  m_sendme( cookie);
  m_flush();
  if( waitmillisecs( 600, fileno( m_termin)) > 0) {
    if( memcmp( cookie, line, strlen( cookie) + 1) == 0)
	pred = 1;
  }
  m_ttyreset();
#endif

  return pred;
}
