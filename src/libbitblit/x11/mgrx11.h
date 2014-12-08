/* Copyright (c) 2014 Howard Chu, hyc@symas.com
 *                            All Rights Reserved
 *       Permission is granted to copy or use this program, EXCEPT that it
 *       may not be sold for profit, the copyright notice must be reproduced
 *       on copies, and credit should be given to the author where it is due.
 *       THE AUTHOR MAKES NO WARRANTY AND ACCEPTS NO LIABILITY FOR THIS PROGRAM.
 */
#define DATA unsigned char

#include <mgr/bitblit.h>

#define LOGBITS 3
#define BITS (~(~(unsigned)0<<LOGBITS))

#define bit_linesize(wide,depth) ((((depth)*(wide)+BITS)&~BITS)>>3)

#define BIT_SIZE(m) BIT_Size(BIT_WIDE(m), BIT_HIGH(m), BIT_DEPTH(m))
#define BIT_Size(wide,high,depth) (((((depth)*(wide)+BITS)&~BITS)*(high))>>3)
#define BIT_LINE(x) ((((x)->primary->depth*(x)->primary->wide+BITS)&~BITS)>>LOGBITS)
