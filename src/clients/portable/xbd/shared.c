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
#define M_NOFLUSH
#include <mgr/mgr.h>
#endif
#include <errno.h>
#include <sys/time.h>
#include <sys/timeb.h>
#include <string.h>
#include "xbd.h"

/* Manufaction a graphics cursor used in a XFill... operation. */
#ifdef X11
GC
makegc(func, bits)
  int             func;		/* Drawing function such as GXcopy or GXor. */
  char            bits[];	/* Bits describing fill pattern.  Produced in
				 * an X11 */
/* bitmap file usually. */
{
  static XGCValues gcv;
  Pixmap          pmap;

  /* Build X11 bitmap from data in bits */
  pmap = XCreatePixmapFromBitmapData(disp, wind, bits, ELEM_XSIZE, ELEM_YSIZE, BlackPixel(disp, 0),
			       WhitePixel(disp, 0), DisplayPlanes(disp, 0));
  /* Assign the graphics cursor parameters */
  gcv.function = func;
  gcv.foreground = BlackPixel(disp, 0);
  gcv.background = WhitePixel(disp, 0);
  gcv.tile = pmap;
  gcv.fill_style = FillTiled;
  /* Return the created graphics cursor */
  return (XCreateGC(disp, wind, GCFunction | GCForeground | GCBackground |
		    GCTile | GCFillStyle, &gcv));
}
#endif
#ifdef MGR
int makegc(bits) char *bits;
{
  static int map=1;
  int width,height,depth;

  m_bitfile(map,bits,&width,&height,&depth);
  ELEM_XSIZE=width;
  ELEM_YSIZE=height;
  return map++;
}
#endif

void
make_gcs()
{
#ifdef X11
  pgc = makegc(GXcopy, player_bits);
  pgc1 = makegc(GXcopy, player_bits);
  pgc2 = makegc(GXcopy, player2_bits);
  wgc = makegc(GXcopy, wall_bits);
  Wgc = makegc(GXcopy, wall_bits);
  Wgc2 = makegc(GXcopy, wall2_bits);
  sgc = makegc(GXcopy, space_bits);
  ggc = makegc(GXcopy, grass_bits);
  dgc = makegc(GXcopy, diamond_bits);
  dgc1 = makegc(GXcopy, diamond_bits);
  dgc2 = makegc(GXcopy, diamond2_bits);
  Sgc = makegc(GXcopy, steel_bits);
  bgc = makegc(GXcopy, boulder_bits);
  xgc = makegc(GXand, explosion_bits);
  lgc = makegc(GXcopy, lmonster_bits);
  lgc1 = makegc(GXcopy, lmonster_bits);
  lgc2 = makegc(GXcopy, lmonster2_bits);
  rgc = makegc(GXcopy, rmonster_bits);
  rgc1 = makegc(GXcopy, rmonster_bits);
  rgc2 = makegc(GXcopy, rmonster2_bits);
  egc = makegc(GXcopy, eater_bits);
  egc1 = makegc(GXcopy, eater_bits);
  egc2 = makegc(GXcopy, eater2_bits);
  Egc = makegc(GXcopy, steel_bits);
  Egc1 = makegc(GXcopy, steel_bits);
  Egc2 = makegc(GXcopy, exit2_bits);
  ngc = makegc(GXcopy, nucbal_bits);
  Bgc = makegc(GXcopy, blob_bits);
  Bgc1 = makegc(GXcopy, blob_bits);
  Bgc2 = makegc(GXcopy, blob2_bits);
  tgc = makegc(GXcopy, wall_bits);
  tgc1 = makegc(GXcopy, wall_bits);
  tgc2 = makegc(GXcopy, tinkle1_bits);
  tgc3 = makegc(GXcopy, tinkle2_bits);
#endif
#ifdef MGR
  pgc = makegc("mgrbd/player");
  pgc1 = makegc("mgrbd/player");
  pgc2 = makegc("mgrbd/player2");
  wgc = makegc("mgrbd/wall");
  Wgc = makegc("mgrbd/wall");
  Wgc2 = makegc("mgrbd/wall2");
  sgc = makegc("mgrbd/space");
  ggc = makegc("mgrbd/grass");
  dgc = makegc("mgrbd/diamond");
  dgc1 = makegc("mgrbd/diamond");
  dgc2 = makegc("mgrbd/diamond2");
  Sgc = makegc("mgrbd/steel");
  bgc = makegc("mgrbd/boulder");
  xgc = makegc("mgrbd/explosion");
  lgc = makegc("mgrbd/lmonster");
  lgc1 = makegc("mgrbd/lmonster");
  lgc2 = makegc("mgrbd/lmonster2");
  rgc = makegc("mgrbd/rmonster");
  rgc1 = makegc("mgrbd/rmonster");
  rgc2 = makegc("mgrbd/rmonster2");
  egc = makegc("mgrbd/eater");
  egc1 = makegc("mgrbd/eater");
  egc2 = makegc("mgrbd/eater2");
  Egc = makegc("mgrbd/steel");
  Egc1 = makegc("mgrbd/steel");
  Egc2 = makegc("mgrbd/exit2");
  ngc = makegc("mgrbd/nucbal");
  Bgc = makegc("mgrbd/blob");
  Bgc1 = makegc("mgrbd/blob");
  Bgc2 = makegc("mgrbd/blob2");
  tgc = makegc("mgrbd/wall");
  tgc1 = makegc("mgrbd/wall");
  tgc2 = makegc("mgrbd/tinkle1");
  tgc3 = makegc("mgrbd/tinkle2");
#endif
}

