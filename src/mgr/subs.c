/*{{{}}}*/
/*{{{  Notes*/
/*                        Copyright (c) 1987 Bellcore
 *                            All Rights Reserved
 *       Permission is granted to copy or use this program, EXCEPT that it
 *       may not be sold for profit, the copyright notice must be reproduced
 *       on copies, and credit should be given to Bellcore where it is due.
 *       BELLCORE MAKES NO WARRANTY AND ACCEPTS NO LIABILITY FOR THIS PROGRAM.
 */

/* misc window and screen mangement routines */
/*}}}  */
/*{{{  #includes*/
#include <mgr/bitblit.h>
#include <mgr/font.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>

#ifdef sun
#include <sundev/kbio.h>
#endif

#include "clip.h"
#include "defs.h"
#include "event.h"

#include "proto.h"
#include "subs.h"
#include "border.h"
#include "do_button.h"
#include "do_event.h"
#include "erase_win.h"
#include "font_subs.h"
#include "icon_server.h"
#include "intersect.h"
#include "kbd.h"
#include "mgr.h"
#include "mouse_get.h"
#include "set_mode.h"
#include "sigdata.h"
#include "startup.h"
#include "update.h"
/*}}}  */

/*{{{  set_covered -- deactivate all windows covered by win*/
void set_covered(check)
register WINDOW *check;			/* window to check covering against */
   {
   register WINDOW *win;

   for(win=active;win != (WINDOW *) 0;win=win->next)
       if (win!=check && intersect(win,check) && W(flags)&W_ACTIVE) {
          save_win(win);
          do_event(EVENT_COVERED,win,E_MAIN);
          W(flags) &= ~W_ACTIVE;
          if (!(W(flags)&W_LOOK))
             FD_CLR( W(to_fd), &mask);
          dbgprintf('o',(stderr,"\t%s covers %s\r\n",check->tty,W(tty)));
          }
   }
/*}}}  */
/*{{{  un_covered -- find and activate all windows previously covered by win*/
void un_covered()
   {
   register WINDOW *win,*check;
   register int cover;

   for(win=active;win != (WINDOW *) 0;win=W(next)) {
      dbgprintf('U',(stderr,"	invalidate cliplist: %s)\r\n",W(tty)));
      dbgprintf('o',(stderr,"	un_cover: %s)\n",W(tty)));
      for(cover=0,check=active;check!=win && cover==0;check=check->next)
         if (intersect(win,check)) cover=1;

      if (cover && W(flags)&W_ACTIVE) {
          do_event(EVENT_COVERED,win,E_MAIN);
          W(flags) &= ~W_ACTIVE;
          if (!(W(flags)&W_LOOK))
             FD_CLR( W(to_fd), &mask);
          dbgprintf('o',(stderr,"becoming inactive (covered by %s)\r\n",
			check->tty));
          }
      else if (!cover && !(W(flags)&W_ACTIVE)) {
          do_event(EVENT_UNCOVERED,win,E_MAIN);
          W(flags) |= W_ACTIVE;
          if (!(W(flags)&W_DIED))
             FD_SET( W(to_fd), &mask);
          dbgprintf('o',(stderr,"becoming active\r\n"));
          }
      else if (cover && !(W(flags)&W_ACTIVE))  {
         dbgprintf('o',(stderr,"remains inactive (covered by %s)\r\n",
		       check->tty));
         }
      else if (!cover && W(flags)&W_ACTIVE) {
         dbgprintf('o',(stderr,"remains active\r\n"));
         }
      else {
	 dbgprintf('o',(stderr,"%s: unknown covering state\r\n",W(tty)));
	 }
      }
   }
