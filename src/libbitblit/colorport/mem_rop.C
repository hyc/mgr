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

/* 8 bit version */

/* bitblit code template generator for portable bitblt code (S. A. Uhler)
 *
 * This file contains templates used to generate fast, yet portable bit-blit
 * code.  Color and monochrome bitmaps are supported.  The templates
 * are assembled with cpp, optionally in multiple passes to aid in debugging.
 */

%include <stdio.h>
%include "screen.h"

%define UPLEFT		0x1	/* direction bottom up, R->L */
%define DOWNRIGHT	0x0	/* direction top->bottom L->R */
%define SMALL		0x2	/* at most 1 full DATA per line */
%define ALIGNED		0x4	/* src & dst aligned, no shifts */
%define NOMASK		0x8	/* no shifts or masking */
%define SRC_COLOR	0x10	/* Use FG/BG color as source */

extern int bit_debug;

%define dprintf	if(bit_debug)fprintf

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

/* the 16 bit-blit functions */

5define ZERO(d,s)	(0)
5define NOR(d,s)	(~((d)|(s)))
5define AND2(d,s)	((d)&~(s))
5define NPROJ2(d,s)	(~(s))
5define AND1(d,s)	(~(d)&(s))
5define NPROJ1(d,s)	(~(d))
5define XOR(d,s)	((d)^(s))
5define NAND(d,s)	(~((d)&(s)))
5define AND(d,s)	((d)&(s))
5define NXOR(d,s)	(~((d)^(s)))
5define PROJ1(d,s)	((d))
5define OR2(d,s)	((d)|~(s))
5define PROJ2(d,s)	(s)
5define OR1(d,s)	(~(d)|(s))
5define OR(d,s)		((d)|(s))
5define ONE(d,s)	(~0)

/* incr and decr macros (redefine when src and dst word size are different) */
/*
 * Here is where Uhler had macros used in the Rop code below which
 * permitted distinguishing between the addressing stride between
 * adjacent pixels in the src and dst bitmap data.
 * As written the code used to make impermissible assumptions about
 * the order of side effects, and it seemed to be motivated by
 * a supposed speed advantage to be gained from using autoincrement
 * modes of addressing instead of plain address arithmetic.
 * Since this is not a feature of hardware I deal with, and since word
 * size differences are addressed already in the bit_blit function,
 * I removed the incr/decr macros.  broman@nosc.mil.
 */

/* The templates go here.  I could combine right->left and left-> right
   into a single template, but its probably not worth the bother.
   The src & dst aligned cases are needed because 32 bit shifts are
   undefined (and wrong on the 3100).   [Uhler]
*/

/* single word bitblt */

2define Rop_s(op) \
   for (i=count; i>0; i--) { \
      osrc = src;  src += 1; \
      *dst = (op(*dst,(GETMSB(*osrc,lshift) | GETLSB(*src,rshift))) \
					& lmask) | (*dst & ~lmask); \
      src += s_skip;  dst += d_skip; \
      }

/* single word bitblt source color */

2define Rop_sC(op) \
   for (i=count; i>0; i--) { \
      *dst = (op(*dst,color) & lmask) | (*dst & ~lmask); \
      dst += d_skip; \
      }

/* single word bitblt source color, no masking */

2ifndef BANKED
2define Rop_sC_nm(op) \
   for (i=count; i>0; i--) { \
      *dst = op(*dst,color); \
      dst += d_skip; \
      }
2else /* banked for super vga */
2define Rop_sC_nm(op) \
   for (i=count; i>0; i--) { \
      DATA *dst_wdp = IS_SCREEN( dst_map)? getdatap( dst, dst_map->data, NULL): dst; \
      *dst_wdp = op(*dst_wdp,color); \
      dst += d_skip; \
      }
2endif

/* single word bitblt (src and dst aligned) */

2define Rop_sa(op) \
   for (i=count; i>0; i--) { \
      *dst = (op(*dst,*src) & lmask) | (*dst & ~lmask); \
      src += s_skip;  dst += d_skip; \
      }

/* single word bitblt (src and dst aligned), no masking */

2ifndef BANKED
2define Rop_sa_nm(op) \
   for (i=count; i>0; i--) { \
      *dst = op(*dst,*src); \
      src += s_skip;  dst += d_skip; \
      }
