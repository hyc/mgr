/*                        Copyright (c) 1988,1989 Bellcore
 *                            All Rights Reserved
 *       Permission is granted to copy or use this program, EXCEPT that it
 *       may not be sold for profit, the copyright notice must be reproduced
 *       on copies, and credit should be given to Bellcore where it is due.
 *       BELLCORE MAKES NO WARRANTY AND ACCEPTS NO LIABILITY FOR THIS PROGRAM.
 */
/* (MTX) handle messages in the message line */

#include "term.h"
#include "mtx.h"
#include "color.h"

/***************************************************************************
 * center text in the message area
 */

#define MSGLIST_SIZE		10

char *msg_list[MSGLIST_SIZE];
static int msg_current = 0;
static int last_len = 0;		/* previous message margin length */

int
message(level,text,a,b,c)
int level;			/* message level */
char *text;
int a,b,c;			/* args to text */
	{
	char buff[80];
	char *str_save();
	int len;							/* length of string margin */
	
	sprintf(buff,text,a,b,c);
	
	/* save on message list */

	if (msg_list[msg_current])
		free(msg_list[msg_current]);
	msg_list[msg_current] = str_save(buff);
	msg_current = (msg_current+1) % MSGLIST_SIZE;

	len = (MAIN_WIDE-strlen(buff))/2;
	Dprintf('O',"Message: %s\n",buff);
#ifndef NOMSG
	M_pushwin(0,"push for message");
	set_color(MESSAGE_COLOR+level,MAIN_BCOLOR);
	if (len > last_len)
		m_clear();
	last_len = len;
	if (len > 0)
		m_move(len,0);
	m_printstr(buff);
	m_printstr("                                            ");
	M_popwin();
#endif
	m_flush();
	if (level == MSG_ERROR)		/* an error condition exists */
		set_code(1);
	return(len);
	}

/***************************************************************************
 * print last few messages
 */

int
print_msgs()
	{
	register int i, indx;

	M_pushwin(1,"Printing messages");
	set_color(MESSAGE_COLOR,MAIN_BCOLOR);
	fprintf(m_termout,"\r\nPrevious messages:\r");
	for(i=0;i<MSGLIST_SIZE;i++) {
		indx = (i+msg_current)%MSGLIST_SIZE;
		if (msg_list[indx])
			fprintf(m_termout,"\n  %s\r",msg_list[indx]);
		}
	M_popwin();
	}

/***************************************************************************
 * clear a message
 */

clear_message()
	{
	M_pushwin(0,"push to clear message");
	set_color(MESSAGE_COLOR,MAIN_BCOLOR);
	m_clear();
	M_popwin();
	m_flush();
	last_len = 0;
	}

