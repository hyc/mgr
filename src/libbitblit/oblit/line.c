/*                        Copyright (c) 1988 Bellcore
 *                            All Rights Reserved
 *       Permission is granted to copy or use this program, EXCEPT that it
 *       may not be sold for profit, the copyright notice must be reproduced
 *       on copies, and credit should be given to Bellcore where it is due.
 *       BELLCORE MAKES NO WARRANTY AND ACCEPTS NO LIABILITY FOR THIS PROGRAM.
 */
/*	$Header: /files/src/linuxmgr/src/libbitblit/oblit/RCS/line.c,v 4.3 1994/01/28 20:59:34 broman Stab $
	$Source: /files/src/linuxmgr/src/libbitblit/oblit/RCS/line.c,v $
*/
static char	RCSid_[] = "$Source: /files/src/linuxmgr/src/libbitblit/oblit/RCS/line.c,v $$Revision: 4.3 $";

/*  Draw a line 16 bit version */

#include "bitmap.h"

bit_line(dest, x0, y0, x1, y1, func)
BITMAP *dest;
int x0, y0, x1, y1;
int func;
{
   register int r, rincr, rdecr, d_incr, count;
   register unsigned short bit;
   register unsigned short *dst;
   int temp, dx, dy;

   /* clip here */

#define TOP		1
#define BOTTOM	2
#define LEFT	4
#define RIGHT	8

#define crossings(x,y) \
	  (x<0 ? LEFT : x>= (b->wide) ? RIGHT : 0) + \
	  (y < 0 ? TOP : y >=  (b -> high) ? BOTTOM : 0)

   {
      register BITMAP *b = dest;

      /* The classic clipping algorithm */

      int Cross0 = crossings(x0, y0);
      int Cross1 = crossings(x1, y1);

      while (Cross0 || Cross1) {
	 int Cross, x, y;
	 if (Cross0 & Cross1)
	    return;
	 if (Cross0 != 0)
	    Cross = Cross0;
	 else
	    Cross = Cross1;
	 if (Cross & (LEFT | RIGHT)) {
	    int edge = (Cross & LEFT) ? 0 : b->wide - 1;
	    y = y0 + (y1 - y0) * (edge - x0) / (x1 - x0);
	    x = edge;
	 }
	 else if (Cross & (TOP | BOTTOM)) {
	    int edge = (Cross & TOP) ? 0 : b->high - 1;
	    x = x0 + (x1 - x0) * (edge - y0) / (y1 - y0);
	    y = edge;
	 }
	 if (Cross == Cross0) {
	    x0 = x;
	    y0 = y;
	    Cross0 = crossings(x, y);
	 }
	 else {
	    x1 = x;
	    y1 = y;
	    Cross1 = crossings(x, y);
	 }
      }
      x0 += b->x0;
      y0 += b->y0;
      x1 += b->x0;
      y1 += b->y0;
   }

   /* always left to right */

   if (x1 < x0) {
      temp = x1, x1 = x0, x0 = temp;
      temp = y1, y1 = y0, y0 = temp;
   }
   dx = x1 - x0;
   dy = y1 - y0;
   if (dy > 0)
      d_incr = BIT_LINE(dest);
   else
      d_incr = -(BIT_LINE(dest)), dy = -dy;

   dst = (x0 >> 4) + y0 * (BIT_LINE(dest)) + (dest->data);	/*-*/
   bit = (0x8000 >> (x0 & 15));

   /* */

   if (dx > dy)
      switch (OPCODE(func)) {
	 case OPCODE(SRC):
	 case OPCODE(SRC | DST):
	 case OPCODE(SRC | ~DST):
	 case OPCODE(~0):
	    {
	       rincr = (dx - dy) << 1;
	       rdecr = -(dy << 1);
	       r = dx + rdecr;
	       for (count = dx; count >= 0; count--) {
		  *dst |= bit;
		  if ((bit >>= 1) == 0) {
		     bit = 0x8000;
		     dst++;
		  }

		  ;
		  if (r < 0) {
		     dst += d_incr;
		     r += rincr;
		  }

		  else {
		     r += rdecr;
		  }

	       }

	    }

	    ;
	    break;
    case OPCODE(~SRC):
    case OPCODE(~(SRC|DST)):
    case OPCODE(DST & ~SRC):
    case OPCODE(0):
	    {
	       rincr = (dx - dy) << 1;
	       rdecr = -(dy << 1);
	       r = dx + rdecr;
	       for (count = dx; count >= 0; count--) {
		  *dst &= ~bit;
		  if ((bit >>= 1) == 0) {
		     bit = 0x8000;
		     dst++;
		  }

		  ;
		  if (r < 0) {
		     dst += d_incr;
		     r += rincr;
		  }

		  else {
		     r += rdecr;
		  }

	       }

	    }

	    ;
	    break;
    case OPCODE(SRC ^ DST):
    case OPCODE(~DST):
    case OPCODE(SRC & ~DST):
    case OPCODE(~(SRC&DST)):
	    {
	       rincr = (dx - dy) << 1;
	       rdecr = -(dy << 1);
	       r = dx + rdecr;
	       for (count = dx; count >= 0; count--) {
		  *dst ^= bit;
		  if ((bit >>= 1) == 0) {
		     bit = 0x8000;
		     dst++;
		  }

		  ;
		  if (r < 0) {
		     dst += d_incr;
		     r += rincr;
		  }

		  else {
		     r += rdecr;
		  }

	       }

	    }

	    ;
	    break;
      }

   else
      switch (OPCODE(func)) {
    case OPCODE(SRC):
    case OPCODE(SRC | DST):
    case OPCODE(SRC | ~DST):
    case OPCODE(~0):
	    {
	       rincr = (dy - dx) << 1;
	       rdecr = -(dx << 1);
	       r = dy + rdecr;
	       for (count = dy; count >= 0; count--) {
		  *dst |= bit;
		  dst += d_incr;
		  if (r < 0) {
		     if ((bit >>= 1) == 0) {
			bit = 0x8000;
			dst++;
		     }

		     ;
		     r += rincr;
		  }

		  else {
		     r += rdecr;
		  }

	       }

	    }

	    ;
	    break;
    case OPCODE(~SRC):
    case OPCODE(~(SRC|DST)):
    case OPCODE(DST & ~SRC):
    case OPCODE(0):
	    {
	       rincr = (dy - dx) << 1;
	       rdecr = -(dx << 1);
	       r = dy + rdecr;
	       for (count = dy; count >= 0; count--) {
		  *dst &= ~bit;
		  dst += d_incr;
		  if (r < 0) {
		     if ((bit >>= 1) == 0) {
			bit = 0x8000;
			dst++;
		     }

		     ;
		     r += rincr;
		  }

		  else {
		     r += rdecr;
		  }

	       }

	    }

	    ;
	    break;
    case OPCODE(SRC ^ DST):
    case OPCODE(~DST):
    case OPCODE(SRC & ~DST):
    case OPCODE(~(SRC&DST)):
	    {
	       rincr = (dy - dx) << 1;
	       rdecr = -(dx << 1);
	       r = dy + rdecr;
	       for (count = dy; count >= 0; count--) {
		  *dst ^= bit;
		  dst += d_incr;
		  if (r < 0) {
		     if ((bit >>= 1) == 0) {
			bit = 0x8000;
			dst++;
		     }

		     ;
		     r += rincr;
		  }

		  else {
		     r += rdecr;
		  }

	       }

	    }

	    ;
	    break;
      }

}
