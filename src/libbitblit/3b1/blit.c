/*                        Copyright (c) 1989 Bellcore
 *                            All Rights Reserved
 *       Permission is granted to copy or use this program, EXCEPT that it
 *       may not be sold for profit, the copyright notice must be reproduced
 *       on copies, and credit should be given to Bellcore where it is due.
 *       BELLCORE MAKES NO WARRANTY AND ACCEPTS NO LIABILITY FOR THIS PROGRAM.
 */
/*
  	$Header:
	$Source:
*/
static char	RCSid_[] = "$Source:";

/* bitblit code template generator for portable bitblt code */

#include <stdio.h>
#include "bitmap.h"

#define RIGHT		0x0	/* direction right -> left */
#define DOWN		0x0	/* direction top->bottom */
#define UP			0x10	/* direction bottom up */
#define LEFT		0x20	/* direction right to left */
#define SMALL		0x40	/* less than 1 full word */
#define ALIGNED	0x80	/* src & dst aligned, no shifts */
#define NSRC	1		/* no source required */

extern int bit_debug;
extern BITMAP *screen;

#define dprintf	if(bit_debug)fprintf

static char nsrc[16] = {		/* fold no source cases */
	0,0,0,0,
	0xf&~DST, 0xf&~DST, 0xf&~DST, 0xf&~DST,
	0xf&DST, 0xf&DST, 0xf&DST, 0xf&DST, 
	0xf, 0xf, 0xf, 0xf
	};

static char zsrc[16] = {		/* no source req'd cases */
	1,0,0,0,0,
	1,0,0,0,0,
	1,0,0,0,0,
	1 };	

/* table of inverted bitblts (changes the sense of black and white) */

char op_invert[16] = {
	15, 7, 11, 3, 13, 5, 9, 1, 14, 6, 10, 2, 12, 4, 8, 0
	};

/* the 16 bit-blit functions */

#define ZERO(d,s)	(0)
#define NOR(d,s)	(~((d)|(s)))
#define AND2(d,s)	((d)&~(s))
#define NPROJ2(d,s)	(~(s))
#define AND1(d,s)	(~(d)&(s))
#define NPROJ1(d,s)	(~(d))
#define XOR(d,s)	((d)^(s))
#define NAND(d,s)	(~((d)&(s)))
#define AND(d,s)	((d)&(s))
#define NXOR(d,s)	(~((d)^(s)))
#define PROJ1(d,s)	((d))
#define OR2(d,s)	((d)|~(s))
#define PROJ2(d,s)	(s)
#define OR1(d,s)	(~(d)|(s))
#define OR(d,s)		((d)|(s))
#define ONE(d,s)	(~0)

/* The templates go here.  I could combine right->left and left-> right
   into a single template, but its probably not worth the bother.
   The src & dst aligned cases are needed because 32 bit shifts are
   undefined (and wrong on the 3100).
*/

/* single word bitblt */

#define Rop_s(op) \
if (GETLSB(lmask,lshift)) {\
/*   for (i=count; i>0; i--) {			\
*/\
    i = count; \
    do { \
      osrc = src++;\
      *dst = (op(*dst,((GETMSB(*osrc,lshift)) \
		 | GETLSB(*src,rshift))) & lmask) | (*dst & ~lmask);\
      src += s_skip;  dst += d_skip;				\
      } while (--i); \
} else {\
/*   for (i=count; i>0; i--) {			\
*/\
    i = count; \
    do { \
      src++;					\
      *dst = (op(*dst,(GETLSB(*src,rshift))) & lmask) | (*dst & ~lmask);\
      src += s_skip;  dst += d_skip;				\
      } while (--i); \
}

/* single word bitblt (src and dst aligned) */

#define Rop_sa(op) \
   for (i=count; i>0; i--) {			\
      *dst = (op(*dst,*src++) & lmask) | (*dst & ~lmask), dst++;\
      src += s_skip;  dst += d_skip;				\
      }

/* multiword left to right bitblt (src and dst aligned) */

#define Rop_incr_a(op) \
/*   for (i=count; i>0; i--) {			\
*/\
    i = count;\
    do {\
      *dst = (op(*dst,*src++) & lmask) | (*dst & ~lmask), dst++;\
      LOOP(words-2,(*dst = op(*dst,*src++),dst++)); \
      *dst = (op(*dst,*src++) & rmask) | (*dst & ~rmask), dst++;\
      src += s_skip;  dst += d_skip;				\
      } while (--i)

/* multiword right to left bitblt (source and dest aligned) */

