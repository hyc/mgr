/*                        Copyright (c) 1987 Bellcore
 *                            All Rights Reserved
 *       Permission is granted to copy or use this program, EXCEPT that it
 *       may not be sold for profit, the copyright notice must be reproduced
 *       on copies, and credit should be given to Bellcore where it is due.
 *       BELLCORE MAKES NO WARRANTY AND ACCEPTS NO LIABILITY FOR THIS PROGRAM.
 */

/* check for new mail  (icon version) */

#include <mgr/mgr.h>
#include <sys/stat.h>
#include <sys/time.h>		/* for fsleep */
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <stdio.h>

#define MSG_READING	"\freading mail ...\r"
#define MSG_CHECKING	"\rChecking for new mail..."

#define MAILF		"/var/spool/mail"	/* spool file */
#define MAIL		"mail"			/* name of mail command */
#define POLL		60			/* polling interval */
#define XPOS		240			/* x start of mail window */
#define YPOS		190			/* y start of mail window */
#define W_WIDE		80			/* width of mail window */
#define W_HIGH		24			/* height of mail window */
#define MAX_ICON	64			/* max icon size */

#define PROCESSED	2			/* new mail already processed */

#define S(x)			statb.x
#define Isflag(arg,flag)	(!strncmp(arg,flag,strlen(flag)))
#define Max(x,y)		((x)>(y)?(x):(y))
#define dprintf			if(debug) fprintf

#define fsleep() \
   { \
   struct timeval time; \
   time.tv_sec = 0; \
   time.tv_usec = 330000; \
   select(0,0,0,0,&time); \
   }

#define MENU_COUNT		(sizeof(menu)/sizeof(struct menu_entry))

struct menu_entry menu[] = {
	{"print","t\n"},
	{"delete","dt\n"},
	{"next","n\n"},
	{"quit","q\n"},
	{"help","?\n"},
	{"headers","h *\n"},
	{"abort","x\n"},
};

static struct	stat statb;	/* spool file status */
static char	mail[255];	/* spool file path name */
static long	omtime=0l;	/* previous file mod. time */
static int	state = 0;	/* mail & window state */
static int	poll = POLL;	/* poll interval */
static int	debug=0;	/* for mgrmail -d >& /dev/tty?? */
static int	x,y;		/* window position */
static int	w,h;		/* window size */
static int  cols,rows;	/* font size */
static int	border;		/* size of mgr border */
static int	local=0;	/* use local icon only */
static int	cwide, chigh;	/* width and height of font characters */
static char	*termcap;
static char *title = NULL;		/* mail window title */
static char command[255]  = MAIL;		/* name of readmail command */

struct icon mbox_closed = {"mbox_closed",1,0,0,0};
struct icon mbox_full = {"mbox_full",1,0,0,0};
struct icon mbox_open = {"mbox_open",1,0,0,0};
struct icon mbox_zip = {"mbox_zip",1,0,0,0};

void clean(), update(), download_icon(), usage(), set_icon(), do_menu();
int do_mail();

extern int fprintf( FILE *_stream, const char *_format, ...);
extern int fflush( FILE *_stream);
extern size_t fread( void *_ptr, size_t _size, size_t _nmemb, FILE *_stream);
extern size_t fwrite( const void *_ptr, size_t _size, size_t _nmemb, FILE *_stream);
extern int fclose( FILE *_stream);
extern int system( const char *_string);

