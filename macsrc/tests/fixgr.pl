#!/usr/bin/perl
# $Id: fixgr.pl,v 1.1 2014/11/16 17:27:44 ayoung Exp $
# -*- tabs: 8; indent-width: 4; -*-
# GRIEF global symbol renamer.
#
#

use strict;
use warnings;

use Cwd 'realpath', 'getcwd';
use Getopt::Long;
use File::Copy;
use File::Basename;

exit &main();

sub
main()
{
    my $o_dryrun = 0;
    my $o_help = 0;

    my $ret = GetOptions(
        'dryrun'    => \$o_dryrun,
        'help'      => \$o_help
        );

    opendir(DIR, '.') or
        die "error opening dir <.> : $!\n";
    my @FILES = grep /\.cr$/, readdir(DIR);
    close DIR;

    foreach my $file (sort @FILES) {
        print "parsing <$file>\n";

        open(FILE, "<${file}") or
            die "cannot open <${file}> : $!\n";
        my @lines = <FILE>;
        close(FILE);

        foreach (@lines) {
            s/\s+$//g;
            s/"crisp\.h"/"grief.h"/g;
            s{"../crisp\.h"}{"../grief.h"}g;
        }

        rename ($file, "${file}.sav") or
            die "cannot rename <${file}> : $!\n";

        open(FILE, ">${file}") or
            die "cannot create <${file}> : $!\n";
        binmode (FILE);
        foreach (@lines) {
            print FILE $_;
            print FILE "\n";
        }
        close(FILE);
    }
    return 0;
}

1;