#define Rop_decr_a(op) \
   for (i=count; i>0; i--) {			\
      *dst = (op(*dst,*src--) & rmask) | (*dst & ~rmask), dst--;\
      LOOP(words-2,(*dst = op(*dst,*src--),dst--)); \
      *dst = (op(*dst,*src--) & lmask) | (*dst & ~lmask), dst--;\
      src += s_skip;  dst += d_skip;				\
      }

/* multiword left to right bitblt */

#define Rop_incr_m(op) \
   for (i=count; i>0; i--) {			\
      osrc = src;  src++;					\
      *dst = (op(*dst,((GETLSB(lmask,lshift) ? GETMSB(*osrc,lshift) : 0) \
		 | GETLSB(*src++,rshift))) & lmask) | (*dst & ~lmask), dst++;\
      osrc++; \
      LOOP(words-2,(*dst = \
			op(*dst,GETMSB(*osrc++,lshift) | GETLSB(*src++,rshift)),dst++) \
			); \
      *dst = (op(*dst,(GETMSB(*osrc++,lshift) | GETLSB(*src++,rshift))) \
					& rmask) | (*dst & ~rmask), dst++;\
      src += s_skip;  dst += d_skip;				\
      }

/* multiword right to left bitblt */

#define Rop_decr_m(op) \
   for (i=count; i>0; i--) {			\
      osrc = src;  osrc--;					\
      *dst = (op(*dst,(GETMSB(*osrc--,lshift) | GETLSB(*src--,rshift))) \
					& rmask) | (*dst & ~rmask), dst--;\
      LOOP(words-2,(*dst = \
			op(*dst,GETMSB(*osrc--,lshift) | GETLSB(*src--,rshift)) \
			,dst--)); \
      *dst = (op(*dst,((GETLSB(lmask,lshift) ? GETMSB(*osrc--,lshift) : 0) \
		 | GETLSB(*src--,rshift))) & lmask) | (*dst & ~lmask), dst--;\
      osrc--; \
      src += s_skip;  dst += d_skip;				\
      }

/* an unrolled loop (some compilers -i.e. DEC3100 can't unroll 16 at a time) */

#define LOOP(n,s) {					\
    cnt = (n);\
    if (cnt>=8) {\
	do {\
	    s;s;s;s;s;s;s;s;			\
	    cnt -= 8;\
	} while (cnt >= 8);\
    }\
    if (cnt>0) {\
	do {\
	   s;\
	} while (--cnt);\
    }\
}

/* do all 16 cases */

#define SWITCH(op,func) \
		switch(op&0xf) { \
			case OPCODE(ZERO(DST,SRC)):	func(ZERO);		break; \
			case OPCODE(NOR(DST,SRC)):		func(NOR);		break; \
			case OPCODE(AND2(DST,SRC)):	func(AND2);		break; \
			case OPCODE(NPROJ2(DST,SRC)):	func(NPROJ2);	break; \
			case OPCODE(AND1(DST,SRC)):	func(AND1);		break; \
			case OPCODE(NPROJ1(DST,SRC)):	func(NPROJ1);	break; \
			case OPCODE(XOR(DST,SRC)):	 	func(XOR);		break; \
			case OPCODE(NAND(DST,SRC)):	func(NAND);		break; \
			case OPCODE(AND(DST,SRC)):		func(AND);		break; \
			case OPCODE(NXOR(DST,SRC)):	func(NXOR);		break; \
			case OPCODE(PROJ1(DST,SRC)):	func(PROJ1);	break; \
			case OPCODE(OR2(DST,SRC)):		func(OR2);		break; \
			case OPCODE(PROJ2(DST,SRC)):	func(PROJ2);	break; \
			case OPCODE(OR1(DST,SRC)):	 	func(OR1);		break; \
			case OPCODE(OR(DST,SRC)):		func(OR);		break; \
			case OPCODE(ONE(DST,SRC)):		func(ONE);		break; \
			}

#define XWITCH(op,func)		/* use to turn off cases for debugging */

