/*{{{}}}*/
/*{{{  #includes*/
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
/*}}}  */
/*{{{  #defines*/
/*
**	size of input and output buffers
*/
#define BFRSIZE		1024
#define SHELL		"/bin/sh"
/*}}}  */

/*{{{  function prototypes */
extern void cleanup();
extern int inmassage(char *, int);
extern int outmassage(char *, int);
int get_command(char **, int *);
/*}}}  */

/*{{{  variables*/
static char line[] = {"/dev/ptypX"};
extern char **environ;
int shellid;
int rem;
int verboseflag = 0;
#ifdef sun
extern int errno;
#endif

/*
** flags to signal that massage routines have more data ready to go
**	these flags are necessary so that massage routines can buffer
**	data if necessary
*/
int more_out = 0;
int more_in = 0;
/*}}}  */

/*{{{  mode*/
/* set terminal mode to: canonical, 0, or raw, 1. */
void mode(int f)
{
  struct termios term;
  static struct termios saved_term;

      tcgetattr(0, &term);
      switch (f) {
      case 0:
              /* restore canonical mode */
              tcsetattr(0, TCSAFLUSH, &saved_term);
              break;
      case 1:
              /* set terminal to raw mode */
              tcgetattr(0, &saved_term);
              term = saved_term;  
              term.c_lflag &= ~(ECHO|ICANON|IEXTEN|ISIG);
              term.c_iflag &= ~(BRKINT|ICRNL|INPCK|ISTRIP|IXON);
              term.c_cflag &= ~(CSIZE|PARENB);
              term.c_cflag |= CS8;
              term.c_oflag &= ~(OPOST);
              term.c_cc[VMIN] = 1;
              term.c_cc[VTIME] = 0;
              tcsetattr(0, TCSAFLUSH, &term);
              break;
      }
}
/*}}}  */
/*{{{  done*/
void
done()
{

	mode(0);
	if (shellid > 0 && kill(shellid, SIGKILL) >= 0)
		wait((int *)0);
	cleanup();
}
/*}}}  */
/*{{{  lostpeer*/
void
lostpeer()
{
	signal(SIGPIPE, SIG_IGN);
	fprintf(stderr,"\007Connection closed.\r\n");
	done();
}
/*}}}  */
/*{{{  getpty*/
int getpty(char **cmd)
{
	char pibuf[BFRSIZE], fibuf[BFRSIZE], *pbp, *fbp;
	int pcc = 0, fcc = 0;
	int cc;

	signal(SIGPIPE, lostpeer);
        shellid = get_command(cmd,&rem);
        if (rem < 0) return(-1);

	signal(SIGINT, exit);
	signal(SIGHUP, exit);
	signal(SIGQUIT, exit);
	mode(1);
	signal(SIGINT, SIG_IGN);
	signal(SIGCHLD, done);
	for (;;)
	{
	fd_set ibits, obits;

		FD_ZERO(&ibits);
		FD_ZERO(&obits);

		if (fcc)
			FD_SET(rem, &obits);
		else
			FD_SET(1, &ibits);
		if (pcc >= 0)
			if (pcc)
				FD_SET(1, &obits);
			else
				FD_SET(rem, &ibits);
		if (fcc < 0 && pcc < 0)
			break;
		select(16, &ibits, &obits, 0, 0);
		if ((fcc == 0) && more_in)
		{
			fbp = fibuf;
			fcc = inmassage(fibuf,-2);
		}
		else
		{
			if (FD_ISSET(0, &ibits)) {
				fcc = read(0, fibuf, sizeof (fibuf));
				if (fcc < 0 && errno == EWOULDBLOCK)
					fcc = 0;
				else {
					if (fcc <= 0)
						break;
					fbp = fibuf;
					fcc = inmassage(fibuf,fcc);
				}
			}
		}
		if ((pcc == 0) && more_out)
		{
			pbp = pibuf;
			pcc = outmassage(pibuf,-2);
		}
		else
		{
			if (FD_ISSET(rem, &ibits)) {
				pcc = read(rem, pibuf, sizeof (pibuf));
				pbp = pibuf;
				if (pcc < 0 && errno == EWOULDBLOCK)
					pcc = 0;
				else if (pcc <= 0)
					pcc = -1;
				pcc = outmassage(pibuf,pcc);
			}
		}
		if ((FD_ISSET(1, &obits)) && pcc > 0) {
			cc = write(1, pbp, pcc);
			if (cc > 0) {
				pcc -= cc;
				pbp += cc;
			}
		}
		if ((FD_ISSET(rem, &obits)) && fcc > 0) {
			cc = write(rem, fbp, fcc);
			if (cc > 0) {
				fcc -= cc;
				fbp += cc;
			}
		}
	}
	fprintf(stderr,"Closed connection.\r\n");
	done();
	/* this point should never be reached !!! */
	return(-2);
}
/*}}}  */

