#!/usr/bin/perl
# $Id: makentypes.pl,v 1.14 2024/10/15 15:50:08 cvsuser Exp $
# Generate crntypes.h from the gen and grunch symbols.
# -*- mode: perl; tabs: 8; indent-width: 4; -*-
#
#
#
# This file is part of the GRIEF Editor.
#
# The GRIEF Editor is free software: you can redistribute it
# and/or modify it under the terms of the GRIEF Editor License.
#
# The GRIEF Editor is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# License for more details.
# ==end==
#

BEGIN {
    my ($var) = $ENV{"PERLINC"};

    if (defined($var) && -d $var) {             # import PERLINC
        my ($quoted_var) = quotemeta($var);
        push(@INC, $var)
            if (! grep /^$quoted_var$/, @INC);

    } elsif ( $^O eq "MSWin32" ) {              # ActivePerl (defaults)
        if (! grep /\/perl\/lib/, @INC) {
            push(@INC, "c:/perl/lib")  if (-d "c:/perl/lib");
            push(@INC, "/perl/lib")    if (-d "/perl/lib");
        }
    }
}

use strict;
use warnings 'all';

my $o_output  = 'crntypes.h';
my $o_debug   = 0;
my $o_yystype = 0;
my $o_value   = 0;

my @source = (
        'grunch.h',
        'yygen.h'
        );

sub Main();
sub Usage;

Main();
exit 0;


#   Function:           Main
#       Main body.
#
#   Parameters:
#       none
#
#   Returns:
#       none
#
sub
Main()
{
    my $argc = 0;                               # argument count
    my $cmd = "";

    while ($_ = $ARGV[ $argc++ ]) {
        last if ( ! /^-/ );                     # end of options

        /^-h$|^-help$|^--help$|^--usage$|^-?$/ && Usage();

        /^--debug$/ && ($o_debug = 1, next);

        /^--value$/ && ($o_value = 1, next);

        /^--yystype$/ && ($o_yystype = 1, next);

        Usage("unknown option '$_'\n");
    }

    $cmd = $ARGV[$argc-1];
    $cmd = "" if (!defined($cmd));

    Usage()
        if ($cmd ne "");

    Export();
}


