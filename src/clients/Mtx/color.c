/*                        Copyright (c) 1988,1989 Bellcore
 *                            All Rights Reserved
 *       Permission is granted to copy or use this program, EXCEPT that it
 *       may not be sold for profit, the copyright notice must be reproduced
 *       on copies, and credit should be given to Bellcore where it is due.
 *       BELLCORE MAKES NO WARRANTY AND ACCEPTS NO LIABILITY FOR THIS PROGRAM.
 */
/* manage color stuff for MTX */

#include "term.h"
#include "color.h"

static unsigned char colors[MAX_COLORS];
static unsigned char default_colors[MAX_COLORS] = {
	1,		/* COMMAND_COLOR	*/
	4,		/* DIALOG1_COLOR	*/
	5,		/* DIALOG2_COLOR	*/
	6,		/* DIALOG3_COLOR	*/
	7,		/* DIALOG4_COLOR	*/
	9,		/* DIALOG_BCOLOR	*/
	9,		/* MAIN_BCOLOR		*/
	6,		/* MAIN_COLOR		*/
	5,		/* MENU1_COLOR		*/
	1,		/* MENU2_COLOR		*/
	9,		/* MENU_BCOLOR		*/
	3,		/* MESSAGE1_COLOR	*/
	5,		/* MESSAGE2_COLOR	*/
	2,		/* MESSAGE3_COLOR	*/
	};

/* change the color map - use #'s for now */

int
change_colors(s)
char *s;
	{
	char *strpbrk();
	register int i;
	register char *p = s;

	if ((p = strpbrk(s,"0123456789")) == NULL) {	/* reset to defaults */
		strncpy(colors,default_colors,MAX_COLORS);
		return(0);
		}

	i = atoi(p);				/* get first color index */
	p = strpbrk(p," \t");

	for(;p && i<MAX_COLORS; i++) {
		if ((p =  strpbrk(p,"0123456789")) == NULL)
			return(i);
		colors[i] = 255&atoi(p);
		p = strpbrk(p," \t");
		}	
	}

/* initialize color (if any) */

static int have_color = -1;					/* true iff we have color */
static int fg_color = -1, bg_color = -1;		/* indeces into color tables */

int
init_color()
	{
	have_color = iscolor();
	strncpy(colors,default_colors,MAX_COLORS);
	return(have_color);
	}

/* set a color */

int
set_color(fg,bg)
int fg,bg;		/* color index to set */
	{
	if (have_color==0)
		return(0);

	if (fg>=0 && fg < MAX_COLORS && fg != fg_color) {
		m_fcolor(colors[fg_color=fg]);
		m_linecolor(B_SET,colors[fg]);
		}
	if (bg>=0 && bg < MAX_COLORS && bg != bg_color) {
		m_bcolor(colors[bg_color=bg]);
		}
	}
	
/* see if COLOR mgr  - should be in term.c */

int
iscolor()
	{
	if (have_color == -1)		/* 1st time only */
		have_color = (m_getdepth() > 1);
	return(have_color);
	}