mem_rop(dst_map,x_dst,y_dst,wide,high,op,src_map,x_src,y_src)
BITMAP *dst_map;				/* source bitmap */
BITMAP *src_map;				/* destination bitmap */
int x_dst,y_dst;				/* destination coords */
int x_src,y_src;				/* source coords */
int wide,high;					/* bitmap size */
int op;							/* bitmap function */
   {
	register short cnt;		/* inner loop counter */
	register DATA *dst;		/* dest bitmap base addresses */
	register DATA *src;		/* source bitmap base addresses */
	register DATA *osrc;		/* previous source addr */

	register int s_offset;			/* source bit offset */
	register int d_skip;				/* words to next line in dst_map */
	register int s_skip;				/* words to next line in src_map */
	register short count=high;		/* # of rows */
   register int words;				/* words across in inner loop (>32 bits) */
   register DATA lmask;		/* bits on left to next long boundary */
   register DATA rmask;		/* bits from last long on right */
	register short lshift;				/* bits to shift left on next word */
	register short rshift;				/* bits to shift right on previous word */
	register int i;

	/* clipping and argument checking */

	if (!src_map) {
      if (bit_debug && op&0xf != 0xf&nsrc[op&0xf])
		   fprintf(stderr,"no src_map, setting op %d->%d\n",op&0xf,nsrc[op&0xf]);
      op = 0xf&nsrc[op&0xf];						/* a NULL src_map sources 1's */
		}

	else if (zsrc[op&0xf])					 {
      if (bit_debug && src_map)
		   fprintf(stderr,"op=%d, setting src_map->NULL\n",op&0xf);
		src_map == BIT_NULL;							/* don't check no src_map cases */
		}

#ifndef NOCLIP
	
	if (wide<0) {
		dprintf(stderr,"Clip: w<0 (%d)\n",wide);
		x_dst += wide;
		wide = - wide;
		}

	if (count<0) {
		y_dst += count;
		count = - count;
		dprintf(stderr,"Clip: h<0 (%d)\n",count);
		}

   if (x_dst < 0) {
		dprintf(stderr,"Clip: x_dst<0 (%d)\n",x_dst);
		if (src_map)
			x_src -= x_dst;
		wide += x_dst;
		x_dst = 0;
		}

   if (y_dst < 0) {
		dprintf(stderr,"Clip: y_dst<0 (%d)\n",y_dst);
		if (src_map)
			y_src -= y_dst;
		count += y_dst;
		y_dst = 0;
		}

	if (src_map) {
		if (x_src < 0) {
			dprintf(stderr,"Clip: x_src<0 (%d)\n",x_src);
			x_dst -= x_src;
			wide += x_src;
			x_src = 0;
			}

		if (y_src < 0) {
			dprintf(stderr,"Clip: y_src<0 (%d)\n",y_src);
			y_dst-=y_src;
			count+=y_src;
			y_src=0;
			}
			
		if ((i = x_src+wide - src_map->wide) > 0) {
			dprintf(stderr,"Clip: wide too big for src (%d->%d)\n",wide,wide-i);
			wide -= i;
			}

		if ((i = y_src+count - src_map->high) > 0) {
			dprintf(stderr,"Clip: high too big for src (%d->%d)\n",count,count-i);
			count -= i;
			}
		}

	if ((i = x_dst + wide - dst_map->wide) > 0) {
		dprintf(stderr,"Clip: wide too big for dst_map (%d->%d)\n",wide,wide-i);
		wide -= i;
		}
	if ((i = y_dst + count - dst_map->high) > 0) {
		dprintf(stderr,"Clip: high too big for dst_map (%d->%d)\n",count,count-i);
		count -= i;
		}

	if (wide<1 || count < 1) {
		dprintf(stderr,"Clip: high or wide < 1 (%d,%d)\n",wide,count);
		return(-1);
		}

	/* end of clipping code */

#endif

	x_dst += dst_map->x0;
	y_dst += dst_map->y0;
	i=0;

	/* set initial conditions */

	if (DOFLIP && dst_map->primary->type&_FLIP) {
		flip(dst_map->data,BIT_LINE(dst_map)*dst_map->primary->high);
		dst_map->primary->type &= 3;
		}

	words = ((x_dst+wide-1)>>LOGBITS) - (x_dst>>LOGBITS) + 1;
	lmask = GETLSB((DATA) ~0, (x_dst&BITS));
	rmask = GETMSB((DATA) ~0, (BITS - ((x_dst+wide-1)&BITS)));
	dst = dst_map->data + BIT_LINE(dst_map)*y_dst + (x_dst>>LOGBITS);
	d_skip = BIT_LINE(dst_map);		/* longs to next row */

   if (src_map) {

		if (DOFLIP && src_map->primary->type&_FLIP) {
			flip(src_map->data,BIT_LINE(src_map)*src_map->primary->high);
			src_map->primary->type &= 3;
			}

		x_src += src_map->x0;
		y_src += src_map->y0;
		src = src_map->data + BIT_LINE(src_map)*y_src + (x_src>>LOGBITS);
		s_skip = BIT_LINE(src_map);
		lshift = (x_src&BITS) - (x_dst&BITS);
		if (lshift > 0) {
			rshift = (BITS+1) - lshift;
			}
		else  if (lshift < 0){
			rshift = -lshift;
			lshift = (BITS+1) - rshift;
			src --;
			}
		else {
			i |= ALIGNED;
			}

		/* set blit direction */

		if (src_map->data == dst_map->data) {
			if (y_dst>y_src)
				i |= UP;
			if (x_dst>x_src)
				i |= LEFT;
			}

		}

	if (words <=1 ) {
		i |= SMALL;
		lmask &= rmask;
		}

	/* do the bitblt */

/*
	dprintf(stderr,"DIR(%x) %dx%d %d,%d->%d,%d (0x%x->0x%x)\n",
			i,count,wide,x_src,y_src,x_dst,y_dst,src,dst);
*/

   switch(i) {
	case RIGHT|DOWN:					/* top->bottom		left->right */
		d_skip -= words;
		s_skip -= (words + 1);
		SWITCH(op,Rop_incr_m);
      break;

	case RIGHT|DOWN|SMALL:					/* top->bottom		small */
	case LEFT|DOWN|SMALL:
		d_skip -= words - 1;
		s_skip -= words;
		SWITCH(op,Rop_s);
      break;

	case RIGHT|DOWN|ALIGNED:		/* top->bottom		left->right */
		d_skip -= words;
		s_skip -= words;
		SWITCH(op,Rop_incr_a);
      break;

	case RIGHT|DOWN|SMALL|ALIGNED:		/* top->bottom		small */
	case LEFT|DOWN|SMALL|ALIGNED:	
		d_skip -= words;
		s_skip -= words;
		SWITCH(op,Rop_sa);
      break;

	case RIGHT|UP:						/* bottom->top		left->right */
		dst += d_skip*(count-1);
		d_skip = - (d_skip + words);
		if (src_map) {
			src += s_skip*(count-1);
			s_skip = - (s_skip + words + 1);
			}
		SWITCH(op,Rop_incr_m);
      break;

	case RIGHT|UP|SMALL:						/* bottom->top		small */
	case LEFT|UP|SMALL:						/* bottom->top		small */
		dst += d_skip*(count-1);
		d_skip = - (d_skip + words - 1);
		if (src_map) {
			src += s_skip*(count-1);
			s_skip = - (s_skip + words);
			}
		SWITCH(op,Rop_s);
      break;

	case RIGHT|UP|ALIGNED:					/* bottom->top		left->right */
		dst += d_skip*(count-1);
		d_skip = - (d_skip + words);
		if (src_map) {
			src += s_skip*(count-1);
			s_skip = - (s_skip + words);
			}
		SWITCH(op,Rop_incr_a);
      break;

	case RIGHT|UP|SMALL|ALIGNED:					/* bottom->top		left->right */
	case LEFT|UP|SMALL|ALIGNED:
		dst += d_skip*(count-1);
		d_skip = - (d_skip + words);
		if (src_map) {
			src += s_skip*(count-1);
			s_skip = - (s_skip + words);
			}
		SWITCH(op,Rop_sa);
      break;

	case LEFT|DOWN:					/* top->bottom		right->left */
		dst += words - 1;
		d_skip += words;
		if (src_map) {
			src += words;
			s_skip += words;
			}
		SWITCH(op,Rop_decr_m);
      break;

	case LEFT|DOWN|ALIGNED:			/* top->bottom		right->left */
		dst += words - 1;
		d_skip += words;
		if (src_map) {
			src += words - 1;
			s_skip += words;
			}
		SWITCH(op,Rop_decr_a);
      break;

	case UP|LEFT:						/* bottom->top		right->left */
		dst += d_skip*(count-1) + words - 1;
		d_skip = - (d_skip - words);
		if (src_map) {
			src += s_skip*(count-1) + words;
			s_skip = -(s_skip - words);
			}
		SWITCH(op,Rop_decr_m);
      break;

	case UP|LEFT|ALIGNED:					/* bottom->top		right->left */
		dst += d_skip*(count-1) + words - 1;
		d_skip = - (d_skip - words);
		if (src_map) {
			src += s_skip*count + words - 1;
			s_skip = -(s_skip - words );
			}
		SWITCH(op,Rop_decr_a);
      break;

	default:
		dprintf(stderr,"Invalid direction: 0x%x\n",i);
		break;
		}
	return(0);
  	}