/*}}}  */
/*{{{  expose -- bring a window to the top*/
void expose(win)
register WINDOW *win;			/* window to expose */
   {
   dbgprintf('o',(stderr,"exposing %s\r\n",W(tty)));
   
   /* reorder windows */

   if (win == active) return;

   W(prev)->next = W(next);
   if (W(next))
      W(next)->prev = W(prev);
   else
      ACTIVE(prev) = W(prev);

   W(prev) = ACTIVE(prev);
   W(next) = active;

   ACTIVE(prev) = win;
   active = win;

   if (!(W(flags)&W_ACTIVE)) {
      for(win=active->next;win!=(WINDOW *) 0;win=W(next))
         if (W(flags)&W_ACTIVE && intersect(active,win))
            save_win(win);
   
      restore_win(active);
   
      clip_bad(active);	/* invalidate clip lists */
      un_covered();
      }
   else {
      dbgprintf('o',(stderr,"expose: %s already active (0%o)\r\n",
		    ACTIVE(tty),(unsigned)ACTIVE(flags)));
      }
   {
      int mouse_was_on = mouse_on;
      /* I think the mouse is always off here, but I cannot prove it. vpb */
      MOUSE_OFF(screen,mousex,mousey);
      SETMOUSEICON( DEFAULT_MOUSE_CURSOR);	/* because active win chg */
      if( mouse_was_on)  MOUSE_ON(screen,mousex,mousey);
   }
   }
/*}}}  */
/*{{{  bury -- move a window at the bottom of window list*/
int
bury(win)
register WINDOW *win;			/* window to bury */
   {
   dbgprintf('o',(stderr,"burying %s\r\n",W(tty)));
   if (!win || !W(next))
      return(0);

   if (win == active)
      active = W(next);

   W(prev)->next = W(next);
   W(next)->prev = W(prev);

   W(prev) = ACTIVE(prev);
   ACTIVE(prev)->next = win;

   ACTIVE(prev) = win;
   W(next) = (WINDOW *) 0;
   return(1);
   }
/*}}}  */
/*{{{  hide -- bury a window at the bottom of the screen*/
void hide(win)
register WINDOW *win;			/* window to hide */
   {
   dbgprintf('o',(stderr,"hiding %s\r\n",W(tty)));
   if (bury(win)==0) return;
   save_win(win);
   repair(win);
   clip_bad(active);	/* invalidate clip lists */
   SETMOUSEICON( DEFAULT_MOUSE_CURSOR);	/* because active win chg */
   }
/*}}}  */
/*{{{  repair -- repair effects of buried window*/
void repair(WINDOW *clip)
   {
   register WINDOW *win;
#ifdef NOCLIP
   for(win=ACTIVE(prev)->prev;win!=active;win=W(prev))
      if (!alone(win)) restore_win(win);
   restore_win(win);
#else
   for(win=clip->prev;win!=active;win=W(prev))
       if (intersect(clip,win))
          clip_win(win,clip);
    if (clip!= active && intersect(clip,active))
       clip_win(active,clip);
#endif
   un_covered();
   }
/*}}}  */
/*{{{  save_win -- save a pixel image of the window*/
void save_win(WINDOW *win)
   {
   dbgprintf('o',(stderr,"\t\t  saving %s\r\n",W(tty)));
   if (W(save) == (BITMAP *) 0) {
      W(save) = bit_alloc(BIT_WIDE(W(border)),BIT_HIGH(W(border)),
                          (DATA*)0,BIT_DEPTH(W(window)));
      }
   else if (BIT_WIDE(W(save)) != BIT_WIDE(W(border))  ||
            BIT_HIGH(W(save)) != BIT_HIGH(W(border))) {
      dbgprintf('o',(stderr,"Saved window %s mismatch\r\n",W(tty)));
      bit_destroy(W(save));
      W(save) = bit_alloc(BIT_WIDE(W(border)),BIT_HIGH(W(border)),
                          (DATA*)0,BIT_DEPTH(W(window)));
      }

   bit_blit(W(save),0,0,BIT_WIDE(W(border)),BIT_HIGH(W(border)),
          BIT_SRC,W(border),0,0);
   }
/*}}}  */
/*{{{  clip_win -- partially restore a previously saved pixel image of the window*/
#define C(x)	(clip->x)

