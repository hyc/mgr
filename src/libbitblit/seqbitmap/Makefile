include ../../../Configfile

CFLAGS=		$(CCFLAGS) $(CPPFLAGS)
OBJS=		bitmapread.o bitmapwrite.o bit_load.o bit_save.o

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
