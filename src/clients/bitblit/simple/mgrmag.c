/*{{{}}}*/
/*{{{  Notes*/
/*

Copies part of the screen selected by the user and writes it,
magnifying as necessary, into the current window.

First draft, Michael Haardt 1993.
Color fixes and speedup, Vincent Broman, 1994.

*/
/*}}}  */
/*{{{  #includes*/
#include <mgr/mgr.h>
#include <mgr/bitblit.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
/*}}}  */
/*{{{  #defines*/
#define ABS(x) ((x)<0 ? -(x) : (x))
#define MIN(x,y) (x<y ? x : y)
/*}}}  */

/*{{{  clean*/
static void clean(int n)
{
  m_setcursor(CS_BLOCK);
  m_popall();
  m_flush();
  m_ttyreset();
  exit(n);
}
/*}}}  */
/*{{{  sigexit*/
static void sigexit(int n)
{
  clean(1);
}
/*}}}  */
/*{{{  prep_color */
static int prev_color = -2;

static void prep_color(int c) {
  if(c != prev_color) {
    if(c != 0) {
      m_func(BIT_SRC);
      m_fgbgcolor(c,0);
    } else {
      m_func(BIT_NOT(BIT_SRC));
      m_fgbgcolor(1,c);
    }
    prev_color=c;
  }
}
/*}}}  */
/*{{{  redraw*/
static void redraw(BITMAP *rect)
{
  int width,height;
  int x,y,w,h;
  int row,col,startcol;
  int first_pix,next_pix,prev_pix;

  m_getwindowsize(&width,&height);
  if (rect)
  {
    /* cover window with likely popular color */
    first_pix=bit_on(rect,0,0);
    prep_color(first_pix);
    m_bitwrite(0,0,width,height);
    /* draw horizontal segments of non-background color */
    for (row=0; row<rect->high; row+=1) {
      for(col=0;;) {
	while(col<rect->wide) {
	  next_pix = bit_on(rect,col,row);
	  if(next_pix!=first_pix)  break;
	  col+=1;
	}
	if(col>=rect->wide)  break;
	/* now col<rect->wide and next_pix!=first_pix */
	prev_pix=next_pix;
	startcol=col;
	while(col<rect->wide) {
	  next_pix = bit_on(rect,col,row);
	  if (next_pix!=prev_pix)  break;
	  col+=1;
	}
	/* startcol..col-1 are contiguous pixels == prev_pix */
	prep_color(prev_pix);
	x=(startcol*width)/rect->wide;
	y=(row*height)/rect->high;
	w=(col*width)/rect->wide - x;
	h=((row+1)*height)/rect->high - y;
	m_bitwrite(x,y,w?w:1,h?h:1);
      }
    }
  } else {
    int col,row;

    m_clear();
    get_colrow(&col,&row);
    m_move((col-sizeof("No Image"))/2,row/2);
    fprintf(m_termout,"No Image");
  }
}
/*}}}  */

/*{{{  main*/
int main(int argc, char *argv[])
{
  /*{{{  variables*/
  char clienthost[64],serverhost[64];
  BITMAP *screen,*rect=(BITMAP*)0;
  int x1,y1,x2,y2;
  /*}}}  */

  /*{{{  setup*/
  ckmgrterm(argv[0]);
  m_setup(M_MODEOK);
  /*}}}  */
  /*{{{  parse arguments*/
  if (argc>2)
  {
    fprintf(stderr,"Usage: mgrmag [file]\n");
    clean(1);
  }
  /*}}}  */
  /*{{{  check if running on mgr host*/
  m_ttyset();
  m_gethostname(serverhost,sizeof(serverhost));
  gethostname(clienthost,sizeof(clienthost));
  m_ttyreset();
  if (strncmp(serverhost,clienthost,sizeof(serverhost)))
  {
    fprintf(stderr,"mgrmag: Only runs on MGR server host.\n");
    clean(1);
  }
  /*}}}  */
  /*{{{  open screen*/
  if ((screen=bit_open(SCREEN_DEV))==(BITMAP*)0)
  {
    fprintf(stderr,"mgrmag: can't open screen.\n");
    clean(1);
  }
  /*}}}  */
  /*{{{  reset suid status*/
  setuid(getuid()); setgid(getgid());
  /*}}}  */
  /*{{{  ok, complete setup*/
  m_ttyset();
  m_push(P_ALL);
  m_setmode(M_ABS);
  m_setcursor(CS_INVIS);
  m_setevent(BUTTON_1,"=%R\n");
  m_setevent(BUTTON_2,"Q\n");
  m_setevent(REDRAW,"R\n");
  m_setevent(RESHAPE,"R\n");
  signal(SIGINT,sigexit);
  signal(SIGHUP,sigexit);
  signal(SIGTERM,sigexit);
  /*}}}  */
  /*{{{  event loop*/
  redraw(rect);
  m_flush();
  while (m_get()) switch (m_linebuf[0])
  {
    /*{{{  =  -- user marked a rectangle on screen, display it*/
    case '=':
    {
      if (sscanf(m_linebuf+1,"%d %d %d %d",&x1,&y1,&x2,&y2)!=4);
      else
      {
        /*{{{  copy rectangle*/
        if (rect!=(BITMAP*)0) bit_destroy(rect);
        rect=bit_alloc(ABS(x2-x1),ABS(y2-y1),(BITMAP*)0,screen->depth);
        bit_blit(rect,0,0,rect->wide,rect->high,BIT_SRC,screen,MIN(x1,x2),MIN(y1,y2));
        /*}}}  */
        redraw(rect);
      }
      break;
    }
    /*}}}  */
    /*{{{  R  -- redraw, reshape*/
    case 'R': redraw(rect); break;
    /*}}}  */
    /*{{{  Q  -- save, if filename given, and exit*/
    case 'Q':
    {
      FILE *fp=(argc==2 ? fopen(argv[1],"wb") : (FILE*)0);

      if (fp==(FILE*)0 || rect==(BITMAP*)0) m_bell();
      else bitmapwrite(fp,rect);
      if (fp!=(FILE*)0) fclose(fp);
      clean(0);
    }
    /*}}}  */
  }
  /*}}}  */
  return 255;
}
/*}}}  */
