/*{{{}}}*/
/*{{{  Notes*/
/*
 *	This program is Copyright (C) 1987 by the Board of Trustees of the
 *	University of Illinois, and by the author Dirk Grunwald.
 *
 *	This program may be freely copied, as long as this copyright
 *	message remains affixed. It may not be sold, altough it may
 *	be distributed with other software which is sold. If the
 *	software is distributed, the source code must be made available.
 *
 *	No warrenty, expressed or implied, is given with this software.
 *	It is presented in the hope that it will prove useful.
 *
 *	Adapted for the 3b1 by Andy Fyfe  (andy@csvax.caltech.edu)
 *	Fixes from John Campbell  (...!arizona!naucse!thunde!jdc)
 *	Adapted for MGR by Ross A. Jekel (s83949@ursa.calvin.edu)
 *      Again fixes, this time from Michael Haardt.
 *	Fixes for Linux Mgr and for aspect_ratio/=1, nov93 broman@nosc.mil
 */
/*}}}  */
/*{{{  #includes*/
#ifndef OLDMGR
#include <mgr/mgr.h>
#include <mgr/bitblit.h>
#include <unistd.h>
#define B_SIZESERV(w,h,d) B_SIZE8(w,h,d)
#else /* OLDMGR */
#include <bitmap.h>
#include <term.h>
#include <dump.h>
#define B_SIZESERV(w,h,d) B_SIZE16(w,h,d)
#endif
#include <ctype.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "dvistuff.h"
#include "proto.h"

/*}}}  */
/*{{{  #defines*/
#ifndef __STDC__
#define const
#endif

#ifndef __GNUC__
#define inline
#define volatile
#endif

#ifndef PAGE_HEIGHT
#define PAGE_HEIGHT	11		/* in inches */
#endif

#ifndef PAGE_WIDTH
#define PAGE_WIDTH	8.5		/* in inches */
#endif

#ifndef FONT_DPI
#define FONT_DPI	300
#endif

#ifndef BLACKNESS
#define BLACKNESS	3
#endif

#ifndef B_CLEAR
#define B_CLEAR BIT_CLR
#endif
#ifndef B_OR
#define B_OR BIT_OR
#endif
#ifndef B_SRC
#define B_SRC BIT_SRC
#endif
#ifndef B_SET
#define B_SET BIT_SET
#endif

#define SHRINKS 3		/* Be careful if you change this! */

#define MAXFUNC(a,b) (((a) < (b)) ? (b) : (a))
#define MINFUNC(a,b) (((a) < (b)) ? (a) : (b))
/*}}}  */

/*{{{  variables*/
static char *version="1.4";

/*
 *	Mgr related variables
 */
typedef short int Int;		/* 3b1's like short's */

static Int	Image[SHRINKS] = { 1, 2, 3 },
		X_origin[SHRINKS], Y_origin[SHRINKS],
		win_width = 0, win_height = 0,
		disp_width, disp_height, using_full_screen,
		globalNumber,
		globalShrink,
		initialShrink,
		inHelp = 0,
		func = 1,
		firstPage;

extern char *dviFileName;

/*
 *	TeX-Dvi related variables
 */

static Int	maxHeight[SHRINKS], maxWidth[SHRINKS], bytesWide[SHRINKS],
		imageWidth[SHRINKS], imageHeight[SHRINKS],
		screenWidth[SHRINKS], screenHeight[SHRINKS],
		shrink[SHRINKS],
		page_w[SHRINKS], page_h[SHRINKS], page_num[SHRINKS];

static struct glyph **shrunkenGlyphs[SHRINKS][MAX_FONTFAMILY];
/*}}}  */

