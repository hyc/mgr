include ../../../Configfile

CFLAGS=		$(CPPFLAGS) $(CCFLAGS)

PRGS=		half many test_menu test_rop hfont spot event trans

all:		$(PRGS)

spot:		spot.o $(LIBDIR)/libmgr.a
		$(CC) $(LDLAGS) -o $@ $@.o $(LIBDIR)/libmgr.a

%:		%.c $(LIBDIR)/libmgr.a
		$(CC) $(CFLAGS) -o $@ $@.c $(LIBDIR)/libmgr.a

install:	all

depend:
		$(MKDEP) $(MKDEPFLAGS) $(CPPFLAGS) *.c > $(MKDEPOUT)

clean:
		$(RMF) spot.o

clobber:	clean
		$(RMF) core $(PRGS)
		$(ZILCH) $(DEPFILE)

include $(DEPFILE)
