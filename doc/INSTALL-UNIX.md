
Notes for UNIX-like platforms
=============================

 For Unix/POSIX runtime systems on Windows,
 please see the [Notes for Windows platforms](INSTALL-WINDOWS.md).

 To build and install GriefEdit, you shall need:

  * Clone of the source repository.
  
  * A supported UNIX/Linux operating system.
  
  * Perl 5 with core modules, see [NOTES-PERL.md](NOTES-PERL.md).
  
  * coreutils - Collection of file and text manipulation utilities.

  * make - _GNU (gmake) version of the 'make' utility_.

  * bison/flex - _General purpose parser generators_.         
  
  * wget - _Command-line utility for retrieving files using HTTP, HTTPS and FTP protocols_.
  
  * An ANSI C/C++ compiler, ``gcc`` is recommended being installed from gcc-core - _GNU Compiler Collection_.
  
  * A development environment in the form of development libraries and C header files.

Optionally the following external packages can be installed adding additional run-time functionality; these shall be auto-detected during 'configure'.

  * enchant-devel -     _Wrapper library for various spell checker engines_.

  * hunspell-devel -    _Spell checker and morphologic_.

  * aspell-devel -      _GNU aspell_.

  * enca-devel -        _Extremely Naive Charset Analyiser_.

  * libguess-devel -    _Speed character set detection_.

  * libexplain-devel -  _System message add-value_.

  * libarchive-devel -  _Archive access_.

  * libicu-devel -      _Character conversion_.

  * libbz2-devel -      _bzip compression_.

  * libzstd-devel -     _gz compression_.

  * xz-devel -          _liblzma_.
  
  * snappy-devel -      _Google fast/compressor/decomp_.

plus the following, which are generally available.

   * ncurses-devel -    _Terminal interface library_.
   
   * libcurl-devel -    _Network tools_.

   * openssl-devel -    _OpenSSL_.


