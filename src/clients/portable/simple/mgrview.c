/*{{{}}}*/
/*{{{  #includes*/
#define _POSIX_SOURCE
#include <errno.h>
#include <string.h>
#include <limits.h>
#include <signal.h>
#include <getopt.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <mgr/mgr.h>
/*}}}  */
/*{{{  #defines*/
#define WINDOW_BITMAP 0
#define IMAGE_BITMAP 1

#ifndef _POSIX_PATH_MAX
#define _POSIX_PATH_MAX 256
#endif
/*}}}  */

/*{{{  variables*/
int display_op=BIT_SRC;
/*}}}  */

/*{{{  clean*/
void clean(int n)
{
  m_setcursor(CS_BLOCK);
  m_pop();
  m_flush();
  m_ttyreset();
  exit(n);
}
/*}}}  */
/*{{{  display*/
void display(int x, int y, int my_width, int my_height, int image_width, int image_height)
{
  int image_x=0,image_y=0;

  m_func(BIT_CLR);
  m_bitwrite(0,0,my_width,my_height);
  m_func(display_op);
  if (x<0) { image_width+=x; image_x-=x; x=0; }
  if (y<0) { image_height+=y; image_y-=y; y=0; }
  m_bitcopyto(x,y,image_width,image_height,image_x,image_y,WINDOW_BITMAP,IMAGE_BITMAP);
}
/*}}}  */