/*{{{  clearPixmap*/
static void clearPixmap(shk) int shk;
{
    if (func!=B_CLEAR) m_func(func=B_CLEAR);
    m_bitwriteto(0, 0, maxWidth[shk], maxHeight[shk], Image[shk]);
}
/*}}}  */
/*{{{  displayLeaves -- display the normal sized leaf*/
static void displayLeaves()
{
    int width, height, byteswide, srcx, srcy;
    static int last_width = 0, last_height;

    byteswide = bytesWide[globalShrink];
    srcx = X_origin[globalShrink];
    srcy = Y_origin[globalShrink];
    width = screenWidth[globalShrink];
    height = screenHeight[globalShrink];

    if ((width < last_width && width < win_width) ||
	(height < last_height && height < win_height))
	m_clear();

    if (func!=B_SRC) m_func(func=B_SRC);
    m_bitcopyto(0, 0, width, height, srcx, srcy, 0, Image[globalShrink]);
    m_flush();

    last_width = width;
    last_height = height;
}
/*}}}  */
/*{{{  buildLeaf*/
static void
buildLeaf(void)
{
    clearPixmap(globalShrink);

    if (dviCurrentPage < 0)
	dviCurrentPage = 0;
    if (dviCurrentPage >= dviTotalPages)
	dviCurrentPage = dviTotalPages - 1;

    dviPreparePage(dviCurrentPage);

    page_num[globalShrink] = dviCurrentPage;
}
/*}}}  */

/*{{{  stop_output*/
static void stop_output(int status)
{
    m_bitdestroy(Image[0]);
    m_bitdestroy(Image[1]);
    m_bitdestroy(Image[2]);
    m_ttyreset();
    m_popall();
    dviFini();
    exit(status);
}
/*}}}  */
/*{{{  adjust_origin*/
static void adjust_origin(void)
{
    if (X_origin[globalShrink] >=
      imageWidth[globalShrink] - screenWidth[globalShrink])
	X_origin[globalShrink] =
	    imageWidth[globalShrink] - screenWidth[globalShrink];
    if (X_origin[globalShrink] < 0)
	X_origin[globalShrink] = 0;
    if (Y_origin[globalShrink] >=
      imageHeight[globalShrink] - screenHeight[globalShrink])
	Y_origin[globalShrink] =
	    imageHeight[globalShrink] - screenHeight[globalShrink];
    if (Y_origin[globalShrink] < 0)
	Y_origin[globalShrink] = 0;
}
/*}}}  */
/*{{{  resized_window*/
static void resized_window(void)
{
    Int i;
    int width, height;

    m_getinfo(G_COORDS);
    m_flush();
    if (fscanf(m_termin, "%*d %*d %d %d%*c", &width, &height) != 2) {
	fprintf(stderr, "%s: m_getinfo(G_COORDS) failed: ", ProgName);
	perror("");
	stop_output(1);
    }

    if (width != win_width || height != win_height) {
	win_width = width;
	win_height = height;
	i=SHRINKS-1;
	do {
	    screenWidth[i] = MINFUNC(imageWidth[i], win_width);
	    screenHeight[i] = MINFUNC(imageHeight[i], win_height);
	} while (i--);
	if (!inHelp) {
	    adjust_origin();
	    displayLeaves();
	}
    }
}
/*}}}  */


/*{{{  toggle_full_screen*/
static void
toggle_full_screen(void)
{
int new_width;

    using_full_screen = ! using_full_screen;
    new_width = using_full_screen? disp_width: disp_height * 4 / 5;
    m_shapewindow(0, 0, new_width, disp_height);
    resized_window();
}
/*}}}  */

