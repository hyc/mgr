/*                        Copyright (c) 1988,1989 Bellcore
 *                            All Rights Reserved
 *       Permission is granted to copy or use this program, EXCEPT that it
 *       may not be sold for profit, the copyright notice must be reproduced
 *       on copies, and credit should be given to Bellcore where it is due.
 *       BELLCORE MAKES NO WARRANTY AND ACCEPTS NO LIABILITY FOR THIS PROGRAM.
 */
/* MTX		functions for debugging only */

#include <ctype.h>
#include "term.h"
#include "mtx.h"

/* debugging stuff */

static char *db_flags = "EGIMOQSWRTL";		/* possible flags */
static char db_list[] = { /* list of active debug flags (db output 1) */
	'E','G','I','M','o','q','s','W','r','t','L',  '\0'
	};

static char db_name[25] = {'\0'};		/* name of debug file (unused) */
static int debug_state = -1;		/* previous debug state (unused) */
static int debug_id = 1;			/* window id for debug2 */


/* debug printout */

#ifndef NODEBUG

Dprintf(level,fmt,a1,a2,a3,a4,a5,a6,a7)
char level;			/* debug level */
char *fmt;			/* format for message */
int a1,a2,a3,a4,a5,a6,a7;	/* args to printf */
	{
	char  *index();

	if (debug && (level=='X' || index(db_list,level))) {
		fprintf(debug,fmt,a1,a2,a3,a4,a5,a6,a7);
		fflush(debug);
		}
/*
	if (debug2 && index(db2_list,level)) {
		fprintf(debug2,fmt,a1,a2,a3,a4,a5,a6,a7);
		fflush(debug2);
		}
*/
	}

#else

Dprintf(level,fmt,a1,a2,a3,a4,a5,a6,a7)
char level;			/* debug level */
char *fmt;			/* format for message */
int a1,a2,a3,a4,a5,a6,a7;	/* args to printf */
	{
	}

#endif
pr_debug(file,object,c)
FILE *file;			/* file to print debugging stuff on */
register struct object *object;		/* window object to debug */
char c;					/* current character */
   {
	if (file && O(id) == debug_id) {
		if (W(state) != debug_state) {
			debug_state = W(state);
			fprintf(file,"\033i%d\033n",debug_state);
			}
		if (c=='\033') { putc('^',file); c = '['; }
		if (c=='\f') { putc('^',file); c = 'L'; }
		putc(c,file);
		fflush(file);
		}
	}

toggle_debug(str)
char *str;		/* debug string (which option) */
	{
	register int i;
	register char *s;
	char *db_get();
	char *index();

	switch (*str) {
		case '+':										/* all strings on */
			strcpy(db_list,db_flags);
			Dprintf('X',"all debugging on\n",str);
			set_dbmenu();
			break;
		case '-':										/* all strings off */
			strcpy(db_list,db_flags);
			for(s=db_list;*s;s++)
				*s = tolower(*s);
			Dprintf('X',"all debugging off\n",str);
			set_dbmenu();
			break;
		case 'f':										/* select new debug file */
			strcpy(db_name,db_get("debug tty?"));
			Dprintf('X',"Switching to %s for debugging\n",db_name);
			if (debug) {
				fclose(debug);
				}
			debug = fopen(db_name,"w");
			if (debug) setlinebuf(debug);
			break;
		default:
			i = index(db_flags,*str) - db_flags;
			if (i<0) {
				Dprintf('E',"Invalid debug option: %c\n",*str);
				break;
				}
			if (isupper(db_list[i])) {
				db_list[i]  = tolower(db_list[i]);
				Dprintf('X',"Turning off %c\n",*str);
				}
			else {
				db_list[i] = toupper(db_list[i]);
				Dprintf('X',"Turning on %c\n",*str);
				}
			set_dbmenu();
			break;
		}
	}

/* set up and download the debug menu */

struct menu_entry debug_menu[] = {
	"  Errors",		"dE\n",
	"  Geometry",	"dG\n",
	"  Input",		"dI\n",
	"  Switching",	"dM\n",
	"  Other",		"dO\n",
	"  Queue",		"dQ\n",
	"  Select",		"dS\n",
	"  Warnings",	"dW\n",
	"  Reads",		"dR\n",
	"  Writes",		"dT\n",
	"  Locks",		"dL\n",
	"ALL ON",		"d+\n",
	"ALL OFF",		"d-\n",
	"debug file",	"df\n",
	};

/* flag the currently selected items on the debug menu, then download it */

set_dbmenu()
	{
	register int i;

	for(i=0;i<strlen(db_list);i++)
		debug_menu[i].value[0] = isupper(db_list[i]) ? '*' : '-';	
	M_pushwin(0,"Reloading debug menu");
	Menu_load(8,MENU_SIZE(debug_menu),debug_menu,dup_key);
	M_popwin();
	}

/* fetch a line from the user from the user */

char *
db_get(prompt)
char *prompt;
	{
	static char prompt_line[60];

	M_pushwin(0,"push prompt");
	m_push(P_EVENT|P_CURSOR);
	fprintf(m_termout,"\f\033i%s\033n:",prompt);
	m_flush();
	m_ttyreset();
	m_gets(prompt_line);
	Dprintf('I',"Got line: %s\n",prompt_line);
	m_ttyset();
	m_pop();
	M_popwin();
	prompt_line[strlen(prompt_line)-1] = '\0';	/* zap trailing '\n' */
	return(prompt_line);
	}

/* debugging version of window selection stacks */

static int win_ids[5];				/* window selection stack */
static int curr_id = 0;				/* current stack entry */
static int _id = -1;					/* current id */

int
M_selectwin(id,note)
int id;				/* new window to select */
char *note;			/* debugging note */
	{
	if (id != _id) {
		Dprintf('M',"--> %d->%d %s\n",_id,id,note);
		m_selectwin(id);
		_id = id;
		}
	/* m_selectwin(id);		/* for debugging only */
	}
	
/* push the window selection stack */
int
M_pushwin(id,note)
int id;			/* new window to switch to */
char *note;		/* debugging note */
	{
	win_ids[curr_id++] = _id;
	M_selectwin(id,note);
	}

/* pop window from the window stack */

int
M_popwin()
	{
	M_selectwin(win_ids[--curr_id],"pop");
	}

/* select previous window */

M_prevwin()
	{
	if (curr_id > 0)
		M_selectwin(win_ids[curr_id-1],"previous window");
	}

/* lock info */

char *locks[] = {
	"Idle",
	"Busy",
	"Proc",
	"Proc, Busy",
	};

char *print_lock(state)
int state;
	{
	return(locks[state&3]);
	}

static char s_buff[2*SH_MAX];
	
char *
safe_print(s,n)
char *s;				/* print s safetly */
int n;
	{
	register char c, *pnt = s_buff;

	while((c = *s++) && n-- > 0) {
		if (c&0x80) {
			*pnt++ = 'M';
			*pnt++ = '-';
			c &= 0x7f;
			}
		if (c < 32) {
			*pnt++ = '^';
			c += '@';
			}
		*pnt++ = c;
		}
	*pnt = '\0';
	return(s_buff);
	}

	
