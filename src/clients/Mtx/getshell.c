/*                        Copyright (c) 1988,1989 Bellcore
 *                            All Rights Reserved
 *       Permission is granted to copy or use this program, EXCEPT that it
 *       may not be sold for profit, the copyright notice must be reproduced
 *       on copies, and credit should be given to Bellcore where it is due.
 *       BELLCORE MAKES NO WARRANTY AND ACCEPTS NO LIABILITY FOR THIS PROGRAM.
 */

/* MTX start a shell [taken from mgr] take II  (S. A. Uhler) */

#include <sys/file.h>
#include <sys/signal.h>
#include <sgtty.h>
#include <stdio.h>

#define SHELL		"/bin/sh"

static char line[] = {'/','d','e','v','/','p','t','y','x','x','\0'};
static int indx = 0;				/* next stack entry to use */
static int master_stack[5];		/* stack of tty #'s */
extern FILE *debug;					/* for debugging */

#define MAXX				(16*6)	/* max # of tty's to look for */

#define GETMSC(x) \
	("pqrstuvw"[((x)>>4)&7])

#define GETLSC(x) \
	("0123456789abcdef"[(x)&0xf])

/*	open the contolling side of a ptty */

int
getapty()
   {
   register int i;
   int fd;

   line[5] = 'p';
	for(i=0;i<MAXX;i++) {
		line[8] = GETMSC(i);
		line[9] = GETLSC(i);
      if ((fd = open(line,2)) >= 0) {
			master_stack[indx++] = i;
         return(fd);
         }
      }
   return(-1);
   }
      
/* open the slave, or terminal side of a ptty */

int getatty()
   {
   int fd;
	int x;

	x = master_stack[--indx];
   line[5] = 't';
	line[8] = GETMSC(x);
	line[9] = GETLSC(x);
   fd=open(line,2);
   if (fd<0) {
      sleep(3);
      return (open(line,2));
      }
   return(fd);
   }

/* return the last 2 chars of the tty name */

char *
get_ttyname()
	{
	static char name[3];
	int x;

	x = master_stack[indx ? indx-1 : 0];
	name[0] = GETMSC(x);
	name[1] = GETLSC(x);
	name[2] = '\0';
	return(name);
	}

/* start a shell */

get_shell(command,file,file2,flags)
char *command;				/* command to run */
int *file;					/* address of file descriptor */
int *file2;					/* address of file descriptor for stderr (if any) */
int flags;					/* initial tty modes */
   {
   register int i;		/* counter */
   int fd1;					/* file desc (primary chanell) */
   int fd2;					/* file desc (secondary chanell) */
   int tty;					/* fd of /dev/tty */
   int pid;					/* pid of shell */
   int group;				/* process group id */
   int tty_slots;			/* # of tty slots */
   char *name;				/* name of shell to exec */
	char **fields;			/* command line broken into fields */
	static char **parse();
   char *getenv();

	if ((name=getenv("SHELL")) == NULL)
		name = SHELL;

	Dprintf('X',"getshell running: %s\n",command?command:name,0);

   if ((*file=getapty()) < 0)
      return(-1);

   if (file2 && (*file2=getapty()) < 0)
      return(-1);

   if ((pid=fork()) > 0) {		/* parent side of fork */
      char buff[2];
      read(*file,buff,sizeof(buff));	/* wait for slave side to open */

		/* try to set non blocking reads */

		set_noblock(*file);
		if (file2) {
			set_noblock(*file2);
			}

      return(pid);
      }

   else if (pid<0)				 /* error side of fork */
      return(pid);

   /* child side of fork - will become the shell */

   for(i=0;i<NSIG;i++)
      signal(i,SIG_DFL);

   /* void association with controlling terminal */

   if (tty = open("/dev/tty",0)) {
		save_modes(tty);
		ioctl(tty,TIOCNOTTY,0);
		close(tty);
		}

   /* open slave side of pttys */

	close(*file);
   if ((fd1=getatty())<0) {		/* this one is /dev/tty */
      perror("Slave side of primary p-tty won't open");
      exit(1);
      }

   if (file2) {
		close(*file2);
		if ((fd2=getatty())<0) {
			perror("Slave side of secondary p-tty won't open");
			exit(1);
			}
      }
	else
		fd2 = dup(fd1);

   group=getpid();

   tty_slots = getdtablesize();
   for(i=0;i<tty_slots;i++)
		if (i != fd1 && i != fd2)
			close(i);

   /* set the uid-0 stuff up */

	set_file(fd1);
	set_file(fd2);

   dup(fd2);				/* stdin */
   dup(fd2);				/* stdout */
  	dup(fd1);				/* stderr */

   setpgrp(group,group);
   ioctl(fd1,TIOCSPGRP,&group);
   ioctl(fd2,TIOCSPGRP,&group);

	adjust_mode(NTTYDISC,flags&1 ? RAW : ECHO|CRMOD|EVENP|ODDP);
	restore_modes(fd1);
	adjust_mode(NTTYDISC,flags&2 ? RAW : ECHO|CRMOD|EVENP|ODDP);
	restore_modes(fd2);

   write(fd1,"\n",1);	/* tell master that slave side is open */
   close(fd1);
	close(fd2);

   /* start the shell */

	Dprintf('X',"getshell:\n",command);
	if (command) {	
		Dprintf('X',"getshell parsing: %s\n",command);
		fields = parse(command);
		execvp(fields[0],fields);
		}
   execlp(name,name,0);
   _exit(1);
   }

