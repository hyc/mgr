/*********************************************/
/* you just keep on pushing my luck over the */
/*           BOULDER        DASH             */
/*                                           */
/*     Jeroen Houttuin, ETH Zurich, 1990     */
/*********************************************/

#include <stdio.h>
#ifdef X11
#include <X11/Xlib.h>
#include <X11/keysym.h>
#endif
#include "xbd.h"

char           *getenv();

#define NUMHIGH 20		/* Number of high scores that will be
				 * remembered */

/* Add a high score to the high score list */
void
add_score()
{
  /* Structure containing top game results */
  struct
  {
    int             score;	/* Final score */
    int             slev, elev;	/* Starting and ending level */
    char            desc[80];	/* Text description */
  }               tops[NUMHIGH], next;
  FILE           *sfile;	/* High score file */
  char            buf[200];
  register int    i;
  int             numscore, cur, numgame;

  /* Generate name of high score file */
  sprintf(buf, "%s/scores", LIB);
  /* Open high score file */
  sfile = fopen(buf, "r");
  /* Set default values for number of games and high scores */
  numscore = 0;
  numgame = 0;
  /* If file is readable, load in old high score list */
  if (sfile != NULL)
  {
    /* Extract score information from line */
    while (fgets(buf, 200, sfile) && numscore < NUMHIGH)
    {
      sscanf(buf, "%d %d %d %[^\n]", &(next.score), &(next.slev), &(next.elev),
	     next.desc);
      tops[numscore] = next;
      numscore++;
    }
    fclose(sfile);
  }
  /* Contruct the structure containing the score for this game */
  next.score = score;
  next.slev = levelstart;
  next.elev = levelnum;
#ifndef hpux
  sprintf(next.desc, "%s ", getenv("USER"));
#else
  sprintf(next.desc, "%s ", getenv("LOGNAME"));
#endif
  cur = -1;
  /* Insert new score in old high score list */
  if (numscore < NUMHIGH || tops[NUMHIGH - 1].score < next.score)
  {
    /* Iterate through high score list */
    for (i = (numscore >= NUMHIGH ? NUMHIGH - 2 : numscore - 1); i >= 0; i--)
    {
      /* Look for place for insertion */
      if (next.score > tops[i].score)
	tops[i + 1] = tops[i];	/* Move old scores down one place in list */
      else
	break;			/* Found spot for insertion */
    }
    tops[i + 1] = next;		/* Overwrite entry in high score list */
    cur = i + 1;		/* Remember where new high score was inserted */
    /* Increment the number of high scores */
    if (numscore < NUMHIGH)
      numscore++;
  }
  /* Increment and print the number of games played */
  /* Print out new high score list */
  for (i = 0; i < numscore; ++i)
  {
    /* Flag new high score with a leading > */
    if (i == cur)
      putchar('*');
    else
      putchar(' ');
    printf("%-16s- Died on level %3d. Started on level %3d.  Score: %8d.\n",
	   tops[i].desc, tops[i].elev, tops[i].slev, tops[i].score);
  }
  /* If current game did not make it to the high score list, print it */
  /* afterwords */
  if (cur == -1)
  {
    puts("You are quite disappointing:");
    printf("*%-16s- Died on level %3d. Started on level %3d.  Score: %8d.\n",
	   next.desc, next.elev, next.slev, next.score);
  }
  /* Save new high score list to score file */
  sprintf(buf, "%s/scores", LIB);
  sfile = fopen(buf, "w");
  if (sfile == NULL)
  {
    perror(buf);
    return;
  }
  for (i = 0; i < numscore; ++i)
    fprintf(sfile, "%d %d %d %s\n", tops[i].score, tops[i].slev,
	    tops[i].elev, tops[i].desc);
  fclose(sfile);
}
