/* SUN dependent part of the generic color bitblit code */

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

#ifdef CG2
/* untested and speculative */
#include <pixrect/cg2reg.h>
#define FB_OFFSET 0
#define COLOR_SIZE(w,h)    ((8*(w) + 31)/32 * 4 * (h))
#define MAP_OFFSET CG2_MAPPED_OFFSET
#endif

#ifdef CG3
#define FB_OFFSET (256*1024)   /* size of overlay and enable planes in bytes */
#define COLOR_SIZE(w,h)    ((8*(w) + 31)/32 * 4 * (h))
#define MAP_OFFSET 0           /* not CG3_MMAP_OFFSET ? */
#endif

#ifdef CG6
#include <sundev/cg6reg.h>
#define FB_OFFSET MMAPSIZE(0)
#define COLOR_SIZE(w,h) ((w)*(h))
#define MAP_OFFSET CG6_VBASE
#endif

#define dprintf   if(bit_debug)fprintf
extern int bit_debug;
static int mmap_length;      /* length of fb to map in bytes */

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
   DATA *result;
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
   buff.fb_size = (buff.fb_size+pagemask) &~ pagemask;
   if (buff.fb_depth > 1) {   /* color of some kind or another */
      mmap_length = FB_OFFSET + COLOR_SIZE(buff.fb_width, buff.fb_height);
      mmap_length = (mmap_length + pagemask) &~ pagemask;
      }
   else
      mmap_length = buff.fb_size;

   dprintf(stderr,"size: %d x %d x %d (%d bytes)\r\n",
      buff.fb_width, buff.fb_height, buff.fb_depth,mmap_length);

   /* map the frame buffer into malloc'd space */

   addr = (DATA *) mmap(NULL, mmap_length,
                   PROT_READ|PROT_WRITE, _MAP_NEW|MAP_SHARED, fd, MAP_OFFSET);
   if (addr == (DATA *)(-1)) {
      dprintf(stderr,"mapped failed\r\n");
      return ((DATA *)0);
      }

   /* setup color or mono display */

   if (buff.fb_depth > 1)  /* color of some kind or another */
      result = addr + FB_OFFSET/sizeof(DATA);
   else
      result = addr;
      
   *width = buff.fb_width;
   *height = buff.fb_height;
   *depth = buff.fb_depth;
#ifdef PIXRECT
   *devi = pr_open(name);
#else
   *devi = NULL;
#endif

   return (result);
}

/* free resources required by the display */

void
display_close(bitmap)
BITMAP *bitmap;
   {
   if (IS_SCREEN(bitmap) && IS_PRIMARY(bitmap)) {
#ifdef PIXRECT
      if (bitmap->deviceinfo) {
         pr_close((struct pixrect *)bitmap->deviceinfo);
         bitmap->deviceinfo = NULL;
      }
#endif
      if (BIT_DEPTH(bitmap)>1)   /* color */
         munmap(BIT_DATA(bitmap) - FB_OFFSET/sizeof(DATA), mmap_length);
      else
         munmap(BIT_DATA(bitmap), mmap_length);
      }
   }
