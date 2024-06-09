#!/usr/bin/perl -w
# -*- mode: perl; -*-
# $Id: guesstest.pl,v 1.1 2024/05/28 08:35:27 cvsuser Exp $
# libguess test front-end
#

use strict;
use warnings 'all';

my $mode;
my $task;

foreach (@ARGV) {
	if (/^--mode=(.*)$/) {
		$mode = $1;
		die "guesstest: unexpected mode <$mode>, either 'run' or 'gen'.\n"
			if ($mode ne 'run' && $mode ne 'gen');

	} elsif (/^--task=(.*)$/) {
		$task = $1;
		$task =~ s/\//\\/g;
		die "guesstest: task <$task> not found.\n"
			if (! -f "${task}.exe");

	} elsif (/^-/) {
		die "guesstest: unknown argument <$_>\n";

	} else {
		die "guesstest: missing --mode\n"
			if (! $mode);
		die "guesstest: missing --task\n"
			if (! $task);
	}
}

foreach my $test (@ARGV) {
	next if ($test =~ /^-/);

	if ($mode eq "run") {
		printf "test: ${test}\n";
		system("$task.exe $test");

	} elsif ($mode eq "gen") {
	}
}

1;

