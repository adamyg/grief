#!/usr/bin/perl -w
# -*- mode: perl; -*-
# $Id: libtool_win32.pl,v 1.22 2014/10/27 13:01:18 ayoung Exp $
# libtool emulation for WIN32 builds.
#
#   **Warning**
#
#       Functionality is limited to the current GRIEF build requirements.
#
#   Example usage:
#
#       $(D_LIB)/%.la:      $(D_OBJ)/%.lo
#               $(LIBTOOL) --mode=link $(CC) $(CFLAGS) -rpath $(D_LIB) -bindir $(D_BIN) -o $@ $(D_OBJ)/$<
#
#       $(D_LIB)/%.lo:      %.c
#               $(LIBTOOL) --mode=compile $(CC) $(CFLAGS) -o $(D_OBJ)/$@ -c $<
#
#       $(D_LIB)/%.lo:      %.cpp
#               $(LIBTOOL) --mode=compile $(CXX) $(CXXFLAGS) -o $(D_OBJ)/$@ -c $<
#
#
use strict;
use warnings 'all';

use Getopt::Long;
use File::Basename;
use File::Copy;
use Cwd;

my $x_libdir = ".libs/";
my $o_help;
my $o_config;
my $o_dryrun;
my $o_features;
my $o_mode;
my $o_tag;
my $o_preserve_dup_deps;
my $o_quiet;
my $o_silent = 0;
my $o_verbose = 0;
my $o_debug;
my $o_version;
my $o_extra = '';

sub Compile();
sub Link();
sub true_object($);
sub unix2dos($);
sub dos2unix($);
sub Help;
sub Usage;
sub Usage_Link();
sub Usage_Compile();
sub System;
sub __SystemReturnCode($);
sub Label;
sub Debug;
sub Verbose;
sub Warning;
sub Error;

exit &Main();


#   libtool ...
#
sub
Main
{
    Help() if (!scalar @ARGV);

    Getopt::Long::Configure("require_order");

    my $ret =
        GetOptions(
            'h|help'            => \$o_help,
            'config'            => \$o_config,
            'n|dry-run'         => \$o_dryrun,
            'features'          => \$o_features,
            'mode=s'            => \$o_mode,
                'finish'        => sub {$o_mode='finish';},
            'tag=s'             => \$o_tag,
            'preserve-dup-deps' => \$o_preserve_dup_deps,
            'quiet'             => \$o_quiet,
            'no-quiet'          => sub {$o_quiet=0;},
            'silent'            => \$o_silent,
            'no-silent'         => sub {$o_silent=0;},
            'v|verbose'         => \$o_verbose,
            'no-verbose'        => sub {$o_verbose=0;},
            'debug'             => \$o_debug,
            'version'           => \$o_version);

    Help() if (!$ret);
    if ($o_help && $o_mode) {
        if ('compile' eq $o_mode) {
            Usage_Compile();

        } elsif ('link' eq $o_mode) {
            Usage_Link();
        }
    }
    Usage() if ($o_help);
    Usage("missing arguments") if (!scalar @ARGV);

    if ('compile' eq $o_mode) {
        # Compile a source file into a libtool object.
        Compile();

    } elsif ('link' eq $o_mode) {
        # Create a library or an executable.
        Link();

    } elsif ('install' eq $o_mode) {
        # Install libraries or executables.

    } elsif ('finish' eq $o_mode) {
        # Complete the installation of libtool libraries on the system.

    } elsif ('execute' eq $o_mode) {
        # Automatically set the library path so that another program can use
        # uninstalled libtool-generated programs or libraries.

    } elsif ('uninstall' eq $o_mode) {
        # Delete installed libraries or executables.

    } elsif ('clean' eq $o_mode) {
        # Delete uninstalled libraries or executables.
        Clean();

    } else {
        Usage("unknown mode <$o_mode>");
    }
    return 0;
}


