/*                        Copyright (c) 1988 Bellcore
 *                            All Rights Reserved
 *       Permission is granted to copy or use this program, EXCEPT that it
 *       may not be sold for profit, the copyright notice must be reproduced
 *       on copies, and credit should be given to Bellcore where it is due.
 *       BELLCORE MAKES NO WARRANTY AND ACCEPTS NO LIABILITY FOR THIS PROGRAM.
 */
/*	$Header: set_colormap.c,v 1.1 88/07/08 11:56:47 sau Exp $
	$Source: /tmp/mgrsrc/demo/icon/RCS/set_colormap.c,v $
*/
static char	RCSid_[] = "$Source: /tmp/mgrsrc/demo/icon/RCS/set_colormap.c,v $$Revision: 1.1 $";

/* init a color map */

#include <pixrect/pixrect_hs.h>
#include <stdio.h>

/* first 24 colormap entries */

unsigned char red[] = {
	255, 	1,	255, 	1, 	1, 	255, 	1, 	255,
	1, 	128,	128, 	1, 	1, 	128, 	1, 	128,
	175,	255,	255, 	128,	128,	255, 	128,	255,
	};
unsigned char green[] = {
	255, 	1,	1, 	255,	1, 	255,	255,	1, 
	1, 	128,	1, 	128,	1, 	128,	128,	1, 
	175,	255,	128,	255,	128,	255,	255,	128,
	};
unsigned char blue[] = {
	255, 	1,	1, 	1, 	255,	1, 	255,	255,
	1, 	128,	1, 	1, 	128,	1, 	128,	128,
	175,	255,	128,	128,	255,	128,	255,	255,
	};

/* last 24 color map entries */

unsigned char rred[] = {
	128,	255,	128,	255,	255,	128,	100,	255,
	1,	128,	1,	128,	128,	1,	1,	128,
	1,	255,	1,	255,	255,	1,	255,	1,
	};

unsigned char rgreen[] = {
	255,	128,	128,	255,	128,	255,	100,	255,
	128,	1,	1,	128,	1,	128,	1,	128,
	255,	1,	1,	255,	1,	255,	255,	1,
	};

unsigned char rblue[] = {
	128,	128,	255,	128,	255,	255,	100,	255,
	1,	1,	128,	1,	128,	128,	1,	128,
	1,	1,	255,	1,	255,	255,	255,	1,
	};

int
main(argc,argv)
int argc; 
char *argv[];
{
	struct pixrect *screen;

	screen = pr_open("/dev/fb");
	pr_putcolormap(screen,0,24,red,green,blue);
	pr_putcolormap(screen,256-24,24,rred,rgreen,rblue);
	pr_close(screen);
	return(0);
}
