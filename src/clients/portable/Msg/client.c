/*	$Header: client.c,v 4.1 88/06/21 14:02:36 bianchi Exp $
	$Source: /tmp/mgrsrc/demo/msg/RCS/client.c,v $
*/
static char	RCSid_[] = "$Source: /tmp/mgrsrc/demo/msg/RCS/client.c,v $$Revision: 4.1 $";

/*                        Copyright (c) 1987 Bellcore
 *                            All Rights Reserved
 *       Permission is granted to copy or use this program, EXCEPT that it
 *       may not be sold for profit, the copyright notice must be reproduced
 *       on copies, and credit should be given to Bellcore where it is due.
 *       BELLCORE MAKES NO WARRANTY AND ACCEPTS NO LIABILITY FOR THIS PROGRAM.
 */
/* sample client for testing server (template only) */

#include <mgr/mgr.h>
#include <stdio.h>
#include <signal.h>

#include "do.h"

#define MAX	20	/* max # of servers */

struct server {
   int id;
   char *name;
   };

struct server servers[MAX];
int cnt=0;			/* server count */
 
main(argc,argv)
int argc;
char **argv;
   {
   register int i;
   char *ttyname();
   char *str_save();
   int clean();
   char message[100];
   char line[100];
   char args[100];
   char name[20];
   int id;

   ckmgrterm( *argv );

   m_setup(M_FLUSH);
   chmod(ttyname(2),0600);		/* this is not gauranteed */
   m_push(P_EVENT|P_FLAGS);		/* save old events and flags */
   signal(SIGTERM,clean);		/* cleanup upon termination */
   signal(SIGINT,clean);
   m_setevent(NOTIFY,"client");		/* my name is "client" */
   m_setevent(ACCEPT,"&%f:%m\n");	/* get messages: &<from>:<message\n */
   m_setevent(BUTTON_1,"D %n");
   m_setevent(BUTTON_2,"X %n is id %w");
   sprintf(message,"%c",C_WHO);
   m_broadcast(message);		/* ask all servers for their id's */
   m_flush();

   /* loop, reading lines from keyboard */

   while(m_gets(line) != NULL) {
      printf("got: %s",line);
      if (*line == '&') {
         sscanf(line+1,"%d:%s %[^\n]", &id,message,args);
         switch (*message) {
            case S_HI:			/* received server announcement */
               printf("Server %d (%s) started [%s]\n",id,message,args);
               install(args,id);
               break;

            case S_BYE:			/* a server died */
               printf("Server %d died\n",id);
               for(i=0;i<cnt;i++)
                  if (servers[i].id == id) {
                     printf("server %s (%d) removed from %d\n",
                            servers[i].name,servers[i].id,i);
                     servers[i].id = 0;
                     free(servers[i].name);
                     break;
                     }
                  if (i+1 == cnt && cnt>0)
                     cnt--;
               break;

            case S_REPLY:			/* a server reply message */
               printf("Server %d replied %s\n",id,message);
               switch (message[1]) {
                  case R_WHO:			/* who are you */
                     install(args,id);
                     break;
                  case R_RCVD:			/* received command */
                     printf("Command received\n");
                     break;
                  case R_DO:			/* working on command */
                     printf("Command is executing\n");
                     break;
                  case R_DONE:			/* command completed */
                     printf("Command completed\n");
                     break;
                  case R_BAD:			/* command rejected */
                     printf("Command rejected\n");
                     break;
                  case R_UNKWN:			/* unknown command */
                     printf("Command unknown\n");
                     break;
                  default:
                     printf("Unknown server reply\n");
                  }
               break;

            case C_WHO:			/* some other client sent this */
               printf("got a client WHO message\n");
               break;

            default:
               printf("non-server message from %d (%s)\n",id,message);
               break;
            }
         }

      /* not a message, must be a button hit or kbd */

      else {
         switch (*line) {
            case 'L':			/* list servers */
               for(i=0;i<cnt;i++) {
                  if (servers[i].id > 0)
                     printf("%d) server %d is [%s]\n",
                             i,servers[i].id, servers[i].name);
                  }
               break;

            case 'S':			/* put file in snarf buffer */
               m_snarf(line+2);
               printf("Putting->snarf buffer: %s\n",line+2);
               break;

            case 'D':			/* send command to server */
               sscanf(line,"%*s %s %[^\n]", name,args);
               for(i=0;i<cnt;i++) 
                  if (servers[i].id > 0 && strcmp(servers[i].name,name)==0) {
                     printf("sending [%s] to %s (%d)\n",
                             args,name,servers[i].id);
                     sprintf(message,"%c %s",C_DO,args);
                     m_sendto(servers[i].id,message);
                     break;
                     }
               if (i==cnt)
                  printf("Can't find server [%s]\n",name);
               break;

            case '?':			/* list commands */
               printf("L		list servers\n");
               printf("D <server> <command>		do a command\n");
               printf("?		ask for help\n");
               break;

            default:
               printf("Not a message: %s\n",line);
            }
         }
      m_flush();
      }
   }
   
/*	Clean up and exit */

clean(n)
int n;
   {
   m_popall();
   exit(n);
   }

/* save a copy of string s */

char *
str_save(s)
char *s;
   {
   char *malloc(), *strcpy();
   return(strcpy(malloc(strlen(s)+1),s)); 
   }

/* install a server */

int
install(name,id)
char *name;
int id;
   {
   register int i,spot = -1;

   for(i=0;i<cnt;i++)
      if (servers[i].id == id) {
         printf("server id %d already installed at %d\n",id,i);
         if (strcmp(name,servers[i].name) != 0) {
            free(servers[i].name);
            servers[i].name = str_save(name);
            }
         return(i);
         }
      else if (spot == -1 && servers[i].id == 0)
         spot = i;


      if (spot != -1) {
         servers[spot].id = id;
         servers[spot].name = str_save(name);
         printf("server %s (%d) installed at %d\n",
                name,id,spot);
         }
      else if (cnt < MAX) {
         servers[cnt].id = id;
         servers[cnt].name = str_save(name);
         printf("server %s (%d) installed at %d\n",
                name,id,cnt);
         cnt++;
         }
   return(i);
   }
