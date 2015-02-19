#!/bin/perl
#$Id: mkerrors.pl,v 1.1 2014/11/16 17:45:57 ayoung Exp $
# Construct error include, libbsdfetch support script
#
#

use strict;
use warnings 'all';
use Getopt::Long;

my $opt_s	= undef;
my $opt_help	= 0;
my $opt_d	= 0;
my $opt_o	= undef;
my $opt_t	= undef;

Usage() if (0 == GetOptions(
		'help'  => \$opt_help,
		'd'     => \$opt_d,
		't=s'   => \$opt_t,
		'o=s'   => \$opt_o)
		    || $opt_help);

Usage("unexpected arguments $ARGV[1] ...") if (scalar @ARGV > 1);
Usage("expected source") if (! scalar @ARGV);

my $source = $ARGV[0];

Usage("unknown source type '$source', .error extension expected")
	if ($source !~ /^(.*)\.errors$/);

my $type = $1;

$opt_t = "${type}" if (! $opt_t);	# eg. ftp and http
$opt_o = "${type}err.h"			# ftp.error => ftperr.h
	if (! $opt_o);

Usage("missing/unknown table type '$opt_t', specify either -t ftp or -t http")
	if ($opt_t ne 'ftp' && $opt_t ne 'http');

my @errors;

print "Importing: ${source}\n";

while (<>) {
	chomp;
	if (/^(\d+)\s+([^\t]+)\s+(.*)$/) {
		print "IN: $1,$2,$3\n" if ($opt_d);
		push @errors, {
			NUM  => $1,
			ENUM => $2,
			DESC => $3
			};
	}
}

print "Building: ${opt_o}\n";

open(OUT, ">$opt_o") or
	die "Can't create '$opt_o' : $!";

print OUT	  "/* do not edit, an auto generated header file, source: ${source} */\n";
print OUT 	  "static struct fetcherr ${opt_t}_errlist[] = {\n";
foreach my $e (@errors) {
	print OUT "    { $e->{NUM}, FETCH_$e->{ENUM}, \"$e->{DESC}\" },\n";
}
print OUT 	  "};\n";
close OUT;


sub
Usage		# ([message])
{
	print "\nmksects @_\n\n" if (@_);
	print <<EOU;

Usage: perl mkerrors.pl [options] <source>

Options:
    -t <type>                Table prefix name (ftp or http).
    -o <file>                Output file.
    -h                       Command line usage; this message.

EOU
	exit 3;
}

