/* graphics print filter for Star, Epson & IBM printers (mgr format) */
/* version 2 - eliminate trailing white space */
/* version 3 - rotate fat pictures */

#include <stdio.h>
#include "dump.h"
#include <sgtty.h>
#include <memory.h>

#define MAX	720		/* maximum dots per line */
#define SETLF   "\0343%c\n\n\n" /* define line-feed spacing */
#define COUNT	"\033*\006%c%c"	/* bytes on this line */
#define RESETP  "\014\033@"     /* form-feed & reset printer */

char buff[MAX];

main(argc,argv)
int argc;
char **argv;
   {
     char epbuf[MAX];                           /* output buffer */
     register char epbit = 0;
     register char epbufbit;
     register char epworkc;
     register int eploopb;
     register int eploop;
     register int eplen;

   register int byteno;				/* byte number */
   register char *pntr;
   int mode;					/* tty local modes */
   int reverse = 0;				/* reverse bits */
   int rotate = 0;				/* rotate bitmap */
   int w,h,d;					/* bitmap size */
   int bytes;					/* bytes/line of bitmap */
   int lastbyte;				/* last significant byte */
   unsigned char mask;				/* to mask last byte */
   char mask1;					/* to mask last byte */
   int speed=0;					/* set speed to 9600 baud */
   int clean();
   char *sprintf();
   FILE *input, *popen();			/* for rotate filter */

   if (argc>1 && strcmp(argv[1],"-r")==0)
      reverse++;
   else if (argc>1 && strcmp(argv[1],"-s")==0)
      speed++;

   if (argc>2 && strcmp(argv[2],"-r")==0)
      reverse++;
   else if (argc>2 && strcmp(argv[2],"-s")==0)
      speed++;

#ifdef SYSV
   setbuf(stdin, NULL); /* no buffering to avoid problems with popen */
#endif
   if (!bitmaphead( stdin, &w, &h, &d, &bytes )) {
      fprintf(stderr,"%s: invalid bitmap format \n",*argv);
      exit(2);
      }

   /* rotate bitmap, if short and fat */
   sprintf(buff,"rotate -w %d -h %d",w,h);
   if (w>MAX && h<MAX && 
            (input = popen(buff,"r"))) {
      rotate++;
      if (!bitmaphead( input, &w, &h, &d, &bytes )) {
         fprintf(stderr,"%s: invalid bitmap format after rotate \n",*argv);
         exit(2);
         }
      }
   else
      input = stdin;
      

   mask = w%8 ? 0xff >> (w%8) : 0;
   mask1 = ~(mask & 0xff);
   lastbyte = w/8;

   /* set up the printer */

#ifndef SYSV	/* Just make this work as a filter, not device dependent!*/
   ioctl(1,TIOCLGET,&mode);
   mode |= LLITOUT;
   ioctl(1,TIOCLSET,&mode);
   if (speed)
      set_speed(1,B9600);
#endif

   printf(SETLF, 50);
   
   while (fread(buff,bytes,1,input) > 0)  {
     if (epbit == 0) {
       epbit = 8;
       eplen = 0;
       memset(epbuf, 0, MAX);
     }
     epbit--;
     if (reverse)
         invert(buff,bytes);
      buff[lastbyte] &= mask1;
      for(byteno=lastbyte, pntr=buff+byteno; byteno>=0; byteno--)
         if (*pntr--)
            break;
     if ((byteno+1)*8 > eplen)
       eplen = (byteno+1)*8;

     for (eploop = 0, eploopb = 0; eploop < byteno+1; eploop++) {
       epworkc = buff[eploop];
       for (epbufbit = 7; epbufbit >= 0; epbufbit--, eploopb++)
	 epbuf[eploopb] |= (epworkc & (1 << epbufbit)) ? 1 << epbit : 0;
     }
     if (epbit == 0) {
       printf(COUNT, (char) ((eplen) % 256), (char) ((eplen) / 256));
       fwrite(epbuf,eplen,1,stdout);
       printf("\n");
     }
   }

   if (epbit != 0) {
     printf(COUNT, (char) ((eplen) % 256), (char) ((eplen) / 256));
     fwrite(epbuf,eplen,1,stdout);
     printf("\n");
   }

   printf(RESETP);
   
#ifndef SYSV	/* Just make this work as a filter, not device dependent!*/
   mode &= ~LLITOUT;
   /* ioctl(1,TIOCLSET,&mode); */
#endif
   if (rotate)
      pclose(input);
   exit(0);
   }

/* invert each bit */

invert(data,count)
register unsigned char *data;
int count;
   {
   register unsigned char *end = data + count;

   while(data < end)
      *data++ = ~*data;
   }


/*
 *	Set the terminal speed 
 */

set_speed(file,speed)
int file;		/* file pointer */
int speed;
{
#ifndef SYSV	/* Just make this work as a filter, not device dependent!*/
	struct sgttyb buff;

	gtty(file,&buff);
	buff.sg_ospeed  = speed;
	buff.sg_ispeed  = speed;
	stty(file,&buff);
#endif
        return(0);
}
