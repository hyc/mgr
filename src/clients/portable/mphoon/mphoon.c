/*
** Copyright (C) 1988 by Jef Poskanzer and Craig Leres.
**
** Permission to use, copy, modify, and distribute this software and its
** documentation for any purpose and without fee is hereby granted, provided
** that the above copyright notice appear in all copies and that both that
** copyright notice and this permission notice appear in supporting
** documentation.  This software is provided "as is" without express or
** implied warranty.
*/

/*
** Adapted to the MGR window manager by Carsten Emde (carsten@ce.pr.net.ch)
*/

#include <signal.h>
#ifdef VIAFILE
#ifdef OSK
#include <modes.h>
#else
#include <sys/types.h>
#include <sys/stat.h>
#endif
#endif

#ifndef OSK
#include <unistd.h>
#include <fcntl.h>
#endif

#ifdef OLDMGR
#include <term.h>
#include <dump.h>
#define B_SIZESERV(w,h,d) B_SIZE16(w,h,d)
static const unsigned long int Endian = 0x12345678;
#define SWAP_FOR_LITTLE_ENDIAN (0x78==*(unsigned char *)&Endian)
#else
#define OLDMGRBITOPS
#include <mgr/mgr.h>
#define B_SIZESERV(w,h,d) B_SIZE8(w,h,d)
#define SWAP_FOR_LITTLE_ENDIAN 0
#endif

#include <math.h>
#include "tws.h"

#ifdef COMPATMGR
#define get_param m_get_param
#define ckmgrterm m_ckmgrterm
#endif

#define SCRATCH 1
#ifdef OSK
#define TEMPMOON "/dd/tempmoon.m1"
#else
#define TEMPMOON "/tmp/tempmoon.m1"
#endif
#define MAGIC "noohpm\n"

/* Global functions */
extern char  *malloc();
extern double jtime(), phase();
#ifdef OSK
extern double atan2();
#endif

/* Local functions */
static void   putinit();
static void setroot(char *tempmoon, int path, int w, int h, char *bits);
static void hackbits(struct tws *t, int w, int h, char *bits,
		     int cx, int cy, int r);
static void   synchronize(void);
extern void getbitmap(int *w, int *h, char **bits, int *cx, int *cy, int *r);

/* Global variables */
extern int    errno;

/* Local variables */
static char  *argv0;
static int    intrpt = 0;
static int    blackflag = 0;
static int    demoflag = 0;
static int    reversecolor = 0;

static char  leftmask[8] = {0xff, 0xfe, 0xfc, 0xf8, 0xf0, 0xe0, 0xc0, 0x80};
static char rightmask[8] = {0x00, 0x01, 0x03, 0x07, 0x0f, 0x1f, 0x3f, 0x7f};

static char  shade_0_bits[] = {0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff};
static char  shade_1_bits[] = {0xfe,0xff,0xff,0xff,0xff,0xff,0xff,0xff};
static char  shade_2_bits[] = {0xfe,0xff,0xfb,0xff,0xff,0xff,0xff,0xff};
static char  shade_3_bits[] = {0xfe,0xff,0xfb,0xff,0x7f,0xff,0xff,0xff};
static char  shade_4_bits[] = {0xfe,0xff,0xfb,0xff,0x7f,0xff,0xff,0xef};
static char  shade_5_bits[] = {0xfe,0xbf,0xfb,0xff,0x7f,0xff,0xff,0xef};
static char  shade_6_bits[] = {0xfe,0xbf,0xfb,0xdf,0x7f,0xff,0xff,0xef};
static char  shade_7_bits[] = {0xfe,0xbf,0xfb,0xdf,0x7f,0xff,0xfe,0xef};
static char  shade_8_bits[] = {0xfe,0xbf,0xfb,0xdf,0x7f,0xdf,0xfe,0xef};
static char  shade_9_bits[] = {0xfe,0xbf,0xfb,0xdf,0x7d,0xdf,0xfe,0xef};
static char shade_10_bits[] = {0xfe,0xbf,0xfb,0xdf,0x7d,0xdf,0xfa,0xef};
static char shade_11_bits[] = {0xfe,0xbf,0xfb,0xdf,0x7d,0xdf,0xfa,0xaf};
static char shade_12_bits[] = {0xfe,0xbf,0xfa,0xdf,0x7d,0xdf,0xfa,0xaf};
static char shade_13_bits[] = {0xfe,0xaf,0xfa,0xdf,0x7d,0xdf,0xfa,0xaf};
static char shade_14_bits[] = {0xfe,0xaf,0xfa,0xdf,0x75,0xdf,0xfa,0xaf};
static char shade_15_bits[] = {0xfa,0xaf,0xfa,0xdf,0x75,0xdf,0xfa,0xaf};

