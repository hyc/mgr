/*                        Copyright (c) 1988 Bellcore
 *                            All Rights Reserved
 *       Permission is granted to copy or use this program, EXCEPT that it
 *       may not be sold for profit, the copyright notice must be reproduced
 *       on copies, and credit should be given to Bellcore where it is due.
 *       BELLCORE MAKES NO WARRANTY AND ACCEPTS NO LIABILITY FOR THIS PROGRAM.
 */
/*	$Header: overlay.c,v 1.1 88/07/08 11:56:58 sau Exp $
	$Source: /tmp/mgrsrc/demo/icon/RCS/overlay.c,v $
*/
static char	RCSid_[] = "$Source: /tmp/mgrsrc/demo/icon/RCS/overlay.c,v $$Revision: 1.1 $";

/*	overlay		(S A Uhler)
 *
 *	enable/disable color overlay plane  (wrong version)
 */

#include <sys/types.h>
#include <sys/ioctl.h>
#include <sun/fbio.h>
#include <sys/file.h>
#include <sys/mman.h>
#include <stdio.h>

#define SCREEN	"/dev/cgfour0"	/* name of sun frame buffer */

main(argc,argv)
int argc;
char **argv;
   {
   int how;

   if (argc < 2) {
      fprintf(stderr,"usage: %s [on|off]\n",*argv);
      exit(3);
      }

   if (strcmp(argv[1],"on") == 0)
      how = -1;
   else
      how = 0;

   overlay(how);
   }

overlay(how)
register int how;
   {
   int fd;
   char  *malloc();
   register int *start,*end;
   int *addr;
   struct fbtype buff;
   int size,temp,pagesize;
   int bits;

   /* open the SUN screen */

   if ((fd = open(SCREEN,O_RDWR)) <0) {
      fprintf(stderr,"Can't open %s\n",SCREEN);
      exit(1);
      }

   /* get the frame buffer size */

   if (ioctl(fd,FBIOGTYPE,&buff) < 0) {
      fprintf(stderr,"Can't get %s parameters\n",SCREEN);
      exit(2);
      }
   /* sun returns the wrong value ...
   if (buff.fb_type != FBTYPE_SUN4COLOR) {
      fprintf(stderr,"Wrong frame buffer type (%d)\n",buff.fb_type);
      exit(4);
      }
   */
   /* malloc space for frame buffer  -- overlay and enable planes */

   pagesize = getpagesize();
   bits = buff.fb_width * buff.fb_height;			/* pixels/plane */
   size = bits >> 2;	/* bitplane size in bytes  * 2 */
   size = (size+pagesize-1) &~ (pagesize-1);		/* round up to next page */

   if ((temp = (int) malloc(size+pagesize)) == 0) {
      fprintf(stderr,"couldn't malloc %d bytes\n",size+pagesize);
      exit(3);
      }

   /* align space on a page boundary */

   addr = (int *)(((unsigned int)temp+pagesize-1) & ~(pagesize-1));

   /* map the frame buffer into malloc'd space */

   if (mmap(addr,size,PROT_WRITE,MAP_SHARED,fd,0) < 0) {
      perror("mmap");
      exit(5);
      }
  
   /* write data to plane */

   start = addr + (1024*128/4); /* start of enable  plane */
   end =   start +(bits>>5);	/* end of enable plane */

   while(start < end) {
      *start++ = how;
      }

   /* clean up and exit */

   munmap(addr,buff.fb_size);
   exit(0);
   }
