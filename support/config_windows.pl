#!/usr/bin/perl -w
# -*- mode: perl; -*-
# $Id: config_windows.pl,v 1.3 2024/05/26 15:41:12 cvsuser Exp $
# Configure front-end for native windows targets.
#

use strict;
use warnings 'all';

use Cwd;
use File::Which qw(which where);

##  resolve binutils

my $localbinutil = 0;
my $mingw = undef;

foreach (@ARGV) {
	if (/^--local-binutil$/) {
		print "config_windows: utilising local binutils\n";
		$localbinutil = 1;
	} elsif (/^--msys=(.*)/) {
		print "config_windows: using mingw=${mingw}\n";
		$mingw = "$1/usr/bin";
	}
}

sub
Resolve 		# (default, apps ...)
{
	my $default = shift;
	return $default
		if ($localbinutil);

	if (! defined $mingw) {
		my $gcc = which("gcc");
		if ($gcc) {
			$gcc =~ s/\.exe$//i;
			$gcc =~ s/\\/\//g;

			$mingw = "";

			if ($gcc =~ /^(.*)\/mingw64\/bin\/gcc$/i) {
				$mingw = "$1/usr/bin"
					if (-d "$1/usr/bin");

			} elsif ($gcc =~ /^(.*)\/mingw64\/bin\/gcc$/i) {
				$mingw = "$1/usr/bin"
					if (-d "$1/usr/bin");
			}

			print "config_windows: detected mingw=${mingw}\n"
				if ($mingw);
		}
	}

	foreach my $app (@_) {

		my $mingw_resolved = undef;
		$mingw_resolved = "${mingw}/${app}"
			if ($mingw && -f "${mingw}/${app}.exe");

		# PATH
		my $resolved = which $app;
		if ($resolved) {
			$resolved =~ s/\.exe$//i;
			$resolved =~ s/\\/\//g;

			if ($mingw_resolved && ($resolved ne $mingw_resolved)) {
				print "config_windows: <${resolved}> and <${mingw_resolved}> found; using Mingw64 instance.\n";
				$resolved = $mingw_resolved;
			}
			return $resolved;

		# Mingw64 package
		} elsif ($mingw_resolved) {
			return $mingw_resolved;

		# chocolatey import
		} elsif (-d "C:/ProgramData/chocolatey/bin") {
			$resolved = "C:/ProgramData/chocolatey/bin/${app}";
			return $resolved
				if (-f "${resolved}.exe");
		}
	}
	return $default;
}

sub
ResolveCoreUtils	# ()
{
	my @paths = (
	    "",                                 # PATH
	    "/devl/gnuwin32",                   # local gnuwin32
	    "C:/Program Files/Git/usr",         # Git for Windows
	    "c:/msys64/usr",                    # MSYS installation
	    "c:/GnuWin32",                      # https://sourceforge.net/projects/getgnuwin32/files (legacy)
	    "C:/Program Files (x86)/GnuWin32",  # choco install gnuwin32-coreutils.install (legacy)
	    );
	my @cmds = ("mkdir", "rmdir", "cp", "mv", "rm", "egrep");

	foreach my $path (@paths) {
		my $success = 1;
		if (! $path) {
			foreach my $app (@cmds) {
				if (! which($app) ) {
					$success = 0;
					goto LAST;
				}
			}
			if ($success) {
				print "config_windows: CoreUtils=PATH\n";
				return "";
			}
		} else {
			my $bin = "${path}/bin";
			if (-d $bin) {
				foreach my $app (@cmds) {
					if (! -f "${bin}/${app}") {
						$success = 0;
						goto LAST;
					}
				}
			}
			if ($success) {
				print "config_windows: CoreUtils=${path}\n";
				return $path;
			}
		}
	}
	die "config_windows: unable to determine coreutils\n";
}

my $busybox	= Resolve('./win32/busybox', 'busybox');
my $wget	= Resolve('./win32/wget', 'wget');
my $bison	= Resolve('$(D_BIN)/byacc', 'bison', 'yacc');
my $flex	= Resolve('$(D_BIN)/flex', 'flex');
my $coreutils	= undef;

##  build command line

my $cwd = getcwd;
die "config_windows: spaces within work directory <$cwd>; rename before proceeding.\n" 
	if ($cwd =~ / /);

die "config_windows: gzip missing, please install within \$PATH before proceeding.\n" 
	if (!which("gzip"));

my @options;
my $target = undef;

my $script  = shift @ARGV;
foreach (@ARGV) {
	if (/^--busybox=(.*)$/) {
		$busybox = $1;
	} elsif (/^--binpath=(.*)$/) {
		$coreutils = $1;
	} elsif (/^--wget=(.*)$/) {
		$wget = $1;
	} elsif (/^--bison=(.*)$/) {
		$bison = $1;
	} elsif (/^--flex=(.*)$/) {
		$flex = $1;
	} elsif (/^--local-binutil$/) {
		# consume
	} elsif (/^--msys=/) {
		# consume
	} else {
		if (/^--/) {
			if (/^--(.*)=(.*)$/) {
				push @options, "--$1=\"$2\"";
			} else {
				push @options, $_;
			}
		} else {
			die "config_windows: multiple targets, <$_> unexpected\n"
				if ($target);
			$target = $_;
		}
	}
}

$coreutils = ResolveCoreUtils()
	if (! $coreutils);
if ($coreutils) {
	if ($coreutils =~ / /) { # spaces, symlink
		print "config_windows: CoreUtils: ./CoreUtils => ${coreutils} (symlink)\n";
		system "mklink /J CoreUtils \"${coreutils}\""
			if (! -d "CoreUtils/bin");
		push @options, "--binpath=./CoreUtils/bin";
	} else {
		push @options, "--binpath=${coreutils}/bin";
	}
}

die "config_windows: target missing\n" 
	if (! $target);

print "\n$^X ${script}\n => --busybox=\"${busybox}\" --wget=\"${wget}\" --flex=\"${flex}\" --bison=\"${bison}\" @options ${target}\n\n";

system "$^X ${script} --busybox=\"${busybox}\" --wget=\"${wget}\" --flex=\"${flex}\" --bison=\"${bison}\" @options ${target}";

#end
