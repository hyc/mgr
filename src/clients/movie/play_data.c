/*{{{}}}*/
/*{{{  Notes*/
/* read op file (binary format) and do the associated actions */
/*}}}  */
/*{{{  #includes*/
#include <mgr/bitblit.h>
#include <mgr/share.h>
#include <sys/signal.h>
#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>

#include "getshort.h"
/*}}}  */
/*{{{  #defines*/
/* default display size (for quiet) */

#define QUIET_WIDE 1152
#define QUIET_HIGH 900
#define MAXUSHORT (1<<16)
#define THRES 200               /* Max expected wait between time stamps */
#define MAX_SLEEP       1000                            /* 10 seconds */

#define Max(x,y)                ((x)>(y)?x:y)
#define dprintf         if (debug) fprintf
#define qprintf         if (debug2) fprintf

#define GET_OPT(i)   \
strlen(argv[i])>2 ? argv[i]+2 : argv[++i]

/* see if string <str> ends in suffix <s> */

#define Suffix(str,s)   \
(strncmp(s,str+strlen(str)-strlen(s),strlen(s))==0)
/*}}}  */

/*{{{  variables*/
int debug;                      /* debugging flag */
int debug2;                     /* more debugging flag (script logging) */
int quiet;                      /* don't use display */

char line[256];                 /* line buffer */
FILE *get_script();             /* open a command script */
FILE *new_script();             /* start a new script */
char *next_script();            /* get next script in list */

unsigned short args[10];        /* list of arguments */
BITMAP *maps[1024];             /* list of bitmaps */
BITMAP *screen = NULL;          /* treat the screen differently */

static unsigned int m_time = 0; /* my initial time (100th seconds) */
static unsigned int y_time = 0; /* your initial time (100th seconds) */
static int my_time, your_time=0;        /* current time info */
static unsigned int old_time=0; /* your previous time (100th seconds) */
static unsigned int scale = 0;  /* for scaling times > 6 minutes */
static unsigned int base_time=0;        /* your first initial time */
static int max_map=0;                           /* maximum bitmap used */
static int ppid = 0;            /* parents pid */
/*}}}  */

/*{{{  parse a line into fields (from dbms)*/
int
parse(line,delim,fields)
char *line;
char **fields;
char delim;
{
  register char c, *start;
  register int count;

  for(count=0,start=line; c= *line; line++)
  if (c == delim)
  {
    fields[count]=start;
    if (count<25) count++;
    *line = '\0';
    start=line+1;
  }
  if (start<line) fields[count++] = start;
  fields[count]=NULL;
  return(count);
}
/*}}}  */
/*{{{  start a series of scripts (e.g. s1,s2,s3,s4)*/
static int next_one = 0;
static char *scripts[25];
static char script_list[256];
static int max_script;

setup_script(s)
char *s;
{
  next_one = 0;
  strcpy(script_list,s);
  max_script = parse(script_list,',',scripts);
  dprintf(stderr,"Setting up script, %d items\n",max_script);
}

char *
next_script()
{
  char *result = NULL;

  result = scripts[next_one];
  dprintf(stderr,"Setting script to [%s]\n",result?result:"END");
  if (result) next_one++;
  return(result);
}

int
reset_script()
{
  next_one=0;
}
/*}}}  */
/*{{{  get a new script*/
FILE *
new_script(name)
char *name;             /* name of script */
{
  FILE *file;
  char *p, *index();
  FILE *get_script();

  scale = 0;                      /* reset script time offset */
  your_time=0;            /* reset script time base */
  y_time=0;                       /* reset my time */
  base_time = 0;          /* reset base time */
  file = get_script(name);
  if ((p=rindex(name,'.')) && p[1]=='Z') 
  {
    *p = '\0';              /* strip off suffix */
    printf("S %s\n",name);
    *p = '.';
  }
  else
  printf("S %s\n",name);
  kill_maps();
  dprintf(stderr,"Sending script [%s]\n",name);
  if (p)                          /* restore suffix */
  *p = '.';
  return(file);
}
/*}}}  */
/*{{{  pause for <pause> 100th seconds, or until input on <file>*/
/* pause for <pause> 100th seconds, or until input on <file>
 *   if pause is 0, block
*/

