/*{{{}}}*/
/*{{{  #includes*/
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/kd.h>
#include <sys/stat.h>
#include <sys/vt.h>
#include <sys/sysmacros.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>

#include "screen.h"
/*}}}  */
/*{{{  #defines*/

#define GRAPH_BASE 0xA0000
#define GRAPH_SIZE 0x10000
#define TEXT_BASE  0xB8000
#define TEXT_SIZE  0x08000
#define FONT_BASE  0xA0000
#define FONT_SIZE  0x02000

/* VGA index register ports */
#define CRT_IC  0x3D4   /* CRT Controller Index - color emulation */
#define CRT_IM  0x3B4   /* CRT Controller Index - mono emulation */
#define CRT_I   CRT_IC  /* CRT Controller Index */
#define ATT_IW  0x3C0   /* Attribute Controller Index & Data Write Register */
#define GRA_I   0x3CE   /* Graphics Controller Index */
#define SEQ_I   0x3C4   /* Sequencer Index */
#define PEL_IW  0x3C8   /* PEL Write Index */
#define PEL_IR  0x3C7   /* PEL Read Index */

/* VGA data register ports */
#define CRT_DC  0x3D5   /* CRT Controller Data Register - color emulation */
#define CRT_DM  0x3B5   /* CRT Controller Data Register - mono emulation */
#define CRT_D   CRT_DC  /* CRT Controller Data Register */
#define ATT_R   0x3C1   /* Attribute Controller Data Read Register */
#define GRA_D   0x3CF   /* Graphics Controller Data Register */
#define SEQ_D   0x3C5   /* Sequencer Data Register */
#define MIS_R   0x3CC   /* Misc Output Read Register */
#define MIS_W   0x3C2   /* Misc Output Write Register */
#define IS1_RC  0x3DA   /* Input Status Register 1 - color emulation */
#define IS1_RM  0x3BA   /* Input Status Register 1 - mono emulation */
#define IS1_R   IS1_RC  /* Input Status Register 1 */
#define PEL_D   0x3C9   /* PEL Data Register */
#define PEL_MSK 0x3C6	/* PEL mask register */

/* VGA indexes max counts */
#define CRT_C   24      /* 24 CRT Controller Registers */
#define ATT_C   21      /* 21 Attribute Controller Registers */
#define GRA_C   9       /* 9  Graphics Controller Registers */
#define SEQ_C   5       /* 5  Sequencer Registers */
#define MIS_C   1       /* 1  Misc Output Register */

/* VGA registers saving indexes */
#define CRT     0               /* CRT Controller Registers start */
#define ATT     CRT+CRT_C       /* Attribute Controller Registers start */
#define GRA     ATT+ATT_C       /* Graphics Controller Registers start */
#define SEQ     GRA+GRA_C       /* Sequencer Registers */
#define MIS     SEQ+SEQ_C       /* General Registers */
#define END     MIS+MIS_C       /* last */

#define SETGRA(x,y)	{port_out(x,GRA_I);port_out(y,GRA_D);}
#define SETSEQ(x,y)	{port_out(x,SEQ_I);port_out(y,SEQ_D);}
#define SETATT(x,y)	{port_in(IS1_R);port_out(x,ATT_IW),port_out(y,ATT_IW);}
/*}}}  */

