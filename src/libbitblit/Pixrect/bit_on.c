#include "screen.h"

int bit_on( bp, x, y ) register BITMAP	*bp; int x, y;
{

    if( x < 0 || x >= BIT_WIDE(bp) || y < 0 ||  y >= BIT_HIGH(bp) )
	return  0;
    return pr_get( bp, x, y);
}
