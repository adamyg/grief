#!/usr/bin/perl -w
# -*- mode: perl; -*-
# $Id: esdbtable.pl,v 1.2 2012/09/03 23:10:17 ayoung Exp $
#
# iconv esdb table generated for windows, derived from the esdb Makefiles
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
sub cmd_esdb;
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


#   esdbtable ...
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

    if ('alias' eq $o_mode) {
        cmd_alias(@ARGV);

    } elsif ('dir' eq $o_mode) {
        cmd_dir(@ARGV);

    } elsif ('esdb' eq $o_mode) {
        cmd_esdb(@ARGV);

    } else {
        Usage("unknown mode <$o_mode>");
    }
    return 0;
}


#   Function:           cmd_alias
#       generate esdb.alias
#
sub
cmd_alias($)                #(subdirs)
{
    my (@ALIAS) = @_;
    my $esdb_alias = 'esdb.alias';

    if ($o_glob) {
        my @SUBDIRS = @ALIAS;

        @ALIAS = ();
        foreach (@SUBDIRS) {
            my $subdir = $_;

            opendir(DIR, $subdir) or
                die "cannot read directory <$subdir> : $!\n";
            push @ALIAS, grep /\.alias/, readdir(DIR);
            close(DIR);
        }
    }

    open(OUT, ">${esdb_alias}") or
        die "cannot create <$esdb_alias> : $!\n";

    print "generating: <$esdb_alias>\n";
    foreach (@ALIAS) {
        print "\t$_\n";

        if (/^[^.]+\.alias$/) {                     # <name>.alias
            my $base = basename($_, '.alias');
            my $sep = (exists $NOSEP{$base} ? '' : '-');

            open(IN, "<$_") or
                die "cannot open <$_> : $!\n";
            print OUT "# $base\n";
            while (<IN>) {
                chomp; s/\s+^//;
                next if (/^\s*#/ || /^\s*$/);
                my @BITS = split(/\s+/, $_);
                my $name = shift @BITS;
                foreach (@BITS) {
                    printf OUT "%-32s%s%s%s\n", $_, $base, $sep, $name;
                }
            }
            close(IN);
            print OUT "\n";

        } elsif (/esdb\.alias\.([^.]+).src$/) {     # esdb.alias.<name>.src
            open(IN, "<$_") or
                die "cannot open <$_> : $!\n";
            while (<IN>) {
                printf OUT $_;
            }
            close(IN);
            print OUT "\n";

        } else {
            print "\tunknown 'alias' format\n";
        }
    }
    close(OUT);
}


#   Function:           cmd_dir
#       generate esdb.dir
#
sub
cmd_dir($)                  #(subdirs)
{
    my (@PART) = @_;
    my $esdb_dir = 'esdb.dir';

    if ($o_glob) {
        my @SUBPART = @PART;

        @PART = ();
        foreach (@SUBPART) {
            my $subdir = $_;

            opendir(DIR, $subdir) or
                die "cannot read directory <$subdir> : $!\n";
            push @PART, grep /\.(part|dir)/, readdir(DIR);
            close(DIR);
        }
    }

    open(OUT, ">${esdb_dir}") or
        die "cannot create <$esdb_dir> : $!\n";

    print "generating: <$esdb_dir\n";
    foreach (@PART) {
        print "\t$_\n";

        if (/^[^.]+\.part$/) {                      # <name>.part
            my $base = basename($_, '.part');
            my $sep = (exists $NOSEP{$base} ? '' : '-');
            my $dir = dirname($_);

            open(IN, "<$_") or
                die "cannot open <$_> : $!\n";
            print OUT "# $base\n";
            while (<IN>) {
                chomp; s/\s+^//;
                next if (/^\s*#/ || /^\s*$/);
                my $name = $base.$sep.$_;
                my $filename = "$dir/$name.esdb";
                $filename =~ s/:/\@/;
                printf OUT "%-32s%s\n", $name, $filename;
            }
            close(IN);
            print OUT "\n";

        } elsif (/esdb\.dir\.([^.]+).src$/) {       # esdb.dir.<name>.src
            open(IN, "<$_") or
                die "cannot open <$_> : $!\n";
            while (<IN>) {
                printf OUT $_;
            }
            close(IN);
            print OUT "\n";

        } else {
            print "\tunknown 'dir' format\n";
        }
    }
    close(OUT);
}


#   Function:           cmd_esdb
#       generate xxx.esdb images
#
sub
cmd_esdb($)             #(subdirs)
{
    my (@PART) = @_;

    if ($o_glob) {
        my @SUBPART = @PART;

        @PART = ();
        foreach (@SUBPART) {
            my $subdir = $_;

            opendir(DIR, $subdir) or
                die "cannot read directory <$subdir> : $!\n";
            push @PART, grep /\.src$/, readdir(DIR);
            close(DIR);
        }
    }

    #reference data
    my %UTFDATA = (                                 # source: UTF/Makefile.inc
            UTF_16_mod      => 'UTF1632',
            UTF_16_var      => 'utf16',
            UTF_16BE_mod    => 'UTF1632',
            UTF_16BE_var    => 'utf16,big,force',
            UTF_16LE_mod    => 'UTF1632',
            UTF_16LE_var    => 'utf16,little,force',
            UTF_32_mod      => 'UTF1632',
            UTF_32_var      => 'utf32',
            UTF_32BE_mod    => 'UTF1632',
            UTF_32BE_var    => 'utf32,big,force',
            UTF_32LE_mod    => 'UTF1632',
            UTF_32LE_var    => 'utf32,little,force',
            UTF_8_mod       => 'UTF8',
            UTF_8_var       => 'utf8',
            UTF_7_mod       => 'UTF7',
            UTF_7_var       => 'utf7'
            );

    my %BIG5DATA;

    open(VAR, "<BIG5/Big5.variable") or
        die "cannot open <BIG5/Big5.variable> : $!\n";
    while (<VAR>) {
        chomp; s/\s+^//;
        next if (/^\s*#/ || /^\s*$/);
        my @bits = split(/\s+/, $_);
        $BIG5DATA{$bits[0]}=$bits[1];
    }
    close(VAR);

    #generate
    my $bindir = ($o_bindir ? "${o_bindir}/" : "");
    my $outdir = ($o_outdir ? $o_outdir : ".");

    print "generating: esdb images <$outdir>\n";

    foreach (@PART) {
        if (/^[^.]+\.src$/) {                       # <name>.part
            my $source = $_;
            my $base   = basename($_, '.src');
            my $dir    = dirname($_);
            my $sep    = (exists $NOSEP{$base} ? '' : '-');
            my $part   = "${dir}/${base}.part";
            my $dstdir = "${outdir}/${dir}";

            if (-f $part) {                         # foreach(part)
                open(PART, "<${part}") or
                    die "cannot open <$_> : $!\n";

                print "\t$source";
                (-d $dstdir || mkdir($dstdir)) or
                    die "unable to create directory <${dstdir}> : $!\n";

                my $srctext;
                open(SRC, "<${source}") or
                    die "cannot open <$source> : $!\n";
                while (<SRC>) {
                    $srctext .= $_;
                }
                close(SRC);

                while (<PART>) {
                    chomp; s/\s+^//;
                    next if (/^\s*#/ || /^\s*$/);

                    $part = $_;
                    $part =~ s/:/\@/;

                                                    # specialised 'src' image, ignore
                    next if (-f "${dir}/${base}${sep}${part}.src");

                    print " ${sep}${part}";
                    $source = "${base}${sep}${part}.src.tmp";
                    open(SRC, ">${source}") or
                        die "cannot create <$source> : $!\n";
                    
                    my $outtext = $srctext;         # mangle 'src' image
                    if ('MAC' eq $base) {
                        $outtext =~ s/changeme/${part}/;

                    } elsif ('ARMSCII' eq $base || 'EBCDIC' eq $base ||
                                'ISO-8859' eq $base || 'ISO646' eq $base) {
                        $outtext =~ s/${base}-x/${base}-${part}/g or
                            die "\n\t\t${base}: unable to convert name\n";

                    } elsif ('CP' eq $base || 'DEC' eq $base || 'KOI' eq $base) {
                        $outtext =~ s/${base}x/${base}${part}/g or
                            die "\n\t\t${base}: unable to convert name\n";

                    } elsif ('GEORGIAN' eq $base) {
                        my $gpart = $part;
                        $gpart =~ s/-/:/;
                        $outtext =~ s/GEORGIANy/GEORGIAN-${gpart}/g or
                            die "\n\t\t${base}: unable to convert name\n";

                    } elsif ('Big5' eq $base) {
                        (exists $BIG5DATA{$part}) or
                            die "\n\t\tBig5: missing reference data for <$part>\n";

                        $outtext =~ s/encoding/Big5-${part}/ or
                            die "\n\t\t${base}: unable to convert name\n";

                        $outtext =~ s/variable/$BIG5DATA{$part}/ or
                            die "\n\t\t${base}: unable to convert variable\n";


                    } elsif ('UTF' eq $base) {
                        (exists $UTFDATA{"UTF_${part}_mod"} && exists $UTFDATA{"UTF_${part}_var"}) or
                            die "\n\t\tUTF: missing reference data for <$part>\n";

                        $outtext =~ s/UTF-x/UTF-${part}/ or
                            die "\n\t\t${base}: unable to convert name\n";

                        $outtext =~ s/UTF-mod/$UTFDATA{"UTF_${part}_mod"}/ or
                            die "\n\t\t${base}: unable to convert mod\n";

                        $outtext =~ s/UTF-var/$UTFDATA{"UTF_${part}_var"}/ or
                            die "\n\t\t${base}: unable to convert var\n";

                    } else {
                        die "\n\tunknown src '$base' image handler\n";
                    }
                    print SRC $outtext;
                    close(SRC);

                    my $dest = "${dstdir}/${base}${sep}${part}.esdb";
                    if ($o_backup) {
                        my $bak  = "${dest}.bak";
                        unlink($bak); rename($dest, $bak);
                    } else {
                        unlink($dest);
                    }
                    System("${bindir}mkesdb -o ${dest} ${source}");
                    unlink $source;
                }

                print "\n";
                close(PART);

            } else {                                # single source
                print "\t$source\n";
                (-d $dstdir || mkdir($dstdir)) or
                    die "unable to create directory <${dstdir}> : $!\n";

                my $dest = "${dstdir}/${base}.esdb";
                if ($o_backup) {
                    my $bak  = "${dest}.bak";
                    unlink($bak); rename($dest, $bak);
                } else {
                    unlink($dest);
                }
                System("${bindir}mkesdb -o ${dest} ${source}");
            }

        } elsif (/esdb\.dir\.([^.]+).src$/) {       # esdb.dir.<name>.src

        } elsif (/esdb\.alias\.([^.]+).src$/) {     # esdb.alias.<name>.src

        } else {
            print "\tunknown 'esdb' format\n";
        }
    }
}


#   Function:           Usage
#       esdbtable command line usage
#
sub
Help {
    print STDERR "@_\n\n"
        if (@_);
    print STDERR << "EOF";
win32 iconv table creator (version: 0.1)

usage: esdbtable.pl [options] ...

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

usage: esdbtable.pl [options] ...

Options:

   --debug
       Trace of shell script execution to standard output.

   -h, --help
       Display a help message and exit.

   --mode=mode
       Use mode as the operation mode, mode must be set to one of the following:

        alias       Compile esdb.alias.

        dir         Compile esdb.dir.

        esdb        Build esdb descriptions (requires 'mkesdb').

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

    Verbose "esdbtable: $cmd\n" if (!$o_silent);
    my $ret = system($cmd);
    $ret = __SystemReturnCode($ret);
    Verbose "esdbtable: result=$ret\n" if (!$o_silent);
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
