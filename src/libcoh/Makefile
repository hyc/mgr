include ../../Configfile

CFLAGS=		$(CCFLAGS) $(CPPFLAGS)
OBJS=		cfsetispeed.o gettimeofday.o select.o

all:		libcoh.a

libcoh.a:	$(OBJS)
		$(RMF) $@
		ar qv $@ $(OBJS)

install:	libcoh.a
		install libcoh.a $(LIBDIR)
		ranlib $(LIBDIR)/libcoh.a

clean:
		$(RMF) $(OBJS)

clobber:	clean
		$(RMF) libcoh.a
