/*{{{}}}*/
/*{{{  Notes*/
/*                        Copyright (c) 1987 Bellcore
 *                            All Rights Reserved
 *       Permission is granted to copy or use this program, EXCEPT that it
 *       may not be sold for profit, the copyright notice must be reproduced
 *       on copies, and credit should be given to Bellcore where it is due.
 *       BELLCORE MAKES NO WARRANTY AND ACCEPTS NO LIABILITY FOR THIS PROGRAM.
 */

/* main routine for MGR */
/*}}}  */
/*{{{  #includes*/
#include <mgr/bitblit.h>
#include <mgr/font.h>
#include <mgr/share.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <getopt.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#if defined(sun)
#define NSIG 32
#endif
#include <stdlib.h>
#include <stdio.h>

#include "clip.h"
#include "defs.h"
#include "event.h"
#include "menu.h"
#include "version.h"

#include "proto.h"
#include "border.h"
#include "colormap.h"
#include "copyright.h"
#include "destroy.h"
#include "do_buckey.h"
#include "do_button.h"
#include "do_event.h"
#include "erase_win.h"
#include "font_subs.h"
#include "icon_server.h"
#include "kbd.h"
#include "mouse.h"
#include "mouse_get.h"
#include "put_window.h"
#include "sigdata.h"
#include "set_mode.h"
#include "startup.h"
#include "subs.h"
/*}}}  */

/*{{{  variables*/
static struct timeval set_poll = { (long)0, (long)POLL_INT };
static struct timeval set_poll_save;
static char *mouse_dev = MOUSE_DEV;
static char *mouse_type = NULL;
BITMAP *pattern=&def_pattern;
#ifdef MOVIE
char *log_command=NULL;		/* process to pipe logging info to */
FILE *log_file=NULL;		/* file pointer for logging */
static int log_now=0;
#endif
/*}}}  */

/*{{{  sig_child -- catch dead children*/
static void sig_child(sig) int sig;
{
  WINDOW *win;
  pid_t pid;
  int status, someonedied;

  /* see if a shell has died, mark deleted */

  dbgprintf('d',(stderr,"Looking for dead windows\r\n"));

#ifdef WSTOPSIG	/* POSIX */
  pid = waitpid(-1,&status,WUNTRACED);
  someonedied = pid > 0 && !WIFSTOPPED(status);
#else
#ifdef WUNTRACED	/* BSD */
  pid = wait3(&status,WUNTRACED,(void *)NULL);
  someonedied = pid > 0 && !WIFSTOPPED(status);
#else			/* other Unix */
  pid = wait(&status);
  someonedied = pid > 0;
#endif
#endif
  if (someonedied) {
    win=active;
    for (win=active; win!=(WINDOW*)0; win=W(next))
    {
      if (W(pid)==pid && !(W(flags)&W_NOKILL) && kill(W(pid),0) != 0)
      {
	W(flags) |= W_DIED;
	dbgprintf('d',(stderr, "window %d, tty %s, pid %d\r\n",
		      W(num),W(tty),W(pid)));
      }
    }
  }
  signal(SIGCHLD,sig_child);
}
/*}}}  */
/*{{{  sig_share -- switch logging on/off*/
static void sig_share(int n)
{
# ifdef MOVIE
  if (n==SIGUSR1) log_start(log_file);
  else log_end();
# endif
}
/*}}}  */
/*{{{  proc_mouse -- process mouse*/
static int
proc_mouse(mouse)
int mouse;
   {
   int dx, dy;
   register int button, done = 0;

   do {
      button = mouse_get(mouse,&dx,&dy);
      MOUSE_OFF(screen,mousex,mousey);
      mousex += 2*dx;
      mousey -= 2*dy;
      mousex = BETWEEN(0,mousex,BIT_WIDE(screen)-1);
      mousey = BETWEEN(0,mousey,BIT_HIGH(screen)-1);
      if (button != button_state) {
         do_button( button );
         done++;
         }
      MOUSE_ON(screen,mousex,mousey);
      } while (mouse_count() && !done);
   return(done);
   }
