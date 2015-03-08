#!/usr/bin/perl
# $Id: makehelp.pl,v 1.15 2015/03/01 23:45:33 cvsuser Exp $
# -*- tabs: 8; indent-width: 4; -*-
# Help collection tool.
#
#
# Copyright (c) 1998 - 2015, Adam Young.
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
use File::Copy;
use File::Basename;
use File::Path qw(make_path remove_tree);
use POSIX 'asctime', 'strftime';
use Data::Dumper;

use Prototype;

my $CWD         = getcwd();
my $o_version   = undef;
my $o_ndbin     = 'http://sourceforge.net/projects/ndplus/files/nd+_beta8';
my $o_ndwk      = './doc';
my $o_srcdir    = './src';
my $o_prmdir    = '../gr';
my $o_manbin    = '../bin/grmandoc';
my $x_manopt    = '';                           # -O indent=3,width=80'
my $o_hlpdir    = '../help';
my $o_index     = undef;
my @o_features  = ();
my $o_verbose   = 0;
my $o_warning   = 0;
my $o_debug     = 0;

my %x_topics    = (     # Macro topics, see: keywd.c
    arith               => 'Arithmetic Operators',
    buffer              => 'Buffer Primitives',
    comms               => 'Communications',
    debug               => 'Debugging Primitives',
    dialog              => 'Dialog Primitives',
    env                 => 'Environment Primitives',
    file                => 'File Primitives',
    float               => 'Floating Point Primitives',
    kbd                 => 'Keyboard Primitives',
    list                => 'List Primitives',
    macro               => 'Macro Language Primitives',
    misc                => 'Miscellaneous Primitives',
    movement            => 'Movement Primitives',
    proc                => 'Process Management Primitives',
    scrap               => 'Scrap Primitives',
    screen              => 'Screen Primitives',
    spell               => 'Spell Checker Primitives',
    search              => 'Search and Translate Primitives',
    string              => 'String Primitives',
    syntax              => 'Syntax Highlighting Primitives',
    var                 => 'Variable Declaration Primitives',
    window              => 'Window Primitives',
    callback            => 'Callbacks'
    );

my @x_progguide  = (    # Programmers Guide
    "introduction",
    "copyright",
    "history",
    "macros",
    "tutorial",
    "language",
    "preprocessor",
    "library",
    "contrib",
    "debugging"
    );

my @x_userguide  = (    # Users Guide
    "quickstart"
    );

my @x_configguide = (   # Confiuration Guide
    "configuration"
    );

my %x_primitives = (    # Predefined primitives
    #internal
    '_default'          => 1,
    '_extension'        => 1,
    '_init'             => 1,
    '_invalid_key'      => 1,
    '_prompt_begin'     => 1,
    '_prompt_end'       => 1,
    'main'              => 1,

    #crunch only
    'foreach'           => 2,
    'case'              => 2,
    'else'              => 2,

    #dos
    'del'               => 3,
    'dos'               => 3,

    #macros
    'redo'              => 4,
    'set'               => 4
    );

my %x_references = (    # Predefined references
    callbacks => 1,
    array => 1,
    xxx => 1
    );
my %x_features;
my %x_sections;
my %x_definitions;

sub BuildSrc();
sub BuildHTML();
sub BuildMANDOC();
sub BuildPrim();

sub Debug;
exit &main();


sub
main()                  #()
{
    my $o_help = 0;

    my $ret = GetOptions(
        'H|hlpdir:s'    => \$o_hlpdir,
        'S|srcdir:s'    => \$o_srcdir,
        'P|prmdir:s'    => \$o_prmdir,
        'N|ndbin:s'     => \$o_ndbin,
        'M|manbins:s'   => \$o_manbin,
        'I|index:s'     => \$o_index,
        'F|feature:s'   => \@o_features,
        'verbose'       => \$o_verbose,
        'warnings'      => \$o_warning,
        'debug'         => \$o_debug,
        'help'          => \$o_help
        );

    Usage() if (!$ret || $o_help);
    Usage('expected command') if (scalar @ARGV < 1);
    my $cmd = $ARGV[0];

    if ($cmd eq 'hlp') {
        Usage("hlp: missing arguments") if (scalar @ARGV != 3);
        MakeHlp($ARGV[1], $ARGV[2]);

    } else {
        Usage("unexpected arguments $ARGV[1] ...")
            if (scalar @ARGV > 1);

        NDPLUSDownload();
        if ($cmd eq 'prog') {
            MakeProg();

        } elsif ($cmd eq 'user') {
            MakeUser();

        } elsif ($cmd eq 'mdoc') {
            ParseSections("keywd.c");
            BuildSRC();
            BuildMANDOC();

        } elsif ($cmd eq 'html') {
            ParseSections("keywd.c");
            BuildSRC();
            BuildHTML();

        } elsif ($cmd eq 'prim') {
            ParseSections("keywd.c");
            BuildSRC();
            BuildMANDOC();
            BuildPrimHLP();
            ExportPrimTOC();
            ExportSections();

        } else {
            Usage("unknown command '$cmd'");
            return 1;
        }
    }
    return 0;
}


