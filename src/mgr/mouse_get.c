/*{{{}}}*/
/*{{{  #includes*/
#include <sys/time.h>
#include <mgr/share.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "proto.h"
#include "mouse.h"
/*}}}  */

/*{{{  variables*/
static int button_map[8] = {0,1,2,3,4,5,6,7};


#ifdef EMUMIDMSBUT
static unsigned int old_butstate = 0;
/*
 * button_faking&MS_BUTMIDDLE on pretends that middle button is down.
 * button_faking&MS_BUTLEFT pretends the left button is up.
 * similarly MS_BUTRIGHT.  Necessary while only one button of chord released.
 */
static unsigned int button_faking = 0;
static int mouse_event_buffered = 0;
static struct ms_event buffered_ms_event;
/*}}}  */

/*
 * predicate soon_get_event indicates whether mouse input
 * is available within 140 milliseconds, otherwise times out.
 */
static int soon_get_event( mfd)
int mfd;
{
fd_set r;
struct timeval timeout;

	FD_ZERO( &r);
	FD_SET( mfd, &r);
	timeout.tv_sec = 0;
	timeout.tv_usec = 140000;

        if (mfd < 0)
		return 0;
        else
		return( select( mfd+1, &r, NULL, NULL, &timeout) > 0);
}
#endif /* EMUMIDMSBUT */

/*{{{  mouse_get*/
/* primary mouse interface
*/

int mouse_get(int mouse, int *x_delta, int *y_delta)
{
  struct ms_event ev;
#ifdef EMUMIDMSBUT
  unsigned int button_change;

	if( mouse_event_buffered) {
		(void) memcpy( &ev, &buffered_ms_event, sizeof( ev));
		mouse_event_buffered = 0;
	}
	else
#endif
		get_ms_event(&ev);
	mfd = mouse;

#ifdef EMUMIDMSBUT
	/* emulate three button mice. middle button is emulated */
	/* by pressing left & right buttons at the same time */

	if( ev.ev_code == MS_BUTUP) {
		button_change = old_butstate & ~ev.ev_butstate;
		if( (button_change & ~button_faking) == 0)
			ev.ev_code = MS_NONE;
		button_faking &= ~(button_change | MS_BUTMIDDLE);
	}
	else if( ev.ev_code == MS_BUTDOWN) {
		button_change = ev.ev_butstate & ~old_butstate;
		if( button_change == (MS_BUTLEFT| MS_BUTRIGHT)) {
			button_faking = (MS_BUTLEFT| MS_BUTRIGHT| MS_BUTMIDDLE);
		}
		else if( old_butstate == 0 && soon_get_event( mouse)) {
			get_ms_event( &buffered_ms_event);
			if( buffered_ms_event.ev_butstate
					== (MS_BUTLEFT| MS_BUTRIGHT)) {
				ev.ev_dx += buffered_ms_event.ev_dx;
				ev.ev_dy += buffered_ms_event.ev_dy;
				ev.ev_butstate = buffered_ms_event.ev_butstate;
				ev.ev_x = buffered_ms_event.ev_x;
				ev.ev_y = buffered_ms_event.ev_y;
				button_faking = (MS_BUTLEFT| MS_BUTRIGHT|
							MS_BUTMIDDLE);
			}
			else {
				mouse_event_buffered = 1;
			}
		}
	}

	old_butstate = ev.ev_butstate;
	ev.ev_butstate ^= button_faking;
#endif /* EMUMIDMSBUT */

#ifdef MOVIE
	log_time();
#endif
	*x_delta = ev.ev_dx;
	*y_delta = -ev.ev_dy;

	return(button_map[(int)ev.ev_butstate]);
}
/*}}}  */
/*{{{  map_mouse buttons (for non-left handers)*/
int *map_mouse(button,map) int button, map;
{
	if (button>0 && button<8) button_map[button]=map;
	return(button_map);
}
/*}}}  */
/*{{{  mouse_count -- are there still characters in the mouse buffer?*/
int mouse_count()
{
fd_set r;
struct timeval timeout;

	FD_ZERO(&r); FD_SET(mfd, &r);
	timeout.tv_sec = 0;
	timeout.tv_usec = 0;

        if (mfd<0) return 0;
        else return (select(mfd+1, &r, NULL, NULL, &timeout));
}
/*}}}  */
