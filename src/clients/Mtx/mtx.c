/*                        Copyright (c) 1988,1989 Bellcore
 *                            All Rights Reserved
 *       Permission is granted to copy or use this program, EXCEPT that it
 *       may not be sold for profit, the copyright notice must be reproduced
 *       on copies, and credit should be given to Bellcore where it is due.
 *       BELLCORE MAKES NO WARRANTY AND ACCEPTS NO LIABILITY FOR THIS PROGRAM.
 */
/* this is the MGR mtx version II (S. A. Uhler) */

#include "term.h"
#include "mtx.h"
#include "color.h"
#include <signal.h>
#include <sys/time.h>
#include <errno.h>

#define P_SAVE \
	P_EVENT|P_CURSOR|P_TEXT|P_MENU|P_FLAGS|P_POSITION 	/* to reset when we die */

struct menu_entry system_menu[] = {		/* main window menu */
	"Make a new window =>", "N\n",		/* entries must start with dup_key */
	"Change connections ...", "I\n",
	"connect to host=>",	"",
	"Print previous messages",		"M\n",
	"Debugging stuff =>","",
	"suspend",	"z\n",
	"quit",	"q\n",
   };

struct menu_entry new_menu[] = {
	"Just a window", "W\n", 
	"Just a shell", "P\n",
	"A shell with 2 connections", "L\n",
	};

struct menu_entry host_menu[MAX_HOSTS+1] = {
	"Other host connections", "h\n",
	};

struct menu_entry conn_menu[] = {
	"THIS IS AN INACTIVE MENU", "",
	"Print Connections", "I\n",
	"Delete a connection =>", "",
	"Add a connection =>",    "",
	"Kill a process =>",      "",
	"Kill a window =>",      "",
	};

int wx,wy,ww,wh;						/* window coordinates for next window */
int border;								/* window border size */
int font;								/* font for windows */
char line[2048];						/* space for longest mgr input */
char title[80];						/* space to store title */
int mask = 0;							/* read select mask */
int wmask = 0;							/* write select mask */
int hmask = 0;							/* select mask for hosts */
char *udp_data;						/* pntr to udp socket data */
char *tcp_data;						/* pntr to socket tcp data */
int state = S_NORMAL;				/* mgr input state */
int conn_id;							/* id of connection edit window */
int my_mgrid;							/* my mgr id */
char dup_key = DUP_KEY;				/* MGR dup key */

FILE *debug = NULL;					/* for debugging output */
FILE *debug2 = NULL;					/* for debugging output */
struct object *object_list=NULL;	/* head of object list */
struct timeval time = {0,0};		/* to poll on select */
int object_count = 0;				/* # of objects */

extern int errno;			/* system error message */

/***************************************************************************
 * MAIN
 */

