/* pgs: Preview GhostScript, a postscript previewer for MGR  (S A Uhler)
 * Based on pilot, by s.d. Hawley
 * linux/sun port by V. Broman, broman@nosc.mil .
 */

#include <sys/types.h>
#include <sys/time.h>
#include <sys/wait.h>
#ifdef sun
#include <stropts.h>
#endif
#include <memory.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <termios.h>
#include <unistd.h>

#define M_NOFLUSH		/* disable automatic MGR flushing */

#define OLDMGRBITOPS		/* this client's bitblit codes are old style */
#ifdef OLDMGR			/* environment is old mgr */
#include "term.h"
#else
#include <mgr/mgr.h>
#endif

#define dprintf	if (debug) fprintf
#define dflush		if (debug) fflush
#define GET_OPT(i)	\
	strlen(argv[i])>2 ? argv[i]+2 : argv[++i]

#define SCALE 		6	/* scrolling distance scale factor */
#define BORDER		4	/* size of top border */
#define MAXQ		3	/* max # of button events in Q */
#define INTERVAL	5	/* button down repeat interval (100th secs) */
#define RES			75	/* default resolution dots/inch */
#define MAXPAGES	200	/* max # of pages */
#define TMPDIR		"/tmp"

#ifndef E_XMENU
#define E_XMENU	'X'		/* extended menu operations */
#endif

/* format strings */

#ifndef VERSION
#define VERSION "PGS 1.3 (c) 1992 Bellcore, by S A Uhler:"
#endif
#ifndef GS
#define GS "gs -r%d -sDEVICE=mgrmono -q -sOUTPUTFILE=%s %s"
#endif
#define TMP_FILE	"%s/pgs_%d.%%d"
#define TITLE		"\r%s: page %d of %d %s"

int debug;			/* enable debugging */
int reverse = 0;		/* reverse video */
int done = 0;			/* done processing GS file */
int wx, wy, ww, wh;		/* window dims */
int mw, mh;			/* map width and height */
int sx, sy, sw, sh;		/* subwindow on map coords */
int old_x = -1, old_y = -1;	/* old map coords */
int q = 0;			/* # of events in button 1 event q */
int pid;			/* pid of gs process */
int fw, fh;			/*      font size */
fd_set mask;			/* select mask */
int fd;				/* fd for m_termin (/dev/tty) */
int pages = 0;			/* number of pages processed so far */
char *name = NULL;		/* name of ps file */

void move(int, int);
int get_file(char **, int, int, int);
void timer();
int start_timer(int);
int end_timer();
int get_till(int, char *, char *);
int run_it(char *, int *, int *, int);
char *save_line(char *);
void child();
void do_title(int, int, char *);
void cleanup(char *);
void init_window();

char line[1024];		/* MGR input buffer */
char temp_file[1024];		/* ps temp file pattern */
char *names[MAXPAGES];		/* where to hold page names */

static struct menu_entry menu[] =
{
	{ "goto page =>", ""},
	{ "next page", "n"},
	{ "prev page", "p"},
	{ "quit", "q"},
	{ "suspend", "\032"}
};

/* only handle 1st 20 pages for now */

struct menu_entry page_menu[] =
{
	{ " 1", "P 1"},
	{ " 2", "P 2"},
	{ " 3", "P 3"},
	{ " 4", "P 4"},
	{ " 5", "P 5"},
	{ " 6", "P 6"},
	{ " 7", "P 7"},
	{ " 8", "P 8"},
	{ " 9", "P 9"},
	{ "10", "P 10"},
	{ "11", "P 11"},
	{ "12", "P 12"},
	{ "13", "P 13"},
	{ "14", "P 14"},
	{ "15", "P 15"},
	{ "16", "P 16"},
	{ "17", "P 17"},
	{ "18", "P 18"},
	{ "19", "P 19"},
	{ "20", "P 20"}
};

