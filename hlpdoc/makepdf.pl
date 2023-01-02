#!/usr/bin/perl
# $Id: makepdf.pl,v 1.4 2022/12/09 15:56:48 cvsuser Exp $
# -*- tabs: 8; indent-width: 4; -*-
# pdf generation tool.
#
#
# Copyright (c) 1998 - 2023, Adam Young.
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
use warnings 'all';

use Cwd 'realpath', 'getcwd';
use Getopt::Long;
use File::Spec;
use File::Copy;                                 # copy()
use File::Basename;
use POSIX 'asctime';
use Data::Dumper;

my $x_binary    = '/usr/local/bin';

my $CWD         = getcwd();
my $o_version   = undef;
my $o_binary    = undef;
my $o_outdir    = '.';
my $o_wkdir     = 'doc';
my $o_srcdir    = 'src';
my $o_debug     = 0;

exit &main();

sub
main()
{
    if ($^O eq 'MSWin32') {                     # determine windows installation path
            if (-d 'C:/Program Files (x86)/wkhtmltopdf') {
                $x_binary = 'C:/Program Files (x86)/wkhtmltopdf';

            } elsif (-d 'C:/Program Files/wkhtmltopdf') {
                $x_binary = 'C:/Program Files/wkhtmltopdf';

            } else {
                $x_binary = $ENV{'ProgramFiles'}.'/wkhtmltopdf';
            }
            $x_binary .= '/bin';
    }

    my $o_clean = 0;
    my $o_help = 0;

    $o_binary = $x_binary;
    my $ret =
        GetOptions(
            'B|binary:s'    => \$o_binary,
            'W|wkdir:s'     => \$o_wkdir,
            'S|srcdir:s'    => \$o_srcdir,
            'D|outdir:s'    => \$o_outdir,
            'debug'         => \$o_debug,
            'help'          => \$o_help
            );

    Usage() if (!$ret || $o_help);

##  ParseMenu();

    #   [page]       Replaced by the number of the pages currently being printed
    #   [frompage]   Replaced by the number of the first page to be printed
    #   [topage]     Replaced by the number of the last page to be printed
    #   [webpage]    Replaced by the URL of the page being printed
    #   [section]    Replaced by the name of the current section
    #   [subsection] Replaced by the name of the current subsection
    #   [date]       Replaced by the current date in system local format
    #   [time]       Replaced by the current time in system local format
    #   [title]      Replaced by the title of the of the current page object
    #   [doctitle]   Replaced by the title of the output document
    #   [sitepage]   Replaced by the number of the page in the current site being converted
    #   [sitepages]  Replaced by the number of pages in the current site being converted

    my $cmd =
        "--javascript-delay 10000 ".            # 10 seconds
        "--header-center   \"[section]\" ".
        "--header-right    \"[page]\" ".
        "--header-line     ".
        "--header-spacing  5 ".
        "--footer-center   \"[page]\" ".
        "--footer-left     \"Grief Edit\" ".
        "--footer-spacing  5 ".
        "cover src/cover/CoverPage1.html ".
        "cover src/cover/CoverPage2.html ".
        "toc ".
                                                # " --outline-depth 3 ".
        "--xsl-style-sheet src/cover/makpdftoc.xsl ".
        "doc/html/files/introduction-txt.html ".
        "doc/html/files/history-txt.html ".
        "doc/html/files/copyright-txt.html ".
        "doc/html/files/contrib-txt.html ".
        "doc/html/files/quickstart-txt.html ".
        "doc/html/files/tutorial-txt.html ".
        "doc/html/files/language-txt.html ".
        "doc/html/files/macros-txt.html ".
        "doc/html/files/preprocessor-txt.html ".
        "doc/html/files/debugging-txt.html ".
        "doc/html/files/library-txt.html ".
        "doc/html/files/prim_arith-txt.html ".
        "doc/html/files/prim_buffer-txt.html ".
        "doc/html/files/prim_callback-txt.html ".
        "doc/html/files/prim_debug-txt.html ".
        "doc/html/files/prim_dialog-txt.html ".
        "doc/html/files/prim_env-txt.html ".
        "doc/html/files/prim_file-txt.html ".
        "doc/html/files/prim_kbd-txt.html ".
        "doc/html/files/prim_list-txt.html ".
        "doc/html/files/prim_macro-txt.html ".
        "doc/html/files/prim_misc-txt.html ".
        "doc/html/files/prim_movement-txt.html ".
        "doc/html/files/prim_proc-txt.html ".
        "doc/html/files/prim_scrap-txt.html ".
        "doc/html/files/prim_screen-txt.html ".
        "doc/html/files/prim_search-txt.html ".
        "doc/html/files/prim_spell-txt.html ".
        "doc/html/files/prim_string-txt.html ".
        "doc/html/files/prim_syntax-txt.html ".
        "doc/html/files/prim_var-txt.html ".
        "doc/html/files/prim_window-txt.html ".
        "doc/html/files/appendixa-txt.html ".
        "doc/html/files/appendixb-txt.html ".
        "doc/html/files/appendixc-txt.html ".
        "doc/html/files/appendixd-txt.html ".
        "src/cover/IndexPage.html ".
        "doc/html/index/General.html ".
        "doc/html/index/General2.html ".
        "doc/html/index/General3.html ".
        "doc/html/index/General4.html ".
        "doc/html/index/General5.html ".
        "doc/html/index/General6.html ".
        "doc/html/index/General7.html ".
        "doc/html/index/General8.html ".
        "doc/html/index/General9.html ".
        "doc/html/index/General10.html ".
        "doc/html/index/General11.html ".
        "doc/html/index/General12.html ".
        "doc/html/index/General13.html ".
        "${o_outdir}/griefprogguide.pdf";

    print "==> ${o_binary}/wkhtmltopdf ${cmd}\n";
    system($o_binary . '/wkhtmltopdf', $cmd);
    return 0;
}


