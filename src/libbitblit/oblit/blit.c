/*                        Copyright (c) 1987 Bellcore
 *                            All Rights Reserved
 *       Permission is granted to copy or use this program, EXCEPT that it
 *       may not be sold for profit, the copyright notice must be reproduced
 *       on copies, and credit should be given to Bellcore where it is due.
 *       BELLCORE MAKES NO WARRANTY AND ACCEPTS NO LIABILITY FOR THIS PROGRAM.
 */
/*	$Header: /files/src/linuxmgr/src/libbitblit/oblit/RCS/blit.c,v 4.3 1994/01/28 20:59:34 broman Stab $
	$Source: /files/src/linuxmgr/src/libbitblit/oblit/RCS/blit.c,v $
*/
static char	RCSid_[] = "$Source: /files/src/linuxmgr/src/libbitblit/oblit/RCS/blit.c,v $$Revision: 4.3 $";

/*  SUN-2 bitblit code */

#include "bitmap.h"

/*
 * your standard unrolled-loop (I don't think this buys much)
 */

#define LOOP(n,s) {				\
    register int cnt;				\
    for (cnt=(n); cnt>=16; cnt-=16) {		\
       s;s;s;s;s;s;s;s;s;s;s;s;s;s;s;s;		\
       }					\
    switch (cnt) {				\
       case 15: s; case 14: s; case 13: s; case 12: s;	\
       case 11: s; case 10: s; case  9: s; case  8: s;	\
       case  7: s; case  6: s; case  5: s; case  4: s;	\
       case  3: s; case  2: s; case  1: s; 		\
       }						\
    }

/*
 * A funny unrolled-loop: even and odd cases differ
 */

#define LOOP2(n,s1,s2) { 				\
    register int cnt;					\
    for (cnt=(n); cnt>=16; cnt-=16)	{		\
       s1;s2;s1;s2;s1;s2;s1;s2;s1;s2;s1;s2;s1;s2;s1;s2;	\
       }						\
    switch (cnt) {					\
       case 15: s1;s2; case 13: s1;s2; case 11: s1;s2; 	\
       case  9: s1;s2; case  7: s1;s2; case  5: s1;s2;	\
       case  3: s1;s2; case  1: s1;			\
          break;					\
       case 14: s1;s2; case 12: s1;s2; case 10: s1;s2;	\
       case  8: s1;s2; case  6: s1;s2; case  4: s1;s2;	\
       case  2: s1;s2; case  0: src=srcx;		\
       }						\
    }

#define lsrc	((unsigned long *)src)
#define lsrcx	((unsigned long *)srcx)

/*
 *  General memory-to-memory rasterop
 */

mem_rop(dest, dx, dy, width, height, func, source, sx, sy)
int sx, sy, dx, dy;		/* properly clipped source and dest */
int width, height;		/* rectangle to be transferred */
BITMAP *source, *dest;		/* bit map pointers */
int func;			/* rasterop function */

