/*                        Copyright (c) 1987 Bellcore
 *                            All Rights Reserved
 *       Permission is granted to copy or use this program, EXCEPT that it
 *       may not be sold for profit, the copyright notice must be reproduced
 *       on copies, and credit should be given to Bellcore where it is due.
 *       BELLCORE MAKES NO WARRANTY AND ACCEPTS NO LIABILITY FOR THIS PROGRAM.
 */
/*	$Header: restart.h,v 4.1 88/06/21 13:40:38 bianchi Exp $
	$Source: /tmp/mgrsrc/lib/RCS/restart.h,v $
*/
static char	h_restart_[] = "$Source: /tmp/mgrsrc/lib/RCS/restart.h,v $$Revision: 4.1 $";

/* stuff for restarting upon reshape/ redraw */

#ifndef ECHO
#include <sgtty.h>
#endif

#include <setjmp.h>

#ifndef SIGQUIT
#include <signal.h>
#endif

extern int _Catch(), _Clean();
extern jmp_buf _env;

#ifdef QUIT_CHAR
static char *_quit = QUIT_CHAR;
#else
static char *_quit = "\034";
#endif

#define Ignore()	signal(SIGQUIT,SIG_IGN)

#define Restart()	signal(SIGINT,_Clean), \
			 signal(SIGTERM,_Clean), \
			 signal(SIGQUIT,_Catch), \
			 m_saveenvcount = m_envcount, \
			 m_pushsave(P_EVENT), \
			 m_setevent(RESHAPE,_quit), \
			 m_setevent(REDRAW,_quit), \
			 m_setevent(UNCOVERED,_quit), \
			 setjmp(_env)
