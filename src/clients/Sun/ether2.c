/*                        Copyright (c) 1987 Bellcore
 *                            All Rights Reserved
 *       Permission is granted to copy or use this program, EXCEPT that it
 *       may not be sold for profit, the copyright notice must be reproduced
 *       on copies, and credit should be given to Bellcore where it is due.
 *       BELLCORE MAKES NO WARRANTY AND ACCEPTS NO LIABILITY FOR THIS PROGRAM.
 */
/*	$Header: ether.c,v 4.2 88/06/22 14:37:31 bianchi Exp $
	$Source: /tmp/mgrsrc/demo/misc/RCS/ether.c,v $
*/
static char	RCSid_[] = "$Source: /tmp/mgrsrc/demo/misc/RCS/ether.c,v $$Revision: 4.2 $";

/* strip_chart vmstat output version II */

#include <signal.h>
#include <sys/ioctl.h>
#define OLDMGRBITOPS
#include <mgr/mgr.h>

/* color stuff */

#define MAX_COLORS	20
int colors[MAX_COLORS] = {
	0,
	9,
	3,
	4,
	5,
	2,
	1,1,1,1,1,
	};

int color=0;		/* true iff color mgr */

FILE *popen(), *file;

#define INTERVAL	60	/* bar interval (in secs) */
#define SCROLL		4	/* # of scrolls per window */
#define MAX		3	/* max number of plots */
#define FREQ		3	/* update frequency (secs)*/
#define DELTA		4	/* pixels/update */

#define Min(x,y)	((x)>(y)?(y):(x))
#define Max(x,y)	((x)>(y)?(x):(y))
#define dprintf		if (debug) fprintf

char *labels[] = {		/* graph labels */
   "Input" , "Output", "Coll."
   };

int indx[] = {			/* field indeces into netstat(1) */
   0, 2, 4
   };

