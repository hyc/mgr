/*{{{}}}*/
/*{{{  #includes*/
#include <mgr/mgr.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
/*}}}  */
/*{{{  #defines*/
#define ICON_GAP 3
#define WIDTH 400
#define MIN_ICON_HEIGHT 55
#define MIN_ICON_WIDTH 55
#define DEFAULT_FONT 0
#define EXAMPLE_FONT 99
/*}}}  */

/*{{{  variables*/
int x,y,border,dummy;
int icon_width,icon_height,font_width,font_height,curfont_height,curfont_width;
int status_height;
char example[40], *example_str="ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
char *oldfontname=(char*)0;
char *cmd_name;
struct icon arr_up = { "arrup",0,0,0,(char*)0 };
struct icon arr_down = { "arrdown",0,0,0,(char*)0 };
struct icon stop = { "stopsign",0,0,0,(char*)0 };
/*}}}  */

/*{{{  clean up and exit*/
void cleanup(int n, char *s)
{
  m_loadfont(EXAMPLE_FONT,oldfontname);
  m_popall();
  m_setcursor(0);
  m_ttyreset();
  if (s) fprintf(stderr,"%s: %s\n",cmd_name,s);
  exit(n);
}

void clean(int n)
{
  cleanup(n,(char*)0);
}
/*}}}  */
/*{{{  down load an icon*/
void download_icon(icon,where) struct icon *icon; int where;
{
  int w_in, h_in, d_in;

  m_bitfile(where,icon->name,&w_in,&h_in,&d_in);
  icon->w=w_in;
  icon->h=h_in;
  icon->type=where;
  if (h_in==0 || w_in==0)
  {
    fprintf(stderr,"Can't find icon %s\n",icon->name);
    clean(1);
  }
}
/*}}}  */
/*{{{  draw_icons*/
void draw_icons(void)
{
m_clear();
m_bitcopyto(WIDTH+(icon_width-arr_up.w)/2+ICON_GAP,ICON_GAP+(icon_height-arr_up.h)/2+status_height,arr_up.w,arr_up.h,0,0,0,arr_up.type);
m_bitcopyto(WIDTH+(icon_width-arr_down.w)/2+ICON_GAP,2*ICON_GAP+icon_height+(icon_height-arr_down.h)/2+status_height,arr_down.w,arr_down.h,0,0,0,arr_down.type);
m_bitcopyto(WIDTH+(icon_width-stop.w)/2+ICON_GAP,3*ICON_GAP+2*icon_height+(icon_height-stop.h)/2+status_height,stop.w,stop.h,0,0,0,stop.type);
m_line(WIDTH,status_height,WIDTH,status_height+4*ICON_GAP+3*icon_height);
m_line(0,curfont_height+2*ICON_GAP,WIDTH+2*ICON_GAP+icon_width,curfont_height+2*ICON_GAP);
}
/*}}}  */
/*{{{  read_font*/
int read_font(char *s)
{
  char name[255];
  int dummy;

  if (oldfontname==(char*)0)
  {
    m_go(0,3*icon_height);
    m_aligntext();
    m_font(EXAMPLE_FONT);
    oldfontname=m_getfontname();
    oldfontname=strcpy(malloc(strlen(oldfontname)+1),oldfontname);
  }
  m_loadfont(EXAMPLE_FONT,s);
  m_go(0,3*icon_height);
  m_aligntext();
  m_font(EXAMPLE_FONT);
  m_getfontsize(&font_width,&font_height);
  return (!strcmp(m_getfontname(),s));
}
/*}}}  */
/*{{{  print_font*/
void print_font(int ok, char *s)
{
  /*{{{  print font size*/
  m_font(DEFAULT_FONT);
  m_go(ICON_GAP,curfont_height+ICON_GAP);
  m_aligntext();
  m_cleareol();
  if (ok) fprintf(m_termout,"%s, %dx%d",s,font_width,font_height);
  else fprintf(m_termout,"%s -- no such font",s);
  /*}}}  */
  /*{{{  print font example*/
  m_func(BIT_CLR);
  m_bitwrite(0,status_height+1,WIDTH-1,4*ICON_GAP+3*icon_height-1);
  m_func(BIT_SRC);
  if (ok)
  {
    m_font(EXAMPLE_FONT);
    m_go(ICON_GAP,status_height+((4*ICON_GAP+3*icon_height)+font_height)/2);
    m_aligntext();
    strcpy(example,example_str);
    example[((WIDTH-ICON_GAP)/font_width)>=sizeof(example) ? sizeof(example)-1 : (WIDTH-ICON_GAP)/font_width]='\0';
    fputs(example,m_termout);
  }
  /*}}}  */
}
/*}}}  */

