/*                        Copyright (c) 1989 Bellcore
 *                            All Rights Reserved
 *       Permission is granted to copy or use this program, EXCEPT that it
 *       may not be sold for profit, the copyright notice must be reproduced
 *       on copies, and credit should be given to Bellcore where it is due.
 *       BELLCORE MAKES NO WARRANTY AND ACCEPTS NO LIABILITY FOR THIS PROGRAM.
 */
/*	$Header: /files/src/linuxmgr/src/libbitblit/dec3100/RCS/bitmap.c,v 4.3 1994/01/28 21:01:44 broman Rel $
	$Source: /files/src/linuxmgr/src/libbitblit/dec3100/RCS/bitmap.c,v $
*/
static char	RCSid_[] = "$Source: /files/src/linuxmgr/src/libbitblit/dec3100/RCS/bitmap.c,v $$Revision: 4.3 $";

/* dec3100 bitblit routines (The bit_open interface needs work) */

#include <stdio.h>
#include <sys/types.h>
#include "bitmap.h"
#include "/usr/sys/mips/mips/pmioctl.h" 
#include "/usr/sys/mips/mips/pmevent.h" 

int bit_debug = 0;

BITMAP *_frame_buffer_;				/* bitmap encompassing entire frame buffer */
extern struct pm_info *info;		/* this has been filled in previously */

/* open the display */

BITMAP *
bit_open(fd)
int fd;			/* fd of frame buffer (ignored) */
{
   BITMAP *result;
   char *malloc();
	DATA *addr;								/* fb address */
	pmCursor cursor;						/* their mouse cursor position */

	addr = (DATA *) info->bitmap; 

   if ((result = (BITMAP *) malloc(sizeof(BITMAP))) == (BITMAP *) 0) {
		fprintf(stderr,"bit_open: Can't malloc bitmap \n");
      return (BIT_NULL);
		}

   result->primary = result;
   result->data = addr;
   result->x0 = 0,
   result->y0 = 0,
   result->wide = 2048;
   result->high = 1024;
   result->type = _SCREEN;
	_frame_buffer_ = result;
	result = bit_create(_frame_buffer_,0,0,1024,864);
	result->type = _SCREEN;
   return (result);
}

/* destroy a bitmap, free up space (might need special code for the display) */

int
bit_destroy(bitmap)
BITMAP *bitmap;
   {
   if (bitmap == (BITMAP *) 0)
      return (-1);
   if (IS_MEMORY(bitmap) && IS_PRIMARY(bitmap))
      free(bitmap->data);
/*
	else if (IS_SCREEN(bitmap)) {
		free(bitmap->primary);
		}
*/
   free(bitmap);
   return(0);
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

   if ((result = (BITMAP *) malloc(sizeof(BITMAP))) == (BITMAP *) 0) {
		fprintf(stderr,"bit_create: Can't malloc\n");
      return (BIT_NULL);
		}

   result->data = map->data;
   result->x0 = map->x0 + x;
   result->y0 = map->y0 + y;
   result->wide = wide;
   result->high = high;
   result->primary = map->primary;
   result->type = 3&map->type;
   return (result);
}

/* allocate space for, and create a memory bitmap */

BITMAP *
bit_alloc(wide, high, data, bits)
unsigned short wide, high;
DATA *data;
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

	size = ((wide+BITS)>>LOGBITS) * high * sizeof(DATA);

   if (data != (DATA *) 0) {
      result->data = data;
		if (MSB==1) {
			flip(data,size/sizeof(DATA));
			}
		}
   else if ((result->data = (DATA *) malloc(size)) == (DATA *) 0) {
      free(result);
      return ((BITMAP *) 0);
   }

   result->primary = result;
   result->type = _MEMORY;
   return (result);
}

/* flip the bit order on count elements of s */

