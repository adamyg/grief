#!/usr/bin/perl -w
# -*- mode: perl; -*-
# $Id: cssimport.pl,v 1.1 2024/04/19 18:11:30 cvsuser Exp $
# html css import
#

use strict;
use warnings 'all';

my $src = undef;
my $dst = undef;

sub styleimport		# (css)
{
	my $css = shift;
	open (STYLE, $css) or
		die "can't open stylesheet <${css}>: $!";
	my $style = '';
	while (<STYLE>) {
		$style .= $_;
	}
	close STYLE;
	return $style;
}

sub generate		# ()
{
	# import
	open (INPUT, $src) or
		die "can't open style sheet <${src}>: $!";
	my $output = '';
	while (<INPUT>) {
		if (m{\<link ([^\\]+)/\>}) {
			my $line = $_;
			my $link = $1;

			if ($link =~ m{rel="stylesheet"} && $link =~ m{type="text/css"} && $link =~ m{href="([^\"]+)"}) {
				my $css = $1;
				my $style = styleimport($css);

				print "importing stylesheet=<$css>\n";
				$line =~ s{\<link [^\\]+/\>}{<style>${style}</style>};
				$output .= $line;
				next;
			}
		}
		$output .= $_;
	}
	close(INPUT) or
		die "can't close <${src}>: $!";

	# export
	open (OUTPUT, '>', $dst) or
		die "can't create <${dst}>: $!";
	print OUTPUT $output;
	close(OUTPUT) or
		die "can't close <${dst}>: $!";
}

sub usage		# ()
{
	die "usage: <src> [<dst>]\n\n";
}

####

while (scalar @ARGV) {
	$_ = shift @ARGV;

	if (/^-/) {			# unknown option
		usage();
	} else {			# arguments
		$src = $_;
		$dst = shift @ARGV;
		$dst = $src if (! defined $dst);
		last;
	}
}

usage() if (scalar @ARGV or !$src or $src !~ /\.html$/ or !$dst);
generate();

#end

