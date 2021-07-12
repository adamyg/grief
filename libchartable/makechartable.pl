#!/usr/bin/perl
# -*- mode: perl; tabs: 8; indent-width: 4; -*-
# $Id: makechartable.pl,v 1.21 2021/07/12 15:37:11 cvsuser Exp $
# Character table generation.
#
# Copyright (c) 2010 - 2021, Adam Young.
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
#
#
#

BEGIN {
    my $var = $ENV{"PERLINC"};

    if (defined($var) && -d $var) {             # import PERLINC
        my ($qvar) = quotemeta($var);
        push (@INC, $var)
            if (! grep /^$qvar$/, @INC);
    } elsif ($^O eq "MSWin32") {                # activePerl (defaults)
        if (! grep /\/perl\/lib/, @INC) {
            push (@INC, "c:/perl/lib") if (-d "c:/perl/lib");
            push (@INC, "/perl/lib") if (-d "/perl/lib");
        }
    }
}


use strict;
use warnings;

use POSIX 'ctime';
use utf8;

use constant CS7BIT     => 0x01;
use constant CS8BIT     => 0x02;
use constant CS16BIT    => 0x04;
use constant CSPACKAGED => 0x10;

my  %PACKAGES   = (
    1 => 'iso8859',
    2 => 'mac8bit',
    3 => 'adobe',
    4 => 'cp8bit',
    5 => 'koi',
    9 => 'misc'
    );

