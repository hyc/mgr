/*{{{}}}*/
/*{{{  Notes*/
/*                        Copyright (c) 1987 Bellcore
 *                            All Rights Reserved
 *       Permission is granted to copy or use this program, EXCEPT that it
 *       may not be sold for profit, the copyright notice must be reproduced
 *       on copies, and credit should be given to Bellcore where it is due.
 *       BELLCORE MAKES NO WARRANTY AND ACCEPTS NO LIABILITY FOR THIS PROGRAM.
 */

/* push - pop window environment */
/*}}}  */
/*{{{  #includes*/
#include <mgr/bitblit.h>
#include <mgr/font.h>
#include <stdlib.h>
#include <stdio.h>

#include "defs.h"
#include "event.h"
#include "menu.h"

#include "border.h"
#include "destroy.h"
#include "do_event.h"
#include "font_subs.h"
#include "get_menus.h"
#include "icon_server.h"
#include "print.h"
#include "put_window.h"
#include "shape.h"
#include "subs.h"
/*}}}  */
/*{{{  #defines*/
#define S(x)	(stack->x)
/*}}}  */

/*{{{  win_push -- push a window on the environment stack*/
int
win_push(win,level)
register WINDOW *win;		/* window to push */
int level;			/* what things to push */
   {
   register WINDOW *stack;	/* pushed window goes here */
   register int i,j;

   if ((stack = (WINDOW *) malloc(sizeof(WINDOW))) == (WINDOW *) 0)
      return(-1);

   if (level == 0)
      level = P_DEFAULT;

   dbgprintf('P',(stderr,"%s Stacking %s\n",W(tty),print_stack(level)));

   /* setup stacked window */

   (void) memcpy(stack,win,sizeof(WINDOW));
   W(stack) = stack;

   for(j=0;j<MAXMENU;j++)
      S(menus)[j] = (struct menu_state *) 0;
   for(j=0;j<MAXEVENTS;j++)
      S(events)[j] = (char *) 0;
   for(j=0;j<MAXBITMAPS;j++)
      S(bitmaps)[j] = (BITMAP *) 0;
   S(save) = (BITMAP *) 0;
   S(clip_list) = (char *) 0;

   /* setup each pushed item */

   for(i=1;i!=P_MAX;i <<= 1)
      switch (level&i) {
         case P_MENU:		/* save the menus */
              dbgprintf('P',(stderr,"  menus "));
              for(j=0;j<MAXMENU;j++)
                 if (W(menus)[j] && (level&P_CLEAR)) {
                    S(menus)[j] = W(menus)[j];
                    W(menus)[j] = (struct menu_state *) 0;
                    dbgprintf('P',(stderr,"%d ",j));
                    }
                 else if (W(menus)[j]) {
                    S(menus)[j] = menu_copy(W(menus)[j]);
                    dbgprintf('P',(stderr,"%d ",j));
                    }
              dbgprintf('P',(stderr,"\n"));
              break;
         case P_EVENT:		/* save the events */

              dbgprintf('P',(stderr,"  events "));
              if (IS_EVENT(win,EVENT_STACK))
                 EVENT_SET_MASK(win,EVENT_STFLAG);

              if (level&P_CLEAR) 
                 W(event_mask) = IS_EVENT(win,EVENT_STFLAG);
              else
                 EVENT_CLEAR_MASK(win,EVENT_STACK);

              for(j=0;j<MAXEVENTS;j++)
                 if (W(events)[j] && (level&P_CLEAR)) {
                    S(events)[j] = W(events)[j]; 
                    W(events)[j] = (char *) 0;
                    dbgprintf('P',(stderr,"%d ",j));
                    }
                 else if (W(events)[j]) {
                    S(events)[j] = strcpy(malloc(strlen(W(events)[j])+1),W(events)[j]);
                    dbgprintf('P',(stderr,"%d ",j));
                    }
              dbgprintf('P',(stderr,"\n"));
              break;
         case P_CURSOR:		/* restore the cursor style */
              if (level&P_CLEAR)
                 W(curs_type) = CS_BLOCK;
              break;
         case P_BITMAP:		/* save the bitmaps */
              dbgprintf('P',(stderr,"  bitmaps "));
              for(j=0;j<MAXBITMAPS;j++)
                 if (W(bitmaps)[j] && level&P_CLEAR) {
                    S(bitmaps)[j] = W(bitmaps)[j];
                    W(bitmaps)[j] = (BITMAP *) 0;
                    dbgprintf('P',(stderr,"%d ",j));
                    }
                 else if (W(bitmaps)[j]) {
                    S(bitmaps)[j] = bit_alloc(BIT_WIDE(W(bitmaps)[j]),
                                       BIT_HIGH(W(bitmaps)[j]),NULL_DATA,
                                       BIT_DEPTH(W(bitmaps)[j]));
                    bit_blit(S(bitmaps)[j],0,0,BIT_WIDE(W(bitmaps)[j]),
                             BIT_HIGH(W(bitmaps)[j]),BIT_SRC,W(bitmaps)[j],0,0);
                    dbgprintf('P',(stderr,"%d ",j));
                    }
              dbgprintf('P',(stderr,"\n"));
              break;
         case P_WINDOW:		/* save the bit image */
              dbgprintf('P',(stderr,"  window\n"));
              S(save) = bit_alloc(BIT_WIDE(W(border)),BIT_HIGH(W(border)),
                                  NULL_DATA,BIT_DEPTH(W(window)));
              if (W(save) && !(W(flags)&W_ACTIVE))
                 bit_blit(S(save),0,0,BIT_WIDE(W(save)),BIT_HIGH(W(save)),
                          BIT_SRC,W(save),0,0);
              else
                 bit_blit(S(save),0,0,BIT_WIDE(W(border)),BIT_HIGH(W(border)),
                          BIT_SRC,W(border),0,0);
              break;
         case P_POSITION:	/* save the window position */
              dbgprintf('P',(stderr,"  position\n"));
              S(esc)[1] = BIT_WIDE(W(border));
              S(esc)[2] = BIT_HIGH(W(border));
              break;
         case P_TEXT:		/* save text region */
              if (level&P_CLEAR) {
                 W(text).x = 0;
                 W(text).y = 0;
                 W(text).wide = 0;
                 W(text).high = 0;
                 set_size(win);
                 }
              break;
         case P_MOUSE:		/* save mouse position */
              dbgprintf('P',(stderr,"  mouse\n"));
              S(esc)[3] = mousex;
              S(esc)[4] = mousey;
	      if (level&P_CLEAR) {
		 S(cursor) = W(cursor);
		 W(cursor) = &mouse_arrow;
		 }
	      else if (IS_STATIC(W(cursor))) {
		 S(cursor) = W(cursor);
		 }
	      else {
		 /* user-defined cursor shape */
		 S(cursor) = bit_alloc(BIT_WIDE(W(cursor)),BIT_HIGH(W(cursor)),
		 		       NULL_DATA,BIT_DEPTH(W(cursor)));
		 bit_blit(S(cursor),0,0,BIT_WIDE(W(cursor)),BIT_HIGH(W(cursor)),
		 	  BIT_SRC,W(cursor),0,0);
		 }
              break;
         case P_FLAGS:		/* save window flags  */
              if (level&P_CLEAR) {
                 W(flags)  &= ~W_SAVE;
                 W(flags)  |= W_BACKGROUND;
                 W(style) = PUTOP(BIT_SRC,W(style));
		 border(win,win==active?BORDER_FAT:BORDER_THIN);
                 W(dup) = '\0';		/* clear the dupkey mode */
                 }
              break;
         case P_COLOR:		/* save the colors  */
              if (level&P_CLEAR) {
                 W(style) = PUTOP(W(style),BIT_SRC);
                 W(op) = PUTOP(W(op),BIT_OR);
                 }
              break;
         case P_BITOP:		/* save the bitblit ops  */
              if (level&P_CLEAR) {
                 W(style) = PUTOP(BIT_SRC,W(style));
                 W(op) = PUTOP(BIT_OR,W(op));
                 }
              break;
         }

   S(code) = level;
   S(window) = (BITMAP *) 0;
   S(border) = (BITMAP *) 0;
   S(snarf) = (char *) 0;
   S(bitmap) = (BITMAP *) 0;
   return(level);
   }
