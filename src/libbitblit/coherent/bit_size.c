#include "screen.h"

int bit_size(int wide, int high, unsigned char depth)
{
  return ((((depth*wide+BITS)&~BITS)*high)>>3)+1;
}