my  @MAPPINGS   = (
    { CN => "US-ASCII",         PK => 0,    FM => 1,    CS => CS7BIT,   CP => 0,        FN => "VENDORS/MISC/US-ASCII-QUOTES.TXT" },

    { CN => "ISO-8859-1",       PK => 1,    FM => 1,    CS => CS8BIT,   CP => 0,        FN => "ISO8859/8859-1.TXT" },
    { CN => "ISO-8859-2",       PK => 1,    FM => 1,    CS => CS8BIT,   CP => 0,        FN => "ISO8859/8859-2.TXT" },
    { CN => "ISO-8859-3",       PK => 1,    FM => 1,    CS => CS8BIT,   CP => 0,        FN => "ISO8859/8859-3.TXT" },
    { CN => "ISO-8859-4",       PK => 1,    FM => 1,    CS => CS8BIT,   CP => 0,        FN => "ISO8859/8859-4.TXT" },
    { CN => "ISO-8859-5",       PK => 1,    FM => 1,    CS => CS8BIT,   CP => 0,        FN => "ISO8859/8859-5.TXT" },
    { CN => "ISO-8859-6",       PK => 1,    FM => 1,    CS => CS8BIT,   CP => 0,        FN => "ISO8859/8859-6.TXT" },
    { CN => "ISO-8859-7",       PK => 1,    FM => 1,    CS => CS8BIT,   CP => 0,        FN => "ISO8859/8859-7.TXT" },
    { CN => "ISO-8859-8",       PK => 1,    FM => 1,    CS => CS8BIT,   CP => 0,        FN => "ISO8859/8859-8.TXT" },
    { CN => "ISO-8859-9",       PK => 1,    FM => 1,    CS => CS8BIT,   CP => 0,        FN => "ISO8859/8859-9.TXT" },
    { CN => "ISO-8859-10",      PK => 1,    FM => 1,    CS => CS8BIT,   CP => 0,        FN => "ISO8859/8859-10.TXT" },
    { CN => "ISO-8859-11",      PK => 1,    FM => 1,    CS => CS8BIT,   CP => 0,        FN => "ISO8859/8859-11.TXT" },
    { CN => "ISO-8859-13",      PK => 1,    FM => 1,    CS => CS8BIT,   CP => 0,        FN => "ISO8859/8859-13.TXT" },
    { CN => "ISO-8859-14",      PK => 1,    FM => 1,    CS => CS8BIT,   CP => 0,        FN => "ISO8859/8859-14.TXT" },
    { CN => "ISO-8859-15",      PK => 1,    FM => 1,    CS => CS8BIT,   CP => 0,        FN => "ISO8859/8859-15.TXT" },
    { CN => "ISO-8859-16",      PK => 1,    FM => 1,    CS => CS8BIT,   CP => 0,        FN => "ISO8859/8859-16.TXT" },

    { CN => "Mac-Arabic",       PK => 2,    FM => 1,    CS => CS8BIT,   CP => 0,        FN => "VENDORS/APPLE/ARABIC.TXT" },
    { CN => "Mac-Celtic",       PK => 2,    FM => 1,    CS => CS8BIT,   CP => 0,        FN => "VENDORS/APPLE/CELTIC.TXT" },
    { CN => "Mac-Centeuro",     PK => 2,    FM => 1,    CS => CS8BIT,   CP => 0,        FN => "VENDORS/APPLE/CENTEURO.TXT" },
    { CN => "Mac-Chinsimp",                 FM => 1,    CS => CS16BIT,  CP => 0,        FN => "VENDORS/APPLE/CHINSIMP.TXT" },
    { CN => "Mac-Chintrad",                 FM => 1,    CS => CS16BIT,  CP => 0,        FN => "VENDORS/APPLE/CHINTRAD.TXT" },
#   { CN => "Mac-Corpchar",                 FM => 1,    CS => NA,       CP => 0,        FN => "VENDORS/APPLE/CORPCHAR.TXT" },
    { CN => "Mac-Croatian",     PK => 2,    FM => 1,    CS => CS8BIT,   CP => 0,        FN => "VENDORS/APPLE/CROATIAN.TXT" },
    { CN => "Mac-Cyrillic",     PK => 2,    FM => 1,    CS => CS8BIT,   CP => 0,        FN => "VENDORS/APPLE/CYRILLIC.TXT" },
    { CN => "Mac-Devanaga",     PK => 2,    FM => 1,    CS => CS8BIT,   CP => 0,        FN => "VENDORS/APPLE/DEVANAGA.TXT" },
    { CN => "Mac-Dingbats",     PK => 2,    FM => 1,    CS => CS8BIT,   CP => 0,        FN => "VENDORS/APPLE/DINGBATS.TXT" },
    { CN => "Mac-Farsi",        PK => 2,    FM => 1,    CS => CS8BIT,   CP => 0,        FN => "VENDORS/APPLE/FARSI.TXT" },
    { CN => "Mac-Gaelic",       PK => 2,    FM => 1,    CS => CS8BIT,   CP => 0,        FN => "VENDORS/APPLE/GAELIC.TXT" },
    { CN => "Mac-Greek",        PK => 2,    FM => 1,    CS => CS8BIT,   CP => 0,        FN => "VENDORS/APPLE/GREEK.TXT" },
    { CN => "Mac-Gujarati",     PK => 2,    FM => 1,    CS => CS8BIT,   CP => 0,        FN => "VENDORS/APPLE/GUJARATI.TXT" },
    { CN => "Mac-Gurmukhi",     PK => 2,    FM => 1,    CS => CS8BIT,   CP => 0,        FN => "VENDORS/APPLE/GURMUKHI.TXT" },
    { CN => "Mac-Hebrew",       PK => 2,    FM => 1,    CS => CS8BIT,   CP => 0,        FN => "VENDORS/APPLE/HEBREW.TXT" },
    { CN => "Mac-Iceland",      PK => 2,    FM => 1,    CS => CS8BIT,   CP => 0,        FN => "VENDORS/APPLE/ICELAND.TXT" },
    { CN => "Mac-Inuit",        PK => 2,    FM => 1,    CS => CS8BIT,   CP => 0,        FN => "VENDORS/APPLE/INUIT.TXT" },
#   { CN => "Mac-Japanese",                 FM => 1,    CS => NA,       CP => 0,        FN => "VENDORS/APPLE/JAPANESE.TXT" },
#   { CN => "Mac-Keyboard",                 FM => 1,    CS => NA,       CP => 0,        FN => "VENDORS/APPLE/KEYBOARD.TXT" },
#   { CN => "Mac-Korean",                   FM => 1,    CS => NA,       CP => 0,        FN => "VENDORS/APPLE/KOREAN.TXT" },
    { CN => "Mac-Roman",        PK => 2,    FM => 1,    CS => CS8BIT,   CP => 0,        FN => "VENDORS/APPLE/ROMAN.TXT" },
    { CN => "Mac-Romanian",     PK => 2,    FM => 1,    CS => CS8BIT,   CP => 0,        FN => "VENDORS/APPLE/ROMANIAN.TXT" },
#   { CN => "Mac-Symbol",                   FM => 1,    CS => NA,       CP => 0,        FN => "VENDORS/APPLE/SYMBOL.TXT" },
    { CN => "Mac-Thai",         PK => 2,    FM => 1,    CS => CS8BIT,   CP => 0,        FN => "VENDORS/APPLE/THAI.TXT" },
    { CN => "Mac-Turkish",      PK => 2,    FM => 1,    CS => CS8BIT,   CP => 0,        FN => "VENDORS/APPLE/TURKISH.TXT" },
    { CN => "Mac-Ukraine",      PK => 2,    FM => 1,    CS => CS8BIT,   CP => 0,        FN => "VENDORS/APPLE/CYRILLIC.TXT" },   #UKRAINE.TXT

    { CN => "Adobe-Stdenc",     PK => 3,    FM => 2,    CS => CS8BIT,   CP => 0,        FN => "VENDORS/ADOBE/stdenc.txt" },
    { CN => "Adobe-Symbol",     PK => 3,    FM => 2,    CS => CS8BIT,   CP => 0,        FN => "VENDORS/ADOBE/symbol.txt" },
    { CN => "Adobe-Zdingbat",   PK => 3,    FM => 2,    CS => CS8BIT,   CP => 0,        FN => "VENDORS/ADOBE/zdingbat.txt" },

    { CN => "CP037",            PK => 4,    FM => 1,    CS => CS8BIT,   CP => 37,       FN => "VENDORS/MICSFT/EBCDIC/CP037.TXT" },
    { CN => "CP500",            PK => 4,    FM => 1,    CS => CS8BIT,   CP => 500,      FN => "VENDORS/MICSFT/EBCDIC/CP500.TXT" },
    { CN => "CP875",            PK => 4,    FM => 1,    CS => CS8BIT,   CP => 875,      FN => "VENDORS/MICSFT/EBCDIC/CP875.TXT" },
    { CN => "CP1026",           PK => 4,    FM => 1,    CS => CS8BIT,   CP => 1026,     FN => "VENDORS/MICSFT/EBCDIC/CP1026.TXT" },

    { CN => "CP437",            PK => 4,    FM => 1,    CS => CS8BIT,   CP => 437,      FN => "VENDORS/MICSFT/PC/CP437.TXT" },
    { CN => "CP737",            PK => 4,    FM => 1,    CS => CS8BIT,   CP => 737,      FN => "VENDORS/MICSFT/PC/CP737.TXT" },
    { CN => "CP775",            PK => 4,    FM => 1,    CS => CS8BIT,   CP => 775,      FN => "VENDORS/MICSFT/PC/CP775.TXT" },
    { CN => "CP850",            PK => 4,    FM => 1,    CS => CS8BIT,   CP => 850,      FN => "VENDORS/MICSFT/PC/CP850.TXT" },
    { CN => "CP852",            PK => 4,    FM => 1,    CS => CS8BIT,   CP => 852,      FN => "VENDORS/MICSFT/PC/CP852.TXT" },
    { CN => "CP855",            PK => 4,    FM => 1,    CS => CS8BIT,   CP => 855,      FN => "VENDORS/MICSFT/PC/CP855.TXT" },
    { CN => "CP857",            PK => 4,    FM => 1,    CS => CS8BIT,   CP => 857,      FN => "VENDORS/MICSFT/PC/CP857.TXT" },
    { CN => "CP860",            PK => 4,    FM => 1,    CS => CS8BIT,   CP => 860,      FN => "VENDORS/MICSFT/PC/CP860.TXT" },
    { CN => "CP861",            PK => 4,    FM => 1,    CS => CS8BIT,   CP => 861,      FN => "VENDORS/MICSFT/PC/CP861.TXT" },
    { CN => "CP862",            PK => 4,    FM => 1,    CS => CS8BIT,   CP => 862,      FN => "VENDORS/MICSFT/PC/CP862.TXT" },
    { CN => "CP863",            PK => 4,    FM => 1,    CS => CS8BIT,   CP => 863,      FN => "VENDORS/MICSFT/PC/CP863.TXT" },
    { CN => "CP864",            PK => 4,    FM => 1,    CS => CS8BIT,   CP => 864,      FN => "VENDORS/MICSFT/PC/CP864.TXT" },
    { CN => "CP865",            PK => 4,    FM => 1,    CS => CS8BIT,   CP => 865,      FN => "VENDORS/MICSFT/PC/CP865.TXT" },
    { CN => "CP866",            PK => 4,    FM => 1,    CS => CS8BIT,   CP => 866,      FN => "VENDORS/MICSFT/PC/CP866.TXT" },
    { CN => "CP869",            PK => 4,    FM => 1,    CS => CS8BIT,   CP => 869,      FN => "VENDORS/MICSFT/PC/CP869.TXT" },
#   { CN => "CP874",                        FM => 1,    CS => CS8BIT,   CP => 874,      FN => "VENDORS/MICSFT/PC/CP874.TXT" },

    { CN => "CP874",            PK => 4,    FM => 1,    CS => CS8BIT,   CP => 874,      FN => "VENDORS/MICSFT/WINDOWS/CP874.TXT" },
    { CN => "CP932",                        FM => 1,    CS => CS16BIT,  CP => 932,      FN => "VENDORS/MICSFT/WINDOWS/CP932.TXT" },
    { CN => "CP936",                        FM => 1,    CS => CS16BIT,  CP => 936,      FN => "VENDORS/MICSFT/WINDOWS/CP936.TXT" },
    { CN => "CP949",                        FM => 1,    CS => CS16BIT,  CP => 949,      FN => "VENDORS/MICSFT/WINDOWS/CP949.TXT" },
    { CN => "CP950",                        FM => 1,    CS => CS16BIT,  CP => 950,      FN => "VENDORS/MICSFT/WINDOWS/CP950.TXT" },
    { CN => "CP1250",           PK => 4,    FM => 1,    CS => CS8BIT,   CP => 1250,     FN => "VENDORS/MICSFT/WINDOWS/CP1250.TXT" },
    { CN => "CP1251",           PK => 4,    FM => 1,    CS => CS8BIT,   CP => 1251,     FN => "VENDORS/MICSFT/WINDOWS/CP1251.TXT" },
    { CN => "CP1252",           PK => 4,    FM => 1,    CS => CS8BIT,   CP => 1252,     FN => "VENDORS/MICSFT/WINDOWS/CP1252.TXT" },
    { CN => "CP1253",           PK => 4,    FM => 1,    CS => CS8BIT,   CP => 1253,     FN => "VENDORS/MICSFT/WINDOWS/CP1253.TXT" },
    { CN => "CP1254",           PK => 4,    FM => 1,    CS => CS8BIT,   CP => 1254,     FN => "VENDORS/MICSFT/WINDOWS/CP1254.TXT" },
    { CN => "CP1255",           PK => 4,    FM => 1,    CS => CS8BIT,   CP => 1255,     FN => "VENDORS/MICSFT/WINDOWS/CP1255.TXT" },
    { CN => "CP1256",           PK => 4,    FM => 1,    CS => CS8BIT,   CP => 1256,     FN => "VENDORS/MICSFT/WINDOWS/CP1256.TXT" },
    { CN => "CP1257",           PK => 4,    FM => 1,    CS => CS8BIT,   CP => 1257,     FN => "VENDORS/MICSFT/WINDOWS/CP1257.TXT" },
    { CN => "CP1258",           PK => 4,    FM => 1,    CS => CS8BIT,   CP => 1258,     FN => "VENDORS/MICSFT/WINDOWS/CP1258.TXT" },

#   //TRANSLIT mode usage??
#
#   { CN => "Windows-874",                  FM => 1,    CS => CS8BIT,   CP => -874,     FN => "VENDORS/MICSFT/WindowsBestFit/bestfit874.txt" },
#   { CN => "Windows-932",                  FM => 1,    CS => CS16BIT,  CP => -932,     FN => "VENDORS/MICSFT/WindowsBestFit/bestfit932.txt" },
#   { CN => "Windows-936",                  FM => 1,    CS => CS16BIT,  CP => -936,     FN => "VENDORS/MICSFT/WindowsBestFit/bestfit936.txt" },
#   { CN => "Windows-949",                  FM => 1,    CS => CS16BIT,  CP => -949,     FN => "VENDORS/MICSFT/WindowsBestFit/bestfit949.txt" },
#   { CN => "Windows-950",                  FM => 1,    CS => CS16BIT,  CP => -950,     FN => "VENDORS/MICSFT/WindowsBestFit/bestfit950.txt" },
#   { CN => "Windows-1250",                 FM => 1,    CS => CS8BIT,   CP => -1250,    FN => "VENDORS/MICSFT/WindowsBestFit/bestfit1250.txt" },
#   { CN => "Windows-1251",                 FM => 1,    CS => CS8BIT,   CP => -1251,    FN => "VENDORS/MICSFT/WindowsBestFit/bestfit1251.txt" },
#   { CN => "Windows-1252",                 FM => 1,    CS => CS8BIT,   CP => -1252,    FN => "VENDORS/MICSFT/WindowsBestFit/bestfit1252.txt" },
#   { CN => "Windows-1253",                 FM => 1,    CS => CS8BIT,   CP => -1253,    FN => "VENDORS/MICSFT/WindowsBestFit/bestfit1253.txt" },
#   { CN => "Windows-1254",                 FM => 1,    CS => CS8BIT,   CP => -1254,    FN => "VENDORS/MICSFT/WindowsBestFit/bestfit1254.txt" },
#   { CN => "Windows-1255",                 FM => 1,    CS => CS8BIT,   CP => -1255,    FN => "VENDORS/MICSFT/WindowsBestFit/bestfit1255.txt" },
#   { CN => "Windows-1256",                 FM => 1,    CS => CS8BIT,   CP => -1256,    FN => "VENDORS/MICSFT/WindowsBestFit/bestfit1256.txt" },
#   { CN => "Windows-1257",                 FM => 1,    CS => CS8BIT,   CP => -1257,    FN => "VENDORS/MICSFT/WindowsBestFit/bestfit1257.txt" },
#   { CN => "Windows-1258",                 FM => 1,    CS => CS8BIT,   CP => -1258     FN => "VENDORS/MICSFT/WindowsBestFit/bestfit1258.txt" },

    { CN => "CP10007",          PK => 4,    FM => 1,    CS => CS8BIT,   CP => 10007,    FN => "VENDORS/MICSFT/MAC/CYRILLIC.TXT" },
    { CN => "CP10006",          PK => 4,    FM => 1,    CS => CS8BIT,   CP => 10006,    FN => "VENDORS/MICSFT/MAC/GREEK.TXT" },
    { CN => "CP10079",          PK => 4,    FM => 1,    CS => CS8BIT,   CP => 10079,    FN => "VENDORS/MICSFT/MAC/ICELAND.TXT" },
    { CN => "CP10029",          PK => 4,    FM => 1,    CS => CS8BIT,   CP => 10029,    FN => "VENDORS/MICSFT/MAC/LATIN2.TXT" },
    { CN => "CP10000",          PK => 4,    FM => 1,    CS => CS8BIT,   CP => 10000,    FN => "VENDORS/MICSFT/MAC/ROMAN.TXT" },
    { CN => "CP10081",          PK => 4,    FM => 1,    CS => CS8BIT,   CP => 10081,    FN => "VENDORS/MICSFT/MAC/TURKISH.TXT" },

#   { CN => "APL_ISO-IR-68",                FM => 1,    CS => NA,       CP => 0,        FN => "VENDORS/MISC/APL-ISO-IR-68.TXT" },
#   { CN => "SGML",                         FM => 1,    CS => NA,       CP => 0,        FN => "VENDORS/MISC/SGML.TXT" },

    { CN => "GSM0338",          PK => 9,    FM => 1,    CS => CS7BIT,   CP => 0,        FN => "ETSI/GSM0338.TXT" },

    { CN => "AtariST",          PK => 9,    FM => 1,    CS => CS8BIT,   CP => 0,        FN => "VENDORS/MISC/ATARIST.TXT" },
    { CN => "CP424",            PK => 4,    FM => 1,    CS => CS8BIT,   CP => 424,      FN => "VENDORS/MISC/CP424.TXT" },
    { CN => "CP856",            PK => 4,    FM => 1,    CS => CS8BIT,   CP => 856,      FN => "VENDORS/MISC/CP856.TXT" },
    { CN => "CP1006",           PK => 4,    FM => 1,    CS => CS8BIT,   CP => 1006,     FN => "VENDORS/MISC/CP1006.TXT" },

    { CN => "KOI8-R",           PK => 5,    FM => 1,    CS => CS8BIT,   CP => 0,        FN => "VENDORS/MISC/KOI8-R.TXT" },
    { CN => "KOI8-U",           PK => 5,    FM => 1,    CS => CS8BIT,   CP => 0,        FN => "VENDORS/MISC/KOI8-U.TXT" },
    { CN => "KZ1048",           PK => 5,    FM => 1,    CS => CS8BIT,   CP => 0,        FN => "VENDORS/MISC/KZ1048.TXT" },

    { CN => "NextStep",         PK => 9,    FM => 10,   CS => CS8BIT,   CP => 0,        FN => "VENDORS/NEXT/NEXTSTEP.TXT" },
    );


