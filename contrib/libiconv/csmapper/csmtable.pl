#!/usr/bin/perl -w
# -*- mode: perl; -*-
# $Id: csmtable.pl,v 1.3 2012/09/05 19:22:51 ayoung Exp $
#
# iconv cs table generated for windows, derived from the cs Makefiles
#
# Copyright (c) 2012 Adam Young.
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
# OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
# OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
# SUCH DAMAGE.
# ==end==
#

use strict;
use warnings 'all';

use Getopt::Long;
use File::Basename;
use File::Copy;
use Cwd;

my $o_help;
my $o_mode;
my $o_glob;
my $o_backup;
my $o_bindir;
my $o_outdir;
my $o_quiet;
my $o_silent = 0;
my $o_verbose = 0;
my $o_debug;
my $o_extra = '';

my %NOSEP = (
    MAC => 1, CP => 1, GB => 1, DEC => 1, KOI => 1
    );

sub cmd_alias;
sub cmd_dir;
sub cmd_mps;
sub Help;
sub Usage;
sub System;
sub __SystemReturnCode($);
sub Label;
sub Debug;
sub Verbose;
sub Warning;
sub Error;

exit &Main();


#   cstable ...
#
sub
Main
{
    Help() if (!scalar @ARGV);

    Getopt::Long::Configure("require_order");

    my $ret =
        GetOptions(
            'h|help'     => \$o_help,
            'mode=s'     => \$o_mode,
            'glob'       => \$o_glob,
            'b|bindir=s' => \$o_bindir,
            'o|outdir=s' => \$o_outdir,
            'backup'     => \$o_backup,
            'quiet'      => \$o_quiet,
            'no-quiet'   => sub {$o_quiet=0;},
            'silent'     => \$o_silent,
            'no-silent'  => sub {$o_silent=0;},
            'v|verbose'  => \$o_verbose,
            'no-verbose' => sub {$o_verbose=0;},
            'debug'      => \$o_debug);

    Help() if (!$ret);
    Usage() if ($o_help);
    Usage("missing arguments") if (!scalar @ARGV);

    if ('dir' eq $o_mode) {
        cmd_dir(@ARGV);

    } elsif ('pivot' eq $o_mode) {
        cmd_pivot(@ARGV);

    } elsif ('mps' eq $o_mode) {
        cmd_mps(@ARGV);

    } else {
        Usage("unknown mode <$o_mode>");
    }
    return 0;
}


