/*
From: anthony@unix.cis.pitt.edu (M. Kapolka)
Newsgroups: unix-pc.sources
Subject: eyes, small MGR application
Keywords: It's a start.
Message-ID: <24223@unix.cis.pitt.edu>
Date: 11 May 90 17:50:29 GMT
Reply-To: anthony@cs.pitt.edu (Anthony Kapolka)
Organization: Univ. of Pittsburgh Computer Science
 */
/* eyes, a quick approximation of xeyes/eyecon-like software under MGR

   save this file as eyes.c, cut off the header and
   compile via:	cc -o eyes eyes.c -lmgr

   I wrote this to get some idea how involved writing simple MGR 
   applications is.  It isn't very.

   eyes looks best in a small window, and reflects the mouse position
   at the screen.  Since the 3b1 screen is small, at best it is only
   a novelty.  The pupil positioning algorithm is rather ad hoc and
   should be improved. 

   I'm not proud.  Feel free to clean this up and repost it.

   Anthony Kapolka - anthony@cs.pitt.edu 
*/


#include <signal.h>
#include <mgr/mgr.h>

#define TIME	2	/* How often to update eyes, in seconds */

static char line[80];
int lastx, lasty;

main()
{
   int clean(), timer();


   m_setup(0);
   m_push(P_ALL & (~P_MOUSE));	
   m_ttyset();
   m_clear();
 
   update();
   		
   signal(SIGINT,clean);
   signal(SIGTERM,clean);
   signal(SIGALRM,timer);
   alarm(TIME);

   m_setevent(ACTIVATE, "A\r");
   m_setevent(REDRAW, "R\r");
   m_setevent(RESHAPE, "R\r");


   while(1) {
   while (m_gets (line) != NULL)
	switch (*line) {
	  case 'R':
		update();
		break;
	  case 'A':
		update();
		break;
	  default:
		break;
	}
   }

}

timer()
{
update();
signal(SIGALRM,timer);
alarm(TIME);
}

/* This draws pupil position relative to mouse position.  It would 
   be better to get the eyes to look at the mouse position.		*/

update()
{
int i;
int rx, ry, lx, ly;
int mx, my;	     	/* display coords of mouse */
int wx, wy, ww, wh;  	/* display coords of eye window */
int centerx, centery;	/* center of eyes */

  m_getinfo(G_MOUSE);
  m_flush();
  m_gets(line);
  sscanf(line,"%d %d",&mx,&my);

  if ((mx != lastx) || (my != lasty)) {   /* Don't redisplay if no change */

     lastx = mx; lasty = my;

     m_clear();
     m_ellipse(250,500,200,400);
     m_ellipse(750,500,200,400);

     m_getinfo(G_COORDS); 
     m_flush();
     m_gets(line);
     sscanf(line,"%d %d %d %d",&wx,&wy,&ww,&wh);

     centerx = wx + (ww / 4);
     centery = wy + (wh / 2);

     rx = (int)((centerx - mx) * (100.0/720.0));  /* numbers are arbitrary */
     ry = (int)((centery - my) * (300.0/348.0));

     centerx = wx + (3 * (ww / 4));

     lx = (int)((centerx - mx) * (100.0/720.0));
     ly = ry;

     for (i=1;i<60;i+=5)		/* no fill command, eh? */
       {m_circle(250-lx,500-ly,i);
        m_circle(750-rx,500-ry,i);};
  }
  return(1);
}

clean()
{
  m_ttyreset();
  m_popall();
  exit(0);
}
