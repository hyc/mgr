/*                        Copyright (c) 1988,1989 Bellcore
 *                            All Rights Reserved
 *       Permission is granted to copy or use this program, EXCEPT that it
 *       may not be sold for profit, the copyright notice must be reproduced
 *       on copies, and credit should be given to Bellcore where it is due.
 *       BELLCORE MAKES NO WARRANTY AND ACCEPTS NO LIABILITY FOR THIS PROGRAM.
 */
/* internet access stuff */

#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <pwd.h>
#include <sys/time.h>
#include "mtx.h"

#define SERVER	"share"
#define N(x)	net->x

#ifdef SO_BROADCAST
#   define SOL_OPTS		SO_BROADCAST
#else
#   define SOL_OPTS		0
#endif

static char host[32];						/* my hostname */
extern int errno;

struct net_udp {
	int fd;							/* socket file descriptor */
	int id;							/* unique net-wide ID */
	int pid;							/* my pid */
	int port;						/* tcp port to connect to */
	long timestamp;				/* local time socket opened */
	char *host;						/* host name */
	char *user;						/* user name */
	struct sockaddr_in sock;	/* socket name */
	};

/* setup a udp broadcast port (this only gets called once) */

static struct net_udp net_space;

char *
udp_setup(name,port)
char *name;		/* name of host to indicate which interface to use */
int port;		/* port to listen on */
	{
	register struct net_udp *net = &net_space;
   int net_id;					/* my network number */
	int fd;						/* socket file descriptor */
   struct hostent *hp;		/* host info for me */
   struct hostent *hp2;		/* host info to indicate which hardware interface to use */
   struct servent *sp;		/* service info */
	struct timeval time;
	struct passwd  *pwd;
	char *malloc();

	Dprintf('O',"setup_udp:\n ");
	gethostname(host,sizeof(host)-1);
	if ((hp=gethostbyname(host)) == NULL) {
		Dprintf('E',"gethostbyname (%s) error (%d)\n",host,errno);
		return(NULL);
		}

	if (name && (hp2=gethostbyname(name)) == NULL) {
		Dprintf('E',"gethostbyname (%s) error (%d)\n",name,errno);
		return(NULL);
		}

	if ((sp=getservbyname(SERVER,"udp")) ==0) {
		Dprintf('E',"unknown service: %s error (%d)\n",SERVER,errno);
		return(NULL);
		}

   if ((fd = socket (AF_INET, SOCK_DGRAM, 0)) < 0) {
		Dprintf('E',"socket error (%d)\n",errno);
		return(NULL);
		}

	bzero(&net->sock,sizeof(struct sockaddr_in));
	if (name) 
		net_id = inet_netof(*((struct in_addr *)hp2->h_addr));
	else
		net_id = inet_netof(*((struct in_addr *)hp->h_addr));
	net->sock.sin_family = hp->h_addrtype;
	net->sock.sin_port = sp->s_port;
	net->sock.sin_addr = inet_makeaddr(net_id,INADDR_ANY);
   if (setsockopt (fd, SOL_SOCKET, SOL_OPTS, &net->sock,sizeof(struct sockaddr_in)) <0) {
		Dprintf('E',"setsockopt  error (%d)\n",errno);
		}

	if (bind (fd, &(net->sock), sizeof(struct sockaddr_in)) < 0) {
		Dprintf('E',"bind error (%d)\n",errno);
		free(net);
		close(fd);
		return(NULL);
		}

	gettimeofday(&time,0l);
	pwd = getpwuid(getuid());

	N(timestamp) = time.tv_sec;
	N(user) = pwd->pw_name;
	N(host) = host;
	N(id)   = gethostid();
	N(fd) = fd;
	N(pid) = getpid();
	N(port) = port;

	Dprintf('O',"fd=%d, host=%s, user=%s, id=%d pid=%d port=%d\n",
			N(fd), N(host), N(user), N(id), N(pid), port);
	return((char *)net);
   }

/* broadcast a (udp) message */

static char line[100];					/* place to build message */

int
udp_send(udp,message)
char *udp;							/* pntr to broadcast socket */
int message;						/* message number */
	{
	register struct net_udp *net = (struct net_udp *) udp;
	int count, len;
	
	if (net) {
		sprintf(line,"%d %d %d %d %s %s",
				message, N(id), N(pid), N(port), N(user), N(host));
		len = strlen(line)+1;
		count = sendto(net->fd,line,len,0,&net->sock,sizeof(struct sockaddr_in));
		Dprintf('O',"broadcast: %d/%d (%s)\n",count,len,line);
		return(count == len);
		}
	else
		return(0);
	}

/* get fd for (udp) broadcast port */

int
udp_fd(udp)
char *udp;							/* pntr to broadcast socket */
	{
	register struct net_udp *net = (struct net_udp *) udp;
	return(net ? net->fd : -1);
	}

