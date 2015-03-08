#!/usr/bin/perl
# $Id: makelib.pl,v 1.77 2015/02/27 00:54:54 ayoung Exp $
# Makefile generation under WIN32 (MSVC/WATCOMC/MINGW) and DJGPP.
# -*- tabs: 8; indent-width: 4; -*-
# Automake emulation for non-unix environments.
#
#
# Copyright (c) 1998 - 2015, Adam Young.
# All rights reserved.
#
# This file is part of the GRIEF Editor.
#
# The GRIEF Editor is free software: you can redistribute it
# and/or modify it under the terms of the GRIEF Editor License.
#
# Redistributions of source code must retain the above copyright
# notice, and must be distributed with the license document above.
#
# Redistributions in binary form must reproduce the above copyright
# notice, and must include the license document above in
# the documentation and/or other materials provided with the
# distribution.
#
# The GRIEF Editor is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# License for more details.
# ==end==
#

use strict;
use warnings;

BEGIN {
    my $var = $ENV{"PERLINC"};

    if (defined($var) && -d $var) {             # import PERLINC
        my ($quoted_var) = quotemeta($var);
        push (@INC, $var)
            if (! grep /^$quoted_var$/, @INC);
    }
}

use Cwd 'realpath', 'getcwd';
use Getopt::Long;
use File::Spec;
use File::Copy;                                 # copy()
use File::Basename;
use POSIX 'asctime';
use Data::Dumper;
use Text::ParseWords;

my $CWD                     = getcwd();
my $BINPATH                 = '';
my $PERLPATH                = '';
my $BUSYBOX                 = 'busybox';
my $PROGRAMFILES            = ProgramFiles();

my $x_libw32                = 'libw32';

my %x_environment   = (
        'dj'            => {    # DJGPPP
            CC              => 'gcc',
            CXX             => 'g++',
            AR              => 'ar',
            CFLAGS          => '-g -fno-strength-reduce -I$(ROOT)/djgpp',
            CWARN           => '-W -Wall -Wshadow -Wmissing-prototypes',
            },

        'mingw'         => {    # MingW
            build_os        => 'mingw32',
            CC              => 'gcc',
            CXX             => 'g++',
            OSWITCH         => '',
            LSWITCH         => '-l',
            XSWITCH         => '-o',
            AR              => 'ar',
            RC              => 'windres -DGCC_WINDRES',
            DEFS            => '-DHAVE_CONFIG_H -DWIN32_LEAN_AND_MEAN -D_WIN32_WINNT=0x501',
            CINCLUDE        => '-I$(ROOT)/libw32',
            CFLAGS          => '-std=gnu11 -g -fno-strength-reduce',
            CXXFLAGS        => '-std=c++11 -g -fno-strength-reduce',
            CWARN           => '-W -Wall -Wshadow -Wmissing-prototypes',
            CXXWARN         => '-W -Wall -Wshadow',
            LDEBUG          => '',
            LDMAPFILE       => '-Xlinker -Map=$(MAPFILE)',
            LIBS            => '-lw32',
            EXTRALIBS       => '-lshlwapi -lpsapi -lole32 -luuid -lgdi32 '.
                                    '-luserenv -lnetapi32 -ladvapi32 -lshell32 -lWs2_32',
            LIBTHREAD       => '-lpthread',
            LIBMALLOC       => '-ldlmalloc',
            },

        'vc1200'        => {    # Visual Studio 7
            CC              => 'cl',
            COMPILERPATH    => '%VCINSTALLDIR%/bin',
            OSWITCH         => '-Fo',
            LSWITCH         => '',
            XSWITCH         => '-Fe',
            AR              => 'lib',
            CINCLUDE        => '-I$(ROOT)/libw32 -I$(ROOT)/libw32/msvc',
            CFLAGS          => '-nologo -Zi -Yd -GZ -MTd',
            CXXFLAGS        => '-nologo -Zi -Yd -GZ -MTd',
            CWARN           => '-W3',
            CXXWARN         => '-W3',
            LDEBUG          => '-nologo -Zi -GZ -MTd',
            LDMAPFILE       => '-MAP:$(MAPFILE)'
            },

        'vc1400'        => {    # 2005, Visual Studio 8
            CC              => 'cl',
            COMPILERPATH    => '%VCINSTALLDIR%/bin',
            VSWITCH         => '',
            VPATTERN        => undef,
            OSWITCH         => '-Fo',
            LSWITCH         => '',
            XSWITCH         => '-Fe',
            AR              => 'lib',
            CINCLUDE        => '-I$(ROOT)/libw32 -I$(ROOT)/libw32/msvc',
            CFLAGS          => '-nologo -Zi -RTC1 -MTd',
            CXXFLAGS        => '-nologo -Zi -RTC1 -MTd -EHsc',
            CWARN           => '-W3',
            CXXWARN         => '-W3',
            LDEBUG          => '-nologo -Zi -RTC1 -MTd',
            LDMAPFILE       => '-MAP:$(MAPFILE)'
            },

        'vc1600'        => {    # 2010, Visual Studio 10
            CC              => 'cl',
            COMPILERPATH    => '%VCINSTALLDIR%/bin',
            VSWITCH         => '',
            VPATTERN        => undef,
            OSWITCH         => '-Fo',
            LSWITCH         => '',
            XSWITCH         => '-Fe',
            AR              => 'lib',
            CINCLUDE        => '-I$(ROOT)/libw32 -I$(ROOT)/libw32/msvc',
            CFLAGS          => '-nologo -Zi -RTC1 -MTd',
            CXXFLAGS        => '-nologo -Zi -RTC1 -MTd -EHsc',
            CWARN           => '-W3',
            CXXWARN         => '-W3',
            LDEBUG          => '-nologo -Zi -RTC1 -MTd',
            LDMAPFILE       => '-MAP:$(MAPFILE)',
            },

        'wc1300'        => {    # Watcom 11
            CC              => 'wcl386',
            COMPILERPATH    => '%WATCOM%/binnt',
            VSWITCH         => '-c',
            VPATTERN        => '(Watcom .*? Version [0-9\.]+)',
            OSWITCH         => '-Fo',
            LSWITCH         => '',
            XSWITCH         => '-Fe',
            AR              => 'lib',
            CINCLUDE        => '-I$(ROOT)/libw32',
            CFLAGS          => '-nologo -showwopts -passwopts:"-q -d2" -Yd -MTd',
            CXXFLAGS        => '-nologo -showwopts -passwopts:"-q -d2" -Yd -MTd',
            CWARN           => '-W3',
            CXXWARN         => '-W3',
            LDEBUG          => '-nologo -passwopts:"-q -d2" -MTd',
            LDMAPFILE       => '-MAP:$(MAPFILE)'
            },

        'owc1900'       => {    # Open Watcom 1.9
            CC              => 'wcl386',
            COMPILERPATH    => '%WATCOM%/binnt',
            VSWITCH         => '-c',
            VPATTERN        => '(Open Watcom .*? Version [0-9\.]+)',
            ISWITCH         => '-i=',
            OSWITCH         => '-fo=',
            LSWITCH         => '',
            XSWITCH         => '-fe=',
            AR              => 'lib',
            CINCLUDE        => '-I$(ROOT)/libw32',
            CFLAGS          => '-q -j -ei -4r -d2 -hc -db -ofr -zlf -bt=nt -bm -aa',
            CXXFLAGS        => '-q -j -ei -4r -d2i    -db -ofr -zlf -bt=nt -bm -xs -xr -cc++',
            CWARN           => '-W3',
            CXXWARN         => '-W3',
            LDEBUG          => '-q -4r -d2 -hc -ofr -zlf -bt=nt -bm',
            LDMAPFILE       => '-fm=$(MAPFILE)'
            }
        );