int
do_pause(file,pause)
FILE *file;
int pause;
{
  int mask = file ? 1<<(fileno(file)) : 0;
  struct timeval time;

  if (pause>0) 
  {
    time.tv_sec = pause/100; 
    time.tv_usec = (pause%100)*10000; 
    return(select(32,&mask,0,0,&time)); 
  }
  else if (mask) return(select(32,&mask,0,0,0));
  else return(0);
}
/*}}}  */
/*{{{  get the script*/
int compress;                           /* script is compressed */

FILE *
get_script(name)
char *name;
{
  FILE *file;
  char buff[100];

  dprintf(stderr,"DO Got script [%s]\n",name);
  if (strcmp(name,"-")==0) 
  {
    compress=0;
    file = stdin;
  }
  else if (Suffix(name,".Z")) 
  {
    sprintf(buff,"zcat %s",name);
    file = popen(buff,"r");
    dprintf(stderr,"DO Using [%s] to read script\n",buff);
    compress=1;
  }
  else 
  {
    file = fopen(name,"r");
    compress=0;
    dprintf(stderr,"DO Using [%s] as script\n",name);
  }
  return(file);
}
/*}}}  */
/*{{{  close a script*/
int close_script(file) FILE *file;
{
  if (file && compress) pclose(file);
  else if (file && file != stdin) fclose(file);
  return(0);
}
/*}}}  */
/*{{{  kill all of the bitmaps*/
int kill_maps(void)
{
  register int c;

  dprintf(stderr,"DO kill all: (%d)", max_map);
  for (c=1;c<max_map;c++) if (maps[c])
  {
    dprintf(stderr,"%d ",c);
    if (maps[c]!=screen)
    bit_destroy(maps[c]);
  }
  dprintf(stderr,"\n");
  bzero(maps,sizeof(maps));
  max_map=0;
}
/*}}}  */
/*{{{  grind -- process until we need to sleep, return sleep in 100'ths seconds*/
int grind(screen,file,speed) BITMAP *screen; FILE *file; int speed;
{
  /*{{{  variables*/
  register int c;                                  /* command type */
  BITMAP *new;                                     /* newly created bitmap */
  register char *data;                             /* pointer to bitmap data */
  /*}}}  */

  if (file == NULL || speed <=0) return(0);

  while ((c=fgetshort(file)) != EOF)
  {
    switch (c&TYPE_MASK) 
    {
      /*{{{  T_NOP*/
      case T_NOP:                                             /* no-op */
      c &= 0xF;               /* # of bytes to skip */
      while(c-- > 0)
      if (debug) putc(getchar(), stderr);
      break;
      /*}}}  */
      /*{{{  T_BLIT -- do a bitblit*/
      case T_BLIT:
      fgetnshort(file,args,8);
      max_map = Max(max_map,args[0]);
      max_map = Max(max_map,args[1]);
      qprintf(stderr,"blit %d->%d %dx%d to %d,%d\n", args[0], args[1],args[4],args[5],args[6],args[7]);
      if (maps[args[0]] && maps[args[1]])
      bit_blit(maps[args[0]],args[2],args[3],args[4],args[5],c&0xF,maps[args[1]],args[6],args[7]);
      else dprintf(stderr,"DO Blit: Invalid bitmap, %d or%d\n", args[0],args[1]);
      break;
      /*}}}  */
      /*{{{  T_WRITE  -- do a bitblit, no src*/
      case T_WRITE:
      fgetnshort(file,args,5);
      max_map = Max(max_map,args[0]);
      qprintf(stderr,"write %d %dx%d\n", args[0], args[3],args[4]);
      if (maps[args[0]]) bit_blit(maps[args[0]],args[1],args[2],args[3],args[4],c&0xF,0,0,0);
      else dprintf(stderr,"Write: Invalid bitmap, %d\n", args[0]);
      break;
      /*}}}  */
      /*{{{  T_LINE   -- draw a line*/
      case T_LINE:
      fgetnshort(file,args,5);
      bit_line(maps[args[0]], args[1], args[2], args[3], args[4], OPCODE(c));
      break;
      /*}}}  */
      /*{{{  T_SCREEN -- get the display*/
      case T_SCREEN:
      fgetnshort(file,args,4);
      maps[args[0]]=screen;
      max_map = Max(max_map,args[0]);
      break;
      /*}}}  */
      /*{{{  T_DATA   -- download some data*/
      case T_DATA:
      fgetnshort(file,args,4);
      max_map = Max(max_map,args[0]);
      if (!(new=maps[args[0]])) 
      {
        maps[args[0]] = new = bit_alloc(args[1], args[2], NULL, 1);
      }
      data = (char *) BIT_DATA(new);
      if (args[3] != 0 && maps[args[0]]==screen)
      { /* the screen */
        BITMAP *tmp = bit_alloc(args[1],args[2],NULL,1);
        data = (char *) BIT_DATA(tmp);
        fread(data, 1, bit_size(args[1],args[2],1), file);
        bit_blit(screen,0,0,BIT_WIDE(tmp),BIT_HIGH(tmp),BIT_SRC,tmp,0,0);
        dprintf(stderr,"DO fiddling screen data\n");
        bit_destroy(tmp);
      }
      else if (args[3] != 0)        /* all other bitmaps */
      fread(data, 1, bit_size(args[1],args[2],1), file);
      break;
      /*}}}  */
      /*{{{  T_TIME   -- get a timestamp*/
      case T_TIME:
      fgetnshort(file,args,2);
      old_time = your_time;                           
      your_time = scale +((args[0]<<16) | args[1]);/* ovrflws at ~6 min */

      if (base_time==0)                               /* first time in this script */
      base_time = your_time;

      /* ether 2 scripts are merged, or time overflowed */

      if (your_time < old_time) 
      {
        dprintf(stderr,"DO Time adj: %d < %d\n",your_time,old_time);
        if (old_time - scale > MAXUSHORT-THRES) {   /* overflow */
          dprintf(stderr,"correcting time overflow %d->%d\n",
          old_time, your_time);
          scale += MAXUSHORT;
          your_time += MAXUSHORT;
        }
        else {          /* this should never happen */
          dprintf(stderr,"DO Script merge, %d->%d correcting time\n",
          old_time,your_time);
          scale += (old_time-your_time) + 10;             /* merge */
          your_time += (old_time-your_time) + 10;         /* merge */
        }
      }

      /* get my time */

      if (y_time == 0) {                      /* initial time */
        y_time = your_time;
        m_time = my_time = timestamp();
      }
      else
      my_time = timestamp();

      /* scale speed */

      if (speed != 100)
      c = 100*(your_time-y_time)/speed - (my_time-m_time);
      else
      c = (your_time-y_time) - (my_time-m_time);

      /* sync script with timing marks */

      if (c>0) 
      {
        qprintf(stderr,"need to sleep %d/100 sec.(%d-%d - %d-%d)\n",
        c,your_time,y_time,my_time,m_time);
        if (c>300) dprintf(stderr, "Sleeping %d/100\n",c);
        if (c>MAX_SLEEP)
        c = MAX_SLEEP;
        return(c);      
      }
      else
      qprintf(stderr,"behind %d/100 sec.(%d-%d - %d-%d)\n",
      c,your_time,y_time,my_time,m_time);
      break;
      /*}}}  */
      /*{{{  T_POINT  -- plot a point*/
      case T_POINT:
      fgetnshort(file,args,3);
      bit_line(maps[args[0]], args[1], args[2], args[1], args[2], OPCODE(c));
      break;
      /*}}}  */
      /*{{{  T_KILL   -- destroy a bitmap*/
      case T_KILL:
      fgetnshort(file,args,1);
      if (args[0] == 0) kill_maps();
      else
      {
        qprintf(stderr,"kill %d\n", args[0]);
        if (maps[args[0]] && maps[args[0]] != screen) bit_destroy(maps[args[0]]);
        else dprintf(stderr,"Destroy: Invalid bitmap, %d\n", args[0]);
        maps[args[0]] = NULL;
      }
      break;
      /*}}}  */
      /*{{{  T_BYTESCROLL*/
      case T_BYTESCROLL:
      fgetnshort(file,args,6);
      max_map = Max(max_map,args[0]);
      if (maps[args[0]]) bit_bytescroll(maps[args[0]],args[1],args[2],args[3],args[4],args[5]);
      break;
      /*}}}  */
      /*{{{  default  -- invalid command*/
      default:
      printf("Oops, got 0x%x\n",c); fflush(stdout);
      break;
      /*}}}  */
    }
  }
  return(-1);
}
/*}}}  */