my  @USASCII = (                # 0x00 .. 0x1f
    "NULL",
    "START OF HEADING",
    "START OF TEXT",
    "END OF TEXT",
    "END OF TRANSMISSION",
    "ENQUIRY",
    "ACKNOWLEDGE",
    "BELL",
    "BACKSPACE",
    "HORIZONTAL TABULATION",
    "LINE FEED",
    "VERTICAL TABULATION",
    "FORM FEED",
    "CARRIAGE RETURN",
    "SHIFT OUT",
    "SHIFT IN",
    "DATA LINK ESCAPE",
    "DEVICE CONTROL ONE",
    "DEVICE CONTROL TWO",
    "DEVICE CONTROL THREE",
    "DEVICE CONTROL FOUR",
    "NEGATIVE ACKNOWLEDGE",
    "SYNCHRONOUS IDLE",
    "END OF TRANSMISSION BLOCK",
    "CANCEL",
    "END OF MEDIUM",
    "SUBSTITUTE",
    "ESCAPE",
    "FN SEPARATOR",
    "GROUP SEPARATOR",
    "RECORD SEPARATOR",
    "UNIT SEPARATOR"
    );


my  %QUOTES = (                 # Unicode quotes
    0x2018 => 0x60,                             # LEFT SINGLE QUOTATION MARK
    0x2019 => 0x27,                             # RIGHT SINGLE QUOTATION MARK
    0x201C => 0x22,                             # LEFT DOUBLE QUOTATION MARK
    0x201D => 0x22,                             # RIGHT DOUBLE QUOTATION MARK
    0x00B4 => 0x27                              # ACUTE ACCENT
    );

my  $COPYRIGHT          =
    " * Copy"."right (c) 2010 - 2020, Adam Young.\n".
    " * All rights reserved.\n".
    " *\n".
    " * This"." file is part of the GRIEF Editor.\n".
    " *\n".
    " * The GRIEF Editor is free software: you can redistribute it\n".
    " * and/or modify it under the terms of the GRIEF Editor License.\n".
    " *\n".
    " * The GRIEF Editor is distributed in the hope that it will be useful,\n".
    " * but WITHOUT ANY WARRANTY; without even the implied warranty of\n".
    " * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n".
    " * See the License for more details.\n";

my  %description_text;                          # unique description text
my  %description_index;                         # description index by unicode

my  @description_strings;                       # description by index
my  @description_unicode;                       # unicode by index
my  @description_hits;                          # hits by index

my  $x_timestamp        = ctime(time());
chomp($x_timestamp);

my  $x_warnings         = 0;                    # import warnings
my  $x_crunch           = 0;                    # cruch description table
my  $x_labels           = 0;                    # character labels
my  $x_packaged         = 0;                    # generated packaged dynamic modules
my  $x_extended         = 1;                    # extended US-ASCII table generation
my  $x_dynamic          = 0;                    # enable dynamic loader support
my  $x_mappings         = undef;
my  $x_data             = "./data";
my  $x_outdir           = "./cnvtables";

my  $x_undef_name       = "UNDEF";
my  $x_undef_unicode    = 0xfffe;
my  $x_undef_external   = 0xfffe;


