/*{{{}}}*/
/*{{{  Notes*/
/* star-trek lock screen (sau/sdh) */
/*}}}  */
/*{{{  #includes*/
#include <mgr/bitblit.h>
#include <mgr/mgr.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pwd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#ifdef sun
#define _POSIX_VDISABLE 0
#endif
/*}}}  */
/*{{{  #defines*/
#define SSIZE	3		/* star size */

#define MAXZ 500 /* maximum z depth */
#define MAXZ 500 /* maximum z depth */
#define NSTARS 256 /* maximum number of stars */
#define SPEED	4		/* star speed */
#define SCALE	(short)6	/* for rotator */
#define COUNT	(short)2	/* for rotator */
#define ON 1  /* plotting states */
#define OFF 0
#define Random() ((unsigned int)rand())
/*}}}  */

/*{{{  variables*/
static short maxv, maxh; /* display size */
static short hmaxv, hmaxh;	/* 1/2 display size */

static struct st 
{
  short x, y, z;
  short color;
} stars[NSTARS]; /* our galaxy */
static int dir = 1;
/*}}}  */

/*{{{  flop*/
static void flop(int n)
{
  dir = 1-dir;
  signal(SIGALRM,flop);
}
/*}}}  */
/*{{{  init_all*/
void init_all(where) register BITMAP *where;
{
  maxv = BIT_HIGH(where);
  hmaxv = maxv>>1;
  maxh = BIT_WIDE(where);
  hmaxh = maxh>>1;
}       
/*}}}  */
/*{{{  cordic*/
/* CORDIC rotator. Takes as args a point (x,y) and spins it */
/* count steps counter-clockwise.                   1       */
/*                                Rotates atan( --------- ) */
/*                                                  scale   */
/*                                                 2        */
/* Therefore a scale of 5 is 1.79 degrees/step and          */
/* a scale of 4 is 3.57 degrees/step                        */

static void cordic(x, y, scale, count)
short *x, *y;
register short scale, count;

{
  register short tempx, tempy;

  tempx = *x;
  tempy = *y;

  if (dir) /* (counter-clockwise) */
  for (; count; count--)
  {
    tempx -= (tempy >> scale);
    tempy += (tempx >> scale); 
  }
  else          /* (clockwise) */
  for (; count; count--)
  {
    tempx += (tempy >> scale);
    tempy -= (tempx >> scale);
  }

  *x = tempx;
  *y = tempy;
}
/*}}}  */
/*{{{  xplot*/
static int xplot(where,x, y, col, state)
register BITMAP *where;
register int x, y;
register int col;
int state;
{
  /* are we on the screen? If not, let the caller know*/
  if (x < 0 || x >= maxh || y < 0 || y >= maxv ) return(1);

  bit_blit(where,x,y,SSIZE,SSIZE, state ?
  BUILDOP(BIT_SRC,col,BLACK) :
  BUILDOP(BIT_NOT(BIT_SRC),col,BLACK) ,
  (BITMAP*)0,0,0);
  return(0);
}
/*}}}  */
/*{{{  project*/
static int project(where,x, y, z, col, state)
register BITMAP *where;
register short x, y, z;
register int col;
register short state;
{
        
  /* one-point perspective projection */
        /* the offsets (maxh/2) and maxv/2) ensure that the
         * projection is screen centered
  */
  x = (x/z) + hmaxh;
  y = (y/z) + hmaxv;
  return(xplot(where,x, y, col, state));

}
/*}}}  */
/*{{{  fly*/
static void fly (where) BITMAP *where;
{
  register short i;
  register struct st *stp;

  init_all(where);     /* set up global variables */
  for (i=0,stp=stars; i<NSTARS; i++,stp++)
  {
    /* initialize galaxy */
    do 
    {
      stp->x = Random();
      stp->y = Random();
      stp->z = (Random() % MAXZ) + 1;
      stp->color = Random() & ((1 << BIT_DEPTH(where))-1);
    } while(project(where,stp->x, stp->y, stp->z, stp->color, ON)); /* on screen? */
  }
}
/*}}}  */
/*{{{  dofly*/
static void dofly (where) BITMAP *where;
{
  register short i;
  register struct st *stp;

  i = NSTARS;
  stp = stars;
  do 
  {
    /* turn star off*/
    project(where,stp->x, stp->y, stp->z, stp->color, OFF);
    if ((stp->z -= SPEED) <= 0)
    {
      /* star went past us */
      stp->x = Random();
      stp->y = Random();
      stp->z = MAXZ;
    }
    else
    {
      /* rotate universe */
      cordic(&stp->x,&stp->y,SCALE,COUNT);
    }
    if (project(where,stp->x, stp->y, stp->z, stp->color, ON))
    {
      /* if projection is off screen, get a new position */
      stp->x = Random();
      stp->y = Random();
      stp->z = MAXZ;
    }
    ++stp;
  } while(--i);
}
/*}}}  */