sub
Usage                   #([message])
{
    print "\nmakehelp @_\n" if (@_);
    print <<EOT;

Usage: perl makehelp.pl [options] <command>

Options:
    -H,--hlpdir             Help output directory ('../help').
    -S,--srcdir <dir>       Source directory (default './src').
    -I,--index <dir>        Index source file ('./src/library.txt.src').
    -P,--prmdir <dir>       Primitive source directory (default '../gr').
    -N,--ndbin <dir>        NaturalDoc binary path.
    -M,--manbin <path>      Mandoc binary path.
    -F,--feature <file>     Features directory (one or more).

    --debug                 Enable runtime diagnostics.
    --help                  Help.

Commands:
    hlp                     Build help.
    prim                    Primitive generation help.
    mandoc                  MANDOC.
    html

EOT
    exit(42);
}


sub
NDPLUSDownload()
{
    my $ndsrc = 'http://sourceforge.net/projects/ndplus/files/latest/download';
    my $nddir = 'ndplus';
    my $ndbin = './ndplus/bin/NaturalDocs';

    if ($o_ndbin =~ /^http/) {

        if (! -f $ndbin) {

            (-d $nddir || mkdir($nddir, 0777)) or
                die "hlp: cannot create directory <${nddir}> : $!\n";

            if ($^O eq 'MSWin32') {             # ActivePerl/Win32
                my $name = 'ndplus.zip';

                if (! -f "$nddir/$name") {
                    system("wget -O $nddir/$name ${o_ndbin}.zip");
                    system("wget -O $nddir/$name ${ndsrc}")
                        if (! -f "$nddir/$name");
                }
                chdir($nddir);
                system("unzip $name");
                chdir('..');

            } else {                            # others
                my $name = 'ndplus.tgz';

                if (! -f "$nddir/$name") {
                    system("wget -O $nddir/$name ${o_ndbin}.tgz");
                    system("wget -O $nddir/$name ${ndsrc}")
                        if (! -f "$nddir/$name");
                }
                chdir($nddir);
                system("gzip -d -c $name | tar -xvf -");
                chdir('..');

            }
        }

        $o_ndbin = $ndbin;
    }
}


sub
MakeHlp($$)             #(source, output)
{
    my ($source, $output) = @_;

    Info("Generating (${output})");

    die "hlp: source extension not 'mandoc'\n"
        if ($source !~ /\.mandoc$/);

    die "hlp: output extension not 'hlp'\n"
        if ($output !~ /\.hlp$/);

    die "hlp: cannot access <${source}> : $!\n"
        if (! -f "${source}");

    Verbose("exec: ${o_manbin} -Tascii ${x_manopt} ${source} |");
    open(TXT, "${o_manbin} -Tascii ${x_manopt} ${source} |") or
        die "cannot execute <${o_manbin} -Tascii ${x_manopt} ${source}> : $!\n";

    if (defined (my $txt = <TXT>)) {

        open(HLP, ">${output}") or
            die "cannot create <${output}}> : $!\n";

        print HLP $txt;
        while (defined ($txt = <TXT>)) {
            print HLP $txt;
        }

        close (HLP);
    }
    close (TXT);
}


sub
MakeProg()              #()
{
    ExportNINFO('prog', \@x_progguide, "Programmers Guide");
}


sub
MakeUser()              #()
{
    ExportNINFO('user', \@x_userguide, "Users Guide");
}


