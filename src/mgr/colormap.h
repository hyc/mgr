/* routines for allocating/freeing colors for use in windows */

void init_colors( BITMAP *bp);
void fill_colormap( BITMAP *bp);
void free_colors( WINDOW *win, unsigned int lo, unsigned int hi);
int allocate_color( WINDOW *win);
void findcolor( BITMAP *bp, unsigned int *co, unsigned int *r, unsigned int *g,
				  unsigned int *b, unsigned int *maxi);

