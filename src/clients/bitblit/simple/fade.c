/*{{{}}}*/
/*{{{  Notes*/
/* fade an image to black

1) locate 2 scripts to fade between, call them S1 and S2
2) run "get_maps -f image < S2" to extract the initial image
   of S2
3) run "fade [options] -b image -z > middle" to generate a fade script
   in "middle" using the initial image of S2, above
4) in the play_data script, put S1,middle,S2

*/
/*}}}  */
/*{{{  #includes*/
#include <mgr/bitblit.h>
#include <mgr/share.h>
#include <sys/time.h>
#include <getopt.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
/*}}}  */
/*{{{  #defines*/
#define BASE	6

/* sleep 100ths of seconds */

#define fsleep(s) \
{ \
  struct timeval time; \
  time.tv_sec = (s)/100; \
  time.tv_usec = ((s)%100)*10000; \
  select(0,0,0,0,&time); \
}

#define dprintf		if(debug)	fprintf
/*}}}  */

/*{{{  variables*/
struct rect
{
  short rand;			/* random value used to sort by */
  short wait;			/* 100th seconds to wait */
  short x,y;			/* initial coordinates */
};

struct rect *rects;
int mode=0;			/* drawing mode - used by cmp */
int midx,midy;
BITMAP *screen;
/*}}}  */

/*{{{  die -- die from a signal*/
static void die(int n)
{
  bit_destroy(screen);
  exit(1);
}
/*}}}  */
/*{{{  cmp -- compare rectangles*/
static int cmp(register struct rect *x, register struct rect *y)
{
  int dx,dy,dr;		/* differences in fields */
  int result = 0;
  int scale;
  static int smap[3] = {3,1,5};

  dx = x->x - y->x;
  dy = x->y - y->y;
  dr = x->rand - y-> rand;

  if (mode/BASE == 1) dx = -dx;
  if (mode/BASE == 2) dy = -dy;
  if (mode/BASE == 3) dy = -dy; dx = -dx;

  switch(mode%BASE) 
  {
    /*{{{  0 -- random*/
    case 0: result = dr; break;
    /*}}}  */
    /*{{{  1 -- by x*/
    case 1: result = dx ? dx : dy; break;
    /*}}}  */
    /*{{{  2 -- by y*/
    case 2: result = dy ? dy : dx; break;
    /*}}}  */
    /*{{{  3 -- by x, semi random*/
    case 3: result = dx ? dx : dr; break;
    /*}}}  */
    /*{{{  4 -- by y, semi random*/
    case 4: result = dy ? dy : dr; break;
    /*}}}  */
    /*{{{  5 -- by x,y, center out*/
    case 5:
    scale =smap[(mode/BASE)%3];
    result = (scale*(x->x-midx)*(x->x-midx) + 3*(x->y-midy)*(x->y-midy)) -
    (scale*(y->x-midx)*(y->x-midx) + 3*(y->y-midy)*(y->y-midy));
    if (mode/BASE==3) result = -result;
    break;
    /*}}}  */
  }
  return (result);
}
/*}}}  */