static unsigned char flp[256] = {
	0x00,	0x80,	0x40,	0xc0,	0x20,	0xa0,	0x60,	0xe0,
	0x10,	0x90,	0x50,	0xd0,	0x30,	0xb0,	0x70,	0xf0,
	0x08,	0x88,	0x48,	0xc8,	0x28,	0xa8,	0x68,	0xe8,
	0x18,	0x98,	0x58,	0xd8,	0x38,	0xb8,	0x78,	0xf8,
	0x04,	0x84,	0x44,	0xc4,	0x24,	0xa4,	0x64,	0xe4,
	0x14,	0x94,	0x54,	0xd4,	0x34,	0xb4,	0x74,	0xf4,
	0x0c,	0x8c,	0x4c,	0xcc,	0x2c,	0xac,	0x6c,	0xec,
	0x1c,	0x9c,	0x5c,	0xdc,	0x3c,	0xbc,	0x7c,	0xfc,
	0x02,	0x82,	0x42,	0xc2,	0x22,	0xa2,	0x62,	0xe2,
	0x12,	0x92,	0x52,	0xd2,	0x32,	0xb2,	0x72,	0xf2,
	0x0a,	0x8a,	0x4a,	0xca,	0x2a,	0xaa,	0x6a,	0xea,
	0x1a,	0x9a,	0x5a,	0xda,	0x3a,	0xba,	0x7a,	0xfa,
	0x06,	0x86,	0x46,	0xc6,	0x26,	0xa6,	0x66,	0xe6,
	0x16,	0x96,	0x56,	0xd6,	0x36,	0xb6,	0x76,	0xf6,
	0x0e,	0x8e,	0x4e,	0xce,	0x2e,	0xae,	0x6e,	0xee,
	0x1e,	0x9e,	0x5e,	0xde,	0x3e,	0xbe,	0x7e,	0xfe,
	0x01,	0x81,	0x41,	0xc1,	0x21,	0xa1,	0x61,	0xe1,
	0x11,	0x91,	0x51,	0xd1,	0x31,	0xb1,	0x71,	0xf1,
	0x09,	0x89,	0x49,	0xc9,	0x29,	0xa9,	0x69,	0xe9,
	0x19,	0x99,	0x59,	0xd9,	0x39,	0xb9,	0x79,	0xf9,
	0x05,	0x85,	0x45,	0xc5,	0x25,	0xa5,	0x65,	0xe5,
	0x15,	0x95,	0x55,	0xd5,	0x35,	0xb5,	0x75,	0xf5,
	0x0d,	0x8d,	0x4d,	0xcd,	0x2d,	0xad,	0x6d,	0xed,
	0x1d,	0x9d,	0x5d,	0xdd,	0x3d,	0xbd,	0x7d,	0xfd,
	0x03,	0x83,	0x43,	0xc3,	0x23,	0xa3,	0x63,	0xe3,
	0x13,	0x93,	0x53,	0xd3,	0x33,	0xb3,	0x73,	0xf3,
	0x0b,	0x8b,	0x4b,	0xcb,	0x2b,	0xab,	0x6b,	0xeb,
	0x1b,	0x9b,	0x5b,	0xdb,	0x3b,	0xbb,	0x7b,	0xfb,
	0x07,	0x87,	0x47,	0xc7,	0x27,	0xa7,	0x67,	0xe7,
	0x17,	0x97,	0x57,	0xd7,	0x37,	0xb7,	0x77,	0xf7,
	0x0f,	0x8f,	0x4f,	0xcf,	0x2f,	0xaf,	0x6f,	0xef,
	0x1f,	0x9f,	0x5f,	0xdf,	0x3f,	0xbf,	0x7f,	0xff,
	};

int
flip(s,count,how)
register DATA *s;
register int count;
int how;						/* number of chars is data type */
	{
	how=1;					/* later */
	switch(how) {
		case 1:				/* flip each byte (and invert) */
			while (count-- > 0) 
				*s++ = ~((flp[*s&0xff]) | (flp[*s>>8&0xff]<<8) |
						 (flp[*s>>16&0xff]<<16) | (flp[*s>>24&0xff]<<24));
			break;
		case 2:				/* flip each byte, flip bytes in short */
			while (count-- > 0) 
				*s++ = (flp[*s&0xff]<<8) | (flp[*s>>8&0xff]<<0) |
						 (flp[*s>>16&0xff]<<24) | (flp[*s>>24&0xff]<<16);
			break;
		case 4:				/* flip each byte, each byte in long */
			while (count-- > 0) 
				*s++ = (flp[*s&0xff]<<24) | (flp[*s>>8&0xff]<<16) |
						 (flp[*s>>16&0xff]<<8) | (flp[*s>>24&0xff]<<0);
			break;
		}
	}
