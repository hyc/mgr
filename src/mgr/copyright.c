/*{{{}}}*/
/*{{{  Notes*/
/*                        Copyright (c) 1988 Bellcore
 *                            All Rights Reserved
 *       Permission is granted to copy or use this program, EXCEPT that it
 *       may not be sold for profit, the copyright notice must be reproduced
 *       on copies, and credit should be given to Bellcore where it is due.
 *       BELLCORE MAKES NO WARRANTY AND ACCEPTS NO LIABILITY FOR THIS PROGRAM.
*/

/*

Flash a copyright notice at Mgr startup.  You are not allowed to remove
this, it is a part of the conditions on which you are allowed to use MGR
without paying fees.

*/

/*
 * porter.c  Steve Hawley 4/3/87
 * rehacked 5/18/1988 for extra speed.
 * re-re hacked 6/20/88 for MGR (SAU)
 * re-re-re hacked by broman for use as screensaver.
*/
/*}}}  */
/*{{{  #includes*/
#include <mgr/bitblit.h>
#include <mgr/font.h>
#include <sys/time.h>
#include <sys/signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "defs.h"

#include "proto.h"
#include "colormap.h"
#include "icon_server.h"
/*}}}  */
/*{{{  #defines*/
#define SSIZE	3		/* star size */

#define MAXZ 500 /* maximum z depth */
#define MAXZ 500 /* maximum z depth */
#define NSTARS 256 /* maximum number of stars */
#define SPEED	4		/* star speed */
#define SCALE	(short)6	/* for rotator */
#define COUNT	(short)2	/* for rotator */
#define ON 1  /* plotting states */
#define OFF 0
#define Random() ((unsigned int)rand())
/*}}}  */

/*{{{  types*/
/* for "star trek" clip areas */

typedef struct rect 
{
  short x1,y1;
  short x2,y2;
} RECT;
static RECT clip1, clip2, clip3;	/* holes in the galaxy */
/*}}}  */
/*{{{  variables*/
static BITMAP *logo[] =
{ &ball_1, &ball_2, &ball_3, &ball_4, &ball_5, &ball_6, &ball_7, &ball_8};

static struct timeval delay = 
{
  0L, 100000L
};

static short maxv, maxh; /* display size */
static short hmaxv, hmaxh;	/* 1/2 display size */
static int clockwise = 0;

static struct st 
{
  short x, y, z;
  short color;
} stars[NSTARS]; /* our galaxy */
/*}}}  */

/*{{{  init_all*/
void init_all(where) register BITMAP *where;
{
  maxv = BIT_HIGH(where);
  hmaxv = maxv>>1;
  maxh = BIT_WIDE(where);
  hmaxh = maxh>>1;
}       
/*}}}  */
static void flip(void) {
  clockwise = !clockwise;
}
/*{{{  cordic*/
/* CORDIC rotator. Takes as args a point (x,y) and spins it */
/* count steps counter-clockwise.                   1       */
/*                                Rotates atan( --------- ) */
/*                                                  scale   */
/*                                                 2        */
/* Therefore a scale of 5 is 1.79 degrees/step and          */
/* a scale of 4 is 3.57 degrees/step                        */

static void cordic(x, y, scale, count)
short *x, *y;
register short scale, count;