/*{{{  main*/
int main(argc,argv) int argc; char **argv;
{
  /*{{{  variables*/
  register int i;
  FILE *file;                             /* file to get script stuff from */
  FILE *vcr_file;         /* file to get control data from */
  int wait=0;                             /* time to wait before next grind call (0 for block) */
  int speed = 0;                  /* playback speed (100 = normal) */
  unsigned int bytes = 0;                 /* byte offset into audio file */
  int done=0;                             /* teminate flag */
  char *name=NULL;                /* name of script */
  int vcr=0;                              /* use VCR interface */
  int  x=0,y=0;                   /* screen window */
  int  w=0,h=0;                   /* " */
  int x_off=0, y_off=0;   /* coord translation offset */
  int stamp = 0;                  /* do timestamps for audio track generation */
  char *p;                                        /* misc char pointer */
  char *end=NULL;         /* tack "end" script at finish */
  BITMAP *display;
  /*}}}  */

  /*{{{  set up environment*/
  debug = getenv("DEBUG");
  debug2 = getenv("DEBUG2");
  quiet = getenv("QUIET");
  /*}}}  */
  /*{{{  parse the args*/
  for(i=1;i<argc;i++) 
  {
    if (*argv[i] == '-')
    switch(argv[i][1]) 
    {
      case 'x':                       /* set the display window */
      x = atoi(GET_OPT(i));
      break;
      case 'y':                       /* set the display window */
      y = atoi(GET_OPT(i));
      break;
      case 'w':                       /* set the display window */
      w = atoi(GET_OPT(i));
      break;
      case 'h':                       /* set the display window */
      h = atoi(GET_OPT(i));
      break;
      case 'S':                       /* set the time_stamper (broken) */
      stamp++;
      break;
      case 's':                       /* set the speed (for no vcr case) */
      speed = atoi(GET_OPT(i));
      break;
      case 'E':                       /* do end script */
      end = GET_OPT(i);
      break;
      case '\0':                      /* use stdin */
      name = "-";
      break;
      case 'v':                       /* run with the vcr */
      vcr++;
      break;
      default:
      fprintf(stderr,"Invalid flag %s ignored\n",argv[i]);
      break;
    }
    else
    name=argv[i];
  }

  if (name==NULL && vcr==0)
  {
    fprintf(stderr,"Must specify either script name or -v\n");
    exit(1);
  }
  /*{{{  open the display*/
  if (!quiet)
  {
    if ((display=bit_open(SCREEN_DEV))==NULL)
    {
      fprintf(stderr,"Can't open the display\n");
      exit(1);
    }
    else bit_grafscreen();
  }
  else if (quiet) display = bit_alloc(QUIET_WIDE,QUIET_HIGH,NULL,1);
  /*}}}  */
  /*{{{  initialize the bitmaps*/
  bzero(maps,sizeof(maps));
  /*}}}  */
  /*}}}  */
  /*{{{  set up display window*/
  if (w<=0) w = BIT_WIDE(display);
  if (h<=0) h = BIT_HIGH(display);

  dprintf(stderr,"DO Got window %d,%d %d x %d\n",x,y,w,h);
  screen = bit_create(display,x,y,w,h);
  /*}}}  */
  setlinebuf(stdout);
  if (vcr)
  /*{{{  */
  {
    setbuf(stdin,NULL);       /* stdio and select() don't mix */
    vcr_file = stdin;
    dprintf(stderr,"DO Using vcr mode\n");
    speed=0;
  }
  /*}}}  */
  else
  /*{{{  */
  {
    setup_script(argv[1]);
    file = new_script(next_script());
    vcr_file = NULL;
    speed = speed ? speed : 100;
    dprintf(stderr,"DO Using command mode [%s] %x!=0\n",name,file);
    printf("s %d\n",speed);         /* start audio */
  }
  /*}}}  */
  if (stamp) vcr_file = stdin;
  /*{{{  do the demo*/
  while(1)
  {
    /* EOF on script file */

    if (wait<0)
    {
      char *next = next_script();
      close_script(file);
      if (next)
      /*{{{  start the next script*/
      {
        file = new_script(next);
        wait=grind(screen,file,speed);
      }
      /*}}}  */
      else
      /*{{{  all done, wait*/
      {
        printf("s 0\n");                /* stop audio */
        speed = 0;
        close_script(file);
        kill_maps();
        if (ppid) 
        {
          kill(ppid,SIGHUP);      /* tell vcr were're done */
        }
        else 
        {
          bit_blit(screen,0,0,BIT_WIDE(screen),BIT_HIGH(screen),BIT_NOT(BIT_DST),0,0,0);
        }
      }
      /*}}}  */
    }

    if (do_pause(vcr_file,wait) > 0)
    /*{{{  got vcr input*/
    {
      if (fgets(line,sizeof(line),stdin)) break;
      dprintf(stderr,"DO Got line [%s]\n",line);
      switch(*line) 
      {
        /*{{{  P*/
        case 'P':                               /* parents pid */
        ppid = atoi(line+1);
        break;
        /*}}}  */
        /*{{{  S*/
        case 'S':                               /* new script */
        close_script(file);
        setup_script(line+2);
        file = new_script(next_script());
        wait=grind(screen,file,speed);
        break;
        /*}}}  */
        /*{{{  >*/
        case '>':                               /* volume up */
        printf(">\n");
        dprintf(stderr,"DO Volume up\n");
        break;
        /*}}}  */
        /*{{{  <*/
        case '<':                               /* volume down */
        printf("<\n");
        dprintf(stderr,"DO Volume down\n");
        break;
        /*}}}  */
        /*{{{  q*/
        case 'q':                               /* quit */
        kill_maps();
        close_script(file);
        /*
            bit_blit(screen,0,0,BIT_WIDE(screen),BIT_HIGH(screen),
                   BIT_SET,0,0,0);
        */
        break;
        /*}}}  */
        /*{{{  s*/
        case 's':                               /* set speed */
        speed = atoi(line+2);
        bytes = (your_time-base_time)*80;               /* wrong */
        printf("s %d\nB %d\n",speed,bytes);
        dprintf(stderr,"DO speed %d, bytes=%d\n",speed,your_time-base_time);

        /* speed was 0 */

        if (wait==0 && speed) {         /* start us up */
          dprintf(stderr,"DO Starting script/time speed %d\n",speed);
          y_time=0;
          wait=grind(screen,file,speed);
        }
        break;
        /*}}}  */
        /*{{{  c*/
        case 'c':                                       /* click: run 1 x-action */
        if (speed==0 && screen && file) 
        {
          dprintf(stderr,"Done time %d to",your_time-y_time);
          grind(screen,file,100);
          dprintf(stderr," %d %s\n",your_time-y_time);
        }
        break;
        /*}}}  */
        /*{{{  \0*/
        case '\000':                            /* send timestamp */
        bytes = your_time-base_time;
        if (stamp)
        printf("TIME: %d.%d\n",bytes/100,bytes%100);
        break;
        /*}}}  */
        /*{{{  default*/
        default:                                        /* unknown, send it on */
        printf("%s\n",line);
        dprintf(stderr,"DO unknown command [%c]\n",*line);
        break;
        /*}}}  */
      }
    }
    /*}}}  */
    else
    /*{{{  process video script until next timing mark*/
    {
      wait=grind(screen,file,speed);
      if (wait==0 && !vcr) { bit_destroy(display); exit(0); }
    }
    /*}}}  */
  }
  /*}}}  */
  /*{{{  clean up and exit*/
  if (!quiet) bit_blit(screen,0,0,BIT_WIDE(screen),BIT_HIGH(screen),BIT_CLR,0,0,0);
  bit_destroy(display);
  close_script(file);
  printf("Q\n");
  exit(0);
  /*}}}  */
}
/*}}}  */
