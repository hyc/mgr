include ../../../Configfile

_CPPFLAGS=	$(CPPFLAGS) -DSCREEN_DEV=\"$(SCREEN_DEV)\"
CFLAGS=		$(_CPPFLAGS) $(CCFLAGS)

all:            play_data get_maps to_ascii to_binary

install:	all
		install -c -s get_maps to_ascii to_binary $(BINDIR)
		install -c -s -m $(BITBLIT_PERM) play_data $(BINDIR)
		install -c -m 644 play_data.1 get_maps.1 to_ascii.1 to_binary.1 $(MANDIR)/man1
		install -c -m 755 time_it.sh $(SHBINDIR)/time_it
		install -c -m 644 time_it.1 $(MANDIR)/man1/time_it.1

play_data:	play_data.o getshort.o
		$(CC) $(LDFLAGS) -o $@ play_data.o getshort.o $(LIBDIR)/libbitblit.a

get_maps:	get_maps.o getshort.o
		$(CC) $(LDFLAGS) -o $@ get_maps.o getshort.o $(LIBDIR)/libbitblit.a

to_ascii:	to_ascii.o getshort.o
		$(CC) $(LDFLAGS) -o $@ to_ascii.o getshort.o $(LIBDIR)/libbitblit.a

to_binary:	to_binary.o putshort.o
		$(CC) $(LDFLAGS) -o $@ to_binary.o putshort.o $(LIBDIR)/libbitblit.a

shrink:		shrink.o shrink_map.o putshort.o getshort.o
		$(CC) $(LDFLAGS) -o $@ shrink.o shrink_map.o putshort.o getshort.o $(LIBDIR)/libbitblit.a

depend:
		$(MKDEP) $(MKDEPFLAGS) $(_CPPFLAGS) *.c > $(MKDEPOUT)

clean:
		$(RMF) *.o

clobber:	clean
		$(RMF) core play_data get_maps to_ascii to_binary
		$(ZILCH) $(DEPFILE)

include $(DEPFILE)