void
clip_win(win,clip)
register WINDOW *win;			/* window to restore to screen */
register WINDOW *clip;			/* clip window */
   {
   int x0 = Max(W(x0),C(x0)) - W(x0);
   int y0 = Max(W(y0),C(y0)) - W(y0);
   int x1 = Min(W(x0)+BIT_WIDE(W(border)),C(x0)+BIT_WIDE(C(border))) - W(x0);
   int y1 = Min(W(y0)+BIT_HIGH(W(border)),C(y0)+BIT_HIGH(C(border))) - W(y0);

   if (W(save) != (BITMAP *) 0) {

/*	******* look at clipping region **********
      bit_blit(W(border),x0,y0,x1-x0,y1-y0 ,
               BIT_NOT(BIT_DST),W(save),x0,y0);
      getchar();
end of debug */

      bit_blit(W(border),x0,y0,x1-x0,y1-y0,
               BIT_SRC,W(save),x0,y0);
      }
#ifdef DEBUG
   else
      if( debug )
	 fprintf(stderr,"clip: can't restore %s\r\n",W(tty));
   dbgprintf('o',(stderr,"\t\t  restore %s (clip to %s)\r\n",W(tty),C(tty)));
#endif
   }
/*}}}  */
/*{{{  restore_win -- restore a previously saved pixel image of the window*/
void
restore_win(win)
register WINDOW *win;			/* window to restore to screen */
   {
   if (W(save) != (BITMAP *) 0)
   bit_blit(W(border),0,0,BIT_WIDE(W(border)),BIT_HIGH(W(border)),
          BIT_SRC,W(save),0,0);
   dbgprintf('o',(stderr,"\t\t  restoring %s\r\n",W(tty)));
   }
/*}}}  */
/*{{{  move_mouse*/
/*****************************************************************************
 *	move the mouse, keep exclusive control 
 *	"how" specifies how we recognize completion:
 *		how == 0:	all buttons were up at start of action,
 *				any button pushed down completes the action.
 *		how != 0:	some button was down at start of action,
 *				all buttons released completes the action.
 */

int
move_mouse(screen,mouse,x,y,how)
BITMAP *screen;
int mouse, *x, *y;
int how;
   {
   register int mx = *x, my = *y;
   register int button = 0;
   int dx,dy;
   MOUSE_ON(screen,mx,my);
   do {
      button=mouse_get(mouse,&dx,&dy);
      MOUSE_OFF(screen,mx,my);
      mx += dx;
      my -= dy;
      mx = BETWEEN(0,mx,BIT_WIDE(screen)); 
      my = BETWEEN(0,my,BIT_HIGH(screen)); 
      MOUSE_ON(screen,mx,my);
      }
   while (how ? button!= 0 : button==0);
   if( how )
	do_button( 0 );
   MOUSE_OFF(screen,mx,my);
   *x = mx;
   *y = my;
   return(button);
   }
/*}}}  */
/*{{{  parse -- parse a line into fields*/
#define iswhite(x)	(strchr(" \t",x))

int
parse(line,fields)
register char *line;
register char **fields;
   {
   int inword = 0;
   int count = 0;
   char *start;
   register char c;

   for(start = line;(c = *line) && c != '\n';line++)
      if (inword && iswhite(c)) {
         inword = 0;
         *line = '\0';
         *fields++ = start;
         count++;
         }
      else if (!inword && !iswhite(c)) {
         start = line;
         inword = 1;
         }

   if (inword) {
      *fields++ = start;
      count++;
      if (c == '\n')
         *line = '\0';
      }
   *fields = (char *) 0;
   return(count);
   }
/*}}}  */
/*{{{  trans -- parse a string to interpret \'s*/
char *
trans(s)
char *s;
   {
   char *result = s;
   register int i=0;
   register char c;
   register int got_slash=0;

   while((c = (*s++)&0x7f)) {
      if (got_slash){
         switch(c) {
            case 'e':
            case 'E': result[i++] = '\033'; break;
            case 'n': result[i++] = '\n';   break;
            case 'r': result[i++] = '\r';   break;
            case 'b': result[i++] = '\b';   break;
            case 'f': result[i++] = '\f';   break;
            case 'g': result[i++] = '\007'; break;
            case 's': result[i++] = ' ';    break;
            case '\\': result[i++] = '\\';  break;
            case 'M': result[i++] = *s++|0x80; break;
            default:  result[i++] = c;      break;
            }
         got_slash = 0;
         }
      else if (c=='\\')
         got_slash++;
      else 
         result[i++] = c;
      } 
   result[i] = '\0';
   return(result);
   }
