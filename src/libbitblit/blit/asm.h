/*                        Copyright (c) 1988 Bellcore
 *                            All Rights Reserved
 *       Permission is granted to copy or use this program, EXCEPT that it
 *       may not be sold for profit, the copyright notice must be reproduced
 *       on copies, and credit should be given to Bellcore where it is due.
 *       BELLCORE MAKES NO WARRANTY AND ACCEPTS NO LIABILITY FOR THIS PROGRAM.
 */
/*	$Header: asm.h,v 4.1 88/06/21 13:18:55 bianchi Exp $
	$Source: /tmp/mgrsrc/src/blit/RCS/asm.h,v $
*/
static char	h_asm_[] = "$Source: /tmp/mgrsrc/src/blit/RCS/asm.h,v $$Revision: 4.1 $";

/* asm declarations for sun asm */

/* bit operations */

#define BF_EXT(value,where,offset,width) \
			asm("	bfextu where@{offset:width},value")	

#define BF_INS(value,where,offset,width) \
			asm("	bfins	value,where@{offset:width}")	

#define BF_SET(where,offset,width) \
			asm("	bfset	where@{offset:width}")	

#define BF_CLR(where,offset,width) \
			asm("	bfclr	where@{offset:width}")	

#define BF_INV(where,offset,width) \
			asm("	bfchg	where@{offset:width}")	

/* other useful asm's */

#define LABEL(x) \
			asm("x:")

#define GOTO(x) \
			asm("	jmp	x")

#define MOVE(src,dst) \
			asm("	movl	src,dst")

#define LOOP(what,where) \
			asm("	dbf	what,where")

#define ADD(what,where) \
			asm("	addl	what,where")

#define SUB(what,where) \
			asm("	subl	what,where")

/* basic logical operations */

#define AND(what,where) \
			asm("	andl	what,where")

#define OR(what,where) \
			asm("	orl	what,where")

#define XOR(what,where) \
			asm("	eorl	what,where")

/* 3 flavors of complement */

#define NOT(what) \
			asm("	notl	what")

#define NOT_DST(what,where) \
			asm("	notl	what")

#define NOT_SRC(what,where) \
			asm("	notl	where")

/* place holder operation */

#define NOP(x,y) \
			asm("	# no operation on x,y")

/* other might be useful */

#define MOVEQ(data,dst) \
			asm("	moveq	#data,dst")

#define NEG(what) \
			asm("	negl	what")

