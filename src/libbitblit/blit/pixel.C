/*                        Copyright (c) 1988 Bellcore
 *                            All Rights Reserved
 *       Permission is granted to copy or use this program, EXCEPT that it
 *       may not be sold for profit, the copyright notice must be reproduced
 *       on copies, and credit should be given to Bellcore where it is due.
 *       BELLCORE MAKES NO WARRANTY AND ACCEPTS NO LIABILITY FOR THIS PROGRAM.
 */
/*	$Header: pixel.C,v 4.2 88/06/30 11:39:58 sau Exp $
	$Source: /tmp/mgrsrc/src/blit/RCS/pixel.C,v $
*/
static char	RCSid_[] = "$Source: /tmp/mgrsrc/src/blit/RCS/pixel.C,v $$Revision: 4.2 $";

/* set/clear/ or invert a pixel */

#include "bitmap.h"
#include "asm.h"

bit_point(map,x,y,op)
BITMAP *map;
register int x,y,op;
   {
   register int offset;
	register int *base = BIT_DATA(map);

   /* clipping */

   if (x<0 || x>BIT_WIDE(map) || y<0 || y>BIT_HIGH(map))
      return(-1);

   offset = (y+map->y0)*BIT_LINE(map)+map->x0+x;
  
   switch(OPCODE(op)) {
		case OPCODE(SRC):
		case OPCODE(SRC | DST):
		case OPCODE(SRC | ~DST):
		case OPCODE(~0):
			BF_SET($base,$offset,IMM(1));
         break;
		case OPCODE(~SRC):
		case OPCODE(~(SRC|DST)):
		case OPCODE(DST & ~SRC):
		case OPCODE(0):
			BF_CLR($base,$offset,IMM(1));
         break;
		case OPCODE(SRC ^ DST):
		case OPCODE(~DST):
		case OPCODE(SRC & ~DST):
		case OPCODE(~(SRC&DST)):
			BF_INV($base,$offset,IMM(1));
         break;
      }
   return(0);
   }
