/*
 * print on stdout shell commands to set TERMCAP, COLUMNS, LINES.
 * A -v flag means add vi-mode sequences to the TERMCAP.
 *   (Assumes TERMCAP from the server contains no ti or te.)
 *
 * author: Vincent Broman, broman@nosc.mil, feb 1994,
 * with thanks to Bellcore and Haardt who supplied ideas in previous imps.
 */

#include <stdio.h>
#include <string.h>
#include "mgr/mgr.h"

void barf( s)
char *s;
{
    fprintf( stderr, "%s: failed to communicate with the MGR server.\n", s);
    exit( 1);
}

int
main( argc, argv)
int argc;
char *argv[];
{
   int vi_flag, columns, rows;
   char *shell, tcap_str[1024], *tp, *getenv();
   enum { SH, CSH, RC} syntax;

   ckmgrterm( argv[0]);

   vi_flag = (argc > 1 && strncmp( "-v", argv[1], 2) == 0);

   shell = getenv( "SHELL");
   if( shell) {
      char *shellend = shell + strlen( shell);
      if( strcmp( shellend - 3, "csh") == 0)
	 syntax = CSH;
      else if( strcmp( shellend - 2, "rc") == 0)
	 syntax = RC;
      else
	 syntax = SH;
   } else
      syntax = SH; /* default */

   m_setup( M_MODEOK);
   m_ttyset();
   if( get_colrow( &columns, &rows) == 0)  barf( argv[0]);
   m_getinfo( G_TERMCAP);
   m_flush();
   if( m_gets( tcap_str) == 0)  barf( argv[0]);
   m_ttyreset();
   
   tp = strrchr( tcap_str, ':');
   if( tp == NULL)  barf( argv[0]);
   if( vi_flag)
      strcpy( tp + 1, "ti=\\EV:te=\\Ev:");
   else
      tp[ 1] = 0;  /* zap the NL */

   switch( syntax) {
   case CSH:
      printf( "set noglob; setenv TERMCAP '%s'; unset noglob;\n", tcap_str);
      printf( "setenv COLUMNS %d;\n", columns);
      printf( "setenv LINES %d\n", rows);
      break;
   case SH:
      printf( "TERMCAP='%s';\n", tcap_str);
      printf( "COLUMNS=%d;\n", columns);
      printf( "ROWS=%d;\n", rows);
      printf( "export TERMCAP COLUMNS ROWS\n");
      break;
   case RC:
      printf( "TERMCAP='%s';\n", tcap_str);
      printf( "COLUMNS=%d;\n", columns);
      printf( "ROWS=%d\n", rows);
   }
   return( 0);
} 
