/*{{{}}}*/
/*{{{  Copyright*/
/* pbmtoprt.c - read a portable bitmap and produce 9/24 Printer graphics
**
** Copyright (C) 1992 by Guido Muesch (odiug@pool.informatik.rwth-aachen.de)
**
** The code is inspired by pbmtoepson written by
** John Tiller (tiller@galois.msfc.nasa.gov)
** and Jef Poskanzer.
** I rewrote the code to generate 240x216 output on 9-needle epson and
** 24-needle printers (NEC and Epson). This program also does not read the
** PBM file in one piece. It reads only so much that it can send one line
** to the printer. (Otherwise 360x360 dpi would need 11MB of memory)
**
** Permission to use, copy, modify, and distribute this software and its
** documentation for any purpose and without fee is hereby granted, provided
** that the above copyright notice appear in all copies and that both that
** copyright notice and this permission notice appear in supporting
** documentation.  This software is provided "as is" without express or
** implied warranty.
*/
/*}}}  */

/*{{{  includes*/
#include "pbm.h"		/* includes everything we need */
#include "getopt.h"
/*}}}  */
/*{{{  globals*/
#define PRINTERS 10

static struct Pdefs {
  char *name;           /* name of printerdriver */
  char *advstr;         /* string to do an advance */
  int mode;             /* printing mode */
  int prtno;            /* index in printer driver list */
} printer[] = {
/*  name:         advstr:           mode: prtno: */
  { "eps72x72",   "\0333%c\012\015",   5, 0 },
  { "eps120x144", "\0333%c\012\015",   2, 1 },
  { "eps240x144", "\0333%c\012\015",   3, 1 },
  { "eps240x216", "\0333%c\012\015",   3, 2 },
  { "eps180x180", "\033J%c\015",     047, 3 },
  { "eps360x180", "\033J%c\015",     050, 3 },
  { "eps360x360", "\033]%c\015",     050, 4 },
  { "nec180x180", "\0333%c\012\015", 047, 3 },
  { "nec360x180", "\0333%c\012\015", 050, 3 },
  { "nec360x360", "\0343%c\012\015", 050, 4 }
};

static int mask[] = {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01};

static int verbose=0;
static int rows, cols, format;
static void (*readpbmrow)();
FILE* fp;
/*}}}  */

/*{{{  static void usage()*/
static void usage()
{
  pm_usage("[-hv] [-p <driver>] [file]");
  exit(1);
}
/*}}}  */
/*{{{  static void my_readpbmrow(FILE* file, bit* bitrow, int cols, int format)*/
static void my_readpbmrow(FILE* file, bit* bitrow, int cols, int format)
{
  char *buf, *c;
  int i,j, bcols, rcols;

  bcols=cols/8;       /* number of bytes for (cols) bit */
  rcols=cols%8;       /* remaining bit cols */

  c=buf=malloc(bcols+1);

  fread(buf, 1, (cols-1)/8+1, file);
  j=-1;
  while(bcols--) {
    bitrow[++j] = (*c & 0x80);
    bitrow[++j] = (*c & 0x40);
    bitrow[++j] = (*c & 0x20);
    bitrow[++j] = (*c & 0x10);
    bitrow[++j] = (*c & 0x08);
    bitrow[++j] = (*c & 0x04);
    bitrow[++j] = (*c & 0x02);
    bitrow[++j] = (*c & 0x01);
    c++;
  }
  i=0;
  while(i<rcols)
    bitrow[++j] = (*c & mask[i++]);

  free(buf);
}
/*}}}  */

/*{{{  9  needle printers*/
/*{{{  static void print8rows(int pno, bit** pr, int *space)*/
static void print8rows(int pno, bit** pr, int *space)
{
  int val, col, lastcol;

  /*{{{  calculate lastcol*/
  /*for (lastcol=cols-1; lastcol>=0; --lastcol)*/
  lastcol=cols-1;
  while(lastcol--)
  {
    if (pr[0][lastcol]) break;
    if (pr[1][lastcol]) break;
    if (pr[2][lastcol]) break;
    if (pr[3][lastcol]) break;
    if (pr[4][lastcol]) break;
    if (pr[5][lastcol]) break;
    if (pr[6][lastcol]) break;
    if (pr[7][lastcol]) break;
  }
  lastcol++;
  /*}}}  */

  /*{{{  print lastcol columns*/
  if (lastcol)
  {
    while(*space >255) {
      printf(printer[pno].advstr, 255);
      *space -= 255;
    }
    printf(printer[pno].advstr, *space); *space = 0;   /* vertical move !! */

    putchar('\033');
    putchar('*');
    putchar(printer[pno].mode);
    putchar(lastcol%256);
    putchar(lastcol/256);
    for (col=0; col<lastcol; col++)
    {
      /*{{{  output one byte*/
      val=0;
      if(pr[0][col]) val|=128;
      if(pr[1][col]) val|=64;
      if(pr[2][col]) val|=32;
      if(pr[3][col]) val|=16;
      if(pr[4][col]) val|=8;
      if(pr[5][col]) val|=4;
      if(pr[6][col]) val|=2;
      if(pr[7][col]) val|=1;
      putchar(val);
      /*}}}  */
    }
  }
  /*}}}  */
}
/*}}}  */

