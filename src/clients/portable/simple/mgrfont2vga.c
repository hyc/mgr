/*
 * Read an mgr font named on the command line or stdin and write out
 * a corresponding VGA font to stdout.
 * Each glyph must be 8 pixels wide.
 * The VGA font is stored in a long column, the MGR font in a long row.
 */

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <malloc.h>
#include <mgr/font.h>

int
main( argc, argv)
int argc;
char *argv[];
{
int fd, size, pcount, count, row, uc;
struct font_header fh;
unsigned char *buf, *zeroc;

	if( argc > 2) {
		fprintf( stderr, "usage: mgrfont2vga [mgrfontfilename]\n");
		exit( 1);
	}
	if( argc < 2)
		fd = 0;
	else {
		char *filename = argv[1];
		if( (fd = open( filename, O_RDONLY)) < 0) {
			fprintf( stderr, "mgrfont2vga: could not read file %s\n",
				filename);
			exit( 1);
		}
	}
	if( read( fd, &fh, sizeof(fh)) < sizeof(fh)) {
		fprintf( stderr, "mgrfont2vga: no font header read\n");
		exit(1);
	}
	if( fh.type != FONT_A || fh.wide != 8 || fh.start+fh.count > 256) {
		fprintf( stderr, "mgrfont2vga: not an 8 pixel wide mgr font\n");
		exit(1);
	}
	count = fh.count == 255? 256: fh.count;
	pcount = (count+3)&~3;
	buf = (unsigned char *) calloc( pcount, fh.high);
	size = fh.high * pcount;
	if( read( fd, buf, size) < size) {
		fprintf( stderr, "mgrfont2vga: font file short/corrupt\n");
		exit(1);
	}
	close( fd);
	zeroc = (unsigned char *) calloc( 1, fh.high);
	for( uc = 0; uc < fh.start; uc += 1)
		write( 1, zeroc, fh.high);
	for( ;  uc < fh.start+count;  uc += 1)
		for( row = 0;  row < fh.high;  row += 1)
			write( 1, &buf[ row*pcount + uc], 1);
	for( ;  uc < 256;  uc += 1)
		write( 1, zeroc, fh.high);
	return( 0);
}
