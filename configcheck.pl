#!/usr/bin/perl
# $Id: configcheck.pl,v 1.2 2024/05/01 14:31:56 cvsuser Exp $
# -*- tabs: 8; indent-width: 4; -*-
# Check config.hin against acdefines.h
#
#
# Copyright (c) 1920 - 2024, Adam Young.
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

my $root = './';
my $include = './include';

sub check()
{
    #################################################################################

    my %CONFIG_H = ();

    if (! open(ACDEFINES, "${root}/acdefines.h")) {
        open(ACDEFINES, "${root}/acdefines.h") or
            die "cannot open ${root}/acdefines.h : $!";
    }
    while (<ACDEFINES>) {
        $_ =~ s/\s*(\n|$)//;                    # kill trailing whitespace & nl
        if (/^#define ([A-Z0-9_]+) /) {
            $CONFIG_H{$1} = 1;
        }
    }
    close ACDEFINES;

    #################################################################################

    my $text = '';

    if (! open(CONFIG, "${include}/config.hin")) {
        open(CONFIG, "${include}/config.hin") or
            die "cannot open ${include}/config.hin : $!";
    }
    while (<CONFIG>) {
        $_ =~ s/\s*(\n|$)//;                    # kill trailing whitespace & nl
        $text .= "$_\n";
    }
    close CONFIG;

    #################################################################################

    my @MISSING = ();
    my @MULTIPLE = ();

    foreach my $config (sort keys %CONFIG_H) {
        my $value = $CONFIG_H{$config};

        if ($text =~ /^([ \t]*#[ \t]*undef[ \t]+${config})([ \t]*|[ \t]+.+)$/m) {
            my $count = 0;

            while ($text =~ s/^([ \t]*#[ \t]*undef[ \t]+${config})([ \t]*|[ \t]+.+)$/#define ${config} ${value}/m) {
                ++$count;
            }
            push @MULTIPLE, $config
                if ($count > 1);

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

    if (scalar @MULTIPLE) {
        foreach my $config (@MULTIPLE) {
            print "multiple: $config\n";
        }
    }
}

check();

1;
