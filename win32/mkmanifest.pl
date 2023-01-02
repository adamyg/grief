#!/usr/bin/perl -w
# -*- mode: perl; -*-
# $Id: mkmanifest.pl,v 1.1 2023/01/02 08:17:55 cvsuser Exp $
#
# Copyright (c) 2022 - 2023, Adam Young.
# All rights reserved.
#
# The applications are free software: you can redistribute it
# and/or modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation, version 3.
#
# Redistributions of source code must retain the above copyright
# notice, and must be distributed with the license document above.
#
# Redistributions in binary form must reproduce the above copyright
# notice, and must include the license document above in
# the documentation and/or other materials provided with the
# distribution.
#
# The applications are distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
# ==end==
#

use strict;
use warnings 'all';

my $verbose = 0;
my $bi  = undef;
my $in  = undef;
my $out = undef;
my %vars;

sub buildinfo
{
        open (INFO, $bi) or
                die "can't open buildinfo <${bi}>: $!";
        while (<INFO>) {
                chomp;
                if (/^#define (GR_.*) (.*)$/i) {
                        my $var=$1;
                        my $val=$2;
                        $val = $1 if ($val =~ m/^\"([^"]*)\"$/); # remove "
                        print "DEF:${var}=${val}\n"
                                if ($verbose >= 2);
                        $vars{$var}=$val;
                }
        }
        close INFO;
}

sub generate
{
        open (INPUT, $in) or
                die "can't open manifest template <${in}>: $!";

        open (OUTPUT, '>', $out) or
                die "can't create <${out}>: $!";

        my $lines = 0;
        while (<INPUT>) {
                chomp;
                while (/\$\{(GR_[^}]+)\}/) {
                        my $var=$1;
                        my $val=$vars{$var};
                        print "VAR:${var}=${val}\n"
                                if ($verbose >= 1);
                        s/\$\{\Q${var}\E\}/${val}/g;
                }
                print OUTPUT $_."\n";
        }

        close(INPUT) or
                die "can't close <${in}>: $!";

        close(OUTPUT) or
                die "can't close <${out}>: $!";
}

sub usage
{
    die "usage: [options] <in> <out>\n".
        "\n".
        "Options\n".
        "  -D <define>=<value>      Variable definition\n".
        "  --defines <buildinfo>    Buildinfo values\n".
        "\n";
}


####

while (scalar @ARGV) {
        $_ = shift @ARGV;

        if (/^-D(.+)=(.+)$/) {                  # -D<define>=<value>
                $vars{$1}=$2;

        } elsif (/^--defines[=]?(.*)/) {        # --defines <buildinfo>
                $bi = ($1 ? $1 : shift @ARGV);

        } elsif (/^--verbose/) {                # --verbose
                $verbose += 1;

        } elsif (/^-/) {                        # unknown option
                usage();

        } else {
                $in = $_;
                $out = shift @ARGV;
                last;
        }
}

#print "BI: $bi\n" if ($bi);
#print "IN: $in\n";
#print "OUT:$out\n";

usage() if (scalar @ARGV or !$in or !$out or ($in eq $out));
buildinfo() if ($bi);
generate();

#end


