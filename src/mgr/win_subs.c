/*{{{}}}*/
/*{{{  Notes*/
/* Teminal emulator functions called from put_window() and down_load() */
/*}}}  */
/*{{{  #includes*/
#include <mgr/bitblit.h>
#include <mgr/font.h>
#include <stdlib.h>
#include <stdio.h>

#include "clip.h"
#include "defs.h"
#include "graph_subs.h"
#include "win_subs.h"
/*}}}  */

/*{{{  win_rop -- Do raster ops*/
void win_rop(WINDOW *win, BITMAP *window)
{
  register int *p = W(esc);
  register int op;

  op = W(op);
  dbgprintf('B',(stderr,"%s: blit\t",W(tty)));
  switch (W(esc_cnt)) 
  {
    /*{{{  0 -- set raster op function*/
    case 0:
    W(op) = PUTOP(*p,op);	/* change op, leave colors alone */
    if (W(flags)&W_OVER) 
    {
      W(style) = PUTOP(*p,W(op));
    }
    dbgprintf('B',(stderr,"setting function %d\r\n",p[0]));
    dbgprintf('B',(stderr,"  new W(op): op=%d, fg=%d, bg=%d\n",
		  OPCODE(W(op)),GETFCOLOR(W(op)),GETBCOLOR(W(op))));
    break;
    /*}}}  */
    /*{{{  1 -- set raster op fg and bg colors*/
    case 1:
    W(op) = BUILDOP(op,
		    p[0]>=0?p[0]:GETFCOLOR(op),
		    p[1]>=0?p[1]:GETBCOLOR(op));
    dbgprintf('B',(stderr,"setting colors %d, %d\r\n",p[0],p[1]));
    dbgprintf('B',(stderr,"  new W(op): op=%d, fg=%d, bg=%d\n",
		  OPCODE(W(op)),GETFCOLOR(W(op)),GETBCOLOR(W(op))));
    break;
    /*}}}  */
    /*{{{  3 -- ras_write*/
    case 3:
    bit_blit(window,Scalex(p[0]),Scaley(p[1]),Scalex(p[2]),Scaley(p[3]),op,(DATA*)0,0,0);
    if (Do_clip()) Set_clip(Scalex(p[0]),Scaley(p[1]),Scalex(p[0])+Scalex(p[2]),Scaley(p[1])+Scaley(p[3]))
    break;
    /*}}}  */
    /*{{{  4 -- ras_write  specify dest*/
    case 4:
    if (p[4]>MAXBITMAPS) break;
    if (p[4]>0 && W(bitmaps)[p[4]-1]==(BITMAP*)0) W(bitmaps)[p[4]-1] = bit_alloc
    (
      Scalex(p[0])+Scalex(p[2]),Scaley(p[1])+Scaley(p[3]),
      (DATA*)0,
      BIT_DEPTH(W(window))
    );
    bit_blit
    (
      p[4]?W(bitmaps)[p[4]-1]:window,
      Scalex(p[0]),Scaley(p[1]),
      Scalex(p[2]),Scaley(p[3]),
      op,0,0,0
    );
    if (Do_clip() && p[4]==0) Set_clip
    (
      Scalex(p[0]),Scaley(p[1]),
      Scalex(p[0])+Scalex(p[2]),
      Scaley(p[1])+Scaley(p[3])
    );
    break;
    /*}}}  */
    /*{{{  5 -- ras_copy*/
    case 5:
    bit_blit
    (
      window,
      Scalex(p[0]),Scaley(p[1]),
      Scalex(p[2]),Scaley(p[3]),
      op,window,
      Scalex(p[4]),Scaley(p[5])
    );
    if (Do_clip()) Set_clip
    (
      Scalex(p[0]),Scaley(p[1]),
      Scalex(p[0])+Scalex(p[2]),
      Scaley(p[1])+Scaley(p[3])
    );
    break;
    /*}}}  */
    /*{{{  7 -- ras_copy specify dst,src*/
    case 7:
    if (p[6]>MAXBITMAPS || p[7]>MAXBITMAPS) break;

    if (p[6]>0 && W(bitmaps)[p[6]-1]==(BITMAP*)0)
    {
      register int depth;

      /* figure out depth of dest if we need to create it */

      if (p[7]&&W(bitmaps)[p[7]-1]) depth = BIT_DEPTH(W(bitmaps)[p[7]-1]);
      else depth = BIT_DEPTH(W(window));

      W(bitmaps)[p[6]-1] = bit_alloc
      (
        Scalex(p[0])+Scalex(p[2]),
        Scaley(p[1])+Scaley(p[3]),
        NULL_DATA,
        depth
      );
    }
    dbgprintf('B',(stderr,"blitting %d to %d (%d x %d)\r\n",p[7],p[6],p[2],p[3]));
    bit_blit
    (
      p[6]?W(bitmaps)[p[6]-1]:window,
      Scalex(p[0]),Scaley(p[1]),
      Scalex(p[2]),Scaley(p[3]),
      op,
      p[7]?W(bitmaps)[p[7]-1]:window,
      Scalex(p[4]),Scaley(p[5])
    );
    if (Do_clip() && p[6]==0) Set_clip
    (
      Scalex(p[0]),Scaley(p[1]),
      Scalex(p[0])+Scalex(p[2]),
      Scaley(p[1])+Scaley(p[3])
    );
    /*}}}  */
  }
}
/*}}}  */
/*{{{  win_map -- down load a bit map  - parse escape sequence*/
void win_map(WINDOW *win, BITMAP *window)
{
  /*{{{  variables*/
  int cnt = W(esc_cnt);
  int *p = W(esc);
  int op=W(op);
  /*}}}  */

  if (W(code)==T_BITMAP)
  /*{{{  convert external bitmap data from snarf buffer to tmp bitmap*/
  {
    W(bitmap)=bit_load(p[0],p[1],p[4],p[cnt],W(snarf));
    p[4]=p[5];
    p[5]=p[6];
    cnt--;
  }
  /*}}}  */
#ifdef MOVIE
  SET_DIRTY(W(bitmap));
#endif
  switch(cnt)
  {
    /*{{{  2 -- bitmap to graphics point*/
    case 2:
    bit_blit(window,Scalex(W(gx)),Scaley(W(gy)),p[0],p[1],op,W(bitmap),0,0);
    if (Do_clip())
    {
      Set_clip(Scalex(W(gx)),Scaley(W(gy)),Scalex(W(gx))+p[0],Scaley(W(gy))+p[1]);
    }
    break;
    /*}}}  */
    /*{{{  3 -- bitmap to graphics point specify dest*/
    case 3:
    if (p[2] > MAXBITMAPS) break;
    if (p[2]>0 && W(bitmaps)[p[2]-1]==(BITMAP*)0)
    {
      W(bitmaps)[p[2]-1]=W(bitmap);
      W(bitmap)=(BITMAP*)0;
    }
    else bit_blit(p[2]?W(bitmaps)[p[2]-1]:window,Scalex(W(gx)),Scaley(W(gy)),p[0],p[1],op,W(bitmap),0,0);
    break;
    /*}}}  */
    /*{{{  4 -- bitmap to specified point*/
    case 4:
    bit_blit(window,p[2],p[3],p[0],p[1],op,W(bitmap),0,0);
    if (Do_clip()) 
    {
      Set_clip(p[2],p[3],p[2]+p[0],p[3]+p[1]);
    }
    break;
    /*}}}  */
    /*{{{  5 -- bitmap to specified point specify dest*/
    case 5:
    if (p[4]>MAXBITMAPS) break;
    if (p[4]>0 && W(bitmaps)[p[4]-1]==(BITMAP*)0)
    {
      W(bitmaps)[p[4]-1]=W(bitmap);
      W(bitmap)=(BITMAP*)0;
    }
    else bit_blit(p[4]?W(bitmaps)[p[4]-1]:window,p[2],p[3],p[0],p[1],op,W(bitmap),0,0);
    break;
    /*}}}  */
  }
  if (W(bitmap)) { bit_destroy(W(bitmap)); W(bitmap)=(BITMAP*)0; }
  if (W(snarf)) { free(W(snarf)); W(snarf) = NULL; }
}
/*}}}  */
/*{{{  win_plot -- plot a line*/
void win_plot(WINDOW *win, BITMAP *window)
{
  register int *p = W(esc);
  int op;

  op = W(op);
  switch (W(esc_cnt)) 
  {
    /*{{{  0 -- set cursor to graphics point*/
    case 0:
    W(x) = Scalex(W(gx));
    W(y) = Scaley(W(gy));
    break;
    /*}}}  */
    /*{{{  1 -- draw to graphics point*/
    case 1:
    Bit_line(win,window,Scalex(W(gx)),Scaley(W(gy)),Scalex(p[0]),Scaley(p[1]),op);
    W(gx) = p[0];
    W(gy) = p[1];
    break;
    /*}}}  */
    /*{{{  3*/
    case 3:
    Bit_line(win,window,Scalex(p[0]),Scaley(p[1]),Scalex(p[2]),Scaley(p[3]),op);
    W(gx) = p[2];
    W(gy) = p[3];
    break;
    /*}}}  */
    /*{{{  4*/
    case 4:
    if (p[4]==0 || (p[4]>0 && p[4]<=MAXBITMAPS && W(bitmaps)[p[4]-1]))
    bit_line(p[4]?W(bitmaps)[p[4]-1]:window,Scalex(p[0]),Scaley(p[1]),Scalex(p[2]),Scaley(p[3]),op);
    break;
    /*}}}  */
  }
}
/*}}}  */
/*{{{  Bit_line*/
void Bit_line(WINDOW *win, BITMAP *dst,int x1,int y1,int x2,int y2,int op)
{
  bit_line(dst,x1,y1,x2,y2,op);
  if (Do_clip()) 
  {
    Set_clip(x1,y1,x2+1,y2+1);
    Set_clip(x2,y2,x1+1,y1+1);
  }
}
/*}}}  */
/*{{{  grunch -- experimantal graphics crunch mode*/
void grunch(WINDOW *win, BITMAP *dst)
{
  register char *buf = W(snarf);
  register int cnt = W(esc)[W(esc_cnt)];
  int op;
  int penup = 0;
  int *p = W(esc);
  register int x,y,x1,y1;

  op = W(op);

  /* set starting point */

  if (W(esc_cnt) > 1) 
  {
    x =  p[0];
    y =  p[1];
  }
  else 
  {
    x = W(gx);
    y = W(gy);
  }
  while (cnt-- > 0) 
  {
    x1 = (*buf>>4 & 0xf) - 8;
    y1 = (*buf & 0xf) - 8;
    if (x1==0 && y1 ==0)
    penup = 1;
    else if (penup == 0)
    {
      bit_line(dst,Scalex(x),Scaley(y),Scalex(x+x1),Scaley(y+y1),op);
      dbgprintf('y',(stderr,"%s: line [%d] %d,%d + %d,%d\n",W(tty),op,x,y,x1,y1));
      x += x1;
      y += y1;
    }
    else 
    {
      x += x1;
      y += y1;
      penup = 0;
    }
    buf++;
  }
  W(gx) = x;
  W(gy) = y;
}
/*}}}  */
/*{{{  circle_plot -- plot a circle*/
void circle_plot(WINDOW *win,BITMAP *window)
{
  register int *p = W(esc);
  int op;

  op = W(op);

  switch (W(esc_cnt)) 
  {
    /*{{{  0 -- draw a 'circle'  at graphics point*/
    case 0:
    circle(window,Scalex(W(gx)),Scaley(W(gy)),Scalexy(p[0]),op);
    break;
    /*}}}  */
    /*{{{  1 -- draw an 'ellipse' at graphics point*/
    case 1:		/* draw an 'ellipse' at graphics point */
    ellipse(window, Scalex(W(gx)), Scaley(W(gy)),
    Scalex(p[0]), Scaley(p[1]), op);
    break;
    /*}}}  */
    /*{{{  2 -- draw a 'circle'*/
    case 2:
    circle(window,Scalex(p[0]),Scaley(p[1]),Scalexy(p[2]),op);
    break;
    /*}}}  */
    /*{{{  3 -- draw an 'ellipse'*/
    case 3:
    ellipse(window, Scalex(p[0]), Scaley(p[1]),
    Scalex(p[2]), Scaley(p[3]), op);
    break;
    /*}}}  */
    /*{{{  4 -- draw an 'ellipse' to offscreen bitmap*/
    case 4:
    if (p[4]>0 && p[4]<=MAXBITMAPS && W(bitmaps)[p[4]-1])
    ellipse(W(bitmaps)[p[4]-1], Scalex(p[0]), Scaley(p[1]),
    Scalex(p[2]), Scaley(p[3]), op);
    break;
    /*}}}  */
    /*{{{  5 -- draw an arc  ccw centered at p0,p1*/
    case 5:
    arc(window, Scalex(p[0]), Scaley(p[1]), Scalex(p[2]), Scaley(p[3]),
    Scalex(p[4]), Scaley(p[5]), op);
    break;
    /*}}}  */
    /*{{{  6 -- draw an arc  ccw centered at p0,p1  to offscreen bitmap*/
    case 6:
    if (p[6]>0 && p[6]<=MAXBITMAPS && W(bitmaps)[p[6]-1])
    arc(W(bitmaps)[p[6]-1], Scalex(p[0]), Scaley(p[1]), Scalex(p[2]),
    Scaley(p[3]), Scalex(p[4]), Scaley(p[5]), op);
    break;
    /*}}}  */
  }
  if (Do_clip())
  Set_all();
}
/*}}}  */
/*{{{  win_go -- move the graphics pointer*/
void win_go(WINDOW *win)
{
  register int *p = W(esc);

  switch (W(esc_cnt)) 
  {
    /*{{{  0 -- set the graphics point to cursor pos*/
    case 0:
    W(gx) = W(x);
    W(gy) = W(y);
    break;
    /*}}}  */
    /*{{{  1,2 -- set the graphics point*/
    case 1:
    case 2:
    W(gx) =  p[0];
    W(gy) =  p[1];
    break;
    /*}}}  */
  }
}
/*}}}  */
#if 0
/*{{{  bmap_size -- determine bitmap style from size*/
int bmap_size(int w,int h,int count)
{
  int format;    /* bitmap format */
  int bytes = count/h;		/* bytes/line */

  if (bytes == ((w+31)&~31))  /* 8 bit, 32 bit aligned */
  format=31 | 0x80;
  else if (bytes == ((w+15)&~15)) /* 8 bit 16 bit aligned */
  format=15 | 0x80;
  else if (bytes == ((w+7)&~7)) /* 8 bit 8 bit aligned */
  format=7 | 0x80;
  else if (bytes*8 == ((w+31)&~31))  /* 1 bit, 32 bit aligned */
  format=31;
  else if (bytes*8 == ((w+15)&~15)) /* 1 bit 16 bit aligned */
  format=15;
  else if (bytes*8 == ((w+7)&~7)) /* 1 bit 8 bit aligned */
  format=7;
  else                             /* unknown format */
  format=0;
  return(format);
}
/*}}}  */
#endif