char *shades[16] = {
    shade_0_bits,  shade_1_bits,  shade_2_bits,  shade_3_bits,
    shade_4_bits,  shade_5_bits,  shade_6_bits,  shade_7_bits,
    shade_8_bits,  shade_9_bits,  shade_10_bits, shade_11_bits,
    shade_12_bits, shade_13_bits, shade_14_bits, shade_15_bits };

#ifndef PI
#define PI 3.14159265358979323846  /* Assume not near black hole or in
                                      Tennessee */
#endif


/*
 * m a i n
 */
int
main(argc, argv)
  int argc;
  char **argv;
{
  char *bits, *xbits;
  char *tempmoon = TEMPMOON;
  int   w, h, cx, cy, r;
  int   wide, high, border;
  int   deactivate = 0;
  int   foundflag;
  int   size;
  int   delayminutes = 0;
  int   path;
  void  clean(), sgnlhndl();

  argv0 = argv[0];
  getbitmap(&w, &h, &bits, &cx, &cy, &r);

#define NEXTARG argv++;argc--

  foundflag = 1;
  while(argc > 1 && foundflag) {
    if (strcmp(argv[1], "-a") == 0) {
      deactivate = 1;
    } else if (strcmp(argv[1], "-b") == 0) {
      blackflag = 1;
    } else if (strcmp(argv[1], "-d") == 0) {
      demoflag = 1;
    } else if (strcmp(argv[1], "-r") == 0) {
      reversecolor = 1;
    } else if (strcmp(argv[1], "-t") == 0) {
      NEXTARG;
      if (sscanf(argv[1], "%d", &delayminutes) != 1)
	goto usage;
    } else if (strcmp(argv[1], "-x") == 0) {
      NEXTARG;
      if (sscanf(argv[1], "%d", &cx) != 1)
	goto usage;
    } else if (strcmp(argv[1], "-y") == 0) {
      NEXTARG;
      if (sscanf(argv[1], "%d", &cy) != 1)
	goto usage;
    } else
      foundflag = 0;
    if( foundflag)
      NEXTARG;
  }

  if (argc > 1) {
usage:
    fprintf(stderr,
      "usage: %s [-a] [-b] [-d] [-t minutes]\n", argv0);
    exit(1);
  }
  
  ckmgrterm(NULL);
  m_setup(0);

#ifdef ENHANCED
  m_setfast(M_IMBSEP);
#endif

  m_push(P_CURSOR|P_EVENT|P_FLAGS|P_MENU|P_POSITION|P_WINDOW);
  signal(SIGINT, sgnlhndl);
  signal(SIGQUIT, sgnlhndl);

  m_ttyset();
  get_param(NULL, &wide, &high, &border);
  border *= 2;

#ifdef VIAFILE
  if ((path = creat(tempmoon, S_IREAD|S_IWRITE)) == -1) {
    m_ttyreset();
    fprintf( stderr, "can't create temporary file '%s'\n", tempmoon);
    exit( 1);
  }
#endif

  if(h > high - border)
  h = high;
  if(w > wide - border) {
    m_ttyreset();
    fprintf( stderr, "sorry, this image (%dx%d) does not fit on your screen.\n",
             w, h);
    exit( 1);
  }

  m_shapewindow((wide - w - border) >> 1, (high - h - border) >> 1,
                 w + border, h + border);

  if(demoflag || delayminutes) {
    if(deactivate)
      m_clearmode(M_ACTIVATE);
  }
  m_setcursor(CS_INVIS);
  m_setmode(M_ABS);
  m_func(reversecolor? B_COPYINVERTED: B_COPY);
    
  if (delayminutes <= 0 && ! demoflag) {
    hackbits(dtwstime(), w, h, bits, cx, cy, r);
    setroot(tempmoon, path, w, h, bits);
    if(intrpt)
      clean(intrpt);
    m_flush();
    sleep(3);
    clean(0);
    exit(0);
  }

  size = (w >> 3) * h;

  xbits = (char *) malloc(size);
  if(xbits == NULL) {
    fprintf(stderr,"can't get enough memory\n");
    exit(errno);
  }
  for (;;) {
    if(intrpt)
      clean(intrpt);
    memcpy((char *) xbits, (char *) bits, size);

    hackbits(dtwstime(), w, h, xbits, cx, cy, r);
    setroot(tempmoon, path, w, h, xbits);
    if(intrpt)
      clean(intrpt);

    if (demoflag)
      sleep(1);  /* continuous mode */
    else
      sleep(delayminutes * 60);
  }
  /* NOTREACHED */
}


/*
 * s e t r o o t
 */
