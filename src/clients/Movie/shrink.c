/*{{{}}}*/
/*{{{  Notes*/
/* read op file (binary format) shrink it in half - print back out */
/*}}}  */
/*{{{  #includes*/
#include <mgr/bitblit.h>
#include <mgr/share.h>

#include "getshort.h"
#include "putshort.h"
/*}}}  */
/*{{{  #defines*/
#define THRES		2		/* shrinking threshold */
#define A(x)		args[x]		/* short hand */
#define Max(a,b)	((a)>(b)?(a):(b))
/*}}}  */

/*{{{  variables*/
unsigned short args[10];
char line[80];
/*}}}  */

/*{{{  main*/
int main(int argc, char *argv[])
	{
	register int c;
	register int type;
	unsigned char *table;		/* table of bit combos */
	unsigned char *build_table();
	BITMAP *map;

	table = build_table(NULL,argc>1?atoi(argv[1]):THRES);

	while((c=getshort()) != EOF) {
		putshort(c);
		switch (c&TYPE_MASK) {
			case T_BLIT:
				getargs(args,8);
				A(2) >>= 1;
				A(3) >>= 1;
				A(4) >>= 1;
				A(5) >>= 1;
				A(6) >>= 1;
				A(7) >>= 1;
				putargs(args,8);
				break;
			case T_WRITE:
				getargs(args,5);
				A(1) >>= 1;
				A(2) >>= 1;
				A(3) >>= 1;
				A(4) >>= 1;
				putargs(args,5);
				break;
			case T_SCREEN:
				getargs(args,4);
				if (A(3)) {
					map = bit_alloc(A(1),A(2),0,1);
					fread(BIT_DATA(map), 1, bit_size(A(1),A(2),1), stdin);
					shrink_map(table,map);
					A(1) = BIT_WIDE(map);
					A(2) = BIT_HIGH(map);
					}
				else {
					A(1) = A(1)>>1;
					A(2) = A(2)>>1;
					}
				putargs(args,4);
				if (A(3)>0) {
					fwrite(BIT_DATA(map), 1, bit_size(A(1),A(2),1), stdout);
					bit_destroy(map);
					}
				break;
			case T_NOP:
				c &= 0xF;		/* # of bytes to skip */
				while(c-- > 0) {
					int ch = getchar();
					putchar(ch);
					}
				break;
			case T_LINE:
				getargs(args,5);
				A(1) >>= 1;
				A(2) >>= 1;
				A(3) >>= 1;
				A(4) >>= 1;
				putargs(args,5);
				break;
			case T_DATA:
				getargs(args,4);
				if (A(3)>0) {
					map = bit_alloc(A(1),A(2),0,1);
					fread(BIT_DATA(map), 1, bit_size(A(1),A(2),1), stdin);
					shrink_map(table,map);
					}
				A(1) = map ? BIT_WIDE(map) : A(1)>>1;
				A(2) = map ? BIT_HIGH(map) : A(2)>>1;
				putargs(args,4);
				if (A(3)>0) {
					fwrite(BIT_DATA(map), 1, bit_size(A(1),A(2),1), stdout);
					bit_destroy(map);
					}
				break;
			case T_TIME:
				getargs(args,2);
				putargs(args,2);
				break;
			case T_POINT:
				getargs(args,3);
				A(1) >>= 1;
				A(2) >>= 1;
				putargs(args,3);
				break;
			case T_KILL:
				getargs(args,1);
				putargs(args,1);
				break;
			default:
				fprintf(stderr,"OOps, got 0x%x\n",c);
				break;
			}
		}
	}
/*}}}  */

getargs(args,count)
short *args;
int count;
	{
	while (count-- > 0)
		*args++ = getshort();
	return(0);
	}

putargs(args,count)
short *args;
int count;
	{
	while (count-- > 0)
		putshort(*args++);
	return(0);
	}
