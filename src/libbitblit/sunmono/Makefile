include ../../../Configfile

CFLAGS=		$(CCFLAGS) $(CPPFLAGS) $(BLITLIBFLAGS)
BLITLIBOPT=0

OBJS=		sun.o bit_blit.o bit_destroy.o bit_line.o bit_point.o \
		bit_bytescr.o bit_size.o bit_on.o

all:		$(OBJS)

install:	all

depend:		bit_blit.c screen.h
		$(MKDEP) $(MKDEPFLAGS) $(CPPFLAGS) $(BITBLITFLAGS) *.c >$(MKDEPOUT)

clean:
		$(RMF) $(OBJS)

clobber:	clean
		$(RMF) bit_blit.c
		$(RMF) screen.h
		$(RMF) test_randops test_rop
		$(ZILCH) $(DEPFILE)

screen.h:
		$(LNS) $(SCREEN).h screen.h

test_randops:	test_randops.o sun.o bit_blit.o
		$(CC) $(CFLAGS) -o test_randops test_randops.o bit_blit.o sun.o
                
test_rop:	test_rop.o sun.o bit_blit.o
		$(CC) $(CFLAGS) -o test_rop test_rop.o bit_blit.o sun.o
                
bit_blit.c:	bit_blit.C
		sed "s/^[$(BLITLIBOPT)]/#/" < bit_blit.C > tmp_bit_blit.c
		$(CPP) -P tmp_bit_blit.c | sed 's/^[1-9%]/#/' > bit_blit.c
		$(RMF) tmp_bit_blit.c

include $(DEPFILE)
