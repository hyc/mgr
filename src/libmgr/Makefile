include ../../Configfile

_CPPFLAGS=	$(CPPFLAGS) $(LIBMGRFLAGS) -DHFONTDIR=\"$(HFONTDIR)\"
CFLAGS=		$(CCFLAGS) $(_CPPFLAGS)
OBJS=		ckmgrterm.o is_mgr_term.o m_setup.o m_tty.o m_bitfile.o term.o hfont.o \
		usleep.o m_getborder.o m_gethost.o m_getscreen.o m_getfontn.o \
		m_getfonts.o m_getwindows.o m_getwindowp.o m_getfontid.o \
		m_bitc_send.o m_getdepth.o m_getevent.o \
		m_getpalette.o m_localsrv.o m_parse.o oldinfo.o

all:		libmgr.a

libmgr.a:	$(OBJS)
		$(RMF) $@
		ar cq $@ $(OBJS)

install:	libmgr.a
		install -c libmgr.a $(LIBDIR)
		ranlib $(LIBDIR)/libmgr.a
		install -c -m 644 m_getborder.3 $(MANDIR)/man3/m_getborder.3
		install -c -m 644 m_gethost.3 $(MANDIR)/man3/m_gethost.3

depend:
		$(MKDEP) $(MKDEPFLAGS) $(_CPPFLAGS) *.c > $(MKDEPOUT)

clean:
		$(RMF) $(OBJS)

clobber:	clean
		$(RMF) libmgr.a
		$(ZILCH) $(DEPFILE)

include $(DEPFILE)
