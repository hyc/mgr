/*{{{}}}*/
/*{{{  Notes*/
/*                        Copyright (c) 1987 Bellcore
 *                            All Rights Reserved
 *       Permission is granted to copy or use this program, EXCEPT that it
 *       may not be sold for profit, the copyright notice must be reproduced
 *       on copies, and credit should be given to Bellcore where it is due.
 *       BELLCORE MAKES NO WARRANTY AND ACCEPTS NO LIABILITY FOR THIS PROGRAM.
*/

/* re-shape a window */
/*}}}  */
/*{{{  #includes*/
#include <mgr/bitblit.h>
#include <mgr/font.h>
#include <stdio.h>

#include "clip.h"
#include "defs.h"
#include "event.h"

#include "border.h"
#include "do_button.h"
#include "do_event.h"
#include "erase_win.h"
#include "font_subs.h"
#include "get_rect.h"
#include "icon_server.h"
#include "intersect.h"
#include "put_window.h"
#include "scroll.h"
#include "subs.h"
#include "update.h"
/*}}}  */

/*{{{  shape -- reshape a window to specified dimentions*/
int shape(int x, int y, int dx, int dy)
{
  int sx,sy,w,h;
  register WINDOW *win;

  if (dx>0)
  {
    sx= x; w = dx;
  }
  else 
  {
    sx= x+dx; w = -dx;
  }
  if (dy>0) 
  {
    sy= y; h = dy;
  }
  else 
  {
    sy= y+dy; h = -dy;
  }

  if (sx < 0) sx = 0;
   
  if (sx + w >= BIT_WIDE(screen))
  w = BIT_WIDE(screen) - sx;

  if (sy + h >= BIT_HIGH(screen))
  h = BIT_HIGH(screen) - sy;

  if (w < 2*ACTIVE(borderwid) + ACTIVE(font)->head.wide*MIN_X ||
      h < 2*ACTIVE(borderwid) + ACTIVE(font)->head.high*MIN_Y)
    return(-1);

#ifdef MGR_ALIGN
  alignwin(screen,&sx,&w,ACTIVE(borderwid));
#endif

  /* remove current window position */
  save_win(active);
  erase_win(ACTIVE(border));
  clip_bad(active);	/* invalidate clip lists */

  /* redraw remaining windows */
  repair(active);

  /* adjust window state */
  ACTIVE(x0) = sx;
  ACTIVE(y0) = sy;
  bit_destroy(ACTIVE(window));
  bit_destroy(ACTIVE(border));
  ACTIVE(border) = bit_create(screen,sx,sy,w,h);
  ACTIVE(window) = bit_create(ACTIVE(border),
			      ACTIVE(borderwid),
			      ACTIVE(borderwid),
			      w-ACTIVE(borderwid)*2,
			      h-ACTIVE(borderwid)*2);

  for(win=ACTIVE(next);win != (WINDOW *) 0;win=W(next)) 
  {
    if (W(flags)&W_ACTIVE && intersect(active,win))
    save_win(win);
  }

  CLEAR(ACTIVE(window),PUTOP(BIT_CLR,ACTIVE(style)));

  border(active,BORDER_THIN);
  bit_blit(ACTIVE(border),0,0,
	   BIT_WIDE(ACTIVE(save))-ACTIVE(borderwid),
	   BIT_HIGH(ACTIVE(save))-ACTIVE(borderwid),
	   BIT_SRC,ACTIVE(save),0,0);

  /* make sure character cursor is in a good spot */
  if (ACTIVE(x) > BIT_WIDE(ACTIVE(window))) 
  {
    ACTIVE(x) = 0;
    ACTIVE(y) += ((int)(ACTIVE(font)->head.high));
  }
  if (ACTIVE(y) > BIT_HIGH(ACTIVE(window))) 
  {
#ifdef WIERD
    ACTIVE(y) = BIT_HIGH(ACTIVE(window));
    scroll(ACTIVE(window),0,BIT_HIGH(ACTIVE(window)),
    ((int)(ACTIVE(font)->head.high)),SWAPCOLOR(ACTIVE(style)));
    bit_blit(ACTIVE(window),0,BIT_HIGH(ACTIVE(window))-((int)(ACTIVE(font)->head.high)),
	     BIT_WIDE(ACTIVE(save)),((int)(ACTIVE(font)->head.high)),
	     BIT_SRC,ACTIVE(save),
	     ACTIVE(borderwid),BIT_HIGH(ACTIVE(save))-((int)(ACTIVE(font)->head.high))-ACTIVE(borderwid));
#else
    ACTIVE(y) = BIT_HIGH(ACTIVE(window))-((int)(ACTIVE(font)->head.high));
#endif
  }

  bit_destroy(ACTIVE(save));
  ACTIVE(save)=(BITMAP *)0;

  /* invalidate clip lists */
  clip_bad(active);
  un_covered();
  set_size(active);
  return(0);
}
/*}}}  */
/*{{{  shape_window -- reshape a window with the mouse*/
void shape_window(void)
{
  int dx=16 ,dy=16 ;

  SETMOUSEICON(&mouse_box);
  move_mouse(screen,mouse,&mousex,&mousey,0);
  SETMOUSEICON(DEFAULT_MOUSE_CURSOR);
  get_rect(screen,mouse,mousex,mousey,&dx,&dy,0);
  do_button(0);

  /* look for shape event here */
  do_event(EVENT_SHAPE,active,E_MAIN);

  (void) shape(mousex,mousey,dx,dy);
}
/*}}}  */
/*{{{  stretch_window -- stretch a window with the mouse*/
#ifdef STRETCH
void stretch_window(void)
{
  int dx, dy;
  int x0, x1, y0, y1;

  SETMOUSEICON(&mouse_box);
  move_mouse(screen,mouse,&mousex,&mousey,0);
  SETMOUSEICON( DEFAULT_MOUSE_CURSOR);
  
  x0 = ACTIVE(x0);
  y0 = ACTIVE(y0);
  x1 = x0 + BIT_WIDE(ACTIVE(border));
  y1 = y0 + BIT_HIGH(ACTIVE(border));
  if( 2*(mousex - x0) < x1 - x0)
    x0 = x1;
  dx = mousex - x0;
  if( 2*(mousey - y0) < y1 - y0)
    y0 = y1;
  dy = mousey - y0;
  /* x0,y0 is corner farthest from mouse. x0+dx,y0+dx is mouse position */

  get_rect(screen,mouse,x0,y0,&dx,&dy,0);
  do_button(0);

  /* look for shape event here */
  do_event(EVENT_SHAPE,active,E_MAIN);

  (void) shape(x0,y0,dx,dy);
}
#endif /* STRETCH */
/*}}}  */
