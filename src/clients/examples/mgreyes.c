#include <assert.h>
#include <mgr/mgr.h>
#include <signal.h>

/*{{{}}}*/
/*{{{  #defines*/
#define EYE_BITMAP 1
#define PUIPLE_BITMAP 2
/*}}}  */

/*{{{  isqrt*/
#define N_BITS 32
#define MAX_BIT ((N_BITS+1)/2-1)

static unsigned int isqrt(unsigned int x)
{
  register unsigned int xroot, m2, x2;

  xroot=0; m2=1<<MAX_BIT*2;
  do
  {
    x2=xroot+m2;
    xroot>>=1;
    if (x2<=x)
    {
      x-=x2; xroot+=m2;
    }
  } while (m2>>=2);
  if (xroot<x) return xroot+1;
  return xroot;
}
/*}}}  */
/*{{{  clean*/
void clean(int n)
{
  m_bitdestroy(EYE_BITMAP);
  m_setcursor(CS_BLOCK);
  m_pop();
  m_flush();
  m_ttyreset();
  exit(n);
}
/*}}}  */
/*{{{  m_fcircleto*/
void m_fcircleto(int bitmap, int cx, int cy, int rx, int ry)
{
  int x,y,asp,ry2=ry*ry;

  asp=(rx*100)/ry;
  for (y=0; y<=ry; y++)
  {
    x=(isqrt(ry2-y*y)*asp)/100;
    m_lineto(bitmap,cx-x,cy+y,cx+x,cy+y);
    m_lineto(bitmap,cx-x,cy-y,cx+x,cy-y);
  }
}
/*}}}  */
/*{{{  mkeye*/
void mkeye(int width, int height, int *puiple)
{
  m_bitcreate(EYE_BITMAP,width,height);
  m_func(BIT_CLR);
  m_bitwriteto(0,0,width,height,EYE_BITMAP);
  m_func(BIT_SET);
  m_fcircleto(EYE_BITMAP,width/2,height/2,(10*width)/22,(10*height)/22);
  m_func(BIT_CLR);
  m_fcircleto(EYE_BITMAP,width/2,height/2,(10*width)/26,(10*height)/26);

  *puiple=(10*(width>height ? height : width))/90;
  m_bitcreate(PUIPLE_BITMAP,2**puiple,2**puiple);
  m_func(BIT_CLR);
  m_bitwriteto(0,0,2**puiple,2**puiple,PUIPLE_BITMAP);
  m_func(BIT_SET);
  m_fcircleto(PUIPLE_BITMAP,*puiple,*puiple,*puiple,*puiple);
}
/*}}}  */
/*{{{  redraw*/
void redraw(int my_width, int my_height)
{
  m_clear();
  m_func(BIT_SRC);
  m_bitcopyto(0,0,my_width/2,my_height,0,0,0,EYE_BITMAP);
  m_bitcopyto(my_width/2,0,my_width/2,my_height,0,0,0,EYE_BITMAP);
}
/*}}}  */
/*{{{  set_puiple*/
void set_puiple(int my_width, int my_height, int puiple, int lx, int ly, int rx, int ry)
{
  m_func(BIT_SRC|BIT_DST);
  m_bitcopyto(lx+my_width/4-puiple,ly+my_height/2-puiple,2*puiple,2*puiple,0,0,0,PUIPLE_BITMAP);
  m_bitcopyto(rx+(3*my_width)/4-puiple,ry+my_height/2-puiple,2*puiple,2*puiple,0,0,0,PUIPLE_BITMAP);
}
/*}}}  */
/*{{{  clr_puiple*/
void clr_puiple(int my_width, int my_height, int puiple, int lx, int ly, int rx, int ry)
{
  m_func(BIT_NOT(BIT_SRC)&BIT_DST);
  m_bitcopyto(lx+my_width/4-puiple,ly+my_height/2-puiple,2*puiple,2*puiple,0,0,0,PUIPLE_BITMAP);
  m_bitcopyto(rx+(3*my_width)/4-puiple,ry+my_height/2-puiple,2*puiple,2*puiple,0,0,0,PUIPLE_BITMAP);
}
/*}}}  */
/*{{{  m_getmousepos*/
int m_getmousepos(int *x, int *y, int *b)
{
  char line[MAXLINE];

  _m_ttyset();
  m_getinfo(G_MOUSE);
  m_flush();
  fgets(line,sizeof(line),m_termin);
  if (*line<'0' || *line>'9')
  {
    strcpy(m_linebuf,line);
    fgets(line,sizeof(line),m_termin);
    m_sendme(m_linebuf);
  }
  _m_ttyreset();
  return (sscanf(line,"%d %d %d",x,y,b)==3);
}
/*}}}  */
/*{{{  puiple_pos*/
void puiple_pos(int ex, int ey, int mx, int my, int *nx, int *ny)
{
}
/*}}}  */

