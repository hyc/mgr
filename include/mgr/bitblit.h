#ifndef _MGR_BLITLIB_H
#define _MGR_BLITLIB_H

#include <stdio.h>

#include "window.h"

/* basic frame buffer word size */
#ifndef DATA
#define DATA void
#endif

/* NULL bitmap data */
#define NULL_DATA	((DATA *) 0)

/* NULL bitmap pointer */
#define BIT_NULL	((BITMAP *) 0)

/* frame buffer */
#define _SCREEN		1

/* malloc'd space */
#define _MEMORY		2

/* don't free space at destroy time */
#define _STATIC		3

/* data is in external format */
#define _FLIP		4

/* data is "dirty" */
#define _DIRTY          8

/* member access macros */

#define IS_SCREEN(x)	((3&(x)->type)==_SCREEN)	/* bitmap is on the display */
#define IS_MEMORY(x)	((3&(x)->type)==_MEMORY)	/* bitmap space malloc'd */
#define IS_STATIC(x)	((3&(x)->type)==_STATIC)	/* bitmap space is static */
#define IS_PRIMARY(x)	((x)->primary == (x))
#define SET_FLIP(x)     ((x)->primary->type |= DOFLIP ? _FLIP : 0)

#define BIT_X(x)	((x)->x0)
#define BIT_Y(x)	((x)->y0)
#define BIT_DATA(x)	((x)->data)
#define BIT_WIDE(x)	((x)->wide)
#define BIT_HIGH(x)	((x)->high)
#define BIT_DEPTH(x)	((int) ((x)->depth))
#define BIT_CACHE(x)    ((x)->primary->cache)
#define BIT_CHCLR(x)    ((x)->primary->color)

#define SET_DIRTY(x) (bit_destroy(BIT_CACHE(x)),BIT_CACHE(x)=NULL)

/* structure and type definitions */

typedef struct bitmap
{
  DATA *data;              /* bitmap data */
  struct bitmap	*primary;  /* pointer to primary bitmap (server only) */
  int x0, y0;              /* starting coordinates, in bits */
  int wide, high;          /* bitmap size, in bits */
  unsigned char depth;     /* bitmap depth */
  char type;               /* bitmap type (server only) */
  unsigned short int id;   /* bitmap ID for movie mgr */
  struct bitmap *cache;    /* cached 8 bit expansion of monochrome images */
  int color;		   /* cached color (op>>4) */
  void *deviceinfo;	   /* dev-dep stuff needed by screen driver, if any */
} BITMAP;

/* Macro to declare a 1 bit per pixel static bitmap */
#define bit_static(name,wide,high,data,depth,id) \
BITMAP name = { (DATA *)data, &(name), 0, 0, wide, high, depth, _STATIC, id, \
		NULL, 0, NULL }

int bitmaphead(FILE *fp, int *wp, int *hp, unsigned char *dp, int *size1p);
BITMAP *bitmapread(FILE *fp);
int bitmapwrite(FILE *fp, BITMAP *bp);

/*
 * The macro "GET Most Significant Bits" defines how the bits in each
 * word map from memory to pixels on the display.  The top left most
 * pixel on the display comes from either the *high* order or *low* order
 * bit of the first frame buffer word.  Use "<<" in the first case, ">>"
 * in the second.
 * 
 * The macro "GET Least Significant Bits" does the inverse of GETMSB
 */

#define GETMSB(word,shift)	\
	((word) << (shift))		/* get most significant bits in word */
#define GETLSB(word,shift) \
	((word) >> (shift))		/* get least significant bits in word */

/* these probably won't need changing */

#define MSB (~GETLSB((DATA)~0,1)) /* most sig, actually leftmost, bit set */
#define LSB (~GETMSB((DATA)~0,1)) /* least sig, actually rightmost, bit set */

/*
 * bitmap data has 2 formats, an internal format and an external format.
 * (Sometimes the formats are the same).  The external format is native
 * 68020 SUN/3, DATA aligned 1=black, 0=white.  The internal format is
 * whatever the frame buffer is.  If DOFLIP is set, data is converted
 * from external to internal format the first time it is used.  Bitmap
 * data is recognized as being in external format if the _FLIP flag is
 * set in the type field.  The installation routine flip() does the
 * conversion.
 */

/* need to flip bytes */

#define DOFLIP GETLSB( 1, 1)

/* Function declarations */

extern BITMAP *bit_load( int w, int h, unsigned char d,
			 int size, unsigned char *src);
extern void *bit_save( BITMAP *bp);
extern BITMAP *bit_alloc( int wide, int high, DATA *data, unsigned char depth);
extern void bit_blit( BITMAP *dst, int dx, int dy, int width, int height,
		      int func, BITMAP *src, int sx, int sy);
extern BITMAP *bit_create( BITMAP *map, int x, int y, int wide, int high);
extern void bit_destroy( BITMAP *map);
extern void bit_line( BITMAP *dst, int x0, int y0, int x1, int y1, int func);
extern int bit_on( BITMAP *bp, int x, int y);
extern BITMAP *bit_open( char *name);
extern int bit_point( BITMAP *map, int dx, int dy, int func);
extern int bit_size( int wide, int high, unsigned char depth);
extern void bit_grafscreen( void);
extern void bit_textscreen( void);
extern DATA *bit_initscreen( char *name,
			     int *width, int *height, unsigned char *depth,
			     void **devi);
extern void bit_bytescroll( BITMAP *map,
			    int x, int y, int wide, int high, int delta);
extern BITMAP *bit_shrink( BITMAP *src_map, int bg_color);
extern int timestamp( void);
extern unsigned int fg_color_idx( void);
extern void setpalette( BITMAP *bp,
			unsigned int index,
			unsigned int red,
			unsigned int green,
			unsigned int blue,
			unsigned int maxi);
extern void getpalette( BITMAP *bp,
			unsigned int index,
			unsigned int *red,
			unsigned int *green,
			unsigned int *blue,
			unsigned int *maxi);

#endif
/*{{{}}}*/