/*{{{  change_mag*/
static void change_mag(int sh) /* zero-based */
{
    int prev = globalShrink;
    int next;
    int x, y, u, v;

    if( sh < 0)			next = 0;
    else if( sh >= SHRINKS)	next = SHRINKS - 1;
    else			next = sh;
    x = screenWidth[prev] / 2;
    y = screenHeight[prev] / 2;
    u = (X_origin[prev] + x) * shrink[prev];
    v = (Y_origin[prev] + y) * shrink[prev];

    X_origin[next] = u / shrink[next] - screenWidth[next]/2;
    Y_origin[next] = v / shrink[next] - screenHeight[next]/2;

    globalShrink = next;
}
/*}}}  */
/*{{{  getmouse*/
static void getmouse(void)
{
    int win_x, win_y;

    fscanf(m_termin, "%d %d%*c", &win_x, &win_y);

    if (win_x >= 0 && win_x < win_width && win_y >= 0 && win_y < win_height) {
    	X_origin[globalShrink] += win_x - screenWidth[globalShrink]/2;
    	Y_origin[globalShrink] += win_y - screenHeight[globalShrink]/2;
    }
}
/*}}}  */
/*{{{  getKeyboard*/
static int getKeyboard(void)
{
    char ch;

    ch = m_getchar();
    if (ch != '\033')
	return ch;

    /*
     * if the char is escape, try and interpret escape sequences and return
     * an equivalent single character.  mouse clicks on border icons return
     * a key sequence.  resize icon handled by sigwind.
     */

    switch (m_getchar()) {
    case '[':
	switch(m_getchar()) {
	case 'A':	/* up arrow */
	case 'T':	/* shift up arrow */
	    return 'k';
	case 'B':	/* down arrow */
	case 'S':	/* shift down arrow */
	    return 'j';
	case 'C':	/* right arrow */
	    return 'l';
	case 'D':	/* left arrow */
	    return 'h';
	case 'H':	/* home */
	    return 'r';
	case 'U':	/* page down */
	    return 'J';
	case 'V':	/* page up */
	    return 'K';
	}
	break;
    case 'O':
	switch(m_getchar()) {
	case 'm':	/* help */
	case 'M':	/* shift help */
	    return '?';
	case 'k':	/* exit */
	case 'K':	/* shift exit */
	case 'w':	/* cancl */
	case 'W':	/* shift cancl */
	    return 'q';
	case 'v':	/* open */
	    return 'R';
	}
	break;
    case 'N':
	switch(m_getchar()) {
	case 'g':	/* prev */
	    return 'b';
	case 'h':	/* next */
	    return 'f';
	case 'L':	/* shift right arrow */
	    return 'l';
	case 'K':	/* shift left arrow */
	    return 'h';
	}
	break;
    case '9':		/* beg */
	globalNumber = 1;
	return 'g';
    case '0':		/* end */
	globalNumber = dviTotalPages;
	return 'g';
    }
    return -1;
}
/*}}}  */
/*{{{  help_screen*/
static void help_screen(void)
{
    inHelp = 1;
    m_clear();

    fprintf (m_termout, "\
    h, j, k, l, H, J, K, L, <ff>, r,\r\n\
       <arrows>, <page>, <home>:        move around the page\r\n\
    <spc>, f, n, <nl>, <cr>, <next>:    next page       (*)\r\n\
    b, p, <bs>, <del>, <prev>:          previous page   (*)\r\n\
    g, G, <beg>, <end>:                 goto page       (*)\r\n\
    m, M:                               change the magnification\r\n\
    R, <open>:                          re-open dvi file\r\n\
    ?, <help>:                          this help screen\r\n\
    q, x, ^C, ^D, <cancl>, <exit>:      exit\r\n\
\r\n\
The (*) commands can be prefixed by a number.  The program can\r\n\
display the dvi file at %d, %d and %d dpi.  The mouse can also\r\n\
be used to change magnifications (using the middle button menu)\r\n\
and move a point to the centre of the screen (using the right button).\r\n\
\r\n\
    Press any key to continue\r\n",
	dviDPI/shrink[0], dviDPI/shrink[1], dviDPI/shrink[2]);

    getKeyboard();

    m_clear();
    inHelp = 0;
}
/*}}}  */
/*{{{  reReadFile -- prepare everything for re-reading a .dvi file*/
static void reReadFile(void)
{
    Int i = SHRINKS - 1;

    dviResetAll();

    if (dviInit()) {
	fprintf(stderr,"Unable to reload dvi file (%s), exiting\n",
		dviFileName);
	stop_output(1);
    }

    do {
	page_h[i] = (dviTallestPage + 2 * dviDPI + shrink[i] - 1) / shrink[i];
	page_w[i] = (dviWidestPage + 2 * dviDPI + shrink[i] - 1) / shrink[i];
	page_num[i] = -1;
	imageWidth[i] = MINFUNC(page_w[i], maxWidth[i]);
	imageHeight[i] = MINFUNC(page_h[i], maxHeight[i]);
    } while (i--);

    buildLeaf();

    globalNumber = 0;
}
/*}}}  */
/*{{{  setShrinks*/
static void setShrinks(char *values)
{
    Int s1, s2, s3;

    s1 = values[0] - '0';
    s2 = values[1] - '0';
    s3 = values[2] - '0';

    if (s1 < 0 || s1 > 9 || s2 < 0 || s2 > 9 || s3 < 0 || s3 > 9) {
	fprintf(stderr,
	    "%s: Illegal shrink values %s: must be 3 single digits\n",
	    ProgName, values);
	exit(1);
    }

    shrink[0] = s1;
    shrink[1] = s2;
    shrink[2] = s3;
}
/*}}}  */
/*{{{  check_input*/
static void check_input(void)
{
    Int ch, direction, s;
    
#define BOGUSDIR -1
#define FOREWARD 0
#define BACKWARD 1
#define ABSOLUTE 2
    
    direction = BOGUSDIR;

    ch = getKeyboard();

    switch(ch) {
    case 'h':
	X_origin[globalShrink] -= dviDPI/4/shrink[globalShrink];
	break;

    case 'k':
	Y_origin[globalShrink] -= dviDPI/2/shrink[globalShrink];
	break;

    case 'l':
	X_origin[globalShrink] += dviDPI/4/shrink[globalShrink];
	break;

    case 'j':
	Y_origin[globalShrink] += dviDPI/2/shrink[globalShrink];
	break;

    case 'H':
	X_origin[globalShrink] = 0;
	break;

    case 'K':
	Y_origin[globalShrink] = 0;
	break;

    case 'L':
	X_origin[globalShrink] =
	    imageWidth[globalShrink] - screenWidth[globalShrink];
	break;

    case 'J':
	Y_origin[globalShrink] =
	    imageHeight[globalShrink] - screenHeight[globalShrink];
	break;

    case 'q':
    case 'x':
    case '\003':	/* control-C */
    case '\004':	/* control-D */
	stop_output(0);
	
    case 'n':
    case 'f':
    case ' ':
    case '\n':
    case '\r':
	/* scroll forward */
	direction = FOREWARD;
	break;
	
    case 'p':
    case 'b':
    case '\b':
    case '\177' : /* DEL */
	/* scroll backward */
	direction = BACKWARD;
	break;
	
    case 'g':
    case 'G':
	/* go to absolute page */
	direction = ABSOLUTE; /* may not be, but tough */
	break;
	
    case '\f':
    case 'r' :
	X_origin[globalShrink] = Y_origin[globalShrink] = 0;
	break;

    case 'm':
	change_mag( globalShrink + 1);
	break;

    case 'M':
	change_mag( globalShrink - 1);
	break;

    case 'R':
	reReadFile();
	break;
	
    case 's':
	change_mag( globalNumber);
	break;

    case 'S':
	resized_window();
	break;

    case 'P':
	getmouse();
	break;

    case 'Z':
	toggle_full_screen();
	break;

    case '0': case '1': case '2': case '3': case '4':
    case '5': case '6': case '7': case '8': case '9':
	globalNumber = 10*globalNumber + (ch - '0');
	return;
    
    case '?':
	help_screen();
	break;

    case '\000':
	break;

    default:
	globalNumber = 0;
	return;
    }
    
    adjust_origin();

    if (direction != BOGUSDIR) {
	if (globalNumber == 0)
	    globalNumber = 1;

/* 
 *	Turn pages 
 */
	if (direction == BACKWARD)
	    dviCurrentPage -= globalNumber;
	else if (direction == FOREWARD)
	    dviCurrentPage += globalNumber;
	else
	    dviCurrentPage = globalNumber - 1;	/* pages globalNumbered at 0 */
	
	/* set origin to 1in,1in from upper left corner of page */
	for( s = 0; s < SHRINKS; s += 1)
	    X_origin[ s] = Y_origin[ s] = dviDPI / shrink[ s];

	dviCurrentPage = MINFUNC(dviCurrentPage, dviTotalPages-1);
	dviCurrentPage = MAXFUNC(dviCurrentPage, 0);
    }

    if (dviCurrentPage != page_num[globalShrink])
	buildLeaf();
    
    displayLeaves();
    globalNumber = 0;
}
/*}}}  */
/*{{{  usage*/
static void usage(void)
{
    fprintf(stderr, "%s: usage: %s [options] dvi-file\noptions:\n\
    -b <blackness>\n\
    -d <dvi Dots Per Inch>\n\
    -f <first (phyical) page>\n\
    -m <default magnification>\n\
    -s <shrink factors (eg 234)>\n\
    -V      [print version]\n", ProgName, ProgName);
    exit(1);
}
/*}}}  */
/*{{{  main*/
int main(int argc, char *argv[])
{
    /*{{{  variables*/
    Int i;
    int w, h;
    int shrinks_chosen;
    char menu_line[100];
    extern int optind;
    extern char *optarg;
    /*}}}  */

    ProgName = argv[0];
    firstPage = 1;
    initialShrink = 1;
    dviDPI = FONT_DPI;
    dviBlackness = BLACKNESS;
    shrinks_chosen = 0;

    while ((i = getopt(argc, argv, "b:d:f:m:s:V")) != EOF) {
	switch(i) {
	case 'b':
	    dviBlackness = atoi(optarg);
	    break;
	case 'd':
	    dviDPI = atoi(optarg);
	    break;
	case 'f':
	    firstPage = atoi(optarg);
	    break;
	case 'm':
	    initialShrink = atoi(optarg) - 1;
	    if (initialShrink < 0)
		initialShrink = 0;
	    else if (initialShrink >= SHRINKS)
		initialShrink = SHRINKS - 1;
	    break;
	case 's':
	    setShrinks(optarg);
	    shrinks_chosen = 1;
	    break;
	case 'V':
	    fprintf(stderr,"%s: version %s\n", ProgName, version);
	    usage();
	    break;
	case '?':
	    usage();
	    break;
	}
    }

    if (optind != argc - 1)
	usage();
    else
	dviFileName = argv[optind];

    if( !shrinks_chosen) {
	shrink[ 2] = MAXFUNC( dviDPI / 75, 3);
	shrink[ 1] = shrink[ 2] * 3 / 4;
	shrink[ 0] = shrink[ 1] * 2 / 3;
    }

    if (dviInit()) {
	dviFini();
	exit(1);
    }

    /*
     * Make sure we're running on a terminal
     */
    i = open("/dev/tty", O_RDWR, 0);
    if (i < 0) {
	fprintf(stderr, "%s: can't open /dev/tty: ", ProgName);
	perror("");
	dviFini();
	exit(1);
    }
    close(i);

    if (! is_mgr_term()) {
	fprintf(stderr, "%s must be run from an MGR window.\n", ProgName);
	dviFini();
	exit(1);
    }

    /*
     * Reshape window and set miscellaneous window attributes.
     */
    m_setup(M_MODEOK);
    m_push(P_ALL);
    m_setcursor(CS_INVIS);
    m_setmode(M_ABS);
    sprintf(menu_line,
	"|Help|Shrink 1/%1d|Shrink 1/%1d|Shrink 1/%1d|Page|Quit|?|0s|1s|2s|n|q|",
	shrink[0], shrink[1], shrink[2]);
    m_loadmenu(0, menu_line);
    m_selectmenu(0);

    /*
     * Set the window to use the full screen height
     * and the left-hand part of the width.
     */
    m_ttyset();
    m_getscreensize( &w, &h, (int *)0);
    disp_width = w;
    disp_height = h;
    using_full_screen = 1;
    toggle_full_screen();	/* reshapes */
    resized_window();		/* needed immediately */

    m_setevent(RESHAPE, "S");
    m_setevent(BUTTON_1, "P %p\n");

    m_clear();
    m_flush();
    m_setraw();

    i=SHRINKS-1;
    do {
	X_origin[i] = dviDPI / shrink[i];
	Y_origin[i] = dviDPI / shrink[i];	/* 1 in, 1 in from topleft */
	maxWidth[i]  = ((int)(PAGE_WIDTH *dviDPI) + (shrink[i]-1)) / shrink[i];
	maxHeight[i] = ((int)(PAGE_HEIGHT*dviDPI) + (shrink[i]-1)) / shrink[i];
	bytesWide[i] = B_SIZESERV(maxWidth[i],1,1);
	maxWidth[i]  = bytesWide[i] * 8;
    } while (i--);

    globalShrink = initialShrink;

    if (((double) dviTallestPage * (double) dviWidestPage) > 4.0e7) {
	fprintf(stderr,"[%s] Warning: Your page size is %d wide and %d tall,\n",
		ProgName, dviTallestPage, dviWidestPage);
	fprintf(stderr,"which may be too big to be displayed\n");
    }

    i=SHRINKS-1;
    do {
	page_h[i] = (dviTallestPage + 2 * dviDPI + shrink[i] - 1) / shrink[i];
	page_w[i] = (dviWidestPage + 2 * dviDPI + shrink[i] - 1) / shrink[i];
	page_num[i] = -1;
	imageWidth[i] = MINFUNC(page_w[i], maxWidth[i]);
	imageHeight[i] = MINFUNC(page_h[i], maxHeight[i]);
    } while (i--);

    i=SHRINKS-1;
    do {
	screenWidth[i] = MINFUNC(imageWidth[i], win_width);
	screenHeight[i] = MINFUNC(imageHeight[i], win_height);
    } while (i--);

    globalNumber = 0;

    dviCurrentPage = firstPage - 1;

    buildLeaf();
    displayLeaves();

    while (1)
	check_input();
    return 255;
}
/*}}}  */