/* retrieve a broadcast message (with header info) */

int
udp_get(udp,msg,len)
char *udp;							/* pntr to brdcst socket */
char *msg;							/* the message */
int len;								/* max msg length */
	{
	register struct net_udp *net = (struct net_udp *) udp;
	int count;

	count = read(net->fd,msg,len);
	Dprintf('O',"Got broadcast %d/%d: %.*s\n",count,len,count,msg);
	return(count>0);
	}

/* see if this is my own broadcast packet */

int
is_me(udp,id,pid)
char *udp;
int id;								/* hostid */
int pid;								/* process id */
	{
	register struct net_udp *net = (struct net_udp *) udp;

	return(net->id == id && net->pid == pid);
	}

/* return hostname */

char *
my_name()
	{
	return(host);
	}

/****************************************************************************/

/* get a tcp socket and port number (only do this once) */

struct net_tcp {
	int fd;							/* socket file descriptor */
	struct sockaddr_in sock;	/* socket name */
	};

static struct net_tcp tcp;

char *
get_tcp()
	{
	int len = sizeof(struct sockaddr_in);				/* length of socket name */

	if ((tcp.fd=socket(AF_INET,SOCK_STREAM,0)) < 0) {
		message(MSG_ERROR,"Can't get TCP socket",0,0,0);
		return(NULL);
		}

	Dprintf('O',"listening on fd %d\n",tcp.fd);
	if (listen(tcp.fd,2) != 0) {
		message(MSG_ERROR,"Can't listen on TCP socket",0,0,0);
		Dprintf('E',"listen failed (%d)\n",errno);
		return(NULL);
		}

	if (getsockname(tcp.fd,&tcp.sock,&len)!=0 || len!=sizeof(tcp.sock)) {
		message(MSG_ERROR,"Can't get TCP socket name",0,0,0);
		Dprintf('E',"getsockname failed (%d)\n",errno);
		return(NULL);
		}
	Dprintf('O',"Get_tcp:  tcp listening socket fd= %d\n",tcp.fd);
	return((char *) &tcp);
	}

int
get_tcpport(s)
char *s;
	{
	int port;

	struct net_tcp *tcp = (struct net_tcp *) s;
	port = ntohs((u_short)tcp->sock.sin_port);
	Dprintf('O',"Got tcp listening port %d\n",port);
	return(port);
	}

/* get file descriptor associated with tcp port (for listening) */

int
get_tcpfd(s)
char *s;
	{
	struct net_tcp *tcp = (struct net_tcp *) s;
	return(tcp->fd);
	}

/* get our own unique id */

int
get_myid()
	{
	return(GET_HID(gethostid(),getpid()));
	}

/* get a connection to a known host/port */

int
get_con(host,port)
char *host;				/* host to connect to */
int port;				/* port # on host */
	{
   struct hostent *hp;			/* host info */
	struct sockaddr_in sock;	/* socket name */
	int fd;							/* fd for socket */
	int timeout;					/* connect retry timer */

	if ((hp=gethostbyname(host)) == NULL) {
		message(MSG_ERROR,"Can't find host %s",host,0,0);
		return(0);
		}

	for(timeout=1;timeout<=8;timeout *= 2) {
		if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
			Dprintf('E',"socket call failed (%d)\n",errno);
			return(0);
			}
		bzero(&sock,sizeof(sock));
		sock.sin_family = hp->h_addrtype;
		sock.sin_port = htons((u_short)port);
		bcopy(hp->h_addr, (caddr_t)&sock.sin_addr, hp->h_length);  
		errno=0;

		Dprintf('O',"Attempting connect to %s on port %d (try=%d)\n",
					host,port,timeout);
		if (connect(fd, &sock, sizeof(sock)) != 0 && errno==ECONNREFUSED) {
			close(fd);
			message(MSG_WARN,"Retrying connect to %s (%d seconds)",host,timeout);
			Dprintf('E',"  Connect failed (refused), sleepiny %d\n",timeout);
			sleep(timeout);
			continue;
			}
		else if (errno>0 && errno != ECONNREFUSED) {
			Dprintf('E',"  Connect failed (%d)\n",errno);
			close(fd);
			return(-1);
			}
		else
			return(fd);
		}
	return(-1);
	}

/* accept a connection, return new FD */

int
do_accept(s)
char *s;
	{
	struct net_tcp *tcp = (struct net_tcp *) s;
	int fd;

	Dprintf('O',"Trying to accept a connection\n");
	if ((fd = accept(tcp->fd,0,0)) == -1) {
		message(MSG_ERROR,"Can't accept a connection",0,0,0);
		Dprintf('E',"Accept failed on fd %d (%d)\n",tcp->fd,errno);
		return(0);
		}
	Dprintf('O',"Accept succeeded, got fd %d\n",fd);
	return(fd > 0 ? fd : -1);
	}
