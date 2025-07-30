#!/usr/bin/perl
# $Id: updateyear.pl,v 1.4 2025/02/07 03:03:23 cvsuser Exp $
# -*- mode: perl; tabs: 8; indent-width: 4; -*-
# Update the copyright year within the specified files
#
# Copyright (c) 2012 - 2024, Adam Young.
# All Rights Reserved
#

use strict;
use warnings;

use Cwd 'realpath', 'getcwd', 'abs_path';
use Getopt::Long;
use File::Copy;                                 # copy()
use File::Basename;
use POSIX 'asctime';

my $CWD         = getcwd();                     # current working directory
my $ROOT        = abs_path(dirname($0));        # script path

sub Main();
sub Usage;

my $o_dryrun    = 0;
my $o_backup    = 1;
my $o_glob      = 0;

Main();
exit 0;

#   Function:           Main
#       main
#
#   Parameters:
#       ARGV - Argument vector.
#
#   Returns:
#       nothing
#
sub
Main()
{
    my $o_directory = '';
    my $o_help = 0;

    my $ret
        = GetOptions(
                'dryrun'        => \$o_dryrun,
                'run'           => sub {$o_dryrun = 0},
                'backup',       => \$o_backup,
                'nobackup',     => sub {$o_backup = 0},
                'directory=s'   => \$o_directory,
                'glob'          => \$o_glob,
                'help'          => \$o_help
                );

    Usage() if (!$ret || $o_help);
    Usage("expected <file ...>") if (scalar @ARGV < 1);

    $o_directory .= "/" if ($o_directory);
    foreach (@ARGV) {
        if ($o_glob) {                          # internal glob
            my @files = glob("${o_directory}$_");
            print "glob: @files\n";
            foreach (@files) {
                Process($_);
            }
        } else {
            Process("${o_directory}$_");        # for-each
        }
    }
}


#   Function:           Usage
#       command line usage.
#
#   Parameters:
#       [msg] - Optional message.
#
#   Returns:
#       nothing
#
sub
Usage                   # ([message])
{
    my ($message) = @_;

    print "\nupdateyear: $message\n\n"
        if ($message ne "");

    print <<EOU;
    print Usage: perl updateyear.pl [options] <source>

Options
    --help                  Help.

    --dryrun                Dryrun mode, report yet dont modify any source.
    --run                   Execute.

    --[no]backup            Backup original source prior to any modifications (default=yes).

    --directory=<path>      Directory prefix (default: none).
    --glob                  Perform interal glob on specified names.

EOU
    exit(42);
}


#   Function:           Process
#       Process the specified source image 'file'.
#
#   Parameters:
#       file - Source file.
#
#   Returns:
#       nothing
#
sub
Process($)              # (file)
{
    my ($file) = @_;

    my $ext = ($file =~ /\.([^.])$/ ? uc($1) : "");

    my ($lines, $result)
            = load($file, $ext);

    if ($result > 0) {
        backup($file) if ($o_backup);

        if (! $o_dryrun) {
            open(OUT, ">$file") or
                die "cannot recreate '$file' : $!";
            foreach (@$lines) {
                print OUT $_;
            }
            close(OUT);
        }
    }
}


sub
load($$)                # (file)
{
    my ($file) = @_;
    my $result = 0;
    my @lines;

    open(IN, "<$file") or
        die "can't open '$file' : $!";
    while (<IN>) {
        chomp(); chomp();
        if (! $result) {
            if (/Copyright.*[ -]+20[12]\d.*Adam/i) {
                my $org = $_;
                if (s/ -[ ]*2020/ - 2025/ or
                    s/ -[ ]*2021/ - 2025/ or
                    s/ -[ ]*2022/ - 2025/ or
                    s/ -[ ]*2023/ - 2025/ or
                    s/ -[ ]*2024/ - 2025/) {
                    print "dryrun: update\n- <$org>\n+ <$_>\n"
                        if ($o_dryrun);
                    $result = 1;
                }
            }
        }
        push @lines, $_."\n";
    }
    close(IN);

    return (\@lines, $result);
}


sub
backup($)               # (file)
{
    my ($file) = @_;

    if ($o_dryrun) {
        print "dryrun: rename($file, $file.bak)\n";
    } else {
        rename ($file, "${file}.bak") or
            die "can't backup '$file' : $!";
    }
}


sub
trimnl($)
{
    $_ = shift;
    s/\n//g;
    return $_;
}

#end