void
main(argc,argv)
char **argv;
int argc;
{
	register int i;
	int xpos = XPOS;		/* screen position of mail subwindow */
	int ypos = YPOS;
	int font = -1;			/* font to use for mail subwindow */
	int shape = 1;			/* initially reshape window */

	char *getenv();
	char *user = getenv("USER");
	char line[MAXLINE];		/* event input buffer */
	char *mbox = NULL;		/* mail box to read */

	/* make sure environment is ok */

	ckmgrterm( *argv );

	if (user==NULL || *user=='\0') {
		fprintf(stderr, "%s: No USER environment variable value.\n",
			argv[0]);
		exit(2);
	}

	/* process arguments */

	for(i=1;i<argc;i++) {
		if (Isflag(argv[i],"-s"))
			shape = 0;
		else if (Isflag(argv[i],"-d"))
			debug = 1;
		else if (Isflag(argv[i],"-l"))
			local = 1;
		else if (Isflag(argv[i],"-x"))
			xpos = atoi(argv[i]+2);
		else if (Isflag(argv[i],"-y"))
			ypos = atoi(argv[i]+2);
		else if (Isflag(argv[i],"-f"))
			font = atoi(argv[i]+2);
		else if (Isflag(argv[i],"-p"))
			poll  = Max(atoi(argv[i]+2),10);
		else if (Isflag(argv[i],"-t"))
			title  = argv[i]+2;
		else if (Isflag(argv[i],"-m"))
			mbox  = argv[i]+2;
		else if (Isflag(argv[i],"-M"))
			strcpy(command,argv[i]+2);
		else
			usage(argv[0],argv[i]);
	}
	if (mbox) {
		strcpy(mail,mbox);
		sprintf(command+strlen(command)," -f %s",mbox);
		}
	else
		sprintf(mail,"%s/%s",MAILF,user);

	/* set up window environment */

	m_setup(M_FLUSH);
	m_push(P_CURSOR|P_MENU|P_BITMAP|P_FONT|P_EVENT|P_FLAGS|P_POSITION);
	if (font < 0)
		font = 0;
	m_font(font);
        m_getfontsize( &cwide, &chigh );

	signal(SIGHUP,clean);
	signal(SIGTERM,clean);
	signal(SIGINT,clean);
	signal(SIGALRM,update);

	dprintf(stderr,"pushing environment\n"); fflush(stderr);
	m_ttyset();
	m_setmode(M_NOWRAP);
	m_setmode(M_ABS);
	m_func(BIT_SRC);

	download_icon(&mbox_closed,1);
	download_icon(&mbox_full,2);
	download_icon(&mbox_zip,5);
	download_icon(&mbox_open,6);

        m_getwindowposition(&x,&y);
        m_getwindowsize(&w,&h);
        border=m_getbordersize();
	m_setcursor(CS_INVIS);

	m_setmode(M_ACTIVATE);
	if (shape) {
		int fw, fh=0;		/* font size */
		if (title) {
			m_getfontsize(&fw,&fh);
			}
		m_shapewindow(x,y,2*border+MAX_ICON, fh+2*border+MAX_ICON);
                m_getwindowposition(&x,&y);
                m_getwindowsize(&w,&h);
      }
	if (title) {
		get_colrow(&cols,&rows);
		dprintf(stderr,"setting title\n"); fflush(stderr);
		}

	m_setevent(ACTIVATE,"A\n");
	m_setevent(REDRAW,"R\n");

	m_clearmode(M_ACTIVATE);
	set_icon(mbox_closed,title);

	dprintf(stderr,"Starting state 0x%x\n",state); fflush(stderr);

	update();

	termcap = getenv("TERMCAP");
	if( termcap )
		*termcap = '\0';

	/* wait for an event */

	while(1) {
		if( m_gets(line) == NULL ) {
			clearerr( m_termin );
			continue;
		}
		dprintf(stderr,"state 0x%x line : %c\n",state,*line);
		fflush(stderr);
		switch(*line) {
		case 'A':	/* window is activated */
			if (!stat(mail,&statb) && S(st_size))
				do_mail(command,font,&xpos,&ypos);
			else {
				set_icon(mbox_open,title);
				sleep(2);
				m_clearmode(M_ACTIVATE);
			}
			state &= ~PROCESSED;
			update();
			break;
		case 'R':	/* screen is redrawn */
			state &= ~PROCESSED;
			m_getwindowposition(&x,&y);
                        m_getwindowsize(&w,&h);
			if (title) get_colrow(&cols,&rows);
			update();
			break;
		}
	}
}

/* run readmail in a subwindow */