sub Main();
sub ImportMapping($$$);
sub ImportUnicodeData($);
sub NameTable($);
sub DefinitionTable($);
sub DescriptionTable($);
sub Unicode2UTF8($);
sub Usage;

Main();
exit 0;


#   Function:           Main
#       Main body.
#
#   Parameters:
#       none.
#
#   Return:
#       nothing.
#
sub
Main()                  #()
{
    my $argc = 0;                               # argument count
    my $cmd = "";

    while ($_ = $ARGV[$argc++]) {
        last if (! /^-/);                       # end of options

        next if (/^--warn[ings]+$|^-w$/ && ++$x_warnings);
        next if (/^--data=(.*)$/        && ($x_data = $1) ne "");
        next if (/^--mappings=(.*)$/    && ($x_mappings = $1) ne "");
        next if (/^--outdir=(.*)$/      && ($x_outdir = $1) ne "");
        next if (/^--crunch$/           && ++$x_crunch);
        next if (/^--labels/            && ++$x_labels);
        next if (/^--packaged/          && ++$x_packaged);
        next if (/^--extended/          && ++$x_extended);
        next if (/^--dynamic/           && ++$x_dynamic);

        /^-h$|^-help$|^--help$|^--usage$|^-\?$/ && Usage();

        Usage("unknown option '$_'\n");
    }

    $x_mappings = "${x_data}/MAPPINGS"          # default MAPPINGS
        if (!$x_mappings);

    $cmd = $ARGV[$argc - 1];
    $cmd = "" if (! defined($cmd));
    Usage() if ($cmd ne "");

    select((select(STDOUT), $|=1)[0]);          # make stdout hot

    system("unzip -u -d ${x_data}/UCD ${x_data}/UCD.zip UnicodeData.txt")
        if (-f "${x_data}/UCD.zip");            # export latest copy
    ImportUnicodeData("${x_data}/UCD/UnicodeData.txt")
        if (-f "${x_data}/UCD/UnicodeData.txt");

    my %packages;

    foreach my $ref (@MAPPINGS) {
        my $file    = $ref->{FN};
        my $name    = $ref->{CN};
        $name =~ s/[-:]/_/g;
        $ref->{MN}  = $name;

        if (ImportMapping($ref, "${x_mappings}/${file}", "${x_outdir}/cm${name}.h") > 0) {
            if ($x_dynamic) {
                my $pk = $ref->{PK};

                if ($x_packaged && defined $pk) {
                    if ($pk) {                  # dynamic package list (0 = implied static)
                        Module($ref, "${x_outdir}/cm${name}.c", "cm${name}.h", 1);
                        push @{$packages{$PACKAGES{$pk}}}, $ref;

                    } else {
                        Module($ref, "${x_outdir}/cm${name}.c", "cm${name}.h", -1);
                    }
                } else {                        # non-packaged
                    Module($ref, "${x_outdir}/cx${name}.c", "cm${name}.h", 0);
                }
            }
        }
    }

    if ($x_dynamic) {
        foreach (sort keys %packages) {
            Package("${x_outdir}/cx$_.c", \@{$packages{$_}});
        }
    }

    NameTable("charsetnames");
    DefinitionTable("charsettables");
    DescriptionTable("charsetdesc");
}