sub
ExportNINFO($$)         #(image, filesRef, [title])
{
    my ($image, $filesRef, $title) = @_;

    my $mandoc = "${o_ndwk}/mandoc";
    my $ninfo  = "${o_hlpdir}/${image}.cr";
    my $manidx = "${o_hlpdir}/${image}.idx";

    my $asctime = asctime(localtime());
    chop($asctime);

    Info("Generating NINFO documentation <$title>");

    my @files;
    my @hlpidx;

    # parse help source
    foreach my $file (@$filesRef) {
        my $source = $mandoc . '/' . $file . '-txt.mandoc';
        my $hlpnam = "${file}.hlp";
        my $hlpdoc = "${o_hlpdir}/${file}.hlp";

        Verbose("exec: ${o_manbin} -Tascii ${x_manopt} ${source} |");
        open(TXT, "${o_manbin} -Tascii ${x_manopt} ${source} |") or
            die "cannot execute <${o_manbin} -Tascii ${x_manopt} ${source}> : $!\n";

        open(HLP, ">${hlpdoc}") or
            die "cannot create <${hlpdoc}}> : $!\n";

        if (defined (my $txt = <TXT>)) {
            my ($sections, $topics, $headers);
            my $line = 0;

            print HLP $txt;
            ++$line;

            while (defined ($txt = <TXT>)) {
                print HLP $txt;
                ++$line;

                $txt =~ s/.\x8//g;              # consume backspaces

                if (! defined $topics) {        # opening section
                    if ($txt =~ /^NAME$/ && defined ($txt = <TXT>)) {
                        push @files, {
                            title       => Trim($txt),
                            fname       => $hlpnam,
                            line        => $line,
                            sections    => [],
                            topics      => []
                            };
                        $sections = $files[-1]->{sections};
                        $topics = $files[-1]->{topics};
                    }

                } else {                        # section
                    if ($txt =~ /^\s*\[\[ (.*) \]\]$/) {
                        push @$sections, {
                            title       => Trim($1),
                            fname       => $hlpnam,
                            line        => $line,
                            topics      => []
                            };
                        $topics = $$sections[-1]->{topics};
                        $headers = undef;

                                                # topic
                    } elsif ($txt =~ /^([A-Za-z][A-Za-z0-9 ]+): (.*)$/) {
                        push @$topics, {
                            topic       => $1,
                            title       => Trim($2),
                            fname       => $hlpnam,
                            line        => $line,
                            headers     => []
                            };
                        $headers = $$topics[-1]->{headers};

                        print HLP "\n";
                        ++$line;
                                                # headings
                    } elsif ($txt =~ /^   ([a-z][a-z0-9_ ]+):$/i) {
                        push @$headers, {
                            title       => Trim($1),
                            fname       => $hlpnam,
                            line        => $line
                            };
                    }
                }
            }
            close (HLP);
        }
        close (TXT);
    }

    die "hlp: no help source found\n"
        if (! scalar @files);

    push @hlpidx, "$title: $files[0]->{fname} 1"
        if ($title);

    # export ninfo definition
    open(NINFO, ">${ninfo}") or
        die "cannot create <${ninfo}}> : $!\n";

    print NINFO
        "/* -*- mode: cr; -*- \n" .
        " * Generated by makehelp ($asctime)\n" .
        ' * $'.'Id: $'. "\n" .
        " */\n" .
        "\n" .
        "static list\n" .
        "ninfo_index = {\n";

        my $oneshot = (1 == scalar @files ? 1 : 0);

        print NINFO "\t\"[[ $title ]]\", \"$files[0]->{fname}\", 1, NULL,\n"
            if (! $oneshot);

        foreach my $file (@files) {
            my $sections = $file->{sections};
            my $depth = 1;

            if (! $oneshot) {
                print NINFO "\n";

                if (scalar @$sections || scalar @{$file->{topics}}) {
                    print NINFO "\t\"$file->{title}\", \"$file->{fname}\", 1, {\n";
                    ++$depth;
                } else {
                    print NINFO "\t\"$file->{title}\", \"$file->{fname}\", $file->{line}, NULL,\n";
                }
            }

            if (scalar @$sections) {
                print NINFO ("\t" x $depth) . "\"[[ $file->{title} ]]\", \"$file->{fname}\", $file->{line}, NULL,\n";
                NINFOPush($file, \@hlpidx);

                foreach my $section (@$sections) {
                    my $topics = $section->{topics};

                    if (scalar @$topics) {
                        print NINFO "\n". ("\t" x $depth++) . "\"$section->{title}\", \"$section->{fname}\", 1, {\n";
                        NINFOTopics($section, $depth, \*NINFO, \@hlpidx);
                        print NINFO ("\t" x $depth--) . "},\n";

                    } else {
                        print NINFO ("\t" x $depth) . "\"$section->{title}\", \"$section->{fname}\", $section->{line}, NULL\n";
                    }
                }

            } else {
                NINFOTopics($file, $depth, \*NINFO, \@hlpidx);
            }

            if ($depth > 1) {
                print NINFO "\t\t},\n";
            }
        }

    print NINFO
        "\t};\n" .
        "\n" .
        "list\n" .
        "${image}_ninfoindex()\n" .
        "{\n" .
        "   return ninfo_index;\n" .
        "}\n" .
        "/*end*/\n";

    # export index
    open(IDX, ">${manidx}") or
        die "cannot create <${manidx}> : $!\n";
    foreach (sort @hlpidx) {
        print IDX "$_\n";
    }
    close(IDX);
}


sub
NINFOTopics($$$$)       #(topic, depth, NINFO, $hlpidx)
{
    my ($topic, $depth, $NINFO, $hlpidx) = @_;
    my $subtopics = $topic->{topics};

    print $NINFO ("\t" x $depth) . "\"[[ $topic->{title} ]]\", \"$topic->{fname}\", $topic->{line}, NULL,\n"
        if (scalar @$subtopics > 1);
    NINFOPush($topic, $hlpidx);

    foreach my $subtopic (@$subtopics) {

        print $NINFO ("\t" x $depth) . "\"$subtopic->{title}\", \"$subtopic->{fname}\", $subtopic->{line}, NULL,\n";
        NINFOPush($subtopic, $hlpidx);

        my $headers = $subtopic->{headers};
        foreach my $header (@$headers) {
            print $NINFO ("\t" x $depth) . "\t// $header->{title}\n";
        }
    }
}


