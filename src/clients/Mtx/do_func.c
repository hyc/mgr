/*                        Copyright (c) 1988,1989 Bellcore
 *                            All Rights Reserved
 *       Permission is granted to copy or use this program, EXCEPT that it
 *       may not be sold for profit, the copyright notice must be reproduced
 *       on copies, and credit should be given to Bellcore where it is due.
 *       BELLCORE MAKES NO WARRANTY AND ACCEPTS NO LIABILITY FOR THIS PROGRAM.
 */
/* Handle MTX system requests (from the command menu) */

#include <sys/signal.h>
#include "term.h"
#include "mtx.h"

/* defined in mtx.c */

extern int wx,wy,ww,wh;						/* window coordinates for next window */
extern int font;								/* font for windows */
extern int mask;								/* select mask */
extern int wmask;								/* write select mask */
extern int hmask;								/* select mask for hosts */
extern char *udp_data;						/* pntr to udp socket data */
extern char *tcp_data;						/* pntr to socket tcp data */
extern int state;								/* mgr input state */
extern int conn_id;							/* id of connection edit window */

char name[MAX_NAME];							/* next object name */
char command[256];							/* command + args (temporary) */

/*
 * do system function 'c', using arguments in "line".
 * We assume arguments have been validated by this point, they
 * are not checked.
 */

