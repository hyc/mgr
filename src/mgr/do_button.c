/*{{{}}}*/
/*{{{  Notes*/
/* Figure out what to do with a button push */
/*}}}  */
/*{{{  #includes*/
#include <mgr/bitblit.h>
#include <mgr/share.h>
#include <mgr/font.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <pwd.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "defs.h"
#include "event.h"
#include "menu.h"

#include "proto.h"
#include "border.h"
#include "copyright.h"
#include "do_event.h"
#include "do_menu.h"
#include "erase_win.h"
#include "font_subs.h"
#include "get_menus.h"
#include "intersect.h"
#include "kbd.h"
#include "mouse_get.h"
#include "set_mode.h"
#include "startup.h"
#include "subs.h"
#include "utmp.h"
/*}}}  */

/*{{{  _quit -- really quit*/
void _quit(void)
   {
   register WINDOW *win;
   static int really_quit=0;

   if (really_quit++) {
   /* we're in bad shape */
      perror("PANIC!!  Error during _quit()!");
      setreuid(getuid(),getuid());
      abort();
      }

   MOUSE_OFF(screen,mousex,mousey);

   sleep(1);		/* let the key (if any) un-press before resetting
			the kbd */
   set_kbd(0);		/* fix up keyboard modes */
   kbd_reset();		/* reset the keyboard */
   reset_tty(0);	/* fix up console tty modes */
#ifdef MOVIE
   log_end(); 		/* turn off logging */
#endif

   /* fix pttys */
   if (geteuid() < 2)
      for(win=active;win != (WINDOW *) 0;win=W(next))
      {
	 if (W(pid)>1) killpg(W(pid),SIGHUP);
         chmod(W(tty),0666);
         chown(W(tty),0,0);
         }

   /* fix utmp file */

#ifdef WHO
   close(getdtablesize()-1); /* make sure there are enough fd's left */
   for(win=active;win != (WINDOW *) 0;win=W(next))
      if (W(tty))
          rm_utmp(W(tty));
#endif

   CLEAR(screen,BIT_CLR);
   do_cmd( 'q' );	/* do the quiting command */
	{ bit_destroy(screen); bit_destroy(prime); }
   }
/*}}}  */
/*{{{  redraw -- redraw screen, restore contents of saved windows*/
void redraw(void)
   {
   register WINDOW *win;

   dbgprintf('b',(stderr,"\r\n\tREDRAW\r\n"));
   for(win=active;win != (WINDOW *) 0;win=W(next)) {
      if (W(flags)&W_ACTIVE) {
         save_win(win);
         do_event(EVENT_REDRAW,win,E_MAIN);
         }
      }

   erase_win(screen);
   if (active) {
      for(win=ACTIVE(prev);win != active;win=W(prev))
         restore_win(win);
      restore_win(active);
      }
   }
/*}}}  */
static void lock_screen(void) {
    WINDOW *win;
    struct passwd *pwd = getpwuid( getuid());

    /* Mouse is off from mgr:main */
    /* cursor and active border off because of button handling */

    for( win = active; win != 0; win = W(next))
	if( W(flags)&W_ACTIVE)
	    save_win( win);

    copyright( screen, pwd? pwd->pw_passwd: "");

    erase_win( screen);
    if( active) {
	for( win = ACTIVE(prev); win != active; win = W(prev))
	    restore_win( win);
	restore_win( active);
    }

    /* flush and ignore intervening mouse events */
    while( mouse_count() > 0) {
	int x, y;
	(void) mouse_get( mouse, &x, &y);
    }
}
/*{{{  quit -- quit with confirm*/
void quit(void)
   {
   struct menu_state *state;		/* place to keep menu state */
   int confirm;

   /* confirm the quit */

   state = menu_define(font,quit_menu,0,0,MENU_COLOR);
   state = menu_setup(state,screen,mousex,mousey,0);

   /* The extra call to menu_get() makes the use of the mouse buttons
      consistent on the menus; namely the action is selected by the button
      going up.
   */
   menu_get(state,mouse,BUTTON_SYS,0);
   menu_get(state,mouse,0,0);

   confirm = menu_ischoice(state) ? menu_choice(state) : 0;
   menu_destroy(state);
   if (confirm == M_QUIT) {
      _quit();
      exit(0);
      }
   else if (confirm == M_SUSPEND)
      suspend();
   else if (confirm == M_LOCK)
      lock_screen();
   else
      /* do nothing for CANCEL etc */;
   }