void
init_level(levelnum)
  int             levelnum;
{

  FILE           *levelfile;
  char            buf[300], *ptr;
  extern char    *getenv();

  Egc = Egc1;			/* don't blink EXIT */
  blobcollapse = False;
  blobcells = 0;
  scoreobs = True;
  tinkact = False;
  levincreased = False;
  strcpy(levname, "No_name_for_this_level_yet");

  /* Manufaction the file name by starting with the world name and */
  /* appending the level number to it. */
  if (ptr = getenv("XBDLIB"))
    strcpy(filename, ptr);
  else
    strcpy(filename, LIB);
  strcat(filename, "/");
  strcat(filename, LEVELPREFIX);
  sprintf(filename + strlen(filename), "%03d", levelnum);
  /* Open level file for reading */
  levelfile = fopen(filename, "r");
  /* If level file does not exist, use the default level file. */
  if (levelfile == NULL)
  {
    /* Build the default level name */
    if (ptr = getenv("XBDLIB"))
      strcpy(buf, ptr);
    else
      strcpy(buf, LIB);
    strcat(buf, "/");
    strcat(buf, "/default");
    /* Open default level file for reading */
    levelfile = fopen(buf, "r");
    if (levelfile == NULL)
    {
      perror(LEVELPREFIX);
      exit(1);
    }
  }
  /* Load the first line of the level file */
  if (fgets(buf, 300, levelfile) == NULL)
  {
    x = w;
    y = h;
    speed = 15;
    diareq = 12;
    diapoints = 0;
    extradiapoints = 0;
    blobbreak = 200;
    time_tck = 1000;
  } else
  {
    /* Extract the level parameters */
    sscanf(buf, "%d %d %d %d %d %d %d %d %d %s", &y, &x, &speed, &diareq,
	 &diapoints, &extradiapoints, &blobbreak, &tinkdur, &time_tck, levname);
  }

  if (xin && yin)
  {
    x = xin;
    y = yin;
  }				/* read in from editor command line */
  /*
   * if (x > w) x = w; if (y > h) y = h; if (x < 2) x = 2; if (y < 1) y = 1;
   */
  /*
   * Iterate through each horizontal line
   */
  for (i = 0; i < y; ++i)
  {
    /* Load the next line from the file */
    if (fgets(buf, 300, levelfile) != NULL)
    {
      /* Go through each horizontal position and copy the data into */
      /* the level array. */
      for (j = 0; j < x; ++j)
      {
	/* Break out if line ends prematurely */
	if (buf[j] == '\n' || buf[j] == '\0')
	  field[i][j].content = STEEL;	/* break; */
	field[i][j].content = buf[j];
	field[i][j].changed = True;
	field[i][j].dir = N;
	field[i][j].speed = 0;
	field[i][j].stage = 0;
	field[i][j].caught = True;
	field[i][j].checked = False;
      }
    } else
      j = 0;
    for (; j < x; ++j)
      field[i][j].content = STEEL;
  }
  /* Close the level file */
  fclose(levelfile);
}

