/*                        Copyright (c) 1988 Bellcore
 *                            All Rights Reserved
 *       Permission is granted to copy or use this program, EXCEPT that it
 *       may not be sold for profit, the copyright notice must be reproduced
 *       on copies, and credit should be given to Bellcore where it is due.
 *       BELLCORE MAKES NO WARRANTY AND ACCEPTS NO LIABILITY FOR THIS PROGRAM.
 */
/*	$Header: blit.C,v 4.1 88/06/21 13:19:02 bianchi Exp $
	$Source: /tmp/mgrsrc/src/blit/RCS/blit.C,v $
*/
static char	RCSid_[] = "$Source: /tmp/mgrsrc/src/blit/RCS/blit.C,v $$Revision: 4.1 $";

/* bitblit code for 68020's */

#include <stdio.h>
#include "bitmap.h"
#include "asm.h"

#define UP		0x10	/* direction bottom up */
#define LEFT	0x20	/* direction right to left */
#define SMALL	0x40	/* > 32 bits */

#define NSRC	1		/* no source required */

/* multiword source <op> destination, long aligned on destination */

#define ROP_TOP(name) \
		GOTO(LL4!name); \
	LABEL(LL1!name)

#define ROP_BOTTOM(how,name) \
		how($d_skip,$d_base); \
		how($s_skip,$s_offset); \
	LABEL(LL4!name); \
		LOOP($count,LL1!name)

/* bits at left edge of long boundary */

#define ROP_LEFT(how,op1,op2) \
		MOVE($l_width,$i); \
		MOVE($l_width,$T_DST); \
		NEG($T_DST); \
		BF_EXT($T_DST,$d_base,$T_DST,$i); \
		BF_EXT($T_SRC,$s_base,$s_offset,$i); \
		op1($T_DST,$T_SRC); \
		op2($T_DST,$T_SRC); \
		MOVE($l_width,$T_DST); \
		NEG($T_DST); \
		BF_INS($T_SRC,$d_base,$T_DST,$i)

/* full words in middle Left to Right */

#define ROP_LR(how,op1,op2,name) \
		ADD($i,$s_offset); \
		MOVE($words,$i); \
		GOTO(LL3!name); \
	LABEL(LL2!name); \
		BF_EXT($T_SRC,$s_base,$s_offset,IMM(0)); \
		MOVE(IND($d_base),$T_DST); \
		op1($T_DST,$T_SRC); \
		op2($T_DST,$T_SRC); \
		MOVE($T_SRC,INCR($d_base)); \
		ADD(IMM(32),$s_offset); \
	LABEL(LL3!name); \
		LOOP($i,LL2!name)

/* full words in middle Right to Left */

#define ROP_RL(how,op1,op2,name) \
		MOVE($words,$i); \
		GOTO(LL3!name); \
	LABEL(LL2!name); \
		SUB(IMM(32),$s_offset); \
		BF_EXT($T_SRC,$s_base,$s_offset,IMM(0)); \
		MOVE(DECR($d_base),$T_DST); \
		op1($T_DST,$T_SRC); \
		op2($T_DST,$T_SRC); \
		MOVE($T_SRC,IND($d_base)); \
	LABEL(LL3!name); \
		LOOP($i,LL2!name); \
		MOVE($l_width,$i); \
		SUB($i,$s_offset);

/* bits at right edge of long boundary */

#define ROP_RIGHT(how,op1,op2) \
		MOVE($r_width,$i); \
		BF_EXT($T_SRC,$s_base,$s_offset,$i); \
		BF_EXT($T_DST,$d_base,IMM(0),$i); \
		op1($T_DST,$T_SRC); \
		op2($T_DST,$T_SRC); \
		BF_INS($T_SRC,$d_base,IMM(0),$i);

/* multiword set/clear/invert - top half */

#define ROP1(how,name) \
            GOTO(L4!name); \
         LABEL(L1!name);\
        		MOVE($l_width,$i); \
        		MOVE($l_width,$T_DST); \
        		NEG($T_DST); \
        		how($d_base,$T_DST,$i);\
			   MOVE($words,$i);\
			   GOTO(L3!name);\
         LABEL(L2!name)

/* multiword set/clear/invert - bottom half */

