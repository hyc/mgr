void _cld(void);
void _std(void);
void _do_mask(char *dst, int mask, int func);
void _do_2mask(char *dst, char *src, int mask, int func, int shift);
void _do_blit(char *dst, int hcnt, int func);
void _do_2blit(char *dst, char *src, int cnt, int func, int shift);