/*{{{  main*/
int main(int argc, char *argv[])
{
  /*{{{  variables*/
  int lx,ly,rx,ry,x,y,b, my_x, my_y, my_width, my_height, screen_width, screen_height, screen_depth, puiple, keypress;
  char evstr[10];
  int ox=-1, oy=-1;
  /*}}}  */

  /*{{{  setup*/
  ckmgrterm(argv[0]);
  m_setup(M_MODEOK);
  signal(SIGINT,clean);
  signal(SIGTERM,clean);
  m_ttyset();
  m_push(P_ALL);
  m_setmode(M_ABS);
  m_setcursor(CS_INVIS);
  m_setevent(REDRAW, "|R\n");
  m_setevent(RESHAPE, "|r\n");
  m_setevent(MOVE, "|m\n");
  m_flush();
  /*}}}  */
  m_getwindowsize(&my_width,&my_height);
  m_getwindowposition(&my_x,&my_y);
  m_getscreensize(&screen_width,&screen_height,&screen_depth);
  mkeye(my_width/2,my_height,&puiple);
  redraw(my_width,my_height);
  do
  {
    m_flush();
    switch (m_getevent(100,&keypress,evstr,sizeof(evstr)))
    {
      case EV_TIMEOUT:
      {
        int olx,oly,orx,ory;

        assert(m_getmousepos(&x,&y,&b));
        if (x!=ox || y!=oy)
        {
          /*{{{  left eye*/
          olx=lx; oly=ly;
          lx=30*(x-(my_x+my_width/4))/screen_width;
          ly=30*(y-(my_y+my_height/2))/screen_height;
          while (lx*lx+ly*ly>=25*25) { lx=(10*lx)/11; ly=(10*ly)/11; }
          lx=(puiple*lx)/10;
          ly=(puiple*ly)/10;
          /*}}}  */
          /*{{{  right eye*/
          orx=rx; ory=ry;
          rx=30*(x-(my_x+(3*my_width)/4))/screen_width;
          ry=ly;
          while (rx*rx+ry*ry>=25*25) { rx=(10*rx)/11; ry=(10*ry)/11; }
          rx=(puiple*rx)/10;
          ry=(puiple*ry)/10;
          /*}}}  */
          if (ox!=-1) clr_puiple(my_width,my_height,puiple,olx,oly,orx,ory);
          set_puiple(my_width,my_height,puiple,lx,ly,rx,ry);
        }
        ox=x; oy=y;
        break;
      }
      case EV_KEYPRESS: break;
      case EV_EVENTSTR:
      {
        ox=-1; oy=-1;
        switch (*evstr)
        {
          case 'R': redraw(my_width,my_height); break;
          case 'r':
          {
            m_getwindowsize(&my_width,&my_height);
            m_getwindowposition(&my_x,&my_y);
            m_bitdestroy(EYE_BITMAP);
            m_bitdestroy(PUIPLE_BITMAP);
            mkeye(my_width/2,my_height,&puiple);
            redraw(my_width,my_height);
            break;
          }
          case 'm': m_getwindowposition(&my_x,&my_y); break;
          default: assert(0);
        }
        break;
      }
      default: assert(0);
    }
  } while (keypress!='q');
  clean(0);
  return 255;
}
/*}}}  */
