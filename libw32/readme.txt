
   WIN32 build support

        This directory contains a number of Unix compat header allowing
        simpler WIN32 native builds.


   WIN32 build notes

        Firstly a WIN perl distribution is required, would advise ActivePerl.

        In addition a few basic WIN32 versions of Unix tools are required,
        would advice the download and installation of the latest GnuWin32
        packages from

                http://gnuwin32.sourceforge.net/packages.html

        At the minimal the following are required

                gmake
                bison
                fileutils
                        mkdir, rmdir, rm, ls, cp, sed, awk and tar.
                wget*

             *  Required for automatic download and update of the Unicode
                reference datasets.

        Plus the following are helpful during development

                mc (Midnight Commander for Windows, see sourceforge/mcwin32)

                coreutils
                diffutils
                less
                gzip

        The following optional packages may also be built, enabling some of more
        advanced features

                libmagic
                    => libregex (prereq)
                libintl
                libiconv
                    => libcitrus (prereq)
                libarchive
                libhunspell

        see the ./contrib tree for details.

        plus the following which are generally available for direct
        download and installation.

                ICU

    Late updated 18/03/2012.

