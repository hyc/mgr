/*                        Copyright (c) 1988 Bellcore
 *                            All Rights Reserved
 *       Permission is granted to copy or use this program, EXCEPT that it
 *       may not be sold for profit, the copyright notice must be reproduced
 *       on copies, and credit should be given to Bellcore where it is due.
 *       BELLCORE MAKES NO WARRANTY AND ACCEPTS NO LIABILITY FOR THIS PROGRAM.
 */
/*	$Header: overlayd.c,v 4.1 88/06/22 14:37:59 bianchi Exp $
	$Source: /tmp/mgrsrc/demo/misc/RCS/overlayd.c,v $
*/
static char	RCSid_[] = "$Source: /tmp/mgrsrc/demo/misc/RCS/overlayd.c,v $$Revision: 4.1 $";

/*	overlayd		(S A Uhler)
 *
 *	turn off overlay plane anytime /dev/console gets scrunged (cgfour only)
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/file.h>
#include <sys/mman.h>
#include <stdio.h>

#define CONSOLE	 "/dev/console"
#define DISPLAY	"/dev/fb"
#define BITS	(1152*900)	/* bits in a plane */
#define SLEEP	5		/* polling interval */

#define INIT	0	/* ok to init frame buffer */
#define OK	1	/* frame buffer ok, do it */
#define BAD	2	/* couldn't get fb, punt */


main(argc,argv)
int argc;
char **argv;
   {
   struct stat statb;
   int fd;
   long mod_time;
   long now;

   switch (fork()) {
   default:	/* parent */
      exit(0);
   case -1:	/* fork() error */
      perror( *argv );
      exit(1);
   case 0: 	/* child */

      /* fix environment */

      fd = open("/dev/tty");
      ioctl(fd,TIOCNOTTY,0);
      close(fd);
      close(0); close(1); close(2);
      chdir("/");

      overlay(DISPLAY,0);
      stat(CONSOLE,&statb);
      mod_time = statb.st_mtime;

      while(1) {
         stat(CONSOLE,&statb);
         sleep(SLEEP);
         if (mod_time < statb.st_mtime) {
            mod_time = statb.st_mtime;
            overlay(DISPLAY,0);
            }
         }
      }
   }

static int state = INIT;

overlay(name,how)
char *name;
int how;
   {
   int fd;
   char  *malloc();
   static int *addr = (int * ) 0;	/* starting address of frame buffer */
   register int *start,*end;	/* range of addresses to change */
   int size,temp,pagesize;
   static int bits = BITS;

   /* open the SUN screen */

   switch(state) {
      case BAD:
         return(-1);
         break;
      case INIT:				/* first call, get fb */
         state = BAD;
         if ((fd = open(name,O_RDWR)) <0) {
            perror(name);
            return(-1);
            }

         /* malloc space for frame buffer  -- overlay and enable planes */

         pagesize = getpagesize();
         size = bits >> 2;	/* bitplane size in bytes  * 2 */
         size = (size+pagesize-1) &~ (pagesize-1);	/* round to next page */

         if ((temp = (int) malloc(size+pagesize)) == 0) {
            perror("malloc");
            return(-1);
            }
   
         /* align space on a page boundary */
   
         addr = (int *)(((unsigned int)temp+pagesize-1) & ~(pagesize-1));
   
         /* map the frame buffer into malloc'd space */
   
         if (mmap(addr,size,PROT_WRITE,MAP_SHARED,fd,0) < 0) {
            perror("mmap");
            free(temp);
            return(-1);
            }
         state = OK;
         /* no break */
      case OK:		 /* write data to plane */

         start = addr + 128*1024/4;		/* start of enable plane */
         end =   start +(bits>>5);		/* end of enable plane */

         while(start < end)
            *start++ = how;
      }
   }
