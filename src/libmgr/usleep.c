#ifdef NEED_USLEEP
/*{{{}}}*/
/*{{{  includes*/
#ifdef hpux
#undef _POSIX_SOURCE
#define _INCLUDE_HPUX_SOURCE
#endif

#ifdef SELECT_USLEEP
#include <sys/time.h>
#else
#ifdef POLL_USLEEP
#include <poll.h>
#else
#include <sys/time.h>
#endif
#endif
/*}}}  */

/*{{{  usleep*/
#define sec ((unsigned long)1000000)

int usleep(unsigned long t)
{
  if (t>sec) { sleep((int)(t/sec));t%=sec; }
  if (t)
# ifdef SELECT_USLEEP
  /*{{{  select*/
  {
    struct timeval timeout;

    timeout.tv_sec=0;
    timeout.tv_usec=t;
    select((size_t)0,(void*)0,(void*)0,(void*)0,&timeout);
  }
  /*}}}  */
# else
# ifdef POLL_USLEEP
  /*{{{  poll*/
  poll((struct pollfd*)0,(size_t)0,(int)(t/1000));
  /*}}}  */
# else
  /*{{{  wait busy*/
  {
    struct timezone zone;
    struct timeval start;

    gettimeofday(&start,&zone);
    do
    { struct timeval end;

      gettimeofday(&end,&zone);
      if (start.tv_usec>end.tv_usec)
      { end.tv_usec+=sec;end.tv_usec--; }
      if (  ((end.tv_sec-start.tv_sec)>0)
      ||((end.tv_usec-start.tv_usec)>t)) t=0;
    }
    while (t);
  }
  /*}}}  */
# endif
# endif
  return(0);
}
/*}}}  */
#endif
