/*                        Copyright (c) 1988,1989 Bellcore
 *                            All Rights Reserved
 *       Permission is granted to copy or use this program, EXCEPT that it
 *       may not be sold for profit, the copyright notice must be reproduced
 *       on copies, and credit should be given to Bellcore where it is due.
 *       BELLCORE MAKES NO WARRANTY AND ACCEPTS NO LIABILITY FOR THIS PROGRAM.
 */
/* stuff to keep track of remote host communication */

#include <stdio.h>
#include "mtx.h"

/* field a broadcast message */

#define NO_NAME		"host"	/* default name of "host" object */
extern int errno;

int
process_broadcast(udp,line)
char *udp;							/* pntr to my udp data */
char *line;							/* incoming broadcast message */
	{
	int fd = -1;					/* fd to turn off (if any) */
	int type;						/* message type */
	int id;							/* host id */
	int pid;							/* process id on host */
	int port;						/* listening port */
	char user[32];					/* user name */
	char host[32];					/* host name */
	struct object *object;		/* host object associated with message */	

	sscanf(line,"%d %d %d %d %s %s", &type, &id, &pid, &port, user, host);
	
	if (is_me(udp,id,pid)) { 		/* its my own, ignore it */
		Dprintf('O',"Got my own broadcast packet\n");
		return(fd);
		}

	switch(type) {
		case M_HI:
			if (object=find_byid(D_HOST,GET_HID(id,pid))) {
				Dprintf('O',"Got another HI from %s@%s\n",user,host);
				}
			else {
				object = setup_host(line);	
				Dprintf('O',"adding %s@%s to host menu\n",user,host);
				menu_addhost(object);
				udp_send(udp,M_HI);	/* tell all other hosts we're here */
				message(MSG_WARN,"welcome %s@%s",user,host,0);
				}
			break;
		case M_BYE:
			if (object=find_byid(D_HOST,GET_HID(id,pid))) {
				menu_dlthost(object);
				fd = kill_host(object);
				}
			break;
		default:
			Dprintf('E',"Recieved unknown message: %s\n",line);
			message(MSG_ERROR,"Recieved unknown message %d from %s@%s",
					type,user,host);
			break;
		}
	return(fd);
	}

static char line[80];

/* a remote host asked for a connection */

int
get_connect(tcp)
char *tcp;
	{
	int fd;				/* new fd for connection */
	int id;				/* id of the other guy */
	int count;
	struct object *object;
	char name[32];		/* name from remote host */

	fd = do_accept(tcp);
	if (fd <= 0) {
		Dprintf('E',"Do_accept failed\n");
		return(-1);
		}

	Dprintf('O',"about to read host ID number from fd %d\n",fd);
	count = read(fd,line,sizeof(line));
	Dprintf('O',"Got %d/%d (%s) from other host\n",count,sizeof(line),line);
	sscanf(line,"%d %s",&id,name);
	object = find_byid(D_HOST,id);	
		
	if (!object) {
		Dprintf('E',"get_connect: Don't have object %d\n",id);
		return(-1);
		}

	H(fd) = fd;
	H(state) = H_CONN;
	H(name) = str_save(name);
	message(MSG_WARN,"You have a connection (%s) from %s@%s",
				name,H(user),H(host));
	open_cmdfile(name);		/* run the command file from this guy */
	return(fd);
	}

/* initiate a connection to another host */

int
connect_host(object,name)
struct object *object;		/* host object to host connection */
char *name;						/* name of connection to send to other host */
	{
	int id;						/* my unique ID */
	int fd;						/* file descriptor */
	int count;

	fd = get_con(H(host), H(port));
	id = get_myid();

	if (fd > 0) {
		H(state) = H_CONN;
		H(fd) = fd;
		sprintf(line,"%d %s",id,name?name:NO_NAME);
		Dprintf('O',"   writing id %d (name %s) to fd %d\n",id,name?name:NO_NAME,fd);
		errno=0;
		count = write(fd,line,strlen(line)+1);
		Dprintf('O',"   wrote %d/%d (%d)\n",count,strlen(line)+1,errno);
		message(MSG_WARN,"You started a connection to %s@%s",H(user),H(host));
		}
	else {
		Dprintf('E',"connect_host: get_con failed to connect to %s\n",H(host));
		message(MSG_ERROR,"Sorry, can't connect to %s",H(host),0,0);
		}
	return(fd);
	}