/*{{{  getapty -- get a pty line*/
static int
getapty()
{
  int i;
  int pty_fd, tty_fd;

  for(line[8]='p';line[8]<='t';line[8]++) for (i=0;i<=16;i++)
  {
    line[9]="0123456789abcdef"[i%16];
    line[5] = 'p';
    if ((pty_fd=open(line,O_RDWR))>=0)
    {
      line[5] = 't';
      if ((tty_fd=open(line,O_RDWR))>=0) { close(tty_fd); return(pty_fd); }
      else close(pty_fd);
    }
  }
  return(-1);
}
/*}}}  */

/*{{{  last_tty*/
char *
last_tty()
{
   return(line);
}
/*}}}  */

/*{{{  doenv -- change an environment variable*/
void do_env(name,value) char *name,*value;
{
   register int i;
   int n = strlen(name);

   for(i=0;environ[i] != (char *) 0;i++)
      if (strncmp(environ[i],name,n) == 0) {
         strcpy(environ[i]+n,value);
         break;
         }
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

   if (index("/.",*name))
      if (access(name,X_OK)==0)
         return(name);
      else
         return((char *)0);

   strcpy(start,getenv("PATH"));
   for (list = start; (next = index(list,':')) != NULL; list = next+1) {
      *next = '\0';
      sprintf(Path,"%s/%s",list,name);
      if (access(Path,X_OK) == 0)
         return(Path);
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
int get_command(char **argv, int *file)
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

   if (name == (char *) 0 || *name == '\0') return(-2);

   if ((*file=getapty())<0) return(-1);
   if ((pid=fork())>0)
   /*{{{  parent side of fork*/
   	return(pid);
   /*}}}  */
   else if (pid<0)
   /*{{{  error side of fork*/
   	return(pid);
   /*}}}  */

   /*{{{  child side of fork*/
   /*{{{  set signal handlers to default*/
   for(i=0;i<NSIG;i++) signal(i,SIG_DFL);
   /*}}}  */
   /*{{{  void association with controlling terminal*/
   /* On a POSIX system the call to setsid will detach the controlling terminal */
   /* On other systems (mostly BSD-derived) you have to do it explicitly  */
   /* But since the latter don't have setsid, this code won't work on them any way */
   /* If support for them is required, I can add later */
   /* setpgrp() is redundant */

   #if defined(linux) || defined(sun)
   tty_slots = getdtablesize();
   #else
   tty_slots = 20;
   #endif

   for (i=0; i<tty_slots; i++) close(i);

   setsid();
   /*}}}  */
   /*{{{  make it controlling tty*/
   /* On some systems we need an explicit ioctl call */
   /* On most modern systems, the first file openned after setsid
      will become the controlling terminal */
   /* set the uid-0 stuff up */
   /* the setre* calls are BSDish, but Linux handles them ok */

   fd=open(line,O_RDONLY);
   if (fd!=0) perror("mgr: internal error with opening fd 0");
   #if defined(linux) || defined(sun)
   if (geteuid() < 1) {
      int uid = getuid();
      fchmod(fd,0622);
      fchown(fd,uid,-1);
      setreuid(uid,uid);

      uid = getgid();
      fchown(fd,-1,uid);
      setregid(uid,uid);
      }
   #endif
   if (open(line,O_WRONLY)!=1) { perror("mgr: internal error with opening fd 1"); exit(1); }
   if (open(line,O_WRONLY)!=2) { perror("mgr: internal error with opening fd 2"); exit(1); }
   /*}}}  */
   /*{{{  set up tty mode*/
   #   if 0
   adjust_mode(0,ECHO|ICANON|ISIG);
   restore_modes(0);
   #   endif
   /*}}}  */
   /*{{{  add a utmp entry*/
   #      ifdef WHO
   add_utmp(line);
   #      endif
   /*}}}  */
   /*{{{  start the command*/
   do_env("TERM=","hp2621-nl");
   execve(name,argv,environ);
   _exit(1);
   /*}}}  */
   /*}}}  */
}
/*}}}  */
