/*
 * ismgrterm - returns 0 (true) if tty is an MGR window.
 */

extern int is_mgr_term(void);

extern int
main()
{
  return ! is_mgr_term();
}
