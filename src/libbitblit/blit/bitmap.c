/*                        Copyright (c) 1987 Bellcore
 *                            All Rights Reserved
 *       Permission is granted to copy or use this program, EXCEPT that it
 *       may not be sold for profit, the copyright notice must be reproduced
 *       on copies, and credit should be given to Bellcore where it is due.
 *       BELLCORE MAKES NO WARRANTY AND ACCEPTS NO LIABILITY FOR THIS PROGRAM.
 */
/*	$Header: bitmap.c,v 4.2 88/07/07 09:04:29 sau Exp $
	$Source: /tmp/mgrsrc/src/blit/RCS/bitmap.c,v $
*/
static char	RCSid_[] = "$Source: /tmp/mgrsrc/src/blit/RCS/bitmap.c,v $$Revision: 4.2 $";

/*  SUN-2  and SUN-3 bitblit code */

#ifdef sun
#  include <sys/ioctl.h>
#  include <sun/fbio.h>
#  include <sys/file.h>
#  include <sys/mman.h>
#endif
#include <stdio.h>
#include "bitmap.h"

int bit_debug = 1;
static int _s_start;
static _s_len;

/* open the screen; it looks like memory */

BITMAP *
bit_open(name)
char *name;			/* name of frame buffer */
{
   BITMAP *result = BIT_NULL;
#ifdef sun
   int fd;
   char *malloc();
   register DATA addr;
   struct fbtype buff;
   int pagesize;

   Bprintf(1) ("bit_open:(%s)\n", name);

   /* open the SUN screen */

   if ((fd = open(name, O_RDWR)) < 0)
      return (BIT_NULL);

   /* get the frame buffer size */

   if (ioctl(fd, FBIOGTYPE, &buff) < 0)
      return (BIT_NULL);

   /* malloc space for frame buffer */

   pagesize = getpagesize();
   if ((_s_start = (int) malloc(buff.fb_size + pagesize)) == 0)
      return (BIT_NULL);

   /* align space on a page boundary */

   buff.fb_size = (buff.fb_size+pagesize-1) &~ (pagesize-1);
   addr = (DATA ) ((_s_start + pagesize - 1) & ~(pagesize - 1));

   /* map the frame buffer into malloc'd space */

#ifdef _MAP_NEW		/* New semantics for mmap in Sun release 4.0 */
   addr = (DATA) mmap(addr, _s_len=buff.fb_size,
						 PROT_READ|PROT_WRITE, _MAP_NEW|MAP_SHARED, fd, 0);
   if ((int)addr == -1)
      return (BIT_NULL);
#else
   if (mmap(addr, _s_len = buff.fb_size, PROT_WRITE, MAP_SHARED, fd, 0) < 0)
      return (BIT_NULL);
#endif

   if ((result = (BITMAP *) malloc(sizeof(BITMAP))) == (BITMAP *) 0)
      return (BIT_NULL);

   result->primary = result;
   result->data = addr;
   result->x0 = 0,
      result->y0 = 0,
      result->wide = buff.fb_width;
   result->high = buff.fb_height;
   result->type = _SCREEN;
   Bprintf(2) ("  O.K.(0x%lx)\n (%d x %x)",
	       (long) result->data, result->wide, result->high);
#endif
   return (result);
}

/* destroy a bitmap, free up space */

int
bit_destroy(bitmap)
BITMAP *bitmap;
{
   Bprintf(1) ("bit_destroy:\n");
   if (bitmap == (BITMAP *) 0)
      return (-1);
   if (IS_MEMORY(bitmap) && IS_PRIMARY(bitmap))
      free(bitmap->data);
   else if (IS_SCREEN(bitmap) && IS_PRIMARY(bitmap)) {
      munmap(BIT_DATA(bitmap), _s_len);
      free(_s_start);
   }
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

   Bprintf(1) ("bit_create:\n");
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
   Bprintf(2) ("  Created %d,%d %d,%d\n", result->x0, result->y0,
	       result->wide, result->high);
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

   Bprintf(1) ("bit_alloc:\n");
   if ((result = (BITMAP *) malloc(sizeof(BITMAP))) == (BITMAP *) 0)
      return (result);

   result->x0 = 0;
   result->y0 = 0;
   result->high = high;
   result->wide = wide;

   size = BIT_SIZE(result);

   if (data != (DATA ) 0)
      result->data = data;
#ifdef ALIGN32
   else if ((result->data = (DATA ) memalign(4,size)) == (DATA ) 0) {
#else
   else if ((result->data = (DATA ) malloc(size)) == (DATA ) 0) {
#endif
      free(result);
      return ((BITMAP *) 0);
   }

   result->primary = result;
   result->type = _MEMORY;
   Bprintf(2) ("  Created %d,%d %d,%d\n", result->x0, result->y0,
	       result->wide, result->high);
   return (result);
}