/*{{{  main*/
int main(int argc, char *argv[])
{
  /*{{{  variables*/
  register int i,j;
  char *sname = SCREEN_DEV;
  int debug = (int) getenv("DEBUG");
  BITMAP *map = NULL;
  FILE *in = NULL;				/* for bitmap to fade to */
  register struct rect *r = rects;
  int op = BIT_SRC;				/* bitblit function */
  int begin=0;					/* wait at begginning */
  int end = 0;					/* addional wait at end */
  int nx=10;						/* # rectangles in x direction */
  int ny=10;						/* # rectangles in y direction */
  int delay=3;					/* 100th seconds delay between rects */
  int skip=0;						/* # of rects to skip between delays */
  int count;						/* # of rects */
  int wide,high;					/* size of each rect */
  int rnd=0;						/* randomize time */
  int blend=0;					/* expreimental */
  int final = -1;				/* final entire screen blit */
  int share=0;
  int c;
  /*}}}  */

  /*{{{  parse arguments*/
  while ((c=getopt(argc,argv,"S:b:m:f:F:r:d:x:y:t:z"))!=EOF) switch (c)
  {
    /*{{{  S screenbuffer  -- use screenbuffer*/
    case 'S': sname=optarg; break;
    /*}}}  */
    /*{{{  b image         -- blend to image*/
    case 'b':
    blend=1;
    in = fopen(optarg,"rb");
    dprintf(stderr,"blending %s\n",in?"OK":"BAD");
    break;
    /*}}}  */
    /*{{{  m mode          -- set drawing mode*/
    case 'm':
    mode = atoi(optarg);
    dprintf(stderr,"Drawing mode %d\n",mode);
    break;
    /*}}}  */
    /*{{{  f op            -- set bitblit function*/
    case 'f':		/* use this blit function */
    op = 0xf&atoi(optarg);
    dprintf(stderr,"Drawing function %d\n",op);
    break;
    /*}}}  */
    /*{{{  F op            -- use this as final blit function*/
    case 'F':
    final = 0xf&atoi(optarg);
    break;
    /*}}}  */
    /*{{{  r delay         -- randomize delay added between rects in 100ths*/
    case 'r':
    rnd = atoi(optarg);
    break;
    /*}}}  */
    /*{{{  d delay         -- delay between rects in 100ts*/
    case 'd':
    delay = atoi(optarg);
    skip = index(optarg,'/') ? atoi(1+index(optarg,'/')) : 0;
    dprintf(stderr,"delay is %d 100ths every %d\n",delay,skip);
    break;
    /*}}}  */
    /*{{{  x number        -- number of rectangles in x direction*/
    case 'x':
    nx = atoi(optarg);
    if (nx<1) nx=1;
    break;
    /*}}}  */
    /*{{{  y number        -- number of rectangles in y direction*/
    case 'y':
    ny = atoi(optarg);
    if (ny<1) nx=1;
    break;
    /*}}}  */
    /*{{{  t delay         -- additional wait at end*/
    case 't':
    begin = atoi(optarg);
    end = index(optarg,'/') ? atoi(1+index(optarg,'/')) : 0;
    dprintf(stderr,"wait is %d secs, then  %d secs\n",begin,end);
    break;
    /*}}}  */
    /*{{{  z               -- log it*/
    case 'z':		/* make a log */
    share++;
    dprintf(stderr,"setting capture mode\n");
    break;
    /*}}}  */
    /*{{{  default*/
    default:
    fprintf(stderr,"Usage: fade [-S framebuffer][-b image][-m mode][-f op][-F op][-r delay]\n            [-d delay][-x number][-y number][-t delay][-z]\n");
    exit(1);
    break;
    /*}}}  */
  }
  if (!(screen=bit_open(sname)))
  {
    fprintf(stderr, "fade: couldn't open %s\n",sname);
    exit(1);
  }
  if (blend && !in)
  {
    fprintf(stderr, "fade: couldn't open image file\n");
    exit(2);
  }
  if (blend && (map = bitmapread(in))==NULL)
  {
    fprintf(stderr, "fade: invalid bitmap format\n");
    exit(3);
  }
  /*}}}  */
  /*{{{  generate random rectangles*/
  midx=screen->wide/2;
  midy=screen->high/2;
  count = nx*ny;
  wide = (screen->wide+nx-1)/nx;
  high = (screen->high+ny-1)/ny;
  midx -= wide/2;
  midy -= high/2;

  dprintf(stderr,"%d rects, %d x %d each %d x %d\n", count,nx,ny,wide,high);

  if (count > 100000)
  {
    fprintf(stderr,"fade: too many rectangles (over 100000)\n");
    exit(2);
  }

  rects = malloc(sizeof(struct rect) * count);

  if (!rects)
  {
    fprintf(stderr,"fade: cant malloc %d rects\n",count);
    exit(1);
  }

  srandom(getpid());
  for(r=rects,i=0;i<screen->wide;i+=wide) for(j=0;j<screen->high;j+=high)
  {
    r->rand = random();
    r->wait = delay + (rnd>0 ? random()%rnd : 0);
    r->x = i;
    r->y = j;
    r++;
  }
  /*}}}  */
  /*{{{  sort into random order*/
  qsort(rects,count,sizeof(struct rect),cmp);
  /*}}}  */
  signal(SIGINT,die);
  signal(SIGTERM,die);
  bit_grafscreen();
  timestamp();
  /*{{{  start logging if specified*/
  #ifdef MOVIE
  if (share)
  {
    log_noinitial = 1;
    log_start(stdout);
    log_time();
  }
  #endif
  /*}}}  */
  /*{{{  initial sleep*/
  if (begin>0)
  {
    dprintf(stderr,"sleeping %d\n",begin);
    sleep(begin);
  #ifdef MOVIE
    log_time();
  #endif
  }
  /*}}}  */
  /*{{{  fade*/
  for(r=rects,i=0;i<count;i++)
  {
    if (blend) bit_blit(screen,r->x,r->y,wide,high,op,map,r->x,r->y);
    else bit_blit(screen,r->x,r->y,wide,high,op,NULL,0,0);
    if (r->wait>0 && (skip==0 || i%skip==0)) 
    {
  #ifdef MOVIE
      log_time();
  #endif
      fsleep(r->wait);
    }
    r++;
  }
  /*}}}  */
  if (final >=0) bit_blit(screen,0,0,screen->wide,screen->high,final,blend?map:0,0,0);
#ifdef MOVIE
  log_time();
#endif
  /*{{{  final sleep*/
  if (end>0)
  {
    dprintf(stderr,"sleeping %d\n",end);
    sleep(end);
  #ifdef MOVIE
    log_time();
  #endif
  }
  /*}}}  */
  /*{{{  quit logging*/
  #ifdef MOVIE
  log_end();
  #endif
  /*}}}  */
  bit_destroy(screen);
  return 0;
}
/*}}}  */