/*}}}  */
/*{{{  set_console*/
void set_console(WINDOW *win, int on)
{
  /*{{{  variables*/
#  ifdef TIOCCONS
  int one = 1;
  int cfd;
#  endif
  WINDOW *run;
  /*}}}  */

  /*{{{  TIOCCONS*/
#  ifdef TIOCCONS
#  ifdef sun
  /*{{{  make sure kbd is in direct mode*/
  {
  int kbd;
  int mode = 0;

  if ((kbd = open("/dev/kbd",O_RDONLY)) < 0 ) fprintf(stderr,"can't open keyboard\n");
  else if (ioctl(kbd,KIOCGDIRECT,&mode) < 0 ) fprintf(stderr,"can't get keyboard status\n");
  else if (mode != 1) fprintf(stderr,"keyboard not in direct mode\n");
  close(kbd);
  }
  /*}}}  */
#  endif
  cfd=open("/dev/console",O_RDWR);
#  ifndef linux
  if (!on)
#  endif
  ioctl(cfd,TIOCCONS,&one);
  close(cfd);
#  endif
  /*}}}  */
  if (on)
  {
    for (run=active; run!=(WINDOW*)0; run=run->next) run->flags&=~W_CONSOLE;
    win->flags|=W_CONSOLE;
    /*{{{  TIOCCONS*/
#    ifdef TIOCCONS
    if (ioctl(win->to_fd,TIOCCONS,&one)<0) fprintf(stderr,"can't set new console device\n");
#    endif
    /*}}}  */
  }
  else
  {
    win->flags&=~W_CONSOLE;
  }
}
/*}}}  */
/*{{{  suspend -- suspend MGR*/
void suspend(void)
   {
#ifdef SIGSTOP
   register WINDOW *win;

   MOUSE_OFF(screen,mousex,mousey);
   sleep(1);	/* give the key time to go up */
   set_kbd(0);	/* fix up keyboard modes */

   for(win=active;win!=(WINDOW *) 0;win=win->next) {
      killpg(W(pid),SIGSTOP);
      if (W(flags)&W_ACTIVE)
         save_win(win);
      }

   bit_textscreen();
   reset_tty(0);
   kbd_reset();
   close(mouse);
   reset_tty(0);

   do_cmd( 's' );	/* do the suspention command */

   /* courtesy DVW */
   signal(SIGTSTP, SIG_DFL);
   kill(0,SIGTSTP);			/* send stop signal to self */
   sleep(1);				/* wait for CONT signal */
   signal(SIGTSTP, catch);

   if (set_kbd(1) != 0) {	/* reopen kbd (as 0) */
      _quit();
      fprintf(stderr,"Sorry, Can't reopen kbd\n");
      exit(1);
      }
   mouse = mouse_reopen();
   set_tty(0);
   bell_on();	/* this resets the keyboard! */

   bit_grafscreen();
   do_cmd( 'r' );	/* do the resumption command */

   erase_win(screen);
   if (active) {
      for(win=ACTIVE(prev);win!=active;win=W(prev)) {
         restore_win(win);
         killpg(W(pid),SIGCONT);
         }
      restore_win(active);
      killpg(ACTIVE(pid),SIGCONT);
      }
#endif
   MOUSE_ON(screen,mousex,mousey);
   }
/*}}}  */
#ifdef MGR_ALIGN
/*{{{  alignwin -- align a window so a byte boundary occurs somewhere insode the border*/
void
alignwin(screen,x,dx,slop)
register BITMAP *screen;
register int *x, *dx;
int slop;
   {
   register int adjust = (BIT_X(screen)+ *x) & 7;

   if (adjust>0 && adjust<(8-slop)) {
      *x -= adjust;
      dbgprintf('A',(stderr,"Adjusting x by %d",adjust));
      }
      dbgprintf('A',(stderr," at [%d/%d]\r\n",*x,(*x)&7));

   adjust = (adjust + *dx) &7;

   if (adjust>slop) { 
      *dx += 8-adjust;
      dbgprintf('A',(stderr,"Adjusting dx by %d\r\n",8-adjust));
      }
      dbgprintf('A',(stderr," at [%d/%d]\r\n",*x + *dx,(*x + *dx)&7));
   }
