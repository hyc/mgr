#include "colors.h"

/* rescale computes a color index from the range [0..newmax]
   corresponding to a color index supplied in the range [0..oldmax].
   0 maps to 0 and oldmax maps to newmax.
   The expanding rescaling produces nearly equispaced outputs,
   and contracting rescaling maps nearly equal intervals to a single output.
   A rescaling followed by the reverse rescaling is a projection mapping.
 */
unsigned int rescale( unsigned int col, unsigned int oldmax,
					unsigned int newmax) {
    unsigned int newcol;

    if( newmax >= oldmax)
	newcol = oldmax? (col*2*newmax + oldmax) / (2*oldmax): newmax;
    else
	newcol = (col*2*(newmax + 1) + newmax) / (2*(oldmax + 1));
    return newcol;
}

/*
 * gamcorr_setp and gamcorr_getp perform gamma correction on color intensities
 * scaled to range from 0 to GAMCORR_SCALE inclusive.
 * gammcorr_setp is appropriate for setpalette operations and
 * gammcorr_getp for getpallette operations, with gamma ~= 1.6174 .
 *
 * The L2 approximant on [0,1] to x**gamma which is quadratic and constrained
 * to go thru (0,0) and (1,1) is x-a*x*(1-x),
 * where a = 5/2 + 30/(gamma+3) - 30/(gamma+2).
 * If gamma = -5/2+1/2*sqrt(25+264/19) ~= 0.61828, a = -2/3 exactly.
 * For the reciprocal 1/gamma, a ~= 0.70390 ~= 19/27.
 *
 * rescale_setp and rescale_getp perform rescaling operations to/from
 * arbitrary scales, with gamma correction performed in between.
 */
#define SANUM 2
#define SADEN 3
#define GANUM 19
#define GADEN 27

unsigned int
gamcorr_setp( unsigned int col) {
    return (col*GAMCORR_SCALE*SADEN + col*(GAMCORR_SCALE-col)*SANUM)
	   /(GAMCORR_SCALE*SADEN);
}

unsigned int
gamcorr_getp( unsigned int col) {
    return (col*GAMCORR_SCALE*GADEN - col*(GAMCORR_SCALE-col)*GANUM)
	   /(GAMCORR_SCALE*GADEN);
}

unsigned int
rescale_setp( unsigned int col,
	      unsigned int oldmax,
	      unsigned int palmax) {
    return rescale( gamcorr_setp( rescale( col, oldmax, GAMCORR_SCALE)),
		    GAMCORR_SCALE, palmax);
}

unsigned int
rescale_getp( unsigned int col,
	      unsigned int palmax,
	      unsigned int newmax) {
    return rescale( gamcorr_getp( rescale( col, palmax, GAMCORR_SCALE)),
		    GAMCORR_SCALE, newmax);
}
