/*                        Copyright (c) 1988,1989 Bellcore
 *                            All Rights Reserved
 *       Permission is granted to copy or use this program, EXCEPT that it
 *       may not be sold for profit, the copyright notice must be reproduced
 *       on copies, and credit should be given to Bellcore where it is due.
 *       BELLCORE MAKES NO WARRANTY AND ACCEPTS NO LIABILITY FOR THIS PROGRAM.
 */
/* those pieces of mtx that don't use MGR calls */

#include <stdio.h>
#include <sys/wait.h>
#include <sys/signal.h>
#include "mtx.h"

extern int errno;

/***************************************************************************
 * Create a destination for a message 
 */

struct dest *
get_dest(dest,object)
struct dest *dest;			/* current destination list */
struct object *object;		/* new object to link into destination */
	{
	char *malloc();
	struct dest *result;

   if ((result=(struct dest *) malloc(sizeof(struct dest))) == NULL) 
      return(dest);

	result->next = dest;
	result->object = object;
	object->ref++;

	return(result);
	}

/***************************************************************************
 * free an entire list of destinations
 */

int
free_alldest(dest)
register struct dest *dest;
	{
	register struct dest *next;

	for(;dest;dest=next) {
		next=D(next);
		dest->object->ref--;
		free(dest);
		}
	}

/***************************************************************************
 * free an item in a destination list 
 */

int
free_dest(object,to)
struct object *object;					/* source object */
struct object *to;						/* destination object */
	{
	register struct dest *prev, *dest = O(dest);

	if (!dest)
		return(0);

	if (dest->object == to) {
		O(dest) = dest->next;
		to->ref--;
		free(dest);
		return(1);	
		}

	for(prev=dest,dest=D(next);dest;prev=dest,dest=D(next)) {
		if (dest->object == to) {
			prev->next = dest->next;
			to->ref--;
			free(dest);
			return(1);
			}
		}
	return(0);
	}

/***************************************************************************
 * purge all references to an object (broken ??)
 */

purge_obj(id)
struct object *id;				/* object to remove references to */
	{
	struct object *object;				/* object to remove references to */
	int free_dest();
	int count;

	count = id->ref;

	Dprintf('O',"Purging %s: %d references\r\n",id->name,count);
	for(object=object_list;count>0 && object;object=O(next))  {
		if (object == id) {
			Dprintf('O',"skipping object %s\r\n",O(name));
			continue;
			}
		Dprintf('O',"Looking at object %s\r\n",O(name));
		while(count>0 && free_dest(object,id))
			count--;
		}
	}

/***************************************************************************
 * manage GET_INFO queue
 */

#define MAXQ	20

static int queue[MAXQ];		/* the queue */
static int q_count = 0;		/* # items in the queue */
static int q_start = 0;		/* the first item in the queue */

int
enqueue(id,what)
register int id;
char *what;		/* for debugging */
	{
	queue[(q_start + q_count++)%MAXQ] = id;
	Dprintf('Q',"Q'ing [%s] %d (0x%x)\n",what,id,id);
	return(q_count < MAXQ);
	}

int
dequeue()
	{
	int result = 0;

	if (q_count) {
		result = queue[q_start++];
		q_count--;
		if (q_start >= MAXQ)
			q_start -= MAXQ;
		}
	Dprintf('Q',"deQ'ing %d (0x%x)\n",result,result);
	return(result);		
	}	

/***************************************************************************
 * kill a process		- return fd bit for select mask
 */

int
kill_proc(object)
struct object *object;			/* must be a process object */
	{
	int result;

	killpg(P(pid),SIGHUP);
	Dprintf('O',"  sent kill to %s pid=%d (fd=%d)\n",O(name),P(pid),P(fd));
	if (geteuid() < 1) {
		fchmod(P(fd),0666);
		fchown(P(fd),0,0);
		}
	close(P(fd));
	Dprintf('O',"In kill_proc (%s)\r\n", O(name));
	result = P(fd);
	purge_obj(object);
	free_obj(object);
	return(result);
	}
/***************************************************************************
 * kill a host connection	- return fd bit for select mask
 */

int
kill_host(object)
struct object *object;			/* must be a host object */
	{
	int result;

	Dprintf('O',"In kill_host (%s@%s)\r\n", H(user),H(host));
	if (H(state)==H_CONN) {		/* host is connected  - disconnect */
		close(H(fd));
		result = H(fd);
		free_alldest(O(dest));
		purge_obj(object);
		H(state) = H_THERE;
		O(dest) = NULL;
		O(type) = D_GHOST;
		message(MSG_WARN,"%s@%s signing off (breaking connection)",
					H(user),H(host),0);
		free_obj(object);
		return(result);
		}
	else if (H(state)==H_THERE) {		/* not connected */
		message(MSG_WARN,"%s@%s signing off",H(user),H(host),0);
		free_obj(object);
		return(-1);
		}
	else
		return(-1);
	}

