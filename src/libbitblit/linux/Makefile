include ../../../Configfile

CFLAGS=		$(CCFLAGS) $(CPPFLAGS) $(BLITLIBFLAGS)

AOBJS=		do_blit.o do_mask.o do_2blit.o do_2mask.o misc.o
COBJS=		bit_blit.o bit_destroy.o bit_line.o \
		bit_on.o bit_point.o bit_size.o bit_bytescr.o \
		$(SCREEN).o $(SCREEN)_cmap.o

OBJS=		$(AOBJS) $(COBJS)

all:		$(OBJS)

install:	all

depend:		screen.h
		$(MKDEP) $(MKDEPFLAGS) $(CPPFLAGS) $(BLITLIBFLAGS) *.c > $(MKDEPOUT)

clean:
		$(RMF) $(OBJS)

clobber:	clean
		$(RMF) screen.h
		$(ZILCH) $(DEPFILE)

screen.h:
		$(LNS) $(SCREEN).h screen.h

$(COBJS):	screen.h
$(AOBJS):	asm.h
