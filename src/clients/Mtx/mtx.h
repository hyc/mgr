/*                        Copyright (c) 1988,1989 Bellcore
 *                            All Rights Reserved
 *       Permission is granted to copy or use this program, EXCEPT that it
 *       may not be sold for profit, the copyright notice must be reproduced
 *       on copies, and credit should be given to Bellcore where it is due.
 *       BELLCORE MAKES NO WARRANTY AND ACCEPTS NO LIABILITY FOR THIS PROGRAM.
 */
/* MTX multiplexor stuff */

/* window state flags */

#define W_NORMAL		0x0000		/* a normal character */
#define W_ESC			0x0001		/* in "esc" mode */
#define W_DOWNLOAD	0x0002		/* in download mode */
#define W_REPLY		0x0004		/* download results in a reply */

#define DUP_KEY		'@'			/* MGR kbd dup key */

#ifndef NULL
#define NULL 0L
#endif

/* hard wired menu numbers */

#define MAIN_MENU		1		/* main menu */
#define HOST_MENU		2		/* hosts menu */
#define NEW_MENU		3		/* new process/window menu */
#define CONN_MENU		4		/* connection edit menu */

/* defines for host communication */

#define MAX_HOSTS		15		/* max number of hosts for menu */
#define H_THERE		1		/* host is there */
#define H_CONN			2		/* host is connected */

#define GET_HID(hid,pid)\
		(((hid)<<16) | ((pid)&0xffff))		/* hope its unique for all hosts */

/* broadcast message types */

#define M_HI		1				/* I'm here, everyone */
#define M_BYE		2				/* Buy, I'm gone */

/* codes for info queueing */

#define Q_MASK		0xff				/* window id is stored here */
#define Q_SINGLE	0x000				/* normal single item request */
#define Q_MULTI	0x100				/* multiple item request */
#define Q_NEW		0x200				/* making a new window with a process */
#define Q_WIN		(Q_NEW+0x100)	/* making a new window only */
#define Q_DIALOG	(Q_NEW+0x200)	/* making a dialog window */
#define Q_ALT		(Q_NEW+0x300)	/* for making a clients alternate window */

/* object types */

#define D_PROC		1					/* a process */
#define D_WINDOW	2					/* a window */
#define D_HOST		4					/* a network connection */
#define D_GHOST	8					/* not there - shouldn't happen */

/* MTX input states */

#define S_NORMAL	1					/* normal operation */
#define S_CONN		2					/* in connection window */

/* Locking modes for procedures */

#define L_IDLE		0					/* normal activity, waiting for input */
#define L_BUSY		1					/* need MGR ack to finish command */
#define L_PROC		2					/* in a procedure */
#define L_SKIP		4					/* unused */

/* simulated MGR-like  stuff */

#define G_BASE		50					/* base value for requests */
#define G_INPUTS	(G_BASE + 1)	/* return names of inputs */
#define G_OUTPUTS	(G_BASE + 2)	/* return names of outputs */
#define G_MYNAME	(G_BASE + 3)	/* return my name */
#define G_IMAX		(G_MYNAME + 1)	/* first invalid request */

/* misc defines */

#define W(x)	object->class.window->x		/* short hand */
#define WIN(o,x)	o->class.window->x		/* short hand */
#define P(x)	object->class.proc->x		/* short hand */
#define H(x)	object->class.host->x		/* short hand */
#define D(x)	dest->x							/* short hand */
#define O(x)	object->x						/* short hand */
#define TITLE	"MTX Window Server"			/* title for main window */
#define SH_MAX		40						/* max chars written to window at once */
#define MAIN_WIDE	60						/* width of main window */
#define MAX_NAME	32						/* max object-name size */
#define START		"start"				/* name of mtx startup file */
#ifndef HOME
#define HOME		"."					/* place to find mtx scripts */
#endif

/* message types */

#define MSG_INFO		0					/* informational message */
#define MSG_WARN		1					/* warning message */
#define MSG_ERROR		2					/* error message *

/* all the object pointers go here */

union class {
	struct window_state *window;	/* dest is a window */
	struct proc *proc;				/* dest is a "process" */
	struct host *host;				/* destination is another host */
	};

/* object strucure (data common to all objects) */

struct object {
	int type;							/* type of object */
	int id;								/* a unique object id (within each type) */
	char *name;							/* name of the object */
	int ref;								/* reference count */
	int dest_mask;						/* mask of destination fd's */
	struct object *next;				/* next object in the list */
	struct dest *dest;				/* list of destinations */
	union class class;				/* class specific data */
	};

/* structure to define window specific object info */

struct window_state {
	int state;							/* window state */
	int esc_count;						/* # of chars in escape sequence */
	int arg_count;						/* # of (,)'s in esc sequence */
	int push_count;					/* # of pushes */
	int alt_id;							/* current alternate ID # */
	int download_count;				/* # of chars to download after esc sequence */
	char *download_data;				/* data sent after an esc (malloc'd space) */
	char *curr;							/* next download char goes here */
	char esc[40];						/* saved escape sequence */
	char dup_char;						/* dup key requested by window */
	};

/* structure to define a process specific object info */

struct proc {
	int pid;								/* pid of process */
	int fd;								/* fd to read/write from process */
	char tty[3];						/* last 2 chars of controlling tty */
	char type;							/* main or secondary channel */
	};

/* define host table entry */

struct host {
	int state;					/* state of host */
	int hostid;					/* unique host id */
	int pid;						/* pid of remote host's process */
	int to_channel;			/* current write channel id (always zero)*/
	int from_channel;			/* current read channel id (always zero)*/
	int port;					/* port host is listening on */
	int fd;						/* file descriptor for this host */
	char host[32];				/* host name */
	char user[16];				/* user id */
	char *name;					/* name from remote host */
	};

/* struct to define a connection */

struct dest {
	struct dest *next;				/* another destination */
	struct object *object;			/* destination object */
	int state;							/* current connection state (unused) */
	};	

/* global variables */

extern FILE *debug;							/* for debugging output */
extern FILE *debug2;							/* for debugging output */

extern struct object *object_list;		/* head of object list */
extern int object_count;					/* current # of objects */
extern char dup_key;							/* kbd dup key */

/* local functions (out of date) */

char * str_save();
int child();
int clean();
int dequeue();
int do_esc();
int do_sh();
int enqueue();
int free_win();
int kill_proc();
int last_arg();
int message();
int setup_display();
int setup_main();
int to_dest();
struct dest *get_dest();
struct object *create_proc();
struct object *do_func();
struct object *find_byid();
struct object *find_byname();
struct object *find_bypos();
struct object *setup_host();
struct object *setup_proc();
struct object *setup_window();
