/*{{{}}}*/
/*{{{  #includes*/
#include <stdio.h>

#include "proto.h"
#include "do_button.h"
/*}}}  */

/*{{{  signal names, descriptions (for debugging)*/
static struct signame {
   char *symbol;
   int  number;
   char *desc;
   } signames[] = {
   { "SIGNONE",	0,	"Internal mgr error", },
   { "SIGHUP",	1,	"hangup", },
   { "SIGINT",	2,	"interrupt", },
   { "SIGQUIT",	3,	"quit", },
   { "SIGILL",	4,	"illegal instruction (not reset when caught)", },
   { "SIGTRAP",	5,	"trace trap (not reset when caught)", },
   { "SIGIOT",	6,	"IOT instruction", },
   { "SIGEMT",	7,	"EMT instruction", },
   { "SIGFPE",	8,	"floating point exception", },
   { "SIGKILL",	9,	"kill (cannot be caught or ignored)", },
   { "SIGBUS",	10,	"bus error", },
   { "SIGSEGV",	11,	"segmentation violation", },
   { "SIGSYS",	12,	"bad argument to system call", },
   { "SIGPIPE",	13,	"write on a pipe with no one to read it", },
   { "SIGALRM",	14,	"alarm clock", },
   { "SIGTERM",	15,	"software termination signal from kill", },
   { "SIGURG",	16,	"urgent condition on IO channel", },
   { "SIGSTOP",	17,	"sendable stop signal not from tty", },
   { "SIGTSTP",	18,	"stop signal from tty", },
   { "SIGCONT",	19,	"continue a stopped process", },
   { "SIGCHLD",	20,	"to parent on child stop or exit", },
   { "SIGTTIN",	21,	"to readers pgrp upon background tty read", },
   { "SIGTTOU",	22,	"like TTIN for output if (tp->t_local&LTOSTOP)", },
   { "SIGIO",	23,	"input/output possible signal", },
   { "SIGXCPU",	24,	"exceeded CPU time limit", },
   { "SIGXFSZ",	25,	"exceeded file size limit", },
   { "SIGVTALRM",	26,	"virtual time alarm", },
   { "SIGPROF",	27,	"profiling time alarm", },
   { "SIGWINCH",	28,	"window changed", },
   { (char *) 0,	0,	(char *) 0 },
};
/*}}}  */

/*{{{  catch -- unexpected signals go here, print message, restore state, then die!*/
void catch(int sig)
{
  _quit();
  fprintf(stderr,"got a %s:%s\r\n",signames[sig].symbol, signames[sig].desc);
  fflush(stderr);
  abort();
}
/*}}}  */
