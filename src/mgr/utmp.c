#ifdef WHO
/*{{{}}}*/
/*{{{  #includes*/
#include <fcntl.h>
#include <pwd.h>
#include <utmp.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

#ifndef USER_PROCESS
#define USER_PROCESS 1
#endif
#ifndef DEAD_PROCESS
#define DEAD_PROCESS 1
#endif

/*}}}  */

/*{{{  utmp_entry*/
static void utmp_entry(char *line, char *name, char *host, time_t logtime, int type, pid_t pid)
{
  /*{{{  variables*/
#  ifdef linux
  struct utmp entry;
  struct utmp *utmp_ptr;
#  endif
#  ifdef sun
  struct utmp entry;
  int fd;
  off_t offset;
#  endif
  /*}}}  */

  /*{{{  linux code*/
#  ifdef linux
  setutent();
  memset((char *)&entry,0,sizeof(entry));
  strncpy(entry.ut_line,line+sizeof("/dev/"),sizeof(entry.ut_line));
  strncpy(entry.ut_id,line+sizeof("/dev/tty"),sizeof(entry.ut_id));
  if ((utmp_ptr=getutline(&entry))!=(struct utmp *)0) entry=*utmp_ptr;
  if (name!=(char*)0) strncpy(entry.ut_name,name,sizeof(entry.ut_name));
  if (host!=(char*)0) strncpy(entry.ut_host,host,sizeof(entry.ut_host));
  entry.ut_time=logtime;
  if (type!=0) entry.ut_type=type;
  if (pid>1) entry.ut_pid=pid;
  pututline(&entry);
  endutent();
#  endif
  /*}}}  */
  /*{{{  sun code*/
#  ifdef sun
  if ((fd=open("/etc/utmp",O_RDWR))<0) return;
  offset=-1L;
  /*{{{  try to find and read user record*/
  while (read(fd,&entry,sizeof(entry))==sizeof(entry)) if (entry.ut_name[0] && !strncmp(line+sizeof("/dev"),entry.ut_line,sizeof(entry.ut_line)))
  {
    offset=tell(fd)-sizeof(entry);
    break;
  }
  /*}}}  */
  if (offset!=-1L)
  /*{{{  seek to its position*/
  lseek(fd,offset,SEEK_SET);
  /*}}}  */
  else
  /*{{{  seek to free one*/
  {
    lseek(fd,(off_t)0,SEEK_SET);
    while (read(fd,&entry,sizeof(entry))==sizeof(entry)) if (entry.ut_name[0]=='\0')
    {
      offset=tell(fd)-sizeof(entry);
      break;
    }
    if (offset!=-1L) lseek(fd,(off_t)offset,SEEK_SET);
    memset((char *)&entry,0,sizeof(entry));
  }
  /*}}}  */
  /*{{{  set entry fields*/
  strncpy(entry.ut_line,line+sizeof("/dev"),sizeof(entry.ut_line));
  if (name!=(char*)0 && *name && type==USER_PROCESS) strncpy(entry.ut_name,name,sizeof(entry.ut_name)); else entry.ut_name[0]='\0';
  if (host!=(char*)0 && *host) strncpy(entry.ut_host,host,sizeof(entry.ut_host)); else entry.ut_host[0]='\0';
  entry.ut_time=logtime;
  /*}}}  */
  write(fd,&entry,sizeof(entry));
  close(fd);
#  endif
  /*}}}  */
}
/*}}}  */

/*{{{  rm_utmp*/
void rm_utmp(line) char *line;
{
  utmp_entry(line,"","",(time_t)0,DEAD_PROCESS,0);
}
/*}}}  */
/*{{{  add_utmp*/
void add_utmp(line) char *line;
{
  time_t t;
  struct passwd *entry=getpwuid(getuid());

  time(&t);
  utmp_entry(line,entry==(struct passwd*)0 ? "unknown" : entry->pw_name,(char*)0,t,USER_PROCESS,getpid());
}
/*}}}  */
#endif