#   Function:           ImportMapping
#       Import the specified UNICODE mapping file and generate associated conversion tables.
#
#   Parameters:
#       ref - Table object reference.
#       source - Source file.
#       output - Output file.
#
#   Returns:
#       Character number.
#
sub
ImportMapping($$$)      #(ref, source, output)
{
    my ($ref, $source, $output) = @_;
    my $name = $ref->{CN};
    my $mname = $ref->{MN};
    my $format = $ref->{FM};
    my $codestyle = $ref->{CS};

    my @comments;
    my %cvalue256;
    my %cvalues;
    my %cdescs;
    my %uvalue256;
    my %uvalues;

    print "Importing $source ...\n";

    open(IN, "<${source}") or
        die "can't open '${source}' : $!";

    my $definition = uc($output)."_INCLUDED";
    $definition =~ s/[^A-Za-z0-9]/_/g;          # convert non alpha-numeric.
    $definition =~ s/^_+//g;                    # remove leading '_'.

    #   import
    #
    my ($cmax, $ccursor, $cont) =
        (($codestyle == CS16BIT ? 0xffff :
            ($codestyle == CS8BIT ? 0xff : 0x7f)), 0, 0, 0);

                                                # default characters 0..255
    for ($ccursor = 0; $ccursor <= 0xff && $ccursor <= $cmax; ++$ccursor) {
        if ($ccursor <= 0x7f) {
            if ($ccursor <= 0x1f) {             # C0
                $cvalues{$ccursor}      = $ccursor;
                $cdescs{$ccursor}       = $USASCII[$ccursor];

            } elsif (10 == $format || 12 == $format) {
                if ($ccursor < scalar(@USASCII)) {
                    $cvalues{$ccursor}  = $ccursor;
                    $cdescs{$ccursor}   = $USASCII[$ccursor];
                } else {
                    $cvalues{$ccursor}  = $ccursor;
                    $cdescs{$ccursor}   = "<IMPLIED ASCII>";
                }
            } else {
                $cvalues{$ccursor} = $x_undef_unicode;
                $cdescs{$ccursor} = $x_undef_name;
            }
        } else {
            $cvalues{$ccursor} = $x_undef_unicode;
            $cdescs{$ccursor} = $x_undef_name;
        }
    }

    $ccursor = 0;
    while (<IN>) {
        s/[\s\r\n]+$//;                         # trailing white-space

        my ($uchar, $cvalue, $desc) = 0;

        if (0 == $ccursor && /^#(.*)/) {        # comments
            my $comment = $1;
            if ($comment =~ /^[\s]+([a-zA-Z0-0]+:)\s*(.*)/ || ($cont && $comment)) {
                push @comments, $comment;
                $cont++;
            } else {
                $cont=0;
            }
            next;
        }

        if (1 == $format || 10 == $format) {
            #   0x<cvalue>   0x<unicode>   <description>
            #
            next if (! /^(0x[0-9a-fA-F]{2,4})[\t ]+(0x[0-9a-fA-F]{4})[\t ]+(.*)/);

            ($cvalue, $uchar, $desc) = (hex($1), hex($2), $3);

            next if ($cvalue > $cmax && $codestyle != CS16BIT);

        } elsif (2 == $format || 12 == $format) {
            #   <unicode>   <cvalue>       <description>
            #
            next if (! /^([0-9a-fA-F]{4})[\t ]+([0-9a-fA-F]{2,4})[\t ]+(.*)/);

            ($uchar, $cvalue, $desc) = (hex($1), hex($2), $3);

        } else {
            die "unknown format ${format} ....\n";
        }

        die "$source ($.): unexpected character value '$cvalue'"
            if ($cvalue > $cmax);               # range error

        if ($uchar >= 0xfffd) {
            die "$source ($.): unsupported unicode value '$uchar'"
                if ($uchar >= 0xfffe);          # 16 bit only

            $cvalues{$ccursor} = $x_undef_unicode;
            $cdescs{$ccursor} = $x_undef_name;

        } else {
            $desc = description_push($uchar, $desc, $source);

            $cvalue256{$cvalue >> 8}++;
            $cvalues{$cvalue} = $uchar;
            $cdescs{$cvalue} = $desc;

            $uvalue256{$uchar >> 8}++;
            $uvalues{$uchar} = $cvalue;
        }
        $ccursor = $cvalue;
    }
    close (IN);

    return 0
        if (! $ccursor);

    if ("US-ASCII" eq $name) {                  # default Unicode mapping
        my $IBMGRAPH = "${x_mappings}/VENDORS/MISC/IBMGRAPH.TXT";

        for (my $idx = 0; $idx <= 0xff; $idx++) {
            $USASCII[$idx] = $cdescs{$idx};
        }

        if ($x_extended) {

            foreach my $u (keys %QUOTES) {      # UNICODE and ASCII quotes
                my $a = $QUOTES{$u};
                $uvalues{$u} = $a;
                $uvalues{$a} = $a;
            }

            open(IN, "<${IBMGRAPH}") or
                die "can't open '${IBMGRAPH}' : $!";

            while (<IN>) {                      # value add to UNICODE conversion
                next if (! /^([0-9a-fA-F]{2,4})[\t ]+([0-9a-fA-F]{2,4})[\t ]+.*[\t ]+(.*)/);

                my ($uchar, $cvalue, $desc) = (hex($1), hex($2), $3);

                if ($uchar > 0xff &&
                        ($cvalue > 0x1f && $cvalue <= 0xff)) {
                    $cvalues{$cvalue}   = $uchar;
                    $cdescs{$cvalue}    = $desc;

                    $uvalue256{$uchar >> 8}++;
                    $uvalues{$uchar}    = $cvalue;
                }
            }
            close (IN);
                                                # pad definition
            for (my $idx = 0x80; $idx <= 0xff; $idx++) {
                if (!exists $cvalues{$idx}) {
                    $cvalues{$idx} = $x_undef_unicode;
                    $cdescs{$idx} = $x_undef_name;
                }
            }
            $cmax = 0xff;

        } else {
            $cvalues{0x27} = 0x27;              # ASCII quotes only
            $cvalues{0x60} = 0x60;
        }
    }

    #   export header
    #
    my $utable = "${mname}_cs2u";
    my $xtable = "${mname}_cs2x";

    open(OUT, ">${output}") or
        die "can't create '${output}' : $!";

    print OUT
        "#ifndef ${definition}\n".
        "#define ${definition}\n".
        "/*\n".
        " *           -::- AN AUTO GENERATED FILE, DO NOT EDIT -::-\n".
        " *\n".
        " *     Character conversion table, Unicode to Native ${name}.\n".
        " *\n".
        $COPYRIGHT .
        " *\n".
        " * Built:  ${x_timestamp}\n".
        " * Source: ${source}\n".
        " *\n";
    foreach (@comments) {
        print OUT " * $_\n";
    }
    print OUT
        " */\n".
        "\n".
        "#define HAVE_CHARSET_".uc($mname)."\n".
        "\n";

    #   charset-to-unicode
    #
    if ($codestyle == CS16BIT) {
        for (my $i1 = 0; $i1 <= 0xff; $i1++) {
            if (exists $cvalue256{$i1}) {
    printf OUT
        "static const uint16_t ${mname}_cu%02x[256] = {\n", $i1;
                my $i3 = ($i1 << 8);

                if ($x_labels) {                # extended format, include character descriptions
                    for (my $i2 = 0; $i2 <= 0xff; $i2++) {
                        if (exists($cvalues{$i3})) {
                            printf OUT "\t0x%04x,  /* [0x%02x] %s */\n", $cvalues{$i3}, $i3, $cdescs{$i3};
                        } else {
                            printf OUT "\t0x%04x,  /* [0x%02x] %s */\n", $x_undef_unicode, $i3, $x_undef_name;
                        }
                        ++$i3;
                    }
                } else {                        # otherwise compat format
                    print OUT "\t";
                    for (my $i2 = 0; $i2 <= 0xff; $i2++) {
                        print OUT (0 == ($i2 % 8)) ? ",\n\t" : ", "
                            if ($i2);
                        printf OUT "0x%04X", (exists $cvalues{$i3} ? $cvalues{$i3} : $x_undef_unicode);
                        ++$i3;
                    }
                    print OUT "\n";
                }
    print OUT
        "\t};\n".
        "\n";
            }
        }

    print OUT
        "static const uint16_t * const ${utable}[256] = {\n\t";

        for (my $i1 = 0; $i1 <= 0xff; $i1++) {
            print OUT (0 == ($i1 % 8)) ? ",\n\t" : ", "
                if ($i1);
            if (exists $cvalue256{$i1}) {
                printf OUT "${mname}_cu%02x", $i1;
            } else {
                print OUT "NULL";
            }
        }

    print OUT
        "\n\t};\n".
        "\n";

    } else {
    print OUT
        "static const uint16_t ${utable}[256] = {\n";

        if ($x_labels) {                        # extended format, include character descriptions
            for (my $i1 = 0; $i1 <= $cmax; $i1++) {
                printf OUT "\t0x%04x,  /* [0x%02x] %s */\n", $cvalues{$i1}, $i1, $cdescs{$i1};
            }
        } else {                                # otherwise compat format
            print OUT "\t";
            for (my $i1 = 0; $i1 <= $cmax; $i1++) {
                print OUT (0 == ($i1 % 8)) ? ",\n\t" : ", "
                    if ($i1);
                printf OUT "0x%04X", $cvalues{$i1};
            }
            print OUT "\n";
        }

    print OUT
        "\t};\n".
        "\n";
    }

    #   unicode-to-charset
    #
    for (my $i1 = 0; $i1 <= 0xff; $i1++) {
        if (exists $uvalue256{$i1}) {
    printf OUT
        "static uint16_t const ${mname}_ux%02x[256] = {\n\t", $i1;
            my $i3 = ($i1 << 8);
            for (my $i2 = 0; $i2 <= 0xff; $i2++) {
                print OUT (0 == ($i2 % 8)) ? ",\n\t" : ", "
                    if ($i2);
                printf OUT "0x%04X", (exists $uvalues{$i3} ? $uvalues{$i3} : $x_undef_external);
                ++$i3;
            }

    print OUT
        "\n".
        "\t};\n".
        "\n";
        }
    }

    print OUT
        "static const uint16_t * const ${xtable}[256] = {\n\t";

    for (my $i1 = 0; $i1 <= 0xff; $i1++) {
        print OUT (0 == ($i1 % 8)) ? ",\n\t" : ", "
            if ($i1);
        if (exists $uvalue256{$i1}) {
            printf OUT "${mname}_ux%02x", $i1;
        } else {
            print OUT "NULL";
        }
    }

    print OUT
        "\n".
        "\t};\n".
        "\n";

    #   conversion functions
    #
    my $undef_unicode  = sprintf("0x%x", $x_undef_unicode);
    my $undef_external = sprintf("0x%x", $x_undef_external);

    if ($codestyle == CS16BIT) {
        print OUT << "EOT16";

static uint32_t     /* 16bit - external/native to unicode */
${mname}_toUni(struct chartable_ccs1 *self, register uint32_t ch)
{
    const uint16_t *table;

    if (0 == (0xffff0000 & ch) &&
            0 != (table = ${utable}\[ch >> 8]) &&
                ${undef_unicode} != (ch = table[ch & 0xff])) {
        return ch;
    }
    return 0xfffd;                              /* replacement character */
}


static uint32_t     /* 16bit - unicode to external/native */
${mname}_toExt(struct chartable_ccs1 *self, register uint32_t ch)
{
    const uint16_t *table;

    if (0 == (0xffff0000 & ch) &&
            NULL != (table = ${xtable}\[ch >> 8]) &&
                ${undef_external} != (ch = table[ch & 0xff])) {
        return ch;
    }
    return '?';                                 /* replacement character */
}
EOT16

    } else {
        print OUT << "EOT8";

static uint32_t     /* 8bit - external/native to unicode */
${mname}_toUni(struct chartable_ccs1 *self, register uint32_t ch)
{
    if (0 == (0xffffff00 & ch) &&
            ${undef_unicode} != (ch = ${utable}\[ch])) {
        return ch;
    }
    return 0xfffd;                              /* replacement character */
}


static uint32_t     /* 8bit - unicode to external/native */
${mname}_toExt(struct chartable_ccs1 *self, register uint32_t ch)
{
    const uint16_t *table;

    if (0 == (0xffff0000 & ch) &&
            NULL != (table = ${xtable}\[ch >> 8]) &&
                ${undef_external} != (ch = table[ch & 0xff])) {
        return ch;
    }
    return '?';                                 /* replacement character */
}
EOT8
    }

    print OUT
        "\n".
        "#endif /*${definition}*/\n";

    $ref->{MC} = $cmax;
    $ref->{SZ} = $ccursor;

    close(IN);
    close(OUT);
    return $ccursor;
}


