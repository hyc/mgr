include ../../../Configfile

#CFLAGS for Video7 VGA - for standard VGA, remove -DVIDEO7 
#though Herc doesn't use -DVIDEO7 or -DRESTOR_TXT, they won't hurt anything

_CPPFLAGS=	$(CPPFLAGS) -DRESTOR_TXT
CFLAGS=		$(CCFLAGS) $(CPPFLAGS)

AOBJS=		do_blit.o do_mask.o do_2blit.o do_2mask.o misc.o port_out.o \
		port_in.o adjust.o
COBJS=		bit_blit.o bit_destroy.o bit_line.o ioperm.o mmap.o \
		bit_on.o bit_point.o bit_size.o fast_scroll.o $(SCREEN).o
OBJS=		$(AOBJS) $(COBJS)

all:		$(OBJS)
		@echo All done!

install:	all

clean:
		$(RMF) $(OBJS)

clobber:	clean
		$(RMF) screen.h

screen.h:	Makefile
		ln -s $(SCREEN).h screen.h

$(COBJS):	screen.h
$(AOBJS):	asm.h

.s.o:
	as $<