2else /* banked for super vga */
2define Rop_sa_nm(op) \
   for (i=count; i>0; i--) { \
      DATA *src_wdp = IS_SCREEN( src_map)? getdata( src, src_map->data, 1): src; \
      DATA *dst_wdp = IS_SCREEN( dst_map)? getdatap( dst, dst_map->data, NULL): dst; \
      *dst_wdp = op(*dst_wdp,*src_wdp); \
      src += s_skip;  dst += d_skip; \
      }
2endif

/* multiword left to right bitblt (src and dst aligned) */

2define Rop_incr_a(op) \
   for (i=count; i>0; i--) { \
      *dst = (op(*dst,*src) & lmask) | (*dst & ~lmask); \
      src += 1;  dst += 1; \
      LOOP(words-2,*dst = op(*dst,*src); src += 1; dst += 1); \
      *dst = (op(*dst,*src) & rmask) | (*dst & ~rmask); \
      src += s_skip + 1;  dst += d_skip + 1; \
      }

/* multiword left to right bitblt (src and dst aligned), no masking */

2ifndef BANKED
2define Rop_incr_a_nm(op) \
   for (i=count; i>0; i--) { \
      LOOP(words,*dst = op(*dst,*src); src += 1; dst += 1); \
      src += s_skip;  dst += d_skip; \
      }
2else /* banked for super vga */
2define Rop_incr_a_nm(op) \
   for (i=count; i>0; i--) { \
      DATA *src_wdp = IS_SCREEN( src_map)? getdata( src, src_map->data, words): src; \
      unsigned int bct = words; \
      DATA *dst_wdp = IS_SCREEN( dst_map)? getdatap( dst, dst_map->data, &bct): dst; \
      unsigned int wlf = words - bct; \
      LOOP(bct,*dst_wdp = op(*dst_wdp,*src_wdp); src_wdp += 1; dst_wdp += 1); \
      if( wlf) { \
	 dst_wdp = getdatap( dst+bct, dst_map->data, NULL); \
	 LOOP(wlf,*dst_wdp = op(*dst_wdp,*src_wdp); src_wdp += 1; dst_wdp += 1); \
      } \
      src += s_skip + words;  dst += d_skip + words; \
      }
2endif

/* multiword right to left bitblt (source and dest aligned) */

2define Rop_decr_a(op) \
   for (i=count; i>0; i--) { \
      *dst = (op(*dst,*src) & rmask) | (*dst & ~rmask); \
      src -= 1;  dst -= 1; \
      LOOP(words-2,*dst = op(*dst,*src); src -= 1; dst -= 1); \
      *dst = (op(*dst,*src) & lmask) | (*dst & ~lmask); \
      src += s_skip - 1;  dst += d_skip - 1; \
      }

/* multiword right to left bitblt (source and dest aligned), no masking */

2ifndef BANKED
2define Rop_decr_a_nm(op) \
   for (i=count; i>0; i--) { \
      LOOP(words,*dst = op(*dst,*src); src -= 1; dst -= 1); \
      src += s_skip;  dst += d_skip; \
      }
2else /* banked for super vga */
2define Rop_decr_a_nm(op) \
   for (i=count; i>0; i--) { \
      DATA *src_wdp = IS_SCREEN( src_map)? \
		  getdata( src+1-words, src_map->data, words)+words-1: src; \
      unsigned int bct = words; \
      DATA *dst_wdp = IS_SCREEN( dst_map)? getdatabkp( dst, dst_map->data, &bct): dst; \
      unsigned int wlf = words - bct; \
      LOOP(bct,*dst_wdp = op(*dst_wdp,*src_wdp); src_wdp -= 1; dst_wdp -= 1); \
      if( wlf) { \
	 dst_wdp = getdatabkp( dst-bct, dst_map->data, NULL); \
	 LOOP(wlf,*dst_wdp = op(*dst_wdp,*src_wdp); src_wdp -= 1; dst_wdp -= 1); \
      } \
      src += s_skip - words;  dst += d_skip - words; \
      }
2endif

/* multiword left to right bitblt */

2define Rop_incr_m(op) \
   for (i=count; i>0; i--) { \
      osrc = src;  src += 1; \
      *dst = (op(*dst,(GETMSB(*osrc,lshift) | GETLSB(*src,rshift))) \
					& lmask) | (*dst & ~lmask); \
      dst += 1;  osrc += 1;  src += 1; \
      LOOP(words-2,*dst = op(*dst,GETMSB(*osrc,lshift) | GETLSB(*src,rshift)); \
		   dst += 1; osrc += 1; src += 1); \
      *dst = (op(*dst,(GETMSB(*osrc,lshift) | GETLSB(*src,rshift))) \
					& rmask) | (*dst & ~rmask); \
      src += s_skip;  dst += d_skip; \
      }