main(argc,argv)
int argc;
char *argv[];
   {
	int fd;										/* temporary fd */
	int count;									/* # of bytes read from a process */
	char ch;										/* current character */
	char dup_char='\0';						/* current dup char (if any) */
	int reads;									/* read on these fd's (for select) */
	int writes;									/* fd's ok for write for select */
	register struct object *object;		/* current object */
	register struct object *active=NULL;	/* active window */
	char *interface = argc>1 ? argv[1] : NULL;	/* host to identify interface */
	char *udp_setup(), *getenv();
	char *get_tcp(), *my_name();

	debug = fopen(getenv("DEBUG"),"w");
	if (debug)	
		setlinebuf(debug);

	if (getenv("DUP_KEY"))
		dup_key = *getenv("DUP_KEY");
   save_modes(0);								/* save tty modes for shells */
	tcp_data = get_tcp();					/* look for TCP connection */
	udp_data = udp_setup(interface,get_tcpport(tcp_data));
	sprintf(title,"%s (%s)",TITLE,my_name());
	setup_display();							/* initialize the environment */
	if (argc>2)
		change_colors(argv[2]);

	setup_main();								/* setup main (control) window */


	mask = BIT(fileno(m_termin));			/* always look for mgr/kbd input */
	mask |= BIT(udp_fd(udp_data));		/* always look for broadcast stuff */
	mask |= BIT(get_tcpfd(tcp_data));	/* always listen for TCP connection */
	wmask = BIT(fileno(m_termout));		/* block on termout writes? */

	Dprintf('S',"select mask: 0x%x | 0x%x | 0x%x = 0x%x\n",
		fileno(m_termin),udp_fd(udp_data), get_tcpfd(tcp_data), mask);
	
	/* tell world we're here */

	if (udp_send(udp_data,M_HI))
		message(MSG_INFO,"Turning on network interface",0,0,0);
	else
		message(MSG_WARN,"Network interface is unavailable",0,0,0);

	/* run startup file */

	open_cmdfile(START);
	
	/* process input */

	while(1) {
		fflush(m_termout);

		/* process command file input (if any) */

		while (get_lock()==L_PROC) {	/* have command but not busy */
			Dprintf('X',"Processing command file line\n");
			if (get_code()) {		/* terminate command file */
				close_cmdfile();
				clear_lock(L_PROC);
				message(MSG_WARN,"Command file terminated do to error");
				}
			if (ch = get_cmdline(line+2))	{			/* leave room for DUPKEY */
				Dprintf('I',"\r\n%d\033i>\033n%c:%s\n",
										strlen(line+4),ch,line+4);
				active = do_func(active,ch,line+4);		/* skip ch and ':' */
				}
			else {
				clear_lock(L_PROC);
				}
			}

		reads = mask;
		while (select(32,&reads,NULL,NULL,NULL) < 0) {		/* wait for input */
			Dprintf('E',"Select read error 0x%x->0x%x\n",mask,reads);
			reads = mask&~hmask;		/* turn off hosts? */
			}
		Dprintf('S',"select (R) 0x%x->0x%x ",mask,reads);

		writes = wmask;
		while (select(32,NULL,&writes,NULL,&time) < 0) {	/* check for blocked output */
			Dprintf('E',"Select write error (%s) 0x%x->0x%x\n",
					strerror(errno),wmask,writes);
			writes = wmask;
			break;		/* TEMPORARY */
			}
		Dprintf('S',"(W) 0x%x->0x%x\n",wmask,writes);

		/* look for broadcast input */

		if (reads & BIT(udp_fd(udp_data))) {
			udp_get(udp_data,line,sizeof(line));
			if ((fd = process_broadcast(udp_data,line)) >0) {
				Dprintf('E',"Connection died: mask: %x - %x = ",mask, BIT(fd));
				mask &= ~(BIT(fd));
				hmask &= ~(BIT(fd));
				wmask &= ~(BIT(fd));
				reset_fdmask(wmask);
				Dprintf('S',"%x\n",mask, BIT(fd));
				continue;
				}
			else
				Dprintf('W',"disposed of harmless broadcast packet\n");
			}

		/* look for a pending tcp connection */

		if (reads & BIT(get_tcpfd(tcp_data))) {
			fd = get_connect(tcp_data);
			mask |= BIT(fd);
			/* wmask |= BIT(fd); */
			}

		/* handle kbd/mgr input */

		if (reads & BIT(fileno(m_termin))) {		/*  have kbd input */
			switch(state) {
				case S_NORMAL:								/* normal processing */
					ch=getc(m_termin);
					if (ch==dup_key) {					/* start escape processing */
						ch = getc(m_termin);
						if (ch == dup_key) { 				/* dup_key from kbd */
							to_dest(NULL,active->dest,&ch,1);
							if (dup_char && ch==dup_char) {
								to_dest(NULL,active->dest,&ch,1);
								Dprintf('O',"Duplicating %d\n",dup_char);
								}
							}
						else {		/* interpret the MTX command */
							fgets(line+2,sizeof(line)-2,m_termin);
							Dprintf('I',"\r\n%d\033i:\033n%c%s",
										strlen(line+2),ch,line+2);
							active = do_func(active,ch,line+2);	
							}
						}
					else if (active) {					/* ordinary character */
						dup_char = active->class.window->dup_char;
						to_dest(NULL,active->dest,&ch,1);
						if (dup_char && ch==dup_char) {
							to_dest(NULL,active->dest,&ch,1);
							Dprintf('O',"Duplicating %d\n",dup_char);
							}
						}
					else {				/* typed at control window */
						cmd_char(ch);
						}
					break;
				case S_CONN:								/* connection window */
					fgets(line,sizeof(line),m_termin);
					M_pushwin(conn_id,"push conn_id");
					if (conn_cmd(wx,wy,line)) {
						state = S_NORMAL;
						free_conmenus();
						M_popwin();
						m_destroywin(conn_id);
						}
					else
						M_popwin();
					break;
				}
			continue;		/* select crit. may have changed */
			}

		/* process input from shells */

		for(object=object_list;object;object=O(next)) {
			if (O(type)==D_PROC && reads&BIT(P(fd)) && O(dest)) {
/*
				if (~writes & O(dest_mask))  {
					Dprintf('S',"Skipping output to %s, would block on 0x%x\n",
							O(name),~writes&O(dest_mask));
				} else 
*/
					{
					count=Read(P(fd),line,SH_MAX);	/* need to catch io errors here */
					if (count<0) {
						mask &= ~BIT(P(fd));
						Dprintf('E',"I/O error reading fd %d\n",P(fd));
						}
					else
						to_dest(object,O(dest),line,count);
					}
				}
			else if (O(type)==D_HOST && H(state)==H_CONN && reads&BIT(H(fd))) {
				count=read(H(fd),line,SH_MAX);
				to_dest(object,O(dest),line,count);
				}
			}
		}
   }

