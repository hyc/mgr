#include "asm.h"

/*
 * misc. assembler stuff for screen access support
 */

.text

ENTRY(_cld)
	cld
	ret

ENTRY(_std)
	std
	ret