#   Function:           cmd_dir
#       generate mapper.dir
#
sub
cmd_dir($)                  #(subdirs)
{
    my (@PART) = @_;
    my $mapper_dir = 'mapper.dir';

    if ($o_glob) {
        my @SUBPART = @PART;

        @PART = ();
        foreach (@SUBPART) {
            my $subdir = $_;

            opendir(DIR, $subdir) or
                die "cannot read directory <$subdir> : $!\n";
            push @PART, grep /\.(dir|part)/, readdir(DIR);
            close(DIR);
        }
    }

    open(O, ">${mapper_dir}") or
        die "cannot create <$mapper_dir> : $!\n";

    print "generating: <$mapper_dir\n";
    foreach (@PART) {
        print "\t$_\n";

        if (/^[^.]+\.part$/) {                      # <name>.part
            my $base = basename($_, '.part');
            my $sep = (exists $NOSEP{$base} ? '' : '-');
            my $dir = dirname($_);
            my $fmt = "%-32s%-32s%s\n";

            open(IN, "<$_") or
                die "cannot open <$_> : $!\n";
            print O "# $dir\n";

            my @IN;
            while (<IN>) {
                chomp; s/\s+^//; next if (/^\s*#/ || /^\s*$/);
                push @IN, $_;
            }

            if ('MAC' eq $base) {
                foreach my $i (@IN) {
                    printf O $fmt, "$i/UCS",            "mapper_std",       "APPLE/$i%UCS.mps";
                    printf O $fmt, "UCS/$i",            "mapper_std",       "APPLE/UCS%$i.mps";
                }

            } elsif ('ARMSCII' eq $base) {
                foreach my $i (@IN) {
                    printf O $fmt, "ARMSCII-$i/UCS",    "mapper_std",       "AST/ARMSCII-$i%UCS.mps";
                    printf O $fmt, "UCS/ARMSCII-$i",    "mapper_std",       "AST/UCS%ARMSCII-$i.mps";
                }

            } elsif ('CP' eq $base) {
                foreach my $i (@IN) {
                    my $f = $i;
                    $f =~ s/:/\@/;
                    printf O $fmt, "CP$i/UCS",          "mapper_std",       "CP/CP$f%UCS.mps";
                    printf O $fmt, "UCS/CP$i",          "mapper_std",       "CP/UCS%CP$f.mps";
                }

            } elsif ('EBCDIC' eq $base) {
                foreach my $i (@IN) {
                    printf O $fmt, "EBCDIC-$i/UCS",     "mapper_std",       "EBCDIC/EBCDIC-$i%UCS.mps";
                    printf O $fmt, "UCS/EBCDIC-$i",     "mapper_std",       "EBCDIC/EBCDIC-$i%UCS.mps";
                }

            } elsif ('GEORGIAN' eq $base) {
                foreach my $i (@IN) {
                    my $f = $i;
                    $f =~ s/:/\@/;
                    printf O $fmt, "GEORGIAN-$i/UCS",   "mapper_std",       "GEORGIAN/GEORGIAN-$f%UCS.mps";
                    printf O $fmt, "UCS/GEORGIAN-$i",   "mapper_std",       "GEORGIAN/UCS%GEORGIAN-$f.mps";
                }

            } elsif ('ISO646' eq $base) {
                foreach my $i (@IN) {
                    my $j = $i;
                    $j =~ s/:/\@/;
                    printf O $fmt, "ISO646-$i/UCS",     "mapper_646",       "ISO646/ISO646-$j%UCS.646";
                    printf O $fmt, "UCS/ISO646-$i",     "mapper_646",       "!ISO646/ISO646-$j%UCS.646";
                }

            } elsif ('ISO-8859' eq $base) {
                printf O $fmt, "ISO-8859-1/UCS",        "mapper_none",      "";
                printf O $fmt, "UCS/ISO-8859-1",        "mapper_zone",      "0x00-0xFF";

                foreach my $i (@IN) {
                    printf O $fmt, "ISO-8859-$i/UCS",   "mapper_std",       "ISO-8859/ISO-8859-$i%UCS.mps";
                    printf O $fmt, "UCS/ISO-8859-$i",   "mapper_std",       "ISO-8859/UCS%ISO-8859-$i.mps";
                    printf O $fmt, "ISO-8859-$i:GR/ISO-8859-$i",
                                                        "mapper_zone",      "0x00 - 0x7F : +0x80";
                    printf O $fmt, "ISO-8859-$i/ISO-8859-$i:GR",
                                                        "mapper_zone",      "0x80 - 0xFF : -0x80";
                    printf O $fmt, "ISO-8859-$i:GR/UCS",
                                                        "mapper_serial",    "ISO-8859-$i:GR/ISO-8859-$i,ISO-8859-$i/UCS";
                    printf O $fmt, "UCS/ISO-8859-$i:GR",
                                                        "mapper_serial",    "UCS/ISO-8859-$i,ISO-8859-$i/ISO-8859-$i:GR";
                }

            } elsif ('KOI8' eq $base) {
                foreach my $i (@IN) {
                    printf O $fmt, "KOI${i}EXT/UCS",    "mapper_std",       "KOI/KOI${i}%UCS.mps";
                    printf O $fmt, "UCS/KOI${i}EXT",    "mapper_std",       "KOI/UCS%KOI${i}.mps";
                }

                foreach my $i (@IN) {
                    printf O $fmt, "KOI${i}/UCS",       "mapper_parallel",  "GOST19768-74/UCS,KOI${i}EXT/UCS";
                    printf O $fmt, "UCS/KOI${i}",       "mapper_parallel",  "UCS/GOST19768-74,UCS/KOI${i}EXT";
                }

            } else {
                print "\tdir: unknown '$base' part format\n";
            }
            close(IN);
            print O "\n";
                                                    # <name>.dir.src
        } elsif (/^[^.]+\.dir\..*\.src$/ || /mapper.dir.src$/) {
            my $dir = dirname($_);

            print O "# $dir\n" if ($dir && $dir ne '.');
            open(IN, "<$_") or
                die "cannot open <$_> : $!\n";
            while (<IN>) {
                print O $_;
            }
            close(IN);
            print O "\n";

        } else {
            print "\tunknown 'dir' format\n";
        }
    }
    close(O);
}


