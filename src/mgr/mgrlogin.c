/*{{{}}}*/
/*{{{  Notes*/
/*

wtmp support and executing getty(8) is still missing, and most probably
bugs are lurking in the dark.

Usage: mgrlogin [-b background-bitmap][-f default-font] tty[1-8]

Michael

*/
/*}}}  */
/*{{{  #includes*/
#define _POSIX_SOURCE
#include <termios.h>
#include <sys/stat.h>
#include <limits.h>
#include <getopt.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <grp.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <pwd.h>
#include <netdb.h>
#include <sysexits.h>
#include <mgr/bitblit.h>
#include <mgr/font.h>
#include <mgr/window.h>

#include "proto.h"
#include "font_subs.h"
#include "set_mode.h"
/*}}}  */
/*{{{  #defines*/
#define MAILDIR "/var/spool/mail"
#define PATH "/bin:/usr/mgr/bin:."
#ifndef _NSIG
#ifdef NSIG
#define _NSIG NSIG
#else
#define _NSIG 32
#endif
#endif

#ifndef _POSIX_PATH_MAX
#define _POSIX_PATH_MAX 255
#endif
/*}}}  */

/*{{{  variables*/
struct font *font;
BITMAP *screen;

#ifdef DEBUG
int debug = 0;
char *debug_level = "";
#endif
/*}}}  */

/*{{{  printcursor*/
void printcursor(int x, int y, int on)
{
  bit_blit(screen,x,y-font->head.high,font->head.wide,font->head.high,on ? BIT_SET : BIT_CLR,(BITMAP*)0,0,0);
}
/*}}}  */
/*{{{  printchar*/
void printchar(int x, int y, unsigned char c)
{
  bit_blit(screen,x,y-font->head.high,font->head.wide,font->head.high,BIT_SRC,font->glyph[c],0,0);
}
/*}}}  */
/*{{{  printstr*/
void printstr(int x, int y, char *s)
{
  while (*s) { printchar(x,y,*s++); x+=font->head.wide; }
}
/*}}}  */
/*{{{  edit*/
unsigned char edit(int x, int y, char *s, int visible)
{
  unsigned char c;
  int len;

  len=strlen(s);
  while (1)
  {
    printcursor(x,y,1);
    read(0,&c,1);
    printcursor(x,y,0);
    switch (c)
    {
      /*{{{  escape*/
      case 27: return c;
      /*}}}  */
      /*{{{  return*/
      case '\r':
      case '\n': return c;
      /*}}}  */
      /*{{{  backspace*/
      case 127:
      case 8:
      {
        if (len) { s[--len]='\0'; if (visible) x-=font->head.wide; }
        break;
      }
      /*}}}  */
      /*{{{  default*/
      default:
      {
        if (len<8)
        {
          s[len++]=c;
          s[len]='\0';
          if (visible) { printchar(x,y,c); x+=font->head.wide; }
        }
      }
      /*}}}  */
    }
  }
}
/*}}}  */
/*{{{  cutebox*/
void cutebox(int bx, int by, int bw, int bh)
{
  bit_blit(screen,bx,by,bw,bh,BIT_CLR,(BITMAP*)0,0,0);

  bit_line(screen,bx,by,bx+bw,by,BIT_SRC);
  bit_line(screen,bx+bw,by,bx+bw,by+bh,BIT_SRC);
  bit_line(screen,bx+bw,by+bh,bx,by+bh,BIT_SRC);
  bit_line(screen,bx,by+bh,bx,by,BIT_SRC);

  bit_line(screen,bx+1,by+1,bx+bw-1,by+1,BIT_SRC);
  bit_line(screen,bx+bw-1,by+1,bx+bw-1,by+bh-1,BIT_SRC);
  bit_line(screen,bx+bw-1,by+bh-1,bx+1,by+bh-1,BIT_SRC);
  bit_line(screen,bx+1,by+bh-1,bx+1,by+1,BIT_SRC);

  bit_line(screen,bx+3,by+3,bx+bw-3,by+3,BIT_SRC);
  bit_line(screen,bx+bw-3,by+3,bx+bw-3,by+bh-3,BIT_SRC);
  bit_line(screen,bx+bw-3,by+bh-3,bx+3,by+bh-3,BIT_SRC);
  bit_line(screen,bx+3,by+bh-3,bx+3,by+3,BIT_SRC);
}
/*}}}  */
/*{{{  quit*/
void quit(int sig)
{
  bit_destroy(screen);
  reset_tty(0);
  exit(1);
}
/*}}}  */