int main(argc, argv)
int argc;
char **argv;
{
	char *getenv();
	char *fixfilename();
	int i;
	int res = RES;		/* gs resolution */
	int bx, by;		/* button 1 position */
	fd_set reads;		/* select read mask */
	int to, from;		/* fd's to talk to gs */
	int from_stdin = 0;	/* true if "gs -" */
	char c;			/* input character from mgr */
	int file_no = 0;	/* current file # */
	char *temp;		/* temp file directory */
	int interval = INTERVAL;	/* button auto-repeat interval */
	char *rindex();

	debug = getenv("DEBUG") ? 1 : 0;
	dprintf(stderr, "debugging on\n");

	ckmgrterm("pgs");

	temp = getenv("TMPDIR");
	if (!temp)
		temp = TMPDIR;

	/* check arguments */

	for (i = 1; i < argc; i++) {
		if (*argv[i] == '-')
			switch (argv[i][1]) {
			case '\0':	/* use stdin */
				name = "-";
				break;
			case 'a':	/* auto repeat interval */
				interval = atoi(GET_OPT(i));
				break;
			case 'i':	/* inverse video */
				reverse++;
				break;
			case 'r':	/* set resolution */
				res = atoi(GET_OPT(i));
				if (res > 600 || res < 10)
					res = RES;
				break;
			case 'd':	/* enable debugging */
				debug = 1;
				break;
			default:
				fprintf(stderr, "%s: invalid flag %c ignored\n",
					argv[0], argv[i][1]);
		} else if (!name)
			name = argv[i];
		else
			dprintf(stderr, "arg [%s] ignored\n", argv[i]);
	}

	if (!name) {
		fprintf(stderr, "usage: %s: [-i -r<res> -a<repeat> -d] <gs_file>\n", *argv);
		exit(1);
	}
	/* run the GS interpreter */

	sprintf(temp_file, TMP_FILE, temp, (int) getpid());
	sprintf(line, GS, res, temp_file, name);
	if ((pid = run_it(line, &to, &from, from_stdin)) == -1) {
		fprintf(stderr, "Sorry, Can't fork\n");
		exit(3);
	}
	dprintf(stderr, "pid=%d, fd's = %d,%d\n", pid, to, from);

	/* fix up the file name */

	if ((from_stdin = (strcmp(name, "-") == 0)))
		name = "(stdin)";
	else if (rindex(name, '/'))
		name = rindex(name, '/') + 1;

	/* setup MGR */

	m_setup(0);
	fd = fileno(m_termin);
	FD_ZERO(&mask);
	FD_SET(fd, &mask);
	FD_SET(from, &mask);
	dprintf(stderr, "mask (first int of) = 0x%x\n", *(int *)&mask);
	signal(SIGINT, cleanup);
	signal(SIGTERM, cleanup);
	signal(SIGCHLD, child);
	signal(SIGALRM, timer);
	m_ttyset();
	init_window();
	m_setraw();		/* MUST be in raw mode */
	{
		char title[100];

		sprintf(title, "\r%s previewing file %s ", VERSION, name);
		m_printstr(title);
	}

	while (1) {
		memcpy(&reads, &mask, sizeof(fd_set));
		m_flush();
		if (select(FD_SETSIZE,
			   &reads, (fd_set *)0, (fd_set *)0,
			   (struct timeval *)0
			   ) < 0) {
			dprintf(stderr, "select error\n");
			continue;
		}
		dprintf(stderr, "(first int of) reads returned 0x%x\n", *(int *)&reads);

		if (FD_ISSET(fd, &reads)) {	/* from MGR */
			read(fd, &c, 1);
			dprintf(stderr, "Got c=[0%o] %c\n", c, c);
			switch (c) {
			case 'P':	/* goto page (from menu) */
				get_till(fd, line, "!");
				sscanf(line, "%d", &file_no);
				file_no = get_file(names, reverse, file_no - 1, pages);
				break;
			case 'n':
			case '\r':	/* next file */
				file_no = get_file(names, reverse, file_no + 1, pages);
				break;
			case 'p':	/* previous file */
				file_no = get_file(names, reverse, file_no - 1, pages);
				break;
			case 'j':	/* go down */
				move(0, (wh / 2) / SCALE);
				break;
			case 'l':	/* go right */
				move((ww / 2) / SCALE, 0);
				break;
			case 'h':	/* go left */
				move((0 - ww / 2) / SCALE, 0);
				break;
			case 'k':	/* go up */
				move(0, (0 - wh / 2) / SCALE);
				break;
			case '$':	/* button release */
				end_timer();
				break;
			case 'X':	/* button hit repeat */
				q--;
				get_till(fd, line, "!");
				sscanf(line, "%d %d", &bx, &by);
				move((bx - ww / 2) / SCALE, (by - wh / 2) / SCALE);
				break;
			case 'B':	/* button hit */
				get_till(fd, line, "!");
				sscanf(line, "%d %d", &bx, &by);
				dprintf(stderr, "B got %d,%d\n", bx, by);
				move((bx - ww / 2) / SCALE, (by - wh / 2) / SCALE);
				q = 1;
				start_timer(interval);
				break;
			case 'R':	/* window redraw */
				m_func(B_CLEAR);
				m_bitwrite(0, 0, ww, wh + BORDER);
				m_func(B_SET);
				m_bitwrite(0, fh - BORDER + 1, ww, BORDER - 2);
				m_func(B_COPY);
				m_bitcopyto(0, fh, sw, sh, sx, sy, 0, 1);
				do_title(file_no + 1, pages, done ? "" : "(processing)");
				break;
			case 'S':	/* window reshape */
				get_size(&wx, &wy, &ww, &wh);
				wh -= fh;
				sw = ww;
				sh = wh;
				m_func(B_SET);
				m_bitwrite(0, fh - BORDER + 1, ww, BORDER - 2);
				m_func(B_COPY);
				m_bitcopyto(0, fh, sw, sh, sx, sy, 0, 1);
				break;
			case 'Q':
			case 'q':
			case '\004':
				tcflush(fd,TCIFLUSH);
				cleanup("Bye now!");
				break;
			case 'g':	/* goto page (from keyboard) */
				m_printstr("\rpage: ");
				m_cleareos();
				m_setcursor(0);
				m_setnoraw();
				m_setecho();
				get_till(fd, line, "\r\n");	/* get page # in cooked mode */
				m_setraw();
				m_setnoecho();
				m_setcursor(CS_INVIS);
				if (sscanf(line, "%d", &bx) && bx <= pages)
					file_no = get_file(names, reverse, bx - 1, pages);
				else
					do_title(file_no + 1, pages, done ? "" : "(processing)");
				break;
			case '\032':	/* suspend */
				m_pop();
				m_setnoraw();
				m_setecho();
				printf("suspended!\n");
				m_flush();
				signal(SIGCHLD, SIG_DFL);
				kill(0, SIGTSTP);	/* send stop signal to self */
				signal(SIGCHLD, child);
				m_setnoecho();
				init_window();
				m_setraw();
				get_file(names, reverse, file_no, pages);
				m_flush();
				break;
			default:	/* invalid */
				dprintf(stderr, "Got [%o]\n", c);
				break;
			}
			continue;
		}
		if (FD_ISSET(from, &reads)) {	/* from GS commmand */
			get_till(from, line, ">\r\n");
			if (strncmp(line, "GS", 2) == 0) {	/* done */
				dprintf(stderr, "GS done\n");
				done = 1;
				file_no = get_file(names, reverse, file_no, pages);
				if (!from_stdin)
					write(to, "\004", 1);
				kill(pid, SIGINT);	/* just in case */
				if (pages == 0) {
					cleanup("No valid pages obtained from GhostScript");
				}
			} else if (strncmp(line, "Unknown", 7) == 0) {	/* unknow device */
				cleanup("GhostScript doesn't have the MGR driver installed");
			} else if (strncmp(line, "showpage", 8) == 0) {		/* got a page */
				sprintf(line, temp_file, pages + 1);
				dprintf(stderr, "Loading file %s\n", line);
				names[pages] = save_line(line);
				pages++;
				if (pages <= 20)
					menu_load(2, pages, page_menu);
				dprintf(stderr, "sending GS NL\n");
				if (pages == 1)		/* first page */
					file_no = get_file(names, reverse, file_no, pages);
				else
					do_title(file_no + 1, pages, done ? "" : "(processing)");
				if (!from_stdin)
					write(to, "\n", 1);
			}
		}
	}
}