/*{{{  static void epsX72(int pno)*/
static void epsX72(int pno)
{
  bit** bits;
  bit* pr[8];             /* Print Row */
  int row, v_sp;
  int i;
  
  v_sp = 0;

  bits = pbm_allocarray(cols, 8);
  
  for (row=0; row<rows; row+=8)
  {
    for(i=0; i<8 && row+i<rows; i++)
      readpbmrow(fp, bits[i], cols, format);
    while(i<8)
      memset(bits[i++], PBM_WHITE, cols);

    for(i=0; i<8; i++)  /* Not very useful in this case */
      pr[i]=bits[i];

    print8rows(pno, pr, &v_sp);

    v_sp += 24;
  }
  putchar('\f');
}
/*}}}  */
/*{{{  static void epsX144(int pno)*/
static void epsX144(int pno)
{
  bit** bits;
  bit* pr[8];
  int row, v_sp;
  int i, j;

  v_sp = 0;   /* counter for vertical spacing */

  bits = pbm_allocarray(cols, 16);
                            
  for (row=0; row<rows; row+=16)
  {
    for(i=0; i<16 && row+i<rows; i++)
      readpbmrow(fp, bits[i], cols, format);
    while(i<16)
      memset(bits[i++], PBM_WHITE, cols);

    for(j=0; j<2; j++)
    {
      for(i=0; i<8; i++)
        pr[i]=bits[2*i+j];

      print8rows(pno, pr, &v_sp);
      v_sp += 1;
    }
    v_sp += 22;
  }
  putchar('\f');
}
/*}}}  */
/*{{{  static void epsX216(int pno)*/
static void epsX216(int pno)
{
  bit** bits;
  bit* pr[8];
  int row, v_sp;
  int i, j;

  v_sp = 0;   /* counter for vertical spacing */

  bits = pbm_allocarray(cols, 24);
                            
  for (row=0; row<rows; row+=24)
  {
    for(i=0; i<24 && row+i<rows; i++)
      readpbmrow(fp, bits[i], cols, format);

    while(i<24)
      memset(bits[i++], PBM_WHITE, cols); /* remaining rows set to 'white' */

    for(j=0; j<3; j++)
    {
      for(i=0; i<8; i++)
        pr[i]=bits[3*i+j];

      print8rows(pno, pr, &v_sp);
      v_sp += 1;
    }
    v_sp += 21;
  }
  putchar('\f');
}
/*}}}  */
/*}}}  */
/*{{{  24 needle printers*/
/*{{{  static void print24rows(int pno, bit** pr, int *space)*/
static void print24rows(int pno, bit** pr, int *space)
{
  int val, col, lastcol;
  
  /*{{{  calculate lastcol*/
  lastcol=cols-1;
  while(lastcol--)
  {
    if (pr[0][lastcol]) break;
    if (pr[1][lastcol]) break;
    if (pr[2][lastcol]) break;
    if (pr[3][lastcol]) break;
    if (pr[4][lastcol]) break;
    if (pr[5][lastcol]) break;
    if (pr[6][lastcol]) break;
    if (pr[7][lastcol]) break;
    if (pr[8][lastcol]) break;
    if (pr[9][lastcol]) break;
    if (pr[10][lastcol]) break;
    if (pr[11][lastcol]) break;
    if (pr[12][lastcol]) break;
    if (pr[13][lastcol]) break;
    if (pr[14][lastcol]) break;
    if (pr[15][lastcol]) break;
    if (pr[16][lastcol]) break;
    if (pr[17][lastcol]) break;
    if (pr[18][lastcol]) break;
    if (pr[19][lastcol]) break;
    if (pr[20][lastcol]) break;
    if (pr[21][lastcol]) break;
    if (pr[22][lastcol]) break;
    if (pr[23][lastcol]) break;
  }
  lastcol++;
  /*}}}  */

  /*{{{  print lastcol columns*/
  if (lastcol)
  {
    while(*space >255) {
      printf(printer[pno].advstr, 255);
      *space -= 255;
    }
    printf(printer[pno].advstr, *space);
    *space = 0;

    putchar('\033');
    putchar('*');
    putchar(printer[pno].mode);
    putchar(lastcol%256);
    putchar(lastcol/256);
    for (col=0; col<lastcol; col++)
    {
      /*{{{  output byte 1*/
      val=0;
      if(pr[0][col]) val|=128;
      if(pr[1][col]) val|=64;
      if(pr[2][col]) val|=32;
      if(pr[3][col]) val|=16;
      if(pr[4][col]) val|=8;
      if(pr[5][col]) val|=4;
      if(pr[6][col]) val|=2;
      if(pr[7][col]) val|=1;
      putchar(val);
      /*}}}  */
      /*{{{  output byte 2*/
      val=0;
      if(pr[8][col]) val|=128;
      if(pr[9][col]) val|=64;
      if(pr[10][col]) val|=32;
      if(pr[11][col]) val|=16;
      if(pr[12][col]) val|=8;
      if(pr[13][col]) val|=4;
      if(pr[14][col]) val|=2;
      if(pr[15][col]) val|=1;
      putchar(val);
      /*}}}  */
      /*{{{  output byte 3*/
      val=0;
      if(pr[16][col]) val|=128;
      if(pr[17][col]) val|=64;
      if(pr[18][col]) val|=32;
      if(pr[19][col]) val|=16;
      if(pr[20][col]) val|=8;
      if(pr[21][col]) val|=4;
      if(pr[22][col]) val|=2;
      if(pr[23][col]) val|=1;
      putchar(val);
      /*}}}  */
    }
  }
  /*}}}  */
}
/*}}}  */

