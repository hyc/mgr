/*	$Header: do.c,v 4.1 88/06/21 14:02:38 bianchi Exp $
	$Source: /tmp/mgrsrc/demo/msg/RCS/do.c,v $
*/
static char	RCSid_[] = "$Source: /tmp/mgrsrc/demo/msg/RCS/do.c,v $$Revision: 4.1 $";

/*                        Copyright (c) 1987 Bellcore
 *                            All Rights Reserved
 *       Permission is granted to copy or use this program, EXCEPT that it
 *       may not be sold for profit, the copyright notice must be reproduced
 *       on copies, and credit should be given to Bellcore where it is due.
 *       BELLCORE MAKES NO WARRANTY AND ACCEPTS NO LIABILITY FOR THIS PROGRAM.
 */
/* do a service */

#include <mgr/mgr.h>
#include <stdio.h>
#include <signal.h>

#include "do.h"

#define dprintf		if (debug) fprintf
#define have_icon(icon)	(icon->w && icon->h)
#define GET_OPT(i)	\
	strlen(argv[i])>2 ? argv[i]+2 : argv[++i]

#define MSG		'&'		/* message leader */

#define TIMEOUT		3		/* need first response by now */

#define NORMAL		0		/* quiescent icon */
#define RUNNING		1		/* icon while running command */
#define ACTIVE		2		/* icon while window active */

/* remote options */

#define NONE	0			/* no remotes at all */
#define LOCAL	1			/* command runs locally, ship file */
#define REMOTE	2			/* command runs remotely */

int debug;
int wx,wy,border;			/* initial window parameters */
int fw, fh;				/* font size */
struct icon *icon;			/* current icon */