/***************************************************************************
 * send the escape sequence to the shell
 */

int
do_esc(object,c)
register struct object *object;			/* must be a window object */
register char c;
	{
	fwrite(W(esc),1,W(esc_count),m_termout);
	putc(c,m_termout);
	return(W(esc_count) + 1);
	}

/***************************************************************************
 * get the last escape argument
 */

int
last_arg(object)
register struct object *object;			/* must be a window object */
	{
	register char *cp, *delim, c;

	W(esc)[W(esc_count)] = '\0';
	for(delim=cp=W(esc)+1;c = *cp;cp++)
		if (c==E_SEP1 || c==E_SEP2)
			delim = cp, delim++;
	return(atoi(delim));
	}

/***************************************************************************
 * setup main window
 */

int
setup_main()
	{
	int buff[10];
	int fw,fh;	/* font size */

	m_push(P_SAVE);							/* to reset when we die */
   m_setmode(M_NOWRAP);						/* so too many conns won't scroll */
	m_setmode(M_ABS);							/* for mouse position */
	m_setcursor(CS_INVIS);					/* turn off cursor */

	get_size(&wx,&wy,&ww,&wh);
	Dprintf('G',"got size %d,%d %d x %d \n",wx,wy,ww,wh);
	get_param(0,0,0,&border);
	Dprintf('G',"got border %d\n",border);
	font = get_font(&fw,&fh);
	Dprintf('G',"got font %d x %d \n",fw,fh);
	wh += 2*border;
	ww += 2*border;
	my_mgrid = get_mgrid(wx+border,wy+border);
	Dprintf('G',"got MGR id %d\n",my_mgrid);

	Dprintf('O',"drawing main window\n");
	draw_main(title,wx,wy,fw,fh,border,1);

	/* save window image */

	m_func(B_SRC);
	m_bitcopyto(0,0,999,999,0,0,1,0);

	setup_menus();
	m_dupkey(dup_key);
	Dprintf('O',"done with menus, setting dupkey, events\n");

	sprintf(buff,"%cR\n",dup_key);
	m_setevent(REDRAW,buff);
	sprintf(buff,"%cS\n",dup_key);
	m_setevent(RESHAPE,buff);
	sprintf(buff,"%cA0\n",dup_key);
	m_setevent(ACTIVATE,buff);
	sprintf(buff,"%cD0\n",dup_key);
	m_setevent(DEACTIVATE,buff);
	return(1);
	}

/***************************************************************************
 * draw main window
 */

static int mw_h;		/* main window location (saved for reshape) */
static int mw_w;

