#!/usr/bin/perl
# $Id: base16.pl,v 1.1 2024/10/25 14:47:52 cvsuser Exp $
# -*- mode: perl; tabs: 8; indent-width: 4; -*-
# base16 profile builder.
#
#

use strict;
use warnings;

use File::Fetch;
use File::Find;
use File::Basename;
use Data::Dumper;
use POSIX qw(strftime asctime);
use YAML::XS 'LoadFile';

my $opull = 0;
my @list;
my %license;
my %schemes;
my %url;

sub main();
sub import_schemes();
sub generate_ini();
sub generate_cr();
sub get_url($);

exit main();

sub
main()
{
        @list = LoadFile("list.yaml");

        if ($opull) {
                mkdir("schemes")
                        if (! -d "schemes");
                foreach my $elm (@list) {
                        while (my($name, $base) = each(%{$elm})) {
                                my $dir = "schemes/${name}";
                                system("git clone $base $dir")
                                        if (! -d $dir);
                        }
                }
        }
        import_schemes();
##      generate_ini();
        generate_cr();
}


sub
import_schemes()
{
        foreach my $elm (@list) {
                while (my($name, $base) = each(%{$elm})) {
                        my $dir = "schemes/${name}";
                        my $license_file = undef;
                        my @files = ();

                        find(sub {
                                if (-f $_) {
                                        if ($_ =~ /\.yaml$/) {
                                                push @files, $File::Find::name;
                                        } elsif ($_ =~ /LICENSE/) {
                                                $license_file = $File::Find::name;
                                        }
                                }
                        }, $dir);

                        if (scalar @files) {
                                $url{$name} = get_url($dir);
                                $license{$name} = $license_file
                                        if ($license_file);
                                $schemes{$name} = \@files;
                        }
                }
        }
}


sub
generate_cr()
{
        open(CR, ">schemes.cr") or
                die "base16: unable to create <schemes.cr>\n";

        my $timestamp = asctime(localtime);
        print CR "// \$Id: base16.pl,v 1.1 2024/10/25 14:47:52 cvsuser Exp $\n";
        print CR "// base16 - scheme's\n";
        print CR "//\n";
        print CR "// See: https://github.com/chriskempson/base16-schemes-source\n";
        print CR "// Auto-generated ${timestamp}\n";

        my %descriptions;

        print CR "void main()\n";
        print CR "{\n";
        print CR "\tmodule(\"base16\");\n";
        print CR "}\n";

        while (my($scheme, $files) = each(%schemes)) {

                print "${scheme}\n";
                print "\t".$license{$scheme}."\n"
                        if (exists $license{$scheme});

                # licence

                print CR "\n";
                print CR "// ${scheme}-license\n";
                if (exists $license{$scheme}) {
                        my $in = $license{$scheme};

                        open (LICENSE, $in) or
                                die "base16: can't open license <${in}>: $!";
                        while (<LICENSE>) {
                                chomp;                  # newline
                                s/\s+$//;               # whitespace

                                print CR "// ".$_."\n";
                        }
                        close(LICENSE);
                } else {
                        print CR "// MIT License\n";
                }
                print CR "// ".$url{$scheme}."\n";

                # process scheme(s)

                foreach my $file (@$files) {
                        my $name = basename($file);
                        next if ($name =~ /^base16-/);

                        print "\t${file}\n";
                        open (SCHEME, $file) or
                                die "base16: can't open schema <${file}>: $!";

                        $name =~ s/\.yaml$//;           # remove extension
                        $name =~ s/[-\.\s]+/_/g;        # specials => '_'
                        $name =~ s/_+/_/g;              # compress

                        print CR "\n";
                        print CR "static list\n";
                        print CR "def_${name}()\n";
                        print CR "{\n";
                        print CR "\tlist scheme = {\n";
                        while (<SCHEME>) {
                                chomp;                  # newline
                                s/#.*$//;               # comments
                                s/\s+$//;               # whitespace

                                if (/^base([[:xdigit:]]+): \"(.*)\"/) {
                                        print CR "\t\t\"$1\", 0x$2,\n";
                                } else {
                                        if (/^scheme: \"(.*)\"/) {
                                                print CR "\t\t\"scheme\", \"$1\",\n";
                                                $descriptions{$1} = $name;
                                        } elsif (/^author: \"(.+)\"/) {
                                                print CR "\t\t\"author\", \"$1\",\n";
                                        }
                                }
                        }
                        print CR "\t};\n";
                        print CR "\treturn scheme;\n";
                        print CR "}\n";
                        close(SCHEME);
                }
        }

        print CR <<EOT;

//
//  base16_schemes ---
//      Return the scheme list.
//
list
base16_schemes()
{
    list schemes = {
EOT
        foreach my $tag (sort {uc($a) cmp uc($b)} keys %descriptions) {
                print CR '        "'.$tag.'", "'.$descriptions{$tag}.'",'."\n";
        }
    print CR <<EOT;
    };
    return schemes;
}

//end
EOT
}


sub
generate_ini()
{
        open(INI, ">base16.ini") or
                die "base16: unable to create <base16.ini>\n";

        my $timestamp = asctime(localtime);
        print INI "; base16 - scheme's\n";
        print INI "; See: https://github.com/chriskempson/base16-schemes-source\n";
        print INI "; Auto-generated ${timestamp}\n";

        while (my($scheme, $files) = each(%schemes)) {

                print "${scheme}\n";
                print "\t".$license{$scheme}."\n"
                        if (exists $license{$scheme});

                # licence

                print INI "[${scheme}-license]\n";
                if (exists $license{$scheme}) {
                        my $in = $license{$scheme};

                        open (LICENSE, $in) or
                                die "base16: can't open license <${in}>: $!";
                        while (<LICENSE>) {
                                chomp;                  # newline
                                s/\s+$//;               # whitespace

                                print INI "; ".$_."\n";
                        }
                        close(LICENSE);
                } else {
                        print INI "; MIT License\n";
                }
                print INI "url=".$url{$scheme}."\n";

                # process scheme(s)

                foreach my $file (@$files) {
                        my $name = basename($file);
                        next if ($name =~ /^base16-/);

                        print "\t${file}\n";
                        open (SCHEME, $file) or
                                die "base16: can't open schema <${file}>: $!";

                        print INI "\n[${name}]\n";
                        while (<SCHEME>) {
                                chomp;                  # newline
                                s/#.*$//;               # comments
                                s/\s+$//;               # whitespace

                                if (/^(base[[:xdigit:]]+): \"(.*)\"/) {
                                        print INI "$1=$2\n";
                                } else {
                                        if (/^author: \"(.+)\"/) {
                                                print INI "author=$1\n";
                                        } elsif (/^scheme: \"(.*)\"/) {
                                                print INI "scheme=$1\n";
                                        }
                                }
                        }
                        close(SCHEME);
                }

                print INI "\n";
        }
}


sub
get_url($)
{
        my $dir = shift;
        my $config = "${dir}/.git/config";
        my $url = "";

        open (URL, $config) or
                die "base16: can't open git config <${config}>: $!";
        while (<URL>) {
                chomp;
                if (/url = (.*)$/) {
                        $url = $1;
                        last;
                }
        }
        close(URL);
        return $url;
}

1;

