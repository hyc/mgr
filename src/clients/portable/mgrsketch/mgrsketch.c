/* mgrsketch -- draw a picture on the screen with mgr windowing system running.
 * David J. Raymond, Physics Department
 * New Mexico Institute of Mining and Technology
 * Socorro, NM 87801
 * raymond@kestrel.nmt.edu
 * $Id: mgrsketch.c,v 1.4 1993/04/24 14:08:55 dave Exp dave $
 */
#include <mgr/mgr.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>

/* default window dimensions */
#define X0 0
#define Y0 0
#define DX 600
#define DY 400
#define BORDER 12

/* offset to tip of cursor arrow */
#define OFFSET 5

/* text offsets */
#define XTOFF 4
#define YTOFF 4

/* paintbrush size */
#define PSIZE 12

/* maximum number of mgr fonts loadable */
#define MAXNFONTS 15

/* maximum length of line */
#define LINESIZE 256

/* define states */
#define LINE 1
#define ERASE 2
#define TEXT 3
#define XLINE 5
#define YLINE 6
#define BOX 7
#define PAINT 8
#define CIRCLE 9

#define MENU_COUNT (sizeof(menu)/sizeof(struct menu_entry))

#define NCHORDS 36
#define RADIAN 57.2958

char buffer[LINESIZE + 1];			/* buffer for input from mgr server */
char cline[LINESIZE + 1];			/* a line buffer */
char fname[LINESIZE + 1];			/* file name */
char fpath[LINESIZE + 1];			/* pathname */
char fpwd[LINESIZE + 1];			/* current working directory */
int mdown = 0;					/* state of mouse */
int state = LINE;				/* state of system */
int x = 0,y = 0;				/* current cursor position */
int oldx = 0,oldy = 0;				/* previous cursor position */
int xx1,yy1,xx2,yy2;				/* arrowhead points */
int nx,ny,nd;					/* size of loaded bitmap */
int xbrush = 1;					/* paintbrush size */
int xarrow = 0;					/* arrow size */
int cfont = 0;					/* current font */
float fdx,fdy,dangle,angle,radius;		/* stuff for circle draw */
int loop,dx,dy,odx,ody;				/* more stuff for circle draw */
struct menu_entry menu[] = {
	{"line","line\n",},
	{"x-line","xline\n",},
	{"y-line","yline\n",},
	{"box","box\n",},
	{"arrowhead","arrow\n",},
	{"circle","circle\n",},
	{"paint","paint\n",},
	{"brushsize","brushsize\n",},
	{"text","text\r",},
	{"new font","newfont\n",},
	{"erase","erase\n",},
	{"load file","load\n",},
	{"save file","save\n",},
	{"quit","quit\n",},
};
FILE *fileptr;					/* pointer to load and save files */
void cleanup();					/* clean up after interrupt */
char *dbox();					/* dialog box */
void fnclean();
void fatline();
void arrowline();
void move();

