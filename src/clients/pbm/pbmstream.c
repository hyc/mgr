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
#include "pbm.h"
/*}}}  */

/*{{{  globals*/
static int verbose=0;
static int cols, rows, format;
/*}}}  */

/*{{{  int main(int argc, char *argv[])*/
int main(int argc, char *argv[])
{
  /*{{{  variables*/
  bit* bitrow;
  int row, page, i;
  FILE* fd_out;
  char argvstr[256], sysstr[256], c;
  /*}}}  */

  pbm_init(&argc, argv);

  strcpy(argvstr, "cat");
  if (argc>1 && !strcmp(argv[1], "-v"))
    verbose=1;

  for(i=1+verbose; i<argc; i++) {
    strcat(argvstr, " ");
    strcat(argvstr, argv[i]);
  }

  page=1;

  while((c=getchar())!=EOF)
  /*{{{  write each bitmap to a fifo and start a program for it*/
  {
    ungetc(c, stdin);           /* Maybe I am stupid ... (gjm) */

    pbm_readpbminit(stdin, &cols, &rows, &format);

    sprintf(sysstr,argvstr,page);
    if (verbose) puts(sysstr);

    fd_out = popen(sysstr, "w");

    pbm_writepbminit(fd_out, cols, rows, 0);

    switch(format)
    {
      case PBM_FORMAT:
         bitrow=pbm_allocrow(cols);
         for (row=0; row<rows; row++) {
           pbm_readpbmrow(stdin, bitrow, cols, format);
           pbm_writepbmrow(fd_out, bitrow, cols, 0);
         }
         pbm_freerow(bitrow);
         break;
      case RPBM_FORMAT:
        {
          long size;

          int block[4096];
          int blocks;

          size=rows*((cols-1)/8+1);     /* lets pray its bytealigned */

          blocks=size/4096;
          size%=4096;

          while(blocks--) {
            fread(block, 1, 4096, stdin);
            fwrite(block, 1, 4096, fd_out);
          }

          while(size--)
            fputc(fgetc(stdin),fd_out);
          break;
        }
      default:
        pm_error("unrecognized format.");
    }

    pclose(fd_out);
    page++;
  }
  /*}}}  */
  
  exit(0);
}
/*}}}  */
