/*                        Copyright (c) 1987 Bellcore
 *                            All Rights Reserved
 *       Permission is granted to copy or use this program, EXCEPT that it
 *       may not be sold for profit, the copyright notice must be reproduced
 *       on copies, and credit should be given to Bellcore where it is due.
 *       BELLCORE MAKES NO WARRANTY AND ACCEPTS NO LIABILITY FOR THIS PROGRAM.
 */

/* Debug flags	(the duplicates are unfortunate) */

char *debug_flags[] = {
/*	"*	fetching and saving bitmaps", */
	"A	Window alignment",
	"B	creating and destroying bitmaps",
	"B	setting bit-blit functions",
	"C	Cut/paste info",
/*	"E	setting and clearing window modes", */
	"F	Calls to fastscroll",
	"L	Movie logging",
	"M	menu downloads",
	"N	creating/destroying alternate client windows",
	"P	Pushing/popping environments",
	"S	Startup file operation",
	"U	obscured window updates",
	"b	keyboard meta-key (buckey) operation",
	"b	mouse button transitions",
	"c	message events",
	"d	window destruction",
	"e	event handling",
	"f	font management",
	"i	get_info requests",
	"k	color palette",
	"l	select polling",
	"m	menu calling",
	"n	new window creation",
	"o	spatial window ordering/obscuring",
	"p	data read from program destined for a window",
/*	"s	shell creation", */
	"t	ioctl TIOCSWINSZ calls",
	"u	window space de-allocation, unlinking",
	"w	calls to put_window",
	"y	yank/put buffer operation",
	"y	saving bitmaps to files",
	(char *) 0
	};
