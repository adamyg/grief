#!/usr/bin/perl
# $Id: tredecl.pl,v 1.4 2024/06/15 08:23:27 cvsuser Exp $
# -*- tabs: 8; indent-width: 4; -*-
# tre.h import utility
# extern to cdecl
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

use POSIX 'asctime';
use Cwd 'realpath', 'getcwd';
use Getopt::Long;
use File::Copy;                                 # copy()

my $CWD                 = getcwd();
my $o_dryrun            = 0;
my $o_trein             = 'tre-master/lib/tre.h';
my $o_treout            = 'tre.h';

#   Main ---
#       Mainline
#
sub Trim($);
sub System($);
sub systemrcode($);

exit &main();

sub
main()
{
    my $o_help = 0;
    my $ret
        = GetOptions(
            'dryrun'    => \$o_dryrun,
            'in:s'      => \$o_trein,
            'out:s'     => \$o_treout,
            'help'      => \$o_help
            );

    Usage() if (!$ret || $o_help);
    Usage("unexpected arguments @ARGV") if (scalar @ARGV);

    open(TREOUT, ">${o_treout}") or
        die "cannot create <${o_treout}> : $!\n";

        open(TREIN, "<${o_trein}") or
            die "cannot open <${o_trein}> : $!";

        my $asctime = Trimr(asctime(localtime()));
        my $once = 1;

        while (<TREIN>) {
            if ($_ =~ /^#ifdef\s+__cplusplus/ && $once) {
                print TREOUT <<EOT;
#ifndef __cplusplus
/**
 *  modified by tredecl.pl, $asctime
 */
#if ((defined __WIN32__) || (defined _WIN32) || defined(__CYGWIN__))
# if defined(LIBTRE_STATIC) || !defined(LIBTRE_DLL)
   /* static library or cdecl calling convention */
#  ifdef __GNUC__
#  elif defined(_MSC_VER)
#   define TRE_DECL  __declspec(cdecl)
#  else
#   define TRE_EXPORT cdecl
#  endif
# else
   /* dynamic library, standard calling convention */
#  ifdef __LIBTRE_BUILD
#   ifdef __GNUC__
#    define TRE_DECL __attribute__((dllexport)) extern
#   else
#    define TRE_DECL __declspec(dllexport)
#   endif
#  else
#   ifdef __GNUC__
#   else
#    define TRE_DECL __declspec(dllimport)
#   endif
#  endif
# endif
#endif

#ifndef TRE_DECL
#define TRE_DECL     extern
#endif
#ifndef TRE_EXPORT
#define TRE_EXPORT
#endif

#else
EOT
                $once = 0;
            } else {
                if ($_ !~ /extern\s+\"C\"/) {
                    $_ =~ s/^extern\s+(.*)$/TRE_DECL $1 TRE_EXPORT/;
                }
                print TREOUT $_;
            }
        }

        close TREIN;
    close TREOUT;

    return 0;
}


sub
Trim($)                 # (text)
{
    my $s = shift;
    return '' if (!defined $s);
    $s =~ s/\s+$//;                             # trailing
    $s =~ s/^\s+//;                             # leading
    return $s;
}


sub
Triml($)                # (text)
{
    my $s = shift;
    return '' if (!defined $s);
    $s =~ s/^\s+//;                             # leading
    return $s;
}


sub
Trimr($)                # (text)
{
    my $s = shift;
    return '' if (!defined $s);
    $s =~ s/\s+$//;                             # trailing
    return $s;
}


#   Usage ---
#       Makelib command line usage.
#
sub
Usage                   # (message)
{
    print "\ntrecdecl @_\n\n" if (@_);
    print <<EOU;

TRE decl Utility

Usage: perl trecdecl.pl [options]

Options:
    --help              Help.

EOU
    exit(42);
}

#end

