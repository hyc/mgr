/* modified for Coherent 4.0 by Harry C. Pulley, IV */

/*{{{}}}*/
/*{{{  #includes*/

#ifndef COHERENT
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/kd.h>
#else
extern int ioperm();
extern char *mmap();
#define PAGE_SIZE 4096
#define getpagesize() PAGE_SIZE
#endif

#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>

#include "screen.h"
/*}}}  */
/*{{{  #defines*/
/* video ports */
#define INDEX_REG  0x3b4
#define DATA_REG   0x3b5
#define MODE_REG   0x3b8
#define CONFIG_REG 0x3bf

/* configurations */
#define TEXT_CONF 1
#define GRAPH_CONF 3

/* modes */
#define ACTIVE_MODE 0x08
#define TEXT_MODE 0x20
#define GRAPH_MODE0 0x02
#define GRAPH_MODE1 0x82

/* memory addresses */
#define GRAPH_MODE0_BASE 0xB0000
#define GRAPH_MODE1_BASE 0xB8000
#define GRAPH_SIZE 0x8000
/*}}}  */

/*{{{  port access*/
inline static void port_out(char value, unsigned short port)
{
__asm__ volatile ("outb %0,%1"
		::"a" ((char) value),"d" ((unsigned short) port));
}


inline static unsigned char port_in(unsigned short port)
{
	unsigned char _v;
__asm__ volatile ("inb %1,%0"
		:"=a" (_v):"d" ((unsigned short) port));
	return _v;
}

/*}}}  */
/*{{{  variables*/
int graphics_fd;
DATA *graph_mem;
/*}}}  */

/*{{{  bit_initscreen*/
DATA *
bit_initscreen(char *name, int *width, int *height, unsigned char *depth, void **devi)
{
  graphics_fd = open("/dev/bm",O_RDWR);
  if (graphics_fd == -1)
  {
    write(2,"Can't open /dev/bm\n",19);
    return((DATA*)0);
  }

  /*}}}  */
  /*{{{  mmap screen*/
  if ((graph_mem = malloc(GRAPH_SIZE + (getpagesize()))) == NULL) return((DATA*)0);
  if ((unsigned long)graph_mem % getpagesize()) graph_mem += getpagesize() - ((unsigned long)graph_mem % getpagesize());

  graph_mem = (DATA*)mmap(graph_mem,GRAPH_MODE0_BASE,GRAPH_SIZE);

  if ((long)graph_mem < 0) {
    write(2,"Can't map screen to memory\n",27);
    return((DATA*)0);
  }
  /*}}}  */

  /* change suggested by Vance Petree */
  memset(graph_mem,0,GRAPH_SIZE);

  ioperm(INDEX_REG, 1);
  ioperm(DATA_REG, 1);
  ioperm(MODE_REG,  1);
  ioperm(CONFIG_REG,  1);
  /*}}}  */
  *width=720;
  *height=348;
  *depth=1;
  *devi=NULL;
  return graph_mem;
}
/*}}}  */
/*{{{  bit_grafscreen*/
void bit_grafscreen(void)
{
  /*{{{  variables*/
  static char grfval[12]={0x35,0x2d,0x2e,0x7,0x5b,0x02,0x57,0x57,0x2,0x3,0x0,0x0};
  int regsel;
  /*}}}  */

  /*{{{  program 6845 for graphics mode*/
  port_out(GRAPH_MODE1,MODE_REG);
  for (regsel=0; regsel<12; regsel++)
  {
    port_out(regsel,INDEX_REG);
    port_out(grfval[regsel],DATA_REG);
  }
  port_out(GRAPH_CONF,CONFIG_REG);
  port_out(GRAPH_MODE1|ACTIVE_MODE,MODE_REG);
  /*}}}  */
}
/*}}}  */
/*{{{  bit_textscreen*/
void bit_textscreen(void)
{
  static char txtval[12]={0x61,0x50,0x52,0xf,0x19,0x06,0x19,0x19,0x2,0xd,0xb,0xc};
  int regsel;

  /*{{{  switch 6845 to text mode*/
  port_out(TEXT_MODE,MODE_REG);
  for (regsel=0; regsel<12; regsel++)
  {
    port_out(regsel,INDEX_REG);
    port_out(txtval[regsel],DATA_REG);
  }
  port_out(TEXT_CONF,CONFIG_REG);
  port_out(TEXT_MODE|ACTIVE_MODE,MODE_REG);
  /*}}}  */
  close(graphics_fd);
}
/*}}}  */

/* stub palette handling routines */

/* returns the color index in the color lookup table of the foreground */
unsigned int fg_color_idx( void){ return 1;} /* ? */

void
setpalette(BITMAP *bp,
	   unsigned int index,
	   unsigned int red,
	   unsigned int green,
	   unsigned int blue,
	   unsigned int maxi)
{ }


void
getpalette(BITMAP *bp,
	   unsigned int index,
	   unsigned int *red,
	   unsigned int *green,
	   unsigned int *blue,
	   unsigned int *maxi)
{
    *red = *green = *blue = 0;
    *maxi = 1;
}
