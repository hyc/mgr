/*                        Copyright (c) 1989 Bellcore
 *                            All Rights Reserved
 *       Permission is granted to copy or use this program, EXCEPT that it
 *       may not be sold for profit, the copyright notice must be reproduced
 *       on copies, and credit should be given to Bellcore where it is due.
 *       BELLCORE MAKES NO WARRANTY AND ACCEPTS NO LIABILITY FOR THIS PROGRAM.
 */
/*	$Header: /files/src/linuxmgr/src/libbitblit/dec3100/RCS/bitmap.h,v 4.3 1994/01/28 21:01:44 broman Rel $
	$Source: /files/src/linuxmgr/src/libbitblit/dec3100/RCS/bitmap.h,v $
*/
static char	h_bitmap_[] = "$Source: /files/src/linuxmgr/src/libbitblit/dec3100/RCS/bitmap.h,v $$Revision: 4.3 $";

/* header file for dec3100-portable bitblt code */

#ifndef Min
#define Min(x,y)	((x)>(y)?y:x)
#endif

typedef unsigned int DATA;			/* how to access the frame buffer */

/* Machine configurations go here */
/* this defines how bits get shifted out of each word */

#define GETMSB(word,shift) \
	(word >> shift)               /* get most significant bits in word */
#define GETLSB(word,shift) \
	(word << shift)               /* get least significant bits in word */

#define LOGBITS  5                 /* Log2 of bits in type DATA */
#define BITS   (~(~0 << LOGBITS))  /* mask for bit# within word */
#define MSB    (~GETLSB((unsigned)~0,1))      /* most sig bit set */    
#define LSB    (~GETMSB((unsigned)~0,1))      /* least sig bit set */    
#define DOFLIP	(MSB==1)					/* need to flip bytes */

#define bit_blit(dest,dx,dy,width,height,func,source,sx,sy)  \
	mem_rop(dest,dx,dy,width,height,func,source,sx,sy) 

#define bit_static(name,wide,high,data,n)	\
	BITMAP name = { \
		(DATA *) data, &name, 0, 0, wide, high, _STATIC | _FLIP \
		};

#define NULL_DATA	((DATA *) 0)
#define BIT_NULL	((BITMAP *) 0)

#define IS_SCREEN(x)		(3&(x)->type==_SCREEN)
#define IS_MEMORY(x)		(3&(x)->type==_MEMORY)
#define IS_PRIMARY(x)	((x)->primary == (x))
#define SET_FLIP(x)     ((x)->primary->type |= DOFLIP ? _FLIP : 0)

/*
 * OPCODE(expr), where expr is boolean expression involving SRC and DST,
 * is one of sixteen numbers encoding a rasterop opcode.
 */

#define			DST 	0xA	/* 1010 */
#define			SRC	0xC	/* 1100 */
#define OPCODE(expr)	(0xF&(expr))

/* names for common bitblit functions */

#ifndef BIT_NOT
#   define BIT_NOT(x)	(0xf&~(x))
#endif
#define BIT_SRC		SRC
#define BIT_DST		DST
#define BIT_SET		(BIT_SRC|BIT_NOT(BIT_SRC))
#define BIT_CLR		(BIT_SRC&BIT_NOT(BIT_SRC))
#define BIT_XOR		(BIT_SRC^BIT_DST)
#define BIT_INVERT	(BIT_NOT(DST))
#define GET_OP(x)	((x)&0xf)

/* change rop function for white-on-black */

#define ROP_INVERT(x)	GET_OP(rev_ops[0xf&(x)])

/* bitmap types */

#define _SCREEN		1		/* frame buffer */
#define _MEMORY		2		/* malloc'd space */
#define _STATIC		3		/* don't free space at destroy time */
#define _FLIP			4		/* flip bits on first access */

/* member access macros */

#define BIT_X(x)	x->x0
#define BIT_Y(x)	x->y0
#define BIT_DATA(x)	x->data
#define BIT_WIDE(x)	x->wide
#define BIT_HIGH(x)	x->high
#define BIT_DEPTH(x)	1		/* no color support for now */

/* BIT_SIZE(map) == the number of chars needed to store the data of the bitmap.
   Usually used with malloc(3).
*/

#define BIT_SIZE(m)     BIT_Size(BIT_WIDE(m), BIT_HIGH(m), BIT_DEPTH(m)) /* bytes */

/* BIT_Size(wide,high,depth) = the number of chars needed to store the data of a
   bitmap of the given number of bits wide high and deep.
	   Typical usage:
		char	bitbuffer[ Bit_Size(16,16,1) ];
*/

#define BIT_Size(wide,high,d)     ((d)*((wide+BITS)&~BITS)*high>>3) /* bytes */

#define BIT_LINE(x)	((x->primary->wide+BITS)>>LOGBITS)	/* words/pre scan line */

/* structure and type definitions */

typedef struct bitmap {
   DATA	*data;		/* bitmap data */
   struct bitmap	*primary;	/* pointer to primary bitmap */
   short		x0, y0;		/* starting coordinates, in bits.
									0, 0  ==  upper left corner */
   short		wide, high;	/* bitmap size, in bits */
   unsigned short	type;		/* bitmap type */
   } BITMAP;

/* function declarations */

extern char op_invert[];		/* table of inverted ops */

int mem_rop();
int bit_destroy();
int bit_line();
BITMAP * bit_create();
BITMAP * bit_alloc();
BITMAP * bit_open();

/* for non existant color support */

#define DEPTH				1			/* bits per pixel */
#define NOCOLOR         0xF
#define GETCOLOR(x)     0
#define PUTCOLOR(x)     0

/* other */

#define Bprintf(x)	/* gone */
