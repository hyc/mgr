/*
 * Convert UNIXpc font(4) files to MGR font(5) files
 *
 * Usage: ft2fnt ft_file > fnt_file
 *
 * No distribution/use restrictions, but give credit/blame where due.
 * Speaking of which, MGR is copyright 1988 Bellcore.
 *
 * John R. MacMillan
 */

#ifndef lint
char	*SCCSid = "@(#)ft2fnt.c	1.1	90/03/20";
#endif

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/font.h>
#include <sys/stat.h>

/*
 * Could grab this from the "font.h" in the MGR distribution, but then
 * I'd either have to distribute it or you'd have to have it, plus
 * it's got useless things which would require other header files, and
 * so on...
 */

struct font_header {
	unsigned char	type;
	unsigned char	wide;
	unsigned char	high;
	unsigned char	baseline;
	unsigned char	count;
	char		start;
};

#define	FONT_A	'\026'			/* ^V */

/*
 * Thoe following returns the width in bytes of (bits) pixels, padded
 * to the nearest word, for dealing with the MGR bitmap of the font.
 */

#define	WIDTH(bits)	((((bits)+15L)&~15L)>>3)

extern	int	errno;
#define	ERR(n)	strerror( n)

char		*prog;			/* Program name */

static void	fontsize();		/* find the size for the font */
static void	convert();		/* convert a character */
static void	bitset();		/* turn on bitmap bits */

int
main(argc,argv)
int argc;
char **argv;
{
	struct font_header *new;	/* new fnt file */
	struct fntdef	*old;		/* old ft file */
	char		*name;		/* ft file name */
	struct stat	buf;		/* to get size of ft file */
	int		fd;		/* ft file */
	int		newsize;	/* size of new fnt file */
	int		vs, hs;		/* vertical, horizontal size */
	int		va, ha;		/* adjustment */
	int		c;
	extern char	*malloc(), *calloc();
	extern void	free(), exit();
	extern int	fprintf(), stat(), open(), read(), write(), close();

	prog = strrchr(argv[0], '/');
	if (prog == NULL)
		prog = argv[0];
	else
		prog++;

	if (argc != 2) {
		(void) fprintf(stderr, "Usage: %s ft_file > fnt_file\n", prog);
		exit(1);
	}

	name = argv[1];

	/*
	 * Get the size, and malloc enough memory to hold the entire
	 * ft file
	 */

	if (stat(name,&buf) < 0) {
		(void) fprintf(stderr, "%s: %s: %s\n", prog, name,
				ERR(errno));
		exit(1);
	}

	old = (struct fntdef *) malloc((unsigned)buf.st_size);
	if (old == NULL) {
		(void) fprintf(stderr, "%s: unable to get memory for %s\n",
				prog, name);
		exit(1);
	}

	/*
	 * Open the file and suck it in
	 */

	if ((fd = open(name, O_RDONLY)) < 0) {
		(void) fprintf(stderr, "%s: unable to open %s: %s\n",
				prog, name, ERR(errno));
		exit(1);
	}

	if (read(fd, (char *)old, (unsigned)buf.st_size) < buf.st_size) {
		(void) fprintf(stderr, "%s: error reading %s: %s\n",
				prog, name, ERR(errno));
		exit(1);
	}
	(void) close(fd);

	if (old->ff_magic != FMAGIC) {
		(void) fprintf(stderr, "%s: %s is not a font(4) file\n",
				prog, name);
		exit(1);
	}

	/*
	 * Determine the character size for the new font, and the
	 * adjustment of the old raster in the new one
	 */

	fontsize(old, &vs, &hs, &va, &ha);

	/*
	 * Set up for the new font
	 *
	 * The new size is the size of the header plus enough bytes
	 * to hold the bitmap.  The bitmap is hs rows of vs * count
	 * pixels; rows are padded to a word boundary.
	 *
	 * We calloc(3) the space so the bitmap will be zeroed
	 */

	newsize = sizeof(struct font_header) + vs * WIDTH(hs * FNTSIZE);

	new = (struct font_header *) calloc((unsigned) newsize, (unsigned) 1);
	if (new == NULL) {
		(void) fprintf(stderr, "%s: unable to get memory for new font\n",
				prog);
		exit(1);
	}

	/*
	 * Fill in the header info
	 */

	new->type = FONT_A;
	new->wide = hs;
	new->high = vs;
	new->baseline = old->ff_vs - old->ff_baseline - va;
	new->count = FNTSIZE;
	new->start = ' ';

	/*
	 * Copy the old font into the new area
	 */

	for (c = 0; c < FNTSIZE; c++)
		convert(c, old, new, va, ha);

	/*
	 * Write out the fnt file
	 */

	if (write(1, (char *)new, (unsigned)newsize) < newsize) {
		(void) fprintf(stderr, "%s: error writing output: %s\n",
				prog, ERR(errno));
		exit(1);
	}

	/*
	 * Clean up, just to be nice
	 */

	free((char *)old);
	free((char *)new);

	return(0);			/* To keep lint happy */
}

