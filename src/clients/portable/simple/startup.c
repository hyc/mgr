/*{{{}}}*/
/*{{{  Notes*/
/* generate a startup file from existing window configuration */
/*}}}  */
/*{{{  #includes*/
#include <mgr/mgr.h>
#include <stdio.h>
/*}}}  */

/*{{{  main*/
int main(int argc, char *argv[])
{
  struct window_data data;
  int oldfont,i,number,dummy;
  char name[255];

  ckmgrterm(*argv);
  m_setup(M_MODEOK);
  m_ttyset();
  /*{{{  get font information*/
  /*{{{  save old font*/
  m_getinfo(G_FONT);
  m_get();
  sscanf(m_linebuf,"%d %d %d",&dummy,&dummy,&oldfont);
  /*}}}  */
  for (i=0; i<100; i++)
  {
    m_font(i);
    m_getinfo(G_FONT);
    m_get();
    sscanf(m_linebuf,"%d %d %d",&dummy,&dummy,&number);
    strncpy(name,m_getfontname(),sizeof(name)-1); name[sizeof(name)-1]='\0';
    if (number)
    {
      m_font(oldfont);
      m_flush();
      m_ttyreset();
      printf("font %d %s\n",i,name);
      m_ttyset();
      fflush(stdout);
    }
  }
  m_font(oldfont);
  /*}}}  */
  /*{{{  get window information*/
  while (get_eachwin(&data)) if (data.num==0)
  {
    m_flush();
    m_ttyreset();
    printf("window %4d %4d %4d %4d\n",data.x,data.y,data.w,data.h);
    fflush(stdout);
    m_ttyset();
  }
  /*}}}  */
  m_flush();
  m_ttyreset();
  printf("done\n");
  fflush(stdout);
  return 0;
} 
/*}}}  */
