/* 
 * colormap: sets and inquires RBG values in the colormap or palette.
 * usages:
 *	colormap single_col
 *	colormap low_col hi_col
 *	colormap col r g b
 *	colormap col r g b max_intensity
 *	colormap - < mapfile
 * The first two calls print on stdout, one color per line,
 * the color number,red intensity,green intensity,blue intensity,max intensity
 * for a single color index or a range of color indices.
 * The 3d and 4th calls set the RGB values for the given color index,
 * suitably scaled relative to the max intensity value, which defaults to 255.
 * The last call reads a text file from stdin, each line of which contains four
 * or five decimal values, and sets the colormap with those values as above.
 *
 * Author:  Vincent Broman, broman@nosc.mil, nov 1993.
 * Permission granted to copy for any purpose.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mgr/mgr.h>
#include <mgr/bitblit.h>

#define MAXCOLOR 255  /* size of colormap table minus one */
#define MAXINTENS 255 /* max value of rgb in the color table */
#ifndef SCREEN_DEV
#define SCREEN_DEV "/dev/fb"	/* VGA ends with a default */
#endif

typedef struct {
    unsigned int col, r, g, b, max;
} CMAP, *CMAPP;

int get_many( unsigned int lo_col, unsigned int hi_col) {
    unsigned int i, r, g, b, M;

    if( is_mgr_term()) {
	/* indirectly through the MGR server */
	m_setup( 0);
	for( i = lo_col; i <= hi_col; i += 1) {
	    m_getpalette( i, &r, &g, &b, &M);
	    printf( "%3u %3u %3u %3u %3u\n", i, r, g, b, M);
	}
    } else {
	/* directly in the libbitblit library */
	char *dev = getenv("SCREEN_DEV");
	BITMAP *bm = bit_open( dev? dev: SCREEN_DEV);
	if( bm == NULL)
	    return( 1);		/* fail */
	bit_grafscreen();
	for( i = lo_col; i <= hi_col; i += 1) {
	    getpalette( bm, i, &r, &g, &b, &M);
	    printf( "%3u %3u %3u %3u %3u\n", i, r, g, b, M);
	}
	bit_destroy( bm);
    }
    return( 0);
}

int set_many( CMAPP cmapp, unsigned int ct) {
    CMAPP cp;

    if( is_mgr_term()) {
	/* indirectly through the MGR server */
	m_setup( 0);
	for( cp = cmapp; cp < cmapp + ct; cp += 1)
	    if( cp->max)
		m_setcolorrgb( cp->col, cp->r, cp->g, cp->b, cp->max);
    } else {
	/* directly thru the libbitblit library */
	char *dev = getenv("SCREEN_DEV");
	BITMAP *bm = bit_open( dev? dev: SCREEN_DEV);

	if( bm == NULL)
	    return( 1);		/* fail */
	bit_grafscreen();
	for( cp = cmapp; cp < cmapp + ct; cp += 1)
	    if( cp->max)
		setpalette( bm, cp->col, cp->r, cp->g, cp->b, cp->max);
	bit_destroy( bm);
    }
    return( 0);
}

int
main(argc,argv)
int argc; 
char *argv[];
{
    unsigned int doset, doget, exitstatus, i, q, r, g, b, M=255;

    if(argc == 2 && argv[1][0] == '-' && argv[1][1] == 0) {
	CMAP cmap[MAXCOLOR+1];
	char cbuf[100];
	int wasread, someok;

	memset( cmap, 0, sizeof(cmap));
	someok = 0;
	while( fgets( cbuf, sizeof( cbuf), stdin) != NULL) {
	    wasread = sscanf( cbuf, "%u%u%u%u%u", &i, &r, &g, &b, &M);
	    if( wasread <= 4)
		M = MAXINTENS;
	    if( (wasread == 4 || wasread == 5) && i <= MAXCOLOR
		    && r <= M && g <= M && b <= M) {
		someok = 1;
		cmap[i].col = i;
		cmap[i].r = r;
		cmap[i].g = g;
		cmap[i].b = b;
		cmap[i].max = M;
	    }
	}
	return( someok? set_many( cmap, sizeof( cmap)/sizeof( *cmap)): 1);

    } else {
	doset = (argc == 5) || (argc == 6);
	doget = (argc == 2) || (argc == 3);
	if( doset || doget) {
	    if( sscanf( argv[1], "%u", &q) < 1)  doset=doget=0;
	    if( argc > 2) {
		if( sscanf( argv[2], "%u", &r) < 1)  doset=doget=0;
	    } else
		r = q;
	}
	if( doset) {
	    if( sscanf( argv[3], "%u", &g) < 1)  doset=0;
	    if( sscanf( argv[4], "%u", &b) < 1)  doset=0;
	    if( argc == 6)
		if( sscanf( argv[5], "%u", &M) < 1)  doset=0;
	}
	if( (doset || doget) && q > MAXCOLOR)  doset=doget=0;
	if( doget && (r > MAXCOLOR || r < q))  doget=0;
	if( doset && (M < 1 || r > M || g > M || b > M))  doget=0;
	/* no check for negative colors or intensities because unsigned */
    }
    if(!doget && !doset) {
	fprintf( stderr, "usage for getting/setting colors:\n%s%s%s%s",
			"colormap col\n",
			"colormap low_col hi_col\n",
			"colormap col r g b [max_inten_if_not_255]\n",
			"colormap - < cmapfile\n");
	return( 1);
    }
    /* now doset^doget and r,g,b,M set and valid as needed */

    if( doset) {
	CMAP cmap;
	cmap.col = q;
	cmap.r = r;
	cmap.g = g;
	cmap.b = b;
	cmap.max = M;
	exitstatus = set_many( &cmap, 1);
    } else
	exitstatus = get_many( q, r);
    return( exitstatus);
}
