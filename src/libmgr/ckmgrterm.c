/*{{{}}}*/
/*{{{  Notes*/
/*	Check to see if the TERM environment variable says this is an "mgr"
	terminal.
	If it is not, print a message, optionally preceed by the given text,
	and exit with exit code 1.
	Otherwise, return.
	Absence of a TERM environment variable is considered OK, so that
	these commands can be run within a remote shell.
*/
/*}}}  */
/*{{{  #includes*/
#include <stdlib.h>
#include <stdio.h>
/*}}}  */
extern int is_mgr_term(void);

/*{{{  ckmgrterm*/
extern
void ckmgrterm(char *text)
{
  if (! is_mgr_term())
  {
    if (text && *text)
    {
      fputs(text,stderr);
      fputs(": o",stderr);
    }
    else fputs("O",stderr);
    fputs("nly runs on mgr terminals.\n",stderr);
    exit(1);
  }
}
/*}}}  */
