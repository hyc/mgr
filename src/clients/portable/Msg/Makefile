include ../../../../Configfile

CFLAGS=		$(CCFLAGS) $(CPPFLAGS) -DOLDLIBMGR

all:		client do send menus alias

client:		client.o
		$(CC) $(LDFLAGS) -o $@ client.o $(LIBDIR)/libmgr.a

do:		do.o
		$(CC) $(LDFLAGS) -o $@ do.o $(LIBDIR)/libmgr.a

menus:		menus.o
		$(CC) $(LDFLAGS) -o $@ menus.o $(LIBDIR)/libmgr.a

send:		send.o
		$(CC) $(LDFLAGS) -o $@ send.o $(LIBDIR)/libmgr.a

alias:
		echo "alias cd 'chdir \!*;menus \`pwd\`'" >  alias
		echo "echo '1,9esend %w\n'"		     >> alias
		echo "set path=(`pwd` \$$path)"		     >> alias

install:        all
		install -c -s client do menus send $(BINDIR)
		install -c -m 644 hpmgr.1 $(MANDIR)/man1

depend:
		$(MKDEP) $(MKDEPFLAGS) $(CPPFLAGS) *.c > $(MKDEPOUT)

clean:
		$(RMF) *.o

clobber:	clean
		$(RMF) core client do send menus alias
		$(ZILCH) $(DEPFILE)

include $(DEPFILE)
