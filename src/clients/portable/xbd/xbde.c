/*********************************************/
/* you just keep on pushing my luck over the */
/*           BOULDER        DASH             */
/*                                           */
/*     Jeroen Houttuin, ETH Zurich, 1990     */
/*********************************************/

#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/Xutil.h>
#include "xbd.h"

char            curchar;
GC              drawgc;

void
init_vars()
{
  levelnum = -1;
  xin = 0;
  yin = 0;
}

/* Save the current level back into a file.  The global variable */
/* filename is used to determine the file name. */
void
save_level()
{
  FILE           *levelfile;
  char            buf[300];
  register int    i, j;

  /* Open the data file */
  levelfile = fopen(filename, "w");
  if (levelfile == NULL)
  {
    exit(1);
  }
  /* Write out the size of the level.  Normal text is used so that */
  /* levels can be easily copied across architectures. */
  fprintf(levelfile, "%d %d %d %d %d %d %d %d %d %s\n", y, x,
	  speed, diareq, diapoints, extradiapoints, blobbreak, tinkdur, time_tck, levname);
  /* Terminate the lines for writing out the horizontal level lines */
  buf[x] = '\n';
  buf[x + 1] = '\0';
  /* Iterate through each vertical position */
  for (i = 0; i < y; ++i)
  {
    /* Copy each horizontal line into the output buffer */
    for (j = 0; j < x; ++j)
      buf[j] = field[i][j].content;
    /* Write the line out to the file */
    fputs(buf, levelfile);
  }
  /* Close the data file */
  fclose(levelfile);
}

/* Main routine for editing levels */
void
main(argc, argv)
  int             argc;
  char          **argv;
{
  register int    i, j;
  static XEvent   xev;
  KeySym          keyhit;
  int             keycount;
  int             tmp;
  char            buf[50];

  init_vars();

  /* Read in command line options */
  for (i = 1; i < argc; ++i)
  {
    if (argv[i][0] == '-')
    {
      /* -w sets the level width */
      if (argv[i][1] == 'w')
      {
	if (argv[i][2] == '\0' && i + 1 < argc)
	{
	  sscanf(argv[i + 1], "%d", &xin);
	  i++;
	} else
	  sscanf(argv[i] + 2, "%d", &xin);
      }
      /* -h sets the level height */
      else if (argv[i][1] == 'h')
      {
	if (argv[i][2] == '\0' && i + 1 < argc)
	{
	  sscanf(argv[i + 1], "%d", &yin);
	  i++;
	} else
	  sscanf(argv[i] + 2, "%d", &yin);
      }
      /* -l sets the level number */
      else if (argv[i][1] == 'l')
      {
	if (argv[i][2] == '\0' && i + 1 < argc)
	{
	  sscanf(argv[i + 1], "%d", &levelnum);
	  i++;
	} else
	  sscanf(argv[i] + 2, "%d", &levelnum);
      } else
      {
	printf("usage: xbde [-h <height>] [-w <width>] -l <level> \n");
	exit(1);
      }
    }
  }
  /* Make sure some value was chosen for the level number.  This */
  /* discourages everybody editing the same level all the time. */
  if (levelnum == -1)
  {
    printf("usage: editor [-h <height>] [-w <width>] -l <level> \n");
    exit(1);
  }
  /* Load in level data from file. */
  init_level(levelnum);

  printf("Welcome.  Type h for help.\n");

  /* Start up X windows and create all graphics cursors */
  xstart(EVMASK);
  /* Set the name of the output window */
  XStoreName(disp, wind, "BOULDER DASH - LEVEL EDITOR");
  XFlush(disp);			/* initializing flush */
  make_gcs();
  Egc = Egc2;
  Wgc = Wgc2;
  tgc = tgc3;
  drawgc = pgc;
  draw_field(True);

  /* Main event loop */
  do
  {
    /* Get the next X window event */
    XWindowEvent(disp, wind, EVMASK, &xev);

    /* If it was an expose event, redraw everything */
    if (xev.type == Expose)
    {
      draw_field(True);
      draw_score();
    } else if (xev.type == KeyPress)
    {
      keycount = XLookupString(&xev, buf, 50, &keyhit, (XComposeStatus *) NULL);
      /* If the 'h', '?' or '/' key was hit, print out the text */
      /* descriptions of each block type */
      if (keyhit == XK_H || keyhit == XK_h || keyhit == XK_question ||
	  keyhit == XK_slash)
      {
	puts("^w - finish editing and save the level.");
	puts("^c - quit editing.");
	puts("^E - erase level.");
	puts("Use the left mouse button to paint blocks.");
	puts("Use the right mouse button to erase blocks.");
	putchar('\n');
      }
      /* A ^E erases the entire level */
      else if ((keyhit == XK_E) &&
	       (xev.xkey.state & ControlMask))
      {
	/* Replace level contents with space */
	for (i = 0; i < y; ++i)
	  for (j = 0; j < x; ++j)
	    if ((i == 0) || (i == y - 1) || (j == 0) || (j == x - 1))
	      set_cell(i, j, STEEL);
	    else
	      set_cell(i, j, SPACE);
	/* Redraw empty level */
	draw_field(False);
      } else
	curchar = keyhit;
    }
    /* If the mouse moves with the button pressed, or the button is */
    /* pressed, draw the current block at that position */
    else if (xev.type == MotionNotify)
    {
      if (xev.xmotion.state & Button3Mask)
	set_cell(xev.xmotion.y >> 5, xev.xmotion.x >> 5, SPACE);
      else if (xev.xmotion.state & Button1Mask)
	set_cell(xev.xmotion.y >> 5, xev.xmotion.x >> 5, curchar);
    } else if (xev.type == ButtonPress)
    {
      if (xev.xbutton.button == Button3)
	set_cell(xev.xbutton.y >> 5, xev.xbutton.x >> 5, SPACE);
      else
	set_cell(xev.xbutton.y >> 5, xev.xbutton.x >> 5, curchar);
    }
    draw_field(False);
    XFlush(disp);
    /* Loop until a control key is pressed */
  } while (xev.type != KeyPress ||
	   (keyhit != XK_C && keyhit != XK_c &&
	    keyhit != XK_W && keyhit != XK_w) ||
	   !(xev.xkey.state & ControlMask));

  /* Save level to data file */
  if (keyhit == XK_W || keyhit == XK_w)
    save_level();
  xend();
  exit(0);
}
