/*{{{}}}*/
/*{{{  #includes*/
#include <mgr/mgr.h>
#include <termios.h>
/*}}}  */
/*{{{  variables*/
static struct termios tty_save[TTYMAX];
static int tty_cnt = 0;
/*}}}  */

/*{{{  m_ttyset -- set and save the terminal mode*/
int m_ttyset(void)
{
  int code;
  struct termios buff;
  
  m_flush();
  code=tcgetattr(fileno(m_termout),&tty_save[tty_cnt]);
  buff=tty_save[tty_cnt];
  buff.c_lflag=0;
  buff.c_oflag=0;
  buff.c_iflag=0;
  buff.c_cc[VTIME]=0;
  buff.c_cc[VMIN]=1;
  tcsetattr(fileno(m_termout),TCSADRAIN,&buff);
  if (tty_cnt < TTYMAX) tty_cnt++;
  return(code);
}
/*}}}  */
/*{{{  m_ttyreset -- restore the terminal mode*/
void m_ttyreset()
{
  if (tty_cnt) tty_cnt--;
  m_flush();
  tcsetattr(fileno(m_termout),TCSADRAIN,&tty_save[tty_cnt]);
}
/*}}}  */