sub
ParseMenu()             #()
{
    my $menu = "${o_wkdir}/Menu.txt";
    my $group = '';
    my @files;

    open(MENU, "<${menu}") or
        die "cannot open <${menu}> : $!\n";

    while (defined (my $line = <MENU>)) {
        next if ($line =~ /^\s*#$/);

        $line =~ s/^\s+//g;

        if ($line =~ /^Format: (.+)\s+$/) {
print "Format:    $1\n";

        } elsif ($line =~ /^Title: (.+)\s+$/) {
print "Title:     $1\n";

        } elsif ($line =~ /^Footer: (.+)\s+$/) {
print "Footer:    $1\n";

        } elsif ($line =~ /^Timestamp: (.+)\s+$/) {
print "Timestamp: $1\n";

        } elsif ($line =~ /^PageFooter: (.+)\s+$/) {
print "Footer:    $1\n";

        } elsif ($line =~ /^File: (.+) \((.+)\)\s+$/) {
print "File:      $1 ($2)\n";
            my ($desc, $file) = ($1, $2);
            my $out = "${o_wkdir}/html/files/$file";

            $out =~ s/\./-/;                    # convert dot
            $out .= '.html';                    # implied extension

            (-f $out) or die "missing ${out}\n";
            push @files, $out;

        } elsif ($line =~ /^Group: (.+?)\s*\{\s*$/) {
print "Group:     $1\n";
            $group = $1;

        } elsif ($group eq 'Index') {
            if ($line =~ /^Index: (.+)\s+$/) {
print "Index:     $1\n";
            }

        } elsif ($line =~ /^}\s+$/) {
print "Group:     }\n";
            $group = '';
        }
    }

    print "-----------------------------\n";
    foreach (@files) {
        print $_."\n";
    }
    print "-----------------------------\n";

    close(MENU);
    exit(0);
}


sub
Usage                   #(message)
{
    print "\nmakepdf @_\n\n" if (@_);
    print <<EOT;

Usage: perl makepdf.pl [options] <command>

Options:
    --help                  Help.
    -B,--binary <path>      wkhtmltopdf path (default $x_binary)
    -D,--outdir <path>      Output directory (default '.').
    -S,--srcdir <path>      Source directory (default '.').

EOT
    exit(42);
}


sub
Copyright               #(file)
{
#   Grief Programmer's Guide
#
#   Copyright © 1998 - 2018 Adam Young
#
#   Permission is granted to make and distribute non-commercial
#   verbatim copies of this documentation provided the copyright
#   notice and this permission notice are preserved on all copies.
#
#   Permission is granted to copy and distribute modified
#   versions of this documentation under the conditions for
#   verbatim copying, provided that the entire resulting derived
#   work is distributed under the terms of a permission notice
#   identical to this one.
#
#   Permission is granted to copy and distribute translations of
#   this documentation into another language, under the above
#   conditions for modified versions, except that this permission
#   notice may be stated in a translation approved by the author.
}


sub
Debug
{
    if ($o_debug) {
        print "@_\n";
    }
}

#end