/***************************************************************************
 * create a process structure 
 */

struct object *
create_proc(fd,pid,type,name)
int fd;								/* fd to talk to process */
int pid;								/* pid of process */
char type;							/* type of process */
char *name;							/* name this process */
	{
	struct object *object;
	struct proc *proc;
	char *malloc(), *get_ttyname();
	char *str_save(), *str_gen();

   if ((object=(struct object *) malloc(sizeof(struct object))) == NULL) 
      return(0);

   if ((proc=(struct proc *) malloc(sizeof(struct proc))) == NULL)  {
		free(object);
      return(0);
		}

	O(id) = fd;
	O(type) = D_PROC;
	O(dest) = NULL;
	O(ref) = 0;
	O(dest_mask) = 0;
	O(class.proc) = proc;
	P(pid) = pid;
	P(fd) = fd;
	P(type) = type;
	strcpy(proc->tty,get_ttyname());	/* wrong */
	if (name)
		O(name) = str_save(name);
	else
		O(name) = str_gen("P(%d.%c)",pid,type,0);

	/* link into list */

	O(next) = object_list;
	object_list = object;
	object_count++;

	return (object);
	}

/***************************************************************************
 * start a process - return object (process) structure
 */

struct object *
setup_proc()
	{
	struct object *object;
	struct proc *proc;
	int pid,fd;

	pid = get_shell(NULL,&fd,NULL,0);
	if (pid > 0)
		return(create_proc(fd,pid,'c',NULL));
	else
		return(NULL);
	}

/***************************************************************************
 * start a host connection - return object (host) structure
 */

struct object *
setup_host(line)
char *line;										/* broadcast message */
	{
	struct object *object;
	struct host *host;
	int hid,pid;
	char *malloc();

   if ((object=(struct object *) malloc(sizeof(struct object))) == NULL) 
      return(0);

   if ((host=(struct host *) malloc(sizeof(struct host))) == NULL)  {
		free(object);
      return(0);
		}

	sscanf(line,"%*d %d %d",&hid, &pid);
	O(type) = D_HOST;
	O(id) = GET_HID(hid,pid);
	O(dest) = NULL;
	O(ref) = 0;
	O(dest_mask) = 0;
	O(class.host) = host;

	H(state) = H_THERE;
	H(hostid) = hid;
	H(pid) = pid;
	sscanf(line,"%*d %*d %*d %d %s %s", &H(port), H(user), H(host));
	O(name) = str_gen("%s@%s",H(user),H(host));
	H(fd) = 0;
	H(from_channel) = H(to_channel) = 0;		/* chanells not yet used */

	/* link into list */

	O(next) = object_list;
	object_list = object;
	object_count++;

	return (object);
	}

/***************************************************************************
 * remove an object from the object list, free up space
 */

int
free_obj(gone)
register struct object *gone;
	{
	register struct object *prev, *object=object_list;
	
	if (object == gone) {
		object_list=gone->next;
		}
	else {
		prev=object_list;
		for(object=prev->next;object;prev=object,object=O(next)) {
			if (gone==object) {
				prev->next = O(next);
				break;
				}
			}
		}
	if (gone==object) {
		free_alldest(gone->dest);
		if (gone->name)
			free(gone->name);
		free(gone);
		object_count--;
		return(1);
		}
	else
		return(0);
	}

/***************************************************************************
 * return pointer to window given window id
 */

struct object *
find_byid(type, id)
register int type, id;
	{
	register struct object *object;

	for(object=object_list;object;object=O(next))
		if (O(type)==type && O(id)==id) 
			break;
	if (!object)
		Dprintf('W',"Findbyid can't find object: %d.%d\n",type,id);
	return(object);
	}

/***************************************************************************
 * return pointer to window given window name  (we could hash names if needed)
 */

struct object *
find_byname(name,flag)
register char *name;		/* name of object to find (can have wild cards) */
int flag;					/* if true, an error if found */
	{
	char *re_comp();
	register struct object *object;

	if (*name == '$') {		/* do a regexp search */
		Dprintf('W',"Find_byname doing wildcarding of %s\n",name+1);
		if (re_comp(name+1)) {
			Dprintf('W',"Find_byname cant compile: %s\n",name);
			return(NULL);
			}
		for(object=object_list;object;object=O(next))
			if (re_exec(O(name))) {
				Dprintf('W',"Matched %s with %s\n",O(name),name+1);
				break;
				}
		}
	else							/* do a normal search */
		for(object=object_list;object;object=O(next))
			if (strcmp(O(name),name)==0) 
				break;

	if (flag ^ !object) {
		Dprintf('W',"Find_byname can%s find object: %s\n",
				name,flag?" not" : "");
		message(MSG_ERROR,"%s object named %s",
				name,flag ? "Already an" : "Can not find",0);
		}
	return(object);
	}