int 
do_mail(command,font,xp,yp)
char *command;
int font,*xp,*yp;
	{
	char *menu_file;
	int xpos = *xp, ypos = *yp;
	int code;
	int n;
	int x,y;

	alarm(0);
	m_push(P_EVENT | P_FONT);
	dprintf(stderr,"doing mail [%s]\n",command); fflush(stderr);
	n = m_makewindow(xpos, ypos, W_WIDE*cwide + 2*border,
		W_HIGH*chigh + 2*border);
	if (n==0) {	/* can't make window */
		m_printstr("\007\fCan't open mail window, sorry");
		m_pop();
		return(0);
		}
	/*
	if( *termcap  &&  newtermcap[0] == '\0' ) {
		strcpy( newtermcap, get_termcap() );
		termcap = newtermcap;
	}
	*/
  	set_icon(mbox_zip,title);
	m_selectwin(n);
	m_font(font);
	if ((menu_file=getenv("MAIL_MENU")))
		do_menu(menu_file);
	else
		menu_load(1,MENU_COUNT,menu);
	m_selectmenu(1);
	m_printstr(MSG_READING);
	m_ttyreset();
	code = system(command);
	m_printstr(MSG_CHECKING);
	sleep(1);	/* for "New mail arrived" message */
	dprintf(stderr,"Readmail completed code %d\n",code); fflush(stderr);
	m_ttyset();

	/* see if window was moved - remember for next time */
	
        m_getwindowposition(&x,&y);
	if (abs(x-xpos) > 10 || abs(y-ypos) > 10) {
		*xp = x;
		*yp = y;
	}
	m_selectwin(0);
	m_destroywin(n);
	m_pop();
	m_clearmode(M_ACTIVATE);
	dprintf(stderr,"window deactivated\n"); fflush(stderr);
	return 1;
}

/* check the spool file for new mail and update message */

void
update()
{
	alarm(0);
	dprintf(stderr,"checking mail state 0x%x\n",state); fflush(stderr);
	if (!stat(mail,&statb) && S(st_mtime)>S(st_atime) && S(st_size)) {
		state &= ~PROCESSED;
		if (S(st_mtime) != omtime) {
		dprintf(stderr,"	First time New mail\n"); fflush(stderr);
                        m_printstr("");
  			set_icon(mbox_full,title);
			omtime = S(st_mtime);
		}
	}
	else if (!(state&PROCESSED)) {
		dprintf(stderr,"	Clearing new mail\n"); fflush(stderr);
  		set_icon(mbox_closed,title);
		state |= PROCESSED;
	}
	alarm(poll);
}

/*	Clean up and exit */

void
clean(n)
int	n;
{
	m_ttyreset();
	m_selectwin(0);
	m_popall();
	exit(n);
}

void
usage(name,error)
char *name, *error;
{
	fprintf(stderr,"Invalid flag: %s\n",error);
	fprintf(stderr,
		"usage: %s -[s|x<pos>|y<pos>|f<font>|p<poll>|M<mail_program>]\n"
		,name);
	exit(1);
}

/* down load an icon */

void
download_icon(icon,where)
register struct icon *icon;	/* name of icon to download */
int where;			/* bitmap to download icon to */
   {
   int w_in=0, h_in=0, d_in=0;

   if (!local) {
	   /* first try the local machine */
      dprintf(stderr,"looking for %s\n",icon->name);
      m_bitfile(where, icon->name, &w_in, &h_in, &d_in );
      }

   if (h_in==0 || w_in==0) {	/* can't find icon */
      fprintf(stderr,"Couldn't find %s, downloading\n",icon->name);
      exit(1);
      }
   else {
      dprintf(stderr,"Found %s (%d x %d) expected %d x %d\n",
               icon->name,w_in,h_in,icon->w,icon->h);
      icon->w = w_in;
      icon->h = h_in;
      }
   icon->type = where;
} 

void
set_icon(name,title)
struct icon name;		/* name of icon */
char *title;			/* title */
{
   int x0 = (w-name.w)/2;
   int y0 = (h-name.h)/2;

   m_clear();
   m_bitcopyto(x0,y0,name.w,name.h,0,0,0,name.type);
   dprintf(stderr,"copy %s to %d,%d (%d x %d)from %d\n",
           name.name,x0,y0,name.w,name.h,name.type);
	if (title) {
		int col = Max((cols - strlen(title))/2,0);
      m_move(col,rows-1);
      m_printstr(title);
		}
   m_flush();
   }

/* read a menu from this file */

void
do_menu(name)
char *name;
{
	char buff[100];
	FILE *f = fopen(name,"r");
	int count;

	if (f) {
		while ((count = fread(buff,1,100,f)) > 0)
			fwrite(buff,1,count,m_termout);
		fclose(f);
		}
}