/*}}}  */
#endif /* MGR_ALIGN */

/*{{{  cursor_ok -- make sure icon is valid*/
int
cursor_ok(map)
BITMAP *map;			/* cursor icon */
   {
   return(map != NULL && BIT_WIDE(map) >= 16  && BIT_HIGH(map) >= 32);
   /* might like to check contents of bitmap to be reasonable */
   }
/*}}}  */
/*{{{  do_cursor*/
static void
do_cursor(win)
WINDOW *win;
	{
	switch(W(curs_type)) {
		case CS_BLOCK:
			bit_blit(W(window), W(x)+W(text.x),
				W(y)+W(text.y)-W(font->head.high),
				W(font->head.wide), W(font->head.high),
				PUTOP(BIT_NOT(BIT_DST),W(style)), 0, 0, 0);
			break;
		case CS_BOX:
			bit_blit(W(window), W(x)+W(text.x),
				W(y)+W(text.y)-W(font->head.high)+1,
				W(font->head.wide), W(font->head.high)-2,
				PUTOP(BIT_NOT(BIT_DST),W(style)), 0, 0, 0);
			bit_blit(W(window), W(x)+W(text.x)-2,
				W(y)+W(text.y)-W(font->head.high)-1,
				W(font->head.wide)+4, W(font->head.high)+2,
				PUTOP(BIT_NOT(BIT_DST),W(style)), 0, 0, 0);
			break;
		case CS_LEFT:
			bit_blit(W(window), W(x)+W(text.x) - 1,
				W(y)+W(text.y)-W(font->head.high),
				2, W(font->head.high),
				PUTOP(BIT_NOT(BIT_DST),W(style)), 0, 0, 0);
			break;
		case CS_RIGHT:
			bit_blit(W(window), W(x)+W(text.x)+W(font->head.wide)-1,
				W(y)+W(text.y)-W(font->head.high),
				2, W(font->head.high),
				PUTOP(BIT_NOT(BIT_DST),W(style)), 0, 0, 0);
			break;
		case CS_UNDER:
			bit_blit(W(window), W(x)+W(text.x),
				W(y)+W(text.y)-1,
				W(font->head.wide), 2,
				PUTOP(BIT_NOT(BIT_DST),W(style)), 0, 0, 0);
			break;
		}
	}
/*}}}  */
/*{{{  cursor_on, cursor_off*/
static int cursoron = 0;

void cursor_on()
{
	if( !active ) {
		cursoron = 0;
		return;
	}
	if( cursoron )
		return;
	do_cursor(active);
	cursoron = 1;
}

void cursor_off()
{
	if( !active ) {
		cursoron = 0;
		return;
	}
	if( !cursoron )
		return;
	cursoron = 0;
	do_cursor(active);
}
/*}}}  */
/*{{{  system command - turn off root privaleges*/
/* system command - turn off root privaleges */

int systemcmd(command) char *command;
{
        int status, pid, w;
        register void (*istat)(), (*qstat)();

	if (!command  ||  *command == '\0')
		return(0);
        if ((pid = vfork()) == 0) { /* does vfork work? */

                /* make sure commands doesn't run as root */
      
                int uid = getuid();
                int gid = getgid();
                setreuid(uid,uid);
                setregid(gid,gid);

		close(0);
		open("/dev/null",O_RDONLY);

                execl("/bin/sh", "sh", "-c", command, 0);
                _exit(127);
        }
        istat = signal(SIGINT, SIG_IGN);
        qstat = signal(SIGQUIT, SIG_IGN);
        while ((w = wait(&status)) != pid && w != -1)
                ;
        if (w == -1)
                status = -1;
        signal(SIGINT, istat);
        signal(SIGQUIT, qstat);
        return(status);
}
/*}}}  */