/*
 *	interfaces to dvistuff
 */

/*{{{  applicationNewfont*/
/*
 *	Whenever a new font is registers, we create a shrunken Glyph
 *	table for it. However, we don't shrink the glyphs -- that's
 *	done on the fly by the putChar routine.
 */

DviFont *
applicationNewFont(f, key)
struct font *f;
int key;
{
    Int shk = SHRINKS-1;

    if (key < 0 || key > MAX_FONTFAMILY) {
	fprintf(stderr,"[%s] bogus key in Newfont = %d\n",
		ProgName, key);
	stop_output(1);
    }
    
    do {
	if (shrunkenGlyphs[shk][key] == 0) {
	    int lth = sizeof(struct glyph *) * MAX_GLYPH;
	    struct glyph **g;
	    
	    g = (struct glyph **) malloc( lth );
	    bzero(g, lth);
	    shrunkenGlyphs[shk][key] = g;
	}
    } while (shk--);

    return(f);
}
/*}}}  */
/*{{{  applicationResetFont*/
/*
 *	When we reset a font, we only need to free the storage for the
 *	shrunken glyphs. We keep the glyph table available because we're
 *	very likely to fill it in again.
 */

void
applicationResetFont(fi, key)
struct fontinfo *fi;
int key;
{
    Int i, shk = SHRINKS - 1;
    struct glyph **theseGlyphs;
    
    do
	if ((theseGlyphs = shrunkenGlyphs[shk][key]) != 0) {
	    i = MAX_GLYPH - 1;
	    do {
		struct glyph *g = theseGlyphs[i];
		
		if (g != 0) {
		    if ( g -> g_raster != 0) {
			free(g -> g_raster);
		    }
		    free(g);
		    theseGlyphs[i] = 0;
		}
	    } while (i--);
	}
    while (shk--);
}
/*}}}  */
/*{{{  applicationPutChar*/
void
applicationPutChar(hh, vv, charCode)
int hh;
int vv;
int charCode;
{
    register struct glyph *g;
    Int x, y, key, size;
    
    key = dviCurrentFont -> family;

    g = shrunkenGlyphs[globalShrink][key][charCode];

    if (g == 0)
	shrunkenGlyphs[globalShrink][key][charCode] =
		g = dviShrinkGlyph(dviCurrentFont -> f -> f_gly[charCode],
	    		shrink[globalShrink], shrink[globalShrink]);

    if (g == 0 || !HASRASTER(g)) return;

    hh /= shrink[globalShrink];
    vv /= shrink[globalShrink];

    x = hh - g -> g_xorigin;
    y = vv - g -> g_yorigin;

    size=B_SIZESERV(g->g_width,g->g_height,1);
    if (func!=B_OR) m_func(func=B_OR);

    m_bitldto((int)(g->g_width), (int)(g->g_height),
	      (x > imageWidth[globalShrink]) ? x-imageWidth[globalShrink] : x, y,
	      Image[globalShrink], size);
    fwrite(g->g_raster, size, 1, m_termout);

#if 0
/* nonworking sketch */
    {
    FILE *fp;
    BITMAP *bp;
    char *bmfn = "/tmp/texmgr";
    int bmnbr = 10;
    int res, w, h;

    bp = bit_alloc( g->g_width, g->g_height, g->g_raster, 1);
    if( !bp) exit( 11);
    fp = fopen( bmfn, "w");
    if( !fp) exit( 12);
    res = bitmapwrite( fp, bp);
    if( !res) exit( 13);
    fclose( fp);
    res = m_bitfile( bmnbr, bmfn, &w, &h, (int *)0);
    if( !res) exit( 14);
    unlink( bmfn);
    m_bitcopyto( x, y, w, h, 0, 0, Image[globalShrink], bmnbr);
    m_bitdestroy( bmnbr);
    }
#endif

    m_flush();
}
/*}}}  */
/*{{{  put_rectangle*/
void put_rectangle(x, y, w, h)
int x, y, w, h;
{
    if (func!=B_SET) m_func(func=B_SET);
    m_bitwriteto((x > imageWidth[globalShrink]) ? x-imageWidth[globalShrink] : x, y,
		 w, h, Image[globalShrink]);
    m_flush();
}
/*}}}  */
/*{{{  applicationSetRule*/
void
applicationSetRule(hh, vv, h, w)
int hh, vv;
int h, w;
{
    int nh, nw;

    hh /= shrink[globalShrink];
    vv /= shrink[globalShrink];

    nh = h / shrink[globalShrink];
    nw = w / shrink[globalShrink];

    if (nh == 0 && h != 0)
	nh = 1;
    if (nw == 0 && w != 0)
	nw = 1;

    put_rectangle(hh, vv - nh, nw, nh);
}
/*}}}  */
/*{{{  put_border*/
void put_border(x, y, w, h, t)
int x, y, w, h, t;
{
    put_rectangle(x, y, w, t);
    put_rectangle(x, y, t, h);
    put_rectangle(x, y + h - t, w, t);
    put_rectangle(x + w - t, y, t, h);
}
/*}}}  */
/*{{{  psfigBegin -- draw the bounding box for a \psfig special*/
/*
 * expected format of the command string is
 *
 * width height bbllx bblly bburx bbury
 *
 * *ll* means lower-left, *ur* means upper-right.
 */
