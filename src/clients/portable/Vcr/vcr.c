/* MGR vcr DEMO */

#include <mgr/mgr.h>
#include <sys/time.h>
#include <signal.h>

#define fsleep(s) /* sleep 100ths of a second */ \
   { \
   struct timeval time; \
   time.tv_sec = 0; \
   time.tv_usec = s*10000; \
   select(0,0,0,0,&time); \
   }

#define Ualarm(a,b)	if (cnt_flg) ualarm(a,b)

#define BUTTON_NAME	"buttons"	/* name of icon */

#define BUTTONS		9		/* number of buttons */
#define GAP				2		/* gap between icons */
#define BW				84		/* button width */
#define BH				52		/* button height */
#define SMP				 1		/* src bitmap */
#define PROG			 2		/* src bitmap for prog names */
#define DY				 1		/* y origin for buttons */

/* button types */

#define TOG				 1		/* toggle button */
#define MOM				 2		/* momentary button */
#define STICK			 4		/* may be down with other buttons */
#define CNT				 8		/* digits for counter icon */

/* button states */

#define OFF				 0		/* default button state */
#define ON				 1		/* button is on */

/* button names */

#define B_FF			0
#define B_PLAY			1
#define B_REW			2
#define B_STOP			3
#define B_SLO			4
#define B_PAUSE		5
#define B_SOFT			6
#define B_LOUD			7
#define B_PROGRAM		8
#define B_COUNT		9

/* counter intervals */

#define T_BASE			200000		/* base pause in us between counter clicks */
#define T_SLO			(2*T_BASE)
#define T_PLAY			(T_BASE)
#define T_FF			(T_BASE/2)
#define T_REW			(T_BASE/35)

#define DEFAULT	B_STOP			/* default "on" button */
#define MAX_PROGS	25					/* max # of programs */
#define DH		16		/* digit height */
#define BDR	11		/* button border */

#define B(x)		buttons[i].x		/* short hand */
#define C(x)		counter.x			/* short hand */
#define dprintf	if(debug)fprintf
#define Printf		printf				/* for commands to audio stuff */

int time_int[] = {						/* counter speeds */
	T_FF, T_PLAY, T_REW, 0, T_SLO, 0,0,0,0,0};
int dirs[] = {								/* counter directions */
	1, 1, -1, 0,1,0,0,0,0,0,0,0};
int speeds[] = {200,100,0,0,50,0,0,0,0,0};	/* driver speeds */

/* hardwire everything for expediency */

struct button {
	int src;					/* src bitmap id */
	int sx,sy;				/* on button source within bitmap */
	int zx,zy;				/* off button source within bitmap */
	int w,h;					/* size of button */
	int dx,dy;				/* destination of button within window */
	int type;				/* type of button  Toggle or Momentary */
	int state;				/* current button state */
	};

struct button buttons[] = {
/* src   sx   sy   zx   zy   w   h   dx  dy  type  state */
	SMP,   0,   0,	 86,   0, BW, BH, 460, DY, TOG,       OFF,	/* fast for */
	SMP, 172,   0,	258,   0, BW, BH, 370, DY, TOG,       OFF,	/* play */
	SMP,   0,  54,	 86,  54, BW, BH,  10, DY, TOG,       OFF,	/* rewind */
	SMP, 172,  54,	258,  54, BW, BH, 100, DY, TOG,       OFF,	/* stop */
	SMP,   0, 108,	 86, 108, BW, BH, 280, DY, TOG,       OFF,	/* slo-mo */
	SMP, 172, 108,	258, 108, BW, BH, 190, DY, TOG|STICK, OFF,	/* pause */
	SMP,   0, 162,	 86, 162, BW, BH, 640, DY, MOM|STICK, OFF,	/* softer */
	SMP, 172, 162,	258, 162, BW, BH, 730, DY, MOM|STICK, OFF,	/* louder */
	SMP,   0, 216,	 86, 216, BW, BH, 820, DY, MOM|STICK, OFF,	/* program */

	SMP, 172, 216,	172, 216, 50, BH, 554, DY, MOM|STICK, OFF,	/* counter */
	};
	
struct button counter = {
	SMP, 344,   0, 344,   2, 12, 164,550, 20+DY, CNT,       OFF, /* counter */
	};
struct button channel = {
	SMP, 258, 216,	258, 216, BW, BH, 820, DY, 0			, OFF,	/* chan */
	};

struct name {
	int id;		/* bitmap id */
	int w;		/* width */
	int h;		/* height */
	};

struct name names[MAX_PROGS];		/* place to hold script names */

char *but_names[] = {
	"FF", "PLAY", "REW", "STOP", "SLO", "PAUSE", "SOFT", "LOUD", "PRO", "COUNT", "?", "?", "?"
	};