#define ROP2(how,name) \
         LABEL(L3!name); \
			   LOOP($i,L2!name); \
            MOVE($r_width,$i); \
            how($d_base,IMM(0),$i);\
			   ADD($d_skip,$d_base); \
         LABEL(L4!name);\
            LOOP($count,L1!name)

/* (<=32 bits) no source, used for SET, CLEAR and INVERT */

#define NO_SRC(func)			\
		  GOTO(L2!func);						\
		LABEL(L1!func);						\
		  func($d_base,$words,$i);	\
		  ADD($d_skip,$words);			\
		LABEL(L2!func);			\
		  LOOP($count,L1!func)

/*
 *  (<=32 bits) DST doesn't count. Use for SRC, ~SRC.
 *  how is ADD for top down, SUB for bottom up
 */

#define NO_DST(how,op)	\
		  GOTO(L2!op!how); \
		LABEL(L1!op!how); \
		  BF_EXT($T_SRC,$s_base,$s_offset,$i); \
		  op($T_DST,$T_SRC); \
		  BF_INS($T_SRC,$d_base,$words,$i); \
		  how($d_skip,$words); \
		  how($s_skip,$s_offset); \
		LABEL(L2!op!how); \
		  LOOP($count,L1!op!how)

/*
 * (<=32 bits) both SRC and DST count
 *  how is ADD for top down, SUB for bottom up
 */

#define ROP(how,op1,op2)	\
		  GOTO(L2!op1!op2!how); \
		LABEL(L1!op1!op2!how); \
		  BF_EXT($T_SRC,$s_base,$s_offset,$i); \
		  BF_EXT($T_DST,$d_base,$words,$i); \
		  op1($T_DST,$T_SRC); \
		  op2($T_DST,$T_SRC); \
		  BF_INS($T_SRC,$d_base,$words,$i); \
		  how($d_skip,$words); \
		  how($s_skip,$s_offset); \
		LABEL(L2!op1!op2!how); \
		  LOOP($count,L1!op1!op2!how)

/* generic case >32 bits */

#define ROP_CASE(op,name,op1,op2) \
      case GET_OP(op):					/* name  left->right top->bottom */\
         ROP_TOP(name!td_lr);\
         ROP_LEFT(ADD,op1,op2);\
         ROP_LR(ADD,op1,op2,name!td_lr);\
         ROP_RIGHT(ADD,op1,op2);\
         ROP_BOTTOM(ADD,name!td_lr);\
         break;\
      case GET_OP(op) | UP:			/* name left->right  bottom->up */\
         ROP_TOP(name!bu_lr);\
         ROP_LEFT(SUB,op1,op2);\
         ROP_LR(SUB,op1,op2,name!bu_lr);\
         ROP_RIGHT(SUB,op1,op2);\
         ROP_BOTTOM(SUB,name!bu_lr);\
         break;\
      case GET_OP(op) | LEFT:			/* name right->left  top->bottom */\
         ROP_TOP(name!td_rl);\
         ROP_RIGHT(ADD,op1,op2);\
         ROP_RL(ADD,op1,op2,name!td_rl);\
         ROP_LEFT(ADD,op1,op2);\
         ROP_BOTTOM(ADD,name!td_rl);\
         break;\
      case GET_OP(op) | UP | LEFT:	/* name right->left  bottom->up */\
         ROP_TOP(name!bu_rl);\
         ROP_RIGHT(SUB,op1,op2);\
         ROP_RL(SUB,op1,op2,name!bu_rl);\
         ROP_LEFT(SUB,op1,op2);\
         ROP_BOTTOM(SUB,name!bu_rl);\
         break

			/* generic case <= 32 bits */

#define SMALL_CASE(op,op1,op2) \
		case GET_OP(op) | SMALL:\
			i = wide; \
			ROP(ADD,op1,op2);\
			break;\
		case GET_OP(op) | SMALL | UP:\
			i = wide; \
			ROP(SUB,op1,op2);\
			break

#define dprintf if(Bdebug)fprintf
int Bdebug = 0;

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

