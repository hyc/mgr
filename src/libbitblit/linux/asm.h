/*
 * asm.h -- definitions for preprocessing .S assembler files
 */

#ifdef __linux__
#define __ASSEMBLY__
#include <linux/linkage.h>
#endif

#ifndef ENTRY
#define SYMBOL_NAME(X) _##X
#define SYMBOL_NAME_LABEL(X) _##X##:

#if !defined(__i486__) && !defined(__i586__)
#define __ALIGN .align 2,0x90
#else
#define __ALIGN .align 4,0x90
#endif

#define ENTRY(name) \
  .globl SYMBOL_NAME(name); \
  __ALIGN; \
  SYMBOL_NAME_LABEL(name)
#endif /* ENTRY */

#define BPW     4			/* bytes per word */
#define LBPW    2
#define ARG(x)  BPW * (x + 1)(%ebp)	/* macro to access arguments */