/*}}}  */
/*{{{  mouse_reopen -- reopen the mouse after suspend*/
int mouse_reopen()
{
  mfd=open(mouse_dev,O_RDWR);
  ms_init(BIT_WIDE(screen),BIT_HIGH(screen),mouse_type);
  return(mfd);
}
/*}}}  */

/* could use faster longs instead of chars if sizeof(fd_set)%sizeof(long)==0 */
typedef char FDword;

int
FD_COMMON( fd_set *left, fd_set *right) {
   /* predicate returns true iff any fds are set in both left and right */
   FDword *lp = (FDword *)left;
   FDword *ep = (FDword *)(left + 1);
   FDword *rp = (FDword *)right;

   while( lp < ep)
      if( *lp++ & *rp++)
	 return 1;
   return 0;
}

void
FD_SET_DIFF( fd_set *target, fd_set *left, fd_set *right) {
   /* assigns the set difference of left-right to target */
   FDword *tp = (FDword *)target;
   FDword *lp = (FDword *)left;
   FDword *ep = (FDword *)(left + 1);
   FDword *rp = (FDword *)right;

   while( lp < ep)
      *tp++ = *lp++ & ~(*rp++);
}
#ifdef DEBUG
#define HD(_fds) (*(unsigned long int *)&(_fds))
#endif