int
main()
{

/* set things up */
	m_setup(M_FLUSH);
	m_push(P_POSITION|P_MENU|P_EVENT);
	menu_load(1,MENU_COUNT,menu);
	m_selectmenu(1);
	m_clear();
	m_shapewindow(X0,Y0,DX + BORDER,DY + BORDER);
	m_setmode(M_ABS);
	m_setevent(BUTTON_1,"rbutdown\n");
	m_setevent(BUTTON_1U,"rbutup\n");
	m_setevent(DESTROY,"destroy\n");
	m_setnoecho();
	m_setcursor(CS_INVIS);
	signal(SIGINT,cleanup);
	m_func(BIT_CLR);
	m_bitwriteto(0,0,PSIZE,PSIZE,ERASE);		/* eraser bitblit */
	m_func(BIT_SET);
	m_bitwriteto(0,0,PSIZE,PSIZE,PAINT);		/* paint bitblit */
	fname[0] = '\0';
	strcpy(fpwd,getenv("PWD"));

/* go into loop and await events and menu entries */
	while (1) {
		m_gets(buffer);

/* quit signal */
		if (strcmp(buffer,"quit\n") == 0) {
			cleanup();
		}

/* line signals */
		else if (strcmp(buffer,"line\n") == 0) {
			state = LINE;
		}
		else if (strcmp(buffer,"xline\n") == 0) {
			state = XLINE;
		}
		else if (strcmp(buffer,"yline\n") == 0) {
			state = YLINE;
		}
		else if (strcmp(buffer,"box\n") == 0) {
			state = BOX;
		}

		else if (strcmp(buffer,"arrow\n") == 0) {
			sscanf(dbox(100,100,40,2,"Type size of arrow in pixels: "),"%d",&xarrow);
		}

		else if (strcmp(buffer,"circle\n") == 0) {
			state = CIRCLE;
		}

/* erase signal */
		else if (strcmp(buffer,"erase\n") == 0) {
			state = ERASE;
		}

/* paint signal */
		else if (strcmp(buffer,"paint\n") == 0) {
			state = PAINT;
		}

/* brushsize signal */
		else if (strcmp(buffer,"brushsize\n") == 0) {
			sscanf(dbox(100,100,40,2,"Type size of brush in pixels: "),"%d",&xbrush);
			if (xbrush < 1) xbrush = 1;
			if (xbrush > PSIZE) xbrush = PSIZE;
		}

/* text signal */
		else if (strcmp(buffer,"text\n") == 0) {
			state = TEXT;
		}

/* new font signal */
		else if (strcmp(buffer,"newfont\n") == 0) {
			sscanf(dbox(100,100,35,2,"Type new font number: "),"%d",&cfont);
			if (cfont < 0 || cfont >= MAXNFONTS) cfont = 0;
			m_font(cfont);
		}

/* load a file */
		else if (strcmp(buffer,"load\n") == 0) {
			strcpy(fname,dbox(100,100,60,2,"Type file to be loaded: "));
			fnclean(fpwd,fname,fpath);
			if (m_bitfile(1,fpath,&nx,&ny,&nd) == 0) {
				dbox(100,100,60,2,"Can't find it! (Type return to continue) ");
			}
			else {
				m_func(BIT_SRC);
				m_bitcopyto(0,0,nx,ny,0,0,0,1);
			}
		}

/* save a file */
		else if (strcmp(buffer,"save\n") == 0) {
			if (strcmp(fname,"") != 0) {
				sprintf(cline,"File name %s ok? (y/n): ",fname);
				strcpy(buffer,dbox(100,100,60,2,cline));
				if (buffer[0] != 'y') {
					strcpy(buffer,dbox(100,100,60,2,"Type file name: "));
					sscanf(buffer,"%s\n",fname);
				}
				fnclean("/tmp",fname,fpath);
				m_windowsave(fpath);
				move(fpath,fname);
			}
			else {
				strcpy(fname,dbox(100,100,60,2,"Type file name: "));
				fnclean("/tmp",fname,fpath);
				m_windowsave(fpath);
				move(fpath,fname);
			}
/*
			sprintf(cline,"Saved as %s.	Type return to continue: ",fname);
			dbox(100,100,60,2,cline);
*/
		}

/* right button goes down */
		else if (strcmp(buffer,"rbutdown\n") == 0) {

/* set initial line point */
			if (state == LINE || state == XLINE || state == YLINE ||
					state == BOX || state == ERASE || state == CIRCLE) {
				get_mouse(&oldx,&oldy);
				oldx -= OFFSET + xbrush/2;
				oldy -= OFFSET + xbrush/2;
			}

/* paint or erase */
			if (state == PAINT || state == ERASE) {
				m_func(BIT_SRC);
				x = y = -1;
				while (1) {
					oldx = x;
					oldy = y;
					m_getinfo(G_MOUSE2);
					m_gets(buffer);
					if (strcmp(buffer,"rbutup\n") == 0) break;
					sscanf(buffer,"%d %d",&x,&y);
					x -= OFFSET + xbrush/2;
					y -= OFFSET + xbrush/2;
					if (oldx != -1 && oldy != -1 && (x != oldx || y != oldy)) {
						fatline(oldx,oldy,x,y,xbrush,state);
					}
				}
			}
		}

/* right button comes up */
		else if (strcmp(buffer,"rbutup\n") == 0) {

/* draw a line */
			if (state == LINE || state == XLINE || state == YLINE ||
					state == BOX || state == CIRCLE) {
				get_mouse(&x,&y);
				x -= OFFSET + xbrush/2;
				y -= OFFSET + xbrush/2;
				m_func(BIT_SET);
				if (state == LINE) {
					arrowline(oldx,oldy,x,y,xbrush,state,xarrow);
				}
				else if (state == XLINE) {
					arrowline(oldx,oldy,x,oldy,xbrush,state,xarrow);
				}
				else if (state == YLINE) {
					arrowline(oldx,oldy,oldx,y,xbrush,state,xarrow);
				}
				else if (state == BOX) {
					fatline(oldx,oldy,x,oldy,xbrush,state);
					fatline(x,oldy,x,y,xbrush,state);
					fatline(x,y,oldx,y,xbrush,state);
					fatline(oldx,y,oldx,oldy,xbrush,state);
				}
				else if (state == CIRCLE) {
					fdx = x - oldx;
					fdy = y - oldy;
					radius = sqrt(fdx*fdx + fdy*fdy);
					dx = radius;
					dy = 0;
					for (loop = 1; loop <= NCHORDS; loop++) {
						odx = dx;
						ody = dy;
						dangle = 360./NCHORDS;
						angle = loop*(dangle/RADIAN);
						dx = radius*cos(angle);
						dy = radius*sin(angle);
						fatline(oldx + odx,oldy + ody,oldx + dx,oldy + dy,xbrush,state);
					}
				}
			}

/* write some text */
			else if (state == TEXT) {
				get_mouse(&x,&y);
				m_movecursor(x - XTOFF,y - YTOFF);
				m_setcursor(CS_BLOCK);
				m_setecho();
				m_func(BIT_SET);
				fgets(buffer,LINESIZE,stdin);
				m_setcursor(CS_INVIS);
				m_setnoecho();
			}
		}
	}
}