#   Function:           cmd_pivot
#       generate cs.pivot
#
sub
cmd_pivot($)                #(subdirs)
{
    my (@PART) = @_;
    my $mapper_dir = 'charset.pivot';

    if ($o_glob) {
        my @SUBPART = @PART;

        @PART = ();
        foreach (@SUBPART) {
            my $subdir = $_;

            opendir(DIR, $subdir) or
                die "cannot read directory <$subdir> : $!\n";
            push @PART, grep /\.(pivot|part)/, readdir(DIR);
            close(DIR);
        }
    }

    open(O, ">${mapper_dir}") or
        die "cannot create <$mapper_dir> : $!\n";

    print "generating: <$mapper_dir\n";
    foreach (@PART) {
        print "\t$_\n";

        if (/^[^.]+\.part$/) {                      # <name>.part
            my $base = basename($_, '.part');
            my $sep = (exists $NOSEP{$base} ? '' : '-');
            my $dir = dirname($_);
            my $fmt = "%-32s%-32s1\n";

            open(IN, "<$_") or
                die "cannot open <$_> : $!\n";
            print O "# $dir\n";

            my @IN;
            while (<IN>) {
                chomp; s/\s+^//; next if (/^\s*#/ || /^\s*$/);
                push @IN, $_;
            }

            if ('MAC' eq$base) {
                foreach my $i (@IN) {
                    printf O $fmt, "${i}", "UCS";
                    printf O $fmt, "UCS", "${i}";    
                }

            } elsif ('ARMSCII' eq $base) {
                foreach my $i (@IN) {
                    printf O $fmt, "ARMSCII-${i}", "UCS";
                    printf O $fmt, "UCS", "ARMSCII-${i}";
                }

            } elsif ('CP' eq $base) {
                foreach my $i (@IN) {
                    printf O $fmt, "CP${i}", "UCS";
                    printf O $fmt, "UCS", "CP${i}";
                }

            } elsif ('EBCDIC' eq $base) {
                foreach my $i (@IN) {
                    printf O $fmt, "EBCDIC-${i}", "UCS";
                    printf O $fmt, "UCS", "EBCDIC-${i}";
                }

            } elsif ('GEORGIAN' eq $base) {
                foreach my $i (@IN) {
                    printf O $fmt, "GEORGIAN-${i}", "UCS";
                    printf O $fmt, "UCS", "GEORGIAN-${i}";
                }

            } elsif ('ISO-8859' eq $base) {
                foreach my $i (@IN) {
                    printf O $fmt, "ISO-8859-${i}", "UCS";  
                    printf O $fmt, "UCS", "ISO-8859-${i}";  
                    printf O $fmt, "ISO-8859-${i}:GR", "UCS";
                    printf O $fmt, "UCS", "ISO-8859-${i}:GR";
                }

            } elsif ('ISO646' eq $base) {
                foreach my $i (@IN) {
                    printf O $fmt, "ISO646-${i}", "UCS";
                    printf O $fmt, "UCS", "ISO646-${i}";
                }

            } elsif ('KOI8' eq $base) {
                foreach my $i (@IN) {
                    printf O $fmt, "KOI${i}", "UCS";
                    printf O $fmt, "UCS", "KOI${i}";
                }

            } else {
                print "\tpivot: unknown '$base' part format\n";
            }

            close(IN);
            print O "\n";
                                                    # <name>.pivot.src
        } elsif (/^[^.]+\.pivot\..*\.src$/ || /charset.pivot.src$/) {
            my $dir = dirname($_);

            print O "# $dir\n" if ($dir && $dir ne '.');
            open(IN, "<$_") or
                die "cannot open <$_> : $!\n";
            while (<IN>) {
                print O $_;
            }
            close(IN);
            print O "\n";

        } else {
            print "\tunknown 'pivot' format\n";
        }
    }
    close(O);
}