#   Function:       Compile
#       Compile a library object.
#
sub
Compile() {
    my $cc = shift @ARGV;
    my $object;
    my $source;
    my $compile;

    my @DEFINES;
    my @INCLUDES;
    my @STUFF;

    while (scalar @ARGV > 1) {
        $_ = shift @ARGV;

        if (/^-o(.*)$/) {                       # -o <object>
            my $val = ($1 ? $1 : shift @ARGV);
            if ($val && $val !~ /\./) {
                push @STUFF, "-o${val}";        # i.e. optimisation flags
            } else {
                $val = shift @ARGV if (! $val);
                Error("compile: multiple objects specified <$object> and <$val>")
                    if ($object);
                $object = $val;
            }

        } elsif (/^[-\/]Fo[=]?(.*)/) {          # -Fo[=]<object>
            my $val = ($1 ? $1 : shift @ARGV);
            Error("compile: multiple objects specified <$object> and <$val>")
                if ($object);
            $object = $val;

        } elsif (/^[-\/]Fd[=]?(.*)/) {          # -Fd[=]<pdb>
            #consume

        } elsif (/^-i[=]?(.*)$/) {              # -i= <path>
            push @INCLUDES, ($1 ? $1 : shift @ARGV);

        } elsif (/^[-\/]I[=]?(.+)$/) {          # -I[=]<path>
            push @INCLUDES, $1;

        } elsif (/^-d(.*)$/) {                  # -d <define[=value]>
            if ('wcl386' eq $cc && (/^-d[1-3][ist]$/ || /^-db$/)) {
                push @STUFF, $_;
            } else {
                push @DEFINES, ($1 ? $1 : shift @ARGV);
            }

        } elsif (/^[-\/]D[=]?(.+)/) {           # -D[=]<define[=value]>
            push @DEFINES, $1;

        } elsif (/^[-\/]c/) {                   # -c
            $compile = 1;

##      } elsif (/^[-\/]M/) {                   # -MT[d], -MD[d]
##

        } elsif (/^-prefer-pic$/) {
            # Libtool will try to build only PIC objects.

        } elsif (/^-prefer-non-pic$/) {
            # Libtool will try to build only non-PIC objects.

        } elsif (/^-shared$/) {
            # Even if Libtool was configured with --enable-static, the object file Libtool builds will
            # not be suitable for static linking. Libtool will signal an error if it was configured
            # with --disable-shared, or if the host does not support shared libraries.

        } elsif (/^-static$/) {
            # Even if libtool was configured with --disable-static, the object file Libtool builds will
            # be suitable for static linking.

        } elsif (/^-Wc,(.*)$/) {
            # Pass a flag directly to the compiler. With -Wc,, multiple flags may be separated by
            # commas, whereas -Xcompiler passes through commas unchanged.

        } elsif (/^-Xcompiler(.*)/) {
            # Pass a flag directly to the compiler. With -Wc,, multiple flags may be separated by
            # commas, whereas -Xcompiler passes through commas unchanged.

        } else {
            push @STUFF, $_;
        }
    }
    $source = shift @ARGV;

    Error("compile: unsupported compiler <$cc>")
        if (!('cl' eq $cc || 'wcl386' eq $cc || 'gcc' eq $cc));
    Error("compile: unable to determine object")
        if (!$object);
    Error("compile: object file suffix not <.lo>")
        if ($object !~ /.lo$/);
    Error("compile: missing source file")
        if (!$source || $source =~ /^-/);       # missing or an option.
    Error("compile: -c options missing")
        if (!$compile);

    my $true_path = unix2dos(dirname($object))."\\${x_libdir}";
    my $true_object = ${true_path}.basename($object, 'lo')."obj";

    Verbose "cc:       $cc";
    Verbose "defines:";
        foreach(@DEFINES) { Verbose "\t$_"; }
    Verbose "includes:";
        foreach(@INCLUDES) { Verbose "\t$_"; }
    Verbose "object:   $object";
    Verbose "  true:   $true_object";
    Verbose "source:   $source";
    Verbose "...:      @STUFF";

    my $cmd = '';

    if ('cl' eq $cc) {
        $cmd  = "$cc @STUFF /DDLL=1";
        foreach (@DEFINES) { $cmd .= " /D$_"; }
        foreach (@INCLUDES) { $cmd .= " /I$_"; }
        $cmd .= " /Fo$true_object";
        $cmd .= " /Fd".$true_path.'\\';         # VCx0.pdb
        $cmd .= " /c $source";

    } elsif ('wcl386' eq $cc) {
        # http://www.openwatcom.org/index.php/Writing_DLLs
        $cmd  = "$cc @STUFF -dDLL=1";
        $cmd .= " -bd";                         # DLL builds
        foreach (@DEFINES) { $cmd .= " -d$_"; }
        foreach (@INCLUDES) { $cmd .= " -I=\"$_\""; }
        $cmd .= " -Fo=\"$true_object\"";
        $cmd .= " -c $source";

    } elsif ('gcc' eq $cc) {
        $cmd  = "$cc @STUFF -D DLL=1 -shared";
        foreach (@DEFINES) { $cmd .= " -D $_"; }
        foreach (@INCLUDES) { $cmd .= " -I \"$_\""; }
        $cmd .= " -o \"$true_object\"";
        $cmd .= " -c $source";
    }

    my $ret = 0;
    mkdir $true_path;
    unlink $object;

    exit($ret)
        if (0 != ($ret = System($cmd)));

    open(LO, ">${object}") or
        die "cannot create <$object> : $!\n";
    print LO "#libtool win32 generated, do not modify\n";
    print LO "mode=dll\n";
    print LO "cc=$cc\n";
    print LO "cmd=$cmd\n";
    print LO "source=$source\n";
    print LO "object=$object\n";
    print LO "true_object=$true_object\n";
    close(LO);
    return 0;
}


