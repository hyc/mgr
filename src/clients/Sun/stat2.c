/*                        Copyright (c) 1987 Bellcore
 *                            All Rights Reserved
 *       Permission is granted to copy or use this program, EXCEPT that it
 *       may not be sold for profit, the copyright notice must be reproduced
 *       on copies, and credit should be given to Bellcore where it is due.
 *       BELLCORE MAKES NO WARRANTY AND ACCEPTS NO LIABILITY FOR THIS PROGRAM.
 */
/*	$Header: stat.c,v 4.2 88/06/22 14:38:11 bianchi Exp $
	$Source: /tmp/mgrsrc/demo/misc/RCS/stat.c,v $
*/
static char	RCSid_[] = "$Source: /tmp/mgrsrc/demo/misc/RCS/stat.c,v $$Revision: 4.2 $";

/* strip_chart vmstat output version II */

/*
#define NEWESC	'E' 
*/

#include <signal.h>
#include <sys/ioctl.h>
#define OLDMGRBITOPS
#include <mgr/mgr.h>

#define MAXTRY	5		/* max number of tries for window parameters */

FILE *popen(), *file;

#define MAX_COLORS	20
int colors[MAX_COLORS] = {
	0,	/* unused */
	3,	/* background */
	6, /* title */
	4, /* lavbels */
	5, /* grid lines */
	2, /* exeeded max's */
	1, /* lines for plot 1 */
	1, /* lines for plot 2 */
	1, /* lines for plot 3 */
	1, /* lines for plot 4 */
	1, /* lines for plot 5 */
	};

int color=0;		/* true iff color mgr */

struct data {
   char *s_title;	/* short label */
   char *l_title;	/* long label */
   int index;		/* which item in vmstat */
   int max;		/* maximum value */
   };

struct data data[] = {
   "r",		"jobs in run q",	0,	5,
   "b",		"jobs blocked",		1,	5,
   "w",		"jobs waiting",		2,	5,
   "fre",	"free memory",		4,	2000,
   "fr",	"freed pages",		9,	10,
   "d1",	"disk 1 accesses",	12,	5,
   "d2",	"disk 2 accesses",	13,	5,
   "d3",	"disk 3 accesses",	14,	5,
   "d4",	"disk 4 accesses",	15,	5,
   "in",	"interrupts",		16,	60,
   "sy",	"system calls",		17,	60,
   "cs",	"context switches",	18,	30,
   "us",	"% user time",		19,	100,
   "kn",	"% system time",	20,	100,
   "id",	"% idle time",		21,	100,
   "",		"",			-1,	-1
   };

#define INTERVAL	60	/* bar interval (in secs) */
#define SCROLL		4	/* # of scrolls per window */
#define MAX		10	/* max number of plots */
#define FREQ		3	/* update frequency (secs)*/
#define DELTA		4	/* pixels/update */

#define Min(x,y)	((x)>(y)?(y):(x))
#define Max(x,y)	((x)>(y)?(x):(y))
#define dprintf		if (debug) fprintf

