/*{{{}}}*/
/*{{{  Notes*/
/* read and process startup file */ /* this needs to be redone */
/*}}}  */
/*{{{  #includes*/
#include <mgr/bitblit.h>
#include <mgr/font.h>
#include <mgr/share.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include "clip.h"
#include "defs.h"

#include "proto.h"
#include "startup.h"
#include "Write.h"
#include "do_buckey.h"
#include "do_menu.h"
#include "erase_win.h"
#include "font_subs.h"
#include "get_font.h"
#include "mouse_get.h"
#include "new_window.h"
#include "put_window.h"
#include "subs.h"
/*}}}  */
/*{{{  #defines*/
#define MAXLINE		512
#define MAXFIELDS	100
/*}}}  */

/*{{{  variables*/
static int newx = -1, newy = -1, newdx, newdy, newfont, newflags;
static char newinit[MAXLINE];
static char *newshell[MAXFIELDS];
static char newstart[MAXLINE];
static char *initcmd, *suspendcmd, *resumecmd, *quitcmd;
/*}}}  */

/*{{{  save_fields*/
/*	Given a character pointer and a pointer to an array of strings,
	catenate the strings into fresh memory and put the address of the copy
	into the character pointer.
	free() any space originally pointed to by the character pointer.
*/
static void save_fields( cpp, fields )
char	**cpp;
char	*fields[];
{
	register int	i;
	char		savestr[MAXLINE];

	if( *cpp )
		free( *cpp );
	*savestr = '\0';
	for( i=1;  fields[i]!=(char *)0;  i++ ) {
		if( i > 1 )
			strcat(savestr, " ");
		strcat(savestr, fields[i]);
	}
	(void) trans(savestr);
	*cpp = strcpy(malloc(strlen(savestr)+1),savestr);
}
/*}}}  */
/*{{{  lastchar*/
static
char
lastchar( cp )
char *cp;
{
   int length = strlen(cp);
   if( !length )
      return '\0';
   return  cp[ length-1 ];
}
/*}}}  */
/*{{{  dowindow*/
#ifndef DEFAULT_LINES
#define DEFAULT_LINES 24
#endif

static void
dowindow( x, y, dx, dy, font, shell, flags, init, start )
int	x, y, dx, dy, font;
char	*shell[];
int	flags;
unsigned char *init, *start;
{
    struct font *fp = Get_font( font );
    int i, mouse_was_on;

    dbgprintf('S',(stderr,"starting shell %s\r\n",shell? *shell: "???"));
    if( dx <= 0 )
      dx = 80*fp->head.wide + 2*SUM_BDR;
    if( dy <= 0 )
      dy = DEFAULT_LINES*fp->head.high + 2*SUM_BDR;
    if( dx > BIT_WIDE(screen))
      dx = BIT_WIDE(screen);
    if( dy > BIT_HIGH(screen))
      dy = BIT_HIGH(screen);
    /*
     * Adjust default position depending on screen size and window number.
     * Leaves a bit of room on bottom for closed windows.
     * Ensures that window will fit on screen.
     */
    {
       int tile_pos = (BIT_WIDE(screen) - dx) / 16;
       if ((BIT_HIGH(screen) - dy) / 16 < tile_pos)
          tile_pos = (BIT_HIGH(screen) - dy) / 18;
       tile_pos = next_windowset_id() % (tile_pos + 1);
       if( x < 0 )
          x = 16 * tile_pos;
       if( y < 0 )
          y = 16 * tile_pos + next_windowset_id();
    }
    mouse_was_on = mouse_on;
    MOUSE_OFF(screen,mousex,mousey);
    create_window(x, y, dx, dy, font, *shell?shell:0);
    if( mouse_was_on)  MOUSE_ON(screen,mousex,mousey);

    if (flags)
       ACTIVE(flags) |= flags;
    if (*init)
       put_window(active,init,strlen(init));
    if (*start) {
       i = Write(ACTIVE(to_fd),start,strlen(start));
       dbgprintf('S',(stderr,"%s: start string %d/%d %s\r\n",
		     ACTIVE(tty),i,strlen(start),start));
       }
}
/*}}}  */

