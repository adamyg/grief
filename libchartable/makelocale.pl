#!/usr/bin/perl
# $Id: makelocale.pl,v 1.10 2022/03/21 14:59:58 cvsuser Exp $
# Character set locale table generation
# -*- mode: perl; tabs: 8; indent-width: 4; -*-
#
# Copyright (c) 2010 - 2022, Adam Young.
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
#
#

BEGIN {
    my $var = $ENV{"PERLINC"};
    if (defined($var) && -d $var) {             # import PERLINC
        my ($qvar) = quotemeta($var);
        push (@INC, $var)
            if (! grep /^$qvar$/, @INC);
    } elsif ($^O eq "MSWin32") {                # ActivePerl (defaults)
        if (! grep /\/perl\/lib/, @INC) {
            push (@INC, "c:/perl/lib")  if (-d "c:/perl/lib");
            push (@INC, "/perl/lib")    if (-d "/perl/lib");
        }
    }
}


use strict;
use warnings;

use POSIX 'ctime';

my  $x_timestamp = ctime(time());
chomp($x_timestamp);

my  $x_warnings = 0;

sub Main();
sub codeset_clean($);
sub Usage;

Main();
exit 0;


#   Function:           Main
#       Main body.
#
#   Parameters:
#       none
#
#   Return:
#       none
#
#   Notes:
#       Data sources include
#
#           o /usr/shared/X11/locales/locale.alias
#           o /usr/share/config/charsets
#
sub
Main()                  #()
{
    my $o_locale_alias = 'locale.alias';        # X11 alias data
    my $argc = 0;                               # argument count
    my $cmd = "";

    #command line arguments
    while ($_ = $ARGV[$argc++]) {
        last if (! /^-/);                       # end of options

        /^-h$|^-help$|^--help$|^--usage$|^-?$/ && Usage();

        next if (/^--warn[ings]+$|^-w$/ && ++$x_warnings);

        Usage("unknown option '$_'\n");
    }

    $cmd = $ARGV[$argc-1];
    $cmd = "" if (! defined($cmd));
    Usage() if ($cmd ne "");

    #parse
    my %x_encoding;
    my %x_altnames;
    my %x_locales;

    my $prev_country = '';
    my $prev_codeset = '';

    open(IN, "<locale.alias") or
        die "can not open '$o_locale_alias' : $!\n";
    while (<IN>) {
        next if (/^#/);
        chomp;
        if (/^([^\s]+)\s+([^\s]+)$/) {
            #
            #   locale:
            #       [language[_territory]].[codeset][@modifier]]
            #
            my ($locale, $encoding) = ($1, $2);

            #locale alias
            $locale = $1                        # remove trailing ':'
                if ($locale =~ /^(.*):$/);

            $encoding = uc($1)                  # convert
                if ($encoding =~ /\.([A-Z0-9_@-]+)$/i);
            $encoding =~ s/^EUC/EUC_/;
            $encoding =~ s/^BIG5HKSCS/BIG5-HKSCS/;

            my $clean_encoding = $encoding;
            $clean_encoding = $1
                if ($clean_encoding =~ /([A-Z0-9_-]+)/i);
            $clean_encoding =~ s/[-]/_/g;       # csymbol safe
            $x_encoding{$clean_encoding} = $encoding;

            if ($locale =~ /^(.+)\.([A-Za-z0-9_@-]+)$/) {
                my $country = $1;
                my ($codeset, $modifier) = split(/\@/, $2);

                if (!$modifier) {
                    if ($country eq $prev_country) {
                        if (codeset_clean($prev_codeset) eq codeset_clean($codeset)) {
##print "     skipping ${locale}\n";
                            next;
                        }
                    }
                    if (codeset_clean($encoding) eq codeset_clean($codeset)) {
##print "     skipping ${locale}\n";
                        next;
                    }
                    $prev_country = $country;
                    $prev_codeset = $codeset;
                } else {
                    $prev_country = '';
                }

                $x_altnames{$codeset} = $clean_encoding;
                next if (uc($codeset) eq $encoding);
            }

##print "${locale}\n";
            $x_locales{$locale} = $clean_encoding;
        }
    }
    close (IN);

    #export
    print
"/*\n".
" *     Alias to encoding table, non-standard/legacy locales plus\n".
" *     locales without explicit code-sets\n".
" *\n".
" *     Generated:  $x_timestamp\n".
" *     By:         makelocale.pl\n".
" */\n".
"\n\n";
    foreach (sort keys %x_encoding) {
        my $clean_encoding = $_;
        my $encoding = $x_encoding{$_};

        printf
"static const char ECS_%s[] %*s= \"%s\";\n",
        $clean_encoding, 18 - length($clean_encoding), "", $encoding;
    }
    printf
"\n\n";

    print
"static const struct locale_alias {\n".
"     const char    *locale;\n".
"     const char    *charset;\n".
"} locale_alias_table[] = { \n";
    foreach (sort keys %x_locales) {
        my $locale = $_;
        my $encoding = $x_locales{$_};

        printf
"     { \"%s\",%*s ECS_%s },\n",
        $locale, 33 - length($locale), "", $encoding;
    }
    printf
"     };\n".
"\n\n";

    print
"static const struct charset_altname {\n".
"     const char    *altname;\n".
"     const char    *charset;\n".
"} charset_altname_table[] = { \n";
    foreach (sort keys %x_altnames) {
        my $altname = $_;
        my $encoding = $x_altnames{$_};

        next if (codeset_clean($altname) eq codeset_clean($encoding));

        printf
"     { \"%s\",%*s ECS_%s },\n",
        $altname, 33 - length($altname), "", $encoding;
    }
    printf
"     };\n".
"\n/*end*/\n";
}


sub
codeset_clean($)        #(codeset)
{
    my $codeset = shift;

    $codeset =~ s/[^A-Za-z0-9]//g;
    return lc($codeset);
}



#   Function:           Usage
#       Command line usage.
#
#   Parameters:
#       [msg] - Optional message.
#
#   Returns:
#       nothing
#
sub
Usage
{
    my $msg = @_;

    print "makelocale $msg\n\n"
        if ($msg);

    print
"usage: perl makelocale.pl [options]\n".
"\n".
"options:\n".
"\t-help                    help.\n".
"\t--warning, -w            enable warnings.\n".
"\n";

    exit(42);
}

#end
