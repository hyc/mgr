/*{{{}}}*/
/*{{{  Notes*/
/*                        Copyright (c) 1987 Bellcore
 *                            All Rights Reserved
 *       Permission is granted to copy or use this program, EXCEPT that it
 *       may not be sold for profit, the copyright notice must be reproduced
 *       on copies, and credit should be given to Bellcore where it is due.
 *       BELLCORE MAKES NO WARRANTY AND ACCEPTS NO LIABILITY FOR THIS PROGRAM.
 */

/* start a shell */
/*}}}  */
/*{{{  #includes*/
#include <mgr/bitblit.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>

#include "defs.h"

#include "proto.h"
#include "do_button.h"
#include "set_mode.h"
#include "utmp.h"
/*}}}  */
/*{{{  #defines*/
#define SHELL		"/bin/sh"

#ifdef O_RDWR
#define O_RD_MAYBE_WR O_RDWR
#define O_WR_MAYBE_RD O_RDWR
#else
#define O_RD_MAYBE_WR O_RDONLY
#define O_WR_MAYBE_RD O_WRONLY
#endif
/*}}}  */

/*{{{  variables*/
static char line[] = {"/dev/ptypX"};
extern char **environ;
/*}}}  */

/*{{{  getdtablesize*/
#ifdef SYSV
extern int getdtablesize(void)
{
   return(20);
}
#endif
/*}}}  */

/*{{{  getapty -- get a pty line*/
static int getapty()
{
   int i;
   int pty_fd;

   line[5] = 'p';
   for(line[8]='p';line[8]<='t';line[8]++)
      for (i=0;i<=16;i++) {
	 line[9]="0123456789abcdef"[i%16];
	 if ((pty_fd=open(line,O_RDWR))>=0) {
	    line[5] = 't';
	    return(pty_fd);
	 }
      }
   return(-1);
}
/*}}}  */
/*{{{  last_tty*/
char *last_tty()
   {
   return(line);
   }
/*}}}  */
/*{{{  get_path -- get a complete path name from command*/
static char Path[512];
static char start[512];

char *
get_path(name)
char *name;
   {
   register char *next, *list;

   if (strchr("/.",*name))
      if (access(name,X_OK)==0) return(name);
      else return((char *)0);

   strcpy(start,getenv("PATH"));
   for (list=start; (next=strchr(list,':')); list=next+1)
   {
      *next = '\0';
      sprintf(Path,"%s/%s",list,name);
      if (access(Path,X_OK) == 0) return(Path);
   }

   sprintf(Path,"%s/%s",list,name);
   if (list && access(Path,X_OK) == 0) {
      return(Path);
      }
   else {
      return((char *) 0);
      }
   }
/*}}}  */
/*{{{  get_command -- start a command*/
int get_command(argv,file) char **argv; int *file;
{
   /*{{{  variables*/
   register int i;				/* counter */
   int fd;					/* file desc */
   pid_t pid;					/* pid of shell */
   int tty_slots;				/* # of tty slots */
   char *name;
   char *shell = getenv("SHELL");
   char *arg[2];
   /*}}}  */

   if (argv == (char **) 0 )
   {
      argv = arg;
      *argv = shell?shell:SHELL;
      *(argv+1) = (char *) 0;
   }
   name = get_path(argv[0]);

   if (name == (char *) 0 || *name == '\0')
      return(-2);

   if ((*file=getapty())<0)
      return(-1);

   pid = fork();
   if (pid != 0)
      return( pid); /*  parent side of fork, or error */

   /*  child side of fork*/
   /*{{{  set signal handlers to default*/
   for(i=0;i<NSIG;i++)
      signal( i, SIG_DFL);
   /*}}}  */
   /*{{{  void association with controlling terminal*/
   tty_slots = getdtablesize();
   for(i=0;i<tty_slots;i++)
      close(i);

#ifdef NO_SETSID
#ifdef TIOCNOTTY
   {
   int tty;
   tty=open("/dev/tty",O_RDONLY);
   ioctl(tty,TIOCNOTTY,0);
   close(tty);
   }
#endif
#else
   setsid();		/*  set process group and session ID */
#endif
   /*}}}  */
   /*{{{  make it controlling tty*/
   /* set the uid-0 stuff up */

   fd = open( line, O_RD_MAYBE_WR);
   if(fd < 0) {
      sleep( 3);
      fd = open( line, O_RD_MAYBE_WR); /* second chance */
   }
   if (fd!=0) {
      perror("mgr: internal error with opening fd 0");
      exit(1);
   }
   if (geteuid() < 1) {
      uid_t uid = getuid();
      gid_t gid = getgid();

      fchmod(fd,0600);
      fchown(fd,uid,gid);
      setreuid(uid,uid);
      setregid(gid,gid);
   }
   if (open(line,O_WR_MAYBE_RD)!=1) {
      perror("mgr: internal error with opening fd 1");
      exit(1);
   }
   if (open(line,O_WR_MAYBE_RD)!=2) {
      perror("mgr: internal error with opening fd 2");
      exit(1);
   }
   /*}}}  */
   /*{{{  set up tty mode*/
   adjust_mode(0,ECHO|ICANON|ISIG);
   restore_modes(0);
   /*}}}  */
   /*{{{  add a utmp entry*/
#ifdef WHO
   add_utmp(line);
#endif
   /*}}}  */
   /*{{{  start the command*/
   putenv("TERM=" TERMNAME);
   execve(name,argv,environ);
   _exit(1);
   return(-1);    /* for lint, of course */
   /*}}}  */
}
/*}}}  */
/*{{{  half_open -- half open a ptty then return*/
char *
half_open(file)
int *file;
   {
   *file = getapty();
   return( *file < 0? NULL: line);
   }
/*}}}  */
