include ../../../../Configfile

CFLAGS=		$(CCFLAGS) $(CPPFLAGS)

PRGS=		browse clock clock2 cut mclock window_print mgrbiff iconmail \
		iconmsgs mgrview squeeze unsqueeze mgrmsgs mgrmail \
		mgrshowfont startup close omgrmail mgrsclock mgrmode maze \
		pbmrawtomgr pgmrawtomgr ppmrawtomgr \
		set_termcap mnew mless mvi resetwin ismgrterm pilot \
		bdftomgr mgrfont2vga vgafont2mgr

all:		$(PRGS)

omgrmail:	omgrmail.c
		$(CC) $(CFLAGS) -o $@ $@.c $(LIBDIR)/libmgr.a

maze:		maze.c
		$(CC) $(CFLAGS) -o $@ $@.c $(LIBDIR)/libmgr.a

close:		close.c
		$(CC) $(CFLAGS) -o $@ $@.c $(LIBDIR)/libmgr.a

startup:	startup.c
		$(CC) $(CFLAGS) -o $@ $@.c $(LIBDIR)/libmgr.a

mgrmsgs:	mgrmsgs.c
		$(CC) $(CFLAGS) -o $@ $@.c $(LIBDIR)/libmgr.a

mgrmail:	mgrmail.c
		$(CC) $(CFLAGS) -o $@ $@.c $(LIBDIR)/libmgr.a

mgrmode:	mgrmode.c
		$(CC) $(CFLAGS) -o $@ $@.c $(LIBDIR)/libmgr.a

mgrshowfont:	mgrshowfont.c
		$(CC) $(CFLAGS) -o $@ $@.c $(LIBDIR)/libmgr.a

clock:		clock.c
		$(CC) $(CFLAGS) -o $@ $@.c $(LIBDIR)/libmgr.a

clock2:		clock2.c
		$(CC) $(CFLAGS) -o $@ $@.c $(LIBDIR)/libmgr.a

browse:		browse.c
		$(CC) $(CFLAGS) -o $@ $@.c $(LIBDIR)/libmgr.a

cut:		cut.c
		$(CC) $(CFLAGS) -o $@ $@.c $(LIBDIR)/libmgr.a

mclock:		mclock.c
		$(CC) $(CFLAGS) -o $@ $@.c $(LIBDIR)/libmgr.a

window_print:	window_print.c
		$(CC) $(CFLAGS) -o $@ $@.c $(LIBDIR)/libmgr.a

mgrbiff:	mgrbiff.c
		$(CC) $(CFLAGS) -o $@ $@.c $(LIBDIR)/libmgr.a

iconmail:	iconmail.c
		$(CC) $(CFLAGS) -o $@ $@.c $(LIBDIR)/libmgr.a

iconmsgs:	iconmsgs.c
		$(CC) $(CFLAGS) -o $@ $@.c $(LIBDIR)/libmgr.a

mgrview:	mgrview.c
		$(CC) $(CFLAGS) -o $@ $@.c $(LIBDIR)/libmgr.a

mgrsclock:	mgrsclock.c
		$(CC) $(CFLAGS) -o $@ $@.c $(LIBDIR)/libmgr.a

set_termcap:	set_termcap.c
		$(CC) $(CFLAGS) -o $@ $@.c $(LIBDIR)/libmgr.a

mnew:		mnew.c
		$(CC) $(CFLAGS) -o $@ $@.c $(LIBDIR)/libmgr.a

mless:		mless.c
		$(CC) $(CFLAGS) -o $@ $@.c $(LIBDIR)/libmgr.a

mvi:		mvi.c
		$(CC) $(CFLAGS) -o $@ $@.c $(LIBDIR)/libmgr.a

ismgrterm:	ismgrterm.c
		$(CC) $(CFLAGS) -DPROBE -o $@ $@.c $(LIBDIR)/libmgr.a

resetwin:	resetwin.c
		$(CC) $(CFLAGS) -o $@ $@.c $(LIBDIR)/libmgr.a

pilot:		pilot.c
		$(CC) $(CFLAGS) -o $@ $@.c $(LIBDIR)/libmgr.a

bdftomgr:	bdftomgr.c
		$(CC) $(CFLAGS) -o $@ $@.c

mgrfont2vga:	mgrfont2vga.c
		$(CC) $(CFLAGS) -o $@ $@.c

vgafont2mgr:	vgafont2mgr.c
		$(CC) $(CFLAGS) -o $@ $@.c

install:        all
		install -s -c $(PRGS) $(BINDIR)
		install -c -m 644 *.1 $(MANDIR)/man1

depend:
		$(MKDEP) $(MKDEPFLAGS) $(CPPFLAGS) *.c > $(MKDEPOUT)

clean:
		$(RMF) *.o

clobber:	clean
		$(RMF) core $(PRGS)
		$(ZILCH) $(DEPFILE)

include $(DEPFILE)