/* move "window" */

void
move(dh, dv)
int dh, dv;
{
	sx += dh;
	if (sx + sw > mw)
		sx = mw - sw;
	if (sx < 0)
		sx = 0;

	sy += dv;
	if (sy + sh > mh)
		sy = mh - sh;
	if (sy < 0)
		sy = 0;

	dprintf(stderr, "Moving %d x %d to %d , %d\n",
		sw, sh, sx, sy);
	if (old_x != sx || old_y != sy) {
		m_bitcopyto(0, fh, sw, sh, sx, sy, 0, 1);
		old_x = sx, old_y = sy;
	}
}

/* find an absolute path name */


char *
fixfilename(s)
char *s;
{
	static char path[1024];

	if (!s || *s == '/')
		return (s);
	getwd(path);
	strcat(path, "/");
	strcat(path, s);
	return (path);
}

/* get the next file, return index */

int
get_file(name, rev, num, max)
char **name;			/* list of file names */
int rev;			/* true if reverse */
int num;			/* number in list */
int max;			/* max number in list */
{
	char *fname;

	if (num < 0)
		return (0);
	if (num >= max)
		return (max - 1);

	if (!name[num]) {
		do_title(num + 1, max, "(file not found)");
		return (num);
	}
	fname = fixfilename(name[num]);
	dprintf(stderr, "Getting file %s (page %d/%d)\n", fname, num, max);
	dflush(stderr);
	m_bitfromfile(1, fname);
	get_till(fd, line, "\r\n");
	sscanf(line, "%d %d", &mw, &mh);
	if (mw == 0 || mh == 0) {
		do_title(num + 1, max, "(invalid format)");
		return (num);
	}
	if (rev) {
		m_func(BIT_NOT(B_DST));
		m_bitwriteto(0, 0, mw, mh, 1);
		m_func(B_COPY);
	}
	sx = sy = 0;
	sw = ww;
	sh = wh;
	old_x = old_y = -1;
	m_bitcopyto(0, fh, sw, sh, sx, sy, 0, 1);
	m_menuitem(2, num);
	if (num == 0)
		m_menuitem(1, 1);
	else if (num >= max - 1)
		m_menuitem(1, 2);
	do_title(num + 1, max, done ? "" : "(processing)");
	return (num);
}