sub
Package($$)
{
    my ($output, $packages) = @_;
    my $count = scalar @$packages;

    open(OUT, ">${output}") or
        die "can't create '${output}' : $!";

    print OUT           # header
        "/*\n".
        " *           -::- AN AUTO GENERATED FILE, DO NOT EDIT -::-\n".
        " *\n".
        " *     Character conversion module, Unicode to Native Package.\n".
        " *\n".
        $COPYRIGHT .
        " *\n".
        " * Built: ${x_timestamp}\n".
        " *\n".
        " */\n".
        "\n".
        "#include <chartable_module.h>\n";

                        # module definitions
    foreach my $ref (@$packages) {
    print OUT
        "#include \"cm".$ref->{MN}.".c\"\n";
    }

    print OUT           # module list
        "\n".
        "static const\n".
        "struct chartable_module * const chartable_modules[] = {\n";
    foreach my $ref (@$packages) {
    print OUT
        "    &".$ref->{MN}."_chartable_module,\n";
    }
    print OUT
        "    NULL\n".
        "    };\n".
        "\n";

    print OUT           # package definition
        "MODULE_LINKAGE\n".
        "const struct chartable_package chartable_module_package[] = {\n".
        "    CHARTABLE_PACKAGE_MAGIC,\n".
        "    $count,\n".
        "    chartable_modules\n".
        "    };\n".
        "\n".
        "/*end*/\n";
    close (OUT);
}


sub
Module($$$$)
{
    my ($ref, $output, $header, $pk) = @_;
    my $name = $ref->{CN};
    my $mname = $ref->{MN};
    my $package = ($pk ? "${mname}_" : '');

    open(OUT, ">${output}") or
        die "can't create '${output}' : $!";

    print OUT           # header
        "/*\n".
        " *           -::- AN AUTO GENERATED FILE, DO NOT EDIT -::-\n".
        " *\n".
        " *     Character conversion module, Unicode to Native ${name}.\n".
        " *\n".
        $COPYRIGHT .
        " *\n".
        " * Built: ${x_timestamp}\n".
        " */\n".
        "\n".
        "#include <chartable_module.h>\n".
        "#include \"${header}\"\n".
        "\n";

    print OUT           # list of supported character-set
        "static const char *${mname}_names[] = {\n".
        "    \"${name}\",\n".
        "    NULL\n".
        "    };\n".
        "\n";

    print OUT           # module description
        "static struct chartable_ccs1 ${mname}_desc = {\n".
        "    ${mname}_names,\n".
        "    $ref->{CS}, $ref->{MC}, $ref->{SZ},\n".
        "    ${mname}_toUni,\n".
        "    ${mname}_toExt\n".
        "    };\n".
        "\n";

                        # module definition
    print OUT "MODULE_LINKAGE\n"
        if ($pk >= 0);
    print OUT
        "const struct chartable_module ${package}chartable_module = {\n".
        "    CHARTABLE_MODULE_MAGIC,                        /* structure magic */\n".
        "    CHARTABLE_SIGNATURE(CHARTABLE_CCS, 0x100),     /* class and descriptor version */\n".
        "    (void *) &${mname}_desc,                       /* descriptor definition */\n".
        "    NULL\n".
        "    };\n".
        "\n";

#   if (! $package) {
#       print OUT
#           "#if defined(WIN32) && defined(MODULE_LOADABLE)\n".
#           "#define  WINDOWS_MEAN_AND_LEAN\n".
#           "#include <windows.h>\n".
#           "BOOL WINAPI\n".
#           "DllMain(HINSTANCE hInst, DWORD reason, LPVOID reserved) {\n".
#           "    return TRUE;\n".
#           "}\n".
#           "#endif\n".
#           "\n";
#   }

    print OUT
        "/*end*/\n";

    close (OUT);
}


#   Function:           ImportUnicodeData
#       xxx
#
#   Parameters:
#       file - Filename.
#
#   Returns:
#       nothing.
#
sub
ImportUnicodeData($)    #(file)
{
    my ($file) = @_;

    print "Importing $file ...\n";

    open(IN, "<${file}") or
        die "can't open '${file}' : $!";

    while (<IN>) {
        my ($uchar, $desc, $bits) = split(/;/, $_);

        if ($desc =~ /^<(.*)>/) {               # region start/end
            # example/
            #   <CJK Ideograph Extension A, First>
            #   <CJK Ideograph Extension A, Last>
            #
            if ("control" ne $1) {
                print("\tskipping $desc\n");
                next;
            }
        }

        $uchar = hex($uchar);
        die "$file ($.): invalid unicode value '$uchar'"
            if ($uchar < 0 || $uchar > 0x10ffff);
        description_push($uchar, $desc, $file);
    }

    close(IN);
}


