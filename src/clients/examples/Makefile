include ../../../Configfile

CFLAGS=		$(CCFLAGS) $(CPPFLAGS)

PRGS=		bounce cycle grav grid hilbert stringart walk oclose mgreyes

all:		$(PRGS)

bounce:
		$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $@.c $(LIBDIR)/libmgr.a

cycle:
		$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $@.c $(LIBDIR)/libmgr.a

grav:
		$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $@.c $(LIBDIR)/libmgr.a

grid:
		$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $@.c $(LIBDIR)/libmgr.a

hilbert:
		$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $@.c $(LIBDIR)/libmgr.a

walk:		walk.c
		$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $@.c $(LIBDIR)/libmgr.a

oclose:
		$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $@.c $(LIBDIR)/libmgr.a

stringart:      art_data.o stringart.o
		$(CC) $(LDFLAGS) -o $@ art_data.o stringart.o $(LIBDIR)/libmgr.a

mgreyes:	mgreyes.o
		$(CC) $(LDFLAGS) -o $@ mgreyes.o $(LIBDIR)/libmgr.a

install:        all
		install -c -s $(PRGS) $(BINDIR)
		install -c -m 644 *.1 $(MANDIR)/man1

depend:
		$(MKDEP) $(MKDEPFLAGS) $(CPPFLAGS) *.c > $(MKDEPOUT)

clean:
		$(RMF) *.o

clobber:	clean
		$(RMF) core $(PRGS)
		$(ZILCH) $(DEPFILE)

include $(DEPFILE)