void psfigBegin(cp)
char *cp;
{
  int bbllx, bblly;
  int bburx, bbury;
  int width, height;

  sscanf(cp, " %d %d %d %d %d %d ",
	 &width, &height,
	 &bbllx, &bblly, &bburx, &bbury);

  bbllx = ( dviHH) / shrink[globalShrink];
  bblly = ( dviVV) / shrink[globalShrink];

  width = fromSP(width) / shrink[globalShrink];
  height = fromSP(height) / shrink[globalShrink];

  put_border( bbllx, bblly, width, height, 1);
}
/*}}}  */
/*{{{  applicationDoSpecial*/
void
applicationDoSpecial(cp)
char *cp;
{
#define	COMLEN	128
    char command[COMLEN], *orig_cp;
    register int len;

    orig_cp = cp;
    while (isspace(*cp)) ++cp;
    len = 0;
    while (!isspace(*cp) && *cp && len < COMLEN-1) command[len++] = *cp++;
    command[len] = '\0';
    if (strcmp(command, "ps::[begin]") == 0) psfigBegin(cp);
    else if (strcmp(command, "ps::plotfile") == 0) 	/* do nothing */;
    else if (strcmp(command, "ps::[end]") == 0) 	/* do nothing */;
    else if (strcmp(command, "ps:plotfile") == 0)	/* do nothing */;
    else if (strcmp(command, "ps:") == 0)		/* do nothing */;
    else if (strcmp(command, "ps::") == 0) 		/* do nothing */;
    else fprintf(stderr, "[%s] special \"%s\" not implemented\n",
		ProgName, orig_cp);
}
/*}}}  */
