/*                        Copyright (c) 1987 Bellcore
 *                            All Rights Reserved
 *       Permission is granted to copy or use this program, EXCEPT that it
 *       may not be sold for profit, the copyright notice must be reproduced
 *       on copies, and credit should be given to Bellcore where it is due.
 *       BELLCORE MAKES NO WARRANTY AND ACCEPTS NO LIABILITY FOR THIS PROGRAM.
 */
/*	$Header: bitmap.h,v 4.3 88/07/19 14:20:06 sau Exp $
	$Source: /tmp/mgrsrc/src/pixrect/RCS/bitmap.h,v $
*/
static char	h_sundep_[] = "$Source: /tmp/mgrsrc/src/pixrect/RCS/bitmap.h,v $$Revision: 4.3 $";

/* map generic bit-blit library calls to sun pixrect */

#include <sys/types.h>
#include <pixrect/pixrect_hs.h>

#define GT0(x)	((x)>0?(x):0)	/* SUN's routines don't always clip */
#define GET_OP(x)	((x)<<1)

#define bit_blit(dst,dx,dy,wide,high,f,src,sx,sy) \
	pr_rop(dst,dx,dy,wide,high,f,src,sx,sy)

#define bit_line(map,x1,y1,x2,y2,op) \
	pr_vector(map,GT0(x1),GT0(y1),GT0(x2),GT0(y2),op,0)

#define bit_create(map,x,y,wide,high) \
	pr_region(map,x,y,wide,high)

#define bit_alloc(wide,high,data,depth) \
        (data==0 ? mem_create(wide,high,depth) : \
         mem_point(wide,high,depth,data))

#define bit_destroy(map) \
	pr_destroy(map)

#define bit_open(dev) \
	pr_open(dev)

#define bit_pattern	pr_replrop  /* aarg */

#define BIT_NOT(x) \
	PIX_NOT(x)

#define bit_static(name,wide,high,data,depth) \
	mpr_static(name,wide,high,depth,data)

#define BITMAP		struct pixrect
#define BIT_NULL		((BITMAP *) 0)
#define NULL_DATA	((unsigned short *) 0)

#define BIT_WIDE(b)	b->pr_size.x
#define BIT_HIGH(b)	b->pr_size.y

#define BIT_X(b) \
	(((struct mpr_data *) ((b)->pr_data))->md_offset.x)

#define BIT_Y(b) \
	(((struct mpr_data *) ((b)->pr_data))->md_offset.y)

#define BIT_DATA(b) \
	((char *) (((struct mpr_data *) ((b)->pr_data))->md_image))

#define BIT_DEPTH(b) b->pr_depth

#define BIT_SIZE(m)	(BIT_DEPTH(m)==1 ? \
			(((BIT_WIDE(m)+15L)&~15)*BIT_HIGH(m)>>3) : \
			BIT_WIDE(m)*BIT_HIGH(m)*(BIT_DEPTH(m)>>3))

#define OPCODE(x)		(x)		/* this is probably wrong */

#define BIT_CLR		PIX_CLR
#define BIT_SET		PIX_SET
#define BIT_SRC		PIX_SRC
#define BIT_DST		PIX_DST

int bit_point();	/* sun doesn't have a point routine */

#ifdef COLOR
#define DEPTH				8				/* bits per pixel */
#define BITS				7				/* row padding for bitmaps */
#define NOCOLOR         0x1F
#define GETCOLOR(x)     ((x)<<5)
#define PUTCOLOR(x)     ((x)>>5)
/* the internal alignment of monochrome bitmaps seems to have changed from
   16 to 32 bits on OS 4.0 when using the color frame buffer, although 
	alignment using mpr_static() is still 16 bits
 */
#ifdef pixrect_hs_DEFINED		/* defined only on OS >= 4.0 */
# define BIT_Size(w,h,d)	(d==1 ? (((w+31)&~31)*h>>3) : w*h*d>>3)
#else
# define BIT_Size(w,h,d)	(d==1 ? (((w+15L)&~15)*h>>3) : w*h*d>>3)
#endif

#define ROP_INVERT(x)   \
   ((255-(255&(x>>5)))<<5) | (x)&NOCOLOR
#else
#define DEPTH				1				/* bits per pixel */
#define BITS				15				/* row padding for bitmaps */
#define NOCOLOR         0x1F
#define GETCOLOR(x)     (0)
#define PUTCOLOR(x)     (0)
#define BIT_Size(wide,high,d)     (((wide+BITS)&~BITS)*high>>3) /* bytes */
#define ROP_INVERT(x)   \
   ((x)&~0x1E | GET_OP(rev_ops[((x)>>1)&0xF]))
#endif
