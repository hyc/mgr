/*                        Copyright (c) 1989,1991 Bellcore
 *                            All Rights Reserved
 *       Permission is granted to copy or use this program, EXCEPT that it
 *       may not be sold for profit, the copyright notice must be reproduced
 *       on copies, and credit should be given to Bellcore where it is due.
 *       BELLCORE MAKES NO WARRANTY AND ACCEPTS NO LIABILITY FOR THIS PROGRAM.
 */
/*	$Header: /home1/Np/broman/RCS/bit_line.c,v 4.10 1994/01/31 19:58:44 broman Rel $
	$Source: /home1/Np/broman/RCS/bit_line.c,v $
*/
static char	RCSid_[] = "$Source: /home1/Np/broman/RCS/bit_line.c,v $$Revision: 4.10 $";

#include "screen.h"

void bit_line(dest, x0, y0, x1, y1, func)
register BITMAP *dest;			/* destination maskmap */	
int x0, y0, x1, y1;			/* line coordinates */
int func;				/* set, clear, or invert  + color */
{
#ifndef NOCLIP

   /* clip here */

#define TOP	001
#define BOTTOM	002
#define LEFT	004
#define RIGHT	010
#define CROSS(x,y) \
	  (x<0 ? LEFT : x>= BIT_WIDE(dest) ? RIGHT : 0) + \
	  (y < 0 ? TOP : y >=  BIT_HIGH(dest) ? BOTTOM : 0)

      {

      /* The classic line clipping algorithm */
		/* (I don't remember anymore where I got it from, sorry -sau) */

      register int cross0 = CROSS(x0, y0);
      register int cross1 = CROSS(x1, y1);

      while (cross0 || cross1) {
	      int cross, x, y;
	      if (cross0 & cross1)
	         return;
	      if (cross0 != 0)
	         cross = cross0;
	      else
	         cross = cross1;
	      /* cross nonzero */
	      if (cross & (LEFT | RIGHT)) {
	         int edge = (cross & LEFT) ? 0 : dest->wide - 1;
	         y = y0 + (y1 - y0) * (edge - x0) / (x1 - x0);
	         x = edge;
	         }
	      else {		/* (cross & (TOP | BOTTOM)) nonzero */
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

#ifdef INVERT
	func = rop_invert(func);
#endif

   pr_vector(dest,x0,y0,x1,y1,PIXRECT_OP(func),0);

}