int
draw_main(title,wx,wy,fw,fh,border,shape)
char *title;
int wx,wy,fw,fh,border;
int shape;							/* 1 for reshape */
	{
	int mw_x;
	int mw_y;

	mw_h = 2 * (fh+border+3);
	mw_w = fw*MAIN_WIDE + 2*border + 6;
	mw_x = wx>border ? wx-border : 0;
	mw_y = wy>mw_h+border ? wy-mw_h-border : 0;

	set_color(MAIN_COLOR,MAIN_BCOLOR);
	if (shape)
		do_shape(mw_x,mw_y);
	m_textreset();
	m_clear();
	m_move((MAIN_WIDE-strlen(title))/2,0);
	fprintf(m_termout,"%s",title);
	m_textregion(3,fh+3,fw*MAIN_WIDE,fh);
	Dprintf('G',"got text region %d,%d  %d x %d\n",3,fh+3,fw*MAIN_WIDE,fh);
	m_move(0,0);

	m_func(B_SET);
	m_go(1,fh+1);
	Dprintf('G',"lines  %d,%d -> %d,%d ->%d,%d ->%d,%d -> %d,%d\n",
		1,fh+1,
		mw_w-2*border-3,fh+1,
		mw_w-2*border-3,2*fh+4,
		1,2*fh+4,
		1,fh+1);
		
	m_draw(mw_w-2*border-3,fh+1);
	m_draw(mw_w-2*border-3,2*fh+4);
	m_draw(1,2*fh+4);
	m_draw(1,fh+1);
	}

int
do_shape(x,y)
int x,y;		/* position of main window */
	{
	static int my_x, my_y;				/* previous shaped values */
	if (x==0) x = my_x; else my_x = x;
	if (y==0) y = my_y; else my_y = y;
	m_shapewindow(x,y,mw_w,mw_h);
	}

/***************************************************************************
 * setup the display
 */

int
setup_display()
	{
   m_setup(0);
	m_menuchar = '/';
	setbuf(m_termin,NULL);	/* stdio buffering and select() don't mix */
   m_ttyset();
	init_color();
	m_setraw();

   signal(SIGINT,clean);
   signal(SIGCHLD,child);
   signal(SIGSEGV,clean);
   signal(SIGTERM,clean);
	}

/***************************************************************************
 * setup the window state
 */

struct object *
setup_window(id,name)
int id;		/* window id */
char *name;	/* name of window */
	{
	register struct window_state *win;
	register struct object *object;
	char *malloc();
	char *str_save(), *str_gen();

   if ((win=(struct window_state *)
				malloc(sizeof(struct window_state))) == NULL) 
      return(NULL);

   if ((object=(struct object *) malloc(sizeof(struct object))) == NULL) {
		free(win);
      return(NULL);
		}

	set_events(id,1);

	O(id) = id;
	O(type) = D_WINDOW;
	O(dest) = NULL;
	O(ref) = 0;
	O(dest_mask) = 0;
	O(class.window) = win;
	W(state) = W_NORMAL;
	W(push_count)= 0;
	W(dup_char) = '\0';
	W(alt_id) = id;
	if (name)
		O(name) = str_save(name);
	else
		O(name) = str_gen("W(%d)",id,0,0);

	/* link into list */

	O(next) = object_list;
	object_list = object;
	object_count++;
	
	return(object);
	}
	
/***************************************************************************

/* setup the events needed by MTX to manage a window */

set_events(id,type)
int id;					/* window id (for MTX) */
int type;				/* 1->main window, 0->alternate window */
	{
	char buff[10];

	Dprintf('O',"  setting up window events %d\n",id);
   m_push(P_EVENT);

	sprintf(buff,"%cA%d\n",dup_key,id);
	m_setevent(ACTIVATE,buff);
	sprintf(buff,"%cD%d\n",dup_key,id);
	m_setevent(DEACTIVATE,buff);
	sprintf(buff,"%cB%d %%w\n",dup_key,id);
	m_setevent(BUTTON_1,buff);
	if (type == 1) {						/* only for main window (for now) */
		sprintf(buff,"%cX%d\n",dup_key,id);
		m_setevent(DESTROY,buff);
		}
	m_dupkey(dup_key);
	m_push(P_EVENT);		/* stack the events */
	}

/***************************************************************************
 * restore window state and exit
 */

int
clean(n)
int n;
   {
	register struct object *object;
	
	/* tell world we're gone */

	udp_send(udp_data,M_BYE);

	/* kill all of the windows */

   signal(SIGCHLD,SIG_IGN);
	for(object=object_list;object;object=O(next))  {	/* kill objects */
		switch (O(type)) {
			case D_WINDOW:
				M_selectwin(O(id),"clean");
				m_pop(); m_pop(); m_pop();	/* should be enough */
				M_selectwin(0,"clean");
				m_destroywin(O(id));
				break;
			case D_PROC:
				if (geteuid() < 1) {
					fchmod(P(fd),0666);
					fchown(P(fd),0,0);
					}
				close(P(fd));
				killpg(P(pid),SIGHUP);
				break;
			case D_HOST:
				close(H(fd));
				break;
			}
		}

   m_pop(); m_pop(); m_pop();	/* should be enough */
	m_clearmode(M_DUPKEY);
   m_ttyreset();
	m_printstr("\fThe end\n");
	sleep(1);
	udp_send(udp_data,M_BYE);		/* try again for good measure */
	if (n==SIGSEGV)		/* drop a core */
		abort(n);
	m_setcursor(0);		/* get around an old MGR bug */
   exit(n);
   }

