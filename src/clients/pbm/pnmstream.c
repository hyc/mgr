/*{{{}}}*/
/*{{{  Copyright*/
/* pbmstream.c - reads a stream of portable bitmaps and forks
**               a program for each one
**
** Copyright (C) 1992 by Guido Muesch (odiug@pool.informatik.rwth-aachen.de)
**
*/
/*}}}  */

/*{{{  includes*/
#include "pnm.h"
#include <assert.h>
/*}}}  */
/*{{{  defines*/
#define BLOCKSIZE 8192
/*}}}  */
/*{{{  globals*/
static int verbose=0;
static int cols, rows, format;
/*}}}  */

/*{{{  int main(int argc, char *argv[])*/
int main(int argc, char *argv[])
{
  /*{{{  variables*/
  int row, page, i;
  FILE* fd_out;
  xelval maxval;
  char argvstr[256], sysstr[256], c;
  /*}}}  */

  pnm_init(&argc, argv);
  /*{{{  check -v flag*/
  if (argc>1 && !strcmp(argv[1], "-v"))
    verbose=1;
  /*}}}  */
  /*{{{  gather argvs*/
  i=1+verbose;
  while(1) {
    strcat(argvstr, argv[i++]);
    if(i==argc) break;
    strcat(argvstr, " ");
  }
  /*}}}  */

  page=1;

  while((c=getchar())!=EOF)
    /*{{{  write each bitmap to a fifo and start a program for it*/
    {
      ungetc(c, stdin);                    /* Maybe I am stupid ... (gjm) */

      pnm_readpnminit(stdin, &cols, &rows, &maxval, &format);
      /*{{{  open outputpipe*/
      sprintf(sysstr, argvstr, page);      /* Insert pagenumber if %d */
      if (verbose) puts(sysstr);

      if(!(fd_out = popen(sysstr, "w")))
        pm_error("could not popen '%s'\n", sysstr);
      /*}}}  */
      pnm_writepnminit(fd_out, cols, rows, maxval, format, 0);
      switch(format)
      {
        case RPBM_FORMAT:
        case RPGM_FORMAT:
        case RPPM_FORMAT:
          /*{{{  Copy raw formats as fast as possible*/
          {
            long size;

            int block[BLOCKSIZE];
            int blocks;

            switch (format) {
              case RPBM_FORMAT: size=rows*((cols-1)/8+1); break;
                                         /* pbm is bytealigned */
              case RPGM_FORMAT: size=rows*cols; break;
              case RPPM_FORMAT: size=rows*cols*3; break;
              default: size=0; assert(0);
            }
            blocks=size/BLOCKSIZE;       /* number of blocks */
            size%=BLOCKSIZE;             /* rest */

            if(verbose) puts("Using RAW");

            while(blocks--) {
              fread(block, 1, BLOCKSIZE, stdin);
              fwrite(block, 1, BLOCKSIZE, fd_out);
            }
            fread(block, 1,size, stdin);
            fwrite(block, 1,size, fd_out);
            break;
          }
          /*}}}  */
        default:
          /*{{{  Handle plain formats the traditional way*/
          {
            xel* xelrow;

            if(verbose) puts("Using Standard");
            xelrow=pnm_allocrow(cols);
            for (row=0; row<rows; row++) {
              pnm_readpnmrow(stdin, xelrow, cols, maxval, format);
              pnm_writepnmrow(fd_out, xelrow, cols, maxval, format, 0);
            }
            pnm_freerow(xelrow);
            break;
          }
          /*}}}  */
      }
      pclose(fd_out);
      page++;
    }
    /*}}}  */
  
  exit(0);
}
/*}}}  */
