#!/usr/bin/perl -w
# -*- mode: perl; -*-
# $Id: iconvtest.pl,v 1.1 2024/06/04 13:15:59 cvsuser Exp $
# iconv tests
#

use strict;
use warnings 'all';
use Cwd 'realpath', 'getcwd';

my $task;
my $data;

foreach (@ARGV) {
	if (/^--task=(.*)$/) {
		$task = $1;
		$task =~ s/\//\\/g;
		die "iconvtest: task <$task> not found.\n"
			if (! -f "${task}.exe");

	} elsif (/^--data=(.*)$/) {
		$data = $1;
		die "iconvtest: data directory <$data> not found.\n"
			if (! -d $data);

	} elsif (/^-/) {
		die "iconvtest: unknown option <$_>\n";

	} else {
		die "iconvtest: unknown argument <$_>\n";
	}
}

die "iconvtest: missing --task\n"
	if (! $task);

die "iconvtest: missing --data directory\n"
	if (! $data);

$data = realpath($data);
open(my $in, "<", "${data}/TESTS") or
	die "cannot open input <${data}/TESTS>: $!";

while (my $line = <$in>) {
	chomp ($line);
	$line =~ s/^\s+//;
	next if (!$line || $line =~ /^#/);      # empty/comment

	# <input> <output> <Y/N> <target-code> ...
	my ($from, $to, $yn, $codes) = split(/\s+/, $line, 4);

	printf "${from},${to},${yn},${codes}\n";

	for my $code (split(/\s+/, $codes)) {
		printf "\t => ${code}\n";

		if (-f "${data}/${from}") {
			my $cmd = "${task} -f ${from} -t ${code} ${data}/${from} >${data}/tmp/${from}_to_${code}";
			print "cmd: ${cmd}\n";
			system($cmd);
		}
	}
}

1;

