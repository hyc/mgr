/*                        Copyright (c) 1988,1989 Bellcore
 *                            All Rights Reserved
 *       Permission is granted to copy or use this program, EXCEPT that it
 *       may not be sold for profit, the copyright notice must be reproduced
 *       on copies, and credit should be given to Bellcore where it is due.
 *       BELLCORE MAKES NO WARRANTY AND ACCEPTS NO LIABILITY FOR THIS PROGRAM.
 */
/* (MTX - sau) Commands that act as extensions to MGR */

#include <stdio.h>
#include "mtx.h"

/* simulate an MGR do_info call */

static char line[255];					/* longest response */

int
do_info(from,object,n)
register struct object *from;			/* object to send info to */
register struct object *object;		/* object requesting info (the window) */
int n;										/* info request */
	{
	register struct dest *dest;

	Dprintf('X',"do_info() -------------\n");
	/* setup dupkey */

	if (W(dup_char)) {
		line[0] = W(dup_char);
		line[1] = ' ';
		line[2] = '\0';
		}
	else
		line[0] = '\0';

	Dprintf('X',"Sending info request to %s\n",from?from->name:"OOPS");
	switch(n) {	
		case G_INPUTS:
			sprintf(line+strlen(line),"Inputs unavailable\n");
			break;
		case G_OUTPUTS:
			for(dest = from->dest;dest;dest = D(next)) {
				strcat(line,D(object->name));
				Dprintf('X',"	output: %s\n",dest->object->name);
				strcat(line," ");
				}
			line[strlen(line)-1] = '\n';
			break;
		case G_MYNAME:			/* who am i */
			strcat(line,from->name);
			Dprintf('X',"	I am: %s\n",from->name);
			break;
		default:						/* not implemented */	
			Dprintf('X',"	Invalid request\n");
			return(0);
			break;
		}
	dest = get_dest(NULL,from);
	to_dest(NULL,dest,line,strlen(line));
	free(dest);
	return(1);
	}