main(argc,argv)
int argc;
char **argv;
   {
   register int i;
   register int x;	/* current plot position */
   int bar=0;		/* hash mark counter */
   int back=0;		/* enable background writes */
   int solid=1;		/* make solid lines */
   int freq = FREQ;	/* update frequency (secs) */
   int size;		/* size of label region */
   int interval;	/* hash mark interval (secs) */
	int warn;		/* line exeeded range */

   char host[16];	/* hostname */
   char title[255];	/* chart title */
   char line[255];	/* vmstat input buffer */
   char *fields[25];	/* pntrs to vmstat fields */
   char labels[MAX][100];	/* place for labels */

   int current[MAX];	/* current data value */
   int old[MAX];	/* previous data value */
   int field[MAX];	/* index into 'data' */
   int first =1;	/* first time through */ 
   int clean();

   int f_high, f_wide;		/* font size */
   int high,wide;		/* window size */
   int x1;			/* left margin */
   int x2;			/* title */
   int y1;			/* title line */
   int y2;			/* first bar */
   int scroll;			/* scroll amount */
   int count;			/* number of plots */
   int delta=DELTA;		/* pixels/line */
   int item;
   int max = 0;
   int dummy;
   int debug=0;

   ckmgrterm( *argv );

   /* process arguments */

   if (argc<2) {
      fprintf(stderr,"usage: %s -[i<incr>sbf<freq>] [-max] arg ...\n",argv[0]);
      fprintf(stderr,"args:\n");
      for(i=0;data[i].max >=0;i++)
         fprintf(stderr,"  %s	(%s)\n",data[i].s_title,data[i].l_title);
      exit(1);
      }

   for(count=0,i=1;i<argc;i++) {
      if (strcmp(argv[i],"-d")==0) {
         debug++;
         continue;
         }
      if (strcmp(argv[i],"-b")==0) {
         back++;
         continue;
         }
      if (strcmp(argv[i],"-s")==0) {
         solid=0;
         continue;
         }
      if (strncmp(argv[i],"-c",2)==0) {
         set_colors(argv[i]+2);
         continue;
         }
      if (strncmp(argv[i],"-f",2)==0) {
         freq = atoi(argv[i]+2);
         if (freq < 1) freq = 1;
         if (freq > 120) freq = 120;
         continue;
         }
      if (strncmp(argv[i],"-i",2)==0) {
         delta = atoi(argv[i]+2);
         if (delta < 1) delta = 1;
         if (freq > 10) delta = 10;
         continue;
         }
      if (*argv[i] == '-') {
         max = atoi(argv[i]+1);
         continue;
         }
      if ((item = get_item(argv[i],data)) < 0) {
         fprintf(stderr,"%s:	%s is an invalid item\n",argv[0],argv[i]);
         continue;
         }
      if (max > 0) {
         data[item].max = max;
         max = 0;
         }
      field[count] = item;
      sprintf(labels[count],"%s",data[item].l_title);
      if (++count == MAX)
         break;
      }

   if (count < 1) {
      fprintf(stderr,"%s:	not enough fields\n,argv[0]");
      exit(5); 
      }

   sprintf(line,"vmstat %d",freq);
   if ((file = popen(line,"r")) == NULL)
      exit(1);

   m_setup(0);
   m_ttyset();
	color = is_color();
   m_push(P_EVENT|P_FLAGS);
   m_setmode(M_ABS);
   if (!back)
      m_setmode(M_BACKGROUND);
   else
      m_clearmode(M_BACKGROUND);

   signal(SIGINT,clean);
   signal(SIGTERM,clean);

   system("stty -ctlecho");
   m_setevent(RESHAPE,"R\fRedrawing...\n");
   m_setevent(REDRAW,"R\fRedrawing...\n");
   first = 1;

   while (1) {

      for(size=0,i=0;i<count;i++)
         size = Max(size,strlen(labels[i]));

      /* clear the screen, flush pending input */

      read_it(fileno(m_termin),line);

      /* get font size */

      for(i=0;i<MAXTRY && get_font(&f_wide,&f_high) < 0;i++);

      /* get window size */

      for(i=0;i<MAXTRY && get_size(0,0,&wide,&high) <= 0;i++);
   
      if (wide==0 || high==0 || f_wide==0 || f_high==0) {
         fprintf(stderr,"Can't get window info\n");
         clean();
         }

      /* get the title */

      gethostname(host,sizeof(host));

      sprintf(title,"Statistics for %s (%d second intervals)",host,freq);

      if (strlen(title)*f_wide > wide)
         sprintf(title,"%s (%d sec.)",host,freq);

      /* make sure window is big enough */

      if (f_high * (count+1) > high) {
         fprintf(stderr,"\fWindow isn't tall enough\n");
         m_gets(line);
         continue;
         }

      if (strlen(title)*f_wide > wide || 3*size*f_wide > wide*2) {
         fprintf(stderr,"\fWindow isn't\nwide enough\n");
         m_gets(line);
         continue;
         }

      /* calculate key positions */

      x1 = (size*f_wide+1);
      x2 = (wide-strlen(title)*f_wide)/2;
      y1 = f_high +1;
      y2 = (high - y1) /count;
      high--;

      m_func(B_SET);
		bg_color(1);
      m_clear();
      x = x1;
      scroll = Max((wide-x1)/SCROLL,10);
      scroll += scroll%DELTA;

      if (freq >15)
         interval = INTERVAL * 10 /freq;
      else 
         interval = INTERVAL / freq;

      /* draw form */

		fg_color(2);
      m_moveprint(x2,y1,title);

		fg_color(3);
      line_color(4);
      for(i=0;i<count;i++) {
         char tmp[10];
         if (f_high * (3*count+1) <= high) {
            sprintf(tmp,"%d",data[field[i]].max);
            m_moveprint(x1-f_wide*strlen(tmp),high-(i+1)*y2+f_wide*2+1,tmp);
            m_moveprint(x1-f_wide,high-i*y2,"0");
            m_moveprint(1,high-i*y2-f_high,labels[i]);
            }
         else
            m_moveprint(1,high-i*y2-1,labels[i]);
         m_line(x1,high-i*y2,wide,high-i*y2);
         }
   
      m_line(0,y1,wide,y1);
      m_line(x1,y1,x1,high);
      m_movecursor(x1,0);
      m_flush();

      /* read the data */

      while (fgets(line,sizeof(line),file) != NULL) {
         i = m_parse(line,fields);
         if (strcmp(*fields,"procs")==0) {
            fgets(line,sizeof(line),file);
            fgets(line,sizeof(line),file);
            continue;
            }
          if (i < 22) continue;

         /* calculate new line position */

         for(i=0;i<count;i++) {
				line_color(6+i);
            current[i] = atoi(fields[data[field[i]].index]) *
                         (y2-3)/data[field[i]].max;
				warn = (current[i] > y2-3);
            current[i] = Min(current[i],y2-3) + y2*i + 1;

            if (!first) {
               m_line(x,high-old[i],x+delta,high-current[i]);
               if (solid) {
						if (warn) line_color(5);
                  m_line(x+delta,high-y2*i,x+delta,high-current[i]);
						}
               }

            dprintf(stderr,"%s %d->%d, ",data[field[i]].s_title,
                            high-old[i],high-current[i]);
            old[i] = current[i];
            }
         dprintf(stderr," [%d]\n",high);
   
         if (++bar  == interval) {
      		line_color(4);
            m_line(x,y1,x,high);
            bar = 0;
            dprintf(stderr,"---------\n");
            }

         if (first)
            first = 0;
         else
            x += delta;

         if (x > wide-delta) {

            /* scroll the display */

            x -= scroll;
            m_func(B_COPY);
            m_bitcopy(x1+1,y1+1,wide-x1-1,high-y1-1,x1+scroll+1,y1+1);
				if (color) {
					line_color(1);
            	m_func(B_SRC);
					}
				else
            	m_func(B_CLEAR);
            m_bitwrite(wide-scroll,y1+1,scroll,high);
            m_func(B_SET);
         
            dprintf(stderr,"scroll to %d,%d from %d,%d\n",
                    x1+1,y1+1,x1+scroll+1,y1+1);
				line_color(4);
            for(i=0;i<count;i++) 
               m_line(wide-scroll,high-i*y2,wide,high-i*y2);
            }
         m_flush();
         if (read_it(fileno(m_termin),line) && *line == 'R')
            break;
         }
      }
   }