/*************************************************************************
 *	catch dead children 
 */

child(sig)
int sig;
   {
	register struct object *object;
	long status;
	int pid;

	Dprintf('S',"  SIGCHLD (%d): waiting...",sig);
	pid = wait(&status);
	Dprintf('S',"for %d\n",pid);
	for(object=object_list;object;object=O(next)) 
      if (O(type)==D_PROC && P(pid)==pid) {		/* kill the process */
			int bit;
			message(MSG_WARN,"Process %s died",O(name));
			bit = BIT(kill_proc(object));
			mask &= ~bit;
			wmask &= ~bit;
         }
	reset_fdmask(wmask);
   }

/*************************************************************************
 * start making a new window */

int
start_window(x,y,w,h,type)
int x,y,w,h;		/* window position */
int type;			/* window type (put on reply Q) */
	{
	m_newwin(x,y,w,h);
	m_flush();
	Dprintf('O',"  asked for window %d %d, %dx%d type 0x%x\n",x,y,w,h,type);
	enqueue(type,"MTX start window");
	set_lock(L_BUSY);
	}

/* debugging read */

int
Read(fd,buff,count)
int fd;
char *buff;
int count;
	{
	char *safe_print();
	Dprintf('R',"   R %d: (%d)..",fd,count);
	count = read(fd,buff,count);
	Dprintf('R',"(%d) [%s]\n",count,safe_print(buff,count));
	return(count);
	}

/* setup main menus */

setup_menus()
	{
	set_color(MENU1_COLOR,MENU_BCOLOR);
	Menu_load(MAIN_MENU,MENU_SIZE(system_menu),system_menu,dup_key);
	set_color(MENU2_COLOR,MENU_BCOLOR);
	Menu_load(HOST_MENU,1,host_menu,dup_key);
	Menu_load(NEW_MENU,MENU_SIZE(new_menu),new_menu,dup_key);
	Menu_load(CONN_MENU,MENU_SIZE(conn_menu),conn_menu,dup_key);
	set_dbmenu();		/* download debugging menu */

	m_linkmenu(MAIN_MENU,4,8,MF_SNIP|MF_AUTO);
	m_linkmenu(MAIN_MENU,2,HOST_MENU,MF_SNIP);
	m_linkmenu(MAIN_MENU,0,NEW_MENU,MF_SNIP);

	m_selectmenu(MAIN_MENU);
	}

/* get mgr id # */

int
get_mgrid(x,y)
int x,y;				/* get mgrid of window at x,y */
	{
	int id;

	m_whatsat(x,y);
	m_gets(line);
	if (sscanf(line,"%*s %*s %*d %d",&id) == 1)
		return(id);
	else
		return(-1);
	}

/******************************************************************************
 *
 *	down load a menu, if dup>0 prefix it to action, iff action>0
 */

Menu_load(n,count,text,dup)
int n;				/* menu number */
int count;			/* number of menu items */
struct menu_entry *text;	/* menu choices */
char dup;				/* dup key prefix (if any) */
   {
   register int i,j, len;

   if (text == (struct menu_entry *) 0)
      return (-1);

   /* calculate string lengths */

   len = 2 * count + 1;

   for (i=0;i<count;i++) {
		j = strlen(text[i].action);
		len += strlen(text[i].value) + (dup&&j?j+1:j);
		}
   
   fprintf(m_termout,"%c%d,%d%c%c",m_escchar,n,len,E_MENU,m_menuchar);

   for (i=0;i<count;i++)
      fprintf(m_termout,"%s%c",text[i].value,m_menuchar);

   for (i=0;i<count;i++)
		if (dup && strlen(text[i].action))
			fprintf(m_termout,"%c%s%c",dup,text[i].action,m_menuchar);
		else
			fprintf(m_termout,"%s%c",text[i].action,m_menuchar);
	}
