/*{{{}}}*/
/*{{{  Notes*/
/*                        Copyright (c) 1987 Bellcore
 *                            All Rights Reserved
 *       This document contains proprietary information that shall
 *       be distributed or routed only within Bellcore and its
 *       authorized clients, except with written permission of Bellcore.
 */

/* muck with tty modes */
/*}}}  */
/*{{{  #includes*/
#include <unistd.h>
#include <termios.h>
#ifdef sun
#define u_char unsigned char
#define u_short unsigned short
#include <sys/time.h>
#include <sundev/vuid_event.h>
#endif

#ifndef XTABS
#define XTABS OXTABS
#endif

/*}}}  */

/*{{{  buffer for set_tty() and reset_tty()*/
static struct termios orig_tty;
/*}}}  */
/*{{{  set_tty*/
void set_tty(file) int file;
{
  struct termios buff;

  tcgetattr(file,&orig_tty);
  buff=orig_tty;
  buff.c_lflag = 0;
  buff.c_iflag = 0;
  buff.c_cflag &= ~CSIZE;
  buff.c_cflag |= CS8;
  buff.c_oflag &= ~XTABS;
  buff.c_cc[VMIN] = 1;
  buff.c_cc[VTIME] = 0;
  tcsetattr(file,TCSANOW,&buff);
}
/*}}}  */
/*{{{  reset_tty*/
void reset_tty(file) int file;
{
  tcsetattr(file,TCSANOW,&orig_tty);
}
/*}}}  */

/*{{{  buffer for save_modes() and restore_modes()*/
static struct termios termio_b;
/*}}}  */
/*{{{  save_modes*/
void save_modes(fd) int fd;
{
  tcgetattr(fd,&termio_b);
}
/*}}}  */
/*{{{  restore_modes*/
void restore_modes(fd) int fd;
{
  tcsetattr(fd,TCSANOW,&termio_b);
}
/*}}}  */

/*{{{  adjust_mode*/
void adjust_mode(disc,flags)
int disc;
int flags;
{
  termio_b.c_lflag |= flags;
}
/*}}}  */
