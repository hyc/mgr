/*                        Copyright (c) 1988 Bellcore
 *                            All Rights Reserved
 *       Permission is granted to copy or use this program, EXCEPT that it
 *       may not be sold for profit, the copyright notice must be reproduced
 *       on copies, and credit should be given to Bellcore where it is due.
 *       BELLCORE MAKES NO WARRANTY AND ACCEPTS NO LIABILITY FOR THIS PROGRAM.
 */
/*	$Header: invert_colormap.c,v 1.1 88/07/08 11:56:37 sau Exp $
	$Source: /tmp/mgrsrc/demo/icon/RCS/invert_colormap.c,v $
*/
static char	RCSid_[] = "$Source: /tmp/mgrsrc/demo/icon/RCS/invert_colormap.c,v $$Revision: 1.1 $";

/* reverse color map */

#include <pixrect/pixrect_hs.h>
#include <stdio.h>

main(argc,argv)
int argc; 
char *argv[];
{
	register int i;
	u_char red[256],green[256],blue[256];

	struct pixrect *screen, *window, *rect;


	screen = pr_open("/dev/fb");


	pr_getcolormap(screen,0,256,red,green,blue);
	for(i=0;i<256;i++) {
		red[i] = 255-red[i];
		green[i] = 255-green[i];
		blue[i] = 255-blue[i];
		}
	pr_putcolormap(screen,0,256,red,green,blue);
	pr_close(screen);
}