/* Draw the score and level number */
void
draw_score()
{
  static char buf[200];
  static char oldbuf[200]="";

  /* Build the output string */
  sprintf(buf, "sc:%d lv:%d ls:%d ds:%d dp:%d ti:%d       %s", score, levelnum,
	  lives, diareq, diapoints, time_tck / 10, levname);
  if (strcmp(buf,oldbuf))
  {
  /* Clear the current score line */
#ifdef X11
  XFillRectangle(disp, wind, whitegc, 0, ELEM_YSIZE*y, ELEM_XSIZE*x, SCORESIZE);
#endif
#ifdef MGR
  m_func(BIT_CLR);
  m_bitwrite(0,ELEM_YSIZE*y,ELEM_XSIZE*x,SCORESIZE);
  m_func(BIT_SRC);
#endif
  /* Actually draw the text */
#ifdef X11
  XDrawString(disp, wind, scoregc, 0, ELEM_YSIZE*y + SCORESIZE - 1, buf,
	      strlen(buf));
#endif
#ifdef MGR
  m_stringto(0,0,ELEM_YSIZE*y+SCORESIZE-1,buf);
  m_func(bit_src_op);
#endif
  strcpy(oldbuf,buf);
  }
  scoreobs = False;
}

#ifdef X11
void
xstart(evmask)
  long            evmask;	/* Event mask which will be used in
				 * XSelectInput */
{
  XGCValues       xgcv;
  XWMHints        wmhints;

  /* create X window */
  disp = XOpenDisplay(NULL);
  /* Check to see if the open display succeeded */
  if (disp == NULL)
  {
    fprintf(stderr, "Display open failed.  Check DISPLAY environment variable.\n"
      );
    exit(-1);
  }
  wind = XCreateSimpleWindow(disp, DefaultRootWindow(disp), 0, 1,
			     ELEM_XSIZE*x, ELEM_YSIZE*y + SCORESIZE, 1,
			     WhitePixel(disp, 0), BlackPixel(disp, 0));
  /* Check to see if the open window succeeded */
  if (wind == 0)
  {
    fprintf(stderr, "Window open failed.\n");
    XCloseDisplay(disp);
    exit(-1);
  }
  /* Load in the font used to display the score */
  scorefont = XLoadFont(disp, SCOREFONT);
  /* Create GC which will be used from drawing score */
  xgcv.function = GXcopy;
  xgcv.font = scorefont;
  xgcv.foreground = BlackPixel(disp, 0);
  xgcv.background = WhitePixel(disp, 0);
  scoregc = XCreateGC(disp, wind,
		      GCFunction | GCFont | GCForeground | GCBackground,
		      &xgcv);
  /* Create GC which will be used for clearing score line */
  xgcv.function = GXcopy;
  scorefont = XLoadFont(disp, SCOREFONT);
  xgcv.foreground = WhitePixel(disp, 0);
  xgcv.background = BlackPixel(disp, 0);
  whitegc = XCreateGC(disp, wind,
		      GCFunction | GCForeground | GCBackground,
		      &xgcv);
  wmhints.flags = InputHint;
  wmhints.input = True;
  XSetWMHints(disp, wind, &wmhints);

  XSelectInput(disp, wind, evmask);
  XMapRaised(disp, wind);
  XWarpPointer(disp, None, wind, 0, 0, 0, 0, 11, 1);
}

/* Gracefully shut X windows down.  It is not strictly necessary to */
/* call this function. */
void
xend()
{
  gamestop = True;		/* tricky; prevent ticker function from using
				 * Xlib fcts */
  XUnloadFont(disp, scorefont);
  XUnmapWindow(disp, wind);
  XDestroyWindow(disp, wind);
  XCloseDisplay(disp);
}
#endif
#ifdef MGR
void mgrstart()
{
  ckmgrterm("mgrbd");
  m_setup(M_MODEOK);
  m_ttyset();
  m_push(P_ALL);
  m_setmode(M_ABS);
  m_setcursor(CS_INVIS);
  m_func(bit_src_op);
  m_setevent(REDRAW,"\022");
  m_flush();
}

