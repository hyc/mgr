#ifdef linux
#include "linux.c"
#endif
#ifdef __FreeBSD__
#include "freebsd.c"
#endif
#ifdef COHERENT
#include "coherent.c"
#endif
#ifdef sun
#include "sunos.c"
#endif
#ifdef hpux
#include "hpux.c"
#endif