void setroot(tempmoon, path, w, h, bits)
char *tempmoon;
int   path;
int   w, h;
char *bits;
{
  register char tmp, *bp, *lbp;
  register long i, j;
  int wbytes = w >> 3;

#ifdef VIAFILE
  lseek(path, 0, 0);
  synchronize();
  putinit(path, h, w);       
#else
  path = fileno(m_termout);
#endif

  for(i = 0; i < h; i++) {
#ifndef VIAFILE
    m_bitld(w, 1, 0, i, B_SIZESERV(w,1,1));
#endif
    for(bp = bits + i * wbytes, lbp = bp + wbytes; bp < lbp; bp++) {
      tmp = *bp;
      *bp = 0;
      for(j = 0; j < 8; j++) {
        if (tmp & (1 << j))
         *bp |= 0x80 >> j;
      }
    }
#ifndef VIAFILE
    if(SWAP_FOR_LITTLE_ENDIAN)
      swap_blongs((unsigned char *) bits + i * wbytes, wbytes);
    m_flush();
#endif
    write(path, bits + i * wbytes, wbytes);
    if(intrpt)
      break;
  }
#ifdef VIAFILE
  m_bitfromfile(SCRATCH, tempmoon);
  m_bitcopyto(0, 0, w, h, 0, 0, 0, SCRATCH); 
#endif
}


/*
 * h a c k b i t s
 */
void hackbits(t, w, h, bits, cx, cy, r)
  struct tws *t;
  int w, h;
  char *bits;
  int cx, cy, r;
{
  double jd, angphase, cphase, aom, cdist, cangdia, csund, csuang;
  int i;
  register int x, y;
  int xleft, xright;
  double fxleft, fxright;
  double fy;
  int wxright, bxright, wxleft, bxleft;
  int off, size;
  double cap, ratio;
  int shadeindex;
  char shade;
  static double demoinc = 0.0;

  jd = jtime( t );
  if ( demoflag ) {
    /* Jump ahead a day each time through. */
    jd += demoinc;
    demoinc += 1.0;
  }

  angphase = phase( jd, &cphase, &aom, &cdist, &cangdia, &csund, &csuang);
  cap = cos( angphase );

  /* Hack to figure approximate earthlighting. */
  if ( cphase < 0.1 ) cphase = 0.1;
  if ( cphase > 0.9 ) cphase = 0.9;
  ratio = (1.0 - cphase) / cphase;  /* ratio varies from 9.0 to 0.111 */
  shadeindex = (int) ( ratio / 9.0 * 15.9999 );

#ifdef DEBUG
printf("angphase %f, cap %f\n", angphase, cap);
#endif

  for (i = 0; i < 2 * r; i++) {
    y = cy - r + i;
    fy = i - r;
    fxright = r * sqrt(1.0 - (fy * fy) / (r * r));
    fxleft = - fxright;
    if (angphase >= 0.0 && angphase < PI)
      fxright *= cap;
    else
      fxleft *= cap;

    xright = fxright + cx;
    xleft = fxleft + cx;

    wxright = xright / 8;
    bxright = xright % 8;

    wxleft = xleft / 8;
    bxleft = xleft % 8;

    off = y * ((w + 7) / 8);
                
    size = (w >> 3) * h;
                
    if ( blackflag )
      shade = 0xff;
      
    else
      shade = shades[shadeindex][y % 8];
    if (wxleft == wxright) {
      if (wxleft + off < size) {
      bits[wxleft + off] |=
        leftmask[bxleft] & shade & rightmask[bxright];
      }
    }
    else {
      if (wxleft + off < size)
        bits[wxleft + off] |= leftmask[bxleft] & shade;
      for (x = wxleft + 1; x < wxright; x++) {
        if (x + off < size)
          bits[x + off] |= shade;
      }
      if (wxright + off < size)
        bits[wxright + off] |= rightmask[bxright] & shade;
    }
  }
}


/*
 * p u t i n i t
 */
static void putinit(path, rows, cols)
int path, rows, cols;
{
  struct b_header head;

  head.magic[0] = 'y';
  head.magic[1] = 'z';
  head.h_wide = ((cols >> 6) & 0x3f) + ' ';
  head.l_wide = (cols & 0x3f ) + ' ';
  head.h_high = ((rows >> 6) & 0x3f) + ' ';
  head.l_high = (rows & 0x3f) + ' ';
  head.depth = (1 & 0x3f)  + ' ';
  head._reserved = ' ';
  write(path, &head, sizeof(head));
}


/*
 * c l e a n
*/ 
void
clean(code)
int code;
{
  m_pop();
  m_flush();
  synchronize();
  m_ttyreset();
  unlink(TEMPMOON);
  exit(code);
}


/*
 * s g n l h n d l
 */
void
sgnlhndl(code)
int code;
{
  intrpt = code;
}


static char syncbuf[512];
/*
 * s y n c h r o n i z e
 */
void synchronize()
{
  char sendbuf[16];

  sprintf(sendbuf, "\n%s", MAGIC);
  m_sendme(sendbuf);
  do
    m_gets(syncbuf);
  while (strcmp(syncbuf, MAGIC));
}