int cnt_flg = 1;		/* do the counter */
int debug=0;			/* on for debug output */
char line[4096];		/* mgr input buffer */
int count = 0;			/* counter count */
int direction = 1;	/* counter direction */
int mode = B_STOP;	/* current VCR state */

int clean();			/* to clean up and exit */
int eot();				/* End Of Tape script is done, rewind */
char *get_names();

/*{{{}}}*/
/*{{{  adv_time*/
int
adv_time() 		/* advance the counter */
	{
	count+= direction;
	if (count>9999)
		count = 0;
	if (count<0)
		count = 9999;

	/* check for stopping a rewind simulate a rewind-press */

	if (count==0 && direction < 0) {
		Printf("s 0\n");		/* stop */
		Ualarm(0,0);
		sprintf(line,"B %d %d\r",buttons[B_REW].dx+10,buttons[B_REW].dy+10);
		m_sendme(line);
		}

	count_it(count);
	}
/*}}}  */

int main(int argc, char *argv[])
	{
	register int i=DEFAULT;	/* for B() macro */
	int tw, th,td;		/* total icon width, height */
	int x,y;			/* button position */
	int mom=0;		/* current button is momentary */
	char *name;		/* name of script */
	int script = 0;

	cnt_flg = !getenv("NOCOUNT");
	debug = getenv("DEBUG");
	dprintf(stderr,"VCR Debugging is on\n");

	if (argc<2) {
		fprintf(stderr,"usage: vcr <script_name>...\n");
		exit(1);
		}

	if (argc>MAX_PROGS) {
		fprintf(stderr,"Too many scripts\n");
		exit(1);
		}

	argv++;
	name = argv[script];
	setlinebuf(stdout);
	m_setup(M_MODEOK);
	m_ttyset();

	signal(SIGHUP,eot);
	signal(SIGTERM,clean);
	signal(SIGINT,clean);
	signal(SIGALRM,adv_time);

	m_push(P_FONT|P_POSITION|P_FLAGS|P_BITMAP|P_MENU|P_EVENT);
	m_setmode(M_ABS);
	m_setcursor(9);
	m_setmode(M_NOWRAP);
	m_movecursor(0,0);
	m_func(BIT_SRC);
	B(state)=1;		/* turn on default button */

	m_setevent(BUTTON_1,"B %p\r");
	m_setevent(BUTTON_2,"X");
	m_setevent(BUTTON_2U,"\r");
	m_setevent(BUTTON_1U,"U\r");
	m_setevent(REDRAW,"R\r");
	m_setevent(RESHAPE,"S\r");

	/* get the icon */

	get_icon(SMP,BUTTON_NAME,&tw,&th,&td);

	draw_all();
	setup_program(argv);
	Printf("P %d\n",getpid());					/* send pid */
	Printf("S %s\n",get_names(name));		/* set script name */
	while(m_gets(line)) {
	m_movecursor(1200,50);		/* bogus */
		mask_int(ON);
		switch(*line) {
			case 'X':		/* other button pushed  - generate click */
				if (buttons[B_PAUSE].state==ON && speeds[mode]==100) {
					Printf("c %s\n",line+1);
					dprintf(stderr,"VCR Sending click [%s]\n",line+1);
					}
				break;
			case 'B':		/* button pushed */
				sscanf(line+1,"%d %d\n",&x,&y);
				i = get_hit(x,y);	
				if (i >=0 && i<BUTTONS) {		/* we hit one */
					B(state) = 1 - B(state);

					/* set counter direction and speed */

					if (i == B_PAUSE && B(state)) {
						dprintf(stderr,"VCR Pausing counter\n");
						Printf("s 0\n");		/* stop */
						Ualarm(0,0);
						}
					else if (i == B_PAUSE) {
						dprintf(stderr,"VCR UN-Pausing counter\n");
						Printf("s %d\n",speeds[mode]);
						Ualarm(time_int[mode], time_int[mode]);
						}
					else if (B(type)==TOG) {
						mode = B(state) ? i : DEFAULT;
						direction = dirs[mode];
						if (direction<0) {
							dprintf(stderr,"VCR rewinding DO\n");
							Printf("q\ns 0\nS %s\n",get_names(argv[script]));
							}
						else {
							dprintf(stderr,"VCR setting speed to %d\n",speeds[mode]);
							Printf("s %d\n",speeds[mode]);
							}
						Ualarm(time_int[mode], time_int[mode]);
						dprintf(stderr,"VCR Setting counter speed\n");
						}

					push(i);
					if (B(type) & MOM)
						mom++;
					if (i==B_SOFT || i==B_LOUD) {	/* auto repeat */
						Printf("%c\n",i==B_SOFT ? '<' : '>');
						}
					}
				if (i==B_COUNT) 		/* reset the counter */
					count_it(count=0);
				break;
			case 'A':		/* Repeat (unused) */
				if (mom) {
					m_sendme("A\r");
					}
				break;
			case 'P':		/* do program button */
				x = do_program(script,argc-1);
				if (x != script) {		/* change channels */
					script = x;
					count_it(count=0);
					Printf("S %s\n",get_names(argv[script]));
					}
				break;
			case 'U':		/* button up */
				if (i==B_PROGRAM) {
					dprintf(stderr,"VCR Setting program\n");
					sprintf(line,"B %d %d\rU\rP\r",
							buttons[B_STOP].dx+5,buttons[B_STOP].dy+5);
					m_sendme(line);			/* fake a "stop", do the program */
					}
				if (mom) {
					B(state) = 0;
					push(i);
					mom=0;
					}
				break;
			case 'R':		/* redraw */
			case 'S':		/* reshape */
				draw_all();
				break;
			default:			/* not mine, pass on (likely kbd commands) */
				Printf("%s\n",line);
				dprintf(stderr,"VCR unknown command %c\n",*line);
				break;
			}
		m_flush();
		fflush(stdout);
		mask_int(OFF);
		}

	clean(0);
	exit(0);
	}

