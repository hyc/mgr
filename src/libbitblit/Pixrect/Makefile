include ../../../Configfile

CFLAGS=		$(CCFLAGS) $(CPPFLAGS) $(BLITLIBFLAGS)
BLITLIBOPT=0

OBJS=		bit_blit.o mem_rop.o bit_line.o bit_point.o bit_bytescr.o \
    bit_size.o bit_on.o bit_destroy.o $(SCREEN).o $(SCREEN)_cmap.o

all:		$(OBJS)

install:	all

clean:
		$(RMF) $(OBJS)

clobber:	clean
		$(RMF) mem_rop.c

depend:		mem_rop.c screen.h
		$(MKDEP) $(MKDEPFLAGS) $(CPPFLAGS) $(BLITLIBFLAGS) *.c >$(MKDEPOUT)

$(OBJS):	screen.h

screen.h:
		$(LNS) $(SCREEN).h screen.h

test_randops:	test_randops.o bit_blit.o mem_rop.o
		$(CC) $(CFLAGS) -o rops rops.o mem_rop.o bit_blit.o
                
test_rop:	test_rop.o bit_blit.o mem_rop.o
		$(CC) $(CFLAGS) -o test_rop test_rop.o mem_rop.o bit_blit.o
                
mem_rop.c:	mem_rop.C
		sed "s/^[$(BLITLIBOPT)]/#/" < mem_rop.C > tmp_mem_rop.c
		$(CPP) -P tmp_mem_rop.c | sed 's/^[1-9%]/#/' > mem_rop.c
		$(RMF) tmp_mem_rop.c

include $(DEPFILE)
