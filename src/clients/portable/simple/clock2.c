/*{{{}}}*/
/*{{{  Notes*/
/* get today's date  (analog clock version) */
/*}}}  */
/*{{{  #includes*/
#include <mgr/mgr.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>
/*}}}  */
/*{{{  #defines*/
#define OFFSET		(500<<10)		/* shift points to 1st quad */
/*}}}  */

/*{{{  sin, cos tables: 0-360 deg. in 6 deg. increments (scaled to +/- 1024)*/
int int_sin[] = {
	 0, 107, 212, 316, 416, 512,
	 601, 685, 760, 828, 886, 935,
	 973, 1001, 1018, 1023, 1018, 1001,
	 973, 935, 886, 828, 760, 685,
	 601, 511, 416, 316, 212, 107,
	 0, -107, -212, -316, -416, -512,
	 -601, -685, -760, -828, -886, -935,
	 -973, -1001, -1018, -1023, -1018, -1001,
	 -973, -935, -886, -828, -760, -685,
	 -601, -511, -416, -316, -212, -107,
	 0 };


int int_cos[] = {
	 1023, 1018, 1001, 973, 935, 886,
	 828, 760, 685, 601, 511, 416,
	 316, 212, 107, 0, -107, -212,
	 -316, -416, -512, -601, -685, -760,
	 -828, -886, -935, -973, -1001, -1018,
	 -1023, -1018, -1001, -973, -935, -886,
	 -828, -760, -685, -601, -511, -416,
	 -316, -212, -107, 0, 107, 212,
	 316, 416, 512, 601, 685, 760,
	 828, 886, 935, 973, 1001, 1018,
	 1023 };
/*}}}  */
/*{{{  variables*/
#define MAX_COLORS	20
int colors[MAX_COLORS] = {
	3,	/* background */
	1, /* face and tics */
	4, /* hands */
	2, /* second hands  */
	};

int color=0;		/* true iff color mgr */

typedef struct coord {
	int x,y;
	} coord;

/* coordinates of the hands at 12:00 */

coord second[] = { {1,-30},  {0,485} };
coord minute[] = { {10,-10},  {0,400},  {-10,-10},  {10,-10} };
coord hour[] = { {35,-10},  {0,270},  {-35,-10},  {35,-10} };

coord big_tic[] = { { -11,485} ,  {0,450},  {11,485} };
coord tic[] = { {0,485},  {0,460} };

int h, old_h, m, old_m, s, old_s;
/*}}}  */

/*{{{  clean -- clean up and exit*/
void clean(n)
int	n;
   {
   m_popall();
   m_clear();
	m_setcursor(0);
   m_flush();
   m_ttyreset();
   exit(1);
   }
/*}}}  */
/*{{{  is_color -- see if MGR is color*/
int
is_color()
	{
	return(m_getdepth() > 1);
	}
/*}}}  */
/*{{{  bg_color -- set the bg color*/
void
bg_color(n)
int n;
	{
	if (color) m_bcolor(colors[n]);
	}
/*}}}  */
/*{{{  fg_color -- set the FG color*/
void
fg_color(n)
int n;
	{
	if (color) m_fcolor(colors[n]);
	}
/*}}}  */
/*{{{  line_color -- set the line color*/
void
line_color(n)
int n;
	{
	if (color) m_linecolor(BIT_SRC,colors[n]);
	}
/*}}}  */
/*{{{  draw -- rotate and draw hands*/
#define X(i)	(what[i].x)
#define Y(i)	(what[i].y)

void draw(what, n, theta)
coord what[];			/* array of points */
int n;				/* number of points */
int theta;			/* angle (in 160th of a circle) */
   {
   register int i;

   for(i=0;i<n;i++) {
      m_line((X(i)*int_cos[theta]+Y(i)*int_sin[theta] + OFFSET)>>10,
           (X(i)*int_sin[theta]-Y(i)*int_cos[theta] + OFFSET)>>10,
           (X(i+1)*int_cos[theta]+Y(i+1)*int_sin[theta] + OFFSET)>>10,
           (X(i+1)*int_sin[theta]-Y(i+1)*int_cos[theta] + OFFSET)>>10);
           }
   }
