/*{{{}}}*/
/*{{{  Notes*/
/* check to make sure it is ok to read/write a file */
/*}}}  */
/*{{{  #includes*/
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <malloc.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include "proto.h"
/*}}}  */
/*{{{  #defines*/
#define Type(file)		(stat(file,&buff) ? 0 : buff.st_mode)
/*}}}  */

/*{{{  write_ok -- ok for user to write this file*/
int write_ok(char *name)
   {
   struct stat buff;
   char *ptr;
   int result;

   if (access(name,F_OK)==0) {
      result = (S_ISREG(Type(name)) && access(name,W_OK)==0);
      }
   else if ((ptr=strrchr(name,'/'))) {
      int len = (ptr-name)? (ptr-name): 1;
      char *dir = malloc(len+1);
      if (dir) {
	 strncpy(dir,name,len);
	 dir[len] = '\0';
	 result = (access(dir,W_OK)==0 && S_ISDIR(Type(dir)));
	 free(dir);
         }
      else
         result = 0;
      }
   else {
      result = (access(".",W_OK)==0);
      }
   return(result);
   }
/*}}}  */
/*{{{  read_ok -- see if ok to read a file*/
int read_ok(char *name)
   {
   struct stat buff;
   extern char *icon_dir;

   if (access(name,R_OK)==0) {
      return(1);
      }
   if (strncmp(name,icon_dir,strlen(icon_dir)) == 0 &&
       strrchr(name,'.') == 0 && stat(name,&buff)==0)
      return(S_ISREG(buff.st_mode));
   return 0;
   }
/*}}}  */
/*{{{  mode_ok -- make sure tty mode is ok for message passing*/
int mode_ok(char *name,int mask)
   {
   struct stat buff;
   if (stat(name,&buff) < 0) return(0);
   return((buff.st_mode&mask) == 0);
   }
/*}}}  */
