#!/usr/bin/perl
# $Id: pkgdepends.pl,v 1.1 2025/01/06 09:46:11 cvsuser Exp $
# package dependencies (alpha)
#

use strict;
use warnings 'all';

use Getopt::Long;
use File::Basename;
use Data::Dumper;

my $overbose    = 0;
my $orecursive  = 0;
my $ounique     = 0;
my $opackage    = 0;
my $output      = 'depend.deb';
my $odelim      = undef;
my $ohelp       = 0;

my %UNIQ;                   # unique map
my @DEPENDS     = (         # dependencies
        'libhunspell',
        'libarchive',
        'libncurses'                            # also: libncursesw
        );

sub Depends($$);
sub Verbose;

Usage() if (0 == GetOptions(
                'recursive'     => \$orecursive,
                'unique'        => \$ounique,
                'package'       => sub {$opackage=1;},
                'all-packages'  => sub {$opackage=2;},
                'output=s'      => \$output,    # TODO
                'delim=s'       => \$odelim,
                'overbose'      => \$overbose,
                'help'          => \$ohelp)
                        || $ohelp);
$odelim ||= "\t";

my $input = shift @ARGV;
die "pkgdepends: unexpected options \"@ARGV\"\n"
        if (scalar @ARGV);

die "pkgdepends: app-name not defined\n"
        if (! $input);

exit Depends($input, 0);

sub
Depends($$)     # (input, depth)
{
        my ($input, $depth) = @_;

        Verbose("Input: ${input}");
        return if $UNIQ{$input} and $ounique;
        $UNIQ{$input} = 1;

        print "$odelim"x$depth . $input . "\n";
        if ($opackage) {
                if ($input =~ /\.so(\.|$)/) {   # package lookup
                    if ($opackage >= 2 || grep { $input =~ /$_/ } @DEPENDS) {
                        system("apt-file find ${input}");
                    }
                }
        }
        return if (!$orecursive && $depth);
        ++$depth;

        chomp(my @ldd = `/usr/bin/ldd ${input}`);
        Verbose("Libraries:\n@ldd");

        foreach my $elm (@ldd) {

                # Examples:
                #
                #   not a dynamic executable
                #   statically linked
                #
                #   linux-vdso.so.1 (0x00007ffe4776b000)
                #   libstdc++.so.6 => /lib/x86_64-linux-gnu/libstdc++.so.6 (0x00007fac0d591000)
                #   libz.so.1 => not found
                #

                $elm =~ s/^\s+//g;
                $elm =~ s/\s+$//g;

                next if not $elm;

                if (($elm =~ /not a dynamic executable/) or ($elm =~ /statically linked/)) {
                        return;

                } elsif ($elm =~ /not found/) {
                        print "$odelim"x$depth . $elm . "\n";
                        $UNIQ{$elm} = 1;
                        next;
                }

                my @newlibs = split(/\s+/,$elm);
                Verbose(Dumper(\@newlibs))
                        if $overbose;

                if (scalar(@newlibs) < 4) {
                        print "$odelim"x$depth . $newlibs[0] . "\n";
                        $UNIQ{$newlibs[0]} = 1;
                        next;
                }

                if ($orecursive) {
                        my $lib = $newlibs[2];
                        Verbose("recursive definitios <$lib>");
                        Depends($lib, $depth);

                } else {
                        my $lib = $newlibs[0];
                        Depends($lib, $depth);
                }
        }
}


sub
Verbose         # ([message])
{
        if ($overbose) {
                print "(V) " . sprintf(shift, @_) . "\n";
        }
}


sub
Usage           # ([message])
{
        print "\npkgdepends @_\n\n" if (@_);
        print <<EOU;

Usage: perl pkgdepends [options]

Options:
    --package               apt-file lookup, for well known dependencies
    --all-packages          otherwise for all dependencies

    --output <file>         Output file.
    --verbose

    --help                  Help.

EOU
        exit 3;
}

1;

#end