int
get_item(s,data)	/* look up an parameter */
char *s;
struct data data[];
   {
   register int i;

   for(i=0;data[i].index>=0;i++)
      if (strcmp(s,data[i].s_title)==0)
        return(i);
   return(-1);
   }

clean()			/* clean up on SIGINT */
   {
   m_pop();
   pclose(file);
   m_clear();
   m_flush();
   m_ttyreset();
   exit(1);
   }

int read_it(fd,line)	/* non blocking read */
int fd;
char *line;
   {
   long rd;

   ioctl(fd,FIONREAD,&rd);
   line[rd] = '\0';
   if (rd > 0)  {
      return(read(fd,line,rd));
      }
   else
      return(0);
   }

/* see if MGR is color */

int
is_color()
	{
	return(m_getdepth() > 1);
	}

/* set the bg color */

int
bg_color(n)
int n;
	{
	if (color) m_bcolor(colors[n]);
	}

/* set the FG color */

int
fg_color(n)
int n;
	{
	if (color) m_fcolor(colors[n]);
	}

/* set the line color */

int
line_color(n)
int n;
	{
	if (color) m_linecolor(B_SRC,colors[n]);
	}

/* get colors from the command line */

int
set_colors(s)
register char *s;
	{
	register int i=0;
	register unsigned char c;

	while((c = *s++) && i<MAX_COLORS) {
		if (c>='0' && c <= '9')
			colors[i++] = c - '0';
		else if (c >= 'a' && c <= 'z')
			colors[i++] = 16 + c - 'a';
		else if (c >= 'A' && c <= 'Z')
			colors[i++] = 16 + c - 'A';
		}
	}
