/*********************************************/
/* you just keep on pushing my luck over the */
/*          BOULDER        DASH              */
/*                                           */
/*     Jeroen Houttuin, ETH Zurich, 1990     */
/*********************************************/

#include <stdio.h>
#ifdef X11
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/Xutil.h>
#endif
#ifdef MGR
#include <mgr/mgr.h>
#endif
#include <signal.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/timeb.h>
#include <unistd.h>
#include "xbd.h"

#ifdef X11
void
die(display, event)
  Display        *display;
  XErrorEvent    *event;
{
  char            buffer[BUFSIZ];
  XGetErrorText(display, event->error_code, buffer, BUFSIZ);
  (void) fprintf(stderr, "Display %s: Error %s\n",
		 XDisplayName(display), buffer);
  setreuid(getuid(), getuid());
  do
  {
    (void) fprintf(stderr, "(R)eturn (D)ump Core (E)xit:");
    switch (fgetc(stdin))
    {
    case 'R':
    case 'r':
      return;
    case 'E':
    case 'e':
      exit(1);
      break;
    case 'D':
    case 'd':
      kill(0, 3);
      break;
    default:
      break;
    }
  } while (1);
}
#endif

void
init_vars()
{
#ifdef MGR
  bit_src_op=BIT_SRC;
  bit_and_op=BIT_AND;
#endif
  blobbreak = 100;
  critical = 100;
  blobcells = 0;
  curorder = STAND;
  gamestop = True;
  scoreobs = True;
  stoplevel = False;
  levelnum = 1;
  speed = 1;
  lives = 4;
  xin = 0;
  yin = 0;
  players = 1;
}

struct itimerval cycletime;	/* Structure used when setting up timer */
void
adapt_timer()
{
  long            period;

  if (speed <= 0)
    speed = 1;
  period = (long) 3 *(long) 625000 / speed;
  cycletime.it_interval.tv_sec = period / 1000000;
  cycletime.it_interval.tv_usec = period % 1000000;
  cycletime.it_value = cycletime.it_interval;
  setitimer(ITIMER_REAL, &cycletime, (struct itimerval *) NULL);
}

/* Handle a key stroke by the user. */
void
handle_key(keyhit)
  KeySym          keyhit;	/* Key symbol for key stroke provided by X
				 * windows */
{
  if (players <= 0)
  {
    init_level(levelnum);
    adapt_timer();
#ifdef X11
    XResizeWindow(disp, wind, x*ELEM_XSIZE, y*ELEM_YSIZE + SCORESIZE);
#endif
#ifdef MGR
    {
      int xpos,ypos,border;

      m_getwindowposition(&xpos,&ypos);
      border=m_getbordersize();
      m_shapewindow(xpos,ypos,2*border+x*ELEM_XSIZE, 2*border+y*ELEM_YSIZE+SCORESIZE);
    }
#endif
    stoplevel = False;
    draw_field(True);
    gamestop = True;
    players = 1;
    return;
  }
  switch (keyhit)
  {
  case XK_question:
  case XK_slash:
    puts("Control the player using keyboard keys.");
    puts("CTRL key - steal instead of go");
    puts("h,left arrow -  left");
    puts("l,right arrow -  right");
    puts("k,up arrow -  up");
    puts("j,down arrow -  down");
    puts("\nSPACE - pause/continue game");
    puts("^C - kill the game");
    puts("^D - give Dutch act");
    puts("^R - redraw the screen");
    break;
  case XK_space:
#ifdef X11
  case XK_R11:
#endif
    gamestop = !gamestop;
    break;
#ifdef X11
  case XK_Left:
#endif
  case XK_H:
  case XK_h:
    curorder = LEFT;
    gamestop = False;
    break;
#ifdef X11
  case XK_Up:
#endif
  case XK_K:
  case XK_k:
    curorder = UP;
    gamestop = False;
    break;
#ifdef X11
  case XK_Down:
#endif
  case XK_J:
  case XK_j:
    curorder = DOWN;
    gamestop = False;
    break;
#ifdef X11
  case XK_Right:
#endif
  case XK_L:
  case XK_l:
    curorder = RIGHT;
    gamestop = False;
    break;
  }
}

/**** Timer  procedures  ****/

/* Function which is called whenever the timer signal goes off */
void
ticker(sig)
  int             sig;
{
  signal(SIGALRM, SIG_IGN);
  /* Ignore any signal which is not an alarm.  Ignore alarm signals */
  /* after a new level has been drawn until a key is hit. */
  if (sig != SIGALRM)
    return;
  if (time_tck)
    time_tck--;
  if (tinkact)
    tinkdur--;
  if (time_tck % 10 == 1)
    scoreobs = True;

  if (!gamestop)
  {
    calculate_field();
    draw_field(False);
#ifdef X11
    XFlush(disp);
#endif
#ifdef MGR
    m_flush();
#endif
  }
  if (stoplevel)
  {
    init_level(levelnum);
    adapt_timer();
#ifdef X11
    XResizeWindow(disp, wind, x*ELEM_XSIZE,y*ELEM_YSIZE + SCORESIZE);
    XFlush(disp);
#endif
#ifdef MGR
    {
      int xpos,ypos,border;

      m_getwindowposition(&xpos,&ypos);
      border=m_getbordersize();
      m_shapewindow(xpos,ypos,2*border+x*ELEM_XSIZE, 2*border+y*ELEM_YSIZE+SCORESIZE);
      m_flush();
    }
#endif
    gamestop = True;
    stoplevel = False;
  }
  signal(SIGALRM, ticker);
}
/**** End timer procedures ****/