/* timer routines */

static struct itimerval val;
static struct timeval timev;
static int e_count = 0;

/* timeout, send mouse location */

void
timer()
{
	if (q > MAXQ) {
		dprintf(stderr, "max_q at %d\n", e_count);
	}
	m_sendme("X %p!");
	m_flush();
	q++;
	e_count++;
}

/* start the timer to "go off" on a regular basis */

int
start_timer(n)
int n;				/* 100ths of seconds */
{
	static struct timeval itime =
	{0, 300000};		/* initial value of timer */

	if (n == 0)
		return (0);
	timev.tv_sec = n / 100;
	timev.tv_usec = (n % 100) * 10000;
	val.it_interval = timev;
	val.it_value = itime;
	return (setitimer(ITIMER_REAL, &val, NULL));
}

int
end_timer()
{
	timev.tv_sec = 0;
	timev.tv_usec = 0;
	val.it_interval = timev;
	val.it_value = timev;
	return (setitimer(ITIMER_REAL, &val, NULL));
}

/* read until record boundary, skip leading boundary characters */

int
get_till(_fd, s, term)
int _fd;			/* fd to read from */
char *s;			/* buffer to dump chars into */
char *term;			/* list of record terminators */
{
	char *index();
	int i = 0;
	int n;

	dprintf(stderr, "Get..");
	dflush(stderr);
	m_flush();
	while ((n = read(_fd, s, 1)) == 1) {	/* single char reads, YUK ! */
		dprintf(stderr, "%c", *s);
		dflush(stderr);
		if (index(term, *s)) {
			if (i == 0) {
				dprintf(stderr, "get_till skipping [%c]\n", *s);
				continue;
			} else {
				dprintf(stderr, "get_till ending with [%c]\n", *s);
				break;
			}
		}
		i++;
		s++;
	}
	if (n != 1) {
		dprintf(stderr, "read returned %d\n", n);
		perror("read in get_till");
	}
	*s = '\0';
	dprintf(stderr, "\n");
	return (i);
}