mem_rop(dst,x_dst,y_dst,wide,high,op,src,x_src,y_src)
BITMAP *dst;				/* bitmaps */
BITMAP *src;				/* bitmaps */
int x_dst,y_dst;			/* destination coords */
int x_src,y_src;			/* source coords */
int wide,high;				/* bitmap size */
int op;						/* bitmap function */
   {
	/* address registers */

	register int *d_base;			/* destination bitmap base addresses */
	register int *s_base;			/* source bitmap base addresses */

	/* data registers */

	register int s_offset;			/* source bit offset */
	register int d_skip;				/* bytes to next line in dst */
	register int s_skip;				/* bits to next line in src */
   register int i = 0;				/* temporary data register */
											/* width in bits (<= 32 bits) */
	register int count=high;		/* # of rows */
   register int words;				/* words across in inner loop (>32 bits) */
											/* dest bit offset (<= 32 bits) */

	/* temporary address reg. storage for 'i' above (>32 bits only) */

   register int *l_width;			/* bits on left to next long boundary */
   register int *r_width;			/* bits from last long boundary on right */

	/* clipping and argument checking */

	if (!src) {
      if (Bdebug && op&0xf != 0xf&nsrc[op&0xf])
		   dprintf(stderr,"no src, setting op %d -> %d\n",op&0xf,nsrc[op&0xf]);
      op = 0xf&nsrc[op&0xf];						/* a NULL src sources 1's */
		}

	else if (zsrc[op&0xf])					 {
      if (Bdebug && src)
		   dprintf(stderr,"op=%d, setting src->NULL\n",op&0xf);
		src == BIT_NULL;							/* don't check no src cases */
		}
	
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
		if (src)
			x_src -= x_dst;
		wide += x_dst;
		x_dst = 0;
		}

   if (y_dst < 0) {
		dprintf(stderr,"Clip: y_dst<0 (%d)\n",y_dst);
		if (src)
			y_src -= y_dst;
		count += y_dst;
		y_dst = 0;
		}

	if (src) {
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
			
		if ((i = x_src+wide - src->wide) > 0) {
			dprintf(stderr,"Clip: wide too big for src (%d->%d)\n",wide,wide-i);
			wide -= i;
			}

		if ((i = y_src+count - src->high) > 0) {
			dprintf(stderr,"Clip: high too big for src (%d->%d)\n",count,count-i);
			count -= i;
			}

		x_src += src->x0;
		y_src += src->y0;
		}

	if ((i = x_dst + wide - dst->wide) > 0) {
		dprintf(stderr,"Clip: wide too big for dst (%d->%d)\n",wide,wide-i);
		wide -= i;
		}
	if ((i = y_dst + count - dst->high) > 0) {
		dprintf(stderr,"Clip: high too big for dst (%d->%d)\n",count,count-i);
		count -= i;
		}

	if (wide<1 || count < 1) {
		dprintf(stderr,"Clip: high or wide < 1 (%d,%d)\n",wide,count);
		return(-1);
		}
	x_dst += dst->x0;
	y_dst += dst->y0;


	/* end of clipping code */

   /* set up common initial conditions */

	if (wide <= 32) {			/* small cases */
		i = SMALL;
		}
	else {
		i = 0;
		l_width = (int *) (32 - (x_dst&31));	/* bits on left edge 1-32 */
		r_width = (int *) ((x_dst + wide) & 31);	/* bits on right edge 0-31 */
		words =   (wide - (int) l_width)>>5;	/* longs in middle */
		if (!r_width) {								/* change 0-31 to 1-32 */
			r_width = (int *) 32;
			words--;
			}
		}

   /* find bitblt direction */

   if (src && src->data == dst->data)  {
      if (y_dst>y_src)
         i |= UP;
      if (x_dst>x_src && wide > 32)
		   i |= LEFT;
		}
       
	/* set initial conditions */

   switch(i) {
	case 0:				/* top->bottom		left->right */
      d_base = dst->data +((BIT_LINE(dst)*y_dst+x_dst+32)>>5);
   	d_skip = (BIT_LINE(dst)>>3) - (words<<2);		/* bytes to next row */
		if (src) {
			s_base = src->data;
			s_skip = BIT_LINE(src) - (wide - (int)r_width);		/* in bits */
			s_offset = BIT_LINE(src) * y_src + x_src;				/* in bits */
			}
		/* dprintf(stderr,"RIGHT DOWN\n"); */
      break;

	case UP:				/* bottom->top		left->right */
      d_base = dst->data +((BIT_LINE(dst)*(y_dst+count-1)+x_dst+32)>>5);
   	d_skip = (BIT_LINE(dst)>>3) + (words<<2);		/* bytes to next row */
		s_base = src->data;
		s_skip = BIT_LINE(src) + (wide - (int)r_width);		/* in bits */
		s_offset = BIT_LINE(src) * (y_src+count-1) + x_src;
		/* dprintf(stderr,"RIGHT UP\n"); */
      break;

	case LEFT:			/* top->bottom		right->left */
      d_base = dst->data +((BIT_LINE(dst)*y_dst+x_dst+wide-1)>>5);
   	d_skip = (BIT_LINE(dst)>>3) + (words<<2);		/* bytes to next row */
		s_base = src->data;
		s_skip = BIT_LINE(src) + (words<<5) + (int) l_width;
		s_offset = BIT_LINE(src)*y_src + x_src+wide-(int)r_width;
		/* dprintf(stderr,"LEFT DOWN\n"); */
      break;

	case UP|LEFT:		/* bottom->top		right->left */
      d_base = dst->data +((BIT_LINE(dst)*(y_dst+count-1)+x_dst+wide-1)>>5);
   	d_skip = (BIT_LINE(dst)>>3) - (words<<2);		/* bytes to next row */
		s_base = src->data;
		s_skip = BIT_LINE(src) - (words<<5) - (int) l_width;
		s_offset = BIT_LINE(src)*(y_src+count-1) + x_src+wide-(int)r_width;
		/* dprintf(stderr,"LEFT UP\n"); */
      break;

	case SMALL:			/* <= 32 bits */
		d_base =  dst->data;	/* destination base address */
		d_skip = BIT_LINE(dst->primary);	/* bits/row */
		words = d_skip*y_dst+x_dst;
		if (src) {
			s_base = src->data; /* source base address */
			s_skip = BIT_LINE(src->primary);	/* bits/row */
			s_offset = s_skip*y_src+x_src;
			}
		/* dprintf(stderr,"SMALL DOWN\n"); */
      break;

	case SMALL | UP:
		d_base =  dst->data;	/* destination base address */
		d_skip = BIT_WIDE(dst->primary);	/* bits/row */
		words = d_skip*(y_dst+count-1)+x_dst;
		s_base = (int *) src->data; /* source base address */
		s_skip = BIT_WIDE(src->primary);	/* bits/row */
		s_offset = s_skip*(y_src+count-1)+x_src;
		/* dprintf(stderr,"SMALL UP\n"); */
      break;

	default:
		dprintf(stderr,"Invalid direction: 0x%x\n",i);
		break;
		}

