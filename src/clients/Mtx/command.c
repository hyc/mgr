/*                        Copyright (c) 1988,1989 Bellcore
 *                            All Rights Reserved
 *       Permission is granted to copy or use this program, EXCEPT that it
 *       may not be sold for profit, the copyright notice must be reproduced
 *       on copies, and credit should be given to Bellcore where it is due.
 *       BELLCORE MAKES NO WARRANTY AND ACCEPTS NO LIABILITY FOR THIS PROGRAM.
 */
/* this is the MGR mtx version II (S. A. Uhler)  command stuff */

#include "term.h"
#include "mtx.h"
#include "color.h"

/* fetch a character in the main window */

static char command[60];			/* place to hold a command */
static char arg[30];					/* a command arg - 1 for now */
static int cmd_next = 0;			/* place for next char */

int
cmd_char(ch)
char ch;
	{
	char *index();

	if (cmd_next == 0) {
		set_color(MAIN_COLOR,MAIN_BCOLOR);
		M_selectwin(0,"Get command");
		m_printstr("\f>");
		m_setcursor(0);						/* turn on cursor */
		cmd_next = 0;
		bzero(command,60);
		set_color(COMMAND_COLOR,MAIN_BCOLOR);
		}
	switch(ch) {
		case '\b':								/* back space */
			if (cmd_next>0) {
				m_printstr("\b \b");
				command[--cmd_next] = '\0';
				}
			break;
		case 'u'&0x1f:							/* erase line */
			m_printstr("\f>");
			cmd_next = 0;
			bzero(command,60);
			break;
		case '\r':								/* do the command  this is temporary */
		case '\n':
			clear_message();
			if (index("KWPCcr+",*command) && command[1]==':') {
				Dprintf('O',"Got command-window command\n");
				do_func(NULL,*command,command+2);
				}
			else if (open_cmdfile(command)) {
				message(MSG_INFO,"Running command file %s",command,0);
				}
			else
				message(MSG_ERROR,"Invalid command file %s",command,0);
			cmd_next = 0;
			break;
		default:
			command[cmd_next++] = ch;
			putc(ch,m_termout);
			break;
		}
	m_flush();
	}

/* initialize the command window */

int
start_command()
	{
	Dprintf('O',"Starting command-window command\n");
	cmd_next = 0;
	return(0);
	}

int
end_command()
	{
	Dprintf('O',"Ending command-window command\n");
	M_pushwin(0,"end command");
	m_setcursor(CS_INVIS);					/* turn off cursor */
	M_popwin();
	clear_message();
	}	

/*************************************************************/
/* manage procedure locks */

static int lock = L_IDLE;							/* mtx state */
char *print_lock();

/* set a lock */

int
set_lock(state)
int state;
	{
	Dprintf('L',"Lock set %s => %s\n",print_lock(state),print_lock(lock|state));
	return (lock |= state);
	}

/* clear a lock */

int
clear_lock(state)
int state;
	{
	Dprintf('L',"Lock Clear %s => %s\n",
				print_lock(state),print_lock(lock&=~state));
	return(lock &= ~ state);
	}

/* get a lock */

int get_lock()
	{
	return(lock);
	}

/*************************************************************/
/* preliminary file manipulation routines */

static FILE *cmd_file;

/* open a command file */

int
open_cmdfile(file)
char *file;			/* name of file (-.mtx) */
	{
	char buff[128];	/* space to hold path name */
	char *p,*index();

	/* get command argument (obviously temporary) */

	if ((p=index(file+1,' '))) {
		*p = '\0';
		strcpy(arg,p+1);
		}
	else
		*arg = '\0';

	if (*file == '.' || *file == '/')
		sprintf(buff,"%s.%s",file,"mtx");
	else
		sprintf(buff,"%s/%s.%s",HOME,file,"mtx");
	Dprintf('X',"Opening command file %s\n",buff,0,0);

	clear_code();
	if ((cmd_file = fopen(buff,"r")) != NULL) {
		Dprintf('X',"Open suceeded\n");
		set_lock(L_PROC);
		return(1);
		}
	else {
		Dprintf('X',"Open FAILED\n");
		return(0);
		}
	}

/* do the next command in a file */

static char cmd_buff[256];		/* max command line length */

int
get_cmdline(buff,n)
char *buff;
int n;			/* sizeof buff */
	{
	register char *cmd;

	while ((cmd=fgets(cmd_buff,n,cmd_file)) && *cmd_buff == '#')
		;
	if (cmd) {
		Dprintf('X',"Got command line [%s]\n",cmd_buff,0);
		cmd_buff[strlen(cmd_buff)-1] = '\0';
		sprintf(buff,cmd_buff,arg);
		return(*cmd);
		}
	else  {
		close_cmdfile();
		return('\0');
		}
	}

/* close a command file - finished or error */

int
close_cmdfile()
	{
	Dprintf('X',"Command file finished\n",0,0);
	fclose(cmd_file);
	}

/* (preliminary) stuff for tracking command file errors */

static int command_code = 0;			/* return code for commands */

int
clear_code()
	{
	command_code = 0;
	return(0);
	}

int
set_code(code)
int code;
	{
	return(command_code = code);
	}

int
get_code()
	{
	return(command_code);
	}
