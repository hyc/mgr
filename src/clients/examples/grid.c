/*{{{}}}*/
/*{{{  Notes*/
/* draw a grid of lines */
/*}}}  */
/*{{{  #includes*/
#include <mgr/mgr.h>
#include <signal.h>
#include <stdlib.h>
/*}}}  */

/*{{{  clean*/
static void clean(int i)
{
  m_pop();
  exit(i);
}
/*}}}  */

/*{{{  main*/
int main(int argc,char *argv[])
{
  int x,y,i;
  int xmax,ymax,dummy;

  ckmgrterm(*argv);

  if (argc >= 2) { x = atoi(argv[1]); y = atoi(argv[2]); }
  else { x = 10; y = 10; }

  if (x<2) x = 10;
  if (y<2) y = 10;

  m_setup(0);
  get_size(&dummy,&dummy,&xmax,&ymax);
  signal(SIGTERM,clean);
  signal(SIGINT,clean);
  signal(SIGHUP,clean);
  m_clear();
  m_push(P_FLAGS);
  m_setmode(M_ABS);

  m_func(BIT_SET);
  for(i=0;i<xmax;i+=x) m_line(i,0,i,ymax);
  for(i=0;i<ymax;i+=y) m_line(0,i,xmax,i);
  clean(0);
  return 255;
}
/*}}}  */