my %win_entries     = (
        SET_MAKE            => 'MAKEFLAGS=',
        RM                  => '@BINPATH@rm.exe',
        MV                  => '@BINPATH@mv.exe',
        CP                  => '@BINPATH@cp.exe',
        TAR                 => '@BINPATH@tar.exe',
        MKDIR               => '@BINPATH@mkdir.exe',
        RMDIR               => '@BINPATH@rmdir.exe',

        ISWIN32             => 'yes',
        DEFS                => '-DHAVE_CONFIG_H -DWIN32=0x501',

        INSTALL             => '@PERLPATH@perl '."${CWD}/win32/install.pl",
        INSTALL_PROGRAM     => '@PERLPATH@perl '."${CWD}/win32/install.pl",
        INSTALL_DATA        => '@PERLPATH@perl '."${CWD}/win32/install.pl",

        RANLIB              => '@echo',
        YACC                => '@BINPATH@bison -y',
        LEX                 => '@BINPATH@flex',
        GREP                => '@BINPATH@egrep',
        AWK                 => '@BINPATH@awk',
        SED                 => '@BINPATH@sed',
        PERL                => '@PERLPATH@perl',
        LIBTOOL             => '@PERLPATH@perl '."${CWD}/win32/libtool_win32.pl",
        LT_OBJDIR           => '.libs/',
        RC                  => 'rc',

        LIBS                => '$(D_LIB)/libw32.lib',
        EXTRALIBS           => 'advapi32.lib gdi32.lib'.
                                  ' shlwapi.lib shell32.lib psapi.lib ole32.lib'.
                                  ' userenv.lib user32.lib ws2_32.lib wsock32.lib',
        LIBMALLOC           => 'libdlmalloc.lib',
        LIBOPENSSL          => ''
        );

my %x_tokens        = (
        #host, build etc
        PACKAGE             => 'GRIEF',
        PACKAGE_BUGREPORT   => '',
        PACKAGE_NAME        => '',
        PACKAGE_STRING      => '',
        PACKAGE_TARNAME     => '',
        PACKAGE_URL         => '',
        PACKAGE_VERSION     => '',
        PATH_SEPARATOR      => ';',

        build               => 'i386-pc-win32',
        build_alias         => '',
        build_cpu           => 'i386',
        build_os            => 'win32',
        build_vendor        => 'pc',

        host                => 'i386-pc-win32',
        host_alias          => '',
        host_cpu            => 'i386',
        host_os             => 'win32',
        host_vendor         => 'pc',

        target              => 'i386-pc-win32',
        target_alias        => '',
        target_cpu          => 'i386',
        target_os           => 'win32',
        target_vendor       => 'pc',

        prefix              => '',
        exec_prefix         => '',
        datarootdir         => '',

        bindir              => '',              # WIN32 implied, see edconfig.h
        sbindir             => '',
        libexecdir          => '',
        libdir              => '',
        datadir             => '',
        includedir          => '',

        ABS_ROOT            => $CWD,
        abs_top_builddir    => $CWD,
        top_build_prefix    => '',
        top_builddir        => '',
        top_srcdir          => '',

        VSWITCH             => '-v',            # e.g. gcc version 4.5.3 (GCC)
        VPATTERN            => 'version ([0-9\.]+)',
        ISWITCH             => '-I',
        OSWITCH             => '',              # object specification
        LSWITCH             => '',              # library
        XSWITCH             => '-o',            # exec specification

        #makefile
        SET_MAKE            => 'MAKEFLAGS=',
        CP                  => 'cp',
        RM                  => 'rm',
        MV                  => 'mv',
        TAR                 => 'tar',
        MKDIR               => 'mkdir',
        RMDIR               => 'rmdir',

        INSTALL             => 'install.pl',
        INSTALL_PROGRAM     => 'install.pl',
        INSTALL_DATA        => 'install.pl',

        RANLIB              => 'ranlib',
        YACC                => 'bison -y',
        GREP                => 'egrep',
        AWK                 => 'awk',
        SED                 => 'sed',
        PERL                => 'perl',
        LIBTOOL             => 'libtool',

        CDEBUG              => '',
        CWARN               => '',
        CXXDEBUG            => '',
        CXXWARN             => '',
        DEFS                => '-DHAVE_CONFIG_H',
        CINCLUDE            => '',

        LIBCURL_CPPFLAGS    => '',

        LDFLAGS             => '',
        LIBS                => '',
        LIBENCA             => '',
        LIBSPELL            => '',
        LIBYACC             => '',
        LIBICU              => '',
        LIBICONV            => '',
        LIBCURL             => '',
        LIBMAGIC            => '',
        LIBARCHIVE          => '',
        EXTRALIBS           => '',
        TERMLIB             => '',
        LIBM                => '',
        LIBX11              => '',
        LIBMALLOC           => '',
        LIBTHREAD           => ''
        );

my @x_headers       = (
        'sys/types.h',
        'sys/cdefs.h',
        'sys/bsdtypes.h',
        'sys/param.h',
        'sys/time.h',
        'sys/select.h',
        'sys/socket.h',
        'sys/wait.h',
        'sys/mman.h',
        'sys/utime.h',
        'sys/mount.h',
        'sys/stat.h',
        'sys/statfs.h',
        'sys/statvfs.h',
        'sys/vfs.h',
        'stdarg.h',
        'stdlib.h',
        'stdio.h',
        'limits.h',
        'inttypes.h',                           # c99
        'stdint.h',                             # c99
        'stdbool.h',                            # c99
        'stdatomic.h',                          # c11
        'stdalign.h',                           # c11
        'threads.h',                            # c11
        'pthread.h',                            # MINGW
        'string.h', 'strings.h',
        'errno.h',
        'wchar.h', 'wctype.h',
        'time.h',                               # TIME_WITH_SYS_TIME
        'alloca.h',                             # alloca()
        'env.h',
        'fcntl.h',
        'fenv.h',
        'float.h',
        'io.h',
        'memory.h',
        'process.h',
        'libgen.h',                             # basename(), dirname()
        'limits.h',
        'share.h',
        'signal.h',
        'utime.h',
        'wait.h',
      # 'argz.h',
      # 'lzma.h',                               # libzme
      # 'copyfile.h',

        #windows
        'windows.h',
        'wincrypt.h',

        #libw32
      # 'getopt.h',                             # NOTE, dont include (see: edgetopt.h)
        'unistd.h',
        'dirent.h',
        'dlfcn.h',                              # dlopen()
        'pwd.h',
        'grp.h',
      # 'poll.h'                                # functionality incomplete
        );

my @x_types         = (
        'inline',
        '__inline',
        '__int8',
        '__int16',
        '__int32',
        '__int64',
        'int64_t',
        'intmax_t',
        'uintmax_t',
        'intptr_t',
        'uintptr_t',
        'long long int',
        'unsigned long long int',
        'bool',
        '_bool',
      # 'mode_t',                               # TODO, need 'native' test
      # 'off_t',
      # 'pid_t',
      # 'size_t',
        'ssize_t',
      # 'uid_t',
      # 'gid_t',
      # 'fpos_t'
        );

my @x_sizes         = (
        'char',
        'short',
        'int',
        'long',
        'long long',
        'float',
        'double',
        'wchar_t',
        'void_p'
        );

my @x_functions     = (
        'putenv',
        'setenv',
        'rename',
        'bcmp', 'bzero',
        'memcmp', 'memset', 'memmove',
        'index', 'rindex',
        'strcasecmp', '__strcasecmp', 'stricmp',
        'strtoul',
        'strnlen',
        'strerror',
        'strftime',
        'strchr', 'strrchr', 'strdup',
        'strlcpy', 'strlcat',                   # BSD
        'strtof', 'strtold', 'strtoll',
        'strverscmp', '__strverscmp',
        'alloca',
        '_alloca',                              # msvc
        'isascii',
        '__isascii',                            # msvc
        'isblank',
        '__isblank',                            # msvc
        'iscsym',
        '__iscsym',                             # msvc
        'printf', 'vprintf', 'doprnt',
        'snprintf', '_snprintf', 'vsnprintf', '_vsnprintf',
        'strrchr', 'strdup',
        'asnprintf', 'vasnprintf',
        'mbrtowc', 'wcrtomb', 'wcscmp', 'wcscpy', 'wcslen', 'wctomb', 'wmemcmp', 'wmemmove', 'wmemcpy',
        'fgetpos', 'fsetpos',
      # 'fgetattr', 'flistattr', 'ftruncate', 'fchdir',
        'fseeko', 'fgetln',                     # bsd/linux extensions
        'ctime_r', 'localtime_r', 'gmtime_r', 'asctime_r',
        'mktime', 
        'timegm',                               # bsd/linux extensions
        'feclearexpect',                        # fenv.h/c99
        'round',                                # c99
        'nearbyintf',
        'va_copy', '__va_copy'                  # c99/gnu
        );