/*{{{  port_out*/
static inline void
port_out(char value, unsigned short port)
{
__asm__ volatile ("outb %0,%1"
		::"a" ((char) value),"d" ((unsigned short) port));
}
/*}}}  */
/*{{{  port_outw*/
static inline void
port_outw( int value, int port ) {
	__asm__ volatile("outw %0,%1"
	: : "a" ((unsigned short)value), "d" ((unsigned short)port));
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

/*{{{  vga mode data*/
struct mode_record {
  char *name;
  int width,height,depth;
  char reg[60];
};

static struct mode_record mode_regs[] = {
  {
    "320x200x256",320,200,8,
    {
#include "320x200x256.h"
    }
  },
#ifdef TSENG4K
/* modes for zeos diamond speedstar, tseng et4000 chip */
  {
    "640x480x256",640,480,8,
    {
#include "640x480x256.h"
    }
  },
  {
    "800x600x256",800,600,8,
    {
#include "800x600x256.h"
    }
  },
  {
    "1024x768x256",1024,768,8,
    {
#include "1024x768x256.h"
    }
  },
#endif
#ifdef S3
/* modes for Fahrenheit Orchid 1280, S3 chip */
  {
    "640x480x256",640,480,8,
    {
#include "640x480x256.h"
    }
  },
  {
    "800x600x256",800,600,8,
    {
#include "800x600x256.h"
    }
  },
  {
    "1024x768x256",1024,768,8,
    {
#include "1024x768x256.h"
    }
  },
#endif /* S3 */
};
/*}}}  */

/*{{{  variables*/
static struct mode_record *cur_mode;
static int mem_fd;
DATA *graph_mem;
static char *text_mem;
int console_fd;
int console_nr;

extern int bit_debug;

/* Save buffers */
static char text_regs[60];
static char text_buf[TEXT_SIZE];   /* saved text mode memory */  
static char font_buf1[FONT_SIZE];   /* saved font data        */
static char font_buf2[FONT_SIZE];
/*}}}  */

/*{{{  screenon*/
void screenon(){  port_in(IS1_R);  port_out(0x20,ATT_IW);}
/*}}}  */
/*{{{  screenoff*/
void screenoff(){  port_in(IS1_R);  port_out(0x00,ATT_IW);}
/*}}}  */
/*{{{  setplane*/
void setplane(int plane){  SETSEQ(0x02,1<<plane);  SETGRA(0x04,plane);}
/*}}}  */
/*{{{  setrplane*/
void setrplane(int plane){SETGRA(0x04,plane);}
/*}}}  */
/*{{{  setwplane*/
void setwplane(int plane){SETSEQ(0x02,1<<plane);}
/*}}}  */
/*{{{  set_regs*/
static int set_regs(char regs[])
{
    int i;

    port_out(0x00,GRA_I); 
    port_out(0x00,GRA_D);  		/* set/reset                        */
    port_in(IS1_R);  	 		/* clear flip-flop                  */
    port_out(0x00,SEQ_I); 
    port_out(0x01,SEQ_D); 		/* synchronous reset on             */
    port_out(regs[MIS+0], MIS_W); 	/* update misc output register      */
    port_out(0x1, SEQ_I); 
    port_out(regs[SEQ+1], SEQ_D);  	/* update clocking mode             */
    for (i = 2; i < SEQ_C; i++) {	/* sequencer registers              */
        port_out(i, SEQ_I); 
        port_out(regs[SEQ+i], SEQ_D); 
    }
    port_out(0x11, CRT_I); 		  
    port_out(regs[CRT+0x11]&0x7F, CRT_D);   /* deprotect registers 0-7      */
    for (i = 0; i < CRT_C; i++) { 	/* CRT controller registers 	    */
        port_out(i, CRT_I); 
        port_out(regs[CRT+i], CRT_D); 
    }
    for (i = 0; i < GRA_C; i++) { 	/* graphics controller registers    */
        port_out(i, GRA_I); 
        port_out(regs[GRA+i], GRA_D); 
    }
    for (i = 0; i < ATT_C; i++) {       /* attribute controller registers   */
        port_in(IS1_R);          	/* reset flip-flop                  */
        port_out(i, ATT_IW);
        port_out(regs[ATT+i],ATT_IW);
    }
    port_out(0x00, SEQ_I); 
    port_out(0x03, SEQ_D);   		/* synchronous reset off            */
    return 0;
}
/*}}}  */
/*{{{  write_mode*/
void write_mode(mode)
     int mode;
{
  SETGRA(0x05,mode);
}
/*}}}  */
/*{{{  setmapmask*/
void setmapmask(mask)
     int mask;
{
  SETSEQ(0x02,mask);
}
/*}}}  */

#ifdef BANKED
static char *bankedsrcbuf = NULL; /* temp copy of bits blitted from screen */
static unsigned char last_page = 0 /* transient glitch if init val wrong */;

#ifdef TSENG4K

#define TSENG4K_SEG_SELECT 0x3CD

/* Bank switching function - set 64K bank number */
extern void tseng4k_setpage( unsigned char page) {

    page &= 0x0F;
    page |= page << 4;
    if( page != last_page)
	port_out( last_page = page, TSENG4K_SEG_SELECT);
}


/* Bank switching function - set 64K read bank number */
extern void tseng4k_setrdpage( unsigned char page) {

    page = (last_page & 0x0f) | ((page & 0x0f) << 4);
    if( page != last_page)
	port_out( last_page = page, TSENG4K_SEG_SELECT);
}

/* Bank switching function - set 64K write bank number */
extern void tseng4k_setwrpage( unsigned char page) {

    page = (last_page & 0xf0) | (page & 0x0f);
    if( page != last_page)
	port_out( last_page = page, TSENG4K_SEG_SELECT);
}

#endif /* TSENG4K */
#ifdef S3

/* this code is an interpretation of the assembler in X11r6p5-xf86-3.1 in
 * xc/programs/Xserver/hw/xfree86/vga256/drivers/s3_svga/s3_bank.s
 */
#define S3_SEG_SELECT 0x35

extern void s3_setpage( unsigned char page) {
    
    if( page != last_page) {
	port_out( S3_SEG_SELECT, CRT_I);
	port_out( last_page = page, CRT_D);
    }
}

#endif /* S3 */
#ifdef TVGA

/* this code is an interpretation of the assembler in X11r6p5-xf86-3.1 in
 * xc/programs/Xserver/hw/xfree86/vga256/drivers/tvga8900/bank.s
 */

extern void tvga_setpage( unsigned char page) {
    
    if( page != last_page) {
	port_outw( 0x020e ^ (page << 8), SEQ_I);
	last_page = page;
    }
}

#endif /* TVGA */
#endif /* BANKED */


/*{{{  bit_initscreen*/
/*
 * bit_initscreen finds the desired screen mode from its name,
 * sets io register permissions, mmap's the screen frame buffer,
 * and returns the buffer address and the screen dimensions for this mode.
 * The VGA is not initialized into the chosen screen mode here.
 */
DATA *
bit_initscreen(char *name, int *width, int *height, unsigned char *depth, void **devi)
{
    struct mode_record *p;
    struct stat buf;
    unsigned long int pagesize, rest;

    bit_debug = getenv("BIT_DEBUG") != NULL;

    /*{{{  open console*/
    if ((console_fd=open("/dev/console",O_RDWR))==-1)
      return((DATA*)0);
    if (stat(ttyname(0),&buf)==-1)
      return((DATA*)0);
    console_nr=minor(buf.st_rdev);
    /*}}}  */

    cur_mode = mode_regs;	/* Figure out what mode we want. */
    for (p = mode_regs; p->name; p++) {
      if (!strcmp(p->name,name)) {
        cur_mode = p;
        break;
      }
    }
    /* Set permissions and memory maps */
  
    /* get I/O permissions for VGA registers */
    if (ioperm(CRT_I, 1, 1)) {
      printf("init: can't get I/O permissions \n");
      exit (-1);
    }
    ioperm(ATT_IW, 1, 1);
    ioperm(GRA_I,  1, 1);
    ioperm(SEQ_I,  1, 1);
    ioperm(PEL_IR, 1, 1);
    ioperm(PEL_IW, 1, 1);
    ioperm(CRT_D,  1, 1);
    ioperm(ATT_R,  1, 1);
    ioperm(GRA_D,  1, 1);
    ioperm(SEQ_D,  1, 1);
    ioperm(MIS_R,  1, 1);
    ioperm(MIS_W,  1, 1);
    ioperm(IS1_R,  1, 1);
    ioperm(PEL_D,  1, 1);
#ifdef BANKED
#ifdef TSENG4K
    ioperm(TSENG4K_SEG_SELECT, 1, 1);
#endif
    /* other chips' special registers here */
#endif
  
    /* open /dev/mem */
    if ((mem_fd = open("/dev/mem", O_RDWR) ) < 0) {
      printf("init: can't open /dev/mem \n");
      exit (-1);
    }
    /* mmap graphics memory */
    pagesize = getpagesize();
    if ((graph_mem = malloc(GRAPH_SIZE + pagesize)) == NULL)
      return((DATA*)0);
    rest = (unsigned long)graph_mem % pagesize;
    if (rest != 0)
      graph_mem = (DATA *) (((unsigned long)graph_mem) + pagesize - rest);
    graph_mem = (DATA *)mmap((caddr_t)graph_mem, 
			     GRAPH_SIZE,
			     PROT_READ|PROT_WRITE,
			     MAP_SHARED|MAP_FIXED,
			     mem_fd, 
			     (off_t)GRAPH_BASE);
    if ((long)graph_mem < 0) {
      printf("init: mmap error \n");
      exit (-1);
    }

    /* mmap text memory */
    if ((text_mem = malloc(TEXT_SIZE + pagesize - 1)) == NULL) {
      printf("init: allocation error \n");
      exit (-1);
    }
    if ((unsigned long)text_mem % pagesize)
      text_mem += pagesize - ((unsigned long)text_mem % pagesize);
    text_mem = (unsigned char *)mmap((caddr_t)text_mem, 
				   TEXT_SIZE,
				   PROT_READ|PROT_WRITE,
				   MAP_SHARED|MAP_FIXED,
				   mem_fd, 
				   (off_t)TEXT_BASE);
    if ((long)text_mem < 0) {
      printf("init: mmap error \n");
      exit (-1);
    }
  *width=cur_mode->width;
  *height=cur_mode->height;
  *depth=cur_mode->depth<8?1:cur_mode->depth;
  *devi=NULL;
#ifdef BANKED
  bankedsrcbuf = (char *)malloc( (((*width)*(*depth)+BITS)>>LOGBITS)
				 * sizeof( DATA));
#endif
  return graph_mem;
}
/*}}}  */
/*{{{  bit_grafscreen*/
void bit_grafscreen(void)
{
  int i;

  /*{{{  block vc -- more a hack ...*/
  ioctl(console_fd,VT_ACTIVATE,console_nr);
  ioctl(console_fd,VT_WAITACTIVE,console_nr);
  ioctl(console_fd,KDSETMODE,KD_GRAPHICS);
  /*}}}  */
  screenoff();
  for (i = 0; i < CRT_C; i++) {
    port_out(i, CRT_I); 
    text_regs[CRT+i] = port_in(CRT_D); 
  }
  for (i = 0; i < ATT_C; i++) {
    port_in(IS1_R);
    port_out(i, ATT_IW); 
    text_regs[ATT+i] = port_in(ATT_R); 
  }
  for (i = 0; i < GRA_C; i++) {
    port_out(i, GRA_I); 
    text_regs[GRA+i] = port_in(GRA_D); 
  }
  for (i = 0; i < SEQ_C; i++) {
    port_out(i, SEQ_I); 
    text_regs[SEQ+i] = port_in(SEQ_D); 
  }
  text_regs[MIS] = port_in(MIS_R); 
  memcpy(text_buf,text_mem,TEXT_SIZE);
  set_regs(cur_mode->reg);
  /* save font data */
  port_out(0x04, GRA_I); 
  port_out(0x02, GRA_D); 
  memcpy(font_buf1, graph_mem, FONT_SIZE);
  port_out(0x04, GRA_I); 
  port_out(0x03, GRA_D); 
  memcpy(font_buf2, graph_mem, FONT_SIZE);
#ifdef NOTDEF
  /* restore map mask register */
  port_out(0x04, GRA_I);
  port_out(0x00, GRA_D); 
  SETGRA(0x01,0x00); /* Disable Set/Reset */
  SETGRA(0x03,0x00); /* Replace/No Rotate */
  write_mode(0x00);  /* Set write mode/read mode 0 */
  SETGRA(0x08,0xff); /* No bit masking */
  setmapmask(0xff);  /* Doens't really matter */
#endif
  screenon();
}
/*}}}  */
/*{{{  bit_textscreen*/
void bit_textscreen(void)
{
  unsigned char tmp;
  screenoff();
  /* write to all bits */
  port_out(0x08, GRA_I ); 
  port_out(0xFF, GRA_D );   
  /* disable Set/Reset Register */
  port_out(0x01, GRA_I ); 
  port_out(0x00, GRA_D );   
  port_out(0x04,SEQ_I);
  tmp = port_in(SEQ_D) & 0xf7;
  port_out(0x04,SEQ_I);
  port_out(tmp,SEQ_D);
  /* restore character map in plane 2 */
  port_out(0x02, SEQ_I ); 
  port_out(0x04, SEQ_D ); 
  memcpy(graph_mem, font_buf1, FONT_SIZE);
  port_out(0x02, SEQ_I );
  port_out(0x08, SEQ_D );
  memcpy(graph_mem, font_buf2, FONT_SIZE);
  /* restore text mode VGA registers */
  set_regs(text_regs);
  /* restore contents of text mode memory */
  memcpy(text_mem, text_buf, TEXT_SIZE);
  screenon();
  ioctl(console_fd,KDSETMODE,KD_TEXT);
}  
/*}}}  */

/* free resources required by the display */

void
display_close(bitmap)
BITMAP *bitmap;
{
   if (IS_SCREEN(bitmap) && IS_PRIMARY(bitmap)) {
      bit_textscreen();
      munmap((caddr_t)text_mem, TEXT_SIZE);
      munmap((caddr_t)graph_mem, GRAPH_SIZE);
   }
}


#ifdef BANKED
/* routines for access to the VGA banked memory */

#define BANKSHIFT 16
#define OFFSETMASK ((1<<BANKSHIFT)-1)
#ifdef TSENG4K
#define setbank( b) tseng4k_setpage( b)
#endif
#ifdef S3
#define setbank( b) s3_setpage( b)
#endif

/*
 * getdata reads _bytes_ bytes from the screen frame buffer,
 * switching banks as needed, copying them to static store and returning
 * a pointer to this store.
 * dp0 is the start of the frame buffer, dp is the address relative to dp0.
 */
extern void *
getdata( void *dp, void *dp0, unsigned int bytes) {
    unsigned int off, bank, left;

    off = (char *)dp - (char *)dp0;
    bank = off >> BANKSHIFT;
    setbank( bank);
    off &= OFFSETMASK;
    left = OFFSETMASK + 1 - off;
    if( bytes < left)  left = bytes;
    memcpy( bankedsrcbuf, off + (char *)dp0, left);
    if( left < bytes) {
	setbank( bank + 1);
	memcpy( bankedsrcbuf + left, dp0, bytes - left);
    }
    return bankedsrcbuf;
}


/*
 * get_addr_fb makes the byte in the screen frame buffer which would be
 * linearly addressed as dp accessible in the current bank.
 * It returns the banked address at which the byte is really accessible.
 * Argument dp0 is the start of the frame buffer.
 * If argument lastbytep is nonNULL it points to an address which is
 * set to be the last/highest address in this bank.
 * 
 * get_addr_fb does the same except that it sets lastbytep to be
 * the first/lowest address in the desired bank of video RAM.
 */
extern void *
get_addr_fb( void *dp, void *dp0, void **lastbytep) {
    unsigned long int off = (char *)dp - (char *)dp0;

    setbank( off >> BANKSHIFT);
    if( lastbytep)
	*lastbytep = (off|OFFSETMASK) + (char *)dp0;
    return (off&OFFSETMASK) + (char *)dp0;
}

extern void *
get_addr_fb_bak( void *dp, void *dp0, void **lastbytep) {
    unsigned long int off = (char *)dp - (char *)dp0;

    setbank( off >> BANKSHIFT);
    if( lastbytep)
	    *lastbytep = (off&~OFFSETMASK) + (char *)dp0;
    return (off&OFFSETMASK) + (char *)dp0;
}

/*
 * getdatap makes the byte at address dp in the screen frame buffer,
 * relative to the buffer start address dp0, accessible in the current bank.
 * *bytesp is the number of bytes which need to be accessed, and this value
 * is decreased if dp+*bytesp extends past a bank boundary.
 */
extern void *
getdatap( void *dp, void *dp0, unsigned int *bytesp) {
    unsigned int off = (char *)dp - (char *)dp0;
    setbank( off >> BANKSHIFT);
    off &= OFFSETMASK;
    if( bytesp) {
	unsigned int left = OFFSETMASK + 1 - off;
	if( left < *bytesp)  *bytesp = left;
    }
    return off + (char *)dp0;
}


/* getdatabkp is like getdatap, but backwards, from dp to dp-*bytesp */
extern void *
getdatabkp( void *dp, void *dp0, unsigned int *bytesp) {
    unsigned int off = (char *)dp - (char *)dp0;
    setbank( off >> BANKSHIFT);
    off &= OFFSETMASK;
    if( bytesp) {
	unsigned int left = off + 1;
	if( left < *bytesp)  *bytesp = left;
    }
    return off + (char *)dp0;
}
#endif /* BANKED */
/* end vga */
