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

#define LINESIZ	256

char inbuf[LINESIZ];

char 		*prog;			/* program name */
char		*name;			/* input file name */
int		line_no=1;

void get_header();
void read_char();
void bit_set();

main(argc, argv)
int argc;
char **argv;
{
	extern char 		*malloc();

	FILE 			*fd;
	int			i;
	int			num;
	unsigned		bitmap_size;
	unsigned char		*bitmap;
	struct font_header	header;

	prog=strrchr(argv[0], '/');
	if (prog == NULL)
		prog = argv[0];
	else
		prog++;
	
	if (argc != 2)
	{
		(void)fprintf(stderr, "Usage: %s asc_fnt > bin_fnt\n", prog);
		exit(1);
	}

	name = argv[1];

	if ((fd = fopen(name, "r")) == NULL)
	{
		(void)fprintf(stderr, "%s: unable to open %s: %s\n",
			      prog, name, ERR(errno));
		exit(1);
	}
	
	if (fscanf(fd, "#magic 0x%x\n", &num) != 1)
	{
		(void)fprintf(stderr, "%s: error reading header from %s: %s\n",
			      prog, name, ERR(errno));
		exit(1);
	}
	line_no++;

	if (num != FONT_A)
	{
		(void)fprintf(stderr, "%s: %s is not a MGR ascii font file\n",
			      prog, name);
		exit(1);
	}

	get_header(fd, &header);

	bitmap_size = (unsigned)(WIDTH(header.wide*header.count)*header.high);
	if ((bitmap = (unsigned char *)malloc(bitmap_size)) == NULL)
	{
		(void)fprintf(stderr,
			      "%s: could not allocate memory for MGR bitmap\n",
			      prog);
		exit(1);
	}
	
	(void)memset(bitmap, 0, bitmap_size);

	for (i=0; i < header.count; i++)
	{
		read_char(fd, i, header, bitmap);
	}

	(void)fclose(fd);

	if (fwrite((char *)&header,
		   sizeof(char),
		   sizeof(struct font_header),
		   stdout) != sizeof(struct font_header))
	{
		(void)fprintf(stderr, "%s: error writing binary header: %s\n",
			      prog, ERR(errno));
		exit(1);
	}
		
	if (fwrite(bitmap, sizeof(char), bitmap_size, stdout) != bitmap_size)
	{
		(void)fprintf(stderr, "%s: error writing binary bitmap: %s\n",
			      prog, ERR(errno));
		exit(1);
	}

	exit(0);
}

void get_header(fd, header)
FILE *fd;
struct font_header *header;
{
	int	t;

	header->type = FONT_A;
	if (fgets(inbuf, LINESIZ, fd) == NULL)
	{
		(void)fprintf(stderr, "%s: read error at line %d of %s\n",
			      prog, line_no, name);
		exit(1);
	}
	line_no++;
	if (fscanf(fd, "wide\t%d\n", &t) != 1)
	{
		(void)fprintf(stderr, "%s: expected 'wide' at line %d in %s\n",
			      prog, line_no, name);
		exit(1);
	}
	line_no++;
	header->wide = (unsigned char)t;
	if (fscanf(fd, "high\t%d\n", &t) != 1)
	{
		(void)fprintf(stderr, "%s: expected 'high' at line %d in %s\n",
			      prog, line_no, name);
		exit(1);
	}
	header->high = (unsigned char)t;
	line_no++;
	if (fscanf(fd, "base\t%d\n", &t) != 1)
	{
		(void)fprintf(stderr, "%s: expected 'base' at line %d in %s\n",
			      prog, line_no, name);
		exit(1);
	}
	header->baseline = (unsigned char)t;
	line_no++;
	if (fscanf(fd, "count\t%d\n", &t) != 1)
	{
		(void)fprintf(stderr,"%s: expected 'count' at line %d in %s\n",
			      prog, line_no, name);
		exit(1);
	}
	header->count = (unsigned char)t;
	line_no++;
	if (fscanf(fd, "start\t%d\n", &t) != 1)
	{
		(void)fprintf(stderr,"%s: expected 'start' at line %d in %s\n",
			      prog, line_no, name);
		exit(1);
	}
	header->start = (char)t;
	line_no++;
}

void read_char(fd, num, header, bitmap)
FILE *fd;
int num;
struct font_header header;
char *bitmap;
{
	int	i,j;
	char	*p;

	/*
	 * skip over blank line, comment line, blank line, and dashes
	 */
	if (fgets(inbuf, LINESIZ, fd) == NULL)
	{
		(void)fprintf(stderr, "%s: read error at line %d of %s\n",
			      prog, line_no, name);
		exit(1);
	}
	line_no++;
	if (fgets(inbuf, LINESIZ, fd) == NULL)
	{
		(void)fprintf(stderr, "%s: read error at line %d of %s\n",
			      prog, line_no, name);
		exit(1);
	}
	line_no++;
	if (fgets(inbuf, LINESIZ, fd) == NULL)
	{
		(void)fprintf(stderr, "%s: read error at line %d of %s\n",
			      prog, line_no, name);
		exit(1);
	}
	line_no++;
	if (fgets(inbuf, LINESIZ, fd) == NULL)
	{
		(void)fprintf(stderr, "%s: read error at line %d of %s\n",
			      prog, line_no, name);
		exit(1);
	}
	line_no++;

	for (i=0; i < header.high; i++)
	{
		p = inbuf;
		if (*p++ != '|')
		{
			(void)fprintf(stderr,
				      "%s: wanted '|bits|' at line %d of %s\n",
				      prog, line_no, name);
			exit(1);
		}
		for (j=0; j < header.wide; j++)
		{
			if (*p == '*')
			{
				bit_set(num, i, j, bitmap, header);
			}
			else if (*p != ' ')
			{
				(void)fprintf(stderr,
				      "%s: wanted '|bits|' at line %d of %s\n",
				      prog, line_no, name);
				exit(1);
			}
			p++;
		}
		if (*p++ != '|')
		{
			(void)fprintf(stderr,
				      "%s: wanted '|bits|' at line %d of %s\n",
				      prog, line_no, name);
			exit(1);
		}
		if (fgets(inbuf, LINESIZ, fd) == NULL)
		{
			(void)fprintf(stderr,
				      "%s: read error at line %d of %s\n",
				      prog, line_no, name);
			exit(1);
		}
		line_no++;
	}

	/*
	 * eat last line of dashes
	 */
	(void)fgets(inbuf, LINESIZ, fd);
	line_no++;
}

void bit_set(num, i, j, bitmap, header)
int num, i, j;
char *bitmap;
struct font_header header;
{
	int 	x, col;

	x = header.wide * header.count;
	col = (i * ((x+15)&~15)) + (num * header.wide) + j;
	bitmap[col/8] |= 1 << (7-(col%8));
}
