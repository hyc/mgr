/* Copyright (c) 2014 Howard Chu, hyc@symas.com
 *                            All Rights Reserved
 *       Permission is granted to copy or use this program, EXCEPT that it
 *       may not be sold for profit, the copyright notice must be reproduced
 *       on copies, and credit should be given to the author where it is due.
 *       THE AUTHOR MAKES NO WARRANTY AND ACCEPTS NO LIABILITY FOR THIS PROGRAM.
 */

#include "bitmap.h"

int bit_size(int wide, int high, unsigned char depth)
{
	return BIT_Size(wide,high,depth);
}