struct object *
do_func(object,c,line)
register struct object *object;			/* current window object */
int c;											/* character to interpret */
char *line;										/* command arguments */
	{
	register struct object *win;			/* must be a window object */
	register struct object *proc;			/* must be a process object */
	register struct object *host;			/* must be a host object */
	char *in = line;	/* we guarantee 2 chars BEFORE line[0] */
	int x,y,w,h;		/* temporary coordinates */
	int id;				/* temp window id */
	int code;			/* queue code */
	int count;			/* # of args */
	char ch;				/* send witch character */

	switch(c) {
		case ' ':								/* A MGR reply to an info request */
			code = id = dequeue();
			id &= Q_MASK;						/* window id of requestor */
			code &= ~Q_MASK;					/* type of request */

			win=find_byid(D_WINDOW,id);		/* who queued this request */
			Dprintf('O'," Mgr reply for %d type 0x%x\n",id,code);

			if (win && WIN(win,dup_char)) { /* get window && dup_key */
				in -= 2;
				in[0] = WIN(win,dup_char);
				in[1] = ' ';
				}

			clear_lock(L_BUSY);
			switch(code) {
				case Q_SINGLE:					/* single line GETINFO request */
					if (win) {
						to_dest(object,win->dest,in,strlen(in));
						}
					else {
						Dprintf('E',"no window %d to send reply [%.5s...] to\n",in);
						}
					break;
				case Q_MULTI:					/* multi line GETINFO request */
					if (win) {
						to_dest(object,win->dest,in,strlen(in));
						}
					for(id=2;id>1;) {
						fgets(line,sizeof(line),m_termin);
						id=strlen(line);
							to_dest(object,win->dest,line,strlen(line));
						}
					break;
				case Q_ALT:					/* made a window for client, get win # */
					Dprintf('O',"  Alt window for %d is <%s>\n",id,in);
					if (win) {
						register int new_id = atoi(line+2);	/* new window ID */
						Dprintf('O',"  selecting Alt window # %d\n",new_id);
						M_selectwin(new_id,"Alternate client window");
						set_events(id,0);					/* setup window events */
						m_pop();								/* activate events */
						m_setmode(M_STACK);				/* activate event stack */
						m_push(P_EVENT);					/* reset the events */
						to_dest(object,win->dest,in,strlen(in)); /* send window # */
						object = win;						/* new current object */
						}
					else {
						message(MSG_ERROR,"No parent for alternate window",0,0,0);
						}
					break;
				case Q_WIN:					/* got a new window number (window only) */
				case Q_DIALOG:				/* got a new window (for dialog only) */
				case Q_NEW:					/* got a new window number */
					sscanf(in,"%d",&id);
					ch = "nwi"[(code-Q_NEW)>>8];		/* GACK!! */
					sprintf(in,"%c%c%d\n",dup_key,ch,id);
					Dprintf('O',"  New window (type %c) will be %d, sending: %s",
									ch,id,line);
					m_sendme(in);		/* finish making the window */
					set_lock(L_BUSY);	/* we'll need another reply */
					break;
				default:
					Dprintf('O',"  invalid queue request %d.%d\n",code>>8,id);
					break;
				}
			break;

		case 'r':								/* rename an object */
				{
				register struct object *object, *to;
				char from_name[MAX_NAME];
				char to_name[MAX_NAME];	

				code = sscanf(in,"%s %s",from_name,to_name);
				Dprintf('X',"Rename: %s -> %s\n",from_name, to_name);
				to = find_byname(to_name,1);		/* error if found */
				object = find_byname(from_name,0);
				if (code==2 && object && !to) {
					message(MSG_INFO,"%s renamed to %s",O(name),to_name,0);
					free(O(name));
					O(name) = str_save(to_name);
					}
				}
			break;

		case 'q':								/* quit */
			m_clearevent(BUTTON_2);
			clean(0);
			break;

		case 'z':								/* suspend */
			M_pushwin(0,"Suspend");
			m_push(P_FLAGS|P_EVENT|P_POSITION|P_TEXT|P_BITMAP|P_MENU|P_FONT);
			m_size(80,24);
			m_flush();
			m_ttyreset();
			m_printstr("\fMTX suspended\n");
			m_setcursor(0);	
			sig_proc(SIGSTOP);				/* stop all process groups */
			Dprintf('O',"Sending self STOP\n");
			kill(getpid(),SIGTSTP);        /* send stop signal to self */
			sleep(1);					/* wait for signal */
			Dprintf('O',"Got start\n");
			m_pop();
			m_ttyset();
			m_setraw();
			m_func(B_SRC);
			m_bitcopyto(0,0,999,999,0,0,0,1);	/* re-draw windows */
			sig_proc(SIGCONT);				/* restart all process groups */
			M_popwin();
			m_setcursor(CS_INVIS);	
			break;

		case 'S':								/* reshape main window (broken) */
			M_pushwin(0,"Push for redraw");
			do_shape(0,0);
			m_func(B_SRC);
			m_bitcopyto(0,0,999,999,0,0,0,1);	/* re-draw windows */
			M_popwin();
			break;

		case 'R':								/* redraw main window */
			M_pushwin(0,"Push for redraw");
			m_func(B_SRC);
			m_bitcopyto(0,0,999,999,0,0,0,1);	/* re-draw windows */
			setup_menus();								/* re-create menus */
			M_popwin();
			break;

		case 'B':				/* process button 1 hit (stub for now) */
			{
			int from, id, to;
			extern int my_mgrid;
			sscanf(in,"%d %d.%d",&from,&id,&to);
			Dprintf('O',"  got Button 1: %s",in);
			if (id == my_mgrid) {
				struct object *foo;
				foo = find_byid(D_WINDOW,to);
				message(MSG_INFO,"ID: %s (%d)",foo?foo->name:"??",to,0);
				}
			}
			break;

		case 'M':				/* print last few messages in most recent window */
			print_msgs();
			break;

		case 'A':								/* activate a window */
			sscanf(in,"%d",&x);
			if (x==0) {		/* main window active */
				start_command();
				}
			else if (object = find_byid(D_WINDOW,x))  {
				message(MSG_INFO,"Input focus: Window %s",O(name),0,0);
				}
			else {
				Dprintf('E',"  got ACTIVATE %d: no such window\n",x);
				}
			break;

		case 'N':								/* make a window  with process */
			start_window(wx,wy,ww,wh,Q_NEW);
			wx+=10,wy+=20;						/* offset next window */
			break;

		case 'W':								/* make a window (window only) */
			count = sscanf(line,"%s %d %d %d %d",name,&x,&y,&w,&h);
			Dprintf('X',"W: sscanf got %d (%s)",count,line);
			switch(count) {	/* no breaks on this one */
				default:
					*name = '\0';		/* no name given, use the default? */
				case 1:
					x = wx;
					wx += 20;
				case 2:
					y = wy;
					wy += 10;
				case 3:
					h = wh;
				case 4:
					w = ww;
				case 5:
					break;
				}
			start_window(x,y,w,h,Q_WIN);
			break;

		case 'I':								/* make a dialog window  */
			start_window(wx,wy,60,60,Q_DIALOG);
			break;

		case 'i':								/* finish making a window (dialog) */
			clear_lock(L_BUSY);
			sscanf(in,"%d",&x);
			if (x>0) {
				state = S_CONN;
				conn_id=x;			/* we need to remember the window id for later */
				M_selectwin(x,"Dialog window");
				start_con(font);
				}
			else {
				message(MSG_ERROR,"Uh-Oh!, Can't make dialog window",0,0,0);
				}
			break;

		case 'n':								/* finish making a window (std) */
		case 'w':								/* finish making a window (no proc) */
			clear_lock(L_BUSY);
			sscanf(in,"%d",&x);
			if (x == 0) {						/* MGR didn't make the window */
				message(MSG_ERROR,"Sorry, MGR couldn't make the window",0,0);
				break;
				}
			M_selectwin(x,"New user window");
			Dprintf('O',"  resume making window  %c %d\n",c,x);
			m_font(font);
			object = setup_window(x,*name?name:NULL);
			*name = '\0';

			if (c=='n') {						/* make default connections */
				if ((proc = setup_proc()) == NULL) {
					message(MSG_ERROR,"Can't make a process, sorry",0,0);
					break;
					}
				message(MSG_INFO,"Making new window %d, process %s",
						x,proc->class.proc->tty,0);
				mask |= BIT(proc->class.proc->fd);
				wmask |= BIT(proc->class.proc->fd);
				set_fdmask(object,proc);
				Dprintf('S',"  Make window/process, mask: 0x%x -> 0x%x\n",
							BIT(proc->class.proc->fd),mask);

				/* make pipe connections */

				O(dest) = get_dest(O(dest),proc);
				proc->dest = get_dest(proc->dest,object);
				}
			else {
				message(MSG_WARN,"Making new window %d NO PROCESS",x,0,0);
				}

			m_pop();								/* activate events */
			m_setmode(M_STACK);				/* activate event stack */
			m_push(P_EVENT);					/* reset the events */
			break;

		case 'C':								/* make a connection */
				{
				struct object *from, *to;
				char from_name[MAX_NAME];		/* name of source object */
				char to_name[MAX_NAME];			/* name of dest object */
				char host_name[MAX_NAME];		/* name to send to remote host */

				code = sscanf(in,"%s %s %s",from_name,to_name,host_name);
				to = find_byname(to_name,0);
				from = find_byname(from_name,0);
				Dprintf('O',"Connecting %s to %s\n",
						from?from->name:"???",to?to->name:"???");
				if (code>=2 && from && to) {
					if (to->type == D_HOST && to->class.host->state != H_CONN) 
						start_host(to,code>2?host_name:NULL);	
					if (from->type == D_HOST && from->class.host->state != H_CONN) 
						start_host(from,NULL);	
					from->dest = get_dest(from->dest,to);
					wmask |= set_fdmask(from,to);
					message(MSG_INFO,"Connection added %s->%s",
								from->name,to->name,0);
					}
				else
					message(MSG_ERROR,"Invalid connection request %s->%s",
								from_name,to_name);
				}
			break;

		case 'c':								/* break a connection */
				{
				struct object *from, *to;
				char from_name[MAX_NAME];
				char to_name[MAX_NAME];	

				code = sscanf(in,"%s %s",from_name,to_name);
				Dprintf('X',"disconnect: %s -> %s\n",from_name, to_name);
				to = find_byname(to_name,0);
				from = find_byname(from_name,0);
				if (code==2 && from && to && free_dest(from,to)) {
					message(MSG_INFO,"Connection deleted: %s -> %s",
								from->name,to_name,0);
					clear_fdmask(from,to);
					}
				else
					message(MSG_ERROR,"%s !=> %s",from_name,to_name,0);
				}
			break;

		case 'K':								/* specify shell (temporary) */
			strcpy(command,line);
			Dprintf('X',"Got cmd: %s\n",command);
			break;
		
		case '+':								/* specify colors */
			change_colors(line);
			Dprintf('X',"Got new colors: %s\n",line);
			/* should send a re-draw here */
			break;
		
		case 'P':								/* make a process (only) */
				{
				int pid, fd1,fd2;
				int flags;						
				char proc1_name[MAX_NAME];
				char proc2_name[MAX_NAME];
				char *cmd = *command ? command : NULL;
				
				Dprintf('X',"Using cmd: %s\n",cmd ? cmd : "NONE");
				code = sscanf(in,"%s %s %d",proc1_name,proc2_name,&flags);
				Dprintf('X',"Got proc name:  (%d) %s,%s\n",code,
						code>0?proc1_name:"NONE",code>1?proc2_name:"NONE");
				switch(code) {
					case -1:		/* why does this happen? */
					case 0:		/* 1 connection, default name  */
					case 1:		/* 1 connection */
						pid = get_shell(cmd,&fd1,NULL,0);
						proc = create_proc(fd1,pid,'c',code ? proc1_name : NULL);
						mask |= BIT(fd1);
						message(MSG_INFO,"Making process %s",proc->name,0,0);
						break;
					case 2:		/* 2 connections */
						flags = 0;
						/* no break */
					case 3:		/* 2 connections  with flags */
						pid = get_shell(cmd,&fd1,&fd2,flags);
						proc = create_proc(fd1,pid,'p',proc1_name);
						proc = create_proc(fd2,pid,'s',proc2_name);
						mask |= BIT(fd1) | BIT(fd2);
						message(MSG_INFO,"Making process %s,%s",
									proc1_name,proc2_name,0);
						break;
					}
			}
			*command = '\0';
			break;

		case 'L':		/* make a process, 2 objects (temporary) */
			{
			int fd1, fd2, pid;

			Dprintf('X',"Creating shell\n");
			pid = get_shell(NULL,&fd1,&fd2,0);
			Dprintf('X',"Got shell %d, fds %d and %d\n",pid,fd1,fd2);
			mask |= BIT(fd1) | BIT(fd2);
			message(MSG_INFO,"Making process %d fd %d and %d",pid,fd1,fd2);
			proc = create_proc(fd1,pid,'p',NULL);
			proc = create_proc(fd2,pid,'s',NULL);
			}
			break;

		case 'h':								/* retransmit broadcast packet */
			udp_send(udp_data,M_HI);
			message(MSG_WARN,"resending greeting message",0,0,0);
			break;

		case 'H':								/* make a host channel */
			sscanf(in,"%d",&x);
			host = find_byid(D_HOST,x);
			if (host && host->class.host->state == H_CONN) 
				message(MSG_WARN,"Host is already connected",0,0);
			else if (!host)
				message(MSG_ERROR,"Oops, can't find host to connect",0,0);
			else {
				start_host(host,NULL);	
				}
			break;

		case 'X':			/* window destroyed via system menu (DESTROY event) */
			sscanf(in,"%d",&x);
			message(MSG_WARN,"Window %d destroyed by user",x,0,0);
			if (win = find_byid(D_WINDOW,x)) {
				purge_obj(win);
				free_obj(win);
				object = NULL;
				}
			else {
				Dprintf('E',"   cant find window %d to kill\n",x);
				}
			break;

		case 'D':								/* Deactivate a window */
			sscanf(in,"%d",&x);
			if (x==0) {		/* main window active */
				end_command();
				}
			else if (object=find_byid(D_WINDOW,x)) {
				object=NULL;
				}
			else {
				Dprintf('E',"Deactive: NO WINDOW!! %d\n",x);
				}
			break;

		case 'd':								/* debugging info */
			toggle_debug(in);
			break;

		case '#':								/* ignore */
			break;

		default:
			Dprintf('E',"Invalid command: %c (%.3o)",c >= ' ' ? c : '?' ,c);
			message(MSG_WARN,"Invalid command: %s",in,0,0);
			break;
		}
	return(object);
	}

/* start a host connection */

start_host(host,name)
char *name;		/* name to send to remote host */
register struct object *host;			/* must be a host object */
	{
	register int x = 0;

	Dprintf('O',"Connecting to host %d (%s)\n", host->id,host->class.host->host);
	if ((x = connect_host(host,name)) > 0) {
		mask |= BIT(x);
		/* wmask |= BIT(x);  no connection on channel */
		hmask |= BIT(x);
		}
	else {						/* host must not be "real" */
		menu_dlthost(host);
		kill_host(host);		/* better remove host link */
		}
	return(x);
	}