int limit[] = {			/* default maximums */
   15, 15, 3
   };

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

   char host[16];	/* hostname */
   char title[255];	/* chart title */
   char line[255];	/* vmstat input buffer */
   char *fields[20];	/* place for raw input fields */

   int current[MAX];	/* current data value */
   int old[MAX];	/* previous data value */
   int first =1;	/* first time through */ 
   int clean();

   int f_high, f_wide;		/* font size */
   int high,wide;		/* window size */
   int x1;			/* left margin */
   int x2;			/* title */
   int y1;			/* title line */
   int y2;			/* first bar */
   int scroll;			/* scroll amount */
   int delta=DELTA;		/* pixels/line */
   int item;
   int dummy;
   int debug=0;
	int warn;

   ckmgrterm( *argv );

   for(i=1;i<argc;i++) {
      if (strcmp(argv[i],"-d")==0) {
         debug++;
         }
      if (strncmp(argv[i],"-c",2)==0) {
         set_colors(argv[i]+2);
         continue;
         }
      else if (strncmp(argv[i],"-f",2)==0) {
         freq = atoi(argv[i]+2);
         if (freq < 1) freq = 1;
         if (freq > 120) freq = 120;
         }
      else if (strncmp(argv[i],"-m",2)==0) {
         int max;
         max = atoi(argv[i]+2);
         if (max < 1) max = 1;
         if (max > 999) max = 999;
         limit[0] = limit[1] = max;
         limit[2] = (max>=5) ? max/5 : 1;
         
         }
      else
         fprintf(stderr,"%s: unknown flag %s. Ignored\n",*argv,argv[i]);
      }

   sprintf(line,"netstat %d",freq);
   if ((file = popen(line,"r")) == NULL)
      exit(1);

   m_setup(M_FLUSH);
   m_ttyset();
	color = is_color();
   m_push(P_EVENT|P_FLAGS);
   m_setmode(M_ABS);

   signal(SIGINT,clean);
   signal(SIGTERM,clean);

   system("stty -ctlecho");
   m_setevent(RESHAPE,"R\fRedrawing...\n");
   m_setevent(REDRAW,"R\fRedrawing...\n");
   first = 1;

   while (1) {

      for(size=0,i=0;i<MAX;i++)
         size = Max(size,strlen(labels[i]));

      /* clear the screen, flush pending input */

      read_it(fileno(m_termin),line);

      /* get font size */

      get_font(&f_wide,&f_high);

      /* get window size */

      get_size(&dummy,&dummy,&wide,&high);
   
      if (wide==0 || high==0 || f_wide==0 || f_high==0) {
         fprintf(stderr,"Can't get window info\n");
         clean();
         }

      /* get the title */

      gethostname(host,sizeof(host));

      sprintf(title,"Network statistics for %s in packets/%d seconds",
              host,freq);

      if (strlen(title)*f_wide > wide)
         sprintf(title,"%s (pkts/%ds.)",host,freq);

      /* make sure window is big enough */

      if (f_high * (2*MAX+1) > high) {
         fprintf(stderr,"\fWindow isn't tall enough\n");
         m_gets(line);
         continue;
         }
      if (f_high * (3*MAX +1) > high)
         size += 3;

      if (strlen(title)*f_wide > wide || 3*size*f_wide > wide*2) {
         fprintf(stderr,"\fWindow isn't\nwide enough\n");
         m_gets(line);
         continue;
         }

      /* calculate key positions */

      x1 = (size*f_wide+1);
      x2 = (wide-strlen(title)*f_wide)/2;
      y1 = f_high +1;
      y2 = (high - y1) /MAX;
      high--;

      m_func(B_SET);
		bg_color(1);
      m_clear();
      x = x1;
      scroll = Max((wide-x1)/SCROLL,10);
      scroll += scroll%delta;

      if (freq >15)
         interval = INTERVAL * 10 /freq;
      else 
         interval = INTERVAL / freq;

      /* draw form */

		fg_color(2);
      m_moveprint(x2,y1,title);

		fg_color(3);
		line_color(4);
      for(i=0;i<MAX;i++) {
         m_moveprint(x1-f_wide,high-i*y2,"0");
         sprintf(line,"%3d",limit[i]);
         m_moveprint(x1-f_wide*3,high-(i+1)*y2+f_wide*2+1,line);
         m_moveprint(1,high-i*y2-(y2-f_high)/2,labels[i]);
         m_line(x1,high-i*y2,wide,high-i*y2);
         }
   
      m_line(0,y1,wide,y1);
      m_line(x1,y1,x1,high);
      m_movecursor(x1,0);
      m_flush();

      /* read the data */

      while (fgets(line,sizeof(line),file) != NULL) {
         i = m_parse(line,fields);
         if (strcmp(*fields,"input")==0) {
            fgets(line,sizeof(line),file);
            fgets(line,sizeof(line),file);
            continue;
            }

         /* calculate new line position */

         for(i=0;i<MAX;i++) {
				line_color(6+i);
            current[i] = atoi(fields[indx[i]]) *
                         (y2-3)/limit[i];
				warn = current[i] > y2-3;
            current[i] = Min(current[i],y2-3) + y2*i + 1;

            if (!first) {
               m_line(x,high-old[i],x+delta,high-current[i]);
               if (solid) {
						if (warn) line_color(5);
                  m_line(x+delta,high-y2*i,x+delta,high-current[i]);
						}
               }

            dprintf(stderr,"%s %d->%d, ",labels[i],
                            high-old[i],high-current[i]);
            old[i] = current[i];
            }
         dprintf(stderr," [%d]\n",high);
   
			line_color(4);
         if (++bar  == interval) {
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
				line_color(1);
				if (color)
            	m_func(B_SRC);
				else
            	m_func(B_CLEAR);
            m_bitwrite(wide-scroll,y1+1,scroll,high);
            m_func(B_SET);
         
            dprintf(stderr,"scroll to %d,%d from %d,%d\n",
                    x1+1,y1+1,x1+scroll+1,y1+1);
				line_color(4);
            for(i=0;i<MAX;i++) 
               m_line(wide-scroll,high-i*y2,wide,high-i*y2);
            }
         m_flush();
         if (read_it(fileno(m_termin),line) && *line == 'R')
            break;
         }
      }
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