/* get program channels (this is a kludge) */

int
Do_program(chan,max)
int chan;									/* current chanell */
int max;										/* max chan. */
	{

	register int i;
	int x,y;						/* hit position */
	struct button *b = &channel;

	if (max>9)
		max = 9;

	m_bitcopyto(b->dx,b->dy,b->w,b->h,b->sx,b->sy,0,SMP);
	m_bitcopyto(b->dx+52,b->dy+16,12,DH,C(sx),C(sy)+DH*chan,0,SMP);
	while(m_gets(line) && *line == 'B') {
		sscanf(line+1,"%d %d\n",&x,&y);
		i = get_hit(x,y);	
		if (i!=B_PROGRAM) {
			m_sendme(line);
			break;
			}
		m_gets(line);			/* get up button */
		if (x > b->dx+22 && x < b->dx+32) {		/* up */
			if (++chan > max)
				chan = 1;
			m_bitcopyto(b->dx+52,b->dy+16,12,DH,C(sx),C(sy)+DH*chan,0,SMP);
			}
		else if (x > b->dx+35 && x < b->dx + 45) {	/* down */
			if (--chan < 1)
				chan = max;
			m_bitcopyto(b->dx+52,b->dy+16,12,DH,C(sx),C(sy)+DH*chan,0,SMP);
			}
		}
	draw(B_PROGRAM);
	return(chan);
	}

/* setup channel names */

int
setup_program(list)
char *list[];						/* prog names */
	{
	char name[1024];					/* get name */
	register int i;
	char *front,*back;
	char *index(), *rindex();
	int fw,fh;

	m_font(0);
        m_getfontsize(&fw,&fh);
	for(i=0;list[i];i++) {

		/* strip off path prefix */

		if (front=rindex(list[i],'/'))
			strcpy(name,front+1);
		else
			strcpy(name,list[i]);
		if (back=rindex(name,'='))		/* strip off command list */
			*back = '\0';
		if (back=rindex(name,'.'))		/* stript off '.Z' suffix */
			*back = '\0';
			
		m_bitcreate(PROG+i,BW-2*BDR-3,fh);	/* clip at right edge */
		m_stringto(PROG+i,0,0,name);
		names[i].id = PROG+i;
		names[i].w = fw*strlen(name);
		names[i].h = fh;
		}
	return(i);
	}

/* compute the name to send to do_ata */

char
*get_names(s)
char *s;
	{
	char *index();
	char *p;

	p = index(s,'=');
	if (p)
		return(p+1);
	else
		return(s);
	}

/* Switch among programs (this is a kludge) */
		
#define N(x)	names[chan].x		/* short hand */

int
do_program(chan,max)
int chan;		/* which channel */
int max;			/* max # of programs */
	{
	register int i = B_PROGRAM;		/* current button */
	int x,y;									/* mouse hit position */
	int offset;								/* offset for centering prog name */

	m_func(BIT_CLR);
	m_bitwrite(B(dx)+BDR,B(dy)+BDR,B(w)-2*BDR,B(h)-2*BDR);
	m_func(BIT_SRC);
	offset = N(w) < BW-2*BDR ? ((BW-2*BDR) - N(w))/2 : 0;
	m_bitcopyto(B(dx)+BDR+2+offset,B(dy)+BDR+5,N(w),N(h),0,0,0,N(id));
	while(m_gets(line) && *line == 'B') {
		sscanf(line+1,"%d %d\n",&x,&y);
		m_movecursor(0,0);		/* come on, find the bug! */
		if (get_hit(x,y) == B_PROGRAM) {
			m_func(BIT_CLR);
			m_bitcopyto(B(dx)+BDR+2+offset,B(dy)+BDR+5,N(w),N(h),0,0,0,N(id));
			m_func(BIT_SRC);
			chan = (chan+1)%max;
			offset = N(w) < BW-2*BDR ? ((BW-2*BDR) - N(w))/2 : 0;
			m_bitcopyto(B(dx)+BDR+2+offset,B(dy)+BDR+5,N(w),N(h),0,0,0,N(id));
			m_gets(line);		/* button up */
			}
		else {
			m_sendme(line);
			break;
			}
		}
	draw(B_PROGRAM);
	return(chan);
	}