my @x_commands      = (
      # 'echo',
        'mkdir',
        'rmdir',
        'tar',
        'mv',
        'cp'
        );

my @x_libraries     = (         #local libraries -l<xxx> lib<xxx>.lib
        'vfs',
        'charudet',
        'chartable',
        'teken',
        'llist',
        'splay',
        'onigrx',                               # Oniguruma - Regular expressions library.
        'tre',                                  # TRE - The free and portable approximate regex matching library.
        'misc',
        'w32',
        'iconv',
        'citrus',
        'bsdio',
        'bsdfetch',
        'bsddb',
        'bsdmdoc',
        'dlmalloc'
        );

my @x_optlibraries  = (
        'aspell',
        'iconv',
        'intl',
        'charset',
        'expat'
        );

my @x_makefiles     = (
        '.',                                    # build order
        'libw32',
        '^contrib',
        '^contrib/makedepend',
        '^contrib/extags',
        '^contrib/libz',
        '^contrib/libbzip2',
        '^contrib/libregex',
        '^contrib/libmagic',
        '^contrib/libguess',
        '^contrib/libarchive',
        '^contrib/libcitrus',
            '!^contrib/libiconv',               # optional/recursive
            '^contrib/libintl',
        '^contrib/hunspell',
      # '^contrib/liblua',
        '!^contrib/icu',                        # optional local ICU installation
        'libvfs',
        'libchartable',
        'libcharudet',
        'libteken',
        'libonigrx',
        'libtre',
        'libmisc',
        'libllist',
        'libsplay',
        'libterm',
        'libmalloc',
        'libbsdio',
        'libbsddb',
        'libbsdfetch',
        'gr',
        'grcpp',
        'grmandoc',
        'grunch',
        'gm',
        'macsrc',
        'hlpdoc',
        'util',
        'win32/AutoUpdater'
        );

my %CONFIG_H        = (
        IS_LITTLE_ENDIAN        => '1',         # TODO
        STDC_HEADERS            => '1',
        HAVE_EIGHTBIT           => '1',
        HAVE_BCMP               => '1',
        HAVE_BZERO              => '1',
        HAVE_ENVIRON            => '1',
        HAVE_SYSERRLIST         => '1'
        );

our @HEADERS        = ();
our @EXTHEADERS     = ();
our %TYPES          = ();
our %SIZES          = ();
our %FUNCTIONS      = ();

my @INCLUDES        = ();
my @LIBS            = ();
my @EXTRALIBS       = ();
my @DLLS            = ();

my $x_tmpdir        = '.makelib';
my $x_compiler      = '';
my $x_version       = '';
my @x_include       = ();
my $x_command       = '';
my $x_signature     = undef;

my $o_keep          = 0;
my $o_version       = undef;
my $o_gnuwin32      = 'auto';
my $o_contrib       = 1;
my $o_gnulibs       = 0;

my $o_icu           = 'auto';
my $o_libhunspell   = undef;
my $o_libarchive    = undef;
my $o_libmagic      = undef;


#   Main ---
#       Mainline
#
sub Configure($$);
sub LoadContrib($$$);
sub CheckCompiler($$);
sub CheckType($$);
sub CheckSize($$);
sub CheckFunction($$);
sub CheckICUFunction($);
sub CheckCommand($$;$);
sub CheckExec($$;$);
sub ExpandENV($);
sub System($);
sub systemrcode($);
sub DumpList($$);
sub ExportPath($);
sub ImportDLL($$;$$);
sub Makefile($$$);
sub MakefileDir($);
sub Config($$);

exit &main();

sub
main()
{
    my $o_clean         = 0;
    my $o_help          = 0;

    my $ret
        = GetOptions(
                'binpath=s'     => \$BINPATH,
                'perlpath=s'    => \$PERLPATH,
                'busybox=s'     => \$BUSYBOX,
                'version=i'     => \$o_version,
                'icu=s'         => \$o_icu,
                'gnuwin32=s'    => \$o_gnuwin32,
                'gnulibs'       => \$o_gnulibs,
                'contrib'       => \$o_contrib,
                'libhunspell=s' => \$o_libhunspell,
                'libarchive=s'  => \$o_libarchive,
                'libmagic=s'    => \$o_libmagic,
                'clean'         => \$o_clean,
                'keep'          => \$o_keep,
                'help'          => \$o_help
                );

    Usage() if (!$ret || $o_help);
    Usage("expected command") if (scalar @ARGV < 1);
    Usage("unexpected arguments $ARGV[1] ...") if (scalar @ARGV > 1);

    my $cmd = $ARGV[0];

    if ('vc12' eq $cmd)         { $o_version = 1200, $cmd = 'vc' }
    elsif ('vc14' eq $cmd)      { $o_version = 1400; $cmd = 'vc' }
    elsif ('vc2005' eq $cmd)    { $o_version = 1400; $cmd = 'vc' }
    elsif ('vc2010' eq $cmd)    { $o_version = 1600; $cmd = 'vc' }
    if (! $o_version) {
        if ($cmd eq 'vc')       { $o_version = 1400; }
        elsif ($cmd eq 'wc')    { $o_version = 1300; }
        elsif ($cmd eq 'owc')   { $o_version = 1900; }
        else                    { $o_version = 0; }
    }

    if ($cmd eq 'vc' ||
            $cmd eq 'owc' || $cmd eq 'wc' ||
            $cmd eq 'dj' || $cmd eq 'mingw') {

        my $cache = "${x_tmpdir}/${cmd}.cache";

        eval {
            do "$cache" if (! $o_clean && -f $cache);
        };

        #build
        Configure($cmd, $o_version);

        foreach (@x_makefiles) {
            Makefile($cmd, $_, 'Makefile');
        }
        Makefile($cmd, 'include', 'edpackage.h');
        Config($cmd, $x_libw32);

        #cache
        open(CACHE, ">${cache}") or
                die "cannot create <$cache> : $!\n";
        $Data::Dumper::Purity = 1;
        $Data::Dumper::Sortkeys = 1;
        print CACHE Data::Dumper->Dump([\%x_tokens],   [qw(*XXTOKENS)]);
        print CACHE Data::Dumper->Dump([\%CONFIG_H],   [qw(*XXCONFIG_H)]);
        print CACHE Data::Dumper->Dump([\@HEADERS],    [qw(*XXHEADERS)]);
        print CACHE Data::Dumper->Dump([\@EXTHEADERS], [qw(*XXEXTHEADERS)]);
        print CACHE Data::Dumper->Dump([\%TYPES],      [qw(*TYPES)]);
        print CACHE Data::Dumper->Dump([\%SIZES],      [qw(*SIZES)]);
        print CACHE Data::Dumper->Dump([\%FUNCTIONS],  [qw(*FUNCTIONS)]);
        print CACHE "1;\n";
        close CACHE;

        #summary
        DumpList('INCLUDES', \@INCLUDES);
        DumpList('LIBS',     \@LIBS);
        DumpList('EXTRALIB', \@EXTRALIBS);
        DumpList('DLLS',     \@DLLS);

    } elsif ($cmd eq 'clean') {
        foreach (@x_makefiles) {
            my $dir = MakefileDir($_);
            unlink "$dir/Makefile";
        }

    } else {
        Usage("unknown command '$cmd'");
        return 1;
    }
    return 0;
}


sub
ProgramFiles
{
    my $path = $ENV{ProgramFiles};
    $path =~ s/\\/\//g;
    return $path;
}