/*{{{  static void prtX180(int pno)*/
static void prtX180(int pno)
{
  bit** bits;
  bit* pr[24];             /* Print Row */
  int row, v_sp;
  int i;
  
  v_sp = 0;

  printf("\033U\001");             /* print unidirectional */
  bits = pbm_allocarray(cols, 24);

  for (row=0; row<rows; row+=24)
  {
    for(i=0; i<24 && row+i<rows; i++)
      readpbmrow(fp, bits[i], cols, format);

    while(i<24)
      memset(bits[i++], PBM_WHITE, cols);

    for(i=0; i<24; i++)      /* senseless ? yes ! */
      pr[i]=bits[i];

    print24rows(pno, pr, &v_sp);

    v_sp += 24;           /* vertical space : 24/180 */
  }
  putchar('\f');
}
/*}}}  */
/*{{{  static void prtX360(int pno)*/
static void prtX360(int pno)
{
  bit** bits;
  bit* pr[24];
  int row, v_sp;
  int i;

  v_sp = 0;

  printf("\033U\001");             /* print unidirectional */
  bits = pbm_allocarray(cols, 48);

  for (row=0; row<rows; row+=48)
  {
    for(i=0; i<48 && row+i<rows; i++)
      readpbmrow(fp, bits[i], cols, format);

    while(i<48)
      memset(bits[i++], PBM_WHITE, cols);

    for(i=0; i<24; i++)
      pr[i]=bits[2*i];
    
    print24rows(pno, pr, &v_sp);   /* even rows */
    v_sp += 1;      /* vertical space : 1/360 */
    
    for(i=0; i<24; i++)
      pr[i]=bits[2*i+1];
    
    print24rows(pno, pr, &v_sp);   /* odd rows */

    v_sp += 47;       /* vertical space : 47/360 */
  }
  putchar('\f');
}
/*}}}  */
/*}}}  */

/*{{{  printer driver list*/
static void (*prtdrv[])() = {
  epsX72,
  epsX144,
  epsX216,
  prtX180,
  prtX360
};
/*}}}  */

/*{{{  void main(int argc, char **argv)*/
void main(int argc, char **argv)
{
  int i;
  int c;
  char *ptype=NULL;
  extern char *optarg;
  extern int optind;

  pbm_init(&argc, argv);
  
  /*{{{  argument parsing*/
  while((c=getopt(argc, argv,"vhp:"))!=EOF)
    switch(c)
    {
      case 'v' : verbose=1; break;
      case 'h' : printf("Available drivers:\n");
                 for(i=0; i<PRINTERS; i++)
                   printf("  %s\n", printer[i].name);
                 putchar('\n');
                 usage(); break;
      case 'p' : ptype=optarg; break;
    }
  /*}}}  */
  if(verbose) fprintf(stderr, "verbose mode:\n");

  /*{{{  open file or stdin*/
  switch(argc-optind)
  {
    case 1:
            if(verbose) fprintf(stderr, "opening %s ...\n", argv[optind]);
            fp=pm_openr(argv[optind]);
            break;
    case 0:
            if(verbose) fprintf(stderr, "opening stdin ...\n");
            fp=stdin;
            break;
    default: usage();
  }
  /*}}}  */

  /*{{{  check printer, read header and go*/
  i=0;

  /*{{{  check printerdevice*/
  if(ptype)
     while(strcmp(ptype, printer[i].name) && i<PRINTERS) i++;

  if(verbose && i<PRINTERS) fprintf(stderr, "selecting printertype %s\n", printer[i].name);

  if (i>=PRINTERS) {
    fprintf(stderr, "%s: '%s': No such printerdriver\n", argv[0], ptype);
    usage();
  }
  /*}}}  */


  while ((c=fgetc(fp))!=EOF) {         /* read multiple pages */
    ungetc(c, fp);                     /* Anybody has a better idea ? */
    /*{{{  read header*/
    pbm_readpbminit(fp, &cols, &rows, &format);
    if(format==RPBM_FORMAT)
      readpbmrow=my_readpbmrow;
    else
      readpbmrow=pbm_readpbmrow;
    /*}}}  */
    prtdrv[printer[i].prtno](i);       /* go */
  }
  /*}}}  */

  printf("\033@");                     /* reset printer */
  pm_close(fp);
  
  exit(0);
}
/*}}}  */