/* mask timer interrupts during button processing */

int
mask_int(how)
int how;		/* ON or OFF */
	{
	static int prev_mask;

	if (how==ON)
		prev_mask = sigsetmask(1<<SIGALRM);
	else
		sigsetmask(prev_mask);
	}

/* get hit */

int
get_hit(x,y)
register int x,y;		/* mouse position */
	{
	register int i;

	for(i=0;i<=BUTTONS;i++) {
		if (B(dx) < x && B(dy) < y && B(dx) + B(w) > x && B(dy) + B(h) > y) {
			dprintf(stderr,"VCR Button hit on %s (%d,%d)\n",but_names[i],x,y);
			return(i);
			}
		}
	return(-1);
	}

/* push a button  make sure to adjust other buttons */

int
push(n)
int n;		/* button pushed */
	{
	register int i=n;	
	int on=0;			/* button is on now */

	dprintf(stderr,"VCR Pushed: ");
	draw(n);

	if (B(type)&STICK) {
		return(0);
		}

	on = B(state);
	for(i=0;on && i<BUTTONS;i++) {		/* turn off other buttons */
		if (i!=n && B(state)) {
			B(state) = 0;
			dprintf(stderr,"VCR Releasing: ");
			draw(i);	
			break;
			}
		}

		/* check pause button */

		i=B_PAUSE;
		if (on && B(state)) {
			B(state) = 0;
			draw(i);	
			}

	if (!on) {		/* turn on default button */
		i = DEFAULT;
		B(state)=1;
		dprintf(stderr,"VCR Defaulting: ");
		draw(i);
		}

	return(0);
	}

/* draw an icon */

int
draw(i)
register int i;
	{

	if (B(state))
		m_bitcopyto(B(dx),B(dy),B(w),B(h),B(sx),B(sy),0,SMP);
	else
		m_bitcopyto(B(dx),B(dy),B(w),B(h),B(zx),B(zy),0,SMP);
	dprintf(stderr," (VCR) setting %s to %s\n",but_names[i],B(state) ? "on" : "off");
	}

/* draw the icons */

int
draw_all()
	{
	register int i;

	m_clear();
	for(i=0;i<BUTTONS;i++) {
		draw(i);
		}
	m_bitcopyto(B(dx),B(dy),B(w),B(h),B(sx),B(sy),0,SMP);	/* counter box */
	reset_digit();
	count_it(count);
	}

/* The SDH memorial counter */

#define DIGITS	3		/* # digits on the counter */

static int divisor[] = {10, 100, 1000, 10000};		/* counter scaling */

int
count_it(n)
int n;			/* counter value 0-9999 */
	{
	register int i = B_COUNT;
	int sy;				/* source X position */
	int digit=0;			/* adjascent (to the right) digit value */
	
	for(i=0;i<DIGITS;i++) {
		sy = C(sy) + DH*(n/divisor[i]%10);
		if ( (i==0) || 9 == digit) {	/* slide it */
			sy += (n%10)*DH/10;
			digit = n/divisor[i]%10;		/* previous digit */
			}
		do_digit(i,sy);
		}
	m_flush();
	}	

/* update a digit on the display */

static int old_y[DIGITS];

reset_digit()
	{
	register int i;

	for(i=0;i<DIGITS;i++)
		old_y[i] = -1;
	}

int
do_digit(i,y)
int i;			/* which digit */
int y;			/* y src value */
	{
	if (old_y[i] != y) {
		m_bitcopyto(DIGITS*C(w)+C(dx)-i*C(w)-1,C(dy),C(w),DH,C(sx),y,0,SMP);
		old_y[i] = y;
		}
	}

clean(n)
int n;
	{
	m_popall();
	m_ttyreset();
	m_setcursor(0);
	m_clear();
	if (n!=0)
		exit(n);
	}

/* when script is finished - simulate rewind */

eot()
	{
	sprintf(line,"B %d %d\r",buttons[B_REW].dx+10,buttons[B_REW].dy+10);
	m_sendme(line);
	m_flush();
	}

get_icon(n,name,tw,th,td)
int n;					/* icon number */
char *name;				/* icon name */
int *tw, *th, *td;				/* icno size */
	{
	m_bitfile(n,name,tw,th,td);

	if (*tw==0) clean(1);
	}
