/*                        Copyright (c) 1988,1989 Bellcore
 *                            All Rights Reserved
 *       Permission is granted to copy or use this program, EXCEPT that it
 *       may not be sold for profit, the copyright notice must be reproduced
 *       on copies, and credit should be given to Bellcore where it is due.
 *       BELLCORE MAKES NO WARRANTY AND ACCEPTS NO LIABILITY FOR THIS PROGRAM.
 */
/***************************************************************************
 * MTX write data to a window (MGR terminal emulator)
 */

#include "term.h"
#include "mtx.h"

/* MGR escape sequence categories */

static char prefix[] = {				/* still in esc sequence */
	",;0123456789-+"
	};

static char down_load[] = {			/* these esc's require data downloading */
	E_FONT, E_GIMME, E_GMAP, E_SMAP, E_SNARF, E_STRING,
	E_GRUNCH, E_EVENT, E_SEND, E_BITLOAD, '\0'
	};

static char special[] = {				/* these esc's need special processing */
	E_PUSH, E_POP, E_MENU, E_MAKEWIN, E_SETMODE, E_CLEARMODE, E_GETINFO,
	E_PUTSNARF, E_BITGET, E_HALFWIN, E_GMAP, '\0'
	};

/* emulate the MGR put_window function */

int
do_sh(from,object,buff,count)
struct object *from;			/* object doing the writing */
struct object *object;		/* object to write to (must be type WINDOW) */
char *buff;						/* data to write */
int count;						/* # of chars to process */
	{
	register char c;								/* current character */
	int n;											/* last esc arg */
	char *end;										/* addr. of last char to process */
	char *index();
	char *malloc();

	end = buff + count;
	if (W(alt_id) == 0)
		Dprintf('E',"AARG, invalid alt_id for window %d\n",O(id));
	M_selectwin(W(alt_id),"in do_sh");

	/* process each character read from shell */

	while (buff<end) {
		c = *buff++;
		pr_debug(debug2,object,c);

		switch (W(state)) {
			case W_ESC:									/* in an escape sequence */
				if (index(special, c)) {			/* special handling required */
					W(state) = W_NORMAL;
					switch(c) {
						case E_POP:						/* pop the environment */
							if (W(push_count)) {
								W(push_count)--;
								do_esc(object,c);
								}
							else {
								message(MSG_WARN,"Window %d: Disabled M_POP ",
											O(id),0,0);
								}
							break;

						case E_PUSH:					/* push the environment */
							W(push_count)++;
							do_esc(object,c);
							break;

						case E_SETMODE:				/* set a mode (dupkey is special)*/
							if (W(arg_count)<1)
								do_esc(object,c);
							else {
								W(dup_char) = last_arg(object);
								Dprintf('O',"Win %d: setting dupkey to %d\r\n",
											O(id),W(dup_char));
								}
							break;

						case E_CLEARMODE:				/* clear a mode, handle dupkey */
							if (last_arg(object) != M_DUPKEY)
								do_esc(object,c);
							else  {
								W(dup_char) = '\0';
								Dprintf('O',"Win %d: clear M_DUPKEY\r\n",O(id));
								}
							break;

						case E_MAKEWIN:				/* multi-window ops */
							switch(W(arg_count)) {
								case 0:					/* select a window for output */
									if ((n=last_arg(object)) ==0) {
										n = O(id);
										Dprintf('O',"select_win(0) changed to %d\n",
												O(id));
										}
									W(alt_id) = n;
									M_selectwin(W(alt_id),"E_MAKEWIN select");
									break;
								case 1:					/* destroy a window */
									W(alt_id) = O(id);	/* iffy */
									do_esc(object,c);	/* pass-through */
									break;
								case 2:					/* no-op */
									do_esc(object,c);	/* pass-through */
									break;
								default:					/* make a new window */
									do_esc(object,c);
									enqueue(O(id)|Q_ALT,"alt_window");
									set_lock(L_BUSY);
									Dprintf('O',"%d: Making client window\n", O(id));
									break;
								}
							break;

						case E_GETINFO:				/* get info from MGR */
							n = last_arg(object);

							/* multiple line responses */

							if (n==G_ALL || n==G_ALLMINE || n==G_NOTIFY) {
								enqueue(O(id)|Q_MULTI,"multi-info");
								}

							/* our simulated responses */

							else if (n>G_BASE && do_info(from,object,n)) {
								break;
								}

							/* single line responses */

							else {
								Dprintf('X',"MGR single info %d\n",n);
								enqueue(O(id)|Q_SINGLE,"single-info");
								}
							set_lock(L_BUSY);
							do_esc(object,c);
							break;

						case E_MENU:				/* do menus, trap download case */
							if (W(arg_count)==1) {
								set_download(object,c);
								}
							else
								do_esc(object,c);	/* pass-through */
							break;

						case E_PUTSNARF:			/* not yet implemented */
							message(MSG_WARN,"Window %d: Disabled E_PUTSNARF",
										O(id),c,0);
							break;
						case E_BITGET:				/* no way to emulate reliably !! */
							message(MSG_WARN,"Window %d: Disabled E_BITGET",
									O(id),c,0);
							break;
						case E_HALFWIN:			/* window is outside our domain */
							if (W(arg_count)==3) {
								enqueue(O(id)|Q_SINGLE,"half_win");
								set_lock(L_BUSY);
								}
							do_esc(object,c);	/* pass-through */
							break;
						case E_GMAP:				/* need download & reply */
							if (W(arg_count)) {
								set_download(object,c);
								W(state) = W_REPLY;		/* will cause a reply */
								}
							else
								do_esc(object,c);	/* pass-through */
							break;
						default:							/* toss entirely */
							message(MSG_WARN,"Window %d: Disabled ESC (%c) ",
										O(id),c,0);
							break;
						}
					}

				/* downloadable data expected (still in esc processing) */

				else if (W(arg_count) && index(down_load, c)) {
					set_download(object,c);
					}

				/* stay in ESC mode */

				else if (index(prefix,c)) {		/* stay in esc */
					W(esc[W(esc_count++)]) = c;
					switch(c) {
						case E_SEP1:
						case E_SEP2:
							W(arg_count)++;
							break;
						default:
							break;
						}
					}

				/* normal esc, pass through un-altered */

				else {									/* pass through */
					W(state) &= ~W_ESC;
					do_esc(object,c);
					}
				break;

			/* downloading data */

			case W_DOWNLOAD:			/* normal download */
			case W_REPLY:				/* download with reply */
				*W(curr)++ = c;
				if (W(curr) - W(download_data) >= W(download_count)) {
					fwrite(W(esc),1,W(esc_count),m_termout);
					fwrite(W(download_data),1,W(download_count),m_termout);
					free(W(download_data));
					if (W(state)&W_REPLY) {
							enqueue(O(id)|Q_SINGLE,"down_load reply");
							set_lock(L_BUSY);
							}
					W(state) = W_NORMAL;
					}
				break;

			/* ordinary character to window */

			case W_NORMAL:
				if (c == '\033') {					/* start an escape sequence */
					W(state) |= W_ESC;
					W(esc_count) = 0;
					W(arg_count) = 0;
					W(esc[W(esc_count++)]) = c;
					}
				else
					putc(c,m_termout);				/* ordinary character */
				break;
			}
		}
	return(count);
	}

/* get ready to download data */

set_download(object,c)
struct object *object;
char c;	/* mgr command character */
	{
	int n;	/* # of chars to download */

	if ((n = last_arg(object)) > 0) {
		W(esc[W(esc_count++)]) = c;
		W(download_count) = n;
		W(curr) = W(download_data) = malloc(n);
		W(state) = W_DOWNLOAD;
		Dprintf('O',"Download for <%c> %d chars\r\n",c,n);
		}
	else {
		do_esc(object,c);
		}
	}