#   Usage ---
#       Makelib command line usage.
#
sub
Usage                   # (message)
{
    print "\nmakelib @_\n\n" if (@_);
    print <<EOU;

Usage: perl makelib.pl [options] <command>

Options:
    --help                  command line help.

    --binpath=<path>        path of support binaries (gmake etc), otherwise
                            these are assumed to be in the path.

    --perlpath=<path>       PERL binary path, otherwise assumed in the path.

    --gnuwin32=<path>       gnuwin32 tool installation path.

    --contib                enable local contrib libraries (default).

    or --gnulibs            search and enable gnuwin32 libraries, using
                            gnuwin32 path.

    --libarchive=<path>     libarchive installation path.

    --libmagic=<path>       libmagic installation path.

    --icu=<path>            ICU installation path.

    --version=<version      compiler version

    --clean                 clean build, ignoring cache.

    --keep                  keep temporary file images.

Commands:
    vc[14|16]           Visual Studio C/C++ Makefiles.
    wc                  Watcom C/C++, using 'cl' interface.
    owc                 Open Watcom C/C++, using a direct interface.
    dj                  DJGPP.
    clean               clean.

EOU
    exit(42);
}


#   Config ---
#       Configuration.
#
sub
Configure($$)           # (type, version)
{
    my ($type, $version) = @_;
    my @CONTRIBINCS = ();
    my @EXTERNINCS = ();

    # paths
    if ($BINPATH) {
        $BINPATH = realpath($BINPATH);
        print "binpath:  ${BINPATH}\n";
        $BINPATH .= '/';
    }

    if ($PERLPATH) {
        $PERLPATH = realpath($PERLPATH);
        print "perlpath: ${PERLPATH}\n";
        $PERLPATH .= '/';
    }

    # environment
    my $signature =                             # ie. vc1600
            ($version ? sprintf("%s%d", $type, $version) : $type);

    (exists $x_environment{$signature}) or
        die "makelib: unknown environment $type, version $version\n";

    $x_signature = $signature;                  # active environment
    my $env = $x_environment{$signature};

    if ('dj' ne $type) {                        # WIN32 profile
        foreach my $entry (keys %win_entries) {
            $x_tokens{$entry} = $win_entries{$entry};
        }
    }

    foreach my $entry (keys %$env) {            # target profile
        $x_tokens{$entry} = $$env{$entry};
    }

    (-d $x_tmpdir || mkdir($x_tmpdir)) or
        die "makelib: unable to access/create tmpdir <$x_tmpdir> : $!\n";

    CheckCompiler($type, $env);

    # modules
    if ($o_gnuwin32 && ('auto' eq $o_gnuwin32)) {
        $o_gnuwin32 = undef;                    # TODO - search path.
    }

    if ($o_icu && ('auto' eq $o_icu)) {
        if (-d "contrib/icu/makelib.def") {     # local contrib
            $o_icu = undef;
        } else {
            my @icupkg = (
                    "pkg/icu",
                    "icu" );
            my @icudir = (
                    "./",
                    "./../",
                    "./../../",
                    "./../../../" );

            $o_icu = undef;
            foreach my $dir (@icudir) {
                foreach my $pkg (@icupkg) {
                    my $icu = $dir.$pkg;
                    if (-d $icu."./include") {
                        $o_icu = realpath($icu);
                        print "icuauto:  $o_icu\n";
                        last;
                    }
                }
                last if ($o_icu);
            }
        }
    }

    # header
    my @INCLUDE = split(/;/, $ENV{"INCLUDE"});

    push @INCLUDE, @x_include;
    foreach my $header (@x_headers) {
        my $fullpath = undef;

        print "header:   ${header} ...";
        print " " x (28 - length($header));
        foreach my $include (@INCLUDE) {
            $fullpath = "${include}/${header}";
            $fullpath =~ s/\\/\//g;
            if (-f $fullpath) {
                print "[${fullpath}]";

                push @HEADERS, $header;
                push @EXTHEADERS, $header
                    if ($include ne $x_libw32);
                $header =~ s/[\/.]/_/g;
                $header = uc($header);
                $CONFIG_H{"HAVE_${header}"} = '1';
                last;
            }
            $fullpath = undef;
        }
        print "[not found]" if (! defined $fullpath);
        print "\n";
    }

    # types
    foreach my $name (@x_types) {
        my $cached = (exists $TYPES{$name});
        my $status = ($cached ? $TYPES{$name} : -1);

        print "type:     ${name} ...";
        print " " x (28 - length($name));
        if (1 == $status ||
                (-1 == $status && 0 == CheckType($type, $name))) {
            $TYPES{$name} = 1;
            $name = uc($name);
            $name =~ s/ /_/g;
            $CONFIG_H{"HAVE_${name}"} = 1;
            print ($cached ? "[yes, cached]" : "[yes]");
        } else {
            $TYPES{$name} = 0;
            print ($cached ? "[no, cached]" : "[no]");
        }
        print "\n";
    }

    # size
    foreach my $name (@x_sizes) {
        my $cached = (exists $SIZES{$name});
        my $status = ($cached ? $SIZES{$name} : -1);

        print "size:     ${name} ...";
        print " " x (28 - length($name));
        if ($status > 0 ||
                (-1 == $status && ($status = CheckSize($type, $name)) > 0)) {
            $SIZES{$name} = $status;
            $name = uc($name);
            $name =~ s/ /_/g;
            $CONFIG_H{"SIZEOF_${name}"} = $status;
            print ($cached ? "[$status, cached]" : "[$status]");
        } else {
            $SIZES{$name} = 0;
            print ($cached ? "[unknown, cached]" : "[unknown]");
        }
        print "\n";
    }

    # functions
    foreach my $name (@x_functions) {
        my $cached = (exists $FUNCTIONS{$name});
        my $status = ($cached ? $FUNCTIONS{$name} : -1);

        print "function: ${name} ...";
        print " " x (28 - length($name));
        if (1 == $status ||
                (-1 == $status && 0 == CheckFunction($type, $name))) {
            $FUNCTIONS{$name} = 1;
            $name = uc($name);
            $CONFIG_H{"HAVE_${name}"} = 1;
            print ($cached ? "[yes, cached]" : "[yes]");
        } else {
            $FUNCTIONS{$name} = 0;
            print ($cached ? "[no, cached]" : "[no]");
        }
        print "\n";
    }

    # compiler/environment
    if ($type eq 'vc' || $type eq 'wc' || $type eq 'owc') {
        my $gnuwin32lib = undef;
        my $gnuwin32inc = undef;

        #gnuwin32 native builds
        if ($o_gnuwin32 && $o_gnulibs) {
            $o_gnuwin32 =~ s/\\/\//g;

            print "gnuwin32: ${o_gnuwin32}\n";
            $gnuwin32inc = "${o_gnuwin32}/include";
            $gnuwin32lib = "${o_gnuwin32}/lib";

            if (-d $gnuwin32inc) {
                $o_libarchive = $o_gnuwin32 if (! $o_libarchive);
                $o_libmagic = $o_gnuwin32 if (! $o_libmagic);
                push @EXTERNINCS, $gnuwin32inc;
            }

            foreach my $lib (@x_optlibraries) {
                my $LIBNAME = uc($lib);

                if (-f "${gnuwin32lib}/lib${lib}.lib" ||
                        -f "${gnuwin32lib}/${lib}.lib") {
                                                # enable work arounds, see source
                    $CONFIG_H{"GNUWIN32_LIB${LIBNAME}"} = '1';
                    $CONFIG_H{"HAVE_LIB${LIBNAME}"} = '1';
                    $CONFIG_H{"HAVE_${LIBNAME}_H"} = '1'
                        if (-f "${gnuwin32inc}/${lib}.h");
                    ImportDLL("${o_libarchive}/bin", \@DLLS, $lib);
                }
            }
        }

        #contrib tree
        my %contribs;

        if ($o_contrib) {
            foreach (@x_makefiles) {
                my $dir = MakefileDir($_);

                if (-f "${dir}/makelib.def") {
                    my $name = basename($dir);
                    LoadContrib($name, $dir, \@CONTRIBINCS);
                    $contribs{$name} = 1;
                }
            }
        }

        #libarchive
        if (! exists $contribs{libarchive}) {
            $o_libarchive =~ s/\\/\//g;
            my $libarchivelib = "${o_libarchive}/lib";
            my $libarchiveinc = "${o_libarchive}/include";

            if (-d $libarchiveinc && -d $libarchivelib) {
                my @libarchives = ('libarchive', 'archive');
                my $LIBARCHIVE = undef;

                foreach my $lib (@libarchives) {
                    if (-f "${libarchivelib}/${lib}.lib") {
                        $LIBARCHIVE = "${libarchivelib}/${lib}.lib";
                        last;
                    }
                }

                if ($LIBARCHIVE) {
                    print "archive:  ${o_libarchive}\n";
                    $CONFIG_H{'HAVE_LIBARCHIVE'} = '1';
                    $CONFIG_H{"GNUWIN32_LIBARCHIVE"} = '1'
                        if ($libarchivelib eq $gnuwin32lib);
                    $CONFIG_H{'HAVE_ARCHIVE_H'} = '1'
                        if (-f "${libarchiveinc}/archive.h");
                    $x_tokens{LIBARCHIVE} = ExportPath($LIBARCHIVE);
                    ImportDLL("${o_libarchive}/bin", \@DLLS, "archive");
                }
                push @EXTERNINCS, $libarchiveinc;
            }
        }

        #libhunspell
        if (! exists $contribs{hunspell}) {
            $o_libhunspell =~ s/\\/\//g;
            my $libhunspelllib = "${o_libhunspell}/lib";
            my $libhunspellinc = "${o_libhunspell}/include";

            if (-d $libhunspellinc && -d $libhunspelllib) {
                my @libhunspells = ('libhunspell', 'hunspell');
                my $LIBHUNSPELL = undef;

                foreach my $lib (@libhunspells) {
                    if (-f "${libhunspelllib}/${lib}.lib") {
                        $LIBHUNSPELL = "${libhunspelllib}/${lib}.lib";
                        last;
                    }
                }

                if ($LIBHUNSPELL) {
                    print "hunspell:  ${o_libhunspell}\n";
                    $CONFIG_H{'HAVE_LIBHUNSPELL'} = '1';
                    $CONFIG_H{"GNUWIN32_LIBHUNSPELL"} = '1'
                        if ($libhunspelllib eq $gnuwin32lib);
                    $CONFIG_H{'HAVE_HUNSPELL_H'} = '1'
                        if (-f "${libhunspellinc}/hunspell.h");
                    $x_tokens{LIBHUNSPELL} = ExportPath($LIBHUNSPELL);
                    ImportDLL("${o_libhunspell}/bin", \@DLLS, "hunspell");
                }
                push @EXTERNINCS, $libhunspellinc;
            }
        }

        #libmagic
        if (! exists $contribs{libmagic}) {
            $o_libmagic =~ s/\\/\//g;
            my $libmagiclib = "${o_libmagic}/lib";
            my $libmagicinc = "${o_libmagic}/include";

            if (-d $libmagicinc && -d $libmagiclib) {
                my @libmagics = ('libmagic.lib', 'magic.lib');
                my $LIBMAGIC = '';

                foreach my $lib (@libmagics) {
                    if (-f "${libmagiclib}/${lib}") {
                        $LIBMAGIC = "${libmagiclib}/${lib}";
                        last;
                    }
                }

                if ($LIBMAGIC) {
                    print "magic:    ${o_libmagic}\n";
                    $CONFIG_H{'HAVE_LIBMAGIC'} = '1';
                    $CONFIG_H{"GNUWIN32_LIBMAGIC"} = '1'
                        if ($libmagiclib eq $gnuwin32lib);
                    $CONFIG_H{'HAVE_MAGIC_H'} = '1'
                        if (-f "${libmagicinc}/magic.h");
                    $x_tokens{LIBMAGIC} = ExportPath($LIBMAGIC);
                    ImportDLL("${o_libmagic}/bin", \@DLLS, "magic");
                }
                push @EXTERNINCS, $libmagicinc;
            }
        }

        # ICU
        if (! exists $contribs{icu} && $o_icu) {
            $o_icu =~ s/\\/\//g;

            if (-f "${o_icu}/makelib.def") {
                my $name = basename($o_icu);
                LoadContrib($name, $o_icu, \@CONTRIBINCS);
                $contribs{'icu'} = 1;

            } else {
                my $iculibpath = "${o_icu}/lib";
                my $icuinc = "${o_icu}/include";

                if (-d $icuinc && -d $iculibpath) {
                    my @libicus = (
                            'icudt.lib',    # data/rsource
                            'icuin.lib',    # i18n
                            'icuio.lib',
                            'icule.lib',
                            'iculx.lib',
                            'icutu.lib',
                            'icuuc.lib'     # common
                            );
                    my $iculib = '';
                    my $LIBICU = '';

                    $CONFIG_H{'HAVE_LIBICU'} = '1';
                    push @EXTERNINCS, $icuinc;
                    foreach my $lib (@libicus) {
                        $LIBICU .= ExportPath("$iculibpath/$lib");
                        $iculib .= "$iculibpath/$lib ";
                    }
                    $$env{ICUINCLUDE} = $icuinc;
                    $$env{ICULIB} = $iculib;
                    ImportDLL("${o_icu}/bin", \@DLLS, "icu", 1);

                    print "icu:      ${o_icu}\n";
                    if ((my $icuver = CheckICUFunction("test1")) >= 40) {
                        $CONFIG_H{'HAVE_LIBICU'} = '1';
                        $CONFIG_H{"HAVE_LIBICU_${icuver}"} = '1';
                        $x_tokens{LIBICU} = $LIBICU;
                        printf "\tICU %d.%d.x available ...\n", $icuver / 10, $icuver % 10;
                        print "\tdef: HAVE_LIBICU\n";
                        print "\tdef: HAVE_LIBICU_${icuver}\n";
                        print "\tinc: $icuinc\n";
                        print "\tlib: $iculib".'(@LIBICU@)'."\n";
                    } elsif ($icuver > 10) {
                        printf "\tUnsupported ICU version %d.%d\n", $icuver / 10, $icuver % 10;
                    } else {
                        print "\tICU either not found or not available, check log for details ($icuver)\n";
                    }
                }
            }
        }
    }

    my %T_INCS;
    push @CONTRIBINCS, @EXTERNINCS;
    foreach my $inc (@CONTRIBINCS) {
        $inc = cannon_path($inc);
        if (! exists $T_INCS{$inc}) {
            $x_tokens{CINCLUDE} .= " -I${inc}";
            push @INCLUDES, $inc;
            $T_INCS{$inc} = 1;
        }
    }

    if (scalar @LIBS) {
        $x_tokens{LIBS} .= ' ';
        foreach my $lib (@LIBS) {
            $x_tokens{LIBS} .= ExportPath($lib);
        }
    }
}


