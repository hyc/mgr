/*{{{}}}*/
/*{{{  Notes*/
/*
mgrload - show cpu load average in an mgr window.

Original version by: Mark Dapoz 90/06/21, mdapoz@hybrid.UUCP or mdapoz%hybrid@cs.toronto.edu
Heavily edited by: Michael Haardt
*/
/*}}}  */
/*{{{  #includes*/
#include <mgr/mgr.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include "getload.h"
/*}}}  */
/*{{{  #defines*/
#define INTERVAL        5      /* time interval (sec) between samples*/
#define PSIZE           100    /* size of one display partition; .50 proc */

#define CONTEXT P_POSITION | P_WINDOW | P_FLAGS | P_EVENT | P_CURSOR
/*}}}  */

/*{{{  variables*/
int xscale, yscale, rscale;
int xmin, xmax, ymin, ymax;
int x,y;
int border;
/*}}}  */

/*{{{  max*/
static int max(nums) /* find maximum load avarage in queue */
int *nums;
{
  static int i,j;

  for (i=j=0; i < xmax; nums++, i++)
  j=*nums > j ? *nums : j;
  return(j);
}
/*}}}  */
/*{{{  draw_scale -- draw scale lines on the graph*/
static void draw_scale(int sections, int all)
{
  int i,j;

  m_func(BIT_XOR);
  for (i=1; i < sections; i++)
  {
    j=ymax-i*PSIZE*yscale/rscale;
    m_line(all ? 0 : xmax-1, j, all ? xmax : xmax-1, j);
  }
}
/*}}}  */
/*{{{  draw_bar*/
void draw_bar(int load,int column)
{
  int foo=load*yscale/rscale;

  if (column==xmax-1) { m_func(BIT_CLR); m_bitwrite(xmax-1, 0, 1, ymax-foo); }
  m_func(BIT_SET); m_bitwrite(column,ymax-foo,1,foo);
}
/*}}}  */
/*{{{  redraw -- redraw graph from history*/
static void redraw(nums, head) int *nums; int head;
{
  register int j,p;

  m_clear();
  for (p=0, j=head+1; p <= xmax-1; p++)
  {
    j=j%xmax;
    if (nums[j]) draw_bar(nums[j],p);
    j++;
  }
}
/*}}}  */
/*{{{  timer_event*/
static void timer_event() /* cause the load average to be sampled */
{
  signal(SIGALRM, SIG_IGN);
  m_sendme("S\n"); /* sample load average event */
  signal(SIGALRM, timer_event);
}
/*}}}  */
/*{{{  done*/
static void done() /* general purpose exit */
{
  m_ttyreset(); /* reset communication channel */
  m_popall(); /* restore window */
  exit(0);
}
/*}}}  */

/*{{{  main*/
int main(argc,argv) int argc; char **argv;
{
  /*{{{  variables*/
  int *samples;           /* circular queue of samples */
  int head=0;                     /* queue pointer */
  int partitions = 0, last_part = 0;
  char event[80];              /* mgr event queue */
  /*}}}  */

  /*{{{  check if an mgr terminal*/
  ckmgrterm(*argv);
  /*}}}  */
  /*{{{  init mgr*/
  m_setup(M_MODEOK);
  m_push(CONTEXT);
  m_setmode(M_ABS);
  m_setcursor(CS_INVIS);
  m_ttyset();
  /*}}}  */
  /*{{{  init variables*/
  xmin = 0;       /* mgr virtual window size */
  ymin = 0;
  m_getwindowsize(&xmax,&ymax);
  m_getwindowposition(&x,&y);
  border=m_getbordersize();
  xscale = xmax-xmin;
  yscale = ymax-ymin;
  samples=malloc(sizeof(int)*xmax);
  memset(samples, 0, xmax*sizeof(int));
  /*}}}  */
  /*{{{  set up signal handlers*/
  signal(SIGALRM, timer_event);
  signal(SIGTERM, done);
  signal(SIGINT, done);
  signal(SIGQUIT, done);
  signal(SIGHUP, done);
  /*}}}  */
  /*{{{  set up mgr events*/
  m_setevent(RESHAPE,   "H\n");
  m_setevent(REDRAW,    "R\n");
  /*}}}  */
  /*{{{  set up window and start first event*/
  m_clear();
  m_sendme("S\n");
  /*}}}  */
  while (1)
  {
    m_flush();
    /*{{{  get event*/
    if (m_gets(event) == (char*)0)
    /* restart interrupted call */
    if (errno==EINTR
#    ifdef EAGAIN
    || errno==EAGAIN
#    endif
    ) continue; else break;
    /*}}}  */
    alarm(0);
    switch (*event)
    {
      /*{{{  R -- redraw, H -- reshape*/
      case 'R':       /* redraw window */
      case 'H':       /* reshape window */
      m_getwindowposition(&x,&y);
      m_shapewindow(x,y,2*border+xmax,2*border+ymax);
      redraw(samples, head-1 < 0 ? xmax-1 : head-1);
      draw_scale(partitions, 1);
      break;
      /*}}}  */
      /*{{{  S -- get load average*/
      case 'S':
      samples[head]=(int)(getload()*100); /* 1 min avg */
      partitions=max(samples)/PSIZE+1;
      rscale=partitions*PSIZE;
      if (last_part == partitions)
      /*{{{  fits on last scale*/
      {
        /* scroll graph left */
        m_func(BIT_SRC); m_bitcopy(0, 0, xmax-1, ymax, 1, 0);
        draw_bar(samples[head],xmax-1);
        draw_scale(partitions, 0);
      }
      /*}}}  */
      else
      /*{{{  change scale*/
      {
        last_part=partitions;
        redraw(samples, head);
        draw_scale(partitions, 1);
      }
      /*}}}  */
      head=((head+1)%xmax);
      break;
      /*}}}  */
      /*{{{  default*/
      default: break;
      /*}}}  */
    }
    alarm(INTERVAL);
  }
  exit(0);
}
/*}}}  */
