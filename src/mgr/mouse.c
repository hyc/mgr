/* simple driver for serial mouse */
/* Andrew Haylett, 14th December 1992 */

#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#ifdef sun
#define u_char unsigned char
#define u_short unsigned short
#include <sys/time.h>
#include <sundev/vuid_event.h>
#endif

#include <string.h>

#include "proto.h"
#include "mouse.h"

/* these settings may be altered by the user */
#ifdef MOUSE
static int mtype = MOUSE; 
#else
static int mtype = 0;
#endif
static const int mbaud = B1200;		/* 1200 baud default */
static const int msample = 100;		/* sample rate for Logitech mice */
static const int mdelta = 25;		/* x+y movements more than 25 pixels..*/
static const int maccel = 2;		/* ..are multiplied by 2. */

#define limit(n,l,u)	n = ((n) < (l) ? (l) : ((n) > (u) ? (u) : (n)))
#define abs(x)		(((x) < 0) ? -(x) : (x))

static int mx = 640;  
static int my = 480;  
static int x = 32, y = 32;
static int buttonwas = 0;
int mfd = -1;

static const unsigned short cflag[7] =
{
      (CS7                   | CREAD | CLOCAL | HUPCL ),   /* MicroSoft */
      (CS8 | CSTOPB          | CREAD | CLOCAL | HUPCL ),   /* MouseSystems 3 */
      (CS8 | CSTOPB          | CREAD | CLOCAL | HUPCL ),   /* MouseSystems 5 */
      (CS8 | PARENB | PARODD | CREAD | CLOCAL | HUPCL ),   /* MMSeries */
      (CS8 | CSTOPB          | CREAD | CLOCAL | HUPCL ),   /* Logitech */
      0,						   /* PS/2 */
      0                                                    /* BusMouse */
};

static const unsigned char proto[7][5] =
{
    /*  hd_mask hd_id   dp_mask dp_id   nobytes */
    { 	0x40,	0x40,	0x40,	0x00,	3 	},  /* MicroSoft */
    {	0xf8,	0x80,	0x00,	0x00,	3	},  /* MouseSystems 3 */
    {	0xf8,	0x80,	0x00,	0x00,	5	},  /* MouseSystems 5 */
    {	0xe0,	0x80,	0x80,	0x00,	3	},  /* MMSeries */
    {	0xe0,	0x80,	0x80,	0x00,	3	},  /* Logitech */
    {	0xc0,	0x00,	0x00,	0x00,	3	},  /* PS/2 */
    {	0xf8,	0x80,	0x00,	0x00,	5	}   /* BusMouse */
};

static void
ms_setspeed(const int old, const int new,
            const unsigned short c_cflag)
{
    struct termios tty;
    char *c;

    tcgetattr(mfd, &tty);
    
    tty.c_iflag = IGNBRK | IGNPAR;
    tty.c_oflag = 0;
    tty.c_lflag = 0;
    tty.c_cc[VTIME] = 0;
    tty.c_cc[VMIN] = 1;
    tty.c_cflag = c_cflag;
#ifndef __FreeBSD__
    tty.c_line = 0;
#endif

    cfsetispeed (&tty, old);
    cfsetospeed (&tty, old);
    tcsetattr(mfd, TCSAFLUSH, &tty);

    switch (new)
    {
    	case 9600: c = "*q"; break;
    	case 4800: c = "*p"; break;
    	case 2400: c = "*o"; break;
    	case 1200:
	default:   c = "*n"; break;
    }

    write(mfd, c, 2);
    usleep(100000);

    cfsetispeed (&tty, new);
    cfsetospeed (&tty, new);
    tcsetattr(mfd, TCSAFLUSH, &tty);
#   ifdef TIOCEXCL
    ioctl(mfd,TIOCEXCL,0);
#   endif
#   ifdef VUIDSFORMAT
    /* SunOS: set normal mode because X (sometimes?) leaves it broken */
    ioctl(mfd,VUIDSFORMAT,0);
#   endif
}

static int init = 0;

