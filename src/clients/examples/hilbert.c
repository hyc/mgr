#include <mgr/mgr.h>
#include <signal.h>

/* program to draw hilbert space filling curves (very quick).
 * Steve Hawley			11/87 (Macintosh C implementation)
 * translated from a pascal version from the Oberlin Computer Science
 * Lab manual, Fall 1984, Author unknown.
 * --------- ported to mgr by SAU: FAST version uses fast vector mode
 */

#define SIZE	75			/* maximum # of points in a shot */
#define MAX	7				/* maximum distance */
static int count = 0;
char buff[1024];	/* grunge buffer */

/* function prototypes */
void rhilbert(int, int);
void Line(int, int);

int dir;

/* global direction variable.  The original program used turtle graphics
 * with turn functions taking angles.  I cheat since all turns in this
 * program are 90 degrees, and make the directions be:
 *		0: down
 *		1: right
 *		2: up
 *		3: left
 */

void
start()
{
	/* put the graphics cursor somewhere nice, and initialize
	 * the direction.
	 */

	m_go(10,10);

	dir = 0;
}

void
left()
{
	/* a turn to the left is actually the direction + 3
	 * modulo 3.
	 */
	dir = (dir + 3) & 0x3;
}

void
right()
{
	/* a right turn is the direction + 1 modulo 3 */
	dir = (dir + 1) & 0x3;
}

void
forward(size)
register int size;
{
	/* move the graphics cursor and draw a line segment.
	 * The Macintosh function Line(dh, dv) draws a line from the
	 * current graphics cursor to the graphics cursor displaced
	 * by dh and dv (horizontal and vertical deltas).
	 */
	switch(dir) {
	case 0:
		Line(0, size);
		break;
	case 1:
		Line(size, 0);
		break;
	case 2:
		Line(0, -size);
		break;
	case 3:
		Line(-size, 0);
		break;
	}
}

/* mutually recursive hilbert functions: */
void
lhilbert(size, level)
register int size, level;
{
	if (level > 0) {
		left();
		rhilbert(size, level-1);
		forward(size);
		right();
		lhilbert(size, level-1);
		forward(size);
		lhilbert(size, level-1);
		right();
		forward(size);
		rhilbert(size, level-1);
		left();
	}
}

void
rhilbert(size, level)
register int size, level;
{
	if (level > 0) {
		right();
		lhilbert(size, level-1);
		forward(size);
		left();
		rhilbert(size, level-1);
		forward(size);
		rhilbert(size, level-1);
		left();
		forward(size);
		lhilbert(size, level-1);
		right();
	}
}

/* flush pending grunge data */

void
flush()
{
	if (count > 0) {
		m_rfastdraw(count,buff);
		count = 0;
	}
}

void
clean()
{
	flush();
	m_flush();
	exit(0);
}

int
main (argc,argv)
int	argc;
char	**argv;
{
	ckmgrterm( *argv );
 	m_setup(0);	
	signal(SIGTERM,clean);
	signal(SIGINT,clean);
	signal(SIGHUP,clean);
	m_func(BIT_SET);
	/* initialize */
	start();
	m_clear();
	/* draw the hilbert (this is *very* fast) */
	rhilbert(8, 7);
	clean();
        return 255;
}

/* FAST drawing stuff */

/* add delta to grunge list */

void
Line(dx,dy)
register int dx,dy;
{
	register int mx,my;

	if (dx > MAX || dy > MAX) {
		mx = (dx>>1);
		my = (dy>>1);
	   buff[count++] = (mx+8)<<4 | (my+8);
		dx = dx-mx;
		dy = dy-my;
		}

	buff[count++] = (dx+8)<<4 | (dy+8);
	if (count >= SIZE)
		flush();

}