/*{{{  main*/
int main(int argc, char *argv[])
{
  /*{{{  variables*/
  enum { NOTHING, OTHER_FS, SAME_FS } in=NOTHING;
  int c;
  int mouse_x,mouse_y;
  int image_width,image_height;
  int image_xoffset=0,image_yoffset=0;
  int my_width,my_height;
  int image_depth,image_size;
  int err=0,usage=0;
  FILE *input=(FILE*)0;
  static struct menu_entry menu[] =
  {
    { "Normal","|n\n" },
    { "Reverse","|r\n" },
    { "-------", "" },
    { "Quit","|q\n" }
  };
  char file[_POSIX_PATH_MAX];
  char event[20];
  /*}}}  */

  /*{{{  parse arguments*/
  while ((c=getopt(argc,argv,"ro:s:"))!=EOF)
  {
    switch (c)
    {
      /*{{{  r*/
      case 'r': display_op=BIT_NOT(BIT_SRC); break;
      /*}}}  */
      /*{{{  o file*/
      case 'o':
      {
        if ((input=fopen(optarg,"r"))==(FILE*)0)
        {
          fprintf(stderr,"%s: Can't open %s\r\n",argv[0],optarg);
          err=1;
        }
        else in=OTHER_FS;
        break;
      }
      /*}}}  */
      /*{{{  s file*/
      case 's':
      {
        char *cwd;

        in=SAME_FS;
        if (*optarg!='/' && *optarg!='.')
        {
          if ((cwd=getcwd((char*)0,(size_t)0))!=(char*)0) { strcpy(file,cwd); strcat(file,"/"); strcat(file,optarg); }
          else { fprintf(stderr,"%s: Can't get current directory\r\n",argv[0]); err=1; }
        }
        else strcpy(file,optarg);
        break;
      }
      /*}}}  */
      /*{{{  default*/
      default:
      {
        usage=1;
        break;
      }
      /*}}}  */
    }
  }
  if (err) exit(err);
  if (usage || optind!=argc)
  {
    fprintf(stderr,"Usage: mgrview [-o file | -s file]\n");
    exit(1);
  }
  if (in==NOTHING) { in=OTHER_FS; input=stdin; }
  /*}}}  */
  /*{{{  setup*/
  ckmgrterm(argv[0]);
  m_setup(M_MODEOK);
  signal(SIGINT,clean);
  signal(SIGTERM,clean);
  signal(SIGPIPE,clean);
  m_ttyset();
  m_push(P_ALL);
  m_setmode(M_ABS);
  m_setcursor(CS_INVIS);
  menu_load(1,4,menu);
  m_setevent(REDRAW, "|R\n");
  m_setevent(RESHAPE, "|R\n");
  m_setevent(BUTTON_1,"|!%p!\n");
  m_setevent(BUTTON_2,"|m\n");
  m_flush();
  /*}}}  */
  if (in==OTHER_FS)
  {
    /*{{{  variables*/
    struct b_header bh;
    void *bp;
    /*}}}  */

    /*{{{  load bitmap to client space*/
    if (fread(&bh,sizeof(struct b_header),1,input)!=1)
    {
      fprintf(stderr,"%s: Can't read header of bitmap.\r\n",argv[0]);
      clean(1);
    }
    if (!B_ISHDR8(&bh))
    {
      fprintf(stderr,"%s: No MGR bitmap or old format.\r\n",argv[0]);
      clean(1);
    }
    B_GETHDR8(&bh,image_width,image_height,image_depth);
    image_size=B_SIZE8(image_width,image_height,image_depth);
    /*}}}  */
    /*{{{  transfer bitmap to server space*/
    m_func(BIT_SRC);
    m_bitcreate(IMAGE_BITMAP,image_width,image_height);
    m_bitldto(image_width,image_height,0,0,IMAGE_BITMAP,image_size);
    bp=malloc(image_size);
    fread(bp,image_size,1,input);
    fwrite(bp,image_size,1,m_termout);
    free(bp);
    /*}}}  */
  }
  else if (in==SAME_FS)
  /*{{{  transfer bitmap from server fs to server space*/
  {
  m_bitfromfile(IMAGE_BITMAP,file);
  m_get();
  if (sscanf(m_linebuf,"%d %d",&image_width,&image_height)<2)
  {
    fprintf(stderr,"%s: MGR server can't load MGR bitmap.\r\n",argv[0]);
    clean(1);
  }
  }
  /*}}}  */
  /*{{{  user interaction*/
  m_getwindowsize(&my_width,&my_height);
  display(image_xoffset,image_yoffset,my_width,my_height,image_width,image_height);
  m_flush();
  do
  {
    if (m_getevent(10000,&c,event,sizeof(event))==EV_EVENTSTR) switch (event[0])
    {
      /*{{{  n,r*/
      case 'n':
      case 'r':
      {
        display_op=(c=='n' ? BIT_SRC : BIT_NOT(BIT_SRC));
        m_func(display_op);
        m_bitcopyto(image_xoffset,image_yoffset,image_width,image_height,0,0,WINDOW_BITMAP,IMAGE_BITMAP);
        m_flush();
        break;
      }
      /*}}}  */
      /*{{{  m -- left button displays menu*/
      case 'm':
      {
        m_selectmenu(1);
        m_flush();
        break;
      }
      /*}}}  */
      /*{{{  !%d %d! -- right button*/
      case '!':
      {
        sscanf(event,"!%d %d!",&mouse_x,&mouse_y);
        /*{{{  compute new x start*/
        if (my_width>image_width) image_xoffset=0;
        else if (mouse_x<=0) image_xoffset=0;
        else if (mouse_x>=my_width) image_xoffset=my_width-image_width;
        else
        {
          /*{{{  move x start by difference from mouse and middle*/
          image_xoffset=image_xoffset-(mouse_x-my_width/2);
          /*}}}  */
          /*{{{  check and corrent range of x start*/
          if (image_xoffset<my_width-image_width) image_xoffset=my_width-image_width;
          else if (image_xoffset>0) image_xoffset=0;
          /*}}}  */
        }
        /*}}}  */
        /*{{{  compute new y start*/
        if (my_height>image_height) image_yoffset=0;
        else if (mouse_y<=0) image_yoffset=0;
        else if (mouse_y>=my_height) image_yoffset=my_height-image_height;
        else
        {
          /*{{{  move y start by difference from mouse and middle*/
          image_yoffset=image_yoffset-(mouse_y-my_height/2);
          /*}}}  */
          /*{{{  check and corrent range of y start*/
          if (image_yoffset<my_height-image_height) image_yoffset=my_height-image_height;
          else if (image_yoffset>0) image_yoffset=0;
          /*}}}  */
        }
        /*}}}  */
        display(image_xoffset,image_yoffset,my_width,my_height,image_width,image_height);
        m_flush();
        break;
      }
      /*}}}  */
      /*{{{  R -- redraw*/
      case 'R':
      {
        m_getwindowsize(&my_width,&my_height);
        /*{{{  compute new x offset*/
        if (my_width<image_width)
        {
          if (image_xoffset<my_width-image_width) image_xoffset=my_width-image_width;
        }
        else image_xoffset=0;
        /*}}}  */
        /*{{{  compute new y offset*/
        if (my_height<image_height)
        {
          if (image_yoffset<my_height-image_height) image_yoffset=my_height-image_height;
        }
        else image_yoffset=0;
        /*}}}  */
        m_func(BIT_CLR);
        m_bitwrite(0,0,my_width,my_height);
        m_func(display_op);
        m_bitcopyto(image_xoffset,image_yoffset,image_width,image_height,0,0,WINDOW_BITMAP,IMAGE_BITMAP);
        m_flush();
        break;
      }
      /*}}}  */
    }
  } while (event[0]!='q');
  /*}}}  */
  /*{{{  exit*/
  m_bitdestroy(IMAGE_BITMAP);
  clean(0);
  /*}}}  */
  return 255;
}
/*}}}  */