/*{{{  main*/
int main(argc,argv) int argc; char *argv[];
{
  /*{{{  variables*/
  int check_w,check_h,ok,quit=0,ch,click_x,click_y,current=1;
  /*}}}  */

  /*{{{  set up*/
  ckmgrterm(*argv);
  cmd_name=argv[0];
  if (argc<=1)
  {
    fprintf(stderr,"Usage: %s font ...\n",cmd_name);
    exit(2);
  }
  m_setup(0);
  m_push(P_ALL);
  m_ttyset();
  m_setmode(M_NOWRAP);
  m_setmode(M_ABS);
  m_func(BIT_SRC);
  m_setcursor(CS_INVIS);
  m_font(DEFAULT_FONT);
  m_getfontsize(&curfont_width,&curfont_height);
  m_getwindowposition(&x,&y);
  border=m_getbordersize();
  status_height=curfont_height+2*ICON_GAP;
  /*}}}  */
  /*{{{  signals*/
  signal(SIGHUP,clean);
  signal(SIGTERM,clean);
  signal(SIGINT,clean);
  /*}}}  */
  /*{{{  load icons*/
  download_icon(&arr_down,1);
  download_icon(&arr_up,2);
  download_icon(&stop,3);
  /*{{{  compute icon_width*/
  icon_width=arr_down.w;
  if (arr_up.w>icon_width) icon_width=arr_up.w;
  if (stop.w>icon_width) icon_width=stop.w;
  if (icon_width<MIN_ICON_WIDTH) icon_width=MIN_ICON_WIDTH;
  /*}}}  */
  /*{{{  compute icon_height*/
  icon_height=arr_down.h;
  if (arr_up.h>icon_height) icon_height=arr_up.h;
  if (stop.h>icon_height) icon_height=stop.h;
  if (icon_height<MIN_ICON_HEIGHT) icon_height=MIN_ICON_HEIGHT;
  /*}}}  */
  /*}}}  */
  /*{{{  initial reshape*/
  m_shapewindow(x,y,2*border+icon_width+2*ICON_GAP+WIDTH, 2*border+4*ICON_GAP+3*icon_height+status_height);
  m_getwindowsize(&check_w,&check_h);
  if (check_w<icon_width+2*ICON_GAP+WIDTH) cleanup(1,"Unable to reshape wide enough, place me more to the left");
  if (check_h<4*ICON_GAP+3*icon_height+status_height) cleanup(1,"Unable to reshape high enough, place me more to the up");
  /*}}}  */
  /*{{{  make first full redraw*/
  draw_icons();
  ok=read_font(argv[current]);
  print_font(ok,argv[current]);
  /*}}}  */
  /*{{{  set events*/
  m_setevent(RESHAPE,"R");
  m_setevent(MOVE,"M");
  m_setevent(BUTTON_1,"[%p]");
  /*}}}  */
  /*{{{  event loop*/
  do
  {
    m_flush();
    ch=m_getchar();
    switch (ch)
    {
      /*{{{  M -- moved*/
      case 'M': m_getwindowposition(&x,&y); break;
      /*}}}  */
      /*{{{  R -- reshape*/
      case 'R': m_shapewindow(x,y,2*border+icon_width+2*ICON_GAP+WIDTH, 2*border+4*ICON_GAP+3*icon_height+status_height); draw_icons(); print_font(ok,argv[current]); break;
      /*}}}  */
      /*{{{  [ -- click*/
      case '[':
      {
        fscanf(m_termin,"%d %d]",&click_x,&click_y);
        if (click_x>WIDTH && click_x<=WIDTH+2*ICON_GAP+icon_width && click_y<=status_height+4*ICON_GAP+3*icon_height && click_y>status_height)
        {
          click_y-=status_height;
          if (click_y>icon_height*2+3*ICON_GAP)
          /*{{{  stop*/
          quit=1;
          /*}}}  */
          else if (click_y>icon_height+2*ICON_GAP)
          /*{{{  arrow down*/
          {
            if (current<argc-1)
            {
              current++;
              ok=read_font(argv[current]);
              print_font(ok,argv[current]);
            } else m_bell();
          }
          /*}}}  */
          else if (click_y>ICON_GAP)
          /*{{{  arrow up*/
          {
            if (current>1)
            {
              current--;
              ok=read_font(argv[current]);
              print_font(ok,argv[current]);
            } else m_bell();
          }
          /*}}}  */
        }
        break;
      }
      /*}}}  */
      /*{{{  EOF*/
      case EOF: quit=1; break;
      /*}}}  */
    }
  } while (!quit);
  /*}}}  */
  clean(0);
  return (64);
}
/*}}}  */
