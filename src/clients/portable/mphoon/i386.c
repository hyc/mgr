/*
 *                    Copyrights (c) 1990-1993 by
 *        reccoware systems, Wolfgang Ocker, Unterwittelsbach
 *
 *                       All rights reserved!
 */


/*
 * s w a p _ b l o n g s
 */
void swap_blongs(p, n)
  register unsigned char *p;
  int                    n;
{
  register unsigned char c;

  n /= 4;
  
  for (; n > 0; n--, p += 4) {
    c    = p[0];
    p[0] = p[3];
    p[3] = c;

    c    = p[1];
    p[1] = p[2];
    p[2] = c;
  }
}


/*
 * s w a p _ b w o r d s
 */
void swap_bwords(p, n)
  register unsigned char *p;
  int                    n;
{
  register unsigned char c;

  n /= 2;
  
  for (; n > 0; n--, p += 2) {
    c    = p[0];
    p[0] = p[1];
    p[1] = c;
  }
}

