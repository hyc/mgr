WINDOW *insert_win(WINDOW *win);
int check_window(int x, int y, int dx, int dy, int fnt);
int next_windowset_id(void);
int setup_window(WINDOW *win, struct font *curr_font, int x, int y, int dx, int dy);
int make_window(BITMAP *screen, int x, int y, int dx, int dy, int fnt, char *start);
int create_window(int x, int y, int dx, int dy, int font_num,char **argv);
char *half_window(int x, int y, int dx, int dy, int font_num);
void new_window(void);
/*{{{}}}*/