/* run a command connected via pipes */

int
run_it(cmd, to, from, how)
char *cmd;			/* command to run */
int *to, *from;			/* fd's */
int how;			/* if "how" connect stdin of gs to my stdin */
{
	int pid;
	int to_pipe[2];
	int from_pipe[2];

	pipe(to_pipe);
	pipe(from_pipe);

	dprintf(stderr, "Running: [%s]\n", cmd);
	switch (pid = fork()) {
	case -1:		/* error */
		perror("fork");
		break;
	case 0:			/* child */
		close(to_pipe[1]);
		close(from_pipe[0]);
		if (!how) {
			close(0);
			dup(to_pipe[0]);
		}
		close(1);
		dup(from_pipe[1]);
		close(to_pipe[0]);
		close(from_pipe[1]);
		execl("/bin/sh", "sh", "-c", cmd, (char *)0);
		perror("exec");
		_exit(128);
		break;
	default:		/* parent */
		close(to_pipe[0]);
		if (how)
			close(to_pipe[1]);
		close(from_pipe[1]);
		*to = to_pipe[1];
		*from = from_pipe[0];
		break;
	}
	return (pid);
}

/* malloc space for and save a line (no error checking) */

char *
save_line(s)
char *s;
{
	char *malloc(), *strcpy();
	return (strcpy(malloc(strlen(s) + 1), s));
}

/* gs died */

void
child()
{
	int n;
	dprintf(stderr, "child died, waiting\n");
	wait(&n);
	done = 1;
	FD_ZERO(&mask);
	FD_SET(fd, &mask);
}

/* print the title banner */

void
do_title(num, max, s)
int num;			/* page number */
int max;			/* max page number */
char *s;			/* extra stuff */
{
	char title[100];

	sprintf(title, TITLE, name, num, max, s ? s : "");
	m_printstr(title);
	m_cleareos();
	m_flush();
}

/* cleanup and exit */

void
cleanup(msg)
char *msg;			/* message to print on cleanup */
{
	int i;

	kill(pid, SIGINT);
	m_bitdestroy(1);
	m_textreset();
	m_setnoraw();
	m_pop();
	m_ttyreset();
	for (i = 1; i <= pages + 10; i++) {	/* a few extras couldn't hurt */
		dprintf(stderr, "unlinking file %s\n", line);
		sprintf(line, temp_file, i);
		if (unlink(line) != 0 && i <= pages)
			fprintf(stderr, "Warning, can't remove temporary file %s\n", line);
	}
	if ((int) msg > 32)	/* barf! */
		printf("%s\n", msg);
	else
		printf("Exiting on signal %d\n", (int) msg);
	exit(0);
}

/* initialize MGR window state */

void
init_window()
{

	/* window state */

	m_push(P_CURSOR | P_POSITION | P_WINDOW | P_BITMAP | P_EVENT | P_FLAGS | P_MENU | P_TEXT);
	m_clear();
	m_setcursor(CS_INVIS);
	m_setmode(M_ABS);

	/* setup menus */

	menu_load(1, 5, menu);
	m_linkmenu(1, 0, 2, MF_AUTO);
	m_selectmenu(1);
	m_menuitem(1, 1);

	/* get geometries */

	get_font(&fw, &fh);
	fh += BORDER;
	get_size(&wx, &wy, &ww, &wh);
	wh -= fh;
	sx = sy = 0;
	sw = ww;
	sh = wh;

	/* setup events */

	m_setevent(RESHAPE, "S");
	m_setevent(REDRAW, "R");
	m_setevent(BUTTON_1, "B %p!");
	m_setevent(BUTTON_1U, "$");
	m_setevent(BUTTON_2U, "!");

	/* set tite bar */

	m_clear();
	m_setmode(M_ABS);
	m_setmode(M_NOWRAP);
	m_scrollregion(0, 0);
	m_func(B_SET);
	m_bitwrite(0, fh - BORDER + 1, ww, BORDER - 2);
	m_func(B_COPY);
}