/* multiword bitblt source color */

2define Rop_C(op) \
   for (i=count; i>0; i--) { \
      *dst = (op(*dst,color) & lmask) | (*dst & ~lmask); \
      dst += 1; \
      LOOP(words-2,*dst = op(*dst,color); dst += 1); \
      *dst = (op(*dst,color) & rmask) | (*dst & ~rmask); \
      dst += d_skip + 1; \
      }

/* multiword bitblt source color, no masking */

2ifndef BANKED
2define Rop_C_nm(op) \
   for (i=count; i>0; i--) { \
      LOOP(words,*dst = op(*dst,color); dst += 1); \
      dst += d_skip; \
      }
2else /* banked for super vga */
2define Rop_C_nm(op) \
   for (i=count; i>0; i--) { \
      unsigned int bct = words; \
      DATA *dst_wdp = IS_SCREEN( dst_map)? getdatap( dst, dst_map->data, &bct): dst; \
      unsigned int wlf = words - bct; \
      LOOP(bct,*dst_wdp = op(*dst_wdp,color); dst_wdp += 1); \
      if( wlf) { \
	 dst_wdp = getdatap( dst+bct, dst_map->data, NULL); \
	 LOOP(wlf,*dst_wdp = op(*dst_wdp,color); dst_wdp += 1); \
      } \
      dst += d_skip + words; \
      }
2endif

/* multiword right to left bitblt */

2define Rop_decr_m(op) \
   for (i=count; i>0; i--) { \
      osrc = src;  osrc -= 1; \
      *dst = (op(*dst,(GETMSB(*osrc,lshift) | GETLSB(*src,rshift))) \
					& rmask) | (*dst & ~rmask); \
      dst -= 1;  osrc -= 1;  src -= 1; \
      LOOP(words-2,*dst = op(*dst,GETMSB(*osrc,lshift) | GETLSB(*src,rshift)); \
		   dst -= 1;  osrc -= 1;  src -= 1); \
      *dst = (op(*dst,(GETMSB(*osrc,lshift) | GETLSB(*src,rshift))) \
					& lmask) | (*dst & ~lmask); \
      src += s_skip;  dst += d_skip; \
      }

%ifdef UNROLL

/* an unrolled loop (some compilers -i.e. DEC3100 can't unroll 16 at a time) */

3define LOOP(n,s) {					\
    register int cnt;					\
    for (cnt=(n); cnt>=8; cnt-=8) {		\
       s;s;s;s;s;s;s;s;			\
       }						\
    switch (cnt) {					\
       case  7: s; case  6: s; case  5: s; case  4: s;	\
       case  3: s; case  2: s; case  1: s; 		\
       }						\
    }
%else
3define LOOP(n,s) {\
    register int cnt;\
    for (cnt=(n); cnt>0; cnt--) {\
       s;\
		} \
    }
%endif

/* do all 16 cases - eliminating never-used cases saves code space */

1define SWITCH(op,func) \
		switch(op&0xf) { \
			case OPCODE(ZERO(DST,SRC)):	func(ZERO);	break; \
			case OPCODE(NOR(DST,SRC)):	func(NOR);	break; \
			case OPCODE(AND2(DST,SRC)):	func(AND2);	break; \
			case OPCODE(NPROJ2(DST,SRC)):	func(NPROJ2);	break; \
			case OPCODE(AND1(DST,SRC)):	func(AND1);	break; \
			case OPCODE(NPROJ1(DST,SRC)):	func(NPROJ1);	break; \
			case OPCODE(XOR(DST,SRC)):	func(XOR);	break; \
			case OPCODE(NAND(DST,SRC)):	func(NAND);	break; \
			case OPCODE(AND(DST,SRC)):	func(AND);	break; \
			case OPCODE(NXOR(DST,SRC)):	func(NXOR);	break; \
			case OPCODE(PROJ1(DST,SRC)):	/* nop */	break; \
			case OPCODE(OR2(DST,SRC)):	func(OR2);	break; \
			case OPCODE(PROJ2(DST,SRC)):	func(PROJ2);	break; \
			case OPCODE(OR1(DST,SRC)):	func(OR1);	break; \
			case OPCODE(OR(DST,SRC)):	func(OR);	break; \
			case OPCODE(ONE(DST,SRC)):	func(ONE);	break; \
			}

1define XWITCH(op,func)		/* use to turn off cases for debugging */