main(argc,argv)
int argc;
char **argv;
   {
   register int i;

   char *index(), *ttyname(), *getenv();
   int timeout(),clean();

   struct icon icons[3];		/* icons */
   char line[100];			/* mgr inout buffer */
   char file[100];			/* command args go here */
   char message[100];			/* place to format messages */
   char temp[100];			/* temp file name */
   char do_host[32];			/* our host */

   char action;				/* message command */
   char *client_host;			/* client host */
   char *command;			/* command to run */
   char *name = NULL;			/* server name (if not command) */
   char *path;				/* path part of file name */

   int id;				/* id of sender */
   int code;				/* return code from command */
   int xpos, ypos;			/* where command window goes */
   int wide ,high;			/* command window size */
   int nowindow = 0;			/* no command window */
   int font=0;				/* font in leiu of icon */
   int wait=0;				/* wait for ack at command term. */
   int remote=NONE;			/* remote requests permitted */
   int icon_count=0;			/* Number of icons */
   int servers = 0;			/* number of servers out there */
   int bailout = 0;			/* bail out w/out running command */

   debug = (int) getenv("DEBUG");

   /* check for valid environment */

   if (argc < 3) {
      fprintf(stderr,"usage: %s [options] -c<command> icon1 icon2 [icon3]\n",
              *argv);
      fprintf(stderr,"options:\n");
      fprintf(stderr,"   -C nnn   number of columns\n");
      fprintf(stderr,"   -R nnn   number of rows\n");
      fprintf(stderr,"   -f n     font\n");
      fprintf(stderr,"   -n name  name of command (if not command name)\n");
      fprintf(stderr,"   -p       wait for CR at command termination\n");
      fprintf(stderr,"   -W       don't make alternate window\n");
      fprintf(stderr,"   -x nnn   starting window position\n");
      fprintf(stderr,"   -y nnn   starting window position\n");
      fprintf(stderr,"   -i       interactive starting icon position\n");
      fprintf(stderr,"   -r       permit remote invocations\n");
      fprintf(stderr,"   -N       remote invocations run over there1\n");
      exit(1);
      }

   ckmgrterm( *argv );
   
   /* setup mgr */

   m_setup(M_FLUSH);
   chmod(ttyname(2),0600);	/* this is not gauranteed */
   m_push(P_POSITION|P_FONT|P_EVENT|P_FLAGS);

   signal(SIGTERM,clean);
   signal(SIGINT,clean);
   signal(SIGHUP,clean);
   signal(SIGALRM,timeout);

   m_ttyset();
   m_setmode(M_NOWRAP);
   m_setmode(M_ABS);
   m_func(BIT_SRC);

   m_setevent(ACCEPT,"&%f:%m\n");
   m_setevent(REDRAW,"R\n");
   m_setevent(RESHAPE,"S\n");
   m_setevent(MOVE,"M\n");
   m_setevent(ACTIVATE,"A\n");
   m_setevent(DEACTIVATE,"D\n");

   get_size(&wx,&wy,&wide,&high);
   border=m_getbordersize();
   font = get_font(&fw,&fh);
   dprintf(stderr,"%d,%d %dx%d font %d (%d,%d)\n",wx,wy,wide,high,font,fw,fh);

   xpos = wx, ypos=wy;
   gethostname(do_host,sizeof(do_host));

   /* check arguments */

   for(i=1;i<argc;i++) {
      if (*argv[i] == '-')
         switch (argv[i][1]) {
            case 'W':				/* don't make alt. window */
               nowindow++;
               break;
            case 'r':				/* remote file, local command */
               remote=LOCAL;
               break;
            case 'N':				/* remote commmand,local file */
               remote=REMOTE;
               break;
            case 'p':				/* wait for ack */
               wait++;
               break;
            case 'n':				/* specify command name */
               name = GET_OPT(i);
               break;
            case 'c':				/* specify command */
               command = GET_OPT(i);
               break;
            case 'f':				/* specify font */
               font = atoi(GET_OPT(i));
               break;
            case 'C':				/* specify columns */
               wide = 2*border + fw * atoi(GET_OPT(i));
               break;
            case 'R':				/* specify rows */
               high = 2*border + fh * atoi(GET_OPT(i));
               break;
            case 'w':				/* specify wide */
               wide = atoi(GET_OPT(i));
               break;
            case 'i':				/* specify icon start */
               {
                  int x,y;
                  m_push(P_EVENT);
                  m_setevent(BUTTON_1,"Z %p\n");
                  fprintf(stderr,"Click button 1 to indicate icon position\n");
                  while (*m_gets(line) != 'Z')
                     ;
                  sscanf(line,"Z %d %d",&x,&y);
                  wx += x, wy +=y;
                  m_pop();
                  }
               break;
            case 'x':				/* specify x_start */
               xpos = atoi(GET_OPT(i));
               break;
            case 'y':				/* specify y_start */
               ypos = atoi(GET_OPT(i));
               break;
            default:
               fprintf(stderr,"%s: bad flag %c ignored\n",argv[0],argv[i][1]);
            }
      else if (icon_count < 3) {
         download_icon(&icons[icon_count],argv[i],icon_count+1,font);
         icon_count++;
         }
      else
         fprintf(stderr,"%s: invalid arg [%s] ignored\n",argv[0],argv[i]);
      }

   if (RUNNING > icon_count) {
      fprintf(stderr,"%s: Not enough icons specified\n",argv[0]);
      exit(2);
      }
      
   if (debug) 
      getchar();

   set_icon(&icons[NORMAL]);

   if (name == NULL)
      name = command;

   m_setevent(NOTIFY,name);
   sprintf(message,"%c %s",S_HI,name);

   m_broadcast(message);
   alarm(TIMEOUT);
   m_clearmode(M_ACTIVATE);
   m_flush();
   dprintf(stderr,"%s: broadcast id\r\n",name);
         
   while(m_gets(line) != NULL) {

      /* process messages */

      if (*line == MSG) {
         sscanf(line+1,"%d:%c %[^\n]", &id,&action,file);
         dprintf(stderr,"%s id: %d message: %s\r\n",name,id,file);
         switch (action) {
            case C_DO:			/* received command */
               dprintf(stderr,"%s received command from %d (%s)\r\n",
                       name,id,file);
               sprintf(message,"%c%c",S_REPLY,R_RCVD);
               m_sendto(id,message);
               set_icon(&icons[RUNNING]);

               /* no file name in command, use snarf buffer */

               if (*file == C_SNARF) {
                  m_put();
                  m_gets(file);
                  dprintf(stderr,"%s snarfing (%s)\r\n", name,file);
                  }

               /* get host name (if none, assume local host) */

               if (path=index(file,HOST)) {
                  *path = '\0';
                  client_host = file;
                  path++;
                  }
               else {
                  path = file;
                  client_host = do_host;
                  }

               dprintf(stderr,"Got %s at %s\n",path,client_host);
              
               if (strcmp(client_host,do_host)!= 0)
                  switch (remote) {
                     case NONE:		/* no remote access allowed */ 
                        sprintf(message,"%c%c",S_REPLY,R_HOST);
                        m_sendto(id,message);
                        bailout++;
                        break;
                     case LOCAL:	/* ship file to command */
                        sprintf(temp,"TEMP/%s:%s.%d",
                               client_host,path,getpid());
                        sprintf(message,"rcp %s:%s %s",
                               client_host,path,temp);
                        if (system(message)) {
                           sprintf(message,"%c%c",S_REPLY,R_NET);
                           m_sendto(id,message);
                           bailout++;
                           }
                        break;
                     case REMOTE:	/* run command remotely */
                        sprintf(message,"rsh %s '%s %s'",
                                client_host,command,path);
                        break;
                     }
               else
                  sprintf(message,"%s %s",command,path);
               
               if (bailout)
                  break;

               if (nowindow) {
                  m_ttyreset();
                  dprintf(stderr,"%s Running [%s] no window\n",name,message);
                  code = system(message);
                  m_ttyset();
                  }
               else 
                  code = do_command(message,0,0,font,xpos,ypos,wide,high,wait);
               set_icon(&icons[NORMAL]);
               sprintf(message,"%c%c",S_REPLY,code==0?R_DONE:R_BAD);
               m_sendto(id,message);
               m_clearmode(M_ACTIVATE);
               break;
            case C_WHO:			/* received who-are-you inquiry */
               dprintf(stderr,"%s received query from %d\r\n",name,id);
               sprintf(message,"%c%c %s",S_REPLY,R_WHO,name);
               m_sendto(id,message);
               break;
            case S_HI:			/* Hi, I'm a server */
               dprintf(stderr,"%s Server HI message, ignored\r\n",name);
               alarm(0);
               break;
            case S_BYE:			/* server died */
               dprintf(stderr,"%s Server BYE message, ignored\r\n",name);
               break;
            case S_REPLY:		/* someone thinks I'm a client */
               dprintf(stderr,"%s Oops, I got a server reply message\r\n",name);
               break;
            default:			/* unknown message */
               sprintf(message,"%c%c",S_REPLY,R_UNKWN);
               m_sendto(id,message);
               dprintf(stderr,"%s unknown message\r\n",name);
               break;
            }
         }
      else {

         /* process mgr events */

         dprintf(stderr,"%s Not a message: %s\r\n",name,line);
         switch(*line) {
            case 'M':				/* moved */
              get_size(&wx,&wy,0,0);
              break;
            case 'S':				/* reshape */
              get_size(&wx,&wy,0,0);
              reset_icon();
              break;
            case 'R':				/* redraw */
              reset_icon();
              break;
#ifdef OOPS
            case 'A':				/* window activated */
              if (icon_count > ACTIVE)
                 set_icon(&icons[ACTIVE]);
              break;
            case 'D':				/* window deactivated */
              if (icon_count > ACTIVE)
                 set_icon(&icons[NORMAL]);
              break;
#endif
            }
         }
      m_flush();
      }
   }
   
