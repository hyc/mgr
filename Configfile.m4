dnl{{{}}}
dnl{{{  m4 rules
changequote([,])dnl
define([check],[ifdef([$1],[define([$1],[changequote({,})]{$[]1}[changequote([,])])],[define([$1],)])dnl])dnl
dnl}}}
dnl{{{  checks
check([coherent])
check([syslinux])
check([sysfreebsd])
check([hwvga])
check([tseng4k])
check([trident])
check([s3vga])
check([hgc])
check([svga1])
check([svga2])
check([sunos])
check([movie])
check([hpux])
check([gropbm])
check([texmgr])
check([mkdep])
check([nomkdep])
check([upixrect])
check([twobuttons])
check([debugserver])
check([stretchserver])
check([extendedmenus])
check([hotkeys])
check([cutpaste])
check([thinborder])
check([banking])
check([smallbanner])
check([bwtwo])
check([cgtwo])
check([cgthree])
check([cgsix])
dnl}}}
dnl{{{  portable definitions
[VERSION=	0.69]

[INCLUDEDIR=	]WHEREHOME[/include]
[BINDIR=	]WHEREHOME[/bin$(ARCHITECTURE)]
[SHBINDIR=	]WHEREHOME[/bin]
[FONTDIR=	]WHEREHOME[/font]
[HFONTDIR=	]WHEREHOME[/hfont]
[ICONDIR=	]WHEREHOME[/icon]
[SERVERICONDIR=	]WHEREHOME[/icon/]smallbanner([small])[server]
[LIBDIR=	]WHEREHOME[/lib$(ARCHITECTURE)]
[MANDIR=	]WHEREHOME[/man]
[XBDDIR=	]WHEREHOME[/mgrbd]

[DEFAULT_FONT=	$(FONTDIR)/]DEFFONT
[EXAMPLES=	]EXAMPLES
movie([MOVIECLIENTS=movie])
gropbm([GROPBM=	gropbm])
gropbm([GROFFFONTDIR=	]WHEREGROFFFONT)
[MS=		]ROFFMS
texmgr([TEXMGR=	texmgr])
texmgr([FONTDESC=]WHEREFONTDESC)
[FONT_DPI_DEF=   -DFONT_DPI=]DEFFONTDPI
[ZILCH=		>]
[RMF=		rm -f]
[DEPFILE=	.depend]

dnl}}}
dnl{{{  linux
syslinux(
[LEX=		flex]
[CC=		gcc]
[CCFLAGS=	-Wall -O2]
[CPPFLAGS=	-I$(INCLUDEDIR) ]movie([-DMOVIE])
[MKDEP=		]mkdep([mkdep -d])nomkdep([gcc -MM -w])
[MKDEPOUT=	$(DEPFILE)]
[AWK=		awk]
[ROFF=		groff]
[ROFFDEVICE=	-Tascii]
[ROFFONLY=	-Z]
[TBL=		tbl]
[LNS=		ln -s]
[MKDIR=         install -d]

[MOUSE_DEV=	/dev/mouse]
[MGRFLAGS=	-DWHO -DVI -DKILL -DMGR_ALIGN -DMOUSE=]MOUSE[ -DTERMNAME=\"mgr-linux\"]twobuttons([ -DEMUMIDMSBUT])debugserver([ -DDEBUG])stretchserver([ -DSTRETCH])extendedmenus([ -DXMENU])hotkeys([ -DBUCKEY])cutpaste([ -DCUT])thinborder([ -DSUM_BDR=3])
[BITBLIT=	]BITBLITPKG
[SYSTEM=	]BITBLITPKG
[BLITLIBFLAGS=	-fomit-frame-pointer -fexpensive-optimizations -frerun-cse-after-loop -fstrength-reduce]tseng4k([ -DTSENG4K])trident([ -DTRIDENT])s3vga([ -DS3])banking([ -DBANKED])

[SERVER=		mgr]
[SERVER_PERM=	4755]
[BITBLIT_PERM=	4755]
dnl{{{  hercules flags
hgc([
SCREEN=		hgc
SCREEN_DEV=	720x348
])
dnl}}}
dnl{{{  vga flags
hwvga([
SCREEN=		vga
SCREEN_DEV=	]SCREENDEV)
dnl}}}
)
dnl}}}
dnl{{{  freebsd
sysfreebsd(
[LEX=		lex]
[CC=		gcc]
[CCFLAGS=	-Wall -O2]
[CPPFLAGS=	-I$(INCLUDEDIR) ]movie([-DMOVIE])
[MKDEP=		]mkdep([mkdep -d])nomkdep([gcc -MM -w])
[MKDEPOUT=	/dev/null]
[AWK=		awk]
[ROFF=		groff]
[ROFFDEVICE=	-Tascii]
[ROFFONLY=	-Z]
[TBL=		tbl]
[LNS=		ln -s]
[MKDIR=		mkdir -p]

[MOUSE_DEV=	/dev/mouse]
[MGRFLAGS=	-DWHO -DVI -DKILL -DMGR_ALIGN -DMOUSE=]MOUSE[ -DTERMNAME=\"mgr-bsd\"]twobuttons([ -DEMUMIDMSBUT])debugserver([ -DDEBUG])stretchserver([ -DSTRETCH])extendedmenus([ -DXMENU])hotkeys([ -DBUCKEY])cutpaste([ -DCUT])thinborder([ -DSUM_BDR=3])
[BITBLIT=	]BITBLITPKG
[SYSTEM=	]BITBLITPKG
[BLITLIBFLAGS=	-fomit-frame-pointer -fexpensive-optimizations -frerun-cse-after-loop -fstrength-reduce]tseng4k([ -DTSENG4K])trident([ -DTRIDENT])s3vga([ -DS3])banking([ -DBANKED])
[CRYPTLIB=	-lcrypt]

[SERVER=		mgr]
[SERVER_PERM=	4755]
[BITBLIT_PERM=	4755]
dnl{{{  hercules flags
hgc([
SCREEN=		hgc
SCREEN_DEV=	720x348
])
dnl}}}
dnl{{{  vga flags
hwvga([
SCREEN=		vga
SCREEN_DEV=	]SCREENDEV)
dnl}}}
)
dnl}}}
dnl{{{  coherent
coherent([
SHELL=		/usr/bin/ksh
LEX=		flex
CC=		gcc
EMULIB=		libcoh
LIBEMU=		$(LIBDIR)/libcoh.a
CCFLAGS=	-O2
CPPFLAGS=	-I$(INCLUDEDIR) -I$(INCLUDEDIR)/coherent ]movie([-DMOVIE])[

AWK=		gawk
ROFF=		groff
ROFFDEVICE=	-Tascii
ROFFONLY=	-Z
TBL=		tbl
LNS=		cp
MKDEP=		gcc -MM -w
MKDEPOUT=	$(DEPFILE)
MKDIR=		install -d

MOUSE_DEV=	/dev/mouse
MGRFLAGS=	-DVI -DKILL -DMGR_ALIGN -DMOUSE=]MOUSE[ -DTERMNAME=\"mgr\" -DRDWR_FD]twobuttons([ -DEMUMIDMSBUT])debugserver([ -DDEBUG])stretchserver([ -DSTRETCH])extendedmenus([ -DXMENU])hotkeys([ -DBUCKEY])cutpaste([ -DCUT])thinborder([ -DSUM_BDR=3])[
BITBLIT=	coherent
SYSTEM=		coherent
CRYPTLIB=

SERVER=		mgr
SERVER_PERM=	4755
BITBLIT_PERM=	755
]
dnl{{{  hercules flags
hgc([
SCREEN=		hgc
SCREEN_DEV=	720x348
])
dnl}}}
dnl{{{  vga flags
hwvga([
SCREEN=		vga
SCREEN_DEV=	640x480
])
dnl}}}
dnl{{{  svga1 flags
svga1([
SCREEN=		vga
SCREEN_DEV=	800x600
])
dnl}}}
dnl{{{  svga2 flags
svga2([
SCREEN=		vga
SCREEN_DEV=	1024x768
])
dnl}}}
)
dnl}}}
dnl{{{  sunos
sunos(
[LEX=		lex]
[LEXLIB=	-ll]
[CC=		gcc]
[CPP=		$(CC) -E]
[CCFLAGS=	-Wall -O2]
[CPPFLAGS=	-I$(INCLUDEDIR) -I$(INCLUDEDIR)/sun -Dsun]movie([ -DMOVIE])
[MKDEP=		]mkdep([mkdep -d])nomkdep([gcc -MM -w])
[MKDEPOUT=	$(DEPFILE)]
[AWK=		awk]
[ROFF=		nroff]
[ROFFDEVICE=]
[ROFFONLY=]
[TBL=		tbl]
[LNS=		ln -s]
[MACHDEPCLIENTS=Sun]
[MKDIR=		install -d]

[MOUSE_DEV=	/dev/mouse]
[MGRFLAGS=	-DVI -DKBD -DKILL -DMGR_ALIGN -DMOUSE=1 -DTERMNAME=\"mgr\"]twobuttons([ -DEMUMIDMSBUT])debugserver([ -DDEBUG])stretchserver([ -DSTRETCH])extendedmenus([ -DXMENU])hotkeys([ -DBUCKEY])cutpaste([ -DCUT])thinborder([ -DSUM_BDR=3])
upixrect([EXTRALIBS=	-lpixrect])

[BITBLIT=	]bwtwo([sunmono])cgtwo([colorport])cgthree([colorport])cgsix([colorport])
[SYSTEM=	]bwtwo([sunmono])cgtwo([colorport])cgthree([colorport])cgsix([colorport])
[BLITLIBFLAGS=	-DUNROLL]cgtwo([ -DCG2])cgthree([ -DCG3])cgsix([ -DCG6])upixrect([ -DPIXRECT])
[CRYPTLIB=	]

[SCREEN=	sun]
[SCREEN_DEV=	/dev/fb]

[SERVER=		mgr]
[SERVER_PERM=	755]
[BITBLIT_PERM=	755]
)
dnl}}}
dnl{{{  hp-ux
hpux([
LEX=		lex
LEXLIB=		-ll
CC=		gcc
CCFLAGS=	-Wall -O
CCFLAGS=	-I$(INCLUDEDIR) -I$(INCLUDEDIR)/hpux -Dsrandom=srand -Drandom=rand]movie([-DMOVIE])[
MKDEP=		gcc -MM -w
MKDEPOUT=	$(DEPFILE)
AWK=		awk
ROFF=		nroff
TBL=		tbl
LNS=		ln -s
MKDIR=		install -d

LIBMGRFLAGS=	-DNEED_USLEEP -DSELECT_USLEEP
SERVER=
BITBLIT=
SYSTEM=
CRYPTLIB=
])
dnl}}}