#   Function:       Link
#       Link a library object.
#
sub
Link() {
    my $cc = shift @ARGV;

    my ($output, $rpath, $bindir, $module);
    my $version_number = '';
    my $fastcall = 0;                           # fastcall
    my @OBJECTS;
    my @RESOURCES;
    my @EXPORTS;
    my @LIBRARIES;
    my @LIBPATHS;
    my @STUFF;

    while (scalar @ARGV) {
        $_ = shift @ARGV;

        if (/^-o(.*)$/) {                       # -o <output>
            my $val = ($1 ? $1 : shift @ARGV);
            Error("link: multiple outputs specified <$output> and <$val>")
                if ($output);
            $output = $val;

        } elsif (/^[-\/]Fo[=]?(.*)/) {          # -Fe[=]<output>
            my $val = ($1 ? $1 : shift @ARGV);
            Error("link: multiple outputs specified <$output> and <$val>")
                if ($output);
            $output = $val;

        } elsif (/^(.*)\.lo$/ || /^(.*)\.o$/ || /^(.*)\.obj$/) {
            push @OBJECTS, $_;

        } elsif (/^(.*)\.res$/) {
            push @RESOURCES, $_;

        } elsif (/^(.*)\.rc$/) {
            Error("link: $_ not supported\n");

        } elsif (/^(.*)\.la$/ || /^(.*)\.a$/ || /^(.*)\.lib$/) {
            push @LIBRARIES, $_;

        } elsif (/^-l(.*)$/) {
            push @LIBRARIES, $1;

        } elsif (/^[-\/]LIBPATH[:]?\s*(.+)/) {  # -LIBPATH[:]<path>
            push @LIBPATHS, $1;

        } elsif (/^-L(.*)/) {
            push @LIBPATHS, $1;

    ##  } elsif (/^[-\/]M/) {                   # -MT[d], -MD[d]
    ##

        } elsif (/^-all-static$/) {
            Error("link: $_ not supported\n");

        } elsif (/^-avoid-version$/) {
            #ignore

        } elsif (/^-bindir[=]?(.*)$/) {
            my $val = ($1 ? $1 : shift @ARGV);
            Error("link: $_ $val, not a valid directory : $!")
                if (! -d $val);
            $bindir = $val;

        } elsif (/^-dlopen(.*)$/) {
            Error("link: $_ not supported\n");

        } elsif (/^-dlpreopen(.*)$/) {
            Error("link: $_ not supported\n");

        } elsif (/^-export-dynamic$/) {
            Error("link: $_ not supported\n");

        } elsif (/^-export-fastcall$/) {        # extension
            $fastcall = 1;

        } elsif (/^-export-symbols[=]?(.*)$/) {
            my $val = ($1 ? $1 : shift @ARGV);
            Error("link: $_ $val, not a valid symbol file : $!")
                if (! -f $val);

            if ($val =~ /\.def$/i) {            # <name.def>
                ParseDefFile($val, \@EXPORTS);
            } else {                            # <name.sym>
                ParseSymFile($val, \@EXPORTS);
            }

        } elsif (/^-export-symbols-regex(.*)$/) {
            Error("link: $_ not supported\n");

        } elsif (/^-module$/) {
            $module=1;

        } elsif (/^-no-fast-install$/) {
            Error("link: $_ not supported\n");

        } elsif (/^-no-install$/) {
            Error("link: $_ not supported\n");

        } elsif (/^-no-undefined$/) {
            Error("link: $_ not supported\n");

        } elsif (/^-objectlist(.*)$/) {
            Error("link: $_ not supported\n");

        } elsif (/^-precious-files-regex(.*)$/) {
            Error("link: $_ not supported\n");

        } elsif (/^-release(.*)$/) {
            Error("link: $_ not supported\n");

        } elsif (/^[-]+rpath[=]?(.*)$/) {
            my $val = ($1 ? $1 : shift @ARGV);
            if (! -d $val) {
                Warning("link: $_ $val, not a valid directory -- ignored");
            } else {
                $rpath = $val;
            }

        } elsif (/^-R(.*)$/) {
            Error("link: $_ not supported\n");

        } elsif (/^-shared$/) {
            Error("link: $_ not supported\n");

        } elsif (/^-shrext(.*)$/) {
            Error("link: $_ not supported\n");

        } elsif (/^-static$/) {
            Error("link: $_ not supported\n");

        } elsif (/^-static-libtool-libs$/) {
            Error("link: $_ not supported\n");

        } elsif (/^-version-info(.*)$/) {
            Error("link: $_ not supported\n");

        } elsif (/^-version-number[=]?(.*)$/) {
            my $val = ($1 ? $1 : shift @ARGV);
            $version_number = $val;

        } elsif (/^-weak(.*)$/) {
            Error("link: $_ not supported\n");

        } elsif (/^-Wc,(.*)$/) {
            Error("link: $_ not supported\n");

        } elsif (/^-Xcompiler(.*)$/) {
            Error("link: $_ not supported\n");

        } elsif (/^-Wl,(.*)$/) {
            Error("link: $_ not supported\n");

        } elsif (/^-Xlinker(.*)$/) {
            Error("link: $_ not supported\n");

        } elsif (/^-XCClinker(.*)$/) {
            Error("link: $_ not supported\n");

        } else {
            push @STUFF, $_;
        }
    }

    Error("link: unsupported compiler <$cc>")
        if (!('cl' eq $cc || 'wcl386' eq $cc || 'gcc' eq $cc));
    Error("link: unable to determine output")
        if (!$output);
    Error("link: output file suffix not <.la>")
        if ($output !~ /.la$/);

    Verbose "cc:       $cc";
    Verbose "output:   $output";
    Verbose "objects:";
        foreach(@OBJECTS) {
            Verbose "\t$_";
        }
    Verbose "libraries:";
        foreach(@LIBRARIES) { Verbose "\t$_"; }
    Verbose "...:      @STUFF";

    my ($dll_version, $dll_major, $dll_minor) = ('', 0, 0);
    if ($version_number) {
        if ($version_number =~ /^\s*(\d+)\s*$/) {
            # <major>
            ($dll_major, $dll_minor) = ($1, 0);

            $dll_version = "$1";                # <name>major
            $version_number = "$1";

        } elsif ($version_number =~ /^\s*(\d+):(\d+)/) {
            # <major>[:<minor>[:<revision>]]
            ($dll_major, $dll_minor) = ($1, $2);

            $dll_version = "$1.$2";             # major.minor
            $version_number = ".$1.$2";         # <name>.<major.<minor>.dll

        } elsif ($version_number =~ /^\s*(\d+)\s*\.\s*(\d+)/) {
            # <major>[.<minor>[.<revision>]]
            ($dll_major, $dll_minor) = ($1, $2);

            $dll_version = "$1.$2";             # major.minor
            $version_number = ".$1.$2";         # <name>.<major.<minor>.dll

        } else {
            Error("link: invalid -version_number <$version_number>\n");
        }
    }

    my ($cmdfile, $deffile, $cmd) = ("__libtool__.lnk", "__libtool__.def");
    my $basedir  = unix2dos(dirname($output));
    my $basename = unix2dos(basename($output, '.la'));
    my $basepath = $basedir.'\\'.$basename;
    my $dllname  = "${basename}${version_number}.dll";
    my $dllpath  = "${basepath}${version_number}.dll";
    my $libpath  = "${basepath}.lib";

    if ('cl' eq $cc) {
        #
        #   MSVC/Watcom
        #
        $cmd = "\"%VCINSTALLDIR%\\bin\\link\" \@$cmdfile";

        open(CMD, ">${cmdfile}") or
            die "cannot create <${$cmdfile}>: $!\n";
        print CMD "/DLL\n";
        print CMD "/SUBSYSTEM:WINDOWS\n";
        print CMD "/ENTRY:_DllMainCRTStartup\@12"."\n";
        print CMD "/OUT:${dllpath}\n";
        print CMD "/IMPLIB:${libpath}\n";
        print CMD "/MAP:${basepath}.map\n";
        print CMD "/MAPINFO:EXPORTS\n";
        print CMD "/VERSION:${dll_version}\n"
            if ($dll_version);
        print CMD "/NOLOGO\n";
        print CMD "/INCREMENTAL:NO\n";
        print CMD "/OPT:REF\n";
        print CMD "/DEBUG\n";
      foreach(@OBJECTS) {
        print CMD true_object($_)."\n";
      }
      foreach(@RESOURCES) {
        print CMD true_object($_)."\n";
      }
      foreach(@EXPORTS) {
        print CMD "/EXPORT:$_\n";
      }
      foreach(@LIBPATHS) {
        print CMD "/LIBPATH:$_\n";
      }
      foreach(@LIBRARIES) {
         print CMD "$_\n";
      }
        close(CMD);

    } elsif ('wcl386' eq $cc) {
        #
        #   OpenWatcom
        #
        $cmd = "wlink \@$cmdfile";

        open(CMD, ">${cmdfile}") or
            die "cannot create <${$cmdfile}>: $!\n";
        print CMD "system   nt_dll\n";
        print CMD "name     ${dllpath}\n";
        print CMD "option   implib=${libpath}\n";
        print CMD "option   version=${dll_version}\n"
            if ($dll_version);
        print CMD "option   map=${basepath}.map\n";
        print CMD "option   symfile=${basepath}.sym\n";
        print CMD "option   checksum\n";
        print CMD "option   quiet\n";
        print CMD "debug    all\n";
      foreach(@OBJECTS) {
        print CMD "file     ".unix2dos(true_object($_))."\n";
      }
      foreach(@RESOURCES) {
        print CMD "option   resource=".unix2dos(true_object($_))."\n";
      }
      foreach(@EXPORTS) {
       if (/^(.+)=(.+)$/) {
        print CMD "export   '$1'='$2'\n";       # quote internal name.
       } elsif ($fastcall) {
        print CMD "export   $_=$_"."_\n";       # fastcall.
       } else {
        print CMD "export   $_=_$_\n";          # cdecl.
       }
      }
      foreach(@LIBPATHS) {
        print CMD "libpath  ".unix2dos($_)."\n";
      }
      foreach(@LIBRARIES) {
        print CMD "library  $_\n";
      }
        close(CMD);

    } elsif ('gcc' eq $cc) {
        #
        #   MinGW
        #
        $cmd = "g++ \@${cmdfile}";

        open(CMD, ">${cmdfile}") or
            die "cannot create <${cmdfile}>: $!\n";
        print CMD "-o ".dos2unix($dllpath)."\n";
        print CMD "-shared\n";
      foreach(@OBJECTS) {
        print CMD dos2unix(true_object($_))."\n";
      }
        print "warning: resources ignored @RESOURCES\n"
            if (scalar @RESOURCES);
        print CMD "-Wl,--subsystem,windows\n";
        print CMD "-Wl,--out-implib,".dos2unix("${basepath}.a")."\n";
        print CMD "-Xlinker -Map=".dos2unix($basepath).".map\n";
      foreach(@LIBPATHS) {
        print CMD "-L ".dos2unix($_)."\n";
      }
      foreach(@LIBRARIES) {
        print CMD "-l".dos2unix($_)."\n";
      }

      if (scalar @EXPORTS) {
        print CMD dos2unix($deffile)."\n";
        open(DEF, ">${deffile}") or
            die "cannot create <${deffile}>: $!\n";
        print DEF "LIBRARY \"${dllname}\"\n";
        print DEF "EXPORTS\n";
        foreach(@EXPORTS) {
          if (/^(.+)=(.+)$/) {
             my ($name, $alias) = ($1, $2);
             $alias =~ s/^_//
                if ($alias =~ /\@/);            # remove leading if <xxxx@##>
             print DEF "'$name'='$alias'\n";    # quote internal name
          } else {
             s/^_// if (/\@/);                  # remove leading if <xxxx@##>
             print DEF "$_\n";
          }
        }
        close(DEF);
      }

        close(CMD);
    }

    my $ret = 0;
    unlink $output;
    if (0 != ($ret = System($cmd))) {
        if ('cl' eq $cc) {
            print "#\n".
            "# LINK: fatal error LNK1123: failure during conversion to COFF; file invalid or corrupt\n".
            "#  can be result of an incorrect version of cvtres.exe due to dual VC10/VC2012 installations,\n".
            "#  rename 'C:/Program Files (x86)/Microsoft Visual Studio 10/VC/Bin/cvtres.exe' => cvtres_org.exe\n".
            "#\n";
        }
        exit ($ret);
    }

    unlink $cmdfile, $deffile;
    open(LO, ">${output}") or
        die "cannot create <$output> : $!\n";
    print LO "#libtool win32 generated, do not modify\n";
    print LO "mode=link\n";
    print LO "cc=$cc\n";
    print LO "lib=${libpath}\n";
    print LO "dll=${dllpath}\n";
    print LO "[objects]\n";
    foreach(@OBJECTS) {
        print LO true_object($_)."\n";
    }
    print LO "[libraries]\n";
    foreach(@LIBRARIES) {
        print LO "$_\n";
    }
    close(LO);

    if ('gcc' eq $cc) {
        copy("${basepath}.a", $libpath) or
            die "link: unable to copy <${basepath}.a> to <${libpath}> : $!\n";
    }

    if ($bindir) {
        print "installing ${dllname} to ${bindir}\n";
        copy($dllpath, "${bindir}/${dllname}") or
            die "link: unable to copy <$dllname> to <${bindir}/${dllname}> : $!\n";
    }
    return 0;
}


