/* SUN dependent part of the monochrome bitblit code */
/* derived from the sunport or colorport code by Uhler */

#undef _POSIX_SOURCE

#include <sys/types.h>
#include <sys/ioctl.h>
#include <sun/fbio.h>
#include <sys/file.h>
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include "sun.h"

#ifdef PIXRECT
#include <pixrect/pixrect_hs.h>
#endif


#define dprintf   if(bit_debug)fprintf
extern int bit_debug;
static int mmap_length;      /* length of fb to map in bytes */

extern char *getenv();
extern int ioctl();
extern int getpagesize();
extern caddr_t mmap();
extern int munmap();

void bit_textscreen(void) {}
void bit_grafscreen(void) {}

/* open the display; it looks like memory */

DATA *
bit_initscreen(char *name, int *width, int *height, unsigned char *depth, void **devi)
/* argument 'name' is the filename of the frame buffer character device */
{
   int fd;              /* file descriptor of frame buffer */
   register DATA *addr; /* address of frame buffer */
   struct fbtype buff;  /* frame buffer parameters */
   int pagemask;  
   int on = FBVIDEO_ON;

   /* open the SUN display */

   bit_debug = getenv("BIT_DEBUG") != NULL;

   if ((fd = open(name, O_RDWR)) < 0) {
      dprintf(stderr,"Cant open frame buffer %s\n",name);
      return ((DATA *)0);
      }

   /* get the frame buffer size */

   if (ioctl(fd, FBIOGTYPE, &buff) < 0) {
      dprintf(stderr,"FBIOGTYPE ioctl\n");
      return ((DATA *)0);
      }

   if (ioctl(fd, FBIOSVIDEO, &on) < 0) {
      dprintf(stderr,"cant enable video ioctl\n");
      return((DATA *)0);
      }

   /* align space (and fb size) on a page boundary */

   pagemask = getpagesize() - 1;
   mmap_length = (buff.fb_size+pagemask) &~ pagemask;

   dprintf(stderr,"size: %d x %d x %d (%d bytes)\r\n",
      buff.fb_width, buff.fb_height, buff.fb_depth,mmap_length);

   /* map the frame buffer in */

   addr = (DATA *) mmap(NULL, mmap_length,
                   PROT_READ|PROT_WRITE, _MAP_NEW|MAP_SHARED, fd, 0);
   if (addr == (DATA *)(-1)) {
      dprintf(stderr,"mapped failed\r\n");
      return ((DATA *)0);
      }

   *width = buff.fb_width;
   *height = buff.fb_height;
   *depth = buff.fb_depth;
#ifdef PIXRECT
   *devi = pr_open(name);
#else
   *devi = NULL;
#endif

   return (addr);
}

/* free resources required by the display */

void
display_close(bitmap)
BITMAP *bitmap;
{
   if (IS_SCREEN(bitmap) && IS_PRIMARY(bitmap)) {
#ifdef PIXRECT
      if(bitmap->deviceinfo) {
         pr_close((struct pixrect *)bitmap->deviceinfo);
         bitmap->deviceinfo = NULL;
      }
#endif
      munmap(BIT_DATA(bitmap), mmap_length);
   }
}


/* stub palette handling routines */

/* returns the color index in the color lookup table of the foreground */
unsigned int fg_color_idx( void){ return 1;}

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

