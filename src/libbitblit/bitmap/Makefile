include ../../../Configfile

CFLAGS=		$(CCFLAGS) $(CPPFLAGS)
OBJS=		bitmaphead.o bit_create.o bit_alloc.o bit_open.o

all:		$(OBJS)

screen.h:
		$(LNS) ../$(BITBLIT)/$(SCREEN).h screen.h

install:	all
		install -c -m 644 bitmap.5 $(MANDIR)/man5

depend:		screen.h
		$(MKDEP) $(MKDEPFLAGS) $(CPPFLAGS) *.c > $(MKDEPOUT)

clean:
		$(RMF) $(OBJS)

clobber:	clean
		$(RMF) screen.h
		$(ZILCH) $(DEPFILE)

include $(DEPFILE)
