/*                        Copyright (c) 1987 Bellcore
 *                            All Rights Reserved
 *       Permission is granted to copy or use this program, EXCEPT that it
 *       may not be sold for profit, the copyright notice must be reproduced
 *       on copies, and credit should be given to Bellcore where it is due.
 *       BELLCORE MAKES NO WARRANTY AND ACCEPTS NO LIABILITY FOR THIS PROGRAM.
 */
/*	$Header: /files/src/linuxmgr/src/libbitblit/stub_lib/RCS/bitmap.c,v 4.3 1994/01/28 21:01:13 broman Stab $
	$Source: /files/src/linuxmgr/src/libbitblit/stub_lib/RCS/bitmap.c,v $
*/
static char	RCSid_[] = "$Source: /files/src/linuxmgr/src/libbitblit/stub_lib/RCS/bitmap.c,v $$Revision: 4.3 $";

/*  generic bitblit code routines*/

#include "bitmap.h"

/* open the display */

BITMAP *
bit_open(name)
char *name;			/* name of frame buffer */
{
   BITMAP *result;
   char *malloc();

   if ((result = (BITMAP *) malloc(sizeof(BITMAP))) == (BITMAP *) 0)
      return (BIT_NULL);

	/* do what you need to do here yo initialize the display */

   result->primary = result;
   result->data = 0;
   result->x0 = 0,
   result->y0 = 0,
   result->wide = 1000;
   result->high = 900;
   result->type = _SCREEN;
   return (result);
}

/* destroy a bitmap, free up space (might nedd special code for the display) */

int
bit_destroy(bitmap)
BITMAP *bitmap;
{
   if (bitmap == (BITMAP *) 0)
      return (-1);
   if (IS_MEMORY(bitmap) && IS_PRIMARY(bitmap))
      free(bitmap->data);
   free(bitmap);
   return (0);
}

/* create a bitmap as a sub-rectangle of another bitmap */

BITMAP *
bit_create(map, x, y, wide, high)
BITMAP *map;
int x, y, wide, high;
{
   char *malloc();
   register BITMAP *result;

   if (x + wide > map->wide)
      wide = map->wide - x;
   if (y + high > map->high)
      high = map->high - y;
   if (wide < 1 || high < 1)
      return (BIT_NULL);

   if ((result = (BITMAP *) malloc(sizeof(BITMAP))) == (BITMAP *) 0)
      return (BIT_NULL);

   result->data = map->data;
   result->x0 = map->x0 + x;
   result->y0 = map->y0 + y;
   result->wide = wide;
   result->high = high;
   result->primary = map->primary;
   result->type = map->type;
   return (result);
}

/* allocate space for, and create a memory bitmap */

BITMAP *
bit_alloc(wide, high, data, bits)
unsigned short wide, high;
DATA data;
int bits;	/* in preparation for color */
{
   char *malloc();
   register BITMAP *result;
   register int size;

   if ((result = (BITMAP *) malloc(sizeof(BITMAP))) == (BITMAP *) 0)
      return (result);

   result->x0 = 0;
   result->y0 = 0;
   result->high = high;
   result->wide = wide;

   size = BIT_SIZE(result);

   if (data != (DATA ) 0)
      result->data = data;
   else if ((result->data = (DATA ) malloc(size)) == (DATA ) 0) {
      free(result);
      return ((BITMAP *) 0);
   }

   result->primary = result;
   result->type = _MEMORY;
   return (result);
}
