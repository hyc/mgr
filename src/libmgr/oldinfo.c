/*{{{  Notes*/
/*                        Copyright (c) 1987 Bellcore
 *                            All Rights Reserved
 *       Permission is granted to copy or use this program, EXCEPT that it
 *       may not be sold for profit, the copyright notice must be reproduced
 *       on copies, and credit should be given to Bellcore where it is due.
 *       BELLCORE MAKES NO WARRANTY AND ACCEPTS NO LIABILITY FOR THIS PROGRAM.
 */

/* obsolete routines for querying the mgr server */
/*}}}  */
/*{{{  #includes*/
#include <stdlib.h>
#include <mgr/mgr.h>

/*}}}  */

/*{{{  variables*/
static char *m_fields[16];
/*}}}  */

/*{{{  get_size -- get the window size*/
int get_size(x,y,wide,high) int *x, *y, *wide, *high;
   { 
   register int count;

   if ((count = get_info(G_COORDS,m_fields)) >= 4) {
      if (x)
         *x = atoi(m_fields[0]); 
      if (y)
         *y = atoi(m_fields[1]); 
      if (wide)
         *wide = atoi(m_fields[2]); 
      if (high)
         *high = atoi(m_fields[3]); 
      return(count);
      }
   else return(-count);
   }
/*}}}  */
/*{{{  get_param -- get system parameters*/
int get_param(host,xmax,ymax,border) char *host; int *xmax, *ymax, *border;
   { 
   register int count;

   if ((count = get_info(G_SYSTEM,m_fields)) >= 4) {
      if (host)
         strcpy(host,m_fields[0]);
      if (xmax)
         *xmax = atoi(m_fields[1]); 
      if (ymax)
         *ymax = atoi(m_fields[2]); 
      if (border)
         *border = atoi(m_fields[3]); 
      return(count);
      }
   else return(-count);
   }
/*}}}  */
/*{{{  get_font -- get the font size in pixels*/
int
get_font(wide,high)
int  *wide, *high;

   { 
   register int count, result;

   if ((count = get_info(G_FONT,m_fields)) >= 3) {
      if (wide)
         *wide = atoi(m_fields[0]); 
      if (high)
         *high = atoi(m_fields[1]); 
      result = atoi(m_fields[2]); 
      return(result);
      }
   else return(-count);
   }
/*}}}  */
