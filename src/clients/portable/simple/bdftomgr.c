/*
 * Read a BDF format monospaced font from stdin and write a
 * font in Mgr format to a file.
 * Extra intercharacter spacing and interline spacing is added,
 * the default being one pixel in x and in y.
 * Author: Vincent Broman, broman@nosc.mil, dec 1993.
 */

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <mgr/font.h>

void
usage( void)
{
    fprintf( stderr,
	     "usage: bdftomgr < fontname.bdf [+ICxIL] mgrfilenamebase\n");
    exit( 1);
}


int
main( argc, argv)
int argc;
char *argv[];
{
    FILE *fout;
    char line[256], *lp, *outname, fulloutname[ 256], *fontname;
    int fbbx_done, fdx, fdy, fx0, fy0, fxlim, fylim;
    int cdx, cdy, cx0, cy0, cxlim, cylim, ch, max_ch, min_ch;
    struct font_header hdr;
    unsigned char *bitmap;
    long int linebits, linebytes;
    int li, interchar, interline;

    if( argc < 2 || argc > 3)
	usage();
    if( argc == 3) {
	char *inter = argv[ 1];      /* e.g.  "+1x2"   */

	if( inter[ 0] != '+' || inter[ 2] != 'x' || inter[ 4] != 0)
	    usage();
	interchar = inter[ 1] - '0';
	interline = inter[ 3] - '0';
	if( interchar < 0 || interchar > 9 || interline < 0 || interline > 9)
	    usage();
	outname = argv[ 2];
    } else {
	interchar = 1;
	interline = 1;
	outname = argv[ 1];
    }

    fbbx_done = 0;
    fontname = NULL;
    while( !fbbx_done && gets( line) != NULL) {
	for( lp = line; *lp; lp += 1)
	    if( islower( *lp))
		*lp = toupper( *lp);
	if( strncmp( line, "FONT ", 5) == 0) {
	    fontname = (char *)malloc( strlen( line) - 5 + 1);
	    strcpy( fontname, line + 5);
	}
	if( strncmp( line, "FONTBOUNDINGBOX", 15) == 0) {
	    if( sscanf( line + 15, "%d %d %d %d",
			&fdx, &fdy, &fx0, &fy0) == 4) {
		fxlim = fx0 + fdx;
		fylim = fy0 + fdy;
		fbbx_done = 1;
	    }
	}
    }
    if( !fbbx_done) {
	fprintf( stderr, "bdftomgr: no FONTBOUNDINGBOX found in input\n");
	exit( 1);
    }
    hdr.type = FONT_A;
    hdr.wide = fdx + interchar;
    hdr.high = fdy + interline;
    hdr.baseline = 1 - fy0;
    linebytes = hdr.wide * 256 / 8; /* 256 chars */
    linebits = linebytes * 8;
    bitmap = (unsigned char *) calloc( hdr.high, linebytes);
    max_ch = ' ';
    min_ch = ' ';
    ch = -1;
    while( gets( line) != NULL && strcmp( line, "ENDFONT") != 0) {
	for( lp = line; *lp; lp += 1)
	    if( islower( *lp))
		*lp = toupper( *lp);
	if( strncmp( line, "ENCODING", 8) == 0) {
	    if( sscanf( line + 8, "%d", &ch) == 1 && ch >= 0 && ch < 256) {
		if( ch > max_ch)
		    max_ch = ch;
		if( ch < min_ch)
		    min_ch = ch;
	    } else
		ch = -1;
	} else if( strncmp( line, "BBX", 3) == 0) {
	    if( ch >= 0 && sscanf( line + 3, "%d %d %d %d",
				   &cdx, &cdy, &cx0, &cy0) == 4) {
		cxlim = cx0 + cdx;
		cylim = cy0 + cdy;
	    } else
		ch = -1;
	} else if( strcmp( line, "ENDCHAR") == 0)
	    ch = -1;
	else if( ch >= 0 && strcmp( line, "BITMAP") == 0) {
	    long int xoff0 = ch * hdr.wide + cx0 - fx0;
	    long int yoff = (fylim - cylim) * linebits;

	    for( li = 0; li < cdy && gets( line) != NULL; li += 1) {
		long int xoff = xoff0;
		
		for( lp = line; *lp; lp += 1) {
		    unsigned int hx =
			isdigit( *lp)? *lp - '0': toupper( *lp) - 'A' + 10;
		    long int off = xoff + yoff;
		    long int byteoff = off >> 3;
		    int bitoff = off - (byteoff << 3);

		    if( hx > 15)
			fprintf( stderr,
				 "bad hex digit, ch=%d, li=%d\n", ch, li);
		    hx <<= 4;  /* in bits 7,6,5,4 */
		    bitmap[ byteoff] |= hx >> bitoff;
		    bitmap[ byteoff + 1] |= 255 & (hx << (8 - bitoff));
		    xoff += 4;
		}
		yoff += linebits;
	    }
	    if( gets( line) == NULL || strcmp( line, "ENDCHAR") != 0)
		fprintf( stderr,
			 "bdftomgr: bitmap ends improperly, ch=%d\n", ch);
	}
	/* else ignore the line */
    }
    if( min_ch == max_ch) {
	fprintf( stderr, "bdftomgr: input corrupt or empty\n");
	exit( 1);
    }
    sprintf( fulloutname, "%s-%1dx%1d", outname, hdr.wide, hdr.high);
    fout = fopen( fulloutname, "w");
    if( fout == NULL) {
	fprintf( stderr, "bdftomgr: could not write %s\n", outname);
	exit( 1);
    }
    if( fontname != NULL)
	printf( "%s   %s\n", fulloutname, fontname);

    while( ((min_ch * hdr.wide) & 7) != 0)
	min_ch -= 1;
    if( min_ch == 0 && max_ch == 255)
	max_ch = 254; /* drop a char if count 256 cant be stored in 8 bits */
    hdr.start = min_ch;
    hdr.count = max_ch + 1 - min_ch;
    fwrite( &hdr, sizeof( hdr), 1, fout);
    for( li = 0; li < hdr.high; li += 1)
	fwrite( bitmap + linebytes * li + ((hdr.start * hdr.wide) >> 3),
	        4, (hdr.count * hdr.wide + 31) >> 5, fout);
    exit( fclose( fout) == EOF);
}
