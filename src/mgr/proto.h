#include <sys/types.h>

#ifndef __STDC__
#ifndef __GNUC__
#define	_PROTOTYPE(_function, _params)	_function()
#endif
#endif
#ifndef _PROTOTYPE
#define	_PROTOTYPE(_function, _params)	_function _params
#endif

#ifndef __FreeBSD__
_PROTOTYPE( extern int ioctl, (int _fd, int _request, .../* void *_argp */));
_PROTOTYPE( extern int fclose, (FILE *_stream));
_PROTOTYPE( extern int fflush, (FILE *_stream));
_PROTOTYPE( extern int fprintf, (FILE *_stream, const char *_format, ...));
_PROTOTYPE( extern int sscanf, (const char *_s, const char *_format, ...));
_PROTOTYPE( extern int fputs, (const char *_s, FILE *_stream));
_PROTOTYPE( extern size_t fread, (void *_ptr, size_t _size, size_t _nmemb, FILE *_stream));
_PROTOTYPE( extern FILE *popen, (const char *_command, const char *_mode));
_PROTOTYPE( extern int pclose, (FILE *_stream));
_PROTOTYPE( extern void perror, (const char *_s));
_PROTOTYPE( extern char *strchr, (const char *_s, int _charwanted));
_PROTOTYPE( extern char *strrchr, (const char *_s, int _charwanted));
_PROTOTYPE( extern int memcmp, (const void *_s1, const void *_s2, size_t _len));
_PROTOTYPE( extern void *memcpy, (void *_dst, const void *_src, size_t _len));
_PROTOTYPE( extern void *memset, (void *_dst, int _uc, size_t _len));
#ifdef sun	/* needed for their FD_ZERO */
_PROTOTYPE( extern void bzero, (void *_dst, size_t _len));
#endif

_PROTOTYPE( extern char *crypt, (const char *key, const char *salt));
_PROTOTYPE( extern int fchmod, (int fd, mode_t mode));
_PROTOTYPE( extern int fchown, (int fd, uid_t owner, gid_t group));
_PROTOTYPE( extern int getdtablesize, (void));
_PROTOTYPE( extern int gethostname, (char *name, size_t namelen));
_PROTOTYPE( extern int initgroups, (const char *name, gid_t basegid));
_PROTOTYPE( extern int killpg, (pid_t pgrp, int sig));
_PROTOTYPE( extern int putenv, (const char *string));
#if defined(FD_SET) && defined(DST_NONE)    /* have the include files */
_PROTOTYPE( extern int select, (int width, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout));
#endif
_PROTOTYPE( extern int setregid, (gid_t rgid, gid_t egid));
_PROTOTYPE( extern int setreuid, (uid_t ruid, uid_t euid));
_PROTOTYPE( extern void usleep, (unsigned long int useconds));
_PROTOTYPE( extern int vfork, (void));
#ifdef WEXITSTATUS
_PROTOTYPE( extern pid_t waitpid, (pid_t _pid, int *statusp, int options));
#elif defined(WNOHANG)  && defined(RUSAGE_SELF)
_PROTOTYPE( extern int wait3, (int *statusp, int options, struct rusage *rusage));
#endif
#endif /* __FreeBSD__ */
