include ../../../Configfile

CFLAGS=		$(CCFLAGS) $(CPPFLAGS)
OBJS=		share.o timestamp.o

all:		$(OBJS)

install:	all
		install -c -m 644 movie.5 $(MANDIR)/man5/movie.5

depend:
		$(MKDEP) $(MKDEPFLAGS) $(CPPFLAGS) *.c > $(MKDEPOUT)

clean:
		$(RMF) $(OBJS)

clobber:	clean
		$(ZILCH) $(DEPFILE)

include $(DEPFILE)