#   Function:           Export
#       Export listed file content.
#
#   Parameters:
#       none
#
#   Returns:
#       none
#
sub
Export()
{
    print "Generating \"${o_output}\"'\n";

    open(OUT, ">${o_output}") or
        die "can't create ${o_output}";

    my @defines;
    my @body;
    my @yystype;
    my @keywd_values;
    my @type_values;
    my $values;

    foreach my $file (@source) {

        print "processing: ${file} ...\n";

        open(IN, "<$file") or
            die "cannot open $file : $!";

        my $export = 0;
        my $enum_value;

        if ($file ne 'grunch.h') {
            $values = \@keywd_values;
            $export = 3;                        # all defines
        }

        my $header = 0;

        while (<IN>) {
            chomp;

            # skip #ifndef inclusion headers
            if ($. <= 4) {                      # first 4 lines only
                if (0 == $header) {
                    if (/#ifndef / || /#if !defined/) {
                        $header = 1;
                        next;
                    }
                } elsif (1 == $header && /^#define /) {
                    $header = 2;
                    next;
                }
            }

            # markers
            if ( /^\/\*--export--\*\// ) {
                print "   export\n"
                    if ($o_debug);
                $export = 1;
                next;
            }

            if ( /^\/\*--export--enum([A-Z]*)--\*\// ) {
                my $type = $1;

                print "   enum\n"
                    if ($o_debug);

                $values = undef;
                if (defined $type) {
                    if ($type eq "KEYWORD") {
                        $values = \@keywd_values;
                    } elsif ($type eq "TYPE") {
                        $values = \@type_values;
                    }
                }
                $enum_value = 0;
                $export = 2;
                next;
            }

            if ( /^\/\*--export--defines--\*\// ) {
                print "   defines\n"
                    if ($o_debug);
                $export = 3;
                next;
            }

            if ($export) {
                if ( /^\s*$/ ) {
                    next;                       # blank

                } elsif ( /^\/\*--end--\*\// ) {
                    push @body, "\n";           # end marker
                    $export = 0;
                    next;

                } elsif ( /^\s*\/\*(.*)\*\/\s*$/ ) {
                    push @body, "/* $1 */\n";   # /* */ comments
                    next;

                } elsif ( /^\s*\/\/(.*)$/ ) {
                    push @body, "/* $1 */\n";   # // comments
                    next;
                }

                if (2 == $export) {
                    #
                    #   Reformat enum data-types
                    #
                    #       TOKEN=value,    /*comment*/
                    #       TOKEN=value,
                    #       TOKEN,          /*comment*/
                    #       TOKEN,
                    #..
                    s/^\s+([a-z0-9_]+)
                        \s*=
                        \s*([-0-9a-fx]+)[,]?
                        \s*(\/*.*\*\/)\s*$/
                        "    e_${1} " .
                            ' 'x(23-length($1)) . ($o_value ? "= $2, " : "= $1, ") .
                            ' 'x(17-length($o_value ? $2 : $1)) . "$3"/exi ||

                    s/^\s+([a-z0-9_]+)[,]?
                        \s*=
                        \s*([-0-9a-fx]+)
                        .*$/
                        "    e_${1} " .
                            ' 'x(23-length($1)) . ($o_value ? "= $2," : "= $1,")/exi ||

                    s/^\s+([a-z0-9_]+)([,]?)
                        \s*(\/*.*\*\/)\s*$/
                        "    e_${1} " .
                            ' 'x(23-length($1)) . "= $enum_value, " .
                            ' 'x(17-length($enum_value)) . "$3"/exi ||

                    s/^\s+([a-z0-9_]+)[,]?
                        .*$/
                        "    e_${1} " .
                            ' ' x (23-length($1)) . "= $enum_value,"/exi ||

                        next;                   # no match

                    $enum_value = $2            # new value
                        if (defined($2) && $2 ne "" && $2 ne ",");

                    push @$values, "${1}\n${enum_value}"
                        if ($values);

                    if (defined($1)) {          # increment enum value
                        if ($enum_value =~ /^0[xX]/) {
                            $enum_value = sprintf("0x%X", hex($enum_value)  + 1);
                        } else {
                            ++$enum_value;
                        }
                    }

                } elsif ($export >= 3) {
                    #
                    #   Reformat defines
                    #
                    #       define TOKEN numeric /*comment*/
                    #       define TOKEN numeric
                    #..
                    my $line = $_;              # original line

                    if ($o_yystype && 3 == $export) {
                        if (/^\#if.*def.*YYSTYPE_IS_DECLARED/) {
                            push @yystype, $line, "\n";
                            $export = 4;
                            next;
                        }
                    } elsif (4 == $export) {
                        next if (/^#.*line/);

                        push @yystype, $_, "\n";
                        if (/^#endif/) {
                            push @yystype, "\nextern YYSTYPE yylval;\n";
                            $export = 3;
                        }
                        next;
                    }

                    next if (/define\sYYSTYPE_IS_DECLARED/);

                    s/^\#define\s+([A-Z0-9_]+)
                        \s*([-0-9a-fx]+)
                        \s*(\/*.*\*\/)/
                        "    e_${1} " .
                            ' 'x(23-length($1)) . ($o_value ? "= $2, " : "= $1, ") .
                            ' 'x(17-length($o_value ? $2 : $1)) . "$3"/exi ||

                    s/^\#define\s+([A-Z0-9_]+)
                        \s*([-0-9a-fx]+)
                        .*$/
                        "    e_${1} " .
                            ' 'x(23-length($1)) . ($o_value ? "= $2," : "= $1,")/exi ||

                            next;               # no match

                    push @defines, $line, "\n";

                    push @$values, "${1}\n${2}"
                        if ($values);
                }

                push @body, $_, "\n";
            }
        }
        close (IN);
    }

    #   Header
    #
    print OUT <<EOT;
#ifndef GRUNCH_NTYPES_H_INCLUDED
#define GRUNCH_NTYPES_H_INCLUDED
/*
 *  Scanner enumerations and definitions
 *
 *      AN AUTO GENERATED FILE - DO NOT EDIT
 *
 *      Source: @source
 */

#ifndef YYTOKENTYPE
#define YYTOKENTYPE /* stop bison creating similar */
#endif

EOT

    #   Defines
    #
    foreach (@defines) {
        print OUT $_;
    }

    #   Definitions
    #
    print OUT<<EOT;
#if !defined(YYEMPTY)
#define YYEMPTY -2 /* bison 3.6.1 issues */
#endif

enum crntypes {
EOT

    foreach (@body) {
        print OUT $_;
    }

    print OUT<<EOT;
};
EOT

    #   YYSTYPE (bison/byacc)
    #
    if (scalar @yystype) {
        print OUT "\n";
        foreach (@yystype) {
            print OUT $_;
        }
        print OUT "\n";
    }

    #   Tables
    #
print OUT <<EOT;

#ifdef GRUNCH_NODEENUM_MAP
struct crmap {
    int val;
    const char *name;
};

EOT

    EnumDump('crenum_typetbl', \@type_values);
    EnumDump('crenum_keywdtbl', \@keywd_values);

print OUT <<EOT;
#endif  /*GRUNCH_NODEENUM_MAP*/

#endif  /*GRUNCH_NTYPES_H_INCLUDED*/
EOT

    close(OUT);
}


#   Function:           EnumDump
#       Makelib command line usage.
#
#   Parameters:
#       enum -              Enumeration name.
#       values -            Values.
#
#   Returns:
#       none
#
sub
EnumDump($$)
{
    my ($enum, $values) = @_;
    my ($ident, $t_ident) = (undef, undef);

    print OUT "struct crmap ${enum}[] = \n";
    print OUT "{\n";

    foreach (@$values) {
        my @parts = split(/\n/);

        #convert
        if ($parts[1] =~ /^[0-9]+$/i) {
            $t_ident = $parts[1];

        } elsif ($parts[1] =~ /^0x[0-9a-f]+$/i) {
            $t_ident = hex substr($parts[1], 2);
        }

        #delimitor
        print OUT "\n"
            if (defined $ident && $ident+1 != $t_ident);
        $ident = $t_ident;

        #data
        print OUT "    { $parts[1], " . ' 'x(8-length($parts[1])) . "\"$parts[0]\" },\n";
    }

    print OUT "};\n";
    print OUT "\n";
}


#   Function:           Usage
#       Makelib command line usage.
#
sub
Usage
{
    my ($msg) = @_;

    print "makentypes $msg\n\n"
        if ($msg ne "");

    print "Usage: perl makentypes.pl [options]\n" .
"\n".
"Options:\n".
"\t--help           Help.\n".
"\t--values         Values, otherwise enumeration value.\n".
"\t--debug          Debug.\n".
"\n";

    exit (42);
}

#end