#   Function:           cmd_mps
#       generate xxx.mps images
#
sub
cmd_mps($)              #(subdirs)
{
    my (@PART) = @_;

    if ($o_glob) {
        my @SUBPART = @PART;

        @PART = ();
        foreach (@SUBPART) {
            my $subdir = $_;

            opendir(DIR, $subdir) or
                die "cannot read directory <$subdir> : $!\n";
            push @PART, grep /.*%.*\.src/, readdir(DIR);
            close(DIR);
        }
    }

    my $bindir = ($o_bindir ? "${o_bindir}/" : "");
    my $outdir = ($o_outdir ? $o_outdir : ".");

    print "generating: msp images <$outdir>\n";
    foreach (@PART) {
        print "\t$_\n";

        if (/^.*[\/\\].*\.src$/) {
            my $source = $_;
            my $base   = basename($_, '.src');
            my $dir    = dirname($_);
            my $dstdir = "${outdir}/${dir}";

            (-d $dstdir || mkdir($dstdir)) or
                die "unable to create directory <${dstdir}> : $!\n";

            my $dest = "${dstdir}/${base}.mps";
            if ($o_backup) {
                my $bak  = "${dest}.bak";
                unlink($bak); rename($dest, $bak);
            } else {
                unlink($dest);
            }
            System("${bindir}mkcsmapper -o ${dest} ${source}");

        } elsif (/^.*[\/\\].*\.646$/) {
            my $source = $_;
            my $base   = basename($_);
            my $dir    = dirname($_);
            my $dstdir = "${outdir}/${dir}";

            (-d $dstdir || mkdir($dstdir)) or
                die "unable to create directory <${dstdir}> : $!\n";

            my $dest   = "${dstdir}/${base}";
            if ($o_backup) {
                my $bak  = "${dest}.bak";
                unlink($bak); rename($dest, $bak);
            } else {
                unlink($dest);
            }
            copy($source, $dest);

        } else {
            print "\tunknown 'mps' format\n";
        }
    }
    close(O);
}


#   Function:           Usage
#       cstable command line usage
#
sub
Help {
    print STDERR "@_\n\n"
        if (@_);
    print STDERR << "EOF";
win32 csmapper table creator                            (version: 0.1)

usage: cstable.pl [options] ...

--help for help
EOF
    exit(1);
}


sub
Usage {
    print STDERR "@_\n\n"
        if (@_);
    print STDERR << "EOF";
win32 iconv table creator (version: 0.1)

usage: cstable.pl [options] ...

Options:

   --debug
       Trace of shell script execution to standard output.

   -h, --help
       Display a help message and exit.

   --mode=mode
       Use mode as the operation mode, mode must be set to one of the following:

        pivot       Compile cs.pivot.

        dir         Compile cs.dir.

        mps         Build csmapping descriptions (requires 'mkcsmapper').

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

EOF
   exit(1);
}


#   Function:           System
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

    Verbose "cstable: $cmd\n" if (!$o_silent);
    my $ret = system($cmd);
    $ret = __SystemReturnCode($ret);
    Verbose "cstable: result=$ret\n" if (!$o_silent);
    return $ret;
}


#   Function:           __SystemReturnCode
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
    return "libtool: @_";
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

