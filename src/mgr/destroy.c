/*{{{}}}*/
/*{{{  Notes*/
/*                        Copyright (c) 1987 Bellcore
 *                            All Rights Reserved
 *       Permission is granted to copy or use this program, EXCEPT that it
 *       may not be sold for profit, the copyright notice must be reproduced
 *       on copies, and credit should be given to Bellcore where it is due.
 *       BELLCORE MAKES NO WARRANTY AND ACCEPTS NO LIABILITY FOR THIS PROGRAM.
 */

/* destroy a window */
/*}}}  */
/*{{{  #includes*/
#include <mgr/bitblit.h>
#include <mgr/font.h>
#include <mgr/window.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include "clip.h"
#include "defs.h"
#include "event.h"

#include "proto.h"
#include "border.h"
#include "colormap.h"
#include "do_event.h"
#include "erase_win.h"
#include "font_subs.h"
#include "get_menus.h"
#include "icon_server.h"
#include "put_window.h"
#include "subs.h"
#include "update.h"
#include "utmp.h"
/*}}}  */
/*{{{  #defines*/
#define ALL	1

#ifdef WEXITSTATUS /* POSIX */
#define wait_nohang(statusp)  waitpid( -1, statusp, WNOHANG)
#else /* BSD */
#define wait_nohang(statusp)  wait3( statusp, WNOHANG, (void *)0)
#endif
/*}}}  */

/*{{{  detach -- unlink an alternate window from list*/
static void detach(win2) WINDOW *win2;
   {
   register WINDOW *win = win2;

   if (!(W(main)))
      return;
   for(win=win2->main;W(alt)!=win2;win=W(alt))
      ;
   W(alt)= win2->alt;
   }
/*}}}  */
/*{{{  set_dead -- notify alternate windows of impending death*/
static void set_dead(win) register WINDOW *win;
   {
   for(win = W(alt); win != (WINDOW *) 0; win = W(alt)) {
      dbgprintf('d',(stderr,"%s: telling %d\r\n",W(tty),W(num)));
      W(main) = (WINDOW *) 0;
      }
   }
/*}}}  */

/*{{{  unlink_win -- free all space associated with a window*/
void unlink_win(win,how) register WINDOW *win;		/* window to unlink */
int how;			/* if how, unlink window stack as well */
   {
   register int i;

   dbgprintf('u',(stderr,"Unlinking %s %s\n",W(tty),how?"ALL":""));

   if (how && W(stack))
      unlink_win(W(stack),how);
   if (W(window))
       bit_destroy(W(window));
   for(i=0; i< MAXBITMAPS;i++)
      if (W(bitmaps)[i])
          bit_destroy(W(bitmaps)[i]);
   bit_destroy(W(cursor));	/* usually noop because static */
   if (W(border))
       bit_destroy(W(border));
   if (W(save))
       bit_destroy(W(save));
   if (W(snarf))
      free(W(snarf));
   if (W(bitmap))
      free(W(bitmap));
   zap_cliplist(win);

   for(i=0; i< MAXEVENTS;i++)
       if (W(events)[i])
          free(W(events)[i]);

   for(i=0; i< MAXMENU;i++)
      if (W(menus)[i])
         menu_destroy(W(menus)[i]);

   free(win);
	win=NULL;
   }
/*}}}  */
/*{{{  destroy -- destroy a window*/
int destroy(win) register WINDOW *win;
   {
   int status;

   if (win == (WINDOW *) 0)
      return(-1);

   MOUSE_OFF(screen,mousex,mousey);
   cursor_off();

   if (win != active) {
      ACTIVE_OFF();
      expose(win);
      }

   active = W(next);

   /* deallocate window slot */

   if (active)
      ACTIVE(prev) = W(prev);

   /* remove window from screen */

   erase_win(W(border));

   if (W(main)==win) {		/* kill process associated with the window */
      dbgprintf('d',(stderr,"%s: destroy main %s\r\n",W(tty),W(alt)?"ALT":""));
      if (W(pid)>1) killpg(W(pid),SIGHUP);

      if (geteuid() < 1) {
         chmod(W(tty),0666);
         chown(W(tty),0,0);
         }

      close(W(to_fd));
      FD_CLR( W(to_fd), &mask);
      FD_CLR( W(to_fd), &to_poll);
#ifdef WHO
      rm_utmp(W(tty));
#endif
      free_colors( win, 0, 255); /* release claims on the colormap */

      /* tell alternate windows main died */

      set_dead(win);

      /* wait for shell to die */

      dbgprintf('d',(stderr,"waiting for ...")); fflush(stderr);
      if (W(pid)>1 && !(W(flags)&W_DIED)) {
	 int wpid;

         wpid = wait_nohang(&status);
         if (wpid == 0) {			/* start it so it can die */
            kill(W(pid),SIGCONT);
	    wpid = wait_nohang(&status);
	    if (wpid==0)
	       fprintf(stderr,"MGR: Wait for %d failed\n",W(pid));
            }
	 dbgprintf('d',(stderr,"wait_nohang returns %d\r\n",wpid));
         }
      next_window--; 
      }

   else if (W(main) && !(W(main)->flags&W_DIED)) {	/* main still alive */
      dbgprintf('d',(stderr,"%s: destroy alt %d\r\n",W(tty),W(num)));
      do_event(EVENT_DESTROY,win,E_MAIN);
      if (W(from_fd)) {		/* re-attach output to main window */
         W(main)->from_fd = W(main)->to_fd;
         W(main)->max = W(max) - W(current); /* ??? */
	 dbgprintf('d',(stderr,"%s: copy %d chars at %d\r\n",
		   W(tty),W(main)->max, W(current)));
         memcpy(W(main)->buff,W(buff)+W(current)+1,W(main)->max);
         W(main)->current = 0;
         set_size(win);
         dbgprintf('d',(stderr,"%s: reattaching main %d chars\r\n",W(tty),W(max)));
         }
      detach(win);
      }
   else if (W(main)) {		/* tell main alts know they are dead */
      W(main)->alt = (WINDOW *) 0;
      dbgprintf('d',(stderr,"%s: destroy alt, (tell main)\r\n",W(tty)));
      }
   else {
      dbgprintf('d',(stderr,"%s: destroy alt, (dead main)\r\n",W(tty)));
      }

   /* fix up display if any windows left */

   SETMOUSEICON( DEFAULT_MOUSE_CURSOR);	/* because active win chg */

   if (active) {
      repair(win);
      un_covered();
      clip_bad(win);	/* invalidate clip lists */
      ACTIVE_ON();
      cursor_on();
      }

   /* free space associated with dead window */

   unlink_win(win,ALL);

   dbgprintf('d',(stderr,"Active: %s-%d\r\n",
	     active?ACTIVE(tty):"NONE", active?ACTIVE(num):0));

   MOUSE_ON(screen,mousex,mousey);

   return(0);
   }
/*}}}  */
/*{{{  destroy_window -- mark active window for destruction*/
void destroy_window()
   {
   ACTIVE(flags) |= W_DIED;
   }
/*}}}  */
