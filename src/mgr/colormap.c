/* routines for allocating/freeing colors for use in windows */

#include <malloc.h>
#include <mgr/bitblit.h>
#include "defs.h"
#include "colormap.h"

#define SERVERID 0	/* setids are positive for windows */
#define FREE (-1)
#define UNINIT (-2)
#define LASTCOLOR 255

static int *alloctab = NULL;
static int wob = 1;

static int eqcolor( unsigned int *c0, unsigned int *c1) {
    return c0[0]*c1[3] == c1[0]*c0[3]
	&& c0[1]*c1[3] == c1[1]*c0[3]
	&& c0[2]*c1[3] == c1[2]*c0[3];
}

static long int lightdiff( unsigned int *c0, unsigned int *c1) {
    return (long int)c0[0]*c1[3] - (long int)c1[0]*c0[3]
	 + (long int)c0[1]*c1[3] - (long int)c1[1]*c0[3]
	 + (long int)c0[2]*c1[3] - (long int)c1[2]*c0[3];
}

/* rescale computes a color index from the range [0..newmax]
   corresponding to a color index supplied in the range [0..oldmax].
   0 maps to 0 and oldmax maps to newmax.
   The expanding rescaling produces nearly equispaced outputs,
   and contracting rescaling maps nearly equal intervals to a single output.
   A rescaling followed by the reverse rescaling is a projection mapping.
 */
static
unsigned int rescale( unsigned int col, unsigned int oldmax,
                                        unsigned int newmax) {
    unsigned int newcol;

    if( newmax >= oldmax)
        newcol = oldmax? (col*2*newmax + oldmax) / (2*oldmax): newmax;
    else
        newcol = (col*2*(newmax + 1) + newmax) / (2*(oldmax + 1));
    return newcol;
}
/* rescale also in libbitblit, could move it out */


void init_colors( BITMAP *bp) {
    unsigned int fgco = fg_color_idx();
    unsigned int c0[4], cfg[4];

    getpalette( bp, 0, c0, c0+1, c0+2, c0+3); /* background color */
    getpalette( bp, fgco, cfg, cfg+1, cfg+2, cfg+3);
    if( c0[3] <= 1 || cfg[3] <= 1)  return;
    if( eqcolor( c0, cfg)) {
	setpalette( bp, fgco, c0[3]-c0[0], c0[3]-c0[1], c0[3]-c0[2], c0[3]);
	getpalette( bp, fgco, cfg, cfg+1, cfg+2, cfg+3);
	/* give up if no palette (or if fg = bg = 50% gray) */
	if( eqcolor( c0, cfg))  return;
    }
    wob = lightdiff( c0, cfg) <= 0;

    if( alloctab == NULL) {
	unsigned int i;

	alloctab = malloc( (LASTCOLOR+1)*sizeof( *alloctab));
	if( alloctab == NULL)  return;
	for( i = 0; i <= LASTCOLOR; i += 1)
	    alloctab[i] = FREE;
	/* claim for server the colors it needs */
	alloctab[0] = SERVERID;
	alloctab[fgco] = SERVERID;
    }
}

/* set entire colormap to 2bitB x 3bitR x 3bitG true color */
/* choose from it colors for the server. */
void fill_colormap( BITMAP *bp) {
    int i;

    if( alloctab == NULL)  return;

    for( i = 0; i <= LASTCOLOR; i += 1) {
	unsigned int tc = wob? i: LASTCOLOR - i;
	unsigned int red, green, blue;

	blue  = rescale( ((tc >> 6) & 3), 3, LASTCOLOR);
	red   = rescale( ((tc >> 3) & 7), 7, LASTCOLOR);
	green = rescale( ((tc >> 0) & 7), 7, LASTCOLOR);
	setpalette( bp, i, red, green, blue, LASTCOLOR);
    }

    color_map[ROOT_COLOR_FG] = wob? 0205: LASTCOLOR-0205; /* dk cyan */
    alloctab[color_map[ROOT_COLOR_FG]] = SERVERID;

    color_map[ROOT_COLOR_BG] = wob? 0355: LASTCOLOR-0355; /* lt blue */
    alloctab[color_map[ROOT_COLOR_BG]] = SERVERID;

    color_map[MENU_COLOR_FG] = wob? 0347: LASTCOLOR-0347; /* lt grn */
    alloctab[color_map[MENU_COLOR_FG]] = SERVERID;
    alloctab[LASTCOLOR - color_map[MENU_COLOR_FG]] = SERVERID;

    color_map[MENU_COLOR_BG] = wob? 0303: LASTCOLOR-0303; /* deep blue */
    alloctab[color_map[MENU_COLOR_BG]] = SERVERID;
    alloctab[LASTCOLOR - color_map[MENU_COLOR_BG]] = SERVERID;
}

void free_colors( WINDOW *win, unsigned int lo, unsigned int hi) {
    unsigned int i, ihi;
    int id;

    if( alloctab == NULL)  return;
    
    id = win? win->setid: SERVERID;
    ihi = (hi > LASTCOLOR)? LASTCOLOR: hi;
    for( i = lo; i <= ihi; i += 1)
	if( alloctab[ i] == id)
	    alloctab[ i] = FREE;
}


int allocate_color( WINDOW *win) {
    int id, k;

    if( alloctab == NULL)
	return -1;
    id = win? win->setid: SERVERID;
    if( id == SERVERID) {
	for( k = LASTCOLOR; k > 0; k -= 1)
	    if( alloctab[ k] == FREE)
		break;
    } else {
	for( k = 0; k < LASTCOLOR; k += 1)
	    if( alloctab[ k] == FREE)
		break;
    }
    if( alloctab[k] == FREE) {
	alloctab[k] = id;
	return k;
    } else
	return -1;
}

static unsigned int medgray[4] = { 50, 50, 50, 100};

void findcolor( BITMAP *bp, unsigned int *co, unsigned int *r, unsigned int *g,
				  unsigned int *b, unsigned int *maxi) {

    /* simple stub */
    if( alloctab) {
	/* correct only as long as the true color colormap stays in effect */
	*co = (rescale(*r,*maxi,7)<<3)
	    | (rescale(*g,*maxi,7)<<0)
	    | (rescale(*b,*maxi,3)<<6);
	if(! wob)
	    *co = LASTCOLOR - *co;
    }
    else {
	unsigned int usercol[4];
	usercol[0] = *r;
	usercol[1] = *g;
	usercol[2] = *b;
	usercol[3] = *maxi;
	*co = (lightdiff( usercol, medgray) < 0)==wob? 0: fg_color_idx();
    }
    getpalette(bp,*co,r,g,b,maxi);
}

