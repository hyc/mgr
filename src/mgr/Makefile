include ../../Configfile

_CPPFLAGS=	$(CPPFLAGS) $(MGRFLAGS) \
		-DFONTDIR=\"$(FONTDIR)\" -DICONDIR=\"$(ICONDIR)\" \
                -DSCREEN_DEV=\"$(SCREEN_DEV)\" -DMOUSE_DEV=\"$(MOUSE_DEV)\"

CFLAGS=		$(CCFLAGS) $(_CPPFLAGS)

OBJS=		Write.o border.o colormap.o copyright.o cut.o \
		data.o debug_flags.o \
		destroy.o do_buckey.o do_button.o do_event.o do_menu.o \
                down_load.o erase_win.o font_subs.o get_font.o get_info.o \
		get_menus.o get_rect.o get_text.o getshell.o graph_subs.o \
		icon_server.o intersect.o kbd.o mgr.o mouse_get.o move.o \
                move_box.o new_window.o print.o put_window.o scroll.o \
                set_mode.o shape.o sigdata.o startup.o subs.o update.o utmp.o \
                win_make.o win_stack.o win_subs.o write_ok.o mouse.o popen.o

all:		mgr mgrlogin

mgr:		$(OBJS)
		$(CC) $(LDFLAGS) -o mgr $(OBJS) $(LIBDIR)/libbitblit.a $(EXTRALIBS) $(CRYPTLIB)

setuid:		mgr
		chown root mgr
		chmod 04755 mgr

default_font.h:	$(DEFAULT_FONT) ../tools/fonttoc
		../tools/fonttoc < $(DEFAULT_FONT) default_font > default_font.h

../tools/fonttoc:
		(cd ../tools; make all)

icon_server.h:
		echo '/* bitmaps used by mgr server, file is created by make(1) */' > $@
		for i in $(SERVERICONDIR)/* ; do echo extern BITMAP `basename $$i`';' >> $@ ; done

icon_server.c:	../tools/bitmaptoc
		echo '/* bitmaps used by mgr server, file is created by make(1) */' > $@
		echo '#include <mgr/bitblit.h>' >> $@
		../tools/bitmaptoc $(SERVERICONDIR)/* >> $@

../tools/bitmaptoc:
		(cd ../tools; make all)

version.h:
		echo "/* compilation info for mgr, file is created by make(1) */" > version.h
		echo >> version.h
		echo "char version[]=\"MGR version $(VERSION) compiled by `whoami` at `date`.\\nSystem: `uname -a`\\n\";" >> version.h

mgrlogin:	mgrlogin.o mgrlogin_fnt.o set_mode.o
		$(CC) $(LDFLAGS) -o $@ mgrlogin.o mgrlogin_fnt.o set_mode.o $(LIBDIR)/libbitblit.a $(EXTRALIBS) $(CRYPTLIB)

mgrlogin.o:	mgrlogin.c
		$(CC) $(CFLAGS) -DMGR_BINARY=\"$(BINDIR)/mgr\" -c mgrlogin.c -o $@

mgrlogin_fnt.o:	font_subs.c
		$(CC) $(CFLAGS) -DMGRLOGIN -c font_subs.c -o $@

depend:         version.h
		$(ZILCH) default_font.h
		$(ZILCH) icon_server.h
		$(ZILCH) icon_server.c
		$(MKDEP) $(MKDEPFLAGS) $(_CPPFLAGS) *.c > $(MKDEPOUT)
		$(RMF) default_font.h icon_server.h icon_server.c

clean:
		$(RMF) $(OBJS) mgrlogin_fnt.o mgrlogin.o

clobber:	clean
		$(RMF) core mgr mgrlogin default_font.h icon_server.[ch] version.h
		$(ZILCH) $(DEPFILE)

install:	all
		install -c -s -m $(SERVER_PERM) mgr $(BINDIR)
		install -c -s -m 755 mgrlogin $(BINDIR)
		install -c -m 644 mgr.1 $(MANDIR)/man1
		install -c -m 644 font.5 $(MANDIR)/man5

include $(DEPFILE)