{
  register short tempx, tempy;

  tempx = *x;
  tempy = *y;

  if (! clockwise)
  for (; count; count--)
  {
    tempx -= (tempy >> scale);
    tempy += (tempx >> scale); 
  }
  else
  for (; count; count--)
  {
    tempx += (tempy >> scale);
    tempy -= (tempx >> scale);
  }

  *x = tempx;
  *y = tempy;
}
/*}}}  */
/*{{{  xplot*/
static int xplot(where,x, y, col, state)
register BITMAP *where;
register int x, y;
register int col;
int state;
{
  /* are we on the screen? If not, let the caller know*/
  if (x < 0 || x >= maxh || y < 0 || y >= maxv ) return(1);
  if (!(x < clip1.x1 || x >= clip1.x2 || y < clip1.y1 || y >= clip1.y2 )) return(0);
  if (!(x < clip2.x1 || x >= clip2.x2 || y < clip2.y1 || y >= clip2.y2 )) return(0);
  if (!(x < clip3.x1 || x >= clip3.x2 || y < clip3.y1 || y >= clip3.y2 )) return(0);

  bit_blit(where,x,y,SSIZE,SSIZE, state ?
    BUILDOP(BIT_SET,col,color_map[LOGO_COLOR_BG]) :
    BUILDOP(BIT_CLR,col,color_map[LOGO_COLOR_BG]) ,
  (BITMAP*)0,0,0);
  return(0);
}
/*}}}  */
/*{{{  project*/
static int project(where,x, y, z, col, state)
register BITMAP *where;
register short x, y, z;
register int col;
register short state;
{
        
  /* one-point perspective projection */
        /* the offsets (maxh/2) and maxv/2) ensure that the
         * projection is screen centered
  */
  x = (x/z) + hmaxh;
  y = (y/z) + hmaxv;
  return(xplot(where,x, y, col, state));

}
/*}}}  */
/*{{{  fly*/
static void fly (where) BITMAP *where;
{
  register short i;
  register struct st *stp;

  init_all(where);     /* set up global variables */
  for (i=0,stp=stars; i<NSTARS; i++,stp++)
  {
    /* initialize galaxy */
    do 
    {
      stp->x = Random();
      stp->y = Random();
      stp->z = (Random() % MAXZ) + 1;
      stp->color = Random() & ((1 << BIT_DEPTH(where))-1);
    } while(project(where,stp->x, stp->y, stp->z, stp->color, ON)); /* on screen? */
  }
}
/*}}}  */
/*{{{  dofly*/
static void dofly (where) BITMAP *where;
{
  register short i;
  register struct st *stp;

  i = NSTARS;
  stp = stars;
  do 
  {
    project(where,stp->x, stp->y, stp->z, stp->color, OFF); /* turn star off*/
    if ((stp->z -= SPEED) <= 0) { /* star went past us */
      stp->x = Random();
      stp->y = Random();
      stp->z = MAXZ;
    }
    else {		/* rotate universe */
      cordic(&stp->x,&stp->y,SCALE,COUNT);
    }
    if (project(where,stp->x, stp->y, stp->z, stp->color, ON)) 
    {
      /* if projection is off screen, get a new position */
      stp->x = Random();
      stp->y = Random();
      stp->z = MAXZ;
    }
    ++stp;
  } while(--i);
}
/*}}}  */

