/*{{{}}}*/
/*{{{  Notes*/
/* read op file (binary format), strip out images */
/* hack -f flag to generate 1st image only */
/*}}}  */
/*{{{  #includes*/
#include <mgr/bitblit.h>
#include <mgr/share.h>

#include "getshort.h"
/*}}}  */
/*{{{  #defines*/
#define A(x)		args[x]		/* short hand */
#define Max(a,b)	((a)>(b)?(a):(b))
#define NAME		"map."
/*}}}  */

/*{{{  variables*/
char path[512];			/* path name for bitmap */
unsigned short args[10];	/* arg buffer */
char line[80];			/* line buffer */
/*}}}  */

/*{{{  write_map*/
static int
write_map(path,w,h)
char *path;					/* name of bitmap */
int w,h;						/* size of bitmap */
	{
	int bytes = bit_size(w,h,1);
	BITMAP *map = bit_alloc(w,h,NULL,1);
	FILE *file = fopen(path,"w");

	printf("Writing map [%s]\n",path);
	fread(BIT_DATA(map),bytes,1,stdin);
	bitmapwrite(file,map);
	fclose(file);
	bit_destroy(map);
	};
/*}}}  */

/*{{{  main*/
int main(argc,argv)
int argc;
char **argv;
	{
	int count=0;
	char *name;				/* name prefix to bitmap */
	register int c;				/* item type */
	int first = 0;				/* get first map only */
	int id=0;				/* id of first map */

	if (argc>1 && strcmp(argv[1],"-f")==0) {
		first++;
		argc--, argv++;
		}

	if (argc>1)
		name = argv[1];
	else
		name = NAME;

	while((c=getshort()) != EOF) {

		switch (c&TYPE_MASK) {
			case T_BLIT:
				getnshort(args,8);
				break;
			case T_WRITE:
				getnshort(args,5);
				break;
			case T_SCREEN:
				getnshort(args,4);
				if (A(3) && first) {
					write_map(name,A(1),A(2));
					exit(1);
					}
				else if (A(3)) {
					sprintf(path,"%s%03d:%dx%d",name,count++,A(1),A(2));
					write_map(path,A(1),A(2));
					}
				else if (first)
					id=A(0);
				break;
			case T_NOP:
				c &= 0xF;		/* # of bytes to skip */
				while(c-- > 0)
					getchar();
				break;
			case T_LINE:
				getnshort(args,5);
				break;
			case T_DATA:
				getnshort(args,4);
				if (A(3) && first && A(0)==id) {
					write_map(name,A(1),A(2));
					exit(1);
					}
				if (A(3) && first) {
					write_map("/dev/null",A(1),A(2));
					}
				else if (A(3)) {
					sprintf(path,"%s%03d:%dx%d",name,count++,A(1),A(2));
					write_map(path,A(1),A(2));
					}
				break;
			case T_TIME:
				getnshort(args,2);
				break;
			case T_POINT:
				getnshort(args,3);
				break;
			case T_KILL:
				getnshort(args,1);
				break;
			case T_MARK:
				getnshort(args,1);
				break;
                        case T_BYTESCROLL:
                        	getnshort(args,6);
                                break;
			default:
				fprintf(stderr,"OOps, got 0x%x\n",c);
				break;
			}
		}
		return 0;
	}
/*}}}  */