Native builds using gcc/clang/cc
================================

  * If you're building with a non-standard compiler or one known, by something other than "cc" or "gcc" then you can set the environment variable "CC" to that.

  * Run the `configure' script by typing:

        $ ./configure_new

    Alternatively select one the available preset configurations

        $ ./support/configure_withmost
        $ ./support/configure_withcurses
        $ ./support/configure_withncurses
        $ ./support/configure_withncursesw
        $ ./support/configure_withtermcap
        $ ./support/configure_withtermlib
        $ ./support/configure_withtinfo

    Note any errors. Please get back to me via github below if you have any configure problems.

    This should built the entire set of makefiles and one include file ``include/config.h``. You may wish to examine this file since there are some settable options at the top (don't fool with what configure has done below that unless you know what you're doing).

    On completion the resulting configuration shall be presented, including installation and details on basic options; for example on the Linux host.

        -
        - Configuration:
        -
        -         Compiler: gcc / g++
        -           Depend: gcc -MM
        -           CFLAGS:   --param max-inline-insns-single=1200 -pthread
        -                     Debug:-g -Og, Release:-O1 -DNDEBUG
        -         CXXFLAGS: -g -O2 -pthread
        -                     Debug:-g -Og, Release:-O1 -DNDEBUG
        -          LDFLAGS:   -pthread
        -     Preprocessor:
        -    Mouse support: none
        -   Termap support: ncurses
        -    Spell Support: -lenchant
        -       Conversion:
        -        Detection:    -lmagic
        -         Security: -lssl -lcrypto
        -      Compression: -lbz2 -llzma -lz
        -   Other Features: -lcurl -larchive
        -        Libraries:  -lX11 -lXft -lXext -lpthread
        -            Extra: -lclang -lm -ldl -lltdl -liberty
        -        Allocator:
        -             Yacc: -ly
        -
        -  Installation:
        -
        -           prefix: /usr/local
        -          datadir: /usr/local/share
        -
        -              BINPATH=/usr/local/bin
        -              GRPATH=/usr/local/share/gr/macro
        -              GRHELP=/usr/local/share/gr/help
        -

        Review the options above for accuracy, to modify see --help for details.
        For example, a user local installation can set using the --prefix option.

            'configure --prefix=/home/user/grief'

        Once they look okay, execute to build:

            'make release'              - build software.

        Installation, one of the follow dependent on location:

            'sudo make release install' - install for common/system usage; or
        or 'make release install'      - local installation.

        Optionally after installation:

            'make release clean'        - remove build tree.

  * Once reviewed execute the following to build the application:

        $ make release

  * To install, type:

        $ make release install

    Note that the default installation paths, compiled into GriefEdit, are:

        executables         /usr/local/bin
        prefix              /usr/local/share/grief
        help files          /usr/local/share/grief/help
        compiled macros     /usr/local/share/grief/macros
        macro source        /usr/local/share/grief/src

    These can be modified, for example install within your home directory, installation base directory can set using the ``-prefix option``.

        configure --prefix=/home/user/grief     


Advanced configuration
----------------------

The ‘configure’ script supports numerous options that allow control of which functionality is enabled, where GriefEdit is installed, etc. Optionally, pass any of the following arguments (not a definitive list) to ‘configure’:

    configure [OPTION]... [VAR=VALUE]...

  * ``--help``

    Print a definitive list of options.

  * ``--prefix=<install-root-dir>``

    Set the base directory in which to install. For example:

        ./configure --prefix=/home/user/grief   

    will cause files to be installed into /home/user/grief.    

Optional Packages can be explicitly enabled or disabled.

    --with-PACKAGE[=ARG]    use PACKAGE [ARG=yes]   
    --without-PACKAGE       do not use PACKAGE (same as --with-PACKAGE=no)

  * ``--with-regex`` -              use libregex
  * ``--with-zlib`` -               enable zlib use (default=yes)
  * ``--with-lzma`` -               enable liblzma use (default=yes)
  * ``--with-bz2`` -                enable libbz2 use (default=yes)
  * ``--with-ncurses`` -            use ncurses library
  * ``--with-ncursesw`` -           use ncursesw library
  * ``--with-pdcurses`` -           use pdcurses library
  * ``--with-tinfo`` -              use tinfo library
  * ``--with-curses`` -             use curses library
  *  ``--with-termcap`` -           use termcap library
  * ``--with-termlib=library`` -    use names library for terminal support
  * ``--without-spell`` -           disable spell
  * ``--with-enchant`` -            enchant library
  * ``--with-hunspell`` -           hunspell library
  * `` --with-aspell`` -            aspell library
  * `` --with-icu`` -               enable libicu use (default=no)
  * `` --with-apriconv`` -          enable libapriconv use (default=no)
  * `` --with-iconv`` -             enable libiconv use (default=no)
  * `` --with-enca`` -              enable libenca use (default=no)
  * `` --with-magic`` -             enable libmagic use (default=yes)
  * `` --with-openssl=DIR`` -       root of the OpenSSL directory
  * `` --with-mouse`` -             enable mouse support
  * `` --with-gpm`` -               use gpm mouse library
  * `` --with-libarchive=DIR`` -    libarchive installation directory
  * `` --with-x11`` -               enable libx11 (default=yes)
  * `` --with-xft`` -               enable Xft (default=yes)
  * `` --with-xkb`` -               enable Xkb extensions (default=yes)
  * ``--with-xshape`` -             enable XShape (default=yes)

The following environment variables (not a definitive list) impact configure's behaviour. To assign environment variables (e.g., CC, CFLAGS...), specify them as VAR=VALUE.  See below for descriptions of some of the useful variables.

  * ``CFLAGS="?"``

  * ``CXXFLAGS="?"``

    Pass these flags to the C/C++ compiler. Any flags set by the configure script are prepended, which means explicitly set flags generally take precedence. Take care when specifying flags such as -Werror, because configure tests may be affected in undesirable ways.

  * ``EXTRA_CFLAGS="?"``

  * ``EXTRA_CXXFLAGS="?"``

    Append these flags to CFLAGS/CXXFLAGS, without passing them to the compiler(s) during configuration. This makes it possible to add flags such as -Werror, while allowing the configure script to determine what other flags are appropriate for the specified configuration.

  * ``CPPFLAGS="?"``

    Pass these flags to the C preprocessor. Note that CFLAGS is not passed to ‘cpp’ when ‘configure’ is looking for include files, so you must use CPPFLAGS instead if you need to help ‘configure’ find header files.

  * ``LD_LIBRARY_PATH="?"``

    ‘ld’ uses this colon-separated list to find libraries.

  * ``LDFLAGS="?"``

    Pass these flags when linking.

  * ``PATH="?"``

    ‘configure’ uses this to find programs.      
  
Last Update: May/2024

=end=