#   Function: ParseDefFile
#       Parse a definition file.
#
#   Syntax:
#       EXPORTS
#           entryname[=internalname] [@ordinal [NONAME]] [PRIVATE] [DATA]
#           ...
#       /HEAP:reserve[,commit]
#       LIBRARY [library][BASE=address]
#       NAME [application][BASE=address]
#       SECTIONS
#           definitions
#       STACKSIZE reserve[,commit]
#       STUB:filename
#       VERSION major[.minor]
#
sub
ParseDefFile($$) {
    my ($file, $EXPORTSRef) = @_;
    my $mode = 0;

    open(DEF, "<${file}") or
        die "cannot open <$file> : $!\n";
    while (<DEF>) {
        s/\s*([\n\r]+|$)//;
        s/^\s+//;
        next if (!$_ || /^;/);                  # blank or comment
        if (/^EXPORTS(.*)/) {
            $mode = 1;
            next if (!$1);
            $_ = $1;

        } elsif (/^\/HEAP:(.*)/) {
            Error("link: $file ($.), HEAP option not supported\n");

        } elsif (/^LIBRARY\s+(.*)/) {
            Warning("link: $file ($.), 'LIBRARY $1' option ignored\n");
            next;

        } elsif (/^NAME\s+(.*)/) {
            Warning("link: $file ($.), 'NAME $1'option ignored\n");
            next;

        } elsif (/^SECTIONS/) {
            Error("link: $file ($.), SECTIONS option not supported\n");

        } elsif (/^STACKSIZE\s+.*$/) {
            Error("link: $file ($.), STACKSIZE option not supported\n");

        } elsif (/^STUB:+.*$/) {
            Error("link: $file ($.), STUB option not supported\n");

        } elsif (/^VERSION\s+.*$/) {
            Error("link: $file ($.), VERSION option not supported\n");

        } elsif (/^DESCRIPTION\s+.*$/) {
            #ignored
            next;
        }

        if (1 == $mode) {                       # EXPORTS
            s/\s+//i;
            push @$EXPORTSRef, $_;
        } else {
            Error("link: $file ($.), unexpected text <$_>\n");
        }
    }
    close(DEF);
}