/* dialog box */
char *dbox(x,y,dx,dy,query)
int x,y;			/* position of box */
int dx,dy;			/* size of box */
char *query;
{
char barf[40];
int window;
int fontx,fonty;
int winlocx,winlocy,winsizex,winsizey;

	m_font(0);
        m_getfontsize(&fontx,&fonty);
        m_getwindowposition(&winlocx,&winlocy);
        m_getwindowsize(&winsizex,&winsizey);
	window = m_makewindow(winlocx + x,winlocy + y,dx*fontx,dy*fonty);
	m_selectwin(window);
	m_setecho();
	printf("%s",query);
	m_gets(buffer);
	m_setnoecho();
	m_destroywin(window);
	m_gets(barf);
	m_font(cfont);
	return(buffer);
}

/* clean up file name and add path if needed */
void
fnclean(pwd,name,path)
char *pwd,*name,*path;
{
int i;

	for (i = 0; name[i] != '\0'; i++) {
		if (name[i] == '\n') {
			name[i] = '\0';
			break;
		}
	}
	if (name[0] != '/') sprintf(path,"%s/%s",pwd,name);
	else strcpy(path,name);
}

/* make a line with an arrow */
void
arrowline(oldx,oldy,x,y,xbrush,state,xarrow)
int oldx,oldy;
int x,y;
int xbrush;
int state;
int xarrow;
{
int nx,ny,px,py,len;

	fatline(oldx,oldy,x,y,xbrush,state);
	if (xarrow > 0) {
		nx = oldx - x;
		ny = oldy - y;
		len = abs(nx) + abs(ny);
		if (len > 0) {
			nx *= xarrow;
			ny *= xarrow;
			nx /= len;
			ny /= len;
			px = nx - ny/2 + x;
			py = nx/2 + ny + y;
			fatline(x,y,px,py,xbrush,state);
			px = nx + ny/2 + x;
			py = -nx/2 + ny + y;
			fatline(x,y,px,py,xbrush,state);
		}
	}
}

/* make a fat line using bit blit call -- otherwise use line call */
void
fatline(oldx,oldy,x,y,xbrush,state)
int oldx,oldy;
int x,y;
int xbrush;
int state;
{
int i,j,k;
float fract;

	if (xbrush == 1) {
		m_line(oldx,oldy,x,y);
	}
	else {
		if (abs(x - oldx) > abs(y - oldy)) {
			j = x > oldx ? 1 : -1;
			for (i = oldx; i != x; i += j) {
				if (x == oldx) fract = 0.;
				else fract = (float)(i - oldx)/(float)(x - oldx);
				if (y == oldy) k = y;
				else k = (fract*y + (1. - fract)*oldy) + .5;
				m_bitcopyto(i,k,xbrush,xbrush,0,0,0,state);
			}
		}
		else {
			j = y > oldy ? 1 : -1;
			for (i = oldy; i != y; i += j) {
				if (y == oldy) fract = 0.;
				else fract = (float)(i - oldy)/(float)(y - oldy);
				if (x == oldx) k = x;
				else k = (fract*x + (1. - fract)*oldx) + .5;
				m_bitcopyto(k,i,xbrush,xbrush,0,0,0,state);
			}
		}
	}
}

/* cleanup routine */
void cleanup()
{
	m_clearmenu(1);
	m_font(0);
	m_clear();
	m_pop();
	m_clearmode(M_ABS);
	m_setecho();
	m_setcursor(CS_BLOCK);
	exit(0);
}

/* move -- move a file from one place to another */
void
move(src,dst)
char *src,*dst;
{
	char tstr[512];
	if (strcmp(src,dst) != 0) {
		sprintf(tstr,"sleep 2;mv %s %s",src,dst);
		system(tstr);
	}
}
