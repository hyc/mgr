.\"                        Copyright (c) 1988 Bellcore
.\"                            All Rights Reserved
.\"       Permission is granted to copy or use this program, EXCEPT that it
.\"       may not be sold for profit, the copyright notice must be reproduced
.\"       on copies, and credit should be given to Bellcore where it is due.
.\"       BELLCORE MAKES NO WARRANTY AND ACCEPTS NO LIABILITY FOR THIS PROGRAM.
.\"
.RP
.TL
\*M - C Language Application Interface
.AU
Stephen A. Uhler
.AI
Bell Communications Research
.AB
\*M (\fBm\fPana\fBg\fPe\fBr\fP) is a window system for Unix that
currently runs on Sun Workstations with SunOS and on Linux, the free
i80386/i80486 PC UN*X.  Older versions of \*M also run on Xenix,
386-Minix, Macintosh, DEC 3100, Atari ST MiNT, and the 3b1 Unix-pc.
\*M manages asynchronous updates of overlapping
windows and provides application support for a heterogeneous network
environment, i.e., many different types of computers connected by
various communications media.  The application interface enables
applications (called client programs) to be written in a variety of
programming languages, and run on different operating systems.  The
client program can take full advantage of the windowing capabilities
regardless of the type of connection to the workstation running \*M.
This document describes the C interface library for \*M which provides a
set of macros and functions that implement the stream protocol.  This
library provides client programs written in \fBC\fP with a function call
interface to \*M.  The library provides the lowest level access to \*M
functions and represents a one to one mapping to the underlying stream
protocol.
.AE
.LP
