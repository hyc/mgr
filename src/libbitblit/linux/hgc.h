#define DATA unsigned char

#include <mgr/bitblit.h>

extern DATA *graph_mem;

#define LOGBITS 3
#define BITS (~(~(unsigned)0<<LOGBITS))

#define bit_linesize(wide,depth) (((depth*wide+BITS)&~BITS)>>3)

#define BIT_SIZE(m) BIT_Size(BIT_WIDE(m), BIT_HIGH(m), BIT_DEPTH(m))
#define BIT_Size(wide,high,depth) (((((depth)*(wide)+BITS)&~BITS)*(high))>>3)
#define BIT_LINE(x) ((((x)->primary->depth*(x)->primary->wide+BITS)&~BITS)>>LOGBITS)

#define NEED_ADJUST
#define FB_AD(bp,pp) (IS_SCREEN(bp)? adjust(pp): (pp))

extern void display_close(BITMAP *bitmap);

/*
 * Adjust the screen address for hercules monochrome card
 * From: Chi-Ting Lam <chiting@u.washington.edu>
 */

inline static DATA *adjust(p) DATA *p;
{
#ifdef HGC_ADJ_ASM
  register DATA *p1 asm("eax");

  __asm__ volatile ("movl %2,%%eax
		     subl %1,%%eax
                     movl $90,%%esi
                     xorl %%edx,%%edx
                     divl %%esi
                     movl %%eax,%%esi            / esi is row=p/90 
                     andl $3,%%eax               / 
                     sall $13,%%eax              /
                     addl %%edx,%%eax            / add reminder
                     sarl $2,%%esi               / row >> 2
                     leal (%%esi,%%esi,8),%%esi  / * 90
                     addl %%esi,%%eax
                     leal (%%esi,%%esi,8),%%esi
                     addl %%esi,%%eax
                     addl %1,%%eax"
		    : "=a" ((int) p1)
		    : "g1" ((int) graph_mem), "g2" ((int) p)
                    : "eax", "edx", "esi");

  return p1;
#else
  unsigned int po = p - graph_mem;
  unsigned int row = po / 90;
  return (((row & 3) << 13)
	  + ((row >> 2) * 90)
	  + (po % 90)
	  + graph_mem);
#endif /* HGC_ADJ_ASM */
}
