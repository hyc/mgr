/*                        Copyright (c) 1988 Bellcore
 *                            All Rights Reserved
 *       Permission is granted to copy or use this program, EXCEPT that it
 *       may not be sold for profit, the copyright notice must be reproduced
 *       on copies, and credit should be given to Bellcore where it is due.
 *       BELLCORE MAKES NO WARRANTY AND ACCEPTS NO LIABILITY FOR THIS PROGRAM.
 */
/*	$Header: /files/src/linuxmgr/src/libbitblit/oblit/RCS/pixel.c,v 4.3 1994/01/28 20:59:34 broman Stab $
	$Source: /files/src/linuxmgr/src/libbitblit/oblit/RCS/pixel.c,v $
*/
static char	RCSid_[] = "$Source: /files/src/linuxmgr/src/libbitblit/oblit/RCS/pixel.c,v $$Revision: 4.3 $";

/* draw a point  16 bit version */

#include "bitmap.h"

int
bit_point(map, dx, dy, func)
BITMAP *map;
register int dx, dy;
int func;
{
   register int x = BIT_X(map) + dx;
   register int y = BIT_Y(map) + dy;
   register unsigned short *word =
   BIT_DATA(map) + (y * BIT_LINE(map) + (x >> 4));
   register unsigned short bit = 0x8000 >> (x & 0xf);

   if (dx < 0 || dy < 0 || dx >= BIT_WIDE(map) || dy >= BIT_HIGH(map))
      return (0);

   switch (OPCODE(func)) {
      case OPCODE(0):
      case OPCODE(SRC):
	 *word &= ~bit;
	 break;
      case OPCODE(~SRC):
      case OPCODE(~SRC | DST):
      case OPCODE(~0):
	 *word |= bit;
	 break;
      case OPCODE(~SRC ^ DST):
      case OPCODE(~DST):
	 *word ^= bit;
	 break;
   }

   return (*word & bit);
}