/*{{{  main*/
int main(int argc, char *argv[])
{
  /*{{{  variables*/
  struct passwd *pwd;
  struct termios sg,save;
  char buff[100];
  pid_t pid;
  BITMAP *display;
  BITMAP *stash;
  /*}}}  */

  /*{{{  check for not be run under mgr*/
  if ( is_mgr_term())
  {
    fprintf(stderr,"mgrlock: I can't run under mgr.\n");
    exit(1);
  }
  /*}}}  */
  /*{{{  init display*/
  if ((display=bit_open(SCREEN_DEV))==(BITMAP*)0)
  {
    fprintf(stderr,"mgrlock: can't open screen\n");
    exit(1);
  }
  if ((stash=bit_alloc(display->wide,display->high,(DATA*)0,display->depth))==(BITMAP*)0)
  {
    fprintf(stderr,"mgrlock: can't alloc bitmap\n");
    exit(1);
  }
  /*}}}  */
  /*{{{  get passwd entry*/
  if ((pwd=getpwuid(getuid()))==(struct passwd*)0)
  {
    fprintf(stderr,"mgrlock: I don't know you, go away.\n");
    exit(1);
  }
  /*}}}  */
  /*{{{  ignore signals*/
  signal(SIGINT,SIG_IGN);
  signal(SIGHUP,SIG_IGN);
  signal(SIGTERM,SIG_IGN);
  signal(SIGQUIT,SIG_IGN);
  signal(SIGTSTP,SIG_IGN);
  /*}}}  */
  /*{{{  set up tty*/
  tcgetattr(0,&sg);
  save=sg;
  sg.c_iflag=ICRNL;
  sg.c_lflag=0;
  memset(sg.c_cc,_POSIX_VDISABLE,NCCS);
  sg.c_cc[VTIME]=0;
  sg.c_cc[VMIN]=1;
  tcsetattr(0,TCSANOW,&sg);
  /*}}}  */

  bit_grafscreen();
  bit_blit(stash,0,0,display->wide,display->high,BIT_SRC,display,0,0);
  bit_blit(display,0,0,display->wide,display->high,BIT_CLR,(BITMAP*)0,0,0);
  init_all(display);
  fly(display);
  signal(SIGALRM,flop);
  switch (pid=fork())
  {
    /*{{{  -1*/
    case -1:
    {
      bit_textscreen();
      fprintf(stderr,"mgrlock: can't fork\n");
      exit(1);
    }
    /*}}}  */
    /*{{{  0*/
    case 0: nice(5); while (1) dofly(display);
    /*}}}  */
    /*{{{  default*/
    default: while (1)
    {
      fgets(buff,sizeof(buff),stdin);
      kill(pid,SIGALRM);	/* change directions */
      if (strcmp(pwd->pw_passwd,crypt(buff,pwd->pw_passwd)) == 0)
      {
        kill(pid,SIGKILL);
        while(wait(0)!=pid);
        bit_blit(display,0,0,display->wide,display->high,BIT_SRC,stash,0,0);
        bit_textscreen();
        tcsetattr(0,TCSANOW,&save),
        exit(0);
      }
    }
    /*}}}  */
  }
  return 0;
}
/*}}}  */
