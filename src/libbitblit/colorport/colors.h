/* rescale computes a color index from the range [0..newmax]
   corresponding to a color index supplied in the range [0..oldmax].
   0 maps to 0 and oldmax maps to newmax.
   The expanding rescaling produces nearly equispaced outputs,
   and contracting rescaling maps nearly equal intervals to a single output.
   A rescaling followed by the reverse rescaling is a projection mapping.
 */
extern unsigned int
rescale( unsigned int col, unsigned int oldmax, unsigned int newmax);

/*
 * gamcorr_setp and gamcorr_getp perform gamma correction on color intensities
 * scaled to range from 0 to GAMCORR_SCALE inclusive.
 * gammcorr_setp is appropriate for setpalette operations and
 * gammcorr_getp for getpallette operations, with gamma ~= 1.6174 .
 *
 * rescale_setp and rescale_getp perform rescaling operations to/from
 * arbitrary scales, with gamma correction performed in between.
 */
#define GAMCORR_SCALE 8191

extern unsigned int
gamcorr_setp( unsigned int col);

extern unsigned int
gamcorr_getp( unsigned int col);

extern unsigned int
rescale_setp( unsigned int col,
	      unsigned int oldmax,
	      unsigned int palmax);

extern unsigned int
rescale_getp( unsigned int col,
	      unsigned int palmax,
	      unsigned int newmax);
