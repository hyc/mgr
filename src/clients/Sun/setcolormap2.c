/* init a color map */
/* this code implements the man page of set_colormap as written
 * but the placement of white/gray/black differs from that in set_colormap
 * Author:  Vincent Broman, broman@nosc.mil, nov 1993.
 * Permission granted to copy for any purpose.
 */

#include <stdio.h>
#include <string.h>
#include <mgr/bitblit.h>
#include <mgr/mgr.h>

#ifndef SCREEN_DEV
#define SCREEN_DEV "/dev/fb"    /* VGA ends with a default */
#endif

#define WHITEI 0
#define BLACKI 1
#define WTCO 2	/* greater than WTWB if DK White lighter than LT Black */
#define WTWB 2
#define WTTOT (WTCO+WTWB)
#define PERIOD 8
#define DARKOFF 8
#define LITEOFF 16
#define NUMCOLOR 24
#define ANTIZEROINT 256
#define ON 255
#define OFF (ANTIZEROINT-ON)

/*
 * first 24 colormap entries.
 * the first eight are
 *      white,  black,  red,    green,  blue,   yellow, cyan,   magenta.
 * the next eight are darker versions of the same, the next eight lighter.
 */

unsigned char red[NUMCOLOR] = {
	ON, 	OFF,	ON, 	OFF, 	OFF, 	ON, 	OFF, 	ON
	};
unsigned char green[NUMCOLOR] = {
	ON, 	OFF,	OFF, 	ON,	OFF, 	ON,	ON,	OFF
	};
unsigned char blue[NUMCOLOR] = {
	ON, 	OFF,	OFF, 	OFF, 	ON,	OFF, 	ON,	ON
	};

/* last 24 color map entries */

unsigned char rred[NUMCOLOR];
unsigned char rgreen[NUMCOLOR];
unsigned char rblue[NUMCOLOR];


void do_col( i, a, ra)
int i;
unsigned char a[], ra[];
{
    a[ i + DARKOFF] = (WTCO*a[ i] + WTWB*a[ BLACKI]) / WTTOT;
    a[ i + LITEOFF] = (WTCO*a[ i] + WTWB*a[ WHITEI]) / WTTOT;
    ra[ NUMCOLOR - 1 - i          ] = ANTIZEROINT - a[ i];
    ra[ NUMCOLOR - 1 - i - DARKOFF] = ANTIZEROINT - a[ i + DARKOFF];
    ra[ NUMCOLOR - 1 - i - LITEOFF] = ANTIZEROINT - a[ i + LITEOFF];
}


int
main(argc,argv)
int argc; 
char *argv[];
{
    int i;
    char *getenv(char *);

    for( i = 0; i < PERIOD; i += 1) {
	do_col( i, red, rred);
	do_col( i, green, rgreen);
	do_col( i, blue, rblue);
    }

#ifdef DEBUG
    for( i = 0; i < NUMCOLOR; i += 1)
	printf("%2d=> %3d %3d %3d\n",i,
		    red[i],blue[i],green[i]);
    for( i = 0; i < NUMCOLOR; i += 1)
	printf("%2d=> %3d %3d %3d\n",256-sizeof(rred)+i,
		    rred[i],rblue[i],rgreen[i]);
#endif

    if( is_mgr_term()) {
	/* indirectly through the MGR server */
	m_setup( 0);
	for( i = 0; i < NUMCOLOR; i += 1)
	    m_setcolorrgb( i, red[ i], green[ i], blue[ i], 255);
	for( i = 0; i < NUMCOLOR; i += 1)
	    m_setcolorrgb( 256-NUMCOLOR+i, rred[ i], rgreen[ i], rblue[ i], 255);
    } else {
	/* directly in the libbitblit library */
        char *dev = getenv("SCREEN_DEV");
        BITMAP *bm = bit_open( dev? dev: SCREEN_DEV);
        if( bm == NULL)
            return( 1);         /* fail */
        bit_grafscreen();

	for( i = 0; i < NUMCOLOR; i += 1)
	    setpalette( bm, i, red[ i], green[ i], blue[ i], 255);
	for( i = 0; i < NUMCOLOR; i += 1)
	    setpalette( bm, 256-NUMCOLOR+i, rred[ i], rgreen[ i], rblue[ i], 255);

	bit_destroy( bm);
    }
    return( 0);
}