/***************************************************************************
 * muck with tty modes (taken from MGR)
 */

/* place to save tty modes */

static int t_ldisc;
static struct sgttyb t_sgttyb;
static struct tchars t_tchars;
static struct ltchars t_ltchars;
static int t_lflags;

/* save current tty mode settings */

save_modes(fd)
int fd;			/* fd to save tty modes from */
	{
   ioctl(fd,TIOCGETD,&t_ldisc);
   ioctl(fd,TIOCGETP,&t_sgttyb);
   ioctl(fd,TIOCGETC,&t_tchars);
   ioctl(fd,TIOCGLTC,&t_ltchars);
   ioctl(fd,TIOCLGET,&t_lflags);
	}

/* restore saved settings */

restore_modes(fd)
int fd;
	{
   ioctl(fd,TIOCSETD,&t_ldisc);
   ioctl(fd,TIOCSETP,&t_sgttyb);
   ioctl(fd,TIOCSETC,&t_tchars);
   ioctl(fd,TIOCSLTC,&t_ltchars);
   ioctl(fd,TIOCLSET,&t_lflags);
	}

/* change tty sg_flags */

adjust_mode(disc,flags)
int flags;		/* flags */
int disc;		/* line disc */
	{
   t_ldisc=disc;
   t_sgttyb.sg_flags = flags;
	}


/* set the uid-0 stuff up */

set_file(fd)
int fd;				/* fd of file to setup */
	{
   if (geteuid() < 1) {
      int uid = getuid();
      fchmod(fd,0622);
      fchown(fd,uid,-1);
      setreuid(uid,uid);

      uid = getgid();
      fchown(fd,-1,uid);
      setregid(uid,uid);
      }
	}

/* set a file to non-blocking */

set_noblock(fd)
int fd;			
	{
	return(fcntl(fd,F_SETFL,fcntl(fd,F_GETFL,0)|FNDELAY));
	}

/* break a command line into fields   (from mgr) */

#define iswhite(x)	(index(" \t",x))

static char *cmd_fields[100];		/* place to store fields */

static char **
parse(line)
register char *line;
   {
	register char **fields = cmd_fields;
   char *index();
   int inword = 0;
   int count = 0;
   char *start;
   register char c;

	Dprintf("parse [%s]\n",line);
   for(start = line;(c = *line);line++)
      if (inword && iswhite(c)) {
         inword = 0;
         *line = '\0';
			Dprintf("parse got [%s]\n",start);
         *fields++ = start;
         count++;
         }
      else if (!inword && !iswhite(c)) {
         start = line;
         inword = 1;
         }

   if (inword) {
      *fields++ = start;
      count++;
      }
   *fields = (char *) 0;
   return(cmd_fields);
   }
