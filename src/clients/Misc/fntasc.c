#include <stdio.h>
#include <string.h>
#include <fcntl.h>

struct font_header {
	unsigned char	type;
	unsigned char	wide;
	unsigned char	high;
	unsigned char	baseline;
	unsigned char	count;
	char		start;
};

#define FONT_A	'\026'

#define WIDTH(bits)	((((bits)+15L)&~15L)>>3)

extern 	int 	errno;
#define	ERR(n)	strerror( n)

char 		*prog;			/* program name */

main(argc, argv)
int argc;
char **argv;
{
	extern char 		*malloc();
	
	int 			fd;
	int			x, y;
	int			i, j, k;
	int			col;
	unsigned		bitmap_size;
	unsigned		ascmap_size;
	unsigned char		*bitmap;
	char			*ascmap;
	char			*name;
	struct font_header	header;

	prog=strrchr(argv[0], '/');
	if (prog == NULL)
		prog = argv[0];
	else
		prog++;
	
	if (argc != 2)
	{
		(void)fprintf(stderr, "Usage: %s bin_fnt > asc_fnt\n", prog);
		exit(1);
	}

	name = argv[1];

	if ((fd = open(name, O_RDONLY)) < 0)
	{
		(void)fprintf(stderr, "%s: unable to open %s: %s\n",
			      prog, name, ERR(errno));
		exit(1);
	}
	
	if (read(fd,(char *)&header,(unsigned)sizeof(header)) < sizeof(header))
	{
		(void)fprintf(stderr, "%s: error reading header from %s: %s\n",
			      prog, name, ERR(errno));
		exit(1);
	}

	if (header.type != FONT_A)
	{
		(void)fprintf(stderr, "%s: %s is not a MGR 16-bit font file\n",
			      prog, name);
		exit(1);
	}

	bitmap_size = (unsigned)(WIDTH(header.wide*header.count)*header.high);
	if ((bitmap = (unsigned char *)malloc(bitmap_size)) == NULL)
	{
		(void)fprintf(stderr,
			      "%s: could not allocate memory for MGR bitmap\n",
			      prog);
		exit(1);
	}
	
	(void)memset(bitmap, 0, bitmap_size);

	if (read(fd, (char *)bitmap, bitmap_size) < bitmap_size)
	{
		(void)fprintf(stderr, "%s: error reading data from %s: %s\n",
			      prog, name, ERR(errno));
		exit(1);
	}

	(void)close(fd);
	
	ascmap_size = (unsigned)(header.wide*header.count*header.high);
	if ((ascmap = malloc(ascmap_size)) == NULL)
	{
		(void)fprintf(stderr,
			      "%s: could not allocate memory for ASCII map\n",
			      prog);
		exit(1);
	}

	(void)memset(ascmap,  (int)' ', ascmap_size);

	(void)printf("#magic 0x%x\n", FONT_A);
	(void)printf("#MGR font from file %s\n", name);
	(void)printf("wide\t%d\n", (int)header.wide);
	(void)printf("high\t%d\n", (int)header.high);
	(void)printf("base\t%d\n", (int)header.baseline);
	(void)printf("count\t%d\n", (int)header.count);
	(void)printf("start\t%d\n", (int)header.start);

	y = header.high;
	x = header.wide * header.count;
	for (i=0; i < y; i++)
	{
		for (j=0; j < x; j++)
		{
			col = (i * ((x+15)&~15)) + j;
			if (bitmap[col/8] & (1 << (7-(col%8))))
			{
				*(ascmap+(i*x)+j) = '*';
			}
		}
	}

	x = header.wide * header.count;
	for (k=0; k < header.count; k++)
	{
		(void)printf("\n##### ASCII %03d ", k+header.start);
		if (k+header.start < 32)
			(void)printf("(^%c)\n", k+header.start+64);
		else if (k+header.start < 127)
			(void)printf("(%c)\n", k+header.start);
		else
			(void)putchar('\n');

		putchar('\n');

		for (i=0; i < header.wide+2; i++)
		{
			(void)putchar('-');
		}
		(void)putchar('\n');
		
		col=k*header.wide;
		for (i=0; i < header.high; i++)
		{
			putchar('|');
			for (j=0; j < header.wide; j++)
			{
				(void)putchar(*(ascmap+(i*x)+col+j));
			}
			(void)printf("|\n");
		}
		
		for (i=0; i < header.wide+2; i++)
		{
			(void)putchar('-');
		}
		(void)putchar('\n');
	}

	exit(0);
}
