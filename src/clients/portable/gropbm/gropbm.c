/*{{{}}}*/
/*{{{  Notes*/
/*

The spirit of old mroff is still alive (and kicking:).  This driver and
the hfont.c file are essentially what the old mroff raster driver
contained in its first version.  Later there was support for LRU paging,
which was needed for a 64k I&D system, now the OS offers virtual memory.

Michael "likes roff since many years"

*/
/*}}}  */
/*{{{  #includes*/
#include <ctype.h>
#include <errno.h>
#include <getopt.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "bit.h"
#include "hfont.h"
/*}}}  */
/*{{{  #defines*/
#define DEFAULT_XRES 72
#define DEFAULT_YRES 72
#define DEFAULT_XINCH 8
#define DEFAULT_YINCH 12

#define BASIC_XRES 288
#define BASIC_YRES 288

/* hfont uses fixpoint arithmetic */
#define PRECISION(x) ((x)<<7)
#define SINGLE(x) (((x)+64)>>7)
/*}}}  */

/*{{{  variables*/
struct
{
  char *name;
  hfont_raw *raw;
  hfont_scaled *scaled;
  int size;
} mounted_font[] =
{
  /* Also change mkdevmgr when changing this */
  { "roman.s", (hfont_raw*)0, (hfont_scaled*)0, 0 },
  { "italic.t", (hfont_raw*)0, (hfont_scaled*)0, 0 },
  { "roman.t", (hfont_raw*)0, (hfont_scaled*)0, 0 },
  { "special.s", (hfont_raw*)0, (hfont_scaled*)0, 0 },

  { "roman.s", (hfont_raw*)0, (hfont_scaled*)0, 0 },
  { "roman.d", (hfont_raw*)0, (hfont_scaled*)0, 0 },
  { "roman.c", (hfont_raw*)0, (hfont_scaled*)0, 0 },
  { "roman.t", (hfont_raw*)0, (hfont_scaled*)0, 0 },
  { "italic.d", (hfont_raw*)0, (hfont_scaled*)0, 0 },
  { "italic.t", (hfont_raw*)0, (hfont_scaled*)0, 0 },
  { "script.s", (hfont_raw*)0, (hfont_scaled*)0, 0 },
  { "greek.s", (hfont_raw*)0, (hfont_scaled*)0, 0 },
};
/*}}}  */

/*{{{  isqrt*/
#define N_BITS 32
#define MAX_BIT ((N_BITS+1)/2-1)

