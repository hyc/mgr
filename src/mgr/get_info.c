/*{{{}}}*/
/*{{{  Notes*/
/*                        Copyright (c) 1987 Bellcore
 *                            All Rights Reserved
 *       Permission is granted to copy or use this program, EXCEPT that it
 *       may not be sold for profit, the copyright notice must be reproduced
 *       on copies, and credit should be given to Bellcore where it is due.
 *       BELLCORE MAKES NO WARRANTY AND ACCEPTS NO LIABILITY FOR THIS PROGRAM.
 */

/* return info to client */
/*}}}  */

/*{{{  #includes*/
#include <mgr/bitblit.h>
#include <mgr/font.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "defs.h"
#include "event.h"
#include "menu.h"

#include "proto.h"
#include "Write.h"
#include "border.h"
#include "font_subs.h"
#include "intersect.h"
/*}}}  */

/*{{{  #defines*/
/*	BUTTON_SYS is never reported as a possible button setting. */
#define	RPT_BUTTON()	(button_state == BUTTON_SYS ? 0 : button_state )
/*}}}  */

/*{{{  get_info*/
void get_info(win,window,text)
register WINDOW *win;			/* window info is about */
BITMAP *window;				/* window's bitmap data */
BITMAP *text;				/* window's text region */
   {
   int cnt = W(esc_cnt);		/* # of leading ESC #'s */
   int count;				/* whatever */
   char coords[1024];			/* space for return value */
char *start = coords;		/* start of reply */
   register WINDOW *win2;		/* generic window pntr */

	if (W(flags)&W_DUPKEY) {
		sprintf(coords,"%c ",W(dup));
		start += strlen(coords);
		}
	else
		*coords = '\0';

   if (cnt == 1) {		/* info about spot */
      for(win2=active;win2!=(WINDOW *) 0;win2=win2->next)
         if (mousein(W(esc)[0],W(esc)[1],win2,1))
            break;
      if (win2 != (WINDOW *) 0)
         sprintf(start,"%s %s %d %d\n",
                  win->tty+strlen(win->tty)-2,
                  win2->tty+strlen(win2->tty)-2,win2->num,win2->pid);
      else
         sprintf(start,"\n");
      Write(W(to_fd),coords,strlen(coords));
      return;
      }
      
   dbgprintf('i',(stderr,"%s: getting info %d\r\n",W(tty),*W(esc)));
   switch (W(esc)[0]) {
      case G_TERMCAP:	/* send termcap entry back to shell */
           {
           int lines = T_HIGH/FSIZE(high);
           int cols  = T_WIDE/FSIZE(wide);

           sprintf(start,"px|mgr|mgr-%d|mgr teminal emulator:%s:li#%d:co#%d:bs:km:cl=^L:ta=^I:ce=\\E%c:cd=\\E%c:cm=\\E%%r%%d%c%%d%c:cs=\\E%%d%c%%d%c:al=\\E%c:dl=\\E%c:AL=\\E%%d%c:DL=\\E%%d%c:ic=\\E%c:dc=\\E%c:IC=\\E%%d%c:DC=\\E%%d%c:up=\\E%c:do=\\E%c:nd=\\E%c:hu=\\E1%c2%c:hd=\\E1%c2%c:ku=\\E[A:kd=\\E[B:kr=\\E[C:kl=\\E[D:so=\\E%c:se=\\E%c:us=\\E4%c:ue=\\E0%c:md=\\E2%c:mr=\\E1%c:me=\\E0%c:RA=\\E%d%c:SA=\\E%d%c:vs=\\E%d%c:vi=\\E%d%c:ve=\\E%c:\n",
                   lines,(W(flags)&W_NOWRAP)?".am":"am",lines,cols,
		   E_CLEAREOL,E_CLEAREOS,
                   E_SEP1,E_CUP,E_SEP1,E_TEXTREGION,
		   E_ADDLINE,E_DELETELINE,E_ADDLINE,E_DELETELINE,
		   E_ADDCHAR,E_DELETECHAR,E_ADDCHAR,E_DELETECHAR,
                   E_UPLINE,E_DOWN,E_RIGHT,
		   E_SEP1,E_UPLINE,E_SEP1,E_DOWN,
		   E_STANDOUT,E_STANDEND,E_STANDEND,E_STANDEND,
		   E_STANDEND,E_STANDEND,E_STANDEND,
		   M_NOWRAP,E_SETMODE,M_NOWRAP,E_CLEARMODE,
                   CS_BLOCK,E_SETCURSOR,CS_INVIS,E_SETCURSOR,E_SETCURSOR);
           }
           break;
      case G_WINSIZE:					/* cols, lines */
           sprintf(start,"%d %d\n", T_WIDE/FSIZE(wide),
              T_HIGH/FSIZE(high));
           break;
      case G_FONT:					/* font wide, high, # */
           {
           register int id = W(font)->ident;
           sprintf(start,"%d %d %d %s\n", FSIZE(wide),
              FSIZE(high),id,id>0?fontlist[id-1]:
              "default");
           }
           break;
      case G_CURSOR:					/* x,y gx,gy cursor_type */
           sprintf(start,"%d %d %d %d %d\n",
              W(x)/FSIZE(wide),
              W(y)/FSIZE(high)-1,
              W(gx), W(gy),
              W(curs_type));
           break;
      case G_TEXT:			/* text region size: x,y,wide,high */
           sprintf(start,"%d %d %d %d\n",
              W(text.x),W(text.y),W(text.wide),W(text.high));
           break;
      case G_MOUSE:					/* mouse coordinates */
           sprintf(start,"%d %d %d\n", mousex, mousey, RPT_BUTTON());
           break;
      case G_MOUSE2:				/* massaged coordinates */
           if (W(flags)&W_ABSCOORDS)
              sprintf(start, "%d %d %d\n", mousex-W(x0), mousey-W(y0), 
                    RPT_BUTTON());
           else
              sprintf(start,"%ld %ld %d\n",
                    (mousex-W(x0))*GMAX/BIT_WIDE(W(window)),
                    (mousey-W(y0))*GMAX/BIT_HIGH(W(window)),
                    RPT_BUTTON());
           break;
      case G_COORDS:					/* window coords */
           sprintf(start,"%d %d %d %d\n",W(x0),W(y0),
              WIDE,HIGH);
           break;
      case G_STATUS:					/* window status */
           sprintf(start,"%c\n",W(flags)&W_ACTIVE
                   ? C_EXPOSED: C_OBSCURED);
           if (win == active)
              *start = C_ACTIVE;
           break;
      case G_ALLMINE:			/* window status for client windows */
      case G_ALL:				/* complete window status */
           {
           register char status;
           for(win2=active;win2!=(WINDOW *) 0;win2=win2->next) {
              if (*W(esc)==G_ALLMINE && win2->main != W(main))
                 continue;
              status = win2->flags&W_ACTIVE ? C_EXPOSED:C_OBSCURED;
              if (win2 == win)
                 status += ('A'-'a');

              sprintf(start+strlen(start),"%d %d %d %d %s %d %c %d\n",
                      win2->x0,
                      win2->y0,
                      BIT_WIDE(win2->border),
                      BIT_HIGH(win2->border),
                      win2->tty+strlen(win2->tty)-2,
                      win2->num,
                      status,
                      win2->setid
                      );
               Write(W(to_fd),coords,strlen(coords));
               *coords = '\0';	/* output already processed */
					start = coords;
               }
           }
           break;
      case G_NOTIFY:			/* list windows with notify set  */
           {
           register char *str;
           for(win2=active;win2!=(WINDOW *) 0;win2=win2->next) {
              dbgprintf('i',(stderr,"  checking %s\r\n",win2->tty));
              if (IS_EVENT(win2,EVENT_NOTIFY) &&
                    (str = win2->events[GET_EVENT(EVENT_NOTIFY)])) {

                 sprintf(start,"%d.%d %d %s\n",
                          win2->pid,win2->num,(int)strlen(str),str);
                 Write(W(to_fd),coords,strlen(coords));
                 dbgprintf('i',(stderr,"    got %s\r\n",str));
                 *coords = '\0';	/* output already processed */
                 } 
              if (*coords)
                 strcat(coords,"\n");
              }

           }
           break;
      case G_SYSTEM:					/* system status */
           strcpy(start,"localhost");
           gethostname(start,sizeof(coords));
           sprintf(start+strlen(start)," %d %d %d %d\n",
                      BIT_WIDE(screen),
                      BIT_HIGH(screen),
                      SUM_BDR,
                      BIT_DEPTH(screen));
           break;
      case G_ID:					/* client window id */
           for(count=0,win2=W(main);win2;win2=win2->alt)
              count++;
           sprintf(start,"%d %d\n",W(num),count);
           break;
      case G_FLAGS:					/* window flags */
           sprintf(coords,"%0lx %d %d %d %d\n",
						W(flags),
						GETFCOLOR(W(style)),
						GETBCOLOR(W(style)),
						GETFCOLOR(W(op)),
						GETBCOLOR(W(op)));

           break;
#ifdef OBSOLETE
      case G_ALLFONT:					/* font information */
           {
           register struct font *temp;
           register int i;
 
           sprintf(start,"%d %d",font->head.wide,
                                 font->head.high);
           for(i=0;i<MAXFONT;i++)  {
              temp=Get_font(i);
              sprintf(start+strlen(start), "  %d %d",
                            temp->head.wide,
                            temp->head.high);
              }
           strcat(start,"\n");
           }
           break;
#endif
      default:								/* invalid request, send back a \n */
           strcat(start,"\n");
           break;
      }

   if (strlen(coords))
      Write(W(to_fd),coords,strlen(coords));
	else
      Write(W(to_fd),"\n",1);
   dbgprintf('i',(stderr,"  sending (%d) [%s]\r\n",strlen(coords),coords));
   }
/*}}}  */