void
main(argc, argv)
  int             argc;
  char          **argv;
{
  long            period;
  KeySym          keyhit;
#ifdef X11
  char            buf[50];
  static XEvent   xev;
  int		  keycount;
#endif
  int             i;
  char c;

  printf("type ? for help.\n");

  init_vars();

  /* scan the command line for executing parameters and flags */
  for (i = 1; i < argc; ++i)
  {
    if (argv[i][0] == '-')
    {
      if (argv[i][1] == 'l')
      {
	if (argv[i][2] == '\0' && i + 1 < argc)
	{
	  sscanf(argv[i + 1], "%d", &levelnum);
	  i++;
	} else
	  sscanf(argv[i] + 2, "%d", &levelnum);
#ifdef MGR
      } else if (argv[i][1]=='r')
      {
        bit_src_op=BIT_NOT(BIT_SRC);
        bit_and_op=BUILDOP(BIT_NOT(BIT_NOT(BIT_SRC)&BIT_NOT(BIT_DST)),BLACK,WHITE);
#endif
      } else
      {
	printf("usage: xbd [-l <level>] \n");
	exit(1);
      }
    }
  }

  levelstart = levelnum;
  init_level(levelnum);
#ifdef X11
  xstart(EVMASK);
  XSetErrorHandler(die);
  XStoreName(disp, wind, "BOULDER DASH ?");
#endif
#ifdef MGR
  mgrstart();
#endif
  make_gcs();
#ifdef MGR
    {
      int xpos,ypos,border;

      m_getwindowposition(&xpos,&ypos);
      border=m_getbordersize();
      m_shapewindow(xpos,ypos,2*border+x*ELEM_XSIZE, 2*border+y*ELEM_YSIZE+SCORESIZE);
      m_flush();
    }
#endif
  draw_field(True);
  draw_score();
#ifdef MGR
  m_flush();
#endif

  /* initialize timer structure according to speed */
  if (speed <= 0) speed = 1;
  period = (long) 3 *(long) 625000 / speed;
  cycletime.it_interval.tv_sec = period / 1000000;
  cycletime.it_interval.tv_usec = period % 1000000;
  cycletime.it_value = cycletime.it_interval;
  /* start the system timer.  the timer signal catcher will be set */
  /* after the first x event is received. */
  signal(SIGALRM, SIG_IGN);
  setitimer(ITIMER_REAL, &cycletime, (struct itimerval *) NULL);

  while (lives > 0)		/* MAIN LOOP */
  {
#ifdef X11
    XWindowEvent(disp, wind, EVMASK, &xev);
    signal(SIGALRM, SIG_IGN);
    if ((xev.type == Expose && xev.xexpose.count == 0))
    {
      signal(SIGALRM, SIG_IGN);
      draw_field(True);
      draw_score();
    }
    else if (xev.type == KeyPress)
    {
      keycount = XLookupString(&xev, buf, 50, &keyhit, (XComposeStatus *) NULL);
      if (steal = (xev.xkey.state & ControlMask))
	switch (keyhit)
	{
	  /* ^C, ^U kill the game */
	case XK_C:
	case XK_U:
	case XK_c:
	case XK_u:
	case XK_backslash:
	  goto game_over;
	  /* ^D Kill; commit suicide */
	case XK_D:
	case XK_d:
	  curorder = KILL;
          break;
	  /* ^R redraws the level */
	case XK_R:
	case XK_r:
	  draw_field(True);
	  break;
	default:
	  handle_key(keyhit);
	  break;
	}
      else
	handle_key(keyhit);
    }
#endif
#ifdef MGR
    if (read(0,&c,1)==1)
    {
      signal(SIGALRM, SIG_IGN);
      keyhit=c;
	switch (keyhit)
        {
	  /* ^C, ^U kill the game */
	case 'C'-'@':
	case 'U'-'@':
	case '\\'-'@':
	  goto game_over;
	  /* ^D Kill; commit suicide */
	case 'D'-'@':
	  curorder = KILL;
          gamestop=False;
          break;
	  /* ^R redraws the level */
	case 'R'-'@':
	  draw_field(True);
	  break;
	default:
	  handle_key(keyhit);
	  break;
          }
    }
#endif
    if (!gamestop)
    {
#ifdef X11
      XSync(disp, False);
#endif
#ifdef MGR
      m_flush();
#endif
      signal(SIGALRM, ticker);
    }
  }
game_over:
#ifdef X11
  XSync(disp, False);
  xend();
#endif
#ifdef MGR
  mgrend();
#endif
  add_score();
}
