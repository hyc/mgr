/*{{{}}}*/
#include "vga.h"

/*{{{  #defines*/

/* VGA index register ports */
#define PEL_IW  0x3C8   /* PEL Write Index */
#define PEL_IR  0x3C7   /* PEL Read Index */

/* VGA data register ports */
#define PEL_D   0x3C9   /* PEL Data Register */
#define PEL_MSK 0x3C6	/* PEL mask register */

/*}}}  */

/*{{{  port_out*/
static inline void
port_out(char value, unsigned short port)
{
__asm__ volatile ("outb %0,%1"
		::"a" ((char) value),"d" ((unsigned short) port));
}
/*}}}  */
/*{{{  port_in*/
static inline unsigned char port_in(unsigned short port)
{
	unsigned char _v;
__asm__ volatile ("inb %1,%0"
		:"=a" (_v):"d" ((unsigned short) port));
	return _v;
}
/*}}}  */

static volatile int vga_delay_counter;
#define vga_delay() do { for( vga_delay_counter=0; vga_delay_counter<10; vga_delay_counter+=1) ;} while( 0)

/* returns the color index in the color lookup table of the foreground */
unsigned int fg_color_idx( void){ return 63;}

/* rescale computes a color index from the range [0..newmax]
   corresponding to a color index supplied in the range [0..oldmax].
   0 maps to 0 and oldmax maps to newmax.
   The expanding rescaling produces nearly equispaced outputs,
   and contracting rescaling maps nearly equal intervals to a single output.
   A rescaling followed by the reverse rescaling is a projection mapping.
 */
unsigned int rescale( unsigned int col, unsigned int oldmax,
					unsigned int newmax) {
    unsigned int newcol;

    if( newmax >= oldmax)
	newcol = oldmax? (col*2*newmax + oldmax) / (2*oldmax): newmax;
    else
	newcol = (col*2*(newmax + 1) + newmax) / (2*(oldmax + 1));
    return newcol;
}

/* The colors in the VGA palette have 6 bits */
void
setpalette(BITMAP *bp, unsigned int index, unsigned int red,
		unsigned int green, unsigned int blue, unsigned int maxi)
{
    if(!IS_SCREEN(bp))  return;

    /* select palette register */
    port_out(index, PEL_IW); 

    /* write RGB components */
    vga_delay();
    port_out( rescale( red, maxi, 63), PEL_D);
    vga_delay();
    port_out( rescale( green, maxi, 63), PEL_D);
    /*
     * writing the `blue' register will load the DAC.
     * Waiting for vertical or horizontal retrace will avoid disturbances
     * at this point if the screen is on.
     */
    vga_delay();
    port_out( rescale( blue, maxi, 63), PEL_D);
}


void
getpalette(BITMAP *bp, unsigned int index, unsigned int *red,
		unsigned int *green, unsigned int *blue, unsigned int *maxi)
{
    /* select palette register */
    port_out(index, PEL_IR);

    /* read RGB components */
    vga_delay();
    *red = (0x3F & (unsigned int) port_in(PEL_D));
    vga_delay();
    *green = (0x3F & (unsigned int) port_in(PEL_D));
    vga_delay();
    *blue = (0x3F & (unsigned int) port_in(PEL_D));
    /* the anding should be a no-op, modulo compiler glitches */
    *maxi = 0x3f;
}
