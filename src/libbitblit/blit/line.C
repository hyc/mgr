/*                        Copyright (c) 1988 Bellcore
 *                            All Rights Reserved
 *       Permission is granted to copy or use this program, EXCEPT that it
 *       may not be sold for profit, the copyright notice must be reproduced
 *       on copies, and credit should be given to Bellcore where it is due.
 *       BELLCORE MAKES NO WARRANTY AND ACCEPTS NO LIABILITY FOR THIS PROGRAM.
 */
/*	$Header: line.C,v 4.2 88/06/30 11:40:57 sau Exp $
	$Source: /tmp/mgrsrc/src/blit/RCS/line.C,v $
*/
static char	RCSid_[] = "$Source: /tmp/mgrsrc/src/blit/RCS/line.C,v $$Revision: 4.2 $";

#include "asm.h"
#include "bitmap.h"

/*  Draw a line  - Bresenham method using bit-field instructions.
 *  The bitfield instructions don't buy much (<10%), but they're there.
 */

bit_line(dest, x0, y0, x1, y1, func)
register BITMAP *dest;				/* destination bitmap */	
int x0, y0, x1, y1;			/* line coordinates */
int func;					/* set, clear, or invert */
   {
   register int err, rincr, rdecr, d_incr, count;
   register int offset;			/* bit offset from x0,y0 */
   register long *base;			/* address of x0,y0 */
   int dx, dy;				/* # of pixels in x and y */
   int temp;

#ifndef NOCLIP
   /* clip here */

#define TOP	001
#define BOTTOM	002
#define LEFT	004
#define RIGHT	010
#define CROSS(x,y) \
	  (x<0 ? LEFT : x>= (dest->wide) ? RIGHT : 0) + \
	  (y < 0 ? TOP : y >=  (dest -> high) ? BOTTOM : 0)

      {

      /* The classic line clipping algorithm */

      int cross0 = CROSS(x0, y0);
      int cross1 = CROSS(x1, y1);

      while (cross0 || cross1) {
	      int cross, x, y;
	      if (cross0 & cross1)
	         return;
	      if (cross0 != 0)
	         cross = cross0;
	      else
	         cross = cross1;
	      if (cross & (LEFT | RIGHT)) {
	         int edge = (cross & LEFT) ? 0 : dest->wide - 1;
	         y = y0 + (y1 - y0) * (edge - x0) / (x1 - x0);
	         x = edge;
	         }
	      else if (cross & (TOP | BOTTOM)) {
	         int edge = (cross & TOP) ? 0 : dest->high - 1;
	         x = x0 + (x1 - x0) * (edge - y0) / (y1 - y0);
	         y = edge;
	         }
	      if (cross == cross0) {
	         x0 = x;
	         y0 = y;
	         cross0 = CROSS(x, y);
	         }
	      else {
	         x1 = x;
	         y1 = y;
	         cross1 = CROSS(x, y);
	         }
         }
      }

   /* end of clipping */
#endif

   x0 += dest->x0;
   y0 += dest->y0;
   x1 += dest->x0;
   y1 += dest->y0;

   /* always draw left to right */

   if (x1 < x0) {
      temp = x1, x1 = x0, x0 = temp;
      temp = y1, y1 = y0, y0 = temp;
      }
   dx = x1 - x0;
   dy = y1 - y0;

   if (dy > 0)
      d_incr = BIT_LINE(dest);
   else
      d_incr = -BIT_LINE(dest), dy = -dy;

   base = y0 * (BIT_LINE(dest) >> 5) + (dest->data);	/* start of line */
   offset = x0;			/* bit offset from line */

#define STEP(dx,dy,xmove,ymove,op) {		\
    rincr = (dx - dy)<<1;			\
    rdecr = -(dy<<1);				\
    err = dx + rdecr;				\
    for (count = dx; count >= 0; count--) {	\
        op($base,$offset,IMM(1)); \
        offset  += (xmove);	    				\
        if (err < 0) {				\
            offset += (ymove);				\
            err += rincr;				\
            }					\
        else {					\
            err += rdecr;				\
            }					\
        } 					\
    }

   /* @+ */

   if (dx > dy)			/* gentle slope */
      switch (OPCODE(func)) {
	 case OPCODE(SRC):
	 case OPCODE(SRC | DST):
	 case OPCODE(SRC | ~DST):
	 case OPCODE(~0):
	    STEP(dx, dy, 1, d_incr, BF_SET);
	    break;
	 case OPCODE(~SRC):
	 case OPCODE(~(SRC|DST)):
	 case OPCODE(DST & ~SRC):
	 case OPCODE(0):
	    STEP(dx, dy, 1, d_incr, BF_CLR);
	    break;
	 case OPCODE(SRC ^ DST):
	 case OPCODE(~DST):
	 case OPCODE(SRC & ~DST):
	 case OPCODE(~(SRC&DST)):
	    STEP(dx, dy, 1, d_incr, BF_INV);
	    break;
         }
   else				/* steep slope */
      switch (OPCODE(func)) {
	 case OPCODE(SRC):
	 case OPCODE(SRC | DST):
	 case OPCODE(SRC | ~DST):
	 case OPCODE(~0):
	    STEP(dy, dx, d_incr, 1, BF_SET);
	    break;
	 case OPCODE(~SRC):
	 case OPCODE(~(SRC|DST)):
	 case OPCODE(DST & ~SRC):
	 case OPCODE(0):
	    STEP(dy, dx, d_incr, 1, BF_CLR);
	    break;
	 case OPCODE(SRC ^ DST):
	 case OPCODE(~DST):
	 case OPCODE(SRC & ~DST):
	 case OPCODE(~(SRC&DST)):
	    STEP(dy, dx, d_incr, 1, BF_INV);
	    break;
	}
   }