/* main entry point for memory-to-memory bit-blts */

int
mem_rop(dst_map,x_dst,y_dst,wide,high,op,src_map,x_src,y_src)
BITMAP *dst_map;				/* source bitmap */
BITMAP *src_map;				/* destination bitmap */
int x_dst,y_dst;				/* destination coords */
int x_src,y_src;				/* source coords */
int wide,high;					/* bitmap size */
int op;						/* bitmap function */
{
	register DATA *dst;		/* dest bitmap base addresses */
	register DATA *src;		/* source bitmap base addresses */
	register DATA *osrc;		/* previous source addr */

	register int d_skip;		/* words to next line in dst_map */
	register int s_skip;		/* words to next line in src_map */
	register int count=high;	/* # of rows */
	register int words;		/* words across in inner loop (>32 bits) */
	register DATA lmask;		/* bits on left to next long boundary */
	register DATA rmask;		/* bits from last long on right */
	register int lshift;		/* bits to shift left on next word */
	register int rshift;		/* bits to shift right on previous word */
	register int i;			/* temporary and loop counter */
	register DATA color;		/* fg or bg color for no source */
	register int mode=0;		/* bitblit mode */
	int depth = dst_map -> depth;	/* depth of operation */

	/* source map not used (should we clip first??) */
	if (src_map && zsrc[op&0xf]) {
		dprintf(stderr,"op=%d, setting src_map->NULL\n",op&0xf);
		src_map = BIT_NULL;		/* don't check no src_map cases */
		}

	if (src_map &&  dst_map && depth != src_map->depth) {
		fprintf(stderr,"Incompatable depths: %d -> %d\n",
					src_map->depth, depth);
		return(-1);
		}

%ifdef INVERT
	/* invert sense of fg and bg colors */
	op = SWAPCOLOR(op);
%endif

	/* color - no source map */

	if (!src_map && depth>1) {
		switch (OPCODE(op)) {
			case OPCODE(0):
				color = GETBCOLOR(op);
				op = OPCODE(SRC);
				break;
			case OPCODE(~0):
				color = GETFCOLOR(op);
				op = OPCODE(SRC);
				break;
			case OPCODE(~SRC):
			case OPCODE(~SRC | DST):
			case OPCODE(~SRC | ~DST):
			case OPCODE(DST & ~SRC):
			case OPCODE(~DST & ~SRC):
			case OPCODE(~SRC ^ DST):	/* invert op, use BG color */
				color = GETBCOLOR(op);
				op = rop_invert(op);
				break;
			default:							/* use FG color */
				color =  GETFCOLOR(op);
				break;
			}
%if LOGBITS > 3
		color = color | (color<<8);
%endif
%if LOGBITS > 4
		color = color | (color<<16);
%endif
		}

	if (depth==1) {
		/* Without a src_map, choose an opcode which ignores
		 * the src, by assuming it to be all foreground.
		 */
		if (!src_map)
			op = PUTOP(nsrc[OPCODE(op)], op);
	}

	if (!src_map)
		mode |= SRC_COLOR;
	/* in monochrome case, the src color is unset, but ignored by the op */

/* clipping and argument checking */

%ifndef NOCLIP
	
	/* clip width and height */

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

	/* clip destination */

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

	/* clip source */

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

%endif

	x_dst += dst_map->x0;
	y_dst += dst_map->y0;

	/* convert from pixels to bits */

	wide *= depth;	
	x_dst *= depth;

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

		/* convert source to internal form as needed */

		if (DOFLIP && src_map->primary->type&_FLIP) {
			flip(src_map->data,BIT_LINE(src_map)*src_map->primary->high);
			src_map->primary->type &= 3;
			}

		x_src += src_map->x0;
		x_src *= depth;
		y_src += src_map->y0;
		src = src_map->data + BIT_LINE(src_map)*y_src + (x_src>>LOGBITS);
		s_skip = BIT_LINE(src_map);

		/* compute source and dest bit offsets */

		lshift = (x_src&BITS) - (x_dst&BITS);
		if (lshift > 0) {
			rshift = (BITS+1) - lshift;
			}
		else  if (lshift < 0){
			rshift = -lshift;
			lshift = (BITS+1) - rshift;
			src--;
			}
		else {
			mode |= ALIGNED;
			}

		/* set blit direction */

		if (src_map->primary->data == dst_map->primary->data)
			if (y_dst > y_src || (y_dst == y_src && x_dst > x_src))
				mode |= UPLEFT;

		}		/* end if (source_map) */

	if (words <=1 ) {
		mode |= SMALL;
		lmask &= rmask;
		}
	if(mode & (SRC_COLOR|ALIGNED))	/* only these cases implemented */
		if(lmask==(DATA)~0 && rmask==(DATA)~0)
			mode |= NOMASK;

	/* do the bitblt.  For each direction, adjust the initial condisions
	 * and choose the proper templates.  For machines where "x << (bits-in-x)"
	 * yields zero, the ALIGNED cases may be omitted
	 */

