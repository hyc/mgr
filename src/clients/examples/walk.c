/*{{{}}}*/
/*{{{  #includes*/
#include <mgr/mgr.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
/*}}}  */
/*{{{  #defines*/
#define ICONPATH        "eye"
#define EYES    24
#define CYC     5
#define EYERT   1
#define EYEDN   7
#define EYELF   13
#define EYEUP   19

#define fsleep(x) usleep( (x) * 1000 )
/*}}}  */

/*{{{  variables*/
int hsize, vsize;
/*}}}  */

/*{{{  cleanup*/
static void cleanup(int n)
{
  m_pop();
  m_setcursor(CS_BLOCK);
  m_clear();
  m_ttyreset();
  exit(0);
}
/*}}}  */
/*{{{  clearit*/
static void clearit(int n)
{
  m_getwindowsize(&hsize,&vsize);
  m_clear();
}
/*}}}  */

/*{{{  main*/
int main(int argc, char *argv[])
{
  /*{{{  variables*/
  int w, h, d, i, j;
  int speed = 1;
  int delay = 30;
  int keypress;
  char buf[101];
  int c;
  /*}}}  */

  /*{{{  parse arguments*/
  ckmgrterm(*argv);
  while ((c=getopt(argc,argv,"d:s"))!=EOF) switch (c)
  {
    case 'd': delay=atoi(optarg); break;
    case 's': speed=0; break;
  }
  /*}}}  */
  /*{{{  set up*/
  m_setup(M_FLUSH);
  m_push(P_BITMAP|P_EVENT|P_FLAGS);
  m_setmode(M_ABS);
  m_setcursor(CS_INVIS);
  m_ttyset();
  /*}}}  */
  /*{{{  set signals*/
  signal(SIGINT,cleanup);
  signal(SIGTERM,cleanup);
  signal(SIGQUIT,clearit);
  /*}}}  */
  /*{{{  set events*/
  m_setevent(RESHAPE,"| \n");
  m_setevent(REDRAW,"| \n");
  /*}}}  */

  m_func(BIT_SRC); /* bit copy, so we don't have to erase */
  /*{{{  download icons*/
  for (i = 1; i <EYES+1; i++) 
  {
    sprintf(buf, "%s/eye%d", ICONPATH,i);
    if( !m_bitfile(i, buf, &w, &h, &d) ) 
    {
      fprintf( stderr, "cannot download %s.  quit\n", buf );
      exit( 1 );
    }
  }
  /*}}}  */
  clearit(0);
  while (1)
  {
    /*{{{  walk right*/
    j = EYERT;
    for (i = 2; i < hsize - w; i += speed)
    {
      m_bitcopyto(i, 2, w, h, 0, 0, 0, j);
      m_flush();
      ++j; /* cycle bitmap number */
      if (j > EYERT+CYC) j = EYERT;
      switch (m_getevent(delay,&keypress,buf,sizeof(buf)))
      {
        case EV_TIMEOUT: break;
        case EV_KEYPRESS: cleanup(0); break;
        case EV_EVENTSTR: clearit(0); break;
      }
    }
    /*}}}  */
    /*{{{  walk down*/
    j = EYEDN;
    for (i = 2; i < vsize - w - 4; i += speed) 
    {
      m_bitcopyto(hsize - w, i, w, h, 0, 0, 0, j);
      m_flush();
      ++j;
      if (j > EYEDN+CYC) j = EYEDN;
      switch (m_getevent(delay,&keypress,buf,sizeof(buf)))
      {
        case EV_TIMEOUT: break;
        case EV_KEYPRESS: cleanup(0); break;
        case EV_EVENTSTR: clearit(0); break;
      }
    }
    /*}}}  */
    /*{{{  walk left*/
    j = EYELF;
    for (i = hsize - w; i > 2; i -= speed) 
    {
      m_bitcopyto(i, vsize - w - 4, w, h, 0, 0, 0, j);
      m_flush();
      ++j; /* cycle the other way */
      if (j > EYELF+CYC) j = EYELF;
      switch (m_getevent(delay,&keypress,buf,sizeof(buf)))
      {
        case EV_TIMEOUT: break;
        case EV_KEYPRESS: cleanup(0); break;
        case EV_EVENTSTR: clearit(0); break;
      }
    }
    /*}}}  */
    /*{{{  walk up*/
    j = EYEUP;
    for (i = vsize - w - 4; i > 2; i -= speed)
    {
      m_bitcopyto(2, i, w, h, 0, 0, 0, j);
      m_flush();
      ++j;
      if (j > EYEUP+CYC) j = EYEUP;
      switch (m_getevent(delay,&keypress,buf,sizeof(buf)))
      {
        case EV_TIMEOUT: break;
        case EV_KEYPRESS: cleanup(0); break;
        case EV_EVENTSTR: clearit(0); break;
      }
    }
    /*}}}  */
  }
  return 0;
}
/*}}}  */