void mgrend()
{
  m_setcursor(CS_BLOCK);
  m_popall();
  m_flush();
  m_ttyreset();
}
#endif

void
draw_field(redrawall)
  short           redrawall;
{
  char            c;

  /* Iterate through each horizontal line */
  for (i = y - 1; i >= 0; --i)
  {
    for (j = 0; j < x; ++j)
    {
      if (field[i][j].changed || redrawall)	/* only redraw changed cells */
      {
	c = field[i][j].content;
	switch (c)
	{
	case GRASS:
#ifdef X11
        XFillRectangle(disp, wind, ggc, ELEM_XSIZE*j, ELEM_YSIZE*i, ELEM_XSIZE, ELEM_YSIZE);
#endif
#ifdef MGR
	m_bitcopyto(ELEM_XSIZE*j,ELEM_YSIZE*i,ELEM_XSIZE,ELEM_YSIZE,0,0,0,ggc);
#endif
        break;
	case SPACE:
#ifdef X11
        XFillRectangle(disp, wind, sgc, ELEM_XSIZE*j, ELEM_YSIZE*i, ELEM_XSIZE, ELEM_YSIZE);
#endif
#ifdef MGR
	m_bitcopyto(ELEM_XSIZE*j,ELEM_YSIZE*i,ELEM_XSIZE,ELEM_YSIZE,0,0,0,sgc);
#endif
        break;
	case PLAYER:
#ifdef X11
        XFillRectangle(disp, wind, pgc, ELEM_XSIZE*j, ELEM_YSIZE*i, ELEM_XSIZE, ELEM_YSIZE);
#endif
#ifdef MGR
	m_bitcopyto(ELEM_XSIZE*j,ELEM_YSIZE*i,ELEM_XSIZE,ELEM_YSIZE,0,0,0,pgc);
#endif
	break;
	case WALL:
#ifdef X11
        XFillRectangle(disp, wind, wgc, ELEM_XSIZE*j, ELEM_YSIZE*i, ELEM_XSIZE, ELEM_YSIZE);
#endif
#ifdef MGR
	m_bitcopyto(ELEM_XSIZE*j,ELEM_YSIZE*i,ELEM_XSIZE,ELEM_YSIZE,0,0,0,wgc);
#endif
	break;
	case MAGICWALL:
#ifdef X11
        XFillRectangle(disp, wind, Wgc, ELEM_XSIZE*j, ELEM_YSIZE*i, ELEM_XSIZE, ELEM_YSIZE);
#endif
#ifdef MGR
	m_bitcopyto(ELEM_XSIZE*j,ELEM_YSIZE*i,ELEM_XSIZE,ELEM_YSIZE,0,0,0,Wgc);
#endif
	break;
	case DIAMOND:
#ifdef X11
        XFillRectangle(disp, wind, dgc, ELEM_XSIZE*j, ELEM_YSIZE*i, ELEM_XSIZE, ELEM_YSIZE);
#endif
#ifdef MGR
	m_bitcopyto(ELEM_XSIZE*j,ELEM_YSIZE*i,ELEM_XSIZE,ELEM_YSIZE,0,0,0,dgc);
#endif
	break;
	case BOULDER:
#ifdef X11
        XFillRectangle(disp, wind, bgc, ELEM_XSIZE*j, ELEM_YSIZE*i, ELEM_XSIZE, ELEM_YSIZE);
#endif
#ifdef MGR
	m_bitcopyto(ELEM_XSIZE*j,ELEM_YSIZE*i,ELEM_XSIZE,ELEM_YSIZE,0,0,0,bgc);
#endif
	break;
	case EXPLOSION:
#ifdef X11
        XFillRectangle(disp, wind, xgc, ELEM_XSIZE*j, ELEM_YSIZE*i, ELEM_XSIZE, ELEM_YSIZE);
#endif
#ifdef MGR
        m_func(bit_and_op);
	m_bitcopyto(ELEM_XSIZE*j,ELEM_YSIZE*i,ELEM_XSIZE,ELEM_YSIZE,0,0,0,xgc);
        m_func(bit_src_op);
#endif
	break;
	case LMONSTER:
#ifdef X11
        XFillRectangle(disp, wind, lgc, ELEM_XSIZE*j, ELEM_YSIZE*i, ELEM_XSIZE, ELEM_YSIZE);
#endif
#ifdef MGR
	m_bitcopyto(ELEM_XSIZE*j,ELEM_YSIZE*i,ELEM_XSIZE,ELEM_YSIZE,0,0,0,lgc);
#endif
	break;
	case RMONSTER:
#ifdef X11
        XFillRectangle(disp, wind, rgc, ELEM_XSIZE*j, ELEM_YSIZE*i, ELEM_XSIZE, ELEM_YSIZE);
#endif
#ifdef MGR
	m_bitcopyto(ELEM_XSIZE*j,ELEM_YSIZE*i,ELEM_XSIZE,ELEM_YSIZE,0,0,0,rgc);
#endif
	break;
	case NUCBAL:
#ifdef X11
        XFillRectangle(disp, wind, ngc, ELEM_XSIZE*j, ELEM_YSIZE*i, ELEM_XSIZE, ELEM_YSIZE);
#endif
#ifdef MGR
	m_bitcopyto(ELEM_XSIZE*j,ELEM_YSIZE*i,ELEM_XSIZE,ELEM_YSIZE,0,0,0,ngc);
#endif
	break;
	case BLOB:
#ifdef X11
        XFillRectangle(disp, wind, Bgc, ELEM_XSIZE*j, ELEM_YSIZE*i, ELEM_XSIZE, ELEM_YSIZE);
#endif
#ifdef MGR
	m_bitcopyto(ELEM_XSIZE*j,ELEM_YSIZE*i,ELEM_XSIZE,ELEM_YSIZE,0,0,0,Bgc);
#endif
	break;
	case TINKLE:
#ifdef X11
        XFillRectangle(disp, wind, tgc, ELEM_XSIZE*j, ELEM_YSIZE*i, ELEM_XSIZE, ELEM_YSIZE);
#endif
#ifdef MGR
	m_bitcopyto(ELEM_XSIZE*j,ELEM_YSIZE*i,ELEM_XSIZE,ELEM_YSIZE,0,0,0,tgc);
#endif
	break;
	case EATER:
#ifdef X11
        XFillRectangle(disp, wind, egc, ELEM_XSIZE*j, ELEM_YSIZE*i, ELEM_XSIZE, ELEM_YSIZE);
#endif
#ifdef MGR
	m_bitcopyto(ELEM_XSIZE*j,ELEM_YSIZE*i,ELEM_XSIZE,ELEM_YSIZE,0,0,0,egc);
#endif
	break;
	case EXIT:
#ifdef X11
        XFillRectangle(disp, wind, Egc, ELEM_XSIZE*j, ELEM_YSIZE*i, ELEM_XSIZE, ELEM_YSIZE);
#endif
#ifdef MGR
	  m_bitcopyto(ELEM_XSIZE*j,ELEM_YSIZE*i,ELEM_XSIZE,ELEM_YSIZE,0,0,0,Egc);
#endif
	  break;
	case STEEL:
        default:
	  field[i][j].content = STEEL;
#ifdef X11
	  XFillRectangle(disp, wind, Sgc, ELEM_XSIZE*j, ELEM_YSIZE*i, ELEM_XSIZE, ELEM_YSIZE);
#endif
#ifdef MGR
	  m_bitcopyto(ELEM_XSIZE*j,ELEM_YSIZE*i,ELEM_XSIZE,ELEM_YSIZE,0,0,0,Sgc);
#endif
	  break;
	}
	field[i][j].changed = False;
      }
    }
  }
  if (scoreobs)
    draw_score();
}

void
set_cell(i, j, content)
  int             i, j;
  char            content;
{
  field[i][j].content = content;
  field[i][j].speed = 0;
  field[i][j].changed = True;
  field[i][j].stage = 0;
  field[i][j].caught = True;
  field[i][j].checked = False;
}