/*}}}  */
/*{{{  get_time -- convert time to angles*/
void get_time(h,m,s)
int *h, *m, *s;		/* hours, minutes, seconds */
   {
   struct tm *tme, *localtime();
   long tmp,time();
    
   tmp = time(0);
   tme = localtime(&tmp);

   *m = tme->tm_min;
   *s = tme->tm_sec;
   if (tme->tm_hour > 11)
      tme->tm_hour -= 12;
   *h = tme->tm_hour*5 + (2+tme->tm_min)/12;
   }
/*}}}  */
/*{{{  dotime -- update the time*/
void dotime(int n)
   {
   old_h=h, old_m=m, old_s=s;
   get_time(&h, &m, &s);

	line_color(3);
   m_func(BIT_XOR);
   draw(second,1,old_s);
   if (old_m != m) {
		if (color) {
      	m_func(BIT_SRC);
			line_color(0);
			draw(minute,3,old_m);
			draw(hour,3,old_h);
			line_color(2);
			draw(hour,3,h);
			draw(minute,3,m);
			}
		else {
      	m_func(BIT_CLR);
			draw(minute,3,old_m);
			draw(hour,3,old_h);
			m_func(BIT_SET);
			draw(hour,3,h);
			draw(minute,3,m);
			}
      }

	line_color(3);
   m_func(BIT_XOR);
   draw(second,1,s);
   m_flush();
   signal(SIGALRM,dotime);
   alarm(1);
   }
/*}}}  */
/*{{{  set_colors -- get colors from the command line*/
void
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
/*}}}  */

/*{{{  main*/
int main(int argc, char *argv[])
   {
   register int i;
   int cal=0;
   char line[80];

   ckmgrterm( *argv );

   if (argc>1 && strcmp(argv[1],"-c")==0)
      cal++;
   else if (argc>1 && strncmp(argv[1],"-c",2)==0)
		set_colors(argv[1]+2);

   /* setup mgr environment */

   m_setup(M_MODEOK);
   m_ttyset();
   m_push(P_FLAGS|P_EVENT);
	m_setcursor(9);
	color=is_color();

   m_setevent(REDRAW,"R\n");
   m_setevent(RESHAPE,"R\n");
   if (cal)
      m_setevent(ACTIVATE,"A\n");

   signal(SIGALRM,dotime);
   signal(SIGTERM,clean);
   signal(SIGINT,clean);
   signal(SIGHUP,clean);

   while(1) {
		bg_color(0);
		fg_color(0);
      m_func(BIT_SET);
      m_clear();
		line_color(1);
      m_ellipse(500,500,490,490);
      for(i=0;i<60;i+=5) 		/* the tic marks */
         if (i%15==0)
            draw(big_tic,2,i);
         else
            draw(tic,1,i);

      m_movecursor(500,500);
      get_time(&h, &m, &s);

		line_color(2);
      draw(hour,3,h);
      draw(minute,3,m);

		line_color(3);
      m_func(BIT_XOR);
      draw(second,1,s);
      m_flush();
      dotime(0);

      /* wait for an event */

      while(1) {
	 extern int   errno;

         errno = 0;
	 *line = '\0';
         if (m_gets(line) == NULL  &&  errno  &&  errno != EINTR)
            clean(0);
         alarm(0);
         if (*line=='R')
            break;
         else if (cal && *line == 'A') {
            long time(), tmp = time(0);
            char *ctime();

            m_push(P_ALL);
            m_font(11);
            m_size(27,3);
            m_clear();
            m_printstr("\n "); 
            m_printstr(ctime(&tmp));
            m_flush();
            sleep(3);
            m_pop();
            m_clearmode(M_ACTIVATE);
            m_flush();
            }
         dotime(0);
         }
      }
   }
/*}}}  */
