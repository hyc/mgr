/*{{{}}}*/
/*{{{  #includes*/
#include <mgr/mgr.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

/*}}}  */

#define PREFIX '|'

/*{{{  getkey*/
/*{{{  Notes*/
/*

Function key scanner, with the exception that PREFIX characters come doubled.

*/
/*}}}  */
static int getkey(char firstchar)
{
  return firstchar;
}
/*}}}  */

/*{{{  m_getevent*/
/*{{{  Notes*/
/*

m_getevent() assumes that all event strings start with PREFIX as first
character and next character anything but PREFIX or space.  Further it
is assumed that they end with '\n' and that the terminal is already set.

*/
/*}}}  */
int m_getevent(int timeout, int *keypress, char *eventstr, size_t eventstrsize)
{
  /*{{{  variables*/
  struct fd_set readfds;
  struct timeval timeoutval;
  int n;
  /*}}}  */

  m_dupkey(PREFIX);
  m_flush();
  /*{{{  set m_termin as only descriptor*/
  FD_ZERO(&readfds);
  FD_SET(fileno(m_termin),&readfds);
  /*}}}  */
  /*{{{  compute timeval struct*/
  timeoutval.tv_sec=timeout/1000;
  timeoutval.tv_usec=(timeout%1000)*1000;
  /*}}}  */
  if ((n=select(32,&readfds,(fd_set*)0,(fd_set*)0,&timeoutval))>=0)
  {
    if (FD_ISSET(fileno(m_termin),&readfds))
    /*{{{  we have something to read, whatever it is*/
    {
      if ((*keypress=fgetc(m_termin))!=PREFIX)
      /*{{{  keyboard input*/
      {
        *keypress=getkey(*keypress);
        m_clearmode(M_DUPKEY);
        return EV_KEYPRESS;
      }
      /*}}}  */
      else if ((*keypress=fgetc(m_termin))==PREFIX)
      /*{{{  this is the dup key*/
      {
        *keypress=getkey(*keypress);
        m_clearmode(M_DUPKEY);
        return EV_KEYPRESS;
      }
      /*}}}  */
      else
      /*{{{  this is an event string*/
      {
        *eventstr++=*keypress;
        fgets(eventstr,eventstrsize-1,m_termin);
        m_clearmode(M_DUPKEY);
        return EV_EVENTSTR;
      }
      /*}}}  */
    }
    /*}}}  */
    else
    {
      m_clearmode(M_DUPKEY);
      return EV_TIMEOUT;
    }
  } else
  {
    m_clearmode(M_DUPKEY);
    return EV_ERROR;
  }
}
/*}}}  */