sub
LoadContrib($$$)        # (name, dir, refIncludes)
{
    my ($name, $dir, $refIncludes) = @_;
    my $basepath = ($dir ? $dir : "contrib/${name}");
    my $def = "${basepath}/makelib.def";
    my $lbl = "HAVE_".uc($name);

    return 0 if (-f $basepath);

    print "contrib:  $basepath\n";

    open(CFG, "<${def}") or
        die "cannot open <$def> : $!\n";
    while (defined (my $line = <CFG>)) {
        $line =~ s/\s*([\n\r]+|$)//;
        next if (!$line || /^\s#/);

        my @parts = split(/=/, $line, 2);
        if (2 == scalar @parts) {
            my ($key, $val) = @parts;

            if ('inc' eq $parts[0]) {
                $val = "${basepath}/".$parts[1]
                    if ($val !~ /^\//);
                push @$refIncludes, '$(ROOT)/'.$val;
                print "\tinc: $val\n";

            } elsif ('lbl' eq $key) {
                $lbl = uc($val);

            } elsif ('lib' eq $key) {
                $x_tokens{$lbl} = ExportPath($val);
                print "\tlib: $val (\@$lbl\@)\n";

            } elsif ('ext' eq $key) {
                $x_tokens{$lbl} .= ' '.ExportPath($val);
                print "\text: $val (\@$lbl\@)\n";

            } elsif ('def' eq $key) {
                if ($val =~ /^(.+)=(.*)$/) {
                    $CONFIG_H{$1} = ($2 ? $2 : '1');
                } else {
                    $CONFIG_H{$val} = '1';
                }
                print "\tdef: $val\n";
            }
        }
    }
    close(CFG);
    return 1;
}


sub
CheckCompiler($$)       # (type, env)
{
    my ($type, $env) = @_;

    $x_compiler  = ExpandENV($$env{COMPILERPATH}).'/'
        if (exists $$env{COMPILERPATH});
    $x_compiler .= $$env{CC};
    $x_compiler =~ s/\//\\/g;
    $x_command   = "\"$x_compiler\" ";

    (-1 != System("$x_command junk-command-line >${x_tmpdir}/compiler.out 2>&1")) or
        die "makelib: unable to access compiler <$x_compiler>\n";

    if (exists $$env{VPATTERN}) {               # version information
        my $vpattern = $$env{VPATTERN};
        my $vswitch = (exists $$env{VSWITCH} ? $$env{VSWITCH} : '');
        my @vtext = ();

        (-1 != System("$x_command $vswitch >${x_tmpdir}/version.out 2>&1")) or
            die "makelib: unable to access compiler <$x_compiler $vswitch>\n";

        open(VERSION, "${x_tmpdir}/version.out") or
            die "makelib: cannot open <${x_tmpdir}/version.out> : $!";
        while (<VERSION>) {
            $_ =~ s/\s*(\n|$)//;
            push @vtext, $_;
        }
        close VERSION;

        if (defined $vpattern) {                # pattern
            my $vflat = "@vtext";
            $vflat =~ /$vpattern/i;
            $x_version = $1;
        } else {                                # first list
            $x_version = $vtext[0];
        }
    }

    my @CINCLUDE = split(/ /, $$env{CINCLUDE});

    foreach (@CINCLUDE) {
        if (/^-I\$\(ROOT\)(.*)$/) {
            push @x_include, "${CWD}$1";
        } elsif (/^-I(.*)$/) {
            push @x_include, $1;
        } else {
            push @x_include, $_;
        }
    }

    my $includes = '';
    $includes .=                                # <edidentifier.h>
         (exists $$env{ISWITCH} ? $$env{ISWITCH} : '-I')."${CWD}/include ";
    foreach (@x_include) {
        $includes .=                            # implied
           (exists $$env{ISWITCH} ? $$env{ISWITCH} : '-I').$_.' ';
    }

    $CONFIG_H{GRIEF_CC_COMPILER} = "\"".basename(${x_compiler})."\"";
    $CONFIG_H{GRIEF_CC_VERSION} = "\"${x_version}\"";

    print "compiler: ${x_compiler}\n";
    print "version:  ${x_version}\n";
    print "command:  ${x_command}\n";
    print "includes: ${includes}\n";

    # build final command
    $x_command  .= "__FLAGS__ ";
    $x_command  .= $includes;
    $x_command  .= "$$env{OSWITCH}__OBJ__ "
        if ($$env{OSWITCH} ne '');
    $x_command  .= "__LIB__ ";                  # lib's
    $x_command  .= "$$env{XSWITCH}__EXE__ ";
    $x_command  .= "__SOURCE__ >__BASE__.out 2>&1";
    $x_command  =~ s/\//\\/g;

    $$env{CXX}  = $$env{CC}
        if (! exists $$env{CXX});
}


sub
CheckType($$)           # (type, name)
{
    my ($type, $name) = @_;

    my $t_name = $name;
    $t_name =~ s/ /_/g;

    my $BASE   = "${type}_${t_name}";
    my $SOURCE = "${BASE}.c";
    my ($cmd, $cmdparts)
              = CheckCommand($BASE, $SOURCE);

    my $asctime = asctime(localtime());
    chop($asctime);
    open(TMP, ">${x_tmpdir}/$SOURCE") or
            die "cannot create ${x_tmpdir}/$SOURCE : $!\n";
    print TMP<<EOT;
/*
 *  Generated by makelib.pl, $asctime
$cmdparts
 */
#if defined(__STDC__)
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#endif
EOT

    foreach my $header (@HEADERS) {
        print TMP "#include <$header>\n";       # list should be more selective
    }

    if ($t_name =~ /inline/) {
        print TMP<<EOT;
static ${name} function(void) {
    return 1;
}
int main(int argc, char **argv) {
    return function();
}
EOT

    } else {
        print TMP<<EOT;
static ${name} name;
int main(int argc, char **argv) {
    name = 0;
    return 1;
}
EOT
    }
    close TMP;

    return CheckExec($BASE, $cmd);
}


sub
CheckSize($$)           # (type, name)
{
    my ($type, $name) = @_;

    my $t_name = $name;
    $t_name =~ s/ /_/g;

    my $BASE   = "${type}_${t_name}";
    my $SOURCE = "${BASE}.c";
    my ($cmd, $cmdparts)
               = CheckCommand($BASE, $SOURCE);

    $name = 'void *'
        if ($name eq 'void_p');

    my $asctime = asctime(localtime());
    chop($asctime);
    open(TMP, ">${x_tmpdir}/$SOURCE") or
            die "cannot create ${x_tmpdir}/$SOURCE : $!\n";
    print TMP<<EOT;
/*
 *  Generated by makelib.pl, $asctime
$cmdparts
 */
#if defined(__STDC__)
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#endif
int main(int argc, char **argv) {
    return sizeof($name);
}
EOT
    close TMP;

    return CheckExec($BASE, $cmd, 1);
}


#   Function: CheckFunction
#       Check that the function exists
#   Returns:
#       0 on success, otherwise non-zero.
#
sub
CheckFunction($$)       # (type, name)
{
    my ($type, $name) = @_;

    my $BASE   = "${type}_${name}";
    my $SOURCE = "${BASE}.c";
    my ($cmd, $cmdparts)
               = CheckCommand($BASE, $SOURCE);

    my $tmpsource = "${x_tmpdir}/$SOURCE";
    my $asctime = asctime(localtime());

    chop($asctime);
    open(TMP, ">${tmpsource}") or
            die "cannot create ${tmpsource} : $!\n";
    print TMP<<EOT;
/*
 *  Generated by makelib.pl, $asctime
$cmdparts
 */
#if defined(__STDC__)
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#endif
EOT

    foreach my $header (@HEADERS) {
        print TMP "#include <$header>\n";
    }

    print TMP<<EOT;
typedef void (*function_t)(void);
static function_t function;
int main(int argc, char **argv) {
    function = (function_t)(&$name);
    return 1;
}
EOT
    close TMP;

    return CheckExec($BASE, $cmd);
}


#   Function: CheckICUFunction
#       Check that the ICU function exists
#   Returns:
#       0 on success, otherwise non-zero.
#
sub
CheckICUFunction($)     # (type, name)
{
    my ($name) = @_;

    my $BASE   = "icufunction_${name}";
    my $SOURCE = "${BASE}.cpp";
    my ($cmd, $cmdparts)
               = CheckCommand($BASE, $SOURCE, 'icu');

    my $tmpsource = "${x_tmpdir}/$SOURCE";
    my $asctime = asctime(localtime());

    chop($asctime);
    open(TMP, ">${tmpsource}") or
            die "cannot create ${tmpsource} : $!\n";
    print TMP<<EOT;
/*
 *  ICU test application.
$cmdparts
 */
#if defined(__STDC__)
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#endif
#include <unicode/uversion.h>
#include <unicode/uclean.h>             // C API
#include <unicode/utypes.h>
#include <unicode/ustdio.h>
#include <unicode/unistr.h>
#include <unicode/uchar.h>
#include <unicode/udata.h>
#include <unicode/ucal.h>               // UCAL_XXX
#include <unicode/calendar.h>           // Calendar::
#include <iostream>
#ifndef U_ICU_VERSION_SHORT
#error  U_ICU_VERSION_SHORT not defined ...
#endif

static void
CalendarTest() {
    UErrorCode status = U_ZERO_ERROR;

    Calendar *cal = Calendar::createInstance("en_UK\@calendar=gregorian", status);
    if (U_FAILURE(status)) {
        printf("Error creating Gregorian calendar.\\n");
        return;
    }

    {   UnicodeString result;
        result = cal->getTimeZone().getDisplayName(result);
        std::cout << "Calendar:";
        for (int i = 0; i < result.length(); ++i) {
            std::cout << (char)result[i];
        }
        std::cout << std::endl;
    }

    printf("1970 - 2020 Jan/July 01:00:00.\\n");
    for (int year = 1970; year < 2020; ++year) {
        for (int month = 1; month <= 2; ++month) {
            cal->set(year, (1 == month ? UCAL_JANUARY : UCAL_JULY), 1, 1, 0, 0);
            if (U_FAILURE(status)) {
                printf("Error setting date %d/%d/%d 01:00:00\\n", year, 1, 1);
                continue;
            }
            const UDate datetime = cal->getTime(status);
            if (U_FAILURE(status)) {
                printf("Error getting time.\\n");
                continue;
            }
            printf(" %04d%02d%02d=%11d:%d:%d:%d,",
                cal->get(UCAL_EXTENDED_YEAR,  status),
                cal->get(UCAL_MONTH, status) + 1,
                cal->get(UCAL_DATE, status),
                (int)(datetime / 1000),
                cal->get(UCAL_JULIAN_DAY /*Calendar::JULIAN_DAY*/, status),
                cal->inDaylightTime(status));
        }
        printf((year % 2) ? "\\n" : "");
    }
    printf("\\n");
}

int
main(int argc, char **argv) {
    UErrorCode status = U_ZERO_ERROR;
    UVersionInfo versionArray = {0};
    char versionString[17];                     /* xxx.xxx.xxx.xxx\\0 */

    u_init(&status);
    if (U_FAILURE(status)) {
        printf("Error in u_init: %s.\\n", u_errorName(status));
        return 0;
    }
    u_getVersion(versionArray);
    u_versionToString(versionArray, versionString);
    printf("ICU Version: %s.\\n", versionString);

    CalendarTest();

    return U_ICU_VERSION_MAJOR_NUM;             /* ie. 52 5.2.x */
}
EOT
    close TMP;

    return CheckExec($BASE, $cmd, 1);
}

#   Function: CheckCommand
#       Build the compile check exec command.
#   Returns:
#       cmd, cmdparts
#
sub
CheckCommand($$;$)      # (base, source, pkg)
{
    my ($base, $source, $pkg) = @_;

    my $env = $x_environment{$x_signature};
    my $cmd = $x_command;

    my $flags = '';
    my $optional = '';
    my $lib = '';

    if ($source =~ /\.cpp/) {
        $flags = $$env{CXXFLAGS}.' '
            if (exists $$env{CXXFLAGS});
    } else {
        $flags = $$env{CFLAGS}.' '
            if (exists $$env{CFLAGS});
    }

    if (defined $pkg) {
        if ('icu' eq $pkg) {
            my @includes = split(/ /, $$env{ICUINCLUDE});
            my @libs = split(/ /, $$env{ICULIB});

            foreach (@includes) {
                $flags .= (exists $$env{ISWITCH} ? $$env{ISWITCH} : '-I').$_.' '
                    if ($_);
            }

            foreach (@libs) {
                $lib .= (exists $$env{LSWITCH} ? $$env{LSWITCH} : '-l').$_.' '
                    if ($_);
            }
        }
    }

    $cmd =~ s/__FLAGS__/$flags/g;
    $cmd =~ s/__SOURCE__/$source/g;
    $cmd =~ s/__BASE__/$base/g;
    $cmd =~ s/__OBJ__/$base.obj/g;
    $cmd =~ s/__LIB__/$lib/g;
    $cmd =~ s/__EXE__/$base.exe/g;

    my $cmdparts = '';
    foreach (my @words = parse_line('\s+', 1, $cmd)) {
        $cmdparts .= "\n" if ($cmdparts);
        $cmdparts .= " *\t$_";
    }
    return ($cmd, $cmdparts);
}


#   Function: CheckExec
#       Execute the compile check command.
#   Parameters:
#       base - Base application name.
#       cmd - Compiler command.
#       exec - Optional boolean flag, if *true* the resulting application
#           is executed.
#   Returns:
#       cmd, cmdparts
#
sub
CheckExec($$;$)         # (base, cmd, [exec])
{
    my ($base, $cmd, $exec) = @_;

    print "(cd tmpdir; $cmd)\n"
        if ($o_keep);

    chdir($x_tmpdir) or
        die "cannot access directory <$x_tmpdir> : $!\n";

    unlink("${base}.exe");
    my $ret = System($cmd);
    $ret = -999
        if (! -f "${base}.exe");
    $ret = System($base)
        if (0 == $ret && $exec);

    if (! $o_keep) {
        opendir(DIR, '.') or
            die "error opening dir <${x_tmpdir}> : $!\n";
        my @FILES = grep /$base/, readdir(DIR);
        unlink(@FILES);
        close DIR;
    }

    chdir($CWD) or
        die "cannot restore directory <$CWD> : $!\n";
    return $ret;
}


sub
ExpandENV($)            # (var)
{
    my ($variable) = shift;

    while ($variable =~/%(.+)%/i) {
        my $var = $1;
        my $val = ${ENV{$var}};
        $val = "UNDEF_${var}_ENVVAR" if (! $val);
        $variable =~ s/%${var}%/$val/;
    }
    return $variable;
}


sub
DumpList($$)            # (prefix, arrayRef)
{
    my ($prefix, $list) = @_;

    if (scalar @$list) {
        my ($fragment, $length) = ('', 0);

        foreach my $elm (@$list) {
            my $newfragment = substr $elm, 0, 3;
            if (($newfragment eq $fragment) &&
                    ($length += length($elm)) < 80) {
                # group comment elements
                printf " $elm";

            } else {
                $fragment = $newfragment;
                $length = printf "%s%-9s $elm", ($prefix ? "" : "\n"), $prefix;
                $prefix = '';
            }
        }
        printf "\n";
    }
}


sub
ExportPath($)           # (name)
{
    my ($name) = @_;
    return $name
        if ($name !~ /\//);
    return '$(subst /,\\,' . $name . ') ';
}


sub
ImportDLL($$;$$)        # (dir, dlls, [name], [test])
{
    my ($dir, $dlls, $name, $test) = @_;

    return if (! -d $dir);

    opendir(DIR, $dir) or
        die "error opening dir <$dir> : $!\n";
    my @DLLS = grep /\.dll/, readdir(DIR);
    @DLLS = grep /$name/, @DLLS if ($name);
    if (scalar @DLLS) {
        foreach my $dll (@DLLS) {
            push @$dlls, $dll;
            copy("${dir}/${dll}", "bin/${dll}");
            copy("${dir}/${dll}", "${x_tmpdir}")
                if ($test);
        }
    }
    close DIR;
}


#   Makefile ---
#       Build a Makefile from the underlying Makefile.in
#
sub
Makefile($$$)           # (type, dir, file)
{
    my ($type, $dir, $file) = @_;
    my $optional = 0;
    my $recursive = 0;
    my $text = "";

    # Import
    if ($dir =~ /^\!(.+)$/) {
        $recursive = 1;
        $dir = $1;
    }
    if ($dir =~ /^\^(.+)$/) {
        $optional = 1;
        $dir = $1;
    }
    printf "building: $dir/$file\n";

    if (! open(MAKEFILE, "${dir}/${file}.in")) {
        if (! open(MAKEFILE, "${dir}/${file}in")) {
            return if ($optional);
            die "cannot open ${dir}/${file}[.]in : $!";
        }
    }
    while (<MAKEFILE>) {
        $_ =~ s/\s*(\n|$)//;                    # kill trailing whitespace & nl
        $text .= "$_\n";
    }
    close MAKEFILE;

    # Installation paths and install options
    $text =~ s/(\nPREFIX=[ \t]*)[^\n]+/$1\/grief/g;
    $text =~ s/(\nBINDIR=[ \t]*)[^\n]+/$1\/grief\/bin/g;
    $text =~ s/(\nDATADIR=[ \t]*)[^\n]+/$1\/grief/g;
    $text =~ s/-o bin -g bin//g;

    $text =~ s/\Q>\/dev\/null\E/>nul/g;         # nul redirection

    # Commands
    if ('dj' ne $type) {
        if ($BINPATH) {
            foreach my $command (@x_commands) {
                $text =~ s/($command) /$BINPATH$1 /g;
            }
        } else {
            foreach my $command (@x_commands) {
                $text =~ s/($command) /$1.exe /g;
            }
        }
    }

    # Specific to the target
    if ('dj' eq $type || 'mingw' eq $type) {    # almost unix
        $text =~ s/\nE=\S*\n/\nE=\t\t.exe\n/g;

    } elsif ($type eq 'vc' || $type eq 'wc' || $type eq 'owc') {
                                                # Visual C/C++ and Watcom C/C++
        # extensions
        $text =~ s/\nO=\s*\.o/\nO=\t\t.obj/g;
        $text =~ s/\nA=\s*\.a/\nA=\t\t.lib/g;
        $text =~ s/\nE=\S*\n/\nE=\t\t.exe\n/g;

        # flags and name mangling
        $text =~ s/(\nARFLAGS=)[^\n]+/$1-nologo/g;
        $text =~ s/(\$\(ARFLAGS\))\s+(\$\@)/$1 \/OUT:$2/g;

        my $clean = '';
        my $xclean = '*.pdb *.ilk';

        if ($type ne 'owc') {
            $text =~ s/(\$\(CFLAGS\).*) -o \$\@/$1 -Fo\$@ -Fd\$(\@D)\//g;
            $text =~ s/(\$\(CXXFLAGS\).*) -o \$\@/$1 -Fo\$@ -Fd\$(\@D)\//g;
            $text =~ s/(\$\(LDFLAGS\).*) -o \$\@/$1 -Fe\$@ -Fd\$(\@D)\//g;
        }

        if ($type eq 'vc') {                    # LIBPATH usage
            $text =~ s/-L/\/link \/LIBPATH:/g;

            foreach my $library (@x_libraries) {
                $text =~ s/-l${library}([\n\t \\])/lib${library}.lib$1/g;
            }

        } else {
            $text =~ s/-L([^\s]+)/-"LIBPATH $1"/g;

            if ($type eq 'owc') {               # OpenWatcom
                # options
                $text =~ s/(\$\(CFLAGS\).*) -o \$\@/$1 -Fo=\$(subst \/,\\,\$@)/g;
                $text =~ s/(\$\(CXXFLAGS\).*) -o \$\@/$1 -Fo=\$(subst \/,\\,\$@)/g;
                $text =~ s/(\$\(LDFLAGS\).*) -o \$@/$1 -Fe=\$(subst \/,\\,\$@)/g;
                $text =~ s/-Fe(.*) \$\(([A-Z_]*OBJS)\)/-Fe$1 \$(subst \/,\\,\$($2))/g;

                # directory slash conversion
                $text =~ s/-I([^\s]+)/-i="$1"/g;
                $text =~ s/\$</\$(subst \/,\\,\$<)/g;
                $text =~ s/\$\^/\$(subst \/,\\,\$^)/g;
            }

            foreach my $library (@x_libraries) {
                $text =~ s/-l${library}([\n\t \\])/lib${library}.lib$1/g;
            }

            $clean .= ' *.err';
            $xclean .= ' $(D_OBJ)/*.mbr';
        }

        # addition clean targets
        $text =~ s/(\nCLEAN=\s+)/$1${clean} /g 
            if ($clean);
        if ($xclean) {
            $text =~ s/(\nXCLEAN=[\t ]+)/$1${xclean} /;
            $text =~ s/(\nXCLEAN=[\t ]*)\n/$1\t\t${xclean}\n/;
        }
    }

    # replace tags
    $x_tokens{top_builddir} = ($dir eq '.' ? '.' : '..');
    $x_tokens{top_srcdir} = ($dir eq '.' ? '.' : '..');
    if ('-i=' eq $x_tokens{ISWITCH}) {
        $x_tokens{CINCLUDE} =~ s/-I([^\s]+)/-i="$1"/g;
    }

    $x_tokens{CXX} = $x_tokens{CC}              # CXX=CC
        if (!exists $x_tokens{CXX});
    foreach my $entry (keys %x_tokens) {
        my $quoted_entry = quotemeta($entry);
        my $replace = $x_tokens{$entry};

        $text =~ s/\@$quoted_entry\@/$replace/g;
    }

    if ($BUSYBOX) {                             # command interface rework
        $text =~ s/\@sh /\@\@BUSYBOX\@ sh /g;
        $text =~ s/\-sh /-\@BUSYBOX\@ sh /g;
        $text =~ s/shell date /shell \@BUSYBOX\@ date /g;
        $text =~ s/shell cat /shell \@BUSYBOX\@ cat /g;
    }

    $text =~ s/\@BINPATH\@/${BINPATH}/g;
    $text =~ s/\@PERLPATH\@/${PERLPATH}/g;
    $text =~ s/\@BUSYBOX\@/${BUSYBOX}/g;

    # export
    my $asctime = asctime(localtime());

    chop($asctime);
    open(MAKEFILE, ">$dir/$file") or
        die "cannot create $dir/$file";
    if ($file eq 'Makefile') {
        print MAKEFILE "# Generated by makelib.pl, $asctime\n";
    } elsif ($file =~ /.h$/) {
        print MAKEFILE "/* Generated by makelib.pl, $asctime */\n";
    }

    if ($file eq 'Makefile') {                  # compact whitespace
        $text =~ s/ [ ]+/ /g;
        $text =~ s/\t[ ]+/\t/g;
    }

    print MAKEFILE $text;
    close MAKEFILE;

    if ($recursive) {
        opendir(DIR, $dir) or
            die "error opening dir <${dir}> : $!\n";
        my @FILES = readdir(DIR);
        close DIR;

        foreach (@FILES) {
            next if (/^\./);
            my $subdir = "$dir/$_";
            if (-d $subdir) {
                if (-f "${subdir}/${file}.in" || -f "${subdir}/${file}in") {
                    Makefile($type, "!${subdir}", $file);
                }
            }
        }
    }
}


sub
MakefileDir($)
{
    my $name = shift;
    $name =~ s/^[\^\!]+//;
    return $name;
}


#   Config ---
#       Build a config from an underlying config.in
#
sub
Config($$)              # (type, dir)
{
    my ($type, $dir) = @_;
    my $file = 'config.h';
    my $text = "";

    printf "building: $dir/$file\n";

    # import
    if (! open(CONFIG, "${dir}/${file}.in")) {
        open(CONFIG, "${dir}/${file}in") or
            die "cannot open ${dir}/${file}[.]in : $!";
    }
    while (<CONFIG>) {
        $_ =~ s/\s*(\n|$)//;                    # kill trailing whitespace & nl
        $text .= "$_\n";
    }
    close CONFIG;

    # update characteristics
    my @MISSING = ();

    foreach my $config (sort keys %CONFIG_H) {
        my $value = $CONFIG_H{$config};

        if ($text =~ /undef[\s]+${config}.*\n/) {
            $text =~ s/undef[\s]+${config}.*\n/define ${config} ${value}\n/;
        } else {
            push @MISSING, $config;
        }
    }
    $text =~ s/(#undef[^*\n]+)\n/\/* $1 *\/\n/g;

    if (scalar @MISSING) {
        foreach my $config (@MISSING) {
            print "missing:  $config\n";
        }
    }

    # export
    my $asctime = asctime(localtime());

    chop($asctime);
    open(CONFIG, ">$dir/$file") or
        die "cannot create $dir/$file";
    print CONFIG "/* Generated by makelib.pl, $asctime */\n";
    print CONFIG $text;
    close CONFIG;
}


sub
cannon_path($)
{
    my $path  = shift;
    my ($volume, $directories, $file) = File::Spec->splitpath(File::Spec->canonpath($path));
    my (@dur) = File::Spec->splitdir($directories);

    my @dar;
    foreach(@dur){
        if ($_ eq '..') {
            pop @dar;
        } else {
            push @dar, $_;
        }
    }
    $path = File::Spec->catpath($volume, File::Spec->catdir(@dar), $file);
    $path =~ s/\\/\//g;
    return $path;
}


sub
System($)               # (cmd)
{
    my $cmd = shift;
    system($cmd);
    return systemrcode($?);
}


sub
systemrcode($)          # (retcode)
{
    my $rc = shift;
    my $rcode = 0;

    if ($rc == -1) {
        $rcode = -1;                            # task exec error
    } elsif ($rc & 127) {
        $rcode = -2;                            # cored
    } elsif ($rc) {
        $rcode = $rc >> 8;                      # application return code
    }
    return $rcode;
}

#end
