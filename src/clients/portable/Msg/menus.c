/*                        Copyright (c) 1987 Bellcore
 *                            All Rights Reserved
 *       Permission is granted to copy or use this program, EXCEPT that it
 *       may not be sold for profit, the copyright notice must be reproduced
 *       on copies, and credit should be given to Bellcore where it is due.
 *       BELLCORE MAKES NO WARRANTY AND ACCEPTS NO LIABILITY FOR THIS PROGRAM.
 */

/* convert current directory into a heirarchy of menus */

/*	$Header: menus.c,v 4.3 88/08/29 13:27:16 sau Exp $
	$Source: /tmp/mgrsrc/demo/msg/RCS/menus.c,v $
*/
static char	RCSid_[] = "$Source: /tmp/mgrsrc/demo/msg/RCS/menus.c,v $$Revision: 4.3 $";

#include <mgr/mgr.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/dir.h>
#include <stdio.h>


#define MAX		512					/* max path length */
#define MENU		1000				/* max menu buffer length */
#define MENU_NAME	".menus"			/* default command menu */
#define MAX_ITEMS	20					/* max items per menu */
#define MAX_MENUS	9					/* max # of paged menus */

/* menu names */

#define CMD_MENU	1
#define MAIN_MENU	2
#define FILE_MENU	3
#define DIR_MENU	4

#define ARG		(argc>1 ? argv[1] : home)
#define COPY(where,index,what) \
	(index+strlen(what)<MENU ? \
              strcpy(where+index,what), index += strlen(what) \
        :     0)

static char buff[MAX];
static char load[MENU];
static char stat_buff[MAX];
static int len;

static int type = S_IFDIR;	/* type of file to accept */

struct menu_entry main_menu[] = {
	"Commands    =>", "",
	"Files       =>", "",
	"Directories =>", "",
	};

struct menu_entry cmd_menu[] = {
	"ls", "ls\n",
	"pwd","pwd\n",
	};

main(argc,argv)
int argc;
char **argv;
   {
   register int i, pos=0;
   int file;
   char **dir_list(),**list;
	register char **chunk;
   char *getenv(), *home = getenv("HOME");
   char *sprintf();
   char host[32];
   char tmp[100];

	int dir_index = 0;			/* for pages dir menus */
	int file_index = 0;			/* for paged file menus */

   ckmgrterm(*argv);
	m_setup(M_DEBUG);

   /* see if hostname required */

   if (argc>1 && strcmp(argv[1],"-h")==0) {
      gethostname(host,sizeof(host));
      argc--, argv++;
      }
   else
      *host = '\0';

	/* command menu (should be menu 1) */

   if ((file = open(sprintf(buff,"%s/%s",ARG,MENU_NAME),0)) > 0) {
      while((i=read(file,buff,sizeof(buff))) > 0)
         write(1,buff,i);
      close(file);
      }
	else		/* load up default command menu */
		menu_load(CMD_MENU,2,cmd_menu);

	menu_load(MAIN_MENU,3,main_menu);	/* load main menu */

	/* get directory menus */

	list = dir_list(ARG);
   chunk = list;
	while (chunk != (char **) 0) {
		if (dir_index==0)
			COPY(load,pos,"\005 ..");
		for(i=0;i<MAX_ITEMS && chunk[i]!=NULL;i++) {
			COPY(load,pos,"\005");
			COPY(load,pos,chunk[i]);
			}
		if (dir_index==0)
			COPY(load,pos,"\005cd ..\n");
	 	for(i=0;i<MAX_ITEMS && chunk[i]!=NULL;i++) {
			COPY(load,pos,"\005cd ");
			COPY(load,pos,chunk[i]);
			COPY(load,pos,"\n");
			}
	   load[pos] = '\0';
	   m_loadmenu(DIR_MENU+dir_index,load);

		if (dir_index > 0)
			m_pagemenu(DIR_MENU+dir_index-2, DIR_MENU+dir_index);
		dir_index += 2;
		if (chunk[i] && dir_index < MAX_MENUS*2)
			chunk += i;
		else 
			break;
		pos = 0;
	  }

	/* get plain file menus */

  pos = 0;
  type = S_IFREG;
  free(list);
  list = dir_list(ARG);
  chunk = list;
  while (chunk != (char **) 0) {
		for(i=0;i<MAX_ITEMS && chunk[i]!=NULL;i++) {		/* items */
			COPY(load,pos,"\005");
			COPY(load,pos,chunk[i]);
			}
		for(i=0;i<MAX_ITEMS && chunk[i]!=NULL;i++) {		/* actions */
			if (*host)
				sprintf(tmp,"%s%c%s/%s",host,':',ARG,chunk[i]);
			else
				sprintf(tmp,"%s/%s",ARG,chunk[i]);
			COPY(load,pos,sprintf(buff,"\005echo %c%d%c",ESC,strlen(tmp)+1,E_SNARF));
			COPY(load,pos,tmp);
			COPY(load,pos,"\n");
			}
		load[pos] = '\0';
		m_loadmenu(FILE_MENU+file_index,load);
		if (file_index > 0)
			m_pagemenu(FILE_MENU+file_index-2, FILE_MENU+file_index);
		file_index += 2;
		if (chunk[i] && file_index < MAX_MENUS*2)
			chunk += i;
		else 
			break;
		pos = 0;
		}

	free(list);

	/* setup menu links */

	m_selectmenu(MAIN_MENU);
	m_linkmenu(MAIN_MENU,0,1,0);
	m_linkmenu(MAIN_MENU,1,FILE_MENU,0);
	m_linkmenu(MAIN_MENU,2,DIR_MENU,0);
	m_flush();
   }

/* list all "right" files in a directory */

char **
dir_list(path)
char *path;
   {
   register int i;
   struct direct **entries;
   int select(), dir_sort();
   int count;
   char *malloc();
   char **list;
   
   len = strlen(path);
   strcpy(stat_buff,path);
   if (stat_buff[len-1] != '/')
      strcat(stat_buff,"/"),len++;
   if ((count = scandir(path,&entries,select,dir_sort)) < 0)
      return ((char **) 0);
   list = (char **) malloc(sizeof(char *) *(count+1));
   for(i=0;i<count;i++) list[i]=entries[i]->d_name;
   list[count] = (char *) 0;
   return(list);
   }

#define SUFFIX(x,s) \
	(strncmp(x + strlen(x) - strlen(s),s,strlen(s))==0)
int
select(d1)
struct direct *d1;
   {
   struct stat buf;

   if (*d1->d_name != '.' && !SUFFIX(d1->d_name,".o")) {
      strcat(stat_buff,d1->d_name);
      stat(stat_buff,&buf);
      stat_buff[len] = '\0';
      if ((buf.st_mode&S_IFMT) == type) 	/* directory */
         return(1);
      else
         return(0);
      }
   return(0);
   }

int
dir_sort(d1,d2)
register struct direct **d1, **d2;
   {
   return(strcmp((*d1)->d_name,(*d2)->d_name));
   }
