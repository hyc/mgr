/*{{{}}}*/
/*{{{  #includes*/
#include <sys/types.h>
#ifdef sun
#include <sundev/kbio.h>
#endif
#include <sys/ioctl.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
/* #include <termios.h> */
#include <unistd.h>
/*}}}  */

/*{{{  set_console*/
/*{{{  Notes*/
/*

Now this is a typical "portable-unix-mess".  On SunOS, everybody can
redirect console to everything, as long as you can open it.  On Linux
you need (e)uid 0 and before redirecting to another pty, first you have
to redirect back to /dev/console.  Coherent does not know redirection
without a custom console driver.  ARGH!!

*/
/*}}}  */
void set_console(int on, int fd)
{
#ifdef TIOCCONS
  /*{{{  variables*/
  int one = 1;
  int cfd;
  /*}}}  */

#ifdef sun
  /*{{{  make sure kbd is in direct mode*/
  {
  int kbd;
  int mode = 0;

  if ((kbd = open("/dev/kbd",0)) < 0 )
  {
    fprintf(stderr,"can't open keyboard\n");
    return;
  }
  if (ioctl(kbd,KIOCGDIRECT,&mode) < 0 )
  {
    fprintf(stderr,"can't get keyboard status\n");
    return;
  }
  if (mode != 1)
  {
    fprintf(stderr,"keyboard not in direct mode\n");
    return;
  }
  }
  /*}}}  */
#endif
  cfd=open("/dev/console",O_RDWR);
#ifndef linux
  if (!on)
#endif
  ioctl(cfd,TIOCCONS,&one);
  close(cfd);
  if (on && ioctl(fd,TIOCCONS,&one)<0) fprintf(stderr,"can't set new console device\n");
#endif
}
/*}}}  */

/*{{{  main*/
int main(argc,argv) int argc; char **argv;
{
  set_console(argc!=2,fileno(stderr));
  exit(0);
}
/*}}}  */
