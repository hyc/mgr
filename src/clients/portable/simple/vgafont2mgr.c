/*
 * Read a screen font named on the command line and write out
 * a corresponding MGR font to stdout.
 * Each glyph must be 8 pixels wide and the font contain 256 glyphs.
 * The input font is stored in a long column, the MGR font in a long row.
 * Char 255 is almost discarded because MGR fonts store their size in 8 bits.
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>

/* Constant spaced font format */

#define FONT_A			'\030'	/* fixed width fonts 32 bit alignment*/
#define FONT_X			'\026'	/* fixed width fonts 16 bit alignment (obsolete) */
#define FONT_S			'\027'	/* proportional fonts (not used yet) */

struct font_header {
   unsigned char type;		/* font type */
   unsigned char wide;		/* character width */
   unsigned char high;		/* char height */
   unsigned char baseline;	/* pixels from bottom */
   unsigned char count;		/* number of chars in font */
   char          start;		/* starting char in font */
   };


/*
 * return the number of blank rows at the bottom of char ch in the font in buf.
 * height is the number of rows in each glyph.
 */
unsigned int
findbaseline( int height, int ch, unsigned char *buf) {
int row;
unsigned char *glyph;

	glyph = buf + height * ch;
	for( row = height - 1; row >= 0 && glyph[ row] == 0; row -= 1)
		;
	return( height - 1 - row);
}


main( argc, argv)
int argc;
char *argv[];
{
struct stat stb;
int fd, size, count, row, uc;
struct font_header fh;
unsigned char *buf;
char *filename;

	if( argc != 2) {
		fprintf( stderr, "usage: vgafont2mgr vgafontfilename\n");
		exit( 1);
	}
	filename = argv[1];
	if( stat( filename, &stb) < 0) {
		fprintf( stderr, "vgafont2mgr: could not find file %s\n", filename);
		exit( 1);
	}
	size = (int) stb.st_size;
	if( (fd = open( filename, 0)) < 0) {
		fprintf( stderr, "vgafont2mgr: could not read file %s\n", filename);
		exit( 1);
	}
	buf = (unsigned char *) malloc( size);
	if( read( fd, buf, size) < size) {
		fprintf( stderr, "vgafont2mgr: read failed after open\n");
		exit(1);
	}
	close( fd);
	fh.type = FONT_A;
	fh.wide = 8;
	fh.start = 0;
	fh.count = 255;
	count = 256;   /* extra char will be present as row padding */
	fh.high = size / count;
	/* the arabic numerals seem to be present in every font */
	fh.baseline = findbaseline( fh.high, '6', buf);
	write( 1, &fh, sizeof( fh));
	for( row = 0;  row < fh.high;  row += 1) {
		for( uc = fh.start;  uc < fh.start+count;  uc += 1)
			write( 1, &buf[ row + uc*fh.high], 1);
	}
	exit( 0);
}
