/*{{{}}}*/
/*{{{  #includes*/
#include <stdio.h>
#include <mgr/mgr.h>
/*}}}  */

/*{{{  main*/
int main()
{
  int result;
  int keypress=0;
  char eventstr[80];

  eventstr[0]='\0';
  m_setup(0);
  m_ttyset();
  m_setevent(ACTIVATE,"|A\n");
  result=m_getevent(3000,&keypress,eventstr,sizeof(eventstr));
  m_ttyreset();
  printf("result: %d\n",result);
  printf("keypress: %d\n",keypress);
  printf("eventstr: <%s>\n",eventstr);
  return 0;
}
/*}}}  */