/*{{{  startup -- do startup file*/
void startup(name) char *name;
   {
   FILE *file = fopen(name,"r");		/* file to read commands from */
   int x=0 ,y=0 ,dx=0 ,dy=0 ,fnt=0 ;		/* window coords, font # */
   char line[MAXLINE];				/* space to store stuff */
   char start[MAXLINE];				/*  "  */
   char init[MAXLINE];				/*  "  */
   char *shell[MAXFIELDS];			/*  "  */
   char *fields[MAXFIELDS];			/*  "  */
   int curr_font = 0;				/* font # for this window */
   int flags = 0;				/* window flags */
   register int i;
   int count,  gotwindow = 0, gotnewwindow = 0;
   int setactive = 0, activesetid = 0;
   int windowsetid = 0;

   (void) memset(start,0,MAXLINE);
   (void) memset(line,0,MAXLINE);
   (void) memset(init,0,MAXLINE);
   (void) memset(shell,0,MAXFIELDS*sizeof(shell[0]));
   (void) memset(fields,0,MAXFIELDS*sizeof(fields[0]));

   if (file == NULL) return;


   /* process each line in startup file */

   while (fgets(line,sizeof(line),file) != NULL) {
      dbgprintf('S',(stderr,"*** got: %s \r\n",line));
      count = parse(line,fields);

      if (*fields == (char *) 0)
         continue;

      else if (strcmp(*fields,"window")==0 || strcmp(*fields,"done")==0) {

         if (gotnewwindow==1) {
	    gotnewwindow = 0;
	    newx = x;
	    newy = y;
	    newdx = dx;
	    newdy = dy;
	    newfont = curr_font;
	    strcpy(newstart, start);
	    strcpy(newinit, init);
	    for( i = 0; shell[i]; i++ ) {
	       if( newshell[i] )
		  free( newshell[i] );
	       newshell[i] = shell[i];
	       shell[i] = (char*)0;
	       }
            gotwindow = 0;
	    newshell[i] = (char*)0;
            gotwindow = 0;
            (void) memset(start,0,MAXLINE);
            (void) memset(init,0,MAXLINE);
            curr_font = 0;
            flags = 0;
	    windowsetid = 0;
	    }
      /* got a window */

         if (gotwindow==1) {
            if (check_window(x,y,dx,dy,curr_font) == 0) {
               gotwindow = 0;
               (void) memset(start,0,MAXLINE);
               (void) memset(init,0,MAXLINE);
               curr_font = 0;
               flags = 0;
	       windowsetid = 0;
               continue;
               }

	    if( !active ) {
	       MOUSE_OFF(screen,mousex,mousey);
               erase_win(screen);
	       MOUSE_ON(screen,mousex,mousey);
	    }
#ifdef MOVIE
		 log_time();
#endif
       dowindow( x, y, dx, dy, curr_font, shell, flags, init, start );

	    if( windowsetid ) {
	       ACTIVE(setid) = windowsetid;
	       windowsetid = 0;
	    }

	    if( setactive  &&  activesetid == 0 )
	       activesetid = ACTIVE(setid);

            if( setactive )
	       topwin( activesetid, -1 );
	    if( *shell )
	       free( *shell );
            *shell = '\0';
            flags = 0;
            (void) memset(start,0,MAXLINE);
            (void) memset(init,0,MAXLINE);
            curr_font = 0;
            }

         if (strcmp(*fields,"window")==0 && count >=5) {
	    char lastchar();
	    struct font *fp;

            gotwindow = 1;
            x = atoi(fields[1]);
            y = atoi(fields[2]);
            dx = atoi(fields[3]);
            dy = atoi(fields[4]);
            if (count <= 5 || (fnt=atoi(fields[5])) > MAXFONT || fnt < 0)
	       fnt = 0;
	    fp = Get_font( fnt );
	    if( lastchar( fields[3] ) == 'c' )
	       dx = dx*fp->head.wide + 2*SUM_BDR;
	    if( lastchar( fields[4] ) == 'c' )
	       dy = dy*fp->head.high + 2*SUM_BDR;
	    curr_font = fnt;
            }
         else
            gotwindow = 0;
         }

      else if (strcmp(*fields,"initcmd")==0 && count >1) {
	    save_fields( &initcmd, fields );
	    do_cmd( 'i' );
	 }
      else if (strcmp(*fields,"suspendcmd")==0 && count >1) {
	    save_fields( &suspendcmd, fields );
	 }
      else if (strcmp(*fields,"resumecmd")==0 && count >1) {
	    save_fields( &resumecmd, fields );
	 }
      else if (strcmp(*fields,"quitcmd")==0 && count >1) {
	    save_fields( &quitcmd, fields );
	 }
      else if (strcmp(*fields,"font")==0 && count >2) {
         fnt = atoi(fields[1]);
         dbgprintf('S',(stderr,"got font %s (%d)\r\n",fields[2],fnt));
         if (fnt > 0  &&  fnt <= MAXFONT) {
            if (fontlist[fnt-1])
               free(fontlist[fnt-1]);
            fontlist[fnt-1] = strcpy(malloc(strlen(fields[2])+1),fields[2]);
            }
         }

      else if (strcmp(*fields,"colors")==0 && count >1) {
         for(i=0;i<count-1 && i < COLORMAP_SIZE-4;i++)
            color_map[i+4] = atoi(fields[i+1]);
         dbgprintf('S',(stderr,"got default colors\r\n"));
         }

      else if (strcmp(*fields,"map")==0 && count >8) {
         for(i=0;i<8;i++)
            map_mouse(i,atoi(fields[i+1]));
         dbgprintf('S',(stderr,"got mouse map \r\n"));
         }

      else if (strcmp(*fields,"slide")==0 && count > 2) {
         set_slide(atoi(fields[1]),atoi(fields[2]));
         dbgprintf('S',(stderr,"set menu slide %d,%d\r\n",
		       atoi(fields[1]),atoi(fields[2])));
         }

      else if (strcmp(*fields,"page")==0 && count > 2) {
         set_page(atoi(fields[1]),atoi(fields[2]));
         dbgprintf('S',(stderr,"set menu page %d,%d\r\n",
		       atoi(fields[1]),atoi(fields[2])));
         }

      else if (strcmp(*fields,"active")==0 && gotwindow==1) {
	 setactive = 1;
	 activesetid = 0;
      }
      else if (strcmp(*fields,"windowsetid")==0 && count>1 && gotwindow==1) {
	 windowsetid = atoi( fields[1] );
      }
      else if (strcmp(*fields,"newwindow")==0 && gotwindow==1) {
	 gotnewwindow = 1;
      }
      else if (strcmp(*fields,"shell")==0 && count>1 && gotwindow==1) {
         for(i=0;i<count-1;i++) {
            if (shell[i])
               free(shell[i]);
            shell[i] = strcpy(malloc(strlen(fields[i+1])+1),fields[i+1]);
            }
         if (shell[count-1]) {
            free (shell[count-1]);
            shell[count-1] = (char *) 0;
            }
         }
      else if (strcmp(*fields,"init")==0) {
         for(i=1;fields[i]!=(char *)0;i++) {
	    if( i > 1 )
	       strcat(init," ");
            strcat(init,fields[i]);
	    }
         (void) trans(init);
         }

      else if (strcmp(*fields,"start")==0) {
         for(i=1;fields[i]!=(char *)0;i++) {
	    if( i > 1 )
	       strcat(start," ");
            strcat(start,fields[i]);
	    }
         (void) trans(start);
         }

      else if (strcmp(*fields,"flags")==0 && count > 1 && gotwindow==1) {
         for(i=1;i<count;i++) {
            if (strcmp(fields[i],"expose")==0)
               flags |= W_EXPOSE;
            else if (strcmp(fields[i],"background")==0)
               flags |= W_BACKGROUND;
            else if (strcmp(fields[i],"nokill")==0)
               flags |= W_NOKILL;
            dbgprintf('S',(stderr,"Flags: %d (%s)\r\n",flags,fields[i]));
            }
         }

      else {
         dbgprintf('S',(stderr,"invalid line: %s\r\n",line));
         }
      }
   fclose(file);
   }
/*}}}  */
/*{{{  do_cmd*/
void do_cmd(char flag)
{
	switch( flag ) {
	case 'i':
		systemcmd( initcmd );
		break;
	case 's':
		systemcmd( suspendcmd );
		break;
	case 'r':
		systemcmd( resumecmd );
		break;
	case 'q':
		systemcmd( quitcmd );
		break;
	}
}
/*}}}  */
/*{{{  initwindow*/
void initwindow(void)
{
    dowindow( newx, newy, newdx, newdy, newfont, newshell, newflags,
	newinit, newstart );
}
/*}}}  */
