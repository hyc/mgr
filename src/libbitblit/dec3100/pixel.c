/*                        Copyright (c) 1989 Bellcore
 *                            All Rights Reserved
 *       Permission is granted to copy or use this program, EXCEPT that it
 *       may not be sold for profit, the copyright notice must be reproduced
 *       on copies, and credit should be given to Bellcore where it is due.
 *       BELLCORE MAKES NO WARRANTY AND ACCEPTS NO LIABILITY FOR THIS PROGRAM.
 */
/*	$Header: /files/src/linuxmgr/src/libbitblit/dec3100/RCS/pixel.c,v 4.3 1994/01/28 21:01:44 broman Rel $
	$Source: /files/src/linuxmgr/src/libbitblit/dec3100/RCS/pixel.c,v $
*/
static char	RCSid_[] = "$Source: /files/src/linuxmgr/src/libbitblit/dec3100/RCS/pixel.c,v $$Revision: 4.3 $";

/* set/clear/ or invert a pixel (DEC 3100 version) */

#include "bitmap.h"

bit_point(map,x,y,op)
register BITMAP *map;
register int x,y;
int op;
   {
   register int bit;							/* dst bit */
	register DATA *base;							/* dst word */

   /* clipping */

#ifndef NOCLIP
   if (x<0 || x>BIT_WIDE(map) || y<0 || y>BIT_HIGH(map))
      return(-1);
#endif

#ifdef INVERT
	/* invert all raster ops */

	op = op_invert[15&op];
#endif

	x += map->x0;
	y += map->y0;
   base = y * BIT_LINE(map) + (x>>LOGBITS) + (map->data);
   bit = GETLSB(MSB,(x & BITS));
  
   switch(OPCODE(op)) {
		case OPCODE(SRC):
		case OPCODE(SRC | DST):
		case OPCODE(SRC | ~DST):
		case OPCODE(~0):
			*base |= bit;
         break;
		case OPCODE(~SRC):
		case OPCODE(~(SRC|DST)):
		case OPCODE(DST & ~SRC):
		case OPCODE(0):
			*base &= ~bit;
         break;
		case OPCODE(SRC ^ DST):
		case OPCODE(~DST):
		case OPCODE(SRC & ~DST):
		case OPCODE(~(SRC&DST)):
			*base ^= bit;
         break;
      }
   return(0);
   }
