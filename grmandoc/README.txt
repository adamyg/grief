
   UNIX manpage compiler:

        The mandoc UNIX manpage compiler toolset (http://mdocml.bsd.lv/)

        Current version 1.13.4 (July 10, 2016) using sqlite3 (version 3.16.2)
        Previous version 1.12.3 (December 31, 2013)

        mandoc is a suite of tools compiling mdoc, the roff macro
        language of choice for BSD manual pages, and man, the
        predominant historical language for UNIX manuals. It is small,
        ISO C, ISC-licensed, and quite fast.

        The main component of the toolset is the mandoc utility
        program, based on the libmandoc validating compiler, to
        format output for UNIX terminals (with support for
        wide-character locales), XHTML, HTML, PostScript, and PDF.

        mandoc has predominantly been developed on OpenBSD and is
        both an OpenBSD and a BSD.lv project. We strive to support
        all interested free operating systems, in particular
        DragonFly, NetBSD, FreeBSD, Minix 3, and GNU/Linux, as well
        as all systems running the pkgsrc portable package build
        system.

   License:

        Copyright (c) 2011, 2012, 2013 - 2016 Ingo Schwarze <schwarze@openbsd.org>
        Copyright (c) 2008, 2009, 2010, 2011 Kristaps Dzonsons <kristaps@bsd.lv>

        Permission to use, copy, modify, and distribute this software for any
        purpose with or without fee is hereby granted, provided that the above
        copyright notice and this permission notice appear in all copies.

        THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHORS DISCLAIM ALL WARRANTIES
        WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
        MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR
        ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
        WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
        ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
        OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.


   Build Notes:

       o Under Cygwin the use of '-liberty' can result in getopt() crashes since
           both liberty and libc have incompatible definitions of optarg.

         Either remove liberty from the generated Makefile link line and/or
         remove getopt() from your local liberty image.

>          ar d /usr/lib/libiberty.a getopt.o getopt1.o

       o pdf support requires a c99 compliant snprintf() implementation,
           for example "%zn".

===