{
   int dwwidth = BIT_LINE(dest);
   int swwidth = 0;
   int xs, ys;
   int xd, yd;
   unsigned short *sbase = (unsigned short *) 0;
   unsigned short *dbase = (dest->data);

   /* clipping, should already be done ? */

   {
      register int t;

      if (width < 0)
	 dx += width, width = -width;
      if (height < 0)
	 dy += height, height = -height;
      if (dx < 0) {
	 if (source)
	    sx -= dx;
	 width += dx, dx = 0;
      }

      if (dy < 0) {
	 if (source)
	    sy -= dy;
	 height += dy, dy = 0;
      }

      if (source) {
	 if (sx < 0)
	    dx -= sx, width += sx, sx = 0;
	 if (sy < 0)
	    dy -= sy, height += sy, sy = 0;
	 if ((t = sx + width - source->wide) > 0)
	    width -= t;
	 if ((t = sy + height - source->high) > 0)
	    height -= t;

	 swwidth = BIT_LINE(source);
	 xs = sx + source->x0, ys = sy + source->y0;
	 sbase = (source->data);
      }

      if ((t = dx + width - dest->wide) > 0)
	 width -= t;
      if ((t = dy + height - dest->high) > 0)
	 height -= t;

      if (width < 1 || height < 1)
	 return;
   }

   /*********/

   xd = dx + dest->x0, yd = dy + dest->y0;
   func = OPCODE(func);

   if (!source) {		/* no source bitmap */

      register unsigned long *dst =
      (unsigned long *) (dbase + yd * dwwidth + (xd >> 4));
      register unsigned int mask1 =
      ((unsigned long) 0xFFFFFFFF) >> (xd & 15);
      register unsigned int h_cnt =
      ((xd + width - 1) - ((xd & ~15)) >> 5) + 1;
      register unsigned int mask2 =
      0xFFFFFFFF << (31 - (((xd + width - 1) - (xd & ~15)) & 31));
      register unsigned int d_incr =
      (dwwidth << 1) - (h_cnt << 2);
      register unsigned int v_cnt;

      if (h_cnt > 1)		/* multi-line bitblit */
	 switch (func) {	/* no source */
	    case OPCODE(0):
	    case OPCODE(~(DST | SRC)):
	    case OPCODE(DST & ~SRC):
	    case OPCODE(~SRC):	/* no source multi-word blit */
	       for (v_cnt = height; v_cnt > 0; v_cnt--) {
		  *dst++ = *dst & ~mask1;
		  LOOP((h_cnt) - 2, *dst++ = 0);
		  *dst++ = *dst & ~mask2;
		  (int) dst += d_incr;
	       }

	       break;
	    case OPCODE(~DST & SRC):
	    case OPCODE(~DST):
	    case OPCODE(DST ^ SRC):
	    case OPCODE(~(DST & SRC)):	/* no source multi-word blit */
	       for (v_cnt = height; v_cnt > 0; v_cnt--) {
		  *dst++ = *dst ^ mask1;
		  LOOP((h_cnt) - 2, *dst++ = ~*dst);
		  *dst++ = *dst ^ mask2;
		  (int) dst += d_incr;
	       }

	       break;
	    case OPCODE(SRC):
	    case OPCODE(~DST | SRC):
	    case OPCODE(DST | SRC):
	    case OPCODE(~0):	/* no source multi-word blit */
	       for (v_cnt = height; v_cnt > 0; v_cnt--) {
		  *dst++ = ((*dst) | (mask1));
		  LOOP((h_cnt) - 2, *dst++ = ~0);
		  *dst++ = ((*dst) | (mask2));
		  (int) dst += d_incr;
	       }

	       break;
	 }

      else {			/* single line bit-blit */
	 mask1 &= mask2;
	 switch (func) {	/* no source */
	    case OPCODE(0):
	    case OPCODE(~(DST | SRC)):
	    case OPCODE(DST & ~SRC):
	    case OPCODE(~SRC):	/* no source single-word blit */
	       for (v_cnt = height; v_cnt > 0; v_cnt--) {
		  *dst++ = *dst & ~mask1;
		  (int) dst += d_incr;
	       }

	       break;
	    case OPCODE(~DST & SRC):
	    case OPCODE(~DST):
	    case OPCODE(DST ^ SRC):
	    case OPCODE(~(DST & SRC)):	/* no source single-word blit */
	       for (v_cnt = height; v_cnt > 0; v_cnt--) {
		  *dst++ = *dst ^ mask1;
		  (int) dst += d_incr;
	       }

	       break;
	    case OPCODE(SRC):
	    case OPCODE(~DST | SRC):
	    case OPCODE(DST | SRC):
	    case OPCODE(~0):	/* no source single-word blit */
	       for (v_cnt = height; v_cnt > 0; v_cnt--) {
		  *dst++ = ((*dst) | (mask1));
		  (int) dst += d_incr;
	       }

	       break;
	 }

      }

   }

   /* source (op) dest bitmap */

   else {
      unsigned int lmask =
      0xFFFF >> (xd & 15);	/* mask for left edge */
      unsigned int h_cnt =
      ((xd + width - 1) >> 4) - (xd >> 4) + 1;	/* shorts in dest */
      unsigned int rmask =
      0xFFFF << (15 - ((xd + width - 1) & 15));	/* mask for right edge */

      register unsigned short *src =
      sbase + ys * swwidth + (xs >> 4);	/* source */
      register unsigned short *srcx;	/* source alternate */
      register unsigned short *dst =
      dbase + yd * dwwidth + (xd >> 4);	/* destination */
      register unsigned int mask1, mask2;	/* masks */
      register unsigned int shift;	/* shift count */
      int s_incr, d_incr;	/* how to get line to line */
      int v_cnt;		/* number of lines */

      if ((xs & 15) > (xd & 15))
	 shift = 16 - (xs & 15) + (xd & 15);
      else
	 shift = (xd & 15) - (xs & 15), src -= 1;

      if (yd < ys)
	 s_incr = swwidth, d_incr = dwwidth;	/* top to bottom */
      else
	 s_incr = -swwidth, d_incr = -dwwidth,
	    src += (height - 1) * swwidth,
	    dst += (height - 1) * dwwidth;	/* bottom to top */

      if (xd < xs) {		/* left to right */
	 s_incr -= h_cnt + 1, d_incr -= h_cnt;
	 if (h_cnt > 1) {
	    mask1 = lmask, mask2 = rmask;
	    {
	       switch (func) {
		  case OPCODE(~(DST | SRC)):	/* left->right multi-word blit */
		     for (v_cnt = height; v_cnt > 0; v_cnt--) {
			srcx = src;
			srcx++;
			*dst++ = ~(*dst | *lsrc++ >> shift) & mask1 | *dst & ~mask1;
			LOOP2((h_cnt) - 2, *dst++ = ~(*dst | *lsrcx++ >> shift), *dst++ = ~(*dst | *lsrc++ >> shift));
			*dst++ = ~(*dst | *lsrc++ >> shift) & mask2 | *dst & ~mask2;
			src += s_incr;
			dst += d_incr;
		     }

		     break;
		  case OPCODE(DST & ~SRC):	/* left->right multi-word blit */
		     for (v_cnt = height; v_cnt > 0; v_cnt--) {
			srcx = src;
			srcx++;
			*dst++ = *dst & ~*lsrc++ >> shift & mask1 | *dst & ~mask1;
			LOOP2((h_cnt) - 2, *dst++ = *dst & ~*lsrcx++ >> shift, *dst++ = *dst & ~*lsrc++ >> shift);
			*dst++ = *dst & ~*lsrc++ >> shift & mask2 | *dst & ~mask2;
			src += s_incr;
			dst += d_incr;
		     }

		     break;
		  case OPCODE(~SRC):	/* left->right multi-word blit */
		     for (v_cnt = height; v_cnt > 0; v_cnt--) {
			srcx = src;
			srcx++;
			*dst++ = ~*lsrc++ >> shift & mask1 | *dst & ~mask1;
			LOOP2((h_cnt) - 2, *dst++ = ~*lsrcx++ >> shift, *dst++ = ~*lsrc++ >> shift);
			*dst++ = ~*lsrc++ >> shift & mask2 | *dst & ~mask2;
			src += s_incr;
			dst += d_incr;
		     }

		     break;
		  case OPCODE(~DST & SRC):	/* left->right multi-word blit */
		     for (v_cnt = height; v_cnt > 0; v_cnt--) {
			srcx = src;
			srcx++;
			*dst++ = ~*dst & *lsrc++ >> shift & mask1 | *dst & ~mask1;
			LOOP2((h_cnt) - 2, *dst++ = ~*dst & *lsrcx++ >> shift, *dst++ = ~*dst & *lsrc++ >> shift);
			*dst++ = ~*dst & *lsrc++ >> shift & mask2 | *dst & ~mask2;
			src += s_incr;
			dst += d_incr;
		     }

		     break;
		  case OPCODE(DST ^ SRC):	/* left->right multi-word blit */
		     for (v_cnt = height; v_cnt > 0; v_cnt--) {
			srcx = src;
			srcx++;
			*dst++ = *dst ^ *lsrc++ >> shift & mask1 | *dst & ~mask1;
			LOOP2((h_cnt) - 2, *dst++ = *dst ^ *lsrcx++ >> shift, *dst++ = *dst ^ *lsrc++ >> shift);
			*dst++ = *dst ^ *lsrc++ >> shift & mask2 | *dst & ~mask2;
			src += s_incr;
			dst += d_incr;
		     }

		     break;
		  case OPCODE(~(DST & SRC)):	/* left->right multi-word blit */
		     for (v_cnt = height; v_cnt > 0; v_cnt--) {
			srcx = src;
			srcx++;
			*dst++ = ~(*dst & *lsrc++ >> shift) & mask1 | *dst & ~mask1;
			LOOP2((h_cnt) - 2, *dst++ = ~(*dst & *lsrcx++ >> shift), *dst++ = ~(*dst & *lsrc++ >> shift));
			*dst++ = ~(*dst & *lsrc++ >> shift) & mask2 | *dst & ~mask2;
			src += s_incr;
			dst += d_incr;
		     }

		     break;
		  case OPCODE(DST & SRC):	/* left->right multi-word blit */
		     for (v_cnt = height; v_cnt > 0; v_cnt--) {
			srcx = src;
			srcx++;
			*dst++ = *dst & *lsrc++ >> shift & mask1 | *dst & ~mask1;
			LOOP2((h_cnt) - 2, *dst++ = *dst & *lsrcx++ >> shift, *dst++ = *dst & *lsrc++ >> shift);
			*dst++ = *dst & *lsrc++ >> shift & mask2 | *dst & ~mask2;
			src += s_incr;
			dst += d_incr;
		     }

		     break;
		  case OPCODE(~(DST ^ SRC)):	/* left->right multi-word blit */
		     for (v_cnt = height; v_cnt > 0; v_cnt--) {
			srcx = src;
			srcx++;
			*dst++ = ~(*dst ^ *lsrc++ >> shift) & mask1 | *dst & ~mask1;
			LOOP2((h_cnt) - 2, *dst++ = ~(*dst ^ *lsrcx++ >> shift), *dst++ = ~(*dst ^ *lsrc++ >> shift));
			*dst++ = ~(*dst ^ *lsrc++ >> shift) & mask2 | *dst & ~mask2;
			src += s_incr;
			dst += d_incr;
		     }

		     break;
		  case OPCODE(DST | ~SRC):	/* left->right multi-word blit */
		     for (v_cnt = height; v_cnt > 0; v_cnt--) {
			srcx = src;
			srcx++;
			*dst++ = *dst | ~*lsrc++ >> shift & mask1 | *dst & ~mask1;
			LOOP2((h_cnt) - 2, *dst++ = *dst | ~*lsrcx++ >> shift, *dst++ = *dst | ~*lsrc++ >> shift);
			*dst++ = *dst | ~*lsrc++ >> shift & mask2 | *dst & ~mask2;
			src += s_incr;
			dst += d_incr;
		     }

		     break;
		  case OPCODE(SRC):	/* left->right multi-word blit */
		     for (v_cnt = height; v_cnt > 0; v_cnt--) {
			srcx = src;
			srcx++;
			*dst++ = *lsrc++ >> shift & mask1 | *dst & ~mask1;
			LOOP2((h_cnt) - 2, *dst++ = *lsrcx++ >> shift, *dst++ = *lsrc++ >> shift);
			*dst++ = *lsrc++ >> shift & mask2 | *dst & ~mask2;
			src += s_incr;
			dst += d_incr;
		     }

		     break;
		  case OPCODE(~DST | SRC):	/* left->right multi-word blit */
		     for (v_cnt = height; v_cnt > 0; v_cnt--) {
			srcx = src;
			srcx++;
			*dst++ = ~*dst | *lsrc++ >> shift & mask1 | *dst & ~mask1;
			LOOP2((h_cnt) - 2, *dst++ = ~*dst | *lsrcx++ >> shift, *dst++ = ~*dst | *lsrc++ >> shift);
			*dst++ = ~*dst | *lsrc++ >> shift & mask2 | *dst & ~mask2;
			src += s_incr;
			dst += d_incr;
		     }

		     break;
		  case OPCODE(DST | SRC):	/* left->right multi-word blit */
		     for (v_cnt = height; v_cnt > 0; v_cnt--) {
			srcx = src;
			srcx++;
			*dst++ = *dst | *lsrc++ >> shift & mask1 | *dst & ~mask1;
			LOOP2((h_cnt) - 2, *dst++ = *dst | *lsrcx++ >> shift, *dst++ = *dst | *lsrc++ >> shift);
			*dst++ = *dst | *lsrc++ >> shift & mask2 | *dst & ~mask2;
			src += s_incr;
			dst += d_incr;
		     }

		     break;	/* these should never be called - use no source case */
		  case OPCODE(0):	/* left->right multi-word blit */
		     for (v_cnt = height; v_cnt > 0; v_cnt--) {
			*dst++ = 0 & mask1 | *dst & ~mask1;
			LOOP2((h_cnt) - 2, *dst++ = 0, *dst++ = 0);
			*dst++ = 0 & mask2 | *dst & ~mask2;
			dst += d_incr;
		     }

		     break;
		  case OPCODE(~DST):	/* left->right multi-word blit */
		     for (v_cnt = height; v_cnt > 0; v_cnt--) {
			*dst++ = ~*dst & mask1 | *dst & ~mask1;
			LOOP2((h_cnt) - 2, *dst++ = ~*dst, *dst++ = ~*dst);
			*dst++ = ~*dst & mask2 | *dst & ~mask2;
			dst += d_incr;
		     }

		     break;
		  case OPCODE(~0):	/* left->right multi-word blit */
		     for (v_cnt = height; v_cnt > 0; v_cnt--) {
			*dst++ = ~0 & mask1 | *dst & ~mask1;
			LOOP2((h_cnt) - 2, *dst++ = ~0, *dst++ = ~0);
			*dst++ = ~0 & mask2 | *dst & ~mask2;
			dst += d_incr;
		     }

		     break;
		  case OPCODE(DST):
		     break;
	       }

	    }

	 }

	 else {
	    mask1 = lmask & rmask;
	    {
	       switch (func) {
		  case OPCODE(~(DST | SRC)):	/* left->right single-word blit */
		     for (v_cnt = height; v_cnt > 0; v_cnt--) {
			srcx = src;
			srcx++;
			*dst++ = ~(*dst | *lsrc++ >> shift) & mask1 | *dst & ~mask1;
			src += s_incr;
			dst += d_incr;
		     }

		     break;
		  case OPCODE(DST & ~SRC):	/* left->right single-word blit */
		     for (v_cnt = height; v_cnt > 0; v_cnt--) {
			srcx = src;
			srcx++;
			*dst++ = *dst & ~*lsrc++ >> shift & mask1 | *dst & ~mask1;
			src += s_incr;
			dst += d_incr;
		     }

		     break;
		  case OPCODE(~SRC):	/* left->right single-word blit */
		     for (v_cnt = height; v_cnt > 0; v_cnt--) {
			srcx = src;
			srcx++;
			*dst++ = ~*lsrc++ >> shift & mask1 | *dst & ~mask1;
			src += s_incr;
			dst += d_incr;
		     }

		     break;
		  case OPCODE(~DST & SRC):	/* left->right single-word blit */
		     for (v_cnt = height; v_cnt > 0; v_cnt--) {
			srcx = src;
			srcx++;
			*dst++ = ~*dst & *lsrc++ >> shift & mask1 | *dst & ~mask1;
			src += s_incr;
			dst += d_incr;
		     }

		     break;
		  case OPCODE(DST ^ SRC):	/* left->right single-word blit */
		     for (v_cnt = height; v_cnt > 0; v_cnt--) {
			srcx = src;
			srcx++;
			*dst++ = *dst ^ *lsrc++ >> shift & mask1 | *dst & ~mask1;
			src += s_incr;
			dst += d_incr;
		     }

		     break;
		  case OPCODE(~(DST & SRC)):	/* left->right single-word blit */
		     for (v_cnt = height; v_cnt > 0; v_cnt--) {
			srcx = src;
			srcx++;
			*dst++ = ~(*dst & *lsrc++ >> shift) & mask1 | *dst & ~mask1;
			src += s_incr;
			dst += d_incr;
		     }

		     break;
		  case OPCODE(DST & SRC):	/* left->right single-word blit */
		     for (v_cnt = height; v_cnt > 0; v_cnt--) {
			srcx = src;
			srcx++;
			*dst++ = *dst & *lsrc++ >> shift & mask1 | *dst & ~mask1;
			src += s_incr;
			dst += d_incr;
		     }

		     break;
		  case OPCODE(~(DST ^ SRC)):	/* left->right single-word blit */
		     for (v_cnt = height; v_cnt > 0; v_cnt--) {
			srcx = src;
			srcx++;
			*dst++ = ~(*dst ^ *lsrc++ >> shift) & mask1 | *dst & ~mask1;
			src += s_incr;
			dst += d_incr;
		     }

		     break;
		  case OPCODE(DST | ~SRC):	/* left->right single-word blit */
		     for (v_cnt = height; v_cnt > 0; v_cnt--) {
			srcx = src;
			srcx++;
			*dst++ = *dst | ~*lsrc++ >> shift & mask1 | *dst & ~mask1;
			src += s_incr;
			dst += d_incr;
		     }

		     break;
		  case OPCODE(SRC):	/* left->right single-word blit */
		     for (v_cnt = height; v_cnt > 0; v_cnt--) {
			srcx = src;
			srcx++;
			*dst++ = *lsrc++ >> shift & mask1 | *dst & ~mask1;
			src += s_incr;
			dst += d_incr;
		     }

		     break;
		  case OPCODE(~DST | SRC):	/* left->right single-word blit */
		     for (v_cnt = height; v_cnt > 0; v_cnt--) {
			srcx = src;
			srcx++;
			*dst++ = ~*dst | *lsrc++ >> shift & mask1 | *dst & ~mask1;
			src += s_incr;
			dst += d_incr;
		     }

		     break;
		  case OPCODE(DST | SRC):	/* left->right single-word blit */
		     for (v_cnt = height; v_cnt > 0; v_cnt--) {
			srcx = src;
			srcx++;
			*dst++ = *dst | *lsrc++ >> shift & mask1 | *dst & ~mask1;
			src += s_incr;
			dst += d_incr;
		     }

		     break;	/* these should never be called - use no source case */
		  case OPCODE(0):	/* left->right single-word blit */
		     for (v_cnt = height; v_cnt > 0; v_cnt--) {
			*dst++ = 0 & mask1 | *dst & ~mask1;
			dst += d_incr;
		     }

		     break;
		  case OPCODE(~DST):	/* left->right single-word blit */
		     for (v_cnt = height; v_cnt > 0; v_cnt--) {
			*dst++ = ~*dst & mask1 | *dst & ~mask1;
			dst += d_incr;
		     }

		     break;
		  case OPCODE(~0):	/* left->right single-word blit */
		     for (v_cnt = height; v_cnt > 0; v_cnt--) {
			*dst++ = ~0 & mask1 | *dst & ~mask1;
			dst += d_incr;
		     }

		     break;
		  case OPCODE(DST):
		     break;
	       }

	    }

	 }

      }

      else {			/* right to left */
	 s_incr += h_cnt + 1, d_incr += h_cnt;
	 src += h_cnt - 1, dst += h_cnt - 1;
	 if (h_cnt > 1) {
	    mask1 = rmask, mask2 = lmask;
	    {
	       switch (func) {
		  case OPCODE(~(DST | SRC)):	/* right->left multi-word blit */
		     for (v_cnt = height; v_cnt > 0; v_cnt--) {
			srcx = src;
			srcx--;
			*dst-- = ~(*dst | *lsrc-- >> shift) & mask1 | *dst & ~mask1;
			LOOP2((h_cnt) - 2, *dst-- = ~(*dst | *lsrcx-- >> shift), *dst-- = ~(*dst | *lsrc-- >> shift));
			*dst-- = ~(*dst | *lsrc-- >> shift) & mask2 | *dst & ~mask2;
			src += s_incr;
			dst += d_incr;
		     }

		     break;
		  case OPCODE(DST & ~SRC):	/* right->left multi-word blit */
		     for (v_cnt = height; v_cnt > 0; v_cnt--) {
			srcx = src;
			srcx--;
			*dst-- = *dst & ~*lsrc-- >> shift & mask1 | *dst & ~mask1;
			LOOP2((h_cnt) - 2, *dst-- = *dst & ~*lsrcx-- >> shift, *dst-- = *dst & ~*lsrc-- >> shift);
			*dst-- = *dst & ~*lsrc-- >> shift & mask2 | *dst & ~mask2;
			src += s_incr;
			dst += d_incr;
		     }

		     break;
		  case OPCODE(~SRC):	/* right->left multi-word blit */
		     for (v_cnt = height; v_cnt > 0; v_cnt--) {
			srcx = src;
			srcx--;
			*dst-- = ~*lsrc-- >> shift & mask1 | *dst & ~mask1;
			LOOP2((h_cnt) - 2, *dst-- = ~*lsrcx-- >> shift, *dst-- = ~*lsrc-- >> shift);
			*dst-- = ~*lsrc-- >> shift & mask2 | *dst & ~mask2;
			src += s_incr;
			dst += d_incr;
		     }

		     break;
		  case OPCODE(~DST & SRC):	/* right->left multi-word blit */
		     for (v_cnt = height; v_cnt > 0; v_cnt--) {
			srcx = src;
			srcx--;
			*dst-- = ~*dst & *lsrc-- >> shift & mask1 | *dst & ~mask1;
			LOOP2((h_cnt) - 2, *dst-- = ~*dst & *lsrcx-- >> shift, *dst-- = ~*dst & *lsrc-- >> shift);
			*dst-- = ~*dst & *lsrc-- >> shift & mask2 | *dst & ~mask2;
			src += s_incr;
			dst += d_incr;
		     }

		     break;
		  case OPCODE(DST ^ SRC):	/* right->left multi-word blit */
		     for (v_cnt = height; v_cnt > 0; v_cnt--) {
			srcx = src;
			srcx--;
			*dst-- = *dst ^ *lsrc-- >> shift & mask1 | *dst & ~mask1;
			LOOP2((h_cnt) - 2, *dst-- = *dst ^ *lsrcx-- >> shift, *dst-- = *dst ^ *lsrc-- >> shift);
			*dst-- = *dst ^ *lsrc-- >> shift & mask2 | *dst & ~mask2;
			src += s_incr;
			dst += d_incr;
		     }

		     break;
		  case OPCODE(~(DST & SRC)):	/* right->left multi-word blit */
		     for (v_cnt = height; v_cnt > 0; v_cnt--) {
			srcx = src;
			srcx--;
			*dst-- = ~(*dst & *lsrc-- >> shift) & mask1 | *dst & ~mask1;
			LOOP2((h_cnt) - 2, *dst-- = ~(*dst & *lsrcx-- >> shift), *dst-- = ~(*dst & *lsrc-- >> shift));
			*dst-- = ~(*dst & *lsrc-- >> shift) & mask2 | *dst & ~mask2;
			src += s_incr;
			dst += d_incr;
		     }

		     break;
		  case OPCODE(DST & SRC):	/* right->left multi-word blit */
		     for (v_cnt = height; v_cnt > 0; v_cnt--) {
			srcx = src;
			srcx--;
			*dst-- = *dst & *lsrc-- >> shift & mask1 | *dst & ~mask1;
			LOOP2((h_cnt) - 2, *dst-- = *dst & *lsrcx-- >> shift, *dst-- = *dst & *lsrc-- >> shift);
			*dst-- = *dst & *lsrc-- >> shift & mask2 | *dst & ~mask2;
			src += s_incr;
			dst += d_incr;
		     }

		     break;
		  case OPCODE(~(DST ^ SRC)):	/* right->left multi-word blit */
		     for (v_cnt = height; v_cnt > 0; v_cnt--) {
			srcx = src;
			srcx--;
			*dst-- = ~(*dst ^ *lsrc-- >> shift) & mask1 | *dst & ~mask1;
			LOOP2((h_cnt) - 2, *dst-- = ~(*dst ^ *lsrcx-- >> shift), *dst-- = ~(*dst ^ *lsrc-- >> shift));
			*dst-- = ~(*dst ^ *lsrc-- >> shift) & mask2 | *dst & ~mask2;
			src += s_incr;
			dst += d_incr;
		     }

		     break;
		  case OPCODE(DST | ~SRC):	/* right->left multi-word blit */
		     for (v_cnt = height; v_cnt > 0; v_cnt--) {
			srcx = src;
			srcx--;
			*dst-- = *dst | ~*lsrc-- >> shift & mask1 | *dst & ~mask1;
			LOOP2((h_cnt) - 2, *dst-- = *dst | ~*lsrcx-- >> shift, *dst-- = *dst | ~*lsrc-- >> shift);
			*dst-- = *dst | ~*lsrc-- >> shift & mask2 | *dst & ~mask2;
			src += s_incr;
			dst += d_incr;
		     }

		     break;
		  case OPCODE(SRC):	/* right->left multi-word blit */
		     for (v_cnt = height; v_cnt > 0; v_cnt--) {
			srcx = src;
			srcx--;
			*dst-- = *lsrc-- >> shift & mask1 | *dst & ~mask1;
			LOOP2((h_cnt) - 2, *dst-- = *lsrcx-- >> shift, *dst-- = *lsrc-- >> shift);
			*dst-- = *lsrc-- >> shift & mask2 | *dst & ~mask2;
			src += s_incr;
			dst += d_incr;
		     }

		     break;
		  case OPCODE(~DST | SRC):	/* right->left multi-word blit */
		     for (v_cnt = height; v_cnt > 0; v_cnt--) {
			srcx = src;
			srcx--;
			*dst-- = ~*dst | *lsrc-- >> shift & mask1 | *dst & ~mask1;
			LOOP2((h_cnt) - 2, *dst-- = ~*dst | *lsrcx-- >> shift, *dst-- = ~*dst | *lsrc-- >> shift);
			*dst-- = ~*dst | *lsrc-- >> shift & mask2 | *dst & ~mask2;
			src += s_incr;
			dst += d_incr;
		     }

		     break;
		  case OPCODE(DST | SRC):	/* right->left multi-word blit */
		     for (v_cnt = height; v_cnt > 0; v_cnt--) {
			srcx = src;
			srcx--;
			*dst-- = *dst | *lsrc-- >> shift & mask1 | *dst & ~mask1;
			LOOP2((h_cnt) - 2, *dst-- = *dst | *lsrcx-- >> shift, *dst-- = *dst | *lsrc-- >> shift);
			*dst-- = *dst | *lsrc-- >> shift & mask2 | *dst & ~mask2;
			src += s_incr;
			dst += d_incr;
		     }

		     break;	/* these should never be called - use no source case */
		  case OPCODE(0):	/* right->left multi-word blit */
		     for (v_cnt = height; v_cnt > 0; v_cnt--) {
			*dst-- = 0 & mask1 | *dst & ~mask1;
			LOOP2((h_cnt) - 2, *dst-- = 0, *dst-- = 0);
			*dst-- = 0 & mask2 | *dst & ~mask2;
			dst += d_incr;
		     }

		     break;
		  case OPCODE(~DST):	/* right->left multi-word blit */
		     for (v_cnt = height; v_cnt > 0; v_cnt--) {
			*dst-- = ~*dst & mask1 | *dst & ~mask1;
			LOOP2((h_cnt) - 2, *dst-- = ~*dst, *dst-- = ~*dst);
			*dst-- = ~*dst & mask2 | *dst & ~mask2;
			dst += d_incr;
		     }

		     break;
		  case OPCODE(~0):	/* right->left multi-word blit */
		     for (v_cnt = height; v_cnt > 0; v_cnt--) {
			*dst-- = ~0 & mask1 | *dst & ~mask1;
			LOOP2((h_cnt) - 2, *dst-- = ~0, *dst-- = ~0);
			*dst-- = ~0 & mask2 | *dst & ~mask2;
			dst += d_incr;
		     }

		     break;
		  case OPCODE(DST):
		     break;
	       }

	    }

	 }

	 else {
	    mask1 = lmask & rmask;
	    {
	       switch (func) {
		  case OPCODE(~(DST | SRC)):	/* right->left single-word blit */
		     for (v_cnt = height; v_cnt > 0; v_cnt--) {
			srcx = src;
			srcx--;
			*dst-- = ~(*dst | *lsrc-- >> shift) & mask1 | *dst & ~mask1;
			src += s_incr;
			dst += d_incr;
		     }

		     break;
		  case OPCODE(DST & ~SRC):	/* right->left single-word blit */
		     for (v_cnt = height; v_cnt > 0; v_cnt--) {
			srcx = src;
			srcx--;
			*dst-- = *dst & ~*lsrc-- >> shift & mask1 | *dst & ~mask1;
			src += s_incr;
			dst += d_incr;
		     }

		     break;
		  case OPCODE(~SRC):	/* right->left single-word blit */
		     for (v_cnt = height; v_cnt > 0; v_cnt--) {
			srcx = src;
			srcx--;
			*dst-- = ~*lsrc-- >> shift & mask1 | *dst & ~mask1;
			src += s_incr;
			dst += d_incr;
		     }

		     break;
		  case OPCODE(~DST & SRC):	/* right->left single-word blit */
		     for (v_cnt = height; v_cnt > 0; v_cnt--) {
			srcx = src;
			srcx--;
			*dst-- = ~*dst & *lsrc-- >> shift & mask1 | *dst & ~mask1;
			src += s_incr;
			dst += d_incr;
		     }

		     break;
		  case OPCODE(DST ^ SRC):	/* right->left single-word blit */
		     for (v_cnt = height; v_cnt > 0; v_cnt--) {
			srcx = src;
			srcx--;
			*dst-- = *dst ^ *lsrc-- >> shift & mask1 | *dst & ~mask1;
			src += s_incr;
			dst += d_incr;
		     }

		     break;
		  case OPCODE(~(DST & SRC)):	/* right->left single-word blit */
		     for (v_cnt = height; v_cnt > 0; v_cnt--) {
			srcx = src;
			srcx--;
			*dst-- = ~(*dst & *lsrc-- >> shift) & mask1 | *dst & ~mask1;
			src += s_incr;
			dst += d_incr;
		     }

		     break;
		  case OPCODE(DST & SRC):	/* right->left single-word blit */
		     for (v_cnt = height; v_cnt > 0; v_cnt--) {
			srcx = src;
			srcx--;
			*dst-- = *dst & *lsrc-- >> shift & mask1 | *dst & ~mask1;
			src += s_incr;
			dst += d_incr;
		     }

		     break;
		  case OPCODE(~(DST ^ SRC)):	/* right->left single-word blit */
		     for (v_cnt = height; v_cnt > 0; v_cnt--) {
			srcx = src;
			srcx--;
			*dst-- = ~(*dst ^ *lsrc-- >> shift) & mask1 | *dst & ~mask1;
			src += s_incr;
			dst += d_incr;
		     }

		     break;
		  case OPCODE(DST | ~SRC):	/* right->left single-word blit */
		     for (v_cnt = height; v_cnt > 0; v_cnt--) {
			srcx = src;
			srcx--;
			*dst-- = *dst | ~*lsrc-- >> shift & mask1 | *dst & ~mask1;
			src += s_incr;
			dst += d_incr;
		     }

		     break;
		  case OPCODE(SRC):	/* right->left single-word blit */
		     for (v_cnt = height; v_cnt > 0; v_cnt--) {
			srcx = src;
			srcx--;
			*dst-- = *lsrc-- >> shift & mask1 | *dst & ~mask1;
			src += s_incr;
			dst += d_incr;
		     }

		     break;
		  case OPCODE(~DST | SRC):	/* right->left single-word blit */
		     for (v_cnt = height; v_cnt > 0; v_cnt--) {
			srcx = src;
			srcx--;
			*dst-- = ~*dst | *lsrc-- >> shift & mask1 | *dst & ~mask1;
			src += s_incr;
			dst += d_incr;
		     }

		     break;
		  case OPCODE(DST | SRC):	/* right->left single-word blit */
		     for (v_cnt = height; v_cnt > 0; v_cnt--) {
			srcx = src;
			srcx--;
			*dst-- = *dst | *lsrc-- >> shift & mask1 | *dst & ~mask1;
			src += s_incr;
			dst += d_incr;
		     }

		     break;	/* these should never be called - use no source case */
		  case OPCODE(0):	/* right->left single-word blit */
		     for (v_cnt = height; v_cnt > 0; v_cnt--) {
			*dst-- = 0 & mask1 | *dst & ~mask1;
			dst += d_incr;
		     }

		     break;
		  case OPCODE(~DST):	/* right->left single-word blit */
		     for (v_cnt = height; v_cnt > 0; v_cnt--) {
			*dst-- = ~*dst & mask1 | *dst & ~mask1;
			dst += d_incr;
		     }

		     break;
		  case OPCODE(~0):	/* right->left single-word blit */
		     for (v_cnt = height; v_cnt > 0; v_cnt--) {
			*dst-- = ~0 & mask1 | *dst & ~mask1;
			dst += d_incr;
		     }

		     break;
		  case OPCODE(DST):
		     break;
	       }

	    }

	 }

      }

   }

}