/*}}}  */
/*{{{  win_pop -- pop the window stack */
int
win_pop(win)
WINDOW *win;				/* window to pop to */
   {
   register int i,j;
   register WINDOW *stack = W(stack);	/* window to pop from */

   if (stack == (WINDOW *) 0) {
      dbgprintf('P',(stderr,"  No environment to pop\n"));
      return(-1);
      }

   dbgprintf('P',(stderr,"%s popping %s\n",W(tty),print_stack(S(code))));

   /* pop each item stacked */

   for(i=1;i!=P_MAX;i <<= 1)
      switch (S(code)&i) {
         case P_MENU:		/* restore the menus */
              dbgprintf('P',(stderr,"  menus "));
              W(menu[0]) = S(menu[0]);
              W(menu[1]) = S(menu[1]);
              for(j=0;j<MAXMENU;j++) {
                 if (W(menus)[j]) {
                    dbgprintf('P',(stderr,"d(%d) ",j));
                    menu_destroy(W(menus)[j]);
                    }
                 if (S(menus)[j]) {
                    dbgprintf('P',(stderr,"r(%d) ",j));
                    W(menus)[j] = S(menus)[j];
                    S(menus)[j] = (struct menu_state *) 0;
                    }
                 else
                    W(menus)[j] = (struct menu_state *) 0;
                 }
              dbgprintf('P',(stderr,"\n"));
              break;
         case P_EVENT:		/* restore the events */

              dbgprintf('P',(stderr,"  events "));
              for(j=0;j<MAXEVENTS;j++) {
                 if (W(events)[j]) {
                    dbgprintf('P',(stderr,"d(%d) ",j));
                    free(W(events)[j]);
                    }
                 W(events)[j] = S(events)[j];
                 S(events)[j] = (char *) 0;
                 }
              W(event_mask) = S(event_mask);
              dbgprintf('P',(stderr,"\n"));
              break;
         case P_CURSOR:		/* restore the cursor position */
              W(x) = S(x);
              W(y) = S(y);
              W(gx) = S(gx);
              W(gy) = S(gy);
              W(curs_type) = S(curs_type);
              break;
         case P_BITMAP:		/* restore the bitmaps */
              for(j=0;j<MAXBITMAPS;j++) {
                 if (W(bitmaps)[j])
                    bit_destroy(W(bitmaps)[j]);
                 W(bitmaps)[j] = S(bitmaps)[j];
                 S(bitmaps)[j] = (BITMAP *) 0;
                 }
              dbgprintf('P',(stderr,"  bitmaps\n"));
              break;
         case P_FONT:		/* restore font */
              W(font) = S(font);
              break;
         case P_TEXT:		/* restore text region */
              W(text) = S(text);
              set_size(win);
              break;
         case P_POSITION:	/* restore the window position */
              if (win!= active)
                 cursor_off();
              ACTIVE_OFF();
              expose(win);

              shape(S(x0),S(y0),S(esc)[1],S(esc)[2]);

              ACTIVE_ON();
              dbgprintf('P',(stderr,"  position\n"));
              break;
         case P_WINDOW:		/* restore the window contents */
              if (W(save))
                 bit_destroy(W(save));
              W(save) = bit_alloc(BIT_WIDE(W(border)),BIT_HIGH(W(border)),
                                  NULL_DATA,BIT_DEPTH(W(window)));
              bit_blit(W(border),0,0,BIT_WIDE(S(save)),BIT_HIGH(S(save)),
                       BIT_SRC,S(save),0,0);
              dbgprintf('P',(stderr,"  window\n"));
              break;
         case P_FLAGS:		/* restore the window flags */
              W(op) = PUTOP(S(op),W(op));
              W(style) = PUTOP(S(style),W(style));
              W(dup) = S(dup);
              W(flags) = (S(flags)&W_SAVE) | (W(flags)&(~W_SAVE));
	      border(win,win==active?BORDER_FAT:BORDER_THIN);
              dbgprintf('P',(stderr,"  flags\n"));
              break;
         case P_COLOR:		/* restore the colors */
              W(op) = PUTOP(W(op),S(op));
              W(style) = PUTOP(W(style),S(style));
              dbgprintf('P',(stderr,"  colors\n"));
              break;
         case P_BITOP:		/* restore the bitblit ops */
              W(op) = PUTOP(S(op),W(op));
              W(style) = PUTOP(S(style),W(style));
              dbgprintf('P',(stderr,"  bitblit ops\n"));
              break;
         case P_MOUSE:		/* save mouse position */
              dbgprintf('P',(stderr,"  mouse\n"));
              mousex =S(esc)[3] ;
              mousey =S(esc)[4] ;
              bit_destroy(W(cursor));	/* no op if static */
              W(cursor) = S(cursor);
              S(cursor) = 0;
              break;
         }
   dbgprintf('P',(stderr,"%s\n",S(stack)?"another stack":"no environments stacked"));
   W(stack) = S(stack);
   unlink_win(stack,0);

   return(0);
   }
/*}}}  */