/* down load an icon */

download_icon(icon,name,where,font)
register struct icon *icon;	/* name of icon to download */
char *name;			/* text string in lieu of icon */
int where;			/* bitmap to download icon to */
int font;			/* char font if no icon */
   {
   int size;
   char line[100];
   int w_in=0,h_in=0;

   /* look only on the local machine */

   dprintf(stderr,"   looking for %s\r\n",name);
   m_bitfromfile(where,name);
   m_flush();
   m_gets(line);
   sscanf(line,"%d %d",&w_in,&h_in);
   dprintf(stderr,"   Found %s (%d x %d)\r\n",name,w_in,h_in);
   icon->w = w_in;
   icon->h = h_in;
   icon->type = (h_in && w_in) ? where : font;
   icon->name = name;
   return(w_in && h_in);
   } 

/* put icon in a window */

set_icon(name)
struct icon *name;		/* name of icon */
   {
   m_push(P_EVENT);
   if (have_icon(name)) {
      m_font(1);
      dprintf(stderr,"Setting icon %s\r\n",name->name);
      m_shapewindow(wx,wy,name->w+2*border,name->h+2*border);
      m_bitcopyto(0,0,name->w,name->h,0,0,0,name->type);
      }
   else {
      dprintf(stderr,"Setting text %s\r\n",name->name);
      m_font(name->type);
      m_size(strlen(name->name),1);
      m_clear();
      m_printstr(name->name);
      }
   m_flush();
   m_clearmode(M_ACTIVATE);
   m_pop();
   icon = name;
   }

reset_icon()
   {
   if (icon)
      set_icon(icon);
   }

/* time out */

int
timeout(n)
int n;
   {
   fprintf(stderr,"Can't send messages, sorry\n");
   clean(n);
   }

/*	Clean up and exit */

clean(n)
int n;
   {
   char message[40];

   sprintf(message,"%c",S_BYE);
   m_broadcast(message);
   m_gets(message);
   m_popall();
   m_clear();
   m_ttyreset();
   exit(n);
   }

/* run  a command in a sub window */

do_command(command,menu,count,font,xpos,ypos,wide,high,wait)
char *command;
struct menu_entry *menu;
int count;
int font,xpos,ypos,wide,high;
int wait;
   {
   int code;		/* return code from command */
   int n;		/* sub-window number */

   n=m_makewindow(xpos,ypos,wide,high);

   if (n==0) {			/* can't make window */
      m_printstr("\007\fCan't open command window, sorry");
      return(-1);
      }

   m_selectwin(n);
   if (count) {
      menu_load(1,count,menu);
      m_selectmenu(1);
      }
   if (font > 0)
      m_font(font);
   m_ttyreset();
   dprintf(stderr,"Running [%s]\n",command);
   code = system(command);
   if (wait) {
      char temp[10];
      m_gets(temp);
      }
   m_ttyset();
   m_selectwin(0);
   m_destroywin(n);
   return(code);
   }
