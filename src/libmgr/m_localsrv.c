/*
   Predicate m_localsrv() indicates whether the Mgr server's
   host machine is the same as that of the client.
   The result is also set to false in case of error.
   This function may only be called after initialization by m_setup().

   This predicate is useful for determining whether
   bitmaps can be sent to, or obtained from, the filesystem,
   instead of sending the bits through the terminal interface.
   No account is taken of setups where several hosts have access
   to the same filesystem and filesystem name space thru NFS or the like.
   The result of the query is cached so that repeat calls are fast.

   Author: Vincent Broman, broman@nosc.mil, nov 1994.
 */

#include <string.h>
#include <unistd.h>
#include <mgr/mgr.h>

#define LEN 65
static int is_local = -1;

int m_localsrv( void) {
    if( is_local < 0) {
	char myhost[LEN], mgrhost[LEN];

	is_local = (m_gethostname( mgrhost, LEN) == 0
		    && gethostname( myhost, LEN) == 0)
		    ? (strncmp( mgrhost, myhost, LEN) == 0): 0;
    }
    return is_local;
}