/*{{{  main*/
int main(int argc, char *argv[])
{
  /*{{{  variables*/
  int x,login_y,password_y;
  unsigned char loginstr[9],passwordstr[9],ret;
  char ttystr[_POSIX_PATH_MAX];
  char *background=(char*)0;
  char *fontname=(char*)0;
  /*}}}  */

  /*{{{  parse arguments*/
  {
    int c;

    while ((c=getopt(argc,argv,"b:f:"))!=EOF) switch (c)
    {
      case 'b': background=optarg; break;
      case 'f': fontname=optarg; break;
    }
    /*{{{  parse tty*/
    {
      int tty;

      if (optind+1>argc)
      {
        fprintf(stderr,"Usage: %s tty\n",argv[0]);
        exit(1);
      }
      else
      {
        strcpy(ttystr,"/dev/");
        strcat(ttystr,argv[optind++]);
      }
      close(0); close(1); close(2);
      setsid();
      if ((tty=open(ttystr,O_RDWR))!=0)
      {
        fprintf(stderr,"%s: Can't open controlling terminal on fd 0.\n",argv[0]);
        exit(1);
      }
      fchmod(tty,0600);
      fchown(tty,getuid(),getgid());
      open(argv[optind],O_RDWR);
      open(argv[optind],O_RDWR);
    }
    /*}}}  */
  }
  /*}}}  */
  /*{{{  get into grafics mode*/
  signal(SIGTERM,quit);
  signal(SIGHUP,quit);
  set_tty(0);
  if ((screen=bit_open(SCREEN_DEV))==(BITMAP*)0)
  {
    reset_tty(0);
    exit(EX_NOPERM);
  }
  bit_grafscreen();
  /*}}}  */
  /*{{{  load font*/
  if (fontname)
  {
    char fontpath[_POSIX_PATH_MAX];

    if (*fontname=='/' || *fontname=='.') strcpy(fontpath,fontname);
    else { strcpy(fontpath,ICONDIR); strcat(fontpath,"/"); strcat(fontpath,fontname); }

    if ((font=open_font(fontname))==(struct font*)0) font=open_font((char*)0);
  }
  else font=open_font((char*)0);
  /*}}}  */
  /*{{{  draw background*/
  bit_blit(screen,0,0,screen->wide,screen->high,BIT_CLR,(BITMAP*)0,0,0);
  if (background)
  {
    BITMAP *bp;
    FILE *fp;
    char backgroundpath[_POSIX_PATH_MAX];

    if (*background=='/' || *background=='.') strcpy(backgroundpath,background);
    else { strcpy(backgroundpath,ICONDIR); strcat(backgroundpath,"/"); strcat(backgroundpath,background); }

    if ((fp=fopen(backgroundpath,"r"))!=(FILE*)0 && (bp=bitmapread(fp))!=(BITMAP*)0)
    {
      int x,y;

      for (x=0; x<screen->wide; x+=bp->wide) bit_blit
      (
        screen,
        x,0,
        screen->wide-x<bp->wide ? screen->wide-x : bp->wide,
        bp->high,
        BIT_SRC,bp,0,0
      );

      for (y=0; y<screen->high; y+=bp->high) bit_blit
      (
        screen,
        0,y,
        screen->wide,
        screen->high-y<bp->high ? screen->high-y : bp->high,
        BIT_SRC,screen,0,0
      );
    }
  }
  /*}}}  */
  /*{{{  draw hostname*/
  {
    int bx,bw,by,bh;
    char hostname[_POSIX_PATH_MAX];
    struct hostent *h;

    gethostname(hostname,sizeof(hostname));
    if ((h=gethostbyname(hostname))!=(struct hostent*)0) strcpy(hostname,h->h_name);
    bw=font->head.wide*(strlen(hostname)+2);
    bh=2*font->head.high;
    bx=(screen->wide-bw)/2;
    by=screen->high/6-bh/2;
    cutebox(bx,by,bw,bh);
    printstr(bx+font->head.wide,by+bh-font->head.high/2,hostname);
  }
  /*}}}  */
  /*{{{  draw login box*/
  {
    int bx,bw,by,bh;

    bx=(screen->wide-font->head.wide*40)/2;
    by=(screen->high-font->head.high*8)/2;
    bw=font->head.wide*40;
    bh=font->head.high*8;
    cutebox(bx,by,bw,bh);
  }
  /*}}}  */
  /*{{{  draw login box contents*/
  x=(screen->wide-font->head.wide*18)/2;
  login_y=screen->high/2-font->head.wide/6;
  password_y=screen->high/2+font->head.high/6+font->head.high;
  printstr(x,password_y,"Password:         ");
  printstr((screen->wide-font->head.wide*28)/2,login_y-2*font->head.high,
  "Press ESC for terminal login");
  /*}}}  */
  while (1)
  {
    /*{{{  get login and password or escape*/
    printstr(x,login_y,"Login:            ");
    *loginstr='\0'; *passwordstr='\0';
    do
    {
      ret=edit(x+font->head.wide*10,login_y,loginstr,1);
    } while ((ret=='\r' || ret=='\n') && *loginstr=='\0');
    if (ret=='\r' || ret=='\n')
    {
      ret=edit(x+font->head.wide*10,password_y,passwordstr,0);
      if (ret=='\r' || ret=='\n');
    }
    /*}}}  */
    if (ret==27)
    /*{{{  exec to usual login -- not implemented yet*/
    {
      bit_destroy(screen);
      reset_tty(0);
      exit(0);
    }
    /*}}}  */
    else
    /*{{{  try login*/
    {
      struct passwd *pw;

      if ((pw=getpwnam(loginstr))!=(struct passwd*)0 && strcmp(crypt(passwordstr,pw->pw_passwd),pw->pw_passwd)==0)
      /*{{{  start window manager*/
      {
        char mgrlogin[_POSIX_PATH_MAX];
        char env_user[_POSIX_PATH_MAX],env_logname[_POSIX_PATH_MAX];
        char env_home[_POSIX_PATH_MAX],env_shell[_POSIX_PATH_MAX];
        char env_path[_POSIX_PATH_MAX],env_mail[_POSIX_PATH_MAX];
        char *login_env[7] = { env_user, env_logname, env_home, env_shell, env_path, env_mail, (char*)0 };
        char *login_argv[2] = { "mgr", (char*)0 };
        int i;

        sprintf(env_user,"USER=%s",pw->pw_name);
        sprintf(env_logname,"LOGNAME=%s",pw->pw_name);
        sprintf(env_home,"HOME=%s",pw->pw_dir);
        sprintf(env_shell,"SHELL=%s",pw->pw_shell==(char*)0 || pw->pw_shell[0]=='\0' ? "/bin/sh" : pw->pw_shell);
        sprintf(env_path,"PATH=%s",PATH);
        sprintf(env_mail,"MAIL=%s/%s",MAILDIR,pw->pw_name);
        if (chdir(pw->pw_dir)!=0) chdir("/");
        if (ttyname(0)) { chown(ttyname(0),pw->pw_uid,pw->pw_gid); chmod(ttyname(0),0600); }
        for (i=1; i<=_NSIG; i++) signal(i,SIG_DFL);
        bit_destroy(screen);
        reset_tty(0);
        initgroups(pw->pw_name,pw->pw_gid);
        setgid(pw->pw_gid);
        setuid(pw->pw_uid);
        sprintf(mgrlogin,"%s/.mgrlogin",pw->pw_dir);
        execve(mgrlogin,login_argv,login_env);
        execve(MGR_BINARY,login_argv,login_env);
        exit(EX_OSFILE);
      }
      /*}}}  */
      else
      /*{{{  incorrect login*/
      {
        printstr((screen->wide-font->head.wide*16)/2,login_y+3*font->head.high,
        "Login incorrect");
      }
      /*}}}  */
    }
    /*}}}  */
  }
}
/*}}}  */