sub
ParseSymFile($$) {
    my ($file, $EXPORTSRef);
    my $mode = 0;

    open(SYM, "<${file}") or
        die "cannot open <$file> : $!\n";
    while (<SYM>) {
        s/\s*([\n\r]+|$)//;
        s/^\s+//;
        next if (!$_ || /^[;#]/);               # blank or comment
        push @$EXPORTSRef, $_;
    }
    close(SYM);
}


#   Function:       Clean
#       Cleanup library object.
#
sub
Clean() {
    my $rm = shift @ARGV;

    my @OBJECTS;
    my @LIBRARIES;

    while (scalar @ARGV) {
        $_ = shift @ARGV;

        if (/\.lo$/ || /\.o$/ || /\.obj$/ || /\.res$/) {
            push @OBJECTS, $_;

        } elsif (/\.la$/ || /\.a$/ || /\.lib$/) {
            push @LIBRARIES, $_;
        }
    }

    Error("clean: unsupported remove command <$rm>")
        if (!('rm' eq $rm || 'rm.exe' eq $rm));

    foreach(@OBJECTS) {
        my $object = $_;
        if (-f $object) {
            my $true_object = true_object($object);
            unlink($object, $true_object);
            rmdir(dirname($true_object));
        }
    }

    return 0;
}


#   Function:       true_object
#       Retrieve the true object name for the specified 'lo' image.
#
sub
true_object($)          #(lo)
{
    my ($lo) = @_;
    my $true_object;

    return $lo
        if ($lo !~ /.lo$/);
    open(LO, "<${lo}") or
        die "cannot open <$lo> : $!\n";
    while (<LO>) {
        s/\s*([\n\r]+|$)//;
        next if (!$_ || /^\s#/);
        if (/^true_object=(.*)$/) {
            $true_object = $1;
            last;
        }
    }
    close(LO);
    die "internal: lo truename missing <$lo>" if (!$true_object);
    return $true_object;
}


#   Function:       unix2dos
#       Forward slash conversion.
#
sub
unix2dos($)             #(name)
{
    my $name = shift;
    $name =~ s/\//\\/g;
    return $name;
}


#   Function:       dos2unix
#       Forward slash conversion.
#
sub
dos2unix($)             #(name)
{
    my $name = shift;
    $name =~ s/\\/\//g;
    return $name;
}


#   Function:       Usage
#       libtool command line usage
#
sub
Help {
    print STDERR "@_\n\n"
        if (@_);
    print STDERR << "EOF";
Minimal win32 libtool emulation (version: 0.3)

usage:  libtool.pl [options] ...

--help for help
EOF
    exit(1);
}


sub
Usage {
    print STDERR "@_\n\n"
        if (@_);
    print STDERR << "EOF";
Minimal win32 libtool emulation (version: 0.3)

usage:  libtool.pl [options] ...

Options:

   --config
       Display libtool configuration variables and exit.

   --debug
       Trace of shell script execution to standard output.

   -n, --dry-run
       List commands only, do not execute.

   --features
       Display basic configuration options.

   --finish
       Same as --mode=finish.

   -h, --help
       Display a help message and exit. If --mode=mode is specified, then
       detailed help for mode is displayed.

   --mode=mode
       Use mode as the operation mode, mode must be set to one of the following:

       compile
           Compile a source file into a libtool object.

       execute
           Automatically set the library path so that another program can use
           uninstalled libtool-generated programs or libraries.

       link
           Create a library or an executable.

       install
           Install libraries or executables.

       finish
           Complete the installation of libtool libraries on the system.

       uninstall
           Delete installed libraries or executables.

       clean
           Delete uninstalled libraries or executables.

   --tag=tag
       Use configuration variables from tag tag (see Tags).

   --preserve-dup-deps
       Do not remove duplicate dependencies in libraries.

   --quiet, --silent
       Do not print out any progress or informational messages.

   -v, --verbose
       Print out progress and informational messages (enabled by default), as
       well as additional messages not ordinary seen by default.

   --no-quiet, --no-silent
       Print out the progress and informational messages that are seen by default.

   --no-verbose
       Do not print out any additional informational messages beyond those
       ordinarily seen by default.

   --version
       Print libtool version information and exit.

EOF
   exit(1);
}


sub
Usage_Compile() {
   print STDERR << "EOF";

For compile mode, mode-args is a compiler command to be used in creating a "standard" object
file. These arguments should begin with the name of the C compiler, and contain the -c compiler
flag so that only an object file is created.

Libtool determines the name of the output file by removing the directory component from the
source file name, then substituting the source code suffix (e.g. '.c' for C source code) with the
library object suffix, '.lo'.

If shared libraries are being built, any necessary PIC generation flags are substituted into the
compilation command.

The following components of mode-args are treated specially:

   -o
       Note that the -o option is now fully supported. It is emulated on the platforms that dont
       support it (by locking and moving the objects), so it is really easy to use libtool, just
       with minor modifications to your Makefiles.

       Typing for example libtool

           --mode=compile gcc -c foo/x.c -o foo/x.lo

       will do what you expect.

       Note, however, that, if the compiler does not support -c and -o, it is impossible to
       compile foo/x.c without overwriting an existing ./x.o. Therefore, if you do have a source
       file ./x.c, make sure you introduce dependencies in your Makefile to make sure ./x.o (or
       ./x.lo) is re-created after any sub-directorys x.lo:

           x.o x.lo:   foo/x.lo bar/x.lo

       This will also ensure that make wont try to use a temporarily corrupted x.o to create a
       program or library. It may cause needless recompilation on platforms that support -c and
       -o together, but its the only way to make it safe for those that dont.

   -no-suppress
       If both PIC and non-PIC objects are being built, libtool will normally suppress the
       compiler output for the PIC object compilation to save showing very similar, if not
       identical duplicate output for each object. If the -no-suppress option is given in
       compile mode, libtool will show the compiler output for both objects.

   -prefer-pic
       Libtool will try to build only PIC objects.

   -prefer-non-pic
       Libtool will try to build only non-PIC objects.

   -shared
       Even if Libtool was configured with --enable-static, the object file Libtool builds will
       not be suitable for static linking. Libtool will signal an error if it was configured
       with --disable-shared, or if the host does not support shared libraries.

   -static
       Even if libtool was configured with --disable-static, the object file Libtool builds will
       be suitable for static linking.

   -Wc,flag
   -Xcompiler flag
       Pass a flag directly to the compiler. With -Wc,, multiple flags may be separated by
       commas, whereas -Xcompiler passes through commas unchanged.

EOF
}


sub
Usage_Link() {
   print STDERR << "EOF";

Link mode links together object files (including library objects) to form another library or to
create an executable program.

mode-args consist of a command using the C compiler to create an output file (with the -o flag)
from several object files.

The following components of mode-args are treated specially:

-all-static
   If output-file is a program, then do not link it against any shared libraries at all. If
   output-file is a library, then only create a static library. In general, this flag cannot be
   used together with 'disable-static' (see LT_INIT).

-avoid-version
   Tries to avoid versioning (see Versioning) for libraries and modules, i.e. no version
   information is stored and no symbolic links are created. If the platform requires versioning,
   this option has no effect.

-bindir BINDIR
   Pass the absolute name of the directory for installing executable programs (see Directory
   Variables). libtool may use this value to install shared libraries there on systems that do
   not provide for any library hardcoding and use the directory of a program and the PATH
   variable as library search path. This is typically used for DLLs on Windows or other systems
   using the PE (Portable Executable) format. On other systems, -bindir is ignored. The default
   value used is libdir/../bin for libraries installed to libdir. You should not use -bindir for
   modules.

-dlopen FILE
   Same as -dlpreopen file, if native dlopening is not supported on the host platform (see
   Dlopened modules) or if the program is linked with -static, -static-libtool-libs, or
   -all-static. Otherwise, no effect. If file is self Libtool will make sure that the program
   can dlopen itself, either by enabling -export-dynamic or by falling back to -dlpreopen self.

-dlpreopen FILE
   Link file into the output program, and add its symbols to the list of preloaded symbols (see
   Dlpreopening). If file is self, the symbols of the program itself will be added to preloaded
   symbol lists. If file is force Libtool will make sure that a preloaded symbol list is always
   defined, regardless of whether its empty or not.

-export-dynamic
   Allow symbols from output-file to be resolved with dlsym (see Dlopened modules).

-export-symbols symfile
   Tells the linker to export only the symbols listed in symfile. The symbol file should end in
   .sym and must contain the name of one symbol per line. This option has no effect on some
   platforms. By default all symbols are exported.

-export-symbols-regex regex
   Same as -export-symbols, except that only symbols matching the regular expression regex are
   exported. By default all symbols are exported.

-Llibdir
   Search libdir for required libraries that have already been installed.

-lname
   output-file requires the installed library libname. This option is required even when
   output-file is not an executable.

-module
   Creates a library that can be dlopened (see Dlopened modules). This option doesnt work for
   programs. Module names dont need to be prefixed with 'lib'. In order to prevent name clashes,
   however, libname and name must not be used at the same time in your package.

-no-fast-install
   Disable fast-install mode for the executable output-file. Useful if the program wont be
   necessarily installed.

-no-install
   Link an executable output-file that cant be installed and therefore doesnt need a wrapper
   script on systems that allow hardcoding of library paths. Useful if the program is only used
   in the build tree, e.g., for testing or generating other files.

-no-undefined
   Declare that output-file does not depend on any libraries other than the ones listed on the
   command line, i.e., after linking, it will not have unresolved symbols. Some platforms
   require all symbols in shared libraries to be resolved at library creation (see Inter-library
   dependencies), and using this parameter allows libtool to assume that this will not happen.

-o output-file
   Create output-file from the specified objects and libraries.

-objectlist FILE
   Use a list of object files found in file to specify objects.

-precious-files-regex regex
   Prevents removal of files from the temporary output directory whose names match this regular
   expression. You might specify "\.bbg?$" to keep those files created with gcc -ftest-coverage
   for example.

-release release
   Specify that the library was generated by release release of your package, so that users can
   easily tell which versions are newer than others. Be warned that no two releases of your
   package will be binary compatible if you use this flag. If you want binary compatibility, use
   the -version-info flag instead (see Versioning).

-rpath libdir
   If output-file is a library, it will eventually be installed in libdir. If output-file is a
   program, add libdir to the run-time path of the program. On platforms that dont support
   hardcoding library paths into executables and only search PATH for shared libraries, such as
   when output-file is a Windows (or other PE platform) DLL, the .la control file will be
   installed in libdir, but see -bindir above for the eventual destination of the .dll or other
   library file itself.

-R libdir
   If output-file is a program, add libdir to its run-time path. If output-file is a library,
   add -Rlibdir to its dependency_libs, so that, whenever the library is linked into a program,
   libdir will be added to its run-time path.

-shared
   If output-file is a program, then link it against any uninstalled shared libtool libraries
   (this is the default behavior). If output-file is a library, then only create a shared
   library. In the later case, libtool will signal an error if it was configured with
   --disable-shared, or if the host does not support shared libraries.

-shrext suffix
   If output-file is a libtool library, replace the systems standard file name extension for
   shared libraries with suffix (most systems use .so here). This option is helpful in certain
   cases where an application requires that shared libraries (typically modules) have an
   extension other than the default one. Please note you must supply the full file name
   extension including any leading dot.

-static
   If output-file is a program, then do not link it against any uninstalled shared libtool
   libraries. If output-file is a library, then only create a static library.

-static-libtool-libs
   If output-file is a program, then do not link it against any shared libtool libraries. If
   output-file is a library, then only create a static library.

-version-info current[:revision[:age]]
   If output-file is a libtool library, use interface version information current, revision, and
   age to build it (see Versioning). Do not use this flag to specify package release
   information, rather see the -release flag.

-version-number major[:minor[:revision]]
   If output-file is a libtool library, compute interface version information so that the
   resulting library uses the specified major, minor and revision numbers. This is designed to
   permit libtool to be used with existing projects where identical version numbers are already
   used across operating systems. New projects should use the -version-info flag instead.

-weak libname
   if output-file is a libtool library, declare that it provides a weak libname interface. This
   is a hint to libtool that there is no need to append libname to the list of dependency
   libraries of output-file, because linking against output-file already supplies the same
   interface (see Linking with dlopened modules).

-Wc,flag
-Xcompiler flag
   Pass a linker-specific flag directly to the compiler. With -Wc,, multiple flags may be
   separated by commas, whereas -Xcompiler passes through commas unchanged.

-Wl,flag
-Xlinker flag
   Pass a linker-specific flag directly to the linker.

-XCClinker flag
   Pass a link-specific flag to the compiler driver (CC) during linking. If the output-file ends
   in .la, then a libtool library is created, which must be built only from library objects (.lo
   files). The -rpath option is required. In the current implementation, libtool libraries may
   not depend on other uninstalled libtool libraries (see Inter-library dependencies).

If the output-file ends in .a, then a standard library is created using ar and possibly ranlib.

If output-file ends in .o or .lo, then a reloadable object file is created from the input files
(generally using 'ld -r'). This method is often called partial linking.

Otherwise, an executable program is created.

EOF
}



#   Function:       System
#       Execute a system command
#
#   Parameters:
#       cmd -               Command.
#
#   Returns:
#       return-code (-1 = exec error, -2 = core, application return code).
#
sub
System                  #(cmd)
{
    my ($cmd) = @_;

    Verbose "libtool: $cmd\n" if (!$o_silent);
    my $ret = system($cmd);
    $ret = __SystemReturnCode($ret);
    print "libtool: result=$ret\n" if (!$o_silent);
    return $ret;
}


#   Function:       __SystemReturnCode
#       Decode the return code from a system() call
#
#   Parameters:
#       rcode -             Return code.
#
#   Returns:
#       return-code
#
sub
__SystemReturnCode($)   #(retcode)
{
    my $rcode = 0;
    my $rc = shift;

    if ($rc == -1) {
        $rcode = -1;                            # task exec error
    } elsif ($rc & 127) {
        $rcode = -2;                            # cored
    } elsif ($rc) {
        $rcode = $rc >> 8;                      # application return code
    }
    return $rcode;
}


sub
Label {
    return "libtool @_";
}


sub
Debug {
    if ($o_verbose || $o_debug)  {
        print Label("(D) ") . sprintf( shift, @_ ) . "\n";
    }
}


sub
Verbose {
    if ($o_verbose)  {
        print Label("(V) ") . sprintf( shift, @_ ) . "\n";
    }
}


sub
Warning {
    print Label("(W) ") . sprintf( shift, @_ ) . "\n";
}


sub
Error {
    print Label("(E) ") . sprintf( shift, @_ ) . "\n";
    print @_;
    exit(3);
}

#end
