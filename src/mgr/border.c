/*
 * draw the border around this window.             broman@nosc.mil, 1996/03
 */

#include <mgr/bitblit.h>
#include "defs.h"


#define ONE_BOX( bm, x, y, w, h, b, op)                                   \
/* top, right, bottom, left */                                            \
bit_blit( bm, (x),         (y),         (w), (b),         (op), 0, 0, 0); \
bit_blit( bm, (x)+(w)-(b), (y)+(b),     (b), (h)-(b)-(b), (op), 0, 0, 0); \
bit_blit( bm, (x),         (y)+(h)-(b), (w), (b),         (op), 0, 0, 0); \
bit_blit( bm, (x),         (y)+(b),     (b), (h)-(b)-(b), (op), 0, 0, 0);


void border( WINDOW *win, int be_fat)
{
    int both = win->borderwid;
    int out = (be_fat==BORDER_FAT)? both - 1: win->outborderwid;
    int inr = both - out;

    int clr = PUTOP(BIT_CLR,W(style));
    int set = PUTOP(BIT_SET,W(style));
    BITMAP *bdr = (W(flags)&W_ACTIVE)? W(border): W(save);
    int w = BIT_WIDE(bdr);
    int h = BIT_HIGH(bdr);

    if( both <= 0)  return;

    ONE_BOX( bdr, 0,   0,   w,         h,         out, set);
    ONE_BOX( bdr, out, out, w-out-out, h-out-out, inr, clr);
}
