#if defined(DEBUG) || defined(MOVIE)
/*
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

/* modified by SAU to be safe for root to run */

#include <sys/types.h>
#include <sys/param.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

#include "proto.h"

#define	tst(a,b)	(*mode == 'r'? (b) : (a))
#define	RDR	0
#define	WTR	1

static	int *popen_pid = NULL;
static	int nfiles = 0;

FILE *popen(const char *cmd, const char *mode)
{
	int p[2];
	int myside, hisside, pid;

	if (nfiles <= 0)
		nfiles = getdtablesize();
	if (popen_pid == NULL) {
		popen_pid = (int *)malloc(nfiles * sizeof *popen_pid);
		if (popen_pid == NULL)
			return (NULL);
		for (pid = 0; pid < nfiles; pid++)
			popen_pid[pid] = -1;
	}
	if (pipe(p) < 0)
		return (NULL);
	myside = tst(p[WTR], p[RDR]);
	hisside = tst(p[RDR], p[WTR]);
	if ((pid = vfork()) == 0) {
		/* turn off root privilages */
		int uid = getuid();
		int gid = getgid();
		setreuid(uid,uid);
		setregid(gid,gid);
		/* myside and hisside reverse roles in child */
		close(myside);
		if (hisside != tst(0, 1)) {
			dup2(hisside, tst(0, 1));
			close(hisside);
		}
		execl("/bin/sh", "sh", "-c", cmd, (char *)NULL);
		_exit(127);
	}
	if (pid == -1) {
		close(myside);
		close(hisside);
		return (NULL);
	}
	popen_pid[myside] = pid;
	close(hisside);
	return (fdopen(myside, mode));
}

int
pclose(ptr)
	FILE *ptr;
{
	int child, pid, status;
#ifdef SIG_BLOCK	/* POSIX */
	sigset_t omask, toblock;
#else			/* BSD */
	int omask;
#endif

	child = popen_pid[fileno(ptr)];
	popen_pid[fileno(ptr)] = -1;
	fclose(ptr);
	if (child == -1)
		return (-1);
#ifdef SIG_BLOCK
	(void) sigemptyset(&toblock);
	(void) sigaddset(&toblock,SIGINT);
	(void) sigaddset(&toblock,SIGQUIT);
	(void) sigaddset(&toblock,SIGHUP);
	(void) sigprocmask(SIG_BLOCK,&toblock,&omask);
#else
	omask = sigblock(sigmask(SIGINT)|sigmask(SIGQUIT)|sigmask(SIGHUP));
#endif

#ifdef WEXITSTATUS	/* POSIX */
	pid = waitpid(child,&status,0);
#else			/* BSD */
	while ((pid = wait(&status)) != child && pid != -1)
		;
#endif

#ifdef SIG_BLOCK
	(void) sigprocmask(SIG_SETMASK,&omask,(sigset_t *)NULL);
#else
	(void) sigsetmask(omask);
#endif
	return (pid == -1 ? -1 : status);
}
#endif