void
ms_init(int screen_w, int screen_h, char *mouse_type)
{

    mx = screen_w;
    my = screen_h;

    if (mouse_type) {
	if      (strcasecmp(mouse_type,"microsoft")==0)      mtype = 0;
	else if (strcasecmp(mouse_type,"mousesystems3")==0)  mtype = 1;
	else if (strcasecmp(mouse_type,"mousesystems5")==0)  mtype = 2;
	else if (strcasecmp(mouse_type,"mmseries")==0)       mtype = 3;
	else if (strcasecmp(mouse_type,"logitech")==0)       mtype = 4;
	else if (strcasecmp(mouse_type,"ps2")==0)            mtype = 5;
	else if (strcasecmp(mouse_type,"busmouse")==0)       mtype = 6;
    }

    init = 1;
    if (mtype != P_BM && mtype != P_PS2)
    {
	ms_setspeed(B9600, mbaud, cflag[mtype]);
	ms_setspeed(B4800, mbaud, cflag[mtype]);
	ms_setspeed(B2400, mbaud, cflag[mtype]);
	ms_setspeed(B1200, mbaud, cflag[mtype]);

	if (mtype == P_LOGI)
	{
	    write(mfd, "S", 1);
	    ms_setspeed(mbaud, mbaud, cflag[P_MM]);
	}

	if	(msample <= 0)		write(mfd, "O", 1);
	else if	(msample <= 15)		write(mfd, "J", 1);
	else if	(msample <= 27)		write(mfd, "K", 1);
	else if	(msample <= 42)		write(mfd, "L", 1);
	else if	(msample <= 60)		write(mfd, "R", 1);
	else if	(msample <= 85)		write(mfd, "M", 1);
	else if	(msample <= 125)	write(mfd, "Q", 1);
	else				write(mfd, "N", 1);
    }
    if( mtype == P_MS)
    {
	char sink[2];
	read( mfd, sink, 2);
	/* discard "M3" read on startup */
    }
}

int
get_ms_event(struct ms_event *ev)
{
    unsigned char buf[5];
    char dx, dy;
    int i, acc;

    if (init == 0) ms_init(mx,my,0);
    if (mfd == -1)
	return -1;
    if (read(mfd, &buf[0], 1) != 1)
    	return -1;
restart:
/* find a header packet */
    while ((buf[0] & proto[mtype][0]) != proto[mtype][1])
    {
	if (read(mfd, &buf[0], 1) != 1)
	    return -1;
    }

/* read in the rest of the packet */
    for (i = 1; i < proto[mtype][4]; ++i)
    {
	if (read(mfd, &buf[i], 1) != 1)
	    return -1;
/* check whether it's a data packet */
	if (mtype != P_PS2 && ((buf[i] & proto[mtype][2]) != proto[mtype][3] || buf[i] == 0x80))
	    goto restart;
    }

/* construct the event */
    switch (mtype)
    {
	case P_MS:		/* Microsoft */
	default:
	    ev->ev_butstate = ((buf[0] & 0x20) >> 3) | ((buf[0] & 0x10) >> 4);
	    dx = (char)(((buf[0] & 0x03) << 6) | (buf[1] & 0x3F));
	    dy = (char)(((buf[0] & 0x0C) << 4) | (buf[2] & 0x3F));
	    break;
        case P_SUN:		/* Mouse Systems 3 byte as used in Sun workstations */
	    ev->ev_butstate = (~buf[0]) & 0x07;
	    dx =    (char)(buf[1]);
	    dy = - (char)(buf[2]);
	    break;
	case P_MSC:             /* Mouse Systems Corp 5 bytes as used for PCs */
	    ev->ev_butstate = (~buf[0]) & 0x07;
	    dx =    (char)(buf[1]) + (char)(buf[3]);
	    dy = - ((char)(buf[2]) + (char)(buf[4]));
	    break;
	case P_MM:              /* MM Series */
	case P_LOGI:            /* Logitech */
	    ev->ev_butstate = buf[0] & 0x07;
	    dx = (buf[0] & 0x10) ?   buf[1] : - buf[1];
	    dy = (buf[0] & 0x08) ? - buf[2] :   buf[2];
	    break;
	case P_PS2:            /* PS/2 */
	    ev->ev_butstate = ((buf[0] & 0x6) >> 1) | ((buf[0] & 0x1) << 2);
	    dx = (buf[0] & 0x10) ?   buf[1]-256 : buf[1];
	    dy = (buf[0] & 0x20) ? - (buf[2]-256) :   -buf[2];
	    break;
	case P_BM:              /* BusMouse */
	    ev->ev_butstate = (~buf[0]) & 0x07;
	    dx =   (char)buf[1];
	    dy = - (char)buf[2];
	    break;
    }

    acc = (abs(dx) + abs(dy) > mdelta) ? maccel : 1;
    ev->ev_dx = dx * acc;
    ev->ev_dy = dy * acc;
    x += ev->ev_dx;
    y += ev->ev_dy;
    limit(x, 0, mx);
    limit(y, 0, my);
    ev->ev_x = x;
    ev->ev_y = y;
    if (buttonwas == ev->ev_butstate)
    {
	if (ev->ev_butstate)
	    ev->ev_code = MS_DRAG;
	else
	    ev->ev_code = MS_MOVE;
    }
    else
    {
	if ((buttonwas & ~ev->ev_butstate) == 0)
	    ev->ev_code = MS_BUTDOWN;
	else
	    ev->ev_code = MS_BUTUP;
    }
    buttonwas = ev->ev_butstate;
    return 0;
}
