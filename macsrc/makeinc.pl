#!/usr/bin/perl
# $Id: makeinc.pl,v 1.14 2014/10/27 23:28:24 ayoung Exp $
# Generate 'grief.h' from the embedded export statements within the source.
# -*- mode: perl; tabs: 8; indent-width: 4; -*-
#
#
#

BEGIN {
    my ($var) = $ENV{"PERLINC"};

    if ( defined($var) && -d $var ) {           # import PERLINC
        my ($quoted_var) = quotemeta( $var );
        push (@INC, $var)
            if ( ! grep /^$quoted_var$/, @INC );

    } elsif ( $^O eq "MSWin32" ) {              # ActivePerl (defaults)
        if ( ! grep /\/perl\/lib/, @INC ) {
            push (@INC, "c:/perl/lib")  if (-d "c:/perl/lib");
            push (@INC, "/perl/lib")    if (-d "/perl/lib");
        }
    }
}

use strict;
use warnings;

my $o_griefh = "grief.h";
my @griefh_source = (
        "./grief_head.h",
        "../include/edfeatures.h",
        "../include/edmacros.h",
        "../include/edstruct.h",
        "../include/edtermio.h",
        "../include/patmatch.h",
        "../include/iniparser.h",
        "../gr/m_search.h",
        "../gr/m_tokenize.h",
        "../gr/m_spell.h",
        "../gr/dialog.h",
        "../gr/echo.h",
        "../gr/mouse.h",
        "../gr/syntax.h",
        "../gr/wild.h",
        "./grief_tail.h"
        );

my $o_debugh = "debug.h";
my @debugh_source = (
        "../gr/debug.h"
        );

sub Main();
sub Export($$;$);
sub Usage;

Main();
exit 0;


#   Main ---
#       Mainline
#..
sub
Main()
{
    my $argc = 0;                               # argument count
    my $cmd = "";

    while ($_ = $ARGV[ $argc++ ]) {
        last if ( ! /^-/ );                     # end of options

        /^-h$|^-help$|^--help$|^--usage$|^-?$/ && Usage();

        Usage("unknown option '$_'\n");
    }

    $cmd = $ARGV[$argc-1];
    $cmd = "" if (! defined($cmd));

    if ($cmd ne "") {
        Usage();
    }

    Export($o_griefh, \@griefh_source);
    Export($o_debugh, \@debugh_source, "DEBUG_H");
}

sub
Export($$;$)
{
    my ($output, $source, $guard) = @_;

    open(OUT, ">$output") or
            die "can't create '$output' : $!";

    print "Creating $output\n";

    if ($guard) {
        print OUT "#ifndef MACSRC_${guard}_INCLUDED\n";
        print OUT "#define MACSRC_${guard}_INCLUDED\n";
        if (1 == scalar @$source) {
            print OUT <<HEADER;
/* -*- mode: cr; indent-width: 4; -*- */
/*  
 *  An auto-generated file, do not modify
 */

HEADER
            }
    }

    foreach my $file (@$source) {
        open(IN, "<$file") or
            die "can't open '$file' : $1";

        my $export = 0;
        my $enum_value;

        while (<IN>) {

            if ( /^\/\*--export--\*\// ) {
                $export = 1;
                next;
            }

            if ( /^\/\*--export--enum--\*\// ) {
                $enum_value = 0;
                $export = 2;
                next;
            }

            if ( /^\/\*--export--defines--\*\// ) {
                $export = 3;
                next;
            }

            if ($export) {
                if ( /^\/\*--end--\*\// ) {
                    print OUT "\n";
                    $export = 0;
                    next;
                }

                if ($export == 2) {
                    #   Reformat enum data-types
                    #..
                    s/^\s+([A-Z0-9_]+)          # TOKEN=value, comment
                        \s*=
                        \s*([-()<>~^&|_0-9A-Z ]+),?     # symbols and binary operators
                        \s*(\/*.*\*\/)/
                        "#define $1 " .
                            ' 'x(23-length($1)) . "$2 " .
                            ' 'x(11-length($2)) . "$3"/exi ||

                    s/^ \s+([A-Z0-9_]+)[,]?     # TOKEN=value,
                        \s*=
                        \s*([-()<>~&^|_0-9A-Z ]+)       # symbols and binary operators
                        .*$/
                        "#define $1 " .
                            ' 'x(23-length($1)) . "$2"/exi ||

                    s/^\s+([A-Z0-9_]+)()[,]?    # TOKEN, comment
                        \s*(\/*.*\*\/)/
                        "#define $1 " .
                            ' 'x(23-length($1)) . "$enum_value " .
                            ' 'x(11-length("$enum_value")) . "$3"/exi ||

                    s/^ \s+([A-Z0-9_]+)[,]?     # TOKEN,
                        .*$/
                        "#define $1 " .
                            ' 'x(23-length($1)) . "$enum_value"/exi;

                    $enum_value = $2            # new value
                        if (defined($2) && $2 ne "");

                    ++$enum_value               # increment enum value
                        if (defined($1));

                } elsif ($export == 3) {
                    #   Reformat defines
                    #
                    #   define XXX numeric /*comment*/
                    #   define XXX numeric
                    #..
                    s/^\#define\s+([A-Z0-9_]+)
                        \s+([0-9a-fx]+)
                        \s+(\/*.*\*\/)/
                        "#define $1 " .
                            ' 'x(23-length($1)) . "$2 " .
                            ' 'x(11-length($2)) . "$3"/exi ||

                    s/^\#define\s+([A-Z0-9_]+)
                        \s+([0-9a-fx]+)
                        .*$/
                        "#define $1 " .
                            ' 'x(23-length($1)) . "$2"/exi;
                }

                print OUT $_;
            }
        }
        close(IN);
    }

    if ($guard) {
        print OUT "#endif  //MACSRC_${guard}_INCLUDED\n";
        print OUT "/*end*/\n";
    }
    close(OUT);
}


#   Usage ---
#       Makelib command line usage.
#..
sub
Usage
{
    my ($msg) = @_;

    print "makelib $msg\n\n"
        if ( $msg ne "" );
    print "Usage: perl makeinc.pl [options]\n".
"Valid options:\n".
"\t-help         Help.\n".
"\n";
    exit(42);
}

#end