/*}}}  */
/*{{{  do_button -- figure out what to do with a button push*/
void do_button(button) int button;
   {
   register WINDOW *win;		/* window of interest */
   int choice;				/* current menu choice */
   int choice_ok;			/* valid choice flag */
   struct menu_state *state;		/* place to keep menu state */
   register int which_menu;		/* which menu indicator */

   dbgprintf('b',(stderr,"do button %d (button state id %d)\n",button,button_state));

   /*	Insist on a transition before taking any action.
   */
   if( button == button_state )
      return;

   /*	Other button codes, such as chords, are not recognized and have no
	effect.
   */
   switch( button ) {
   case 0:
   case BUTTON_SYS:
   case BUTTON_2:
   case BUTTON_1:
	break;
   default:
	return;
   }

   /*	If button was down and now no button is down,
		send the event stating the formerly down button is now up.
	If some button is down, the same or some other,
		do nothing.
	Note that this creates a fundmental property of MGR:  namely that
	once a button is pressed, no other button pressings have any effect
	until all buttons are released.
   */
   if( button_state ) {
      if ( button == 0 ) {
	 int	b_event = -button_state;

	 /* button_state must be cleared before sending a Button_Up event to
	    prevent any event action from thinking the button is still down.
	 */
         button_state = 0;
         do_event( b_event, active, E_MAIN );
         }
      return;
      }

   /* button_state is the global record of the current button state.
   */
   button_state = button;

   /* Check for events associated with this button being pushed.
   */
   do_event(button,active,E_MAIN);
   switch (button) {
      case BUTTON_1:	/* temporary vi hack */
#ifdef VI
           if (active && ACTIVE(flags)&W_VI) {

              int x = mousex-(ACTIVE(x0)+ACTIVE(text).x);
              int y = mousey-(ACTIVE(y0)+ACTIVE(text).y);
              int dx = ACTIVE(text).wide ? 
                       ACTIVE(text).wide : BIT_WIDE(ACTIVE(window));
              int dy = ACTIVE(text).wide ? 
                       ACTIVE(text).high : BIT_HIGH(ACTIVE(window));

              if (x<0 || x > dx)
                 break;
              if (y>=0 && y<=dy) {
                 char buff[10];
                 sprintf(buff,"%dH%d|",
                      y/ACTIVE(font)->head.high+1,
                      x/ACTIVE(font)->head.wide+1);
                 write(ACTIVE(to_fd),buff,strlen(buff));
                 } 
              else if (y<0)
                 write(ACTIVE(to_fd),"\025",1);	/* ASCII Control-U */
              else
                 write(ACTIVE(to_fd),"\004",1);	/* ASCII Control-D */
              }
#endif
           if (active) {
              go_menu(1);
	   }
           break;
      case BUTTON_2:				/* for applic. menu */
           if (active) {
              go_menu(0);
	   }
           break;
      case BUTTON_SYS:				/* for system operation */
           /* see if mouse is in a window */
           if (mousex < STRIPE)
              win = (WINDOW *) 0;
           else
              for(win=active;win != (WINDOW *) 0;win=W(next))
                 if(mousein(mousex,mousey,win,1))
                    break;

           /* do a menu for no window, or active window */
           if (win == active || win == (WINDOW *) 0) {
              if (active && win == active) {
                 state = menu_define(font,active_menu,0,0,MENU_COLOR);
                 which_menu = 1;
                 }
              else if (next_window >= MAXWIN) {
                 state = menu_define(font,full_menu,0,0,MENU_COLOR);
                 which_menu = 2;
                 }
              else {
                 state = menu_define(font,main_menu,0,0,MENU_COLOR);
                 which_menu = 3;
                 }
              if (active) {
                 cursor_off();
                 if (which_menu != 1)
                    ACTIVE_OFF();
                 }
              state = menu_setup(state,screen,mousex,mousey,0);
              menu_get(state,mouse,0,0);
              choice = menu_choice(state);
              choice_ok = menu_ischoice(state);
              menu_destroy(state);
              if (choice_ok) {
                 switch(which_menu) {
                    case 1:
			 ACTIVE_OFF();
                         (*active_functions[choice])();
                         break;
                    case 2:
                         (*full_functions[choice])();
                         break;
                    case 3:
                         (*main_functions[choice])();
                         break;
                    }
		 }
              if (active) {
                 ACTIVE(flags) &= ~W_NOINPUT;
                 ACTIVE_ON();
                 cursor_on();
                 }
	      do_button(0);
              }
           else {
		   /* bring obscured window to the top */
              dbgprintf('b',(stderr,"activating: %s\r\n",W(tty)));
              if (active) {
                  ACTIVE_OFF();
                  cursor_off();
                  }
              expose(win);
              if (active) {
                 ACTIVE(flags) &= ~W_NOINPUT;
                 ACTIVE_ON();
                 cursor_on();
                 }
              }
           break;
           }
   return;
   }
/*}}}  */
/*{{{  hide_win -- hide the active window*/
void hide_win(void)
   {
   hide(active);
   }
/*}}}  */