static unsigned int isqrt(unsigned int x)
{
  register unsigned int xroot, m2, x2;

  xroot=0; m2=1<<MAX_BIT*2;
  do
  {
    x2=xroot+m2;
    xroot>>=1;
    if (x2<=x)
    {
      x-=x2; xroot+=m2;
    }
  } while (m2>>=2);
  if (xroot<x) return xroot+1;
  return xroot;
}
/*}}}  */
/*{{{  main*/
int main(int argc, char *argv[])
{
  /*{{{  variables*/
  int x,y,f,size,err=0,usage=0,verbose=0,page,newpage,firstpage=1,c;
#    ifdef MGR
  int mgr=0;
#    endif
  int xres=DEFAULT_XRES,yres=DEFAULT_YRES,xinch=DEFAULT_XINCH,yinch=DEFAULT_YINCH,wide,high;
  char ln[256],*s,*cmd=(char*)0;
  FILE *fp;
  /*}}}  */

  /*{{{  parse arguments*/
  /*{{{  parse arguments*/
  while ((c=getopt(argc,argv,
#    ifdef MGR
  "m"
#    endif
  "c:x:y:w:l:v"))!=EOF)
  {
    switch (c)
    {
#        ifdef MGR
      /*{{{  m*/
      case 'm':
      {
        mgr=1;
        break;
      }
      /*}}}  */
#        endif
      /*{{{  x*/
      case 'x':
      {
        xres=atoi(optarg);
        break;
      }
      /*}}}  */
      /*{{{  y*/
      case 'y':
      {
        yres=atoi(optarg);
        break;
      }
      /*}}}  */
      /*{{{  w*/
      case 'w':
      {
        xinch=atoi(optarg);
        break;
      }
      /*}}}  */
      /*{{{  l*/
      case 'l':
      {
        yinch=atoi(optarg);
        break;
      }
      /*}}}  */
      /*{{{  v*/
      case 'v': verbose=1; break;
      /*}}}  */
      /*{{{  c*/
      case 'c':
      {
        cmd=optarg;
        break;
      }
      /*}}}  */
      /*{{{  default*/
      default: usage=1; break;
      /*}}}  */
    }
  }
  if (optind!=argc) usage=1;
  wide=xres*xinch;
  high=yres*yinch;
  /*}}}  */
  /*{{{  exit if err or usage*/
  if (usage)
  {
    fprintf(stderr,"Usage: %s [-x dpi-x][-y dpi-y][-w inch-wide][-l inch-long][-c command][-v][-m]\n",argv[0]);
  }
  if (usage || err) exit(1);
  /*}}}  */
  /*}}}  */
  bitmalloc(wide,high);
  do
  {
    /*{{{  process one page*/
    bitclear();
    newpage=0;
    while (fgets(ln,sizeof(ln),stdin)!=(char*)0 && !newpage)
    {
      s=ln; if (*s) *(s+strlen(s)-1)='\0';
      do
      {
      switch (*s)
      {
        /*{{{  hn*/
        case 'h': s++; x+=PRECISION(atoi(s)*xres)/BASIC_XRES; while (isdigit(*s)) s++; break;
        /*}}}  */
        /*{{{  vn*/
        case 'v': s++; y+=PRECISION(atoi(s)*yres)/BASIC_YRES; while (isdigit(*s)) s++; break;
        /*}}}  */
        /*{{{  fn*/
        case 'f':
        {
          s++;
          f=atoi(s)-1;
          if (mounted_font[f].raw==(hfont_raw*)0) mounted_font[f].raw=hfont_open(mounted_font[f].name);
          while (isdigit(*s)) s++;
          break;
        }
        /*}}}  */
        /*{{{  sn*/
        case 's':
        {
          s++;
          size=atoi(s);
          while (isdigit(*s)) s++;
          break;
        }
        /*}}}  */
        /*{{{  Hn*/
        case 'H': s++; x=PRECISION(atoi(s)*xres)/BASIC_XRES; while (isdigit(*s)) s++; break;
        /*}}}  */
        /*{{{  Vn*/
        case 'V': s++; y=PRECISION(atoi(s)*yres)/BASIC_YRES; while (isdigit(*s)) s++; break;
        /*}}}  */
        /*{{{  tstr*/
        case 't':
        {
          if (mounted_font[f].size!=size)
          {
            if (mounted_font[f].scaled!=(hfont_scaled*)0) free(mounted_font[f].scaled);
            mounted_font[f].scaled=(hfont_scaled*)0;
          }
          if (mounted_font[f].scaled==(hfont_scaled*)0)
          {
            mounted_font[f].scaled=hfont_scale(mounted_font[f].raw,xres,yres,size);
            mounted_font[f].size=size;
          }
          hfont_print(mounted_font[f].scaled,&x,&y,0,s+1);
          *s='\0';
          break;
        }
        /*}}}  */
        /*{{{  un str*/
        case 'u':
        {
          int add_width;
          char r[2]=" ";

          if (mounted_font[f].size!=size)
          {
            if (mounted_font[f].scaled!=(hfont_scaled*)0) free(mounted_font[f].scaled);
            mounted_font[f].scaled=(hfont_scaled*)0;
          }
          if (mounted_font[f].scaled==(hfont_scaled*)0)
          {
            mounted_font[f].scaled=hfont_scale(mounted_font[f].raw,xres,yres,size);
            mounted_font[f].size=size;
          }
          s++;
          add_width=PRECISION(atoi(s)*xres)/BASIC_XRES; if (*s=='-') s++; while(isdigit(*s)) s++; s++;
          while (*s && *s!='\n')
          {
            r[0]=*s;
            hfont_print(mounted_font[f].scaled,&x,&y,0,r);
            x+=add_width;
            s++;
          }
          *s='\0';
          break;
        }
        /*}}}  */
        /*{{{  Cxy*/
        case 'C':
        {
#        define INT(x,y) ((x)<<8|(y))

          int fixed_x=x,fixed_y=y;
          char r[2]="?";

          s++;
          if (mounted_font[f].size!=size)
          {
            if (mounted_font[f].scaled!=(hfont_scaled*)0) free(mounted_font[f].scaled);
            mounted_font[f].scaled=(hfont_scaled*)0;
          }
          if (mounted_font[f].scaled==(hfont_scaled*)0)
          {
            mounted_font[f].scaled=hfont_scale(mounted_font[f].raw,xres,yres,size);
            mounted_font[f].size=size;
          }
          switch (INT(*s,*(s+1)))
          {
            case INT(':','A'): r[0]='\200'; break;
            case INT(':','a'): r[0]='\201'; break;
            case INT(':','O'): r[0]='\202'; break;
            case INT(':','o'): r[0]='\203'; break;
            case INT(':','U'): r[0]='\204'; break;
            case INT(':','u'): r[0]='\205'; break;
            case INT('s','s'): r[0]='\206'; break;
#           include "special.c"
            default: fprintf(stderr,"gropbm: Unknown special character \\(%c%c.\n",*s,*(s+1)); break;
          }
          hfont_print(mounted_font[f].scaled,&fixed_x,&fixed_y,0,r);
          s+=2;
          break;
#        undef INT
        }
        /*}}}  */
        /*{{{  cx*/
        case 'c':
        {
          char str[2];
          int fixed_x=x,fixed_y=y;

          s++;
          str[0]=*s;
          str[1]='\0';
          s++;
          hfont_print(mounted_font[f].scaled,&fixed_x,&fixed_y,0,str);
          break;
        }
        /*}}}  */
        /*{{{  pn*/
        case 'p':
        {
          s++;
          y=0;
          page=atoi(s);
          if (!firstpage) newpage=1;
          else
          {
            firstpage=0;
            if (verbose) { fprintf(stderr,"[%d] ",page); fflush(stderr); }
          }
          while (isdigit(*s)) s++;
          break;
        }
        /*}}}  */
        /*{{{  D?*/
        case 'D':
        {
          s++;
          switch (*s)
          {
            /*{{{  e*/
            case 'e':
            {
              int dx,dy,rx,ry;

              sscanf(s+1,"%d %d",&dx,&dy);
              rx=PRECISION(dx*xres)/(2*BASIC_XRES);
              ry=PRECISION(dy*yres)/(2*BASIC_YRES);
              bitellipse(SINGLE(x+rx),SINGLE(y),SINGLE(rx),SINGLE(ry));
              break;
            }
            /*}}}  */
            /*{{{  c*/
            case 'c':
            {
              int d,rx,ry;

              sscanf(s+1,"%d",&d);
              rx=PRECISION(d*xres)/(2*BASIC_XRES);
              ry=PRECISION(d*yres)/(2*BASIC_YRES);
              bitellipse(SINGLE(x+rx),SINGLE(y),SINGLE(rx),SINGLE(ry));
              break;
            }
            /*}}}  */
            /*{{{  l*/
            case 'l':
            {
              int nx,ny;

              sscanf(s+1,"%d %d",&nx,&ny);
              nx=PRECISION(nx*xres)/BASIC_XRES;
              ny=PRECISION(ny*yres)/BASIC_YRES;
              bitline(SINGLE(x),SINGLE(y),SINGLE(x+nx),SINGLE(y+ny));
              x+=nx;
              y+=ny;
              break;
            }
            /*}}}  */
            /*{{{  p*/
            case 'p':
            {
              int nx,ny,ox=x,oy=y;

              s++; while (isblank(*s)) s++;
              do
              {
                nx=PRECISION(atoi(s)*xres)/BASIC_XRES; while (isdigit(*s) || *s=='-') s++; while (isblank(*s)) s++;
                ny=PRECISION(atoi(s)*yres)/BASIC_YRES; while (isdigit(*s) || *s=='-') s++; while (isblank(*s)) s++;
                bitline(SINGLE(ox),SINGLE(oy),SINGLE(ox+nx),SINGLE(oy+ny));
                ox+=nx;
                oy+=ny;
              } while (*s);
              bitline(SINGLE(ox),SINGLE(oy),SINGLE(x),SINGLE(y));
              x+=nx;
              y+=ny;
              break;
            }
            /*}}}  */
            /*{{{  a*/
            case 'a':
            {
              int dh1,dv1,dh2,dv2,a,b;

              sscanf(s+1,"%d %d %d %d",&dh1,&dv1,&dh2,&dv2);
              /* radius in troff units */
              a=isqrt(PRECISION(dh2)*PRECISION(dh2)+PRECISION(dv2)*PRECISION(dv2));
              /* y half axis in pixels */
              b=(a*yres)/BASIC_YRES;
              /* x half axis in pixels */
              a=(a*xres)/BASIC_XRES;
              dh1=PRECISION(dh1*xres)/BASIC_XRES;
              dv1=PRECISION(dv1*yres)/BASIC_YRES;
              dh2=PRECISION(dh2*xres)/BASIC_XRES;
              dv2=PRECISION(dv2*yres)/BASIC_YRES;
              bitarc(SINGLE(x+dh1),SINGLE(y+dv1),SINGLE(a),SINGLE(b),SINGLE(x),SINGLE(y),SINGLE(x+dh1+dh2),SINGLE(y+dv1+dv2));
              x=x+dh1+dh2;
              y=y+dv1+dv2;
              break;
            }
            /*}}}  */
          }
          *s='\0';
          break;
        }
        /*}}}  */
        /*{{{  w*/
        case 'w': s++; break;
        /*}}}  */
        /*{{{  [0-9]*/
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
        {
          char str[2];
          int fixed_x,fixed_y;

          x+=PRECISION(((*s-'0')*10+(*(s+1))-'0')*xres)/BASIC_XRES; s+=2;

          fixed_x=x; fixed_y=y;
          str[0]=*s;
          str[1]='\0';
          s++;
          hfont_print(mounted_font[f].scaled,&fixed_x,&fixed_y,0,str);
          break;
        }
        /*}}}  */
        /*{{{  default*/
        default: *s='\0';
        /*}}}  */
      }
      } while (*s);
    }
    /*{{{  output page*/
    if (cmd!=(char*)0)
    {
      if ((fp=popen(cmd,"w"))==(FILE*)0)
      {
        fprintf(stderr,"%s: Can't execute %s: %s\n",argv[0],cmd,strerror(errno));
        exit(1);
      }
    }
    else fp=stdout;
#        ifdef MGR
    if (mgr) bitmgrwrite(fp); else
#        endif
    bitpbmwrite(fp);
    if (cmd!=(char*)0) pclose(fp);
    /*}}}  */
    if (newpage && !firstpage && verbose) { fprintf(stderr,"[%d] ",page); fflush(stderr); }
    /*}}}  */
  } while (newpage);
  bitfree();
  exit(0);
}
/*}}}  */