/*{{{  main*/
int main(argc,argv) int argc; char **argv;
   {
   register WINDOW *win;		/* current window to update */
   register int i;			/* counter */
   register int count;			/* # chars read from shell */
   int maxbuf = MAXBUF;			/* # chars processed per window */
   int shellbuf = MAXSHELL;		/* # chars processed per shell */
   fd_set reads;			/* masks, result of select */
   struct timeval *poll_time;
   int flag;
   unsigned char c;			/* reads from kbd go here */
   char start_file[MAX_PATH];		/* name of startup file */
   char *screen_dev = SCREEN_DEV;	/* name of frame buffer */
   char *default_font = (char * )0;	/* default font */
   int touch_colormap = 1;

   timestamp();					/* initialize the timestamp */
   SETMOUSEICON(DEFAULT_MOUSE_CURSOR);

   sprintf(start_file,"%s/%s",getenv("HOME"),STARTFILE);
   /*{{{  parse arguments*/
   while ((flag=getopt(argc,argv,"d:p:cvxm:s:F:M:P:b:B:f:i:S:z:Z:"))!=EOF)
   switch(flag)
   {
     /*{{{  d*/
#     ifdef DEBUG
     case 'd': debug = 1;
               strcpy(debug_level,optarg);
               fprintf(stderr,"Debug level: [%s]\n",debug_level);
               break;
#     endif
     /*}}}  */
     /*{{{  p -- set background pattern*/
     case 'p':
     {
       FILE *fp;

       if ((fp=fopen(optarg,"rb"))!=(FILE*)0)
       {
         if ((pattern=bitmapread(fp))==(BITMAP*)0)
         {
           fprintf(stderr,"mgr: Invalid background pattern bitmap %s.\n",optarg);
           exit(1);
         }
         fclose(fp);
       }
       else
       {
         fprintf(stderr,"mgr: Can't open background pattern bitmap %s.\n",optarg);
         exit(1);
       }
       break;
     }
     /*}}}  */
     /*{{{  v -- print version number and quit*/
     case 'v':
               fputs(version,stdout);
               exit(1);
     /*}}}  */
     /*{{{  c -- dont touch the colormap on startup, wait for orders */
     case 'c':
               touch_colormap = 0;
               break;
     /*}}}  */
     /*{{{  x -- use /dev/null as startfile*/
     case 'x': strcpy(start_file,"/dev/null");
               break;
     /*}}}  */
     /*{{{  m -- set mouse device*/
     case 'm':
          mouse_dev = optarg;
          break;
     /*}}}  */
     /*{{{  M -- set mouse protocol type */
     case 'M':
          mouse_type = optarg;
          break;
     /*}}}  */
     /*{{{  s -- set start file*/
     case 's':
          strcpy(start_file,optarg);
          break;
     /*}}}  */
     /*{{{  F -- set default font*/
     case 'F':
          default_font = optarg;
          break;
     /*}}}  */
     /*{{{  P -- set polling timeout*/
     case 'P':
          set_poll.tv_usec = (long) atoi(optarg);
          break;
     /*}}}  */
     /*{{{  b -- set shell buffering*/
     case 'b':
          shellbuf = atoi(optarg);
          shellbuf = BETWEEN(5,shellbuf,1024);
          break;
     /*}}}  */
     /*{{{  B -- set window buffering*/
     case 'B':
          maxbuf = atoi(optarg);
          maxbuf = BETWEEN(1,maxbuf,shellbuf);
          break;
     /*}}}  */
     /*{{{  f -- set font directory*/
     case 'f':
          font_dir = optarg;
          break;
     /*}}}  */
     /*{{{  i -- set icon directory*/
     case 'i':
          icon_dir = optarg;
          break;
     /*}}}  */
     /*{{{  S -- set alternate frame buffer*/
     case 'S':
          screen_dev = optarg;
          break;
     /*}}}  */
#   ifdef MOVIE
     /*{{{  Z -- set save file and start logging now*/
     case 'Z':
          log_command = optarg;
          log_now++;
          dbgprintf('L',(stderr,"Starting logging NOW at [%s]\n",log_command));
          break;
     /*}}}  */
     /*{{{  z -- set save file*/
     case 'z':
          log_command = optarg;
          dbgprintf('L',(stderr,"Starting logging LATER at [%s]\n",log_command));
          break;
     /*}}}  */
#   endif
     /*{{{  default -- invalid flag*/
     default:
     fprintf(stderr,"Usage: mgr ");
     fprintf(stderr,"[-d <level>]");
     fprintf(stderr,"[-vx][-m <mouse>][-s <.rc file>][-F <default font>]\n");
     fprintf(stderr,"           [-P <polling timeout>][-b <shell buffer>][-B <window buffer>]\n");
     fprintf(stderr,"           [-f <font directory>][-i <icon directory>][-S <frame buffer>]\n");
#     ifdef MOVIE
     fprintf(stderr,"           [-Z <logfile>][-z <logfile>]\n");
#     endif
     fprintf(stderr,"           [-p <pattern>]\n");
     exit(1);
     /*}}}  */
   }
   /*}}}  */
   /*{{{  keep mgr from being run within itself*/
   {
     char *t = getenv("TERM");

     if (t && strncmp(t,TERMNAME,strlen(TERMNAME)) == 0) {
       fprintf(stderr,"mgr: I can't invoke me from within myself.\n");
       exit(1);
     }
   }
   /*}}}  */
   /*{{{  save tty modes for ptty's*/
   save_modes(0);
   /*}}}  */
   /*{{{  free all unused fd's*/
   /* free all unused fd's */
   count=getdtablesize();
   for (i=3; i<count; i++) close(i);
   /*}}}  */
   /*{{{  initialize the keyboard; sometimes a special device*/
   initkbd();
   /*}}}  */
   /*{{{  initialize the bell; sometimes a special device requiring funnys*/
   initbell();
   /*}}}  */
   /*{{{  get the default font file*/
   if (default_font || (default_font = getenv(DEFAULT_FONT)))
      font = open_font(default_font);

   if (font == (struct font *) 0)
      font = open_font("");
   font->ident = 0;
   /*}}}  */
   /*{{{  open the mouse*/
   if ((mouse=open(mouse_dev,O_RDWR)) <0) {
      perror("mgr: Can't find the mouse, or it is already in use.\n");
      exit(1);
      }
   mfd=mouse;
   /*}}}  */
   /*{{{  find screen*/
   if ((screen = bit_open(screen_dev)) == (BITMAP *) 0)
   {
      perror("mgr: Can't open the screen.");
      exit(2);
   }
   ms_init(BIT_WIDE(screen),BIT_HIGH(screen),mouse_type);
   mousex=mousey=32;
   bit_grafscreen();
   if (getenv("MGRSIZE"))
   {
        int x, y, w, h;

        prime=screen;
   	sscanf(getenv("MGRSIZE"),"%d %d %d %d",&x,&y,&w,&h);
     	screen = bit_create(prime,x,y,w,h);
   }
   else prime=(BITMAP*)0;
   init_colors( screen);
   if( touch_colormap)
      fill_colormap( screen);
   /*}}}  */
   set_tty(0);
   /*{{{  catch the right interrupts*/
   for (i=0; i<NSIG; i++) switch(i)
   {
      case SIGUSR1:     /* experimental logging stuff */
      case SIGUSR2:     signal(i,sig_share);
                        break;
      case SIGCHLD:     signal(SIGCHLD,sig_child);
                        break;
      case SIGILL:	/* <= 3.0 abort gererates this one */
      case SIGCONT:
      case SIGIOT:	/* 3.2 abort generates this (gee thanks, SUN!) */
      case SIGQUIT:
                        break;
      case SIGTTIN:
      case SIGTTOU:     signal(i,SIG_IGN);
                        break;
      default:          signal(i,catch);
                        break;
      }
   /*}}}  */
   copyright(screen, "");
   mouse_save=bit_alloc(16,32,0,BIT_DEPTH(screen));
   SETMOUSEICON(&mouse_cup);
   /*{{{  always look for keyboard and mouse input*/
   FD_ZERO( &mask);
   FD_SET( mouse, &mask);
   FD_SET( 0, &mask);
   FD_ZERO( &to_poll);
   FD_ZERO( &reads);
   memcpy(&set_poll_save,&set_poll,sizeof(set_poll));
   /*}}}  */
   /*{{{  get default font definitions*/
      {
      char buff[MAX_PATH];
      sprintf(buff,"%s/%s",font_dir,STARTFILE);
      startup(buff);
      }
   /*}}}  */
#   ifdef MOVIE
   /*{{{  start logging*/
   if (log_now) {
     log_noinitial = 1;			/* no need to save initial display image */
     do_buckey('S' | 0x80);		/* simulate the key-press */
   }
   /*}}}  */
#   endif
   /*{{{  process startup file*/
   startup(start_file);
   if (active != (WINDOW *) 0)
      ACTIVE_ON();
   else {
      MOUSE_OFF(screen,mousex,mousey);
      erase_win(screen);
      MOUSE_ON(screen,mousex,mousey);
   }
   /*}}}  */
   /*{{{  turn on mouse cursor*/
   MOUSE_OFF(screen,mousex,mousey);
   SETMOUSEICON(DEFAULT_MOUSE_CURSOR);
   MOUSE_ON(screen,mousex,mousey);
   /*}}}  */
   /* main polling loop */

   while(1) {

      /* see if any window died */

      for(win=active;win != (WINDOW *) 0; )
         if (W(flags)&W_DIED) {
            dbgprintf('d',(stderr,"Destroying %s-%d\r\n",W(tty),W(num)));
            destroy(win);
            win = active;
            }
         else
            win = W(next);

      /* wait for input */

      FD_SET_DIFF( &reads, &mask, &to_poll);	/* reads = mask & ~to_poll */

      dbgprintf('l',(stderr,"select: mask=0x%lx to_poll=0x%lx 0x%lx got\r\n",
		    (unsigned)HD(mask),(unsigned)HD(to_poll),(unsigned)HD(reads)));
#ifdef MOVIE
      log_time();
#endif
      if(FD_COMMON(&to_poll,&mask)) {
	 (void) memcpy(&set_poll,&set_poll_save,sizeof(set_poll));
	 poll_time = &set_poll;
      }
      else
	 poll_time = NULL;
      if (select(FD_SETSIZE,&reads,0,0,poll_time) <0) {
#ifdef DEBUG
         dbgprintf('l',(stderr,"select failed %ld->%ld\r\n",
		       (int)HD(reads), (int) (HD(mask) & ~HD(to_poll))));
         if (debug)
            perror("Select:");
#endif
         FD_SET_DIFF( &reads, &mask, &to_poll);	/* reads = mask & ~to_poll */
         continue;
         }
      dbgprintf('l',(stderr,"reads=0x%lx\r\n",(unsigned long)HD(reads)));

      /* process mouse */

      if (FD_ISSET( mouse, &reads)) {
	 do {
	    proc_mouse(mouse);
	 } while(mouse_count() > 0);
      }
      /* process keyboard input */

      if (FD_ISSET( 0, &reads) && active && !(ACTIVE(flags)&W_NOINPUT))
      {
         read(0,&c,1);
#ifdef BUCKEY
         if ( (ACTIVE(flags)&W_NOBUCKEY)  ||  !do_buckey(c) )
            write(ACTIVE(to_fd),&c,1);
#else
         write(ACTIVE(to_fd),&c,1);
#endif
		if (ACTIVE(flags)&W_DUPKEY && c==ACTIVE(dup))
         	write(ACTIVE(to_fd),&c,1);
         continue;
         }
       else if (FD_ISSET( 0, &reads) && !active) {	/* toss the input */
         read(0,&c,1);
#ifdef BUCKEY
         do_buckey(c);
#endif
	 }

      /* process shell output */

      for(win=active;win != (WINDOW *) 0;win=W(next))
      {
         /* read data into buffer */

	 if (W(from_fd) && FD_ISSET( W(from_fd), &reads)
			&& !FD_ISSET( W(from_fd), &to_poll)) {
            W(current) = 0;
            if ((W(max) = read(W(from_fd),W(buff),shellbuf)) > 0) {
               FD_SET( W(from_fd), &to_poll);
               dbgprintf('p',(stderr,"%s: reading %d [%.*s]\r\n",W(tty),
			     W(max),W(max),W(buff)));
               }
            else {
               FD_CLR( W(from_fd), &to_poll);
#ifdef KILL
               if (W(flags)&W_NOKILL) W(flags) |= W_DIED;
#endif
#ifdef DEBUG
               if(debug) {
                  fprintf(stderr,"%s: read failed after select on fd(%d) returning %d\r\n",
                          W(tty),W(from_fd),W(max));
                  perror(W(tty));
                  }
#endif
               }
            }

         /* check for window to auto-expose */

         if (W(from_fd) && FD_ISSET( W(from_fd), &to_poll)
	     && W(flags)&W_EXPOSE && !(W(flags)&W_ACTIVE)) {
            dbgprintf('o',(stderr,"%s: activating self\r\n",W(tty)));
            MOUSE_OFF(screen,mousex,mousey);
            cursor_off();
            ACTIVE_OFF();
            expose(win);
            ACTIVE_ON();
            cursor_on();
            MOUSE_ON(screen,mousex,mousey);
            }

         /* write data into the window */

         if (W(from_fd) && FD_ISSET( W(from_fd), &to_poll)
	     && W(flags)&(W_ACTIVE|W_BACKGROUND)) {

#ifdef PRIORITY			/* use priority scheduling */
            if (win==active)
               count = Min(maxbuf,W(max)-W(current));
            else if (W(flags)&W_ACTIVE)
               count = Min(maxbuf>>1,W(max)-W(current));
            else
               count = Min(maxbuf>>2,W(max)-W(current));
#else				/* use round robin scheduling */
            count = Min(maxbuf,W(max)-W(current));
#endif

            i = put_window(win,W(buff)+W(current),count);
            dbgprintf('w',(stderr,"%s: writing %d/%d %.*s [%.*s]\r\n",
			  W(tty),i,count,i,W(buff)+W(current),count-i,
			  W(buff)+W(current)+i));

            W(current) += i;
            if (W(current) >= W(max))
               FD_CLR( W(from_fd), &to_poll);
            }
         }
      }
   }
/*}}}  */