/*
	dprintf(stderr,"DIR(%x) %dx%d %d,%d->%d,%d (0x%x->0x%x)\n",
			mode,count,wide,x_src,y_src,x_dst,y_dst,src,dst);
*/

   switch(mode) {

	case SRC_COLOR:				/* no source map, use color */
		d_skip -= words;
		SWITCH(op,Rop_C);
      break;

	case SRC_COLOR|NOMASK:			/* no source map, use color */
		d_skip -= words;
		SWITCH(op,Rop_C_nm);
      break;

	case SRC_COLOR|SMALL:			/* no source map, use color (small) */
		SWITCH(op,Rop_sC);
      break;

	case SRC_COLOR|SMALL|NOMASK:		/* no source map, use color (small) */
		SWITCH(op,Rop_sC_nm);
      break;

	case DOWNRIGHT:				/* top->bottom		left->right */
		d_skip -= words - 1;
		s_skip -= words;
		SWITCH(op,Rop_incr_m);
      break;

	case DOWNRIGHT|SMALL:			/* small */
	case UPLEFT|SMALL:
		if( (mode&UPLEFT)) {
			dst += d_skip*(count-1);
			d_skip = - d_skip;
			src += s_skip*(count-1);
			s_skip = - s_skip;
		}
		s_skip -= 1;
		SWITCH(op,Rop_s);
      break;

	case DOWNRIGHT|ALIGNED:			/* top->bottom		left->right */
		d_skip -= words;
		s_skip -= words;
		SWITCH(op,Rop_incr_a);
      break;

	case DOWNRIGHT|ALIGNED|NOMASK:		/* top->bottom		left->right */
		d_skip -= words;
		s_skip -= words;
		SWITCH(op,Rop_incr_a_nm);
      break;

	case DOWNRIGHT|SMALL|ALIGNED:		/* top->bottom		small */
	case UPLEFT|SMALL|ALIGNED:
		if ((mode&UPLEFT)) {
			dst += d_skip*(count-1);
			d_skip = - d_skip;
			src += s_skip*(count-1);
			s_skip = - s_skip;
		}
		SWITCH(op,Rop_sa);
      break;

	case DOWNRIGHT|SMALL|ALIGNED|NOMASK:	/* small, aligned, nomask */
	case UPLEFT|SMALL|ALIGNED|NOMASK:
		if ((mode&UPLEFT)) {
			dst += d_skip*(count-1);
			d_skip = - d_skip;
			src += s_skip*(count-1);
			s_skip = - s_skip;
		}
		SWITCH(op,Rop_sa_nm);
      break;

	case UPLEFT:				/* bottom->top		right->left */
		dst += d_skip*(count-1) + words - 1;
		d_skip = - d_skip + words - 1;
		src += s_skip*(count-1) + words;
		s_skip = - s_skip + words - 1;
		SWITCH(op,Rop_decr_m);
      break;

	case UPLEFT|ALIGNED:			/* bottom->top		right->left */
		dprintf(stderr,"Hit UPLEFT|ALIGNED case\n");
		dst += d_skip*(count-1) + words - 1;
		d_skip = - d_skip + words;
		/* on the next line Uhler had count, not count-1 */
		src += s_skip*(count-1) + words - 1;
		s_skip = - s_skip + words;
		SWITCH(op,Rop_decr_a);
      break;

	case UPLEFT|ALIGNED|NOMASK:		/* bottom->top		right->left */
		dprintf(stderr,"Hit UPLEFT|ALIGNED|NOMASK case\n");
		dst += d_skip*(count-1) + words - 1;
		d_skip = - d_skip + words;
		/* on the next line Uhler had count, not count-1 */
		src += s_skip*(count-1) + words - 1;
		s_skip = - s_skip + words;
		SWITCH(op,Rop_decr_a_nm);
      break;

	default:
		dprintf(stderr,"Invalid direction: 0x%x\n",mode);
		break;
		}
	return(0);
}