sub
NINFOPush($$)           #(topic, $hlpidx)
{
    my ($topic, $hlpidx) = @_;

    push @$hlpidx, "$topic->{title}: $topic->{fname} $topic->{line} 0";
}


sub
ParseSections($)        #(filename)
{
    my ($filename) = @_;

    Info("Importing macro names (${filename})");

    open(SRC, "<${o_prmdir}/${filename}") or
        die "cannot open <${o_prmdir}/$filename> : $!\n";

    while (defined (my $line = <SRC>)) {
        if ($line =~ /^\s*{.*\"([^\"]+)\".*MACRO\(.*\/\*\s*(.+)\s*\*\//) {
            my $name  = $1;
            my @sects = split(/[ ,]+/, $2);
            my $count = 0;

            $x_primitives{$name} = 0;
            foreach my $section (@sects) {      # macro section(s)
                PushSection($section, $name, (0 == $count++));
            }
        }
    }
    close(SRC);
}


sub
PushSection($$$)        #(section, name, primary)
{
    my ($section, $name, $primary) = @_;

    $section = lc($section);

    if (!exists $x_sections{$section}) {
        $x_sections{$section} = {
                TYPES   => {},
                NAMES   => []
            };
    }

    push @{$x_sections{$section}->{NAMESALL}}, $name;
    push @{$x_sections{$section}->{NAMES}}, $name
        if ($primary);

    if (!exists $x_definitions{$name}) {
        $x_definitions{$name} = {
                SECTION => $section,
                TYPE    => '',
                DEST    => undef,
                BODY    => undef,
                REFS    => undef
            };
    } else {
        $x_definitions{$name}->{SECTION} = $section;
    }
}


sub
ExportSections()        #()
{
    foreach my $section (keys %x_sections) {

        my $sect = "${o_hlpdir}/${section}.sec";
        open(SEC, ">$sect") or
            die "cannot create <${sect}> : $!";

        foreach my $name (sort @{$x_sections{$section}->{NAMESALL}}) {
            print SEC "${name}\n";
        }

        close(SEC);
    }
}


sub
BuildSRC()              #()
{
    Info("Parsing source documentation (${o_prmdir})");

    $o_prmdir = $CWD
        if ($o_prmdir eq '.');

    foreach (@o_features) {
        ParseFeatures($_);
    }

    ParseTopics();

    opendir(DIR, $o_prmdir) or
        die "error opening dir <${o_prmdir}> : $!\n";
    my @files =                                 # .h, .hpp, .c and .cpp
        grep{ !/\.xh$/ && /^.*\.[chp]$/ } readdir(DIR);
    close DIR;

    foreach (@files) {
        next if (! -f "${o_prmdir}/$_");
        ParseSRCFile($_);
    }

    Info("Exporting source documentation (${o_srcdir})");
    ExportSRC();
}


sub
BuildMANDOC()           #()
{
    my $mandoc = $o_ndwk.'/mandoc';

    Info("Generating MANDOC documentation ($mandoc)");
    make_path($mandoc);
    system("perl ${o_ndbin} -t 8 -i ${o_srcdir} -o ManDoc ${mandoc} -r -p ${o_ndwk} --documented-only");
}


sub
BuildHTML()             #()
{
    my $htmldoc = $o_ndwk.'/html';

    Info("Generating HTML documentation ($htmldoc)");
    make_path($htmldoc);
    system("perl ${o_ndbin} -t 8 -i ${o_srcdir} -o FramedHTML ${htmldoc} -r -p ${o_ndwk} -s Default Extra hlpsrc2 --documented-only");
}


sub
BuildPrimHLP()          #()
{
    my $timestamp = strftime("%B %d, %Y", localtime());
    my $mandoc = $o_ndwk.'/mandoc';
    my $manidx = $o_hlpdir.'/prim.idx';

    Info("Generating HELP documentation");

    sub
    SsSh
    {
        return '.Sh '.uc(shift);
    }

    opendir(DIR, $mandoc) or
        die "error opening dir <${mandoc}> : $!\n";
    my @mandocs =
        grep{ /^prim.*\.mandoc$/ } readdir(DIR);
    close DIR;

    my $tmpdoc = "${o_hlpdir}/.prim.mandoc";    # temporary working document
    my $hlpseq = 0;
    my @hlpidx;

    # parse and build prim001.hlp ... primxxx.hlp
    foreach my $file (@mandocs) {

        my ($type, $name, $desc, $prot);
        my $mode = 0;

        Info("Parsing $file");

        my $hlpnam =                            # prim001.hlp ...
            sprintf("prim%03d.hlp", ++$hlpseq);
        my $hlpdoc =
            "${o_hlpdir}/${hlpnam}";
        my $hlpcnt = 0;

        open(HLP, ">${hlpdoc}") or
            die "cannot create <${hlpdoc}> : $!\n";

        open(SRC, "<${mandoc}/${file}") or
            die "cannot open <${mandoc}/${file}> : $!\n";

        while (defined (my $line = <SRC>)) {
            if (! $type) {
                if ($line =~ /^\.\\\" START:type=(macro|function|constant|variable)\tanchor=([^\t]+)\ttitle=([^\t]+)\tsummary=([^\t]+)/) {
                    $type = $1;
                    $name = $3;
                    $desc = Trim($4);
                    $mode = ('Constant' eq $type ? 2 : 1);

                    open(OUT, ">${tmpdoc}") or
                        die "cannot create <${tmpdoc}> : $!\n";

                    print OUT
                        '.\"' . "\n" .
                        '.\" Generated by ND+ and makehelp.pl' . "\n" .
                        '.\" $'.'Id: $'. "\n" .
                        '.\"' . "\n" .
                        '.Dd ' . $timestamp . "\n" .
                        '.Dt "Primitives" 2' . "\n" .
                        '.Os' . "\n";
                }

            } elsif (1 == $mode) {
                if ($line =~ /^\.\\\" START:prototype=([^\t]+)/) {
                    $prot = Trim($1);

                } elsif ($line =~ /^\.\\\" END:prototype/) {
                    my $prototype = Prototype->ParsePrototype($prot);

                    print OUT
                        '.Sh NAME' . "\n" .
                        '.Nm ' . $name . "\n" .
                        '.Nd ' . $desc . "\n" .
                        '.Sh SYNOPSIS' . "\n" .
                        $prototype->Print() . "\n";
                    $mode = 2;

                } elsif ($line =~ /^\.\\\" START-END:prototype/) {
                    print OUT                   # no prototype available
                        '.Sh NAME' . "\n" .
                        '.Nm ' . $name . "\n" .
                        '.Nd ' . $desc . "\n";
                    $mode = 2;
                }

            } elsif (2 == $mode) {
                if ($line =~ /^\.Ss (.*):/) {
                    print OUT SsSh($1) . "\n";
                    $mode = 3;
                }

            } elsif ($line !~ /^\.\\\" END:type=/) {
                $line =~ s/^\.Ss (.*):/SsSh($1)/ge;
                $line =~ s/^\.Ss (.*),/.Ss $1:/g;
                print OUT $line;

            } else {
                $type = undef;
                close(OUT);

                Verbose("exec: ${o_manbin} -Tascii ${x_manopt} ${tmpdoc} |");
                open(TXT, "${o_manbin} -Tascii ${x_manopt} ${tmpdoc} |") or
                    die "cannot execute <${o_manbin} -Tascii ${x_manopt} ${tmpdoc}> : $!\n";

                if (defined (my $txt = <TXT>)) {
                    print HLP $txt;

                    my $s = ++$hlpcnt;
                    while (defined ($txt = <TXT>)) {
                        print HLP $txt;
                        ++$hlpcnt;
                    }
                    my $e = ($hlpcnt - 1);

                    push @hlpidx, "${name}: ${hlpnam} $s $e";
                    }

                unlink ${tmpdoc};
                close (TXT);
            }
        }

        close(SRC);
        close(HLP);
    }

    # export index
    open(IDX, ">${manidx}") or
        die "cannot create <${manidx}> : $!\n";
    foreach (sort @hlpidx) {
        print IDX "$_\n";
    }
    close(IDX);
}


sub
ParseFeatures($)        #(featuredir)
{
    my ($featuredir) = @_;
    my %titles;

##  $o_warning = 1;
    Info("Parsing ${featuredir}/INDEX");

    # import index
    open(SRC, "<${featuredir}/INDEX") or
        die "cannot open <${featuredir}/INDEX> : $!\n";
    while (defined (my $line = <SRC>)) {
        if ($line =~ /^(.*):(.*)$/) {
            $titles{lc(Trim($1))} = 1;
            my @sects = split(/[ ,]+/, $2);
            foreach my $feat (@sects) {
                $feat = $1                      # alias=xxx
                    if ($feat =~ /^.*=(.*)$/);
                $feat = lc($feat);
                $x_features{$feat} = 1;

                Warning("feature: ${feat}.mandoc missing")
                    if (! -f "${featuredir}/${feat}.mandoc");
            }
        }
    }
    close(SRC);

    # check against features
    if ($featuredir =~ 'features$' && $o_warning) {
        my $featurescr = '../macsrc/feature.cr';
        my $state = 0;

        print "checking <$featurescr>\n";
        open(SRC, "<$featurescr") or
            die "cannot open <$featurescr> : $!\n";
        while (defined (my $line = <SRC>)) {
            if (!$state) {
                $state = 1
                    if ($line =~ /\<\<START-INDEX\>\>/);

            } elsif ($state) {
                last if ($line =~ /\<\<END-INDEX\>\>/);
                if ($line =~ /\"([^\"]+)\"/) {
                    my $item = lc($1);

                    if (exists $titles{$item}) {
                        delete $titles{$item};
                    } else {
                        Warning("feature: '${item}' missing");
                    }
                }
            }
        }
        close(SRC);
    }
}


sub
ParseTopics()           #()
{
    opendir(DIR, $o_srcdir) or
        die "error opening dir <${o_srcdir}> : $!\n";
    my @files =
        grep{ /\.txt$/ && !/^prim_/ } readdir(DIR);
    close DIR;

    foreach my $file (@files) {
        open(SRC, "<${o_srcdir}/$file") or
            die "cannot open <$o_srcdir/$file> : $!\n";

        while (defined (my $line = <SRC>)) {
            if ($line =~ /^\s*(Section|Topic|Title):\s+(.*)\s*$/) {
                $x_references{lc($2)} = uc($1); # Section: xxx, Topic: xxx or Title: xxx
            }
        }

        close(SRC);
    }
}


sub
PushDefinition($$$$$;$) #(name, type, desc, body, refs, opts)
{
    my ($name, $type, $desc, $body, $refs, $opts) = @_;

    if (!exists $x_definitions{$name}) {        # internal macro
        PushSection('general', $name, 1);       # XXX misc - 'Miscellaneous Primitives'
    }

    my $section = $x_definitions{$name}->{SECTION};
    push @{$x_sections{$section}->{"BODIES:$type"}}, $name;
    $x_sections{$section}->{TYPES}->{$type} = 1;

    if ($type =~ /^Constant/i) {
        $x_definitions{$name}->{TYPE} = 'const';
    } elsif ($type =~ /^Enum/i) {
        $x_definitions{$name}->{TYPE} = 'enum';
    } else {
        $x_definitions{$name}->{TYPE} = '';
    }
    $x_definitions{$name}->{DESC} = $desc;
    $x_definitions{$name}->{BODY} = $body;
    $x_definitions{$name}->{REFS} = $refs;
    $x_definitions{$name}->{OPTS} = $opts;
}


sub
ExportSRC()             #()
{
    # index
    if (!$o_index || (defined $o_index && $o_index =~ /^(.*)\.src$/)) {
        my $index = (defined $o_index ? $1 : "${o_srcdir}/prim_index.txt");

        open(INDEXO, ">$index") or
            die "cannot create <$index> : $!";

        if (defined $o_index) {
            open(INDEXI, "<${o_index}") or
                die "cannot open <${o_index}> : $!";

            while (defined (my $line = <INDEXI>)) {
                if ($line =~ /^\s+--MACRO--INDEX--\s+$/) {
                    ExportSRCIndex(\*INDEXO);
                } else {
                    print INDEXO $line;
                }
            }
            close(INDEXI);

        } else {
            ExportSRCIndex(\*INDEXO);
        }
        close (INDEXO);
    }

    # sections
    Info("Building sections");
    foreach my $section (sort keys %x_sections) {

        my $topic = $section;
        my $types = $x_sections{$section}->{TYPES};
        my $names = $x_sections{$section}->{NAMES};
        my $file  = $o_srcdir . '/prim_' . $section . '.txt';
        my $count = 0;

        Debug("section :<$section>");
        if (exists $x_topics{$section}) {
            $topic = $x_topics{$section};
        } else {
            my $Section = ucfirst($section);
            $topic = "$Section Primitives";
            $x_topics{$section} = $topic;
            Warning("unknown section topic '$Section'")
                if ('General' ne $Section);
        }

        foreach my $name (@$names) {
            if (! exists $x_definitions{$name} ||
                    ! defined $x_definitions{$name}->{BODY})  {
                Warning("missing '$name' body");
            }
        }

        foreach my $type (sort keys %$types) {

            # constants, enum, macro, struct, var
            my $bodies = $x_sections{$section}->{"BODIES:$type"};

            foreach my $name (sort @$bodies) {
                if (0 == $count++) {            # section data
                    open(SECT, ">$file") or
                        die "cannot create $file: $!";
                    print SECT " -ND- language=cpp -ND-\n";
                    print SECT "\n";
                    print SECT "Title:          $topic\n";
                    print SECT "\n";
                }

                my $body = $x_definitions{$name}->{BODY};
                my $refs = $x_definitions{$name}->{REFS};

                if (defined($refs)) {
                    foreach my $ref (@$refs) {
                        Warning("'${name}' unknown reference '${ref}'")
                            if (!exists $x_definitions{$ref} &&
                                    !exists $x_references{lc($ref)} &&
                                    !exists $x_features{lc($ref)});
                    }
                }

                foreach (@$body) {
                    print SECT "$_\n";
                }
            }
        }

        if (defined fileno(SECT)) {
            print SECT
                "\n" .
                '       $'.'Id: $'."\n" .
                '-*- mode: txt; margin: 75; -*-'."\n";
            close SECT;
        }
    }
}


sub
ExportSRCIndex($)       #(file)
{
    my ($file) = @_;

    print $file '(table "Macro Primitives", format=cvs, justify=center)'."\n";
    print $file '"Macro","Description"'."\n";
    ExportIndexType($file, '');
    print $file '(end table)'."\n\n";

    print $file '(table "Macro Constants", format=cvs, justify=center)'."\n";
    print $file '"Constant","Description"'."\n";
    ExportIndexType($file, 'const');
    print $file '(end table)'."\n\n";
}


sub
ExportIndexType($$)     #(file, type)
{
    my ($file, $type) = @_;

    foreach my $name (sort keys %x_definitions) {

        next if ($name =~ / /);                 # ignore non-macro
        next if ($type ne $x_definitions{$name}->{TYPE});

        my $desc = $x_definitions{$name}->{DESC};
        $desc = 'undocumented' if (! $desc);    # FIXME -- default

        if ($name =~ /[-=<>]/) {                # special reference
            printf $file '    "<link:\'%s\'>",%*s"%s"'."\n", $name, 17 - length($name), "", $desc;

        } else {                                # reference
            printf $file '    "<%s>",%*s"%s"'."\n", $name, 24 - length($name), "", $desc;
        }
    }
}


sub
ExportPrimTOC()         #()
{
    my $toc = "${o_hlpdir}/prim.toc";

    open(TOC, ">$toc") or
        die "cannot create <$toc> : $!";
    ExportTOCType(\*TOC, 'const');
    ExportTOCType(\*TOC, '');
    close TOC;
}


sub
ExportTOCType($$)       #(TOC, type)
{
    my ($TOC, $type) = @_;

    foreach my $name (sort keys %x_definitions) {

        next if ($name =~ / /);                 # ignore non-macro
        next if ($type ne $x_definitions{$name}->{TYPE});

        my $desc = $x_definitions{$name}->{DESC};
        $desc = 'undocumented' if (! $desc);    # FIXME -- default

        printf $TOC '%s%*s %s'."\n", $name, 24 - length($name), "", $desc;
    }
}


sub
ParseSRCFile($)         #(filename)
{
    my ($filename) = @_;

    open(SRC, "<${o_prmdir}/${filename}") or
        die "cannot open <$o_prmdir/$filename> : $!\n";

    my $state = 0;
    my $headers = 0;
    my $sect  = '';
    my $block = 0;
    my $type  = undef;
    my $name  = undef;
    my $desc  = undef;
    my $refs  = undef;
    my $opts  = undef;
    my @lines;

    while (defined (my $line = <SRC>)) {

        $line =~ s/\s*([\n\r]+|$)//;
        $line =~ s/\s+$//;                      # trailing whitespace

NEXT:;  if (0 == $state) {                      # <<GRIEF>> ...
            if ($line =~ /\<\<GRIEF\>\>/) {
                $sect = '';
                                                # <<GRIEF>> [<section>,<options>]
                if ($line =~ /\<\<GRIEF\>\>\s+\[(.+)\]/) {
                    my @parts = split(/[\s,]+/, $1);

                    $sect = shift @parts;
                    Warning("<<GRIEF>> [$sect] unknown section ($filename, $.)")
                        if ($sect && !exists $x_topics{$sect});

                    if (scalar @parts) {
                        $opts = {};
                        foreach(@parts)
                            { $$opts{lc($_)} = 1; }
                    }
                }
                $state = 1;
            }
            next;
        }

        if (1 == $state) {                      # Macro: name - description
            if ($line =~ /(Macro|Operator|Enum|Enumeration|Topic|Constant|Constants):\s+(.+?)\s+-\s+(.*)\s*$/) {
                $type  = $1;
                $name  = $2;
                $desc  = $3;
                $desc  =~ s/\.$//;
                $refs  = [];
                $state = 2;
                Debug("type: $type, section:$sect, desc:$desc");

            } elsif ($line =~ /(topic): (.*)\s*$/i) {
                $type  = lc($1);
                $name  = $2;
                $desc  = '';
                $state = 2;
                Debug("type: $type, section:$sect, desc:n/a");

            } elsif ($line =~ /(section): (.*)\s*$/i) {
                Warning("<<GRIEF>> encountered 'Section:' ($filename, $.)");

            } else {
                Warning("<<GRIEF>> encountered missing 'Macro:' ($filename, $.)");
                $state = $headers = 0;
            }

        } elsif ($line =~ /^\s*[\*]+\// ||      # '[*..]*/', <<END>>, <<GRIEF>>, <<GRIEF-TODO>>,
                    $line =~ /\<\<END\>\>/ || $line =~ /\<\<GRIEF.*\>\>/) {

            if (defined $type) {
                my $body = [];

                Debug("end ($type, $name)");
                if (defined $opts) {
                    foreach (keys %$opts) {
                        Debug("opt: $_");
                    }
                }

                push @$body,
                    sprintf "%s:%*s%s", $type, (15 - length($type)), "", $name;

                push @$body,     "        $desc."
                    if ($desc);

                if ($type ne 'topic' &&
                        (!defined $opts || !exists $$opts{noprototype})) {
                    push @$body, "";
                    push @$body, "    Prototype:";
                }

                    push @$body, @lines;
                    push @$body, "";
                    push @$body, "        " . ("-" x 50);
                    push @$body, "";

                if ('Macro' eq $type) {
                    if (!exists $x_primitives{$name}) {
                        if (!exists $x_features{$name}) {
                            Warning("unknown macro '$name'")
                                if (!$sect || ($sect ne 'callback'));
                        }
                    } elsif (!$sect && (my $lvl = $x_primitives{$name}) > 0) {
                        $sect = (1 == $lvl ? 'callback' : (2 == $lvl ? 'macro' : 'general'));
                    }
                }

                PushSection($sect, $name, 1)
                    if ($sect);                 # update section

                PushDefinition($name, $type, $desc, $body, $refs, $opts);

                @lines = ();
                $sect = '';
                $type = undef;
                $name = undef;
                $desc = undef;
                $refs = undef;
                $opts = undef;
            }

            $state   = 0;
            $headers = 0;
            $block   = 0;
            goto NEXT;

        } elsif ($type && $line =~ /\s*${type}\s+(.*):\s*$/) {
                                                # remove 'Type Xxxxxx:', for example 'Constant See Also:'
            Debug("+    $1:");
            push @lines, "    $1:";

            $state = ($line =~ /See Also:\s*$/i ? 3 : 2);

        } elsif ($type && ($type ne 'Macro') &&
                        $line =~ /\s*Macro\s+(.*):\s*$/) {
                                                # remove 'Macro Xxxxxx:'
            Debug("+    $1:");
            push @lines, "    $1:";

            $state = ($line =~ /See Also:\s*$/i ? 3 : 2);

        } elsif (3 == $state || 4 == $state) {
                                                # See Also:
            if ($line =~ /------/) {
                $state = 2;
                next;
            }

            my @parts = split(/(,|\s+and\s+)/, $line);
            if (scalar @parts) {

                $line = '';
                foreach (@parts) {

                    s/^\s+//g;                  # leading whitespace
                    s/[.;:!]//g;                # delimiters
                    s/\s+$//g;                  # trailing whitespace
                    s/\(\)$//g;                 # remove trailing ()

                    next if (!$_);
                    my $ref = $_;
                    next if ($ref eq ',' || $ref eq 'and');
                                                # xxx, xxx and xxx

                    next if ($ref eq $name);    # Ignore selfies

                    Debug("+         ${ref}");
                    $line .= ', ' if ($line);
                    if ($ref =~ /[-=<>]/) {     # link's
                        $line .= "<link:'${ref}'>";
                    } else {
                        $line .= "<${ref}>";
                    }
                    push @$refs, $ref;
                }

                $lines[-1] .= ','               # continuation
                    if (4 == $state);
                push @lines, "        $line";
                $state = 4;

            } else {
                push @lines, "";
                $state = 3;
            }

        } else {
            # leading comments
            $line =~ s/^\/\//  /;               # //
            $line =~ s/^[ ]?\*/  /;             #  *

            if ($block) {
                if ($line =~ /^ *\( *(?:end|finish|done)(?: +(?:code|example|diagram|ditaa|mscgen|sdedit|drawing))? *\)$/i) {
                    $block = 0;                 # ND, end block markups
                }

            } elsif ($line =~ /^\( *(?:(?:start|begin)? +)?(?:code|example|diagram|ditaa|mscgen|sdedit|drawing)([^\)]*)\)$/i) {
                $block = 1;                     # ND, start of block markups


            } elsif ($line !~ /<\s*\</) {       # ignoring inline code markups

                $headers++                      # Header:
                    if ($line =~ /^\s.*:\s*$/);

                $x_references{lc($1)} = 'topic' # Topic:
                    if ($line =~ /^\s*Topic:\s+(.*)$/);

                if ($headers) {                 # Markup topic within body
                    $line =~ s/ ${name}([ ,.;:!])/ '${name}'$1/g
                        if ($name =~ /^[A-Za-z_]+$/);
                }

                                                # links
                $line =~ s/\(see ([a-z0-9_]+)\)/(See: <$1>)/gi;

                $line =~ s/\s+$//;              # white-space

                                                # Grief style
                $line =~ s/ GriefEdit([ ,.;:!])/ *GriefEdit*$1/g;   # GRIEF
                $line =~ s/ Grief([ ,.;:!])/ *GriefEdit*$1/g;       # GRIEF

                                                # CRiSP style
                $line =~ s/ CrispEdit([ ,.;:!])/ 'CRiSP (tm)'$1/gi;
                $line =~ s/ Crisp([ ,.;:!])/ 'CRiSP (tm)'$1/g;
            }

            Debug("+ ${line}");
            push @lines, $line;
        }
    }
    close(SRC);
}


sub
Trim                    #(text)
{
    my $text = shift;
    $text =~ s/^\s+//;
    $text =~ s/\s+$//;
    return $text;
}


sub
Info
{
    print "@_\n";
}


sub
Verbose
{
    print "@_\n"
        if ($o_verbose);
}


sub
Warning
{
    print "(W) @_\n"
        if ($o_warning);
}


sub
Debug
{
    print "(D) @_\n"
        if ($o_debug);
}

#end


