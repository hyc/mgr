/*{{{}}}*/
/*{{{  #includes*/
#include <mgr/mgr.h>
#include <stdio.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>
/*}}}  */
/*{{{  #defines*/
#define ICON_BITMAP 1
#define ICON_NAME "sandclock"
/*}}}  */

/*{{{  clean up and exit*/
static void clean(n) int n;
{
  signal(SIGHUP,SIG_IGN);
  signal(SIGTERM,SIG_IGN);
  signal(SIGINT,SIG_IGN);
  m_popall();
  m_setcursor(0);
  m_ttyreset();
  exit(n);
}
/*}}}  */

/*{{{  main*/
int main(int argc,char *argv[])
{
  /*{{{  variables*/
  int icon_width,icon_height,icon_depth;
  int font_width,font_height;
  int x,y,width,height,border;
  time_t end_time,sand;
  int mm,ss;
  /*}}}  */

  /*{{{  compute end time*/
  if (argc!=2 || sscanf(argv[1],"%02d:%02d",&mm,&ss)!=2)
  {
    fprintf(stderr,"Usage: mgrsclock mm:ss\n");
    exit(1);
  }
  end_time=time((time_t*)0)+mm*60+ss;
  /*}}}  */
  /*{{{  set up*/
  ckmgrterm(*argv);
  m_setup(M_FLUSH);
  m_push(P_ALL);
  m_ttyset();
  m_setmode(M_NOWRAP);
  m_setmode(M_ABS);
  m_func(BIT_SRC);
  m_setcursor(5);
  /*}}}  */
  /*{{{  signals*/
  signal(SIGHUP,clean);
  signal(SIGTERM,clean);
  signal(SIGINT,clean);
  /*}}}  */
  /*{{{  load icon*/
  m_bitfile(ICON_BITMAP,ICON_NAME,&icon_width,&icon_height,&icon_depth);
  if (icon_width==0 || icon_height==0) clean(1);
  /*}}}  */
  /*{{{  get font size*/
  m_getfontsize(&font_width,&font_height);
  /*}}}  */
  /*{{{  reshape*/
  m_getwindowposition(&x,&y);
  border=m_getbordersize();
  width=(font_width*(sizeof("00:00")-1)>icon_width ? font_width*(sizeof("00:00")-1) : icon_width);
  height=font_height+icon_height;
  m_shapewindow(x,y,2*border+width,2*border+height);
  /*}}}  */
  /*{{{  draw icon*/
  m_clear();
  m_bitcopyto((width-icon_width)/2,0,icon_width,icon_height,0,0,0,ICON_BITMAP);
  /*}}}  */
  /*{{{  let time pass  ...*/
  while ((sand=(end_time-time((time_t*)0)))>=(time_t)0)
  {
    m_movecursor((width-font_width*(sizeof("00:00")-1))/2,height);
    fprintf(m_termout,"%02d:%02d",sand/60,sand%60);
    m_flush();
    sleep(1);
  }
  /*}}}  */
  /*{{{  done*/
  m_movecursor((width-font_width*(sizeof("00:00")-1))/2,height);
  fprintf(m_termout,"--:--");
  m_bell();
  m_flush();
  getc(m_termin);
  /*}}}  */
  clean(0);
  return 255;
}
/*}}}  */