/*
 * Scan the font to determine the the biggest character, and how to
 * place the old raster in the new one.
 */

static void
fontsize(font, vsp, hsp, vap, hap)
struct fntdef	*font;			/* Font being sized */
int		*vsp, *hsp;		/* RETURN: vertical/horiz size */
int		*vap, *hap;		/* RETURN: vertical/horiz adjust */
{
	int	minrow = 0;		/* Minimum row of any char */
	int	maxrow = font->ff_vs-1;	/* Maximum row of any char */
	int	mincol = 0;		/* Minimum col of any char */
	int	maxcol = font->ff_hs-1;	/* Maximum col of any char */
	int	c;			/* Current character */
	int	n;			/* Scratch min/max values */

	for (c = 0; c < FNTSIZE; c++) {
		n = font->ff_baseline + font->ff_fc[c].fc_va;
		if (n < minrow)
			minrow = n;
		n += font->ff_fc[c].fc_vs - 1;
		if (n > maxrow)
			maxrow = n;
		n = font->ff_fc[c].fc_ha;
		if (n < mincol)
			mincol = n;
		n += font->ff_fc[c].fc_hs - 1;
		if (n > maxcol)
			maxcol = n;
	}

	if (minrow < 0)
		*vap = -minrow;
	else
		*vap = 0;
	if (mincol < 0)
		*hap = -mincol;
	else
		*hap = 0;
	*vsp = maxrow - minrow + 1;
	*hsp = maxcol - mincol + 1;
}

/*
 * Convert a character in the font(4) area into the MGR font(5) area
 */

static void
convert(c, from, to, va, ha)
int		c;			/* Character offset from ' ' */
struct fntdef	*from;			/* UNIXpc font(4) */
struct font_header *to;			/* MGR font(5) */
int		va, ha;			/* adjustment */
{
	int		rowsize;	/* words in a row */
	int		row;		/* current row in miniraster */
	int		col;		/* current col in miniraster */
	unsigned short	*mr;		/* current word pointer */
	struct fcdef	*fc;		/* character definition */

	/*
	 * Set up to read the miniraster.  It starts fc->fc_mr bytes
	 * after its own fc_mr field.  Ick.
	 */

	fc = &from->ff_fc[c];
	rowsize = (fc->fc_hs + 15) >> 4;
	mr = (unsigned short *)((char *) &from->ff_fc[c].fc_mr + fc->fc_mr);

	/*
	 * Calculate the adjustments to put the miniraster into the
	 * character of the bitmap
	 */

	ha = ha + fc->fc_ha;
	va = va + from->ff_baseline + fc->fc_va;

	/*
	 * Go through the miniraster row by row, column by column.  If
	 * a bit is set in the miniraster, set it in the bitmap.
	 */

	for (row = 0; row < fc->fc_vs; row++) {
		for (col = 0 ; col < fc->fc_hs; col++) {
			if (mr[col/16] & (1<<col%16))
				bitset(to, c, row + va, col + ha);
		}
		mr += rowsize;
	}
}

/*
 * Set a bit in the bitmap in the font(5) area pointed to by fnt.  The
 * bit set is in character c, (row, col)
 */

static void
bitset(fnt, c, row, col)
struct font_header *fnt;
int		c;
int		row;
int		col;
{
	char		*bm;		/* Correct row of bitmap */
	unsigned	bit;		/* Bit number in bitmap */
	extern int	fprintf();

	/*
	 * The correct row is the start of bitmap (start of font plus
	 * size of header) plus (width of row * row number).
	 */

	bm = (char *)fnt + sizeof(struct font_header) +
			WIDTH(fnt->wide * fnt->count) * row;

	bit = c * fnt->wide + col;

	bm[bit/8] |= 1 << (7 - bit%8);
}
