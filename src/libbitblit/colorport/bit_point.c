/*                        Copyright (c) 1989,1991 Bellcore
 *                            All Rights Reserved
 *       Permission is granted to copy or use this program, EXCEPT that it
 *       may not be sold for profit, the copyright notice must be reproduced
 *       on copies, and credit should be given to Bellcore where it is due.
 *       BELLCORE MAKES NO WARRANTY AND ACCEPTS NO LIABILITY FOR THIS PROGRAM.
 */
static char	RCSid_[] = "$Source: /files/src/linuxmgr/src/libbitblit/colorport/RCS/bit_point.c,v $$Revision: 4.12 $";

#include <stdio.h>		/* debugging only */
#include "screen.h"

/*  Draw a pixel  - portable 8-bit Bitblt version (S. A. Uhler)
 */

int
bit_point(map, x, y, func)
register BITMAP *map;			/* destination maskmap */	
int x, y;				/* point coordinates */
int func;				/* set, clear, or invert  + color */
   {
   register DATA mask;			/* mask offset in current word */
   register DATA bits;			/* initial mask */
   register DATA *dest;			/* current word in maskmap */
   register int depth = map->depth;	
   DATA color;				/* use as source color */

#ifndef NOCLIP
   /* clipping */

   if (x<0 || x>BIT_WIDE(map) || y<0 || y>BIT_HIGH(map))
      return(0);
#endif

#ifdef MOVIE
  log_point(map,x,y,func);
#endif

#ifdef INVERT
   /* invert all raster ops */

   op = rop_invert(op);
#endif

   x += map->x0;
   y += map->y0;

   /* codes with ~src get bg color, otherwise fg color */

   switch (OPCODE(func)) {
      case OPCODE(~SRC):
      case OPCODE(0):
      case OPCODE(~SRC | DST):
      case OPCODE(~SRC | ~DST):
      case OPCODE(DST & ~SRC):
      case OPCODE(~DST & ~SRC):
      case OPCODE(~SRC ^ DST):
         color =  GETBCOLOR(func);
         break;
      default:
         color =  GETFCOLOR(func);
      }
#if LOGBITS > 3
   color |= color<<8;
#endif
#if LOGBITS > 4
   color |= color<<16;
#endif

         
   bits = ~GETLSB((DATA)~0,depth);    /* basic bits for mask */
   dest = y * BIT_LINE(map) + (depth*x>>LOGBITS) + BIT_DATA(map);
#ifdef BANKED
   if( IS_SCREEN( map))
      dest = getdatap( dest, BIT_DATA(map), NULL);
#endif

   mask = GETLSB(bits,x*depth&BITS);

#  define OP(x)	(*dest = (*dest&~mask) | ((x) & mask))

   switch (OPCODE(func)) {
      case OPCODE(SRC):
      case OPCODE(~SRC):
      case OPCODE(~0):
      case OPCODE(0):
         OP(color);
         break;
      case OPCODE(~SRC | DST):
      case OPCODE(SRC | DST):
         OP(*dest|color);
         break;
      case OPCODE(SRC | ~DST):
      case OPCODE(~SRC | ~DST):
         OP(~*dest|color);
         break;
      case OPCODE(DST & SRC):
      case OPCODE(DST & ~SRC):
         OP(*dest&color);
         break;
      case OPCODE(~DST & SRC):
      case OPCODE(~DST & ~SRC):
         OP(~*dest&color);
         break;
      case OPCODE(SRC ^ DST):
      case OPCODE(~SRC ^ DST):
         OP(*dest^color);
         break;
      case OPCODE(~DST):
         *dest ^= mask;    /* invert */
      case OPCODE(DST):    /* no-op */
         break;
      }
   return(*dest&mask);	/* wrong! */
   }