/***************************************************************************
 * return pointer to window given position and type
 */

struct object *
find_bypos(index,flag)
int index;				/* which object from the top of the list */
int flag;				/* consider only this type of object */
	{
	register struct object *object;

	for(object=object_list;object;object=O(next))
		if (O(type)&flag && index-- <=0)
			break;
	if (!object)
		Dprintf('W',"find_bypos Cant find object: # %d, type: 0x%x\n",index,flag);
	return(object);
	}

/***************************************************************************
 * write a string to the appropriate destinations
 */

int
to_dest(object,dest,buff,count)
register struct object *object;	/* who's it from (NULL for kbd) - unused */
register struct dest *dest;		/* list of destinations */
char *buff;								/* buffer to send */
int count;								/* how many bytest to send */
	{
	char *safe_print();

	if (count <= 0) {
		extern int errno;
		Dprintf('E',"to_dest error: %d %s\n",count,strerror(errno));
		return(count);
		}

	for(;dest;dest = D(next)) {
		switch (D(object->type)) {
			case D_WINDOW:
				Dprintf('T',"Writing Window %s %d..",
						D(object->name), count);
				count = do_sh(object,D(object),buff,count);
				Dprintf('T',"%d [%s]\n",count,safe_print(buff,count));
				break;
			case D_PROC:
				Dprintf('T',"Writing  %s (proc %d) %d ..",
						D(object->name),D(object->class.proc->fd),count);
				count = write(D(object->class.proc->fd),buff,count);
				Dprintf('T',"%d [%s]\n",count,safe_print(buff,count));
				break;
			case D_HOST:
				Dprintf('T',"Writing %s (host %d) %d ..",
						D(object->name),D(object->class.host->fd),count);
				count = write(D(object->class.host->fd),buff,count);
				Dprintf('T',"%d [%s]\n",count,safe_print(buff,count));
				break;
			}
		}
	if (count < 0)
		Dprintf('E',"to_dest write error (%d)\n",errno);
	return(count);
	}

/* return select fd bit (no side effects) */

int
BIT(x)
	{
	return(x<0 ? 0 : 1<<x);
	}

/* set write fd mask for an object, make sure wmask is updated */

int
set_fdmask(from,to)
register struct object *from, *to;
	{
	int result;		/* wmask bit */
	Dprintf('S',"Setting write mask for %s: 0x%x->",
				from->name,from->dest_mask);
	switch(to->type) {
		case D_PROC:				/* these objects can block */
			from->dest_mask |= BIT(to->class.proc->fd);
			result = BIT(to->class.proc->fd);
			break;
		case D_HOST:
			from->dest_mask |= BIT(to->class.host->fd);
			result = BIT(to->class.host->fd);
			break;
		default:						/* no blocking possible, ignore */
			break;
			}
	Dprintf('S',"0x%x\n",from->dest_mask);
	}

/* clear write mask bit for an object */

int
clear_fdmask(from,to)
register struct object *from, *to;
	{
	Dprintf('S',"Clearing write mask for %s: 0x%x->",
				from->name,from->dest_mask);
	switch(to->type) {
		case D_PROC:				/* these objects can block */
			from->dest_mask &= ~BIT(to->class.proc->fd);
			break;
		case D_HOST:
			from->dest_mask &= ~BIT(to->class.host->fd);
			break;
		default:						/* no blocking possible, ignore */
			break;
			}
	Dprintf('S',"0x%x\n",from->dest_mask);
	}

/* insure that no object references invalid write mask bits */

reset_fdmask(mask)
int mask;				/* only these bits may be turned on */
	{
	register struct object *object;

	for(object=object_list;object;object=O(next)) 
		O(dest_mask) &= mask;							/* turn off fd bit */
	}

/* send a signal to all process objects (for suspending) */

sig_proc(sig)
int sig;				/* signal to send */
	{
	register struct object *object;

   signal(SIGCHLD,SIG_IGN);
	for(object=object_list;object;object=O(next))
		if (O(type)==D_PROC) {
			Dprintf('O',"Suspending  %s pid=%d\n",O(name),P(pid));
			killpg(P(pid),sig);
			}
   signal(SIGCHLD,child);
	}

/* malloc space for and return a sprintf'd string */

char
*str_gen(fmt,a,b,c)
char *fmt;			/* sprintf format */
int a,b,c;			/* sprintf args */
	{
	char *str_save();
	static char buff[128];

	sprintf(buff,fmt,a,b,c);
	return(str_save(buff));
	}
