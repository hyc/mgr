#ifndef _BIT_H
#define _BIT_H

#include <stdio.h>

int bitmalloc(int width, int height);
void bitfree(void);
void bitclear(void);
void bitline(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2);
void bitellipse(int x0, int y0, int rx, int ry);
void bitarc(int cx, int cy, int a, int b, int x1, int y1, int x2, int y2);
int bitpbmwrite(FILE *fp);

#ifdef MGR
int bitmgrwrite(FILE *fp);
#endif

#endif
/*{{{}}}*/