/*
   dprintf(stderr,"op:%d d_base:0x%x\td_skip:%d\	(%d %d*32 %d)\n",
						op&0xf,d_base,d_skip,(int)l_width,words,(int)r_width);					
   if (src)
		dprintf(stderr,"\ts_base:0x%x\ts_skip:%d\ts_offset:%d\n",
						s_base,s_skip,s_offset);					

	dprintf(stderr,"go %x\n",i);
*/

	/* @+		DON't DISTURB THIS COMMENT */

	switch(op&0xf | i) {

		/* no source involvement <= 32 bits */

		case GET_OP(BIT_SET) | SMALL:
		case GET_OP(BIT_SET) | UP | SMALL:
			i = wide;
			NO_SRC(BF_SET);
			break;	
		case GET_OP(BIT_CLR) | SMALL:
		case GET_OP(BIT_CLR) | UP | SMALL:
			i = wide;
			NO_SRC(BF_CLR);
			break;	
		case GET_OP(BIT_NOT(BIT_DST)) | SMALL:
		case GET_OP(BIT_NOT(BIT_DST)) | UP | SMALL:
			i = wide;
			NO_SRC(BF_INV);
			break;	

		/* no dest involement */

		case GET_OP(BIT_SRC) | SMALL:
			i = wide;
			NO_DST(ADD,NOP);
			break;
		case GET_OP(~BIT_SRC) | SMALL:
			i = wide;
			NO_DST(ADD,NOT_SRC);
			break;

		case GET_OP(BIT_SRC) | UP | SMALL:
			i = wide;
			NO_DST(SUB,NOP);
			break;
		case GET_OP(~BIT_SRC) | UP | SMALL:
			i = wide;
			NO_DST(SUB,NOT_SRC);
			break;

		/* source and dest  and cases */

		SMALL_CASE(BIT_SRC&BIT_DST,AND,NOP);
		SMALL_CASE(~(BIT_SRC&BIT_DST),AND,NOT_SRC);
		SMALL_CASE(~BIT_SRC&BIT_DST,NOT_SRC,AND);
		SMALL_CASE(BIT_SRC&~BIT_DST,NOT_DST,AND);

		/* source and dest  or cases */

		SMALL_CASE(BIT_SRC|BIT_DST,OR,NOP);
		SMALL_CASE(~(BIT_SRC|BIT_DST),OR,NOT_SRC);
		SMALL_CASE(~BIT_SRC|BIT_DST,NOT_SRC,OR);
		SMALL_CASE(BIT_SRC|~BIT_DST,NOT_DST,OR);

		/* source and dest  xor cases */

		SMALL_CASE(BIT_SRC^BIT_DST,XOR,NOP);
		SMALL_CASE(~(BIT_SRC^BIT_DST),XOR,NOT_SRC);


		/****************************************************************
		 * > 32 bits
		 */

		/* no source involvement */

		case GET_OP(BIT_SET):
		case GET_OP(BIT_SET) | UP:
		case GET_OP(BIT_SET) | LEFT:
		case GET_OP(BIT_SET) | UP | LEFT:
			MOVEQ(0,$T_SRC);
         NOT($T_SRC);
	      ROP1(BF_SET,set);
         MOVE($T_SRC,INCR($d_base));
		   ROP2(BF_SET,set);
			break;	

		case GET_OP(BIT_CLR):
		case GET_OP(BIT_CLR) | UP:
		case GET_OP(BIT_CLR) | LEFT:
		case GET_OP(BIT_CLR) | UP | LEFT:
			MOVEQ(0,$T_SRC);
	      ROP1(BF_CLR,clear);
         MOVE($T_SRC,INCR($d_base));
	      ROP2(BF_CLR,clear);
			break;	

		case GET_OP(~BIT_DST):
		case GET_OP(~BIT_DST) | UP:
		case GET_OP(~BIT_DST) | LEFT:
		case GET_OP(~BIT_DST) | UP | LEFT:
	      ROP1(BF_INV,invert);
		   MOVE(IND($d_base),$T_SRC);
	      NOT($T_SRC);
         MOVE($T_SRC,INCR($d_base));
	      ROP2(BF_INV,invert);
			break;	

		/* source involvement, no DST  (this could be better) */
      /* The optimizer doesn't toss the un-needed loads of the destination */

		ROP_CASE(BIT_SRC,src,NOP,NOP);
		ROP_CASE(~BIT_SRC,not_src,NOP,NOT_SRC);

		/* source involvement  - and operations */

		ROP_CASE(BIT_SRC&BIT_DST,and,AND,NOP);
		ROP_CASE(~(BIT_SRC&BIT_DST),not_and,AND,NOT_SRC);
		ROP_CASE(~BIT_SRC&BIT_DST,mask,NOT_SRC,AND);
		ROP_CASE(BIT_SRC&~BIT_DST,not_mask,NOT_DST,AND);

		/* source involvement  - or operations */

		ROP_CASE(BIT_SRC|BIT_DST,or,OR,NOP);
		ROP_CASE(~(BIT_SRC|BIT_DST),not_or,OR,NOT_SRC);
		ROP_CASE(~BIT_SRC|BIT_DST,project,NOT_SRC,OR);
		ROP_CASE(BIT_SRC|~BIT_DST,not_project,NOT_DST,OR);

		/* source involvement  - xor operations */

		ROP_CASE(BIT_SRC^BIT_DST,xor,XOR,NOP);
		ROP_CASE(~(BIT_SRC^BIT_DST),not_xor,XOR,NOT_SRC);

		/* no-op cases */

		case GET_OP(DST) :	
		case GET_OP(DST) | LEFT:
		case GET_OP(DST) | SMALL:
		case GET_OP(DST) | UP | SMALL:
			break;

      default:									/* not implemented */
         fprintf(stderr,"operation 0x%x not implemented\n",op);
         break;
		}
	return(0);
  	}