#   Function:           description_push
#       xxx
#
#   Parameters:
#       uchar -             Unicode character code.
#       desc -              Description string.
#
#   Returns:
#       nothing.
#
sub
description_push($$)    #(uchar, desc, source)
{
    my ($uchar, $desc, $source) = @_;

    $desc =~ s/^[\s#;-]+//;                 # leading
    $desc =~ s/#[^#]+$//g;                  # trailing
    $desc =~ s/[\t ]+$//;                   # trailing
    $desc =~ s/[\t ]+/ /g;                  # compress
    $desc = uc($desc);

    if (! exists $description_index{$uchar}) {
        #   new value/
        #       asign description record
        #
        if (! exists $description_text{$desc}) {
            #   new description/
            #       assign unique description index
            #
            $description_text{$desc} = scalar @description_strings;
            push @description_strings, $desc;
            push @description_hits, 0;
        }
        my $sidx = $description_text{$desc};
        $description_index{$uchar} = $sidx;
        $description_unicode[$sidx] = $uchar;
        $description_hits[$sidx]++;

    } else {
        if ($x_warnings) {                  # description diff
            my $t_desc = $description_strings[$description_index{$uchar}];
            printf "WARNING: $source ($.): ${t_desc} != ${desc}\n"
                if ($t_desc ne $desc);
        }
    }
    return $desc;
}


#   Function:           NameTable
#       Generate character-set name table.
#
#   Parameters:
#       tablename - Table name.
#
#   Return:
#       nothing.
#
sub
NameTable($)            #(tablename)
{
    my ($tablename) = @_;

    open(OUT, ">${tablename}.h") or
        die "can't create '${tablename}.h' : $!";

    printf OUT
        "/*\n".
        " *           -::- AN AUTO GENERATED FILE, DO NOT EDIT -::-\n".
        " *\n".
        $COPYRIGHT .
        " *\n".
        " * Built: ${x_timestamp}\n".
        " * Table: ${tablename}\n".
        " */\n".
        "\n".
        "static const char * const ${tablename}[] = {\n";

    foreach (@MAPPINGS) {
        my $name    = $_->{CN};
    printf OUT
        "    \"${name}\",\n";
    }

    print OUT
        "    NULL\n".
        "    };\n";

    close(OUT);
}


#   Function:           DefinitionTable
#       Generate character-set definition table.
#
#   Parameters:
#       tablename - none.
#
#   Return:
#       nothing.
#
sub
DefinitionTable($)      #(tablename)
{
    my ($tablename, $charset) = @_;

    open(OUT, ">${tablename}.h") or
        die "can't create '${tablename}.h' : $!";

    # header
    print OUT
        "/*\n".
        " *         -::- AN AUTO GENERATED FILE, DO NOT EDIT -::-\n".
        " *\n".
        $COPYRIGHT .
        " *\n".
        " * Built: ${x_timestamp}\n".
        " * Table: ${tablename}\n".
        " */\n".
        "\n".
        "#if defined(_MSC_VER)\n".
        "#include <unistd.h>                /* Visual Studio */\n".
        "#else\n".
        "#include <stdint.h>\n".
        "#endif\n".
        "\n";

    print OUT
        "#if !defined(CHARTABLE_STATIC)\n".
        "#define CHARTABLE_DYNAMIC          /* implied dynamic configurationn */\n".
        "#endif\n\n"
            if ($x_dynamic);

    # conversion tables
    foreach (@MAPPINGS) {
        my $name    = $_->{CN};
        my $mname   = $_->{MN};
        my $pk      = $_->{PK};
        my $cs      = $_->{CS};
        my $csname  = (CS7BIT == $cs ? "7BIT" : (CS8BIT == $cs ? "8BIT" : "16BIT"));

    print OUT
        "#if !defined(CHARTABLE_DYNAMIC) && defined(CHARTABLES_${csname})\n"
            if (!defined $pk || $pk);           # implied static
    print OUT
        "#include \"${x_outdir}/cm${mname}.c\"\n";
    print OUT
        "#endif\n"
            if (!defined $pk || $pk);
    }

    # table lookup
    print OUT
        "\n".
        "static struct ${tablename} {\n".
        "    const char *    name;\n".
        "    uint32_t        namelen;\n".
        "    uint32_t        flag;\n".
        "#define CHARTABLE_7BIT      ". CS7BIT. "\n".
        "#define CHARTABLE_8BIT      ". CS8BIT. "\n".
        "#define CHARTABLE_16BIT     ". CS16BIT. "\n".
        "#define CHARTABLE_PACKAGE   ". CSPACKAGED. "\n".
        "    uint32_t        codepage;\n".
        "    uint32_t        count;\n".
        "    const char *    container;\n".
        "    const struct chartable_module *module;\n".
        "} ${tablename}[] = {\n".
        "\n".
        "#if defined(CHARTABLE_DYNAMIC)\n".
        "#define __CVFN(__fn)        NULL\n".
        "#else\n".
        "#define __CVFN(__fn)        __fn\n".
        "#endif\n".
        "#define __CVST(__fn)        __fn\n".
        "\n";

    my $namemax = 8;

    foreach (@MAPPINGS) {
        my $name    = $_->{CN};
        $namemax    = length($name)
            if (length($name) > $namemax);
    }

    foreach my $ref (@MAPPINGS) {
        my $name    = $ref->{CN};
        my $mname   = $ref->{MN};
        my $cname   = $mname;
        my $cs      = $ref->{CS};
        my $csname  = (CS7BIT == $cs ? '7BIT' : (CS8BIT == $cs ? '8BIT' : '16BIT'));
        my $func    = '__CVFN';

        if ($x_dynamic) {
            my $pk = $ref->{PK};

            if (defined $pk) {                  # packaged character-tables
                if (!$pk) {
                    $func = '__CVST';           # implied static
                } elsif ($x_packaged) {
                    $cname = $PACKAGES{$pk};
                    $cs |= CSPACKAGED;
                }
            }
        }

    printf OUT
        "#if defined(CHARTABLES_${csname})\n".
        "    { \"${name}\",%*s %3d, 0x%02x, %6d, %6d, \"%s\",%*s %s },\n".
        "#endif\n",
            $namemax - length($name), "",
                length($name), $cs, $ref->{CP}, $ref->{SZ},
            $cname, 15 - length($cname), "",
                "${func}(&${mname}_chartable_module)";
    }

    print OUT
        "\n".
        "#undef  __CVFN\n".
        "    };\n".
        "\n".
        "/*end*/\n";

    close(OUT);
}


#   Function:           DescriptionTable
#       Generate description table.
#
#   Parameters:
#       none.
#
#   Return:
#       nothing.
#
sub
DescriptionTable($)     #(tablename)
{
    my ($tablename) = @_;
    my $wordhash0;
    my $wordhits0;
    my @desccooked;

    print "building ${tablename}.h ..\n";

    if ($x_crunch) {
        #
        #   crunch sentences into a hash of words, hashing high-frequency fragments/
        #       result around 150k, with a raw gzip of the same being ~130k yet is
        #       very simple/fast to uncrunch in comparision the raw text being
        #       aroung 1.2Mb.
        #
        my $longest = 1;

        print "crunching\n";

        for (my $sidx = 0; $sidx < scalar @description_strings; ++$sidx) {
            my $desc = $description_strings[$sidx];
            my $hex  = sprintf("%X", $description_unicode[$sidx]);

            $desc =~ s/-/ -/g;                  # seperate words
            $description_strings[$sidx] = $desc;

            my @descparts = split(/[ ]+/, $desc);
            my $length = scalar @descparts;

            $longest = $length                  # longest segment
                if $length > $longest;

            if ($desc =~ /^</) {
                die "bad special '$desc'\n"
                    if ($desc =~ /[^<>A-Za-z0-9 ,.()=-]/);
            } else {
                die "bad desc '$desc'\n"
                    if ($desc =~ /[^<>A-Z0-9 ,.()=-]/);
            }

            if ($desc =~ /${hex}/i) {           # replace inline character-value with '#'
                $desc =~ s/-${hex}/-#/i;
                $description_strings[$sidx] = $desc;
            }
        }

        my $last = $longest--;
        for (my $phase = 1; $phase <= $last; ++$phase) {
            my $wordnext = 0;
            my %wordhits;
            my %wordhash;

            --$longest;

            for (my $sidx = 0; $sidx < scalar @description_strings; ++$sidx) {
                my $desc  = $description_strings[$sidx];
                my @words = split(/[ ]+/, $desc);
                my $wcnt  = scalar @words;
                my $widx  = 0;

                for (my $t_widx = $wcnt; $t_widx > 0;) {
                    my $t_word = "";

                    for (my $w = 0; $w < $t_widx; ++$w) {
                        $t_word .= " " if ($t_word);
                        $t_word .= $words[$w];
                    }

                    if ($t_widx-- == $longest ||
                            exists $$wordhits0{$t_word} && $$wordhits0{$t_word} > 1) {
                        $words[$t_widx] = $t_word;
                        $widx = $t_widx;
                        last;
                    }
                }

                # hash word and reference counts
                if ($last == $phase) {
                    my $delimiter = 0;
                    my $new = "";

                    for (; $widx < $wcnt; ++$widx) {
                        my $word = $words[$widx];

                        if (exists $$wordhash0{$word}) {
                            $new .= "\"" . UTF8(0x60 + $$wordhash0{$word}) . "\"";
                            $delimiter = 0;
                        } else {
                            $new .= "\"";
                            $new .= ' ' if ($delimiter);
                            $new .= $word;
                            $new .= "\"";
                            $delimiter = 1;
                        }
                    }
                    $new =~ s/ \-/-/g;          # join words
                    $desccooked[$sidx] = $new;

                } else {
                    for (; $widx < $wcnt; ++$widx) {
                        my $word = $words[$widx];

                        if (exists $wordhash{$word}) {
                            ++$wordhits{$word};
                        } elsif (length($word) >= 3) {
                            $wordhash{$word} = $wordnext++;
                            $wordhits{$word} = 1;
                        }
                    }
                }
            }

            if ($phase < $last) {
                my $idx = 0;                    # remove low frequency words
                foreach my $key (sort { $wordhits{$b} <=> $wordhits{$a} } keys %wordhits) {
                    my $hits = $wordhits{$key};
                    if ($hits <= 1 ||
                            (2 == $hits && $idx >= 0x750 && length($key) <= 3)) {
                        delete $wordhash{$key};
                        delete $wordhits{$key};
                    } else {
                        ++$idx;
                    }
                }

                $idx = 0;                       # reorder, highest -> lowest hits
                foreach my $key (sort { $wordhits{$b} <=> $wordhits{$a} } keys %wordhits) {
                    $wordhash{$key} = $idx++;
                }

                $wordhits0 = \%wordhits;
                $wordhash0 = \%wordhash;
            }
            printf "." x $phase . "\r";
        }
    }
    print "\n";

    print "generating ${tablename}.h ..\n";

    open(OUT, ">${tablename}.h") or
        die "can't create '${tablename}.h' : $!";

    print OUT
        "/* -*- mode: c; tabs: 8; indent-width: 4; -*-\n".
        " *\n".
        " *           -::- AN AUTO GENERATED FILE, DO NOT EDIT -::-\n".
        " *\n".
        " * Built: ${x_timestamp}\n".
        " */\n".
        "\n";

    # sentences
    #
    for (my $sidx = 0; $sidx < scalar @description_strings; ++$sidx) {
        if ($description_hits[$sidx] > 1) {
            my $desc = $description_strings[$sidx];

            if ($x_crunch) {
    printf OUT
        ("\t" x 9) . "/* $desc */\n";
    printf OUT
        "static const char s%05x[] = %s;\n",
                $sidx, $desccooked[$sidx];
            } else {
    printf OUT
        "static const char s%05x[] = \"%s\";\n",
                $sidx, $desc;
            }
        }
    }

    # word table
    #
    print OUT
        "\n".
        "static const char * x_${tablename}_words[] = {\n";

    foreach my $key (sort { $$wordhash0{$a} <=> $$wordhash0{$b} } keys %$wordhash0) {
        my $desc = $key;
        $desc =~ s/ -/-/g;                      # join words
    printf OUT
        "    /* %4x, %5d */ \"%s\",\n", $$wordhash0{$key}, $$wordhits0{$key}, $desc;
    }

    print OUT
        "    NULL\n",
        "    };\n".
        "\n";

    # character table
    #
    print OUT
        "\n".
        "static const char * x_${tablename}_characters[] = {\n";

    my $hits_nval = 0;
    my $hits_sidx = -1;
##  my $hits_run = 0;
    my $idx = 0;

    foreach my $val (sort { $a <=> $b } keys %description_index) {
        my $sidx = $description_index{$val};
        my $hits = $description_hits[$sidx];

        # multiple instances, replace with common definition
        if ($hits > 1) {
##          if ($hits_sidx == $sidx && $hits_nval == $val) {
##              $hits_nval = $val + 1;
##              ++$hits_run;
##              next;
##          }

    printf OUT
        "    /*[%05x] %05x*/ s%05x,\n", $idx++, $val, $sidx;
            $hits_nval = $val + 1;
            $hits_sidx = $sidx;
##          $hits_run = 0;
            next;
        }

##      if ($hits_run) {
##  printf OUT
##      "    /* : : */\n".
##      "    /*%05x*/\n", $hits_nval - 1;
##          $hits_run = 0;
##      }

        # single instance
        my $desc = $description_strings[$sidx];
        my $hex  = sprintf("%X", $val);

        $desc =~ s/ -/-/g;                      # join words
        $desc =~ s/-#/-${hex}/;                 # inline character-value

        if ($x_crunch) {
            my $cook = $desccooked[$sidx];

            if (length($cook) > 64) {
    print OUT
        ("\t" x 9) . "/* $desc */\n";
            }
    printf OUT
        "    /*[%05x] %05x*/ %s,", $idx++, $val, $cook;
            if (length($cook) <= 64) {
    print OUT
        ("\t" x (((64 - length($cook))/8) + 1)) . "/* $desc */\n";
            } else {
    print OUT
        "\n";
            }
        } else {
    printf OUT
        "    /*[%05x] %05x*/ \"%s\",\n", $idx++, $val, $desc;
        }
    }

    print OUT
        "    NULL\n".
        "    };\n";

    # lookup table
    #
    print OUT
        "\n".
        "static const struct charsetdesc {\n".
        "    uint32_t   base;\n".
        "    uint32_t   offset;\n".
        "    uint32_t   count;\n".
        "\n".
        "} x_${tablename}_lookup[] = {\n";

    my ($last, $count, $offset) = (0, 0, 0);
    foreach my $val (sort { $a <=> $b } keys %description_index) {
        if (($last + 1) == $val) {
            ++$count;
        } else {
    printf OUT
        "%-5d },\n", $count
                if ($count);
    printf OUT
        "    { 0x%05x,\t0x%05x,\t", $val, $offset;
            $count = 1;
        }
        $last = $val;
        ++$offset;
    }
    printf OUT "%-5d },\n", ($count - 1)
        if ($count);
    print OUT
        "};\n".
        "\n".
        "/*end*/\n";

    close(OUT);
}


#   Function:           UTF8
#       Character value to hex encoded UTF8 string.
#
#   Parameters:
#       unichar - Unicode character value.
#
#   Return:
#       UTF8 encoded value.
#
sub
UTF8($)                 #(character)
{
    my ($unichar) = @_;
    my $hex = "";

    if ($unichar < 0x80) {
        $hex .= sprintf("%c", $unichar);

    } elsif ($unichar < 0x800) {
        $hex .= sprintf("\\x%02x", 0xC0 |  ($unichar >> 6));
        $hex .= sprintf("\\x%02x", 0x80 |  ($unichar &  0x3F));

    } elsif ($unichar < 0x10000) {
        $hex .= sprintf("\\x%02x", 0xE0 |  ($unichar >> 12));
        $hex .= sprintf("\\x%02x", 0x80 | (($unichar >> 6) & 0x3F));
        $hex .= sprintf("\\x%02x", 0x80 |  ($unichar &  0x3F));

    } elsif ($unichar < 0x200000) {
        $hex .= sprintf("\\x%02x", 0xF0 |  ($unichar >> 18));
        $hex .= sprintf("\\x%02x", 0x80 | (($unichar >> 12) & 0x3F));
        $hex .= sprintf("\\x%02x", 0x80 | (($unichar >> 6)  & 0x3F));
        $hex .= sprintf("\\x%02x", 0x80 |  ($unichar &  0x3F));

    } elsif ($unichar < 0x4000000) {
        $hex .= sprintf("\\x%02x", 0xF8 |  ($unichar >> 24));
        $hex .= sprintf("\\x%02x", 0x80 | (($unichar >> 18) & 0x3F));
        $hex .= sprintf("\\x%02x", 0x80 | (($unichar >> 12) & 0x3F));
        $hex .= sprintf("\\x%02x", 0x80 | (($unichar >> 6)  & 0x3F));
        $hex .= sprintf("\\x%02x", 0x80 |  ($unichar &  0x3F));

    } elsif ($unichar < 0x80000000) {
        $hex .= sprintf("\\x%02x", 0xFC |  ($unichar >> 30));
        $hex .= sprintf("\\x%02x", 0x80 | (($unichar >> 24) & 0x3F));
        $hex .= sprintf("\\x%02x", 0x80 | (($unichar >> 18) & 0x3F));
        $hex .= sprintf("\\x%02x", 0x80 | (($unichar >> 12) & 0x3F));
        $hex .= sprintf("\\x%02x", 0x80 | (($unichar >> 6)  & 0x3F));
        $hex .= sprintf("\\x%02x", 0x80 |  ($unichar &  0x3F));
    }
    return $hex;
}


#   Function:           Unicode2UTF8
#       Unicode value toUTF8.
#
#   Parameters:
#       unichar - Unicode character value.
#
#   Return:
#       UTF8 encoded value.
#
sub
Unicode2UTF8($)         #(character)
{
    my ($unichar) = @_;
    my $hex = "0x";

    if ($unichar < 0x80) {
        $hex .= sprintf("%08x", $unichar);

    } elsif ($unichar < 0x800) {
        $hex .= sprintf("%06x", 0xC0 |  ($unichar >> 6));
        $hex .= sprintf("%02x", 0x80 |  ($unichar &  0x3F));

    } elsif ($unichar < 0x10000) {
        $hex .= sprintf("%04x", 0xE0 |  ($unichar >> 12));
        $hex .= sprintf("%02x", 0x80 | (($unichar >> 6) & 0x3F));
        $hex .= sprintf("%02x", 0x80 |  ($unichar &  0x3F));

    } elsif ($unichar < 0x200000) {
        $hex .= sprintf("%02x", 0xF0 |  ($unichar >> 18));
        $hex .= sprintf("%02x", 0x80 | (($unichar >> 12) & 0x3F));
        $hex .= sprintf("%02x", 0x80 | (($unichar >> 6)  & 0x3F));
        $hex .= sprintf("%02x", 0x80 |  ($unichar &  0x3F));

    } elsif ($unichar < 0x4000000) {
        $hex .= sprintf("%02x", 0xF8 |  ($unichar >> 24));
        $hex .= sprintf("%02x", 0x80 | (($unichar >> 18) & 0x3F));
        $hex .= sprintf("%02x", 0x80 | (($unichar >> 12) & 0x3F));
        $hex .= sprintf("%02x", 0x80 | (($unichar >> 6)  & 0x3F));
        $hex .= sprintf("%02x", 0x80 |  ($unichar &  0x3F));

    } elsif ($unichar < 0x80000000) {
        $hex .= sprintf("%02x", 0xFC |  ($unichar >> 30));
        $hex .= sprintf("%02x", 0x80 | (($unichar >> 24) & 0x3F));
        $hex .= sprintf("%02x", 0x80 | (($unichar >> 18) & 0x3F));
        $hex .= sprintf("%02x", 0x80 | (($unichar >> 12) & 0x3F));
        $hex .= sprintf("%02x", 0x80 | (($unichar >> 6)  & 0x3F));
        $hex .= sprintf("%02x", 0x80 |  ($unichar &  0x3F));
    }
    return $hex;
}


#   Function:           Usage
#       Makelib command line usage.
#
#   Parameters:
#       [msg] - Optional message.
#
#   Returns:
#       nothing.
#
sub
Usage                   #([message])
{
    my $msg = @_;

    print "makecharset $msg\n\n" if ($msg);
    print << "USAGE";
usage: perl makechartable.pl [options]

options:
    -help                   help.

    --warnings, -w          enable warnings.

    --crunch                crunch description table.
    --dynamic               dynamic loader support.

    --labels                generate verbose character descriptions/comments.
    --extended              extended US-ASCII conversion table.

USAGE
    exit(42);
}

#end