/*{{{  copyright*/
void copyright(BITMAP *where, char *password)
{
  BITMAP *notice = &cr;
  fd_set mask;
  int i = 0;
  char rbuf[64], *readp = rbuf;
  char *crypt();
  unsigned int ind, r, g, b, maxi;
  int at_startup = (*password == 0);

  /* find w/o claiming the colors we want on the startup screen */
  r = 255; g = 180; b = 60; maxi = 255; /* sun yellow */
  findcolor( screen, &ind, &r, &g, &b, &maxi);
  color_map[LOGO_COLOR] = ind;

  if( at_startup) {
    r = 0; g = 16; b = 64; maxi = 255; /* deep blue */
  } else {
    r = 0; g =  0; b =  0; maxi = 255; /* black for screen saving */
  }
  findcolor( screen, &ind, &r, &g, &b, &maxi);
  color_map[LOGO_COLOR_BG] = ind;

  r = 255; g = 108; b = 0; maxi = 255; /* orange */
  findcolor( screen, &ind, &r, &g, &b, &maxi);
  color_map[CR_COLOR] = ind;

  r = 0; g = 255; b = 0; maxi = 255;  /* unused green? */
  findcolor( screen, &ind, &r, &g, &b, &maxi);
  color_map[CR_COLOR_BG] = ind;

  /* clear display */
        
  bit_blit(where,0,0,BIT_WIDE(where),BIT_HIGH(where),
	   BUILDOP(BIT_CLR,color_map[LOGO_COLOR],color_map[LOGO_COLOR_BG]),
	   (BITMAP*)0,0,0);

  if( at_startup) {
    /* get the cr notice hole */

    clip1.x1 = (BIT_WIDE(where)-BIT_WIDE(notice))/2 - SSIZE;
    clip1.y1 = (3*BIT_HIGH(where)-2*BIT_HIGH(notice))/4 - SSIZE;
    clip1.x2 = clip1.x1 + SSIZE + BIT_WIDE(notice);
    clip1.y2 = clip1.y1 + SSIZE + BIT_HIGH(notice);

    bit_blit(where,clip1.x1+SSIZE,clip1.y1+SSIZE,
	     BIT_WIDE(notice),BIT_HIGH(notice),
	     BUILDOP(BIT_SRC,color_map[CR_COLOR],color_map[LOGO_COLOR_BG]),
	     notice,0,0);

    /* get the globe hole */

    clip2.x1 = (BIT_WIDE(where)-BIT_WIDE(logo[0]))/2-SSIZE;
    clip2.y1 = (BIT_HIGH(where)-2*BIT_HIGH(logo[0]))/4-SSIZE;
    clip2.x2 = clip2.x1 + SSIZE + BIT_WIDE(logo[0]);
    clip2.y2 = clip2.y1 + SSIZE + BIT_HIGH(logo[0]);

#ifdef MESSAGE
    /* get the message hole */

    clip3.x1 = 10 - SSIZE;
    clip3.y1 = BIT_HIGH(where) - font->head.high - 10 - SSIZE;
    clip3.x2 = 10 + 2*SSIZE + strlen(MESSAGE) * font->head.wide;
    clip3.y2 = BIT_HIGH(where) - 10 + 2*SSIZE;
    put_str(where,10,clip3.y2-SSIZE,font,BIT_SRC,MESSAGE);
#else
    clip3 = clip2;
#endif
  } else {
    /* no messages during screen lock, just stars */
    clip1.x1 = clip1.x2 = clip1.y1 = clip1.y2 = 0;
    clip2 = clip1;
    clip3 = clip1;
  }

  /* kick off stars */

  fly(where);
  FD_ZERO(&mask);
  /* keep drawing stars until enough read from kb to stop */
  for(;;)
  {
    int sel;
    struct timeval tmpdelay = delay;

    FD_SET( STDIN_FILENO, &mask);
    sel = select( FD_SETSIZE, &mask, (fd_set*)0, (fd_set*)0, &tmpdelay);
    if( sel > 0) {
      read( STDIN_FILENO, readp, 1);
      if( at_startup)
	break;		/* any char at all */
      if( *readp == '\r' || *readp == '\n') {
	*readp = 0;
	if( strcmp( password, crypt( rbuf, password)) == 0)
	  break;	/* password matched, done */
	else {
	  readp = rbuf;
	  flip();
	}
      } else if( *readp == '\b' || *readp == 0177) {
	/* erase a char */
	if( readp > rbuf)
	  readp -= 1;
      } else {
	/* normal char read */
	if( readp < rbuf + sizeof( rbuf) - 1)
	  readp += 1;
      }
    }

    dofly(where);
    if( at_startup)
      bit_blit(where,clip2.x1+SSIZE,clip2.y1+SSIZE,
	       BIT_WIDE(logo[0]),BIT_HIGH(logo[0]),
	       BUILDOP(BIT_SRC,color_map[LOGO_COLOR],color_map[LOGO_COLOR_BG]),
	       logo[i++%8],0,0);
  }
  memset( rbuf, 0, sizeof( rbuf));
}
/*}}}  */
