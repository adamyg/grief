/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: view.cr,v 1.12 2014/10/27 23:28:30 ayoung Exp $
 * Buffer view manipulation.
 *
 *
 */

#include "grief.h"

/*
 *  hex view -- all characters in hexadecimal.
 */
static list             hex_view = {
    "00 ",      "01 ",      "02 ",      "03 ",      "04 ",      "05 ",      "06 ",      "07 ",
    "08 ",      "09 ",      "0a ",      "0b ",      "0c ",      "0d ",      "0e ",      "0f ",
    "10 ",      "11 ",      "12 ",      "13 ",      "14 ",      "15 ",      "16 ",      "17 ",
    "18 ",      "19 ",      "1a ",      "1b ",      "1c ",      "1d ",      "1e ",      "1f ",
    "20 ",      "21 ",      "22 ",      "23 ",      "24 ",      "25 ",      "26 ",      "27 ",
    "28 ",      "29 ",      "2a ",      "2b ",      "2c ",      "2d ",      "2e ",      "2f ",
    "30 ",      "31 ",      "32 ",      "33 ",      "34 ",      "35 ",      "36 ",      "37 ",
    "38 ",      "39 ",      "3a ",      "3b ",      "3c ",      "3d ",      "3e ",      "3f ",
    "40 ",      "41 ",      "42 ",      "43 ",      "44 ",      "45 ",      "46 ",      "47 ",
    "48 ",      "49 ",      "4a ",      "4b ",      "4c ",      "4d ",      "4e ",      "4f ",
    "50 ",      "51 ",      "52 ",      "53 ",      "54 ",      "55 ",      "56 ",      "57 ",
    "58 ",      "59 ",      "5a ",      "5b ",      "5c ",      "5d ",      "5e ",      "5f ",
    "60 ",      "61 ",      "62 ",      "63 ",      "64 ",      "65 ",      "66 ",      "67 ",
    "68 ",      "69 ",      "6a ",      "6b ",      "6c ",      "6d ",      "6e ",      "6f ",
    "70 ",      "71 ",      "72 ",      "73 ",      "74 ",      "75 ",      "76 ",      "77 ",
    "78 ",      "79 ",      "7a ",      "7b ",      "7c ",      "7d ",      "7e ",      "7f ",
    "80 ",      "81 ",      "82 ",      "83 ",      "84 ",      "85 ",      "86 ",      "87 ",
    "88 ",      "89 ",      "8a ",      "8b ",      "8c ",      "8d ",      "8e ",      "8f ",
    "90 ",      "91 ",      "92 ",      "93 ",      "94 ",      "95 ",      "96 ",      "97 ",
    "98 ",      "99 ",      "9a ",      "9b ",      "9c ",      "9d ",      "9e ",      "9f ",
    "a0 ",      "a1 ",      "a2 ",      "a3 ",      "a4 ",      "a5 ",      "a6 ",      "a7 ",
    "a8 ",      "a9 ",      "aa ",      "ab ",      "ac ",      "ad ",      "ae ",      "af ",
    "b0 ",      "b1 ",      "b2 ",      "b3 ",      "b4 ",      "b5 ",      "b6 ",      "b7 ",
    "b8 ",      "b9 ",      "ba ",      "bb ",      "bc ",      "bd ",      "be ",      "bf ",
    "c0 ",      "c1 ",      "c2 ",      "c3 ",      "c4 ",      "c5 ",      "c6 ",      "c7 ",
    "c8 ",      "c9 ",      "ca ",      "cb ",      "cc ",      "cd ",      "ce ",      "cf ",
    "d0 ",      "d1 ",      "d2 ",      "d3 ",      "d4 ",      "d5 ",      "d6 ",      "d7 ",
    "d8 ",      "d9 ",      "da ",      "db ",      "dc ",      "dd ",      "de ",      "df ",
    "e0 ",      "e1 ",      "e2 ",      "e3 ",      "e4 ",      "e5 ",      "e6 ",      "e7 ",
    "e8 ",      "e9 ",      "ea ",      "eb ",      "ec ",      "ed ",      "ee ",      "ef ",
    "f0 ",      "f1 ",      "f2 ",      "f3 ",      "f4 ",      "f5 ",      "f6 ",      "f7 ",
    "f8 ",      "f9 ",      "fa ",      "fb ",      "fc ",      "fd ",      "fe ",      "ff "
    };


/*
 *  HEX & ASCII mode ---
 *      Use printable ascii where we can and hex other places.
 *      All characters being the same width (similar to od -c).
 */
static list             hexasc_view = {
    "00 ",      "01 ",      "02 ",      "03 ",      "04 ",      "05 ",      "06 ",      "07 ",
    "08 ",      "09 ",      "0a ",      "0b ",      "0c ",      "0d ",      "0e ",      "0f ",
    "10 ",      "11 ",      "12 ",      "13 ",      "14 ",      "15 ",      "16 ",      "17 ",
    "18 ",      "19 ",      "1a ",      "1b ",      "1c ",      "1d ",      "1e ",      "1f ",
    "   ",      "!  ",      "\"  ",     "#  ",      "$  ",      "%  ",      "&  ",      "'  ",
    "(  ",      ")  ",      "*  ",      "+  ",      ",  ",      "-  ",      ".  ",      "/  ",
    "0  ",      "1  ",      "2  ",      "3  ",      "4  ",      "5  ",      "6  ",      "7  ",
    "8  ",      "9  ",      ":  ",      ";  ",      "<  ",      "=  ",      ">  ",      "?  ",
    "@  ",      "A  ",      "B  ",      "C  ",      "D  ",      "E  ",      "F  ",      "G  ",
    "H  ",      "I  ",      "J  ",      "K  ",      "L  ",      "M  ",      "N  ",      "O  ",
    "P  ",      "Q  ",      "R  ",      "S  ",      "T  ",      "U  ",      "V  ",      "W  ",
    "X  ",      "Y  ",      "Z  ",      "[  ",      "\\  ",     "]  ",      "^  ",      "_  ",
    "`  ",      "a  ",      "b  ",      "c  ",      "d  ",      "e  ",      "f  ",      "g  ",
    "h  ",      "i  ",      "j  ",      "k  ",      "l  ",      "m  ",      "n  ",      "o  ",
    "p  ",      "q  ",      "r  ",      "s  ",      "t  ",      "u  ",      "v  ",      "w  ",
    "x  ",      "y  ",      "z  ",      "{  ",      "|  ",      "}  ",      "~  ",      "7f ",
    "80 ",      "81 ",      "82 ",      "83 ",      "84 ",      "85 ",      "86 ",      "87 ",
    "88 ",      "89 ",      "8a ",      "8b ",      "8c ",      "8d ",      "8e ",      "8f ",
    "90 ",      "91 ",      "92 ",      "93 ",      "94 ",      "95 ",      "96 ",      "97 ",
    "98 ",      "99 ",      "9a ",      "9b ",      "9c ",      "9d ",      "9e ",      "9f ",
    "a0 ",      "a1 ",      "a2 ",      "a3 ",      "a4 ",      "a5 ",      "a6 ",      "a7 ",
    "a8 ",      "a9 ",      "aa ",      "ab ",      "ac ",      "ad ",      "ae ",      "af ",
    "b0 ",      "b1 ",      "b2 ",      "b3 ",      "b4 ",      "b5 ",      "b6 ",      "b7 ",
    "b8 ",      "b9 ",      "ba ",      "bb ",      "bc ",      "bd ",      "be ",      "bf ",
    "c0 ",      "c1 ",      "c2 ",      "c3 ",      "c4 ",      "c5 ",      "c6 ",      "c7 ",
    "c8 ",      "c9 ",      "ca ",      "cb ",      "cc ",      "cd ",      "ce ",      "cf ",
    "d0 ",      "d1 ",      "d2 ",      "d3 ",      "d4 ",      "d5 ",      "d6 ",      "d7 ",
    "d8 ",      "d9 ",      "da ",      "db ",      "dc ",      "dd ",      "de ",      "df ",
    "e0 ",      "e1 ",      "e2 ",      "e3 ",      "e4 ",      "e5 ",      "e6 ",      "e7 ",
    "e8 ",      "e9 ",      "ea ",      "eb ",      "ec ",      "ed ",      "ee ",      "ef ",
    "f0 ",      "f1 ",      "f2 ",      "f3 ",      "f4 ",      "f5 ",      "f6 ",      "f7 ",
    "f8 ",      "f9 ",      "fa ",      "fb ",      "fc ",      "fd ",      "fe ",      "ff "
    };


/*
 *  Fixed width ascii view  (0x80-0x7F).
 */
static list             fw_view = {
    ".",        ".",        ".",        ".",        ".",        ".",        ".",        ".",
    ".",        ".",        ".",        ".",        ".",        ".",        ".",        ".",
    ".",        ".",        ".",        ".",        ".",        ".",        ".",        ".",
    ".",        ".",        ".",        ".",        ".",        ".",        ".",        ".",
    " ",        "!",        "\"",       "#",        "$",        "%",        "&",        "'",
    "(",        ")",        "*",        "+",        ",",        "-",        ".",        "/",
    "0",        "1",        "2",        "3",        "4",        "5",        "6",        "7",
    "8",        "9",        ":",        ";",        "<",        "=",        ">",        "?",
    "@",        "A",        "B",        "C",        "D",        "E",        "F",        "G",
    "H",        "I",        "J",        "K",        "L",        "M",        "N",        "O",
    "P",        "Q",        "R",        "S",        "T",        "U",        "V",        "W",
    "X",        "Y",        "Z",        "[",        "\\",       "]",        "^",        "_",
    "`",        "a",        "b",        "c",        "d",        "e",        "f",        "g",
    "h",        "i",        "j",        "k",        "l",        "m",        "n",        "o",
    "p",        "q",        "r",        "s",        "t",        "u",        "v",        "w",
    "x",        "y",        "z",        "{",        "|",        "}",        "~",        ".",
    ".",        ".",        ".",        ".",        ".",        ".",        ".",        ".",
    ".",        ".",        ".",        ".",        ".",        ".",        ".",        ".",
    ".",        ".",        ".",        ".",        ".",        ".",        ".",        ".",
    ".",        ".",        ".",        ".",        ".",        ".",        ".",        ".",
    ".",        ".",        ".",        ".",        ".",        ".",        ".",        ".",
    ".",        ".",        ".",        ".",        ".",        ".",        ".",        ".",
    ".",        ".",        ".",        ".",        ".",        ".",        ".",        ".",
    ".",        ".",        ".",        ".",        ".",        ".",        ".",        ".",
    ".",        ".",        ".",        ".",        ".",        ".",        ".",        ".",
    ".",        ".",        ".",        ".",        ".",        ".",        ".",        ".",
    ".",        ".",        ".",        ".",        ".",        ".",        ".",        ".",
    ".",        ".",        ".",        ".",        ".",        ".",        ".",        ".",
    ".",        ".",        ".",        ".",        ".",        ".",        ".",        ".",
    ".",        ".",        ".",        ".",        ".",        ".",        ".",        ".",
    ".",        ".",        ".",        ".",        ".",        ".",        ".",        ".",
    ".",        ".",        ".",        ".",        ".",        ".",        ".",        ".",
    };


/*
 *  EBCDIC - Extend Binary Coded Decimal Interchange Code (IBM).
 */
static list             ebcdic_view = {
    "<NUL>",    "<SOH>",    "<STX>",    "<ETX>",    "<PF>",     "<HT>",     "<LC>",     "<DEL>",
    "",         "",         "<SMM>",    "<VT>",     "<FF>",     "<CR>",     "<SO>",     "<SI>",
    "<DLE>",    "<DC1>",    "<DC2>",    "<TM>",     "<RES>",    "<NL>",     "<BS>",     "<IL>",
    "<CAN>",    "<EM>",     "<CC>",     "<CU1>",    "<IFS>",    "<IGS>",    "<IRS>",    "<IUS>",
    "<DS>",     "<SOS>",    "<FS>",     "",         "<BYP>",    "<LF>",     "<ETB>",    "",
    "",         "<SM>",     "<CU2>",    "",         "<ENQ>",    "<ACK>",    "<BEL>",    "",
    "",         "",         "<SYN>",    "",         "<PN>",     "<RS>",     "<UC>",     "<EOT>",
    "",         "",         "",         "<CU3>",    "<DC4>",    "<NAK>",    "",         "<SUB>",
    " ",        "",         "",         "",         "",         "",         "",         "",
    "",         "",         "<CENT>",   ".",        "<",        "(",        "+",        "|",
    "&",        "",         "",         "",         "",         "",         "",         "",
    "",         "",         "!",        "$",        "*",        ")",        ";",        "5f ",
    "-",        "/",        "",         "",         "",         "",         "",         "",
    "",         "",         ",",        "%",        "_",        ">",        "?",        "",
    "",         "",         "",         "",         "",         "",         "",         "",
    "",         "",         ":",        "#",        "@",        "'",        "=",        "\"",
    "",         "a",        "b",        "c",        "d",        "e",        "f",        "g",
    "h",        "i",        "",         "",         "",         "",         "",         "",
    "",         "j",        "k",        "l",        "m",        "n",        "o",        "p",
    "q",        "r",        "",         "",         "",         "",         "",         "",
    "",         "s",        "t",        "u",        "v",        "w",        "x",        "y",
    "z",        "",         "",         "",         "",         "",         "",         "",
    "",         "",         "",         "",         "",         "",         "",         "",
    "",         "`",        "",         "",         "",         "",         "",         "",
    "",         "A",        "B",        "C",        "D",        "E",        "F",        "G",
    "H",        "I",        "",         "",         "",         "",         "",         "",
    "",         "J",        "K",        "L",        "M",        "N",        "O",        "P",
    "Q",        "R",        "",         "",         "",         "",         "",         "",
    "",         "s",        "t",        "u",        "v",        "w",        "x",        "y",
    "z",        "",         "",         "",         "",         "",         "",         "",
    "0",        "1",        "2",        "3",        "4",        "5",        "6",        "7",
    "8",        "9",        "",         "",         "",         "",         "",         ""
    };


/*
 *  literal -- C0 control character mapping     (0x00-0x1f)
 */
static list             literal_c0_view = {
    "<NUL>",    "<SOH>",    "<STX>",    "<ETX>",    "<EOT>",    "<ENQ>",    "<ACK>",    "<BEL>",
    "<BS>",     "<TAB>",    "<NL>",     "<VT>",     "<FF>",     "<CR>",     "<SO>",     "<SI>",
    "<DLE>",    "<DC1>",    "<DC2>",    "<DC3>",    "<DC4>",    "<NAK>",    "<SYN>",    "<ETB>",
    "<CAN>",    "<EM>",     "<SUB>",    "<ESC>",    "<FS>",     "<GS>",     "<RS>",     "<US>"
    };


/*
 *  literal -- C1 control character mapping     (0x80-0x9f)
 */
static list             literal_c1_view = {
    "<PAD>",    "<HOP>",    "<BPH>",    "<NBH>",    "<IND>",    "<NEL>",    "<SSA>",    "<ESA>",
    "<HTS>",    "<HTJ>",    "<VTS>",    "<PLD>",    "<PLU>",    "<RI>",     "<SS2>",    "<SS3>",
    "<DCS>",    "<PU1>",    "<PU2>",    "<STS>",    "<CCH>",    "<MW>",     "<SPA>",    "<EPA>",
    "<SOS>",    "<SGI>",    "<SCI>",    "<CSI>",    "<ST>",     "<OSC>",    "<PM>",     "<APC>"
    };


/*
 *  Octal view.
 */
static list             octal_view = {
    "000 ",     "001 ",     "002 ",     "003 ",     "004 ",     "005 ",     "006 ",     "007 ",
    "010 ",     "011 ",     "012 ",     "013 ",     "014 ",     "015 ",     "016 ",     "017 ",
    "020 ",     "021 ",     "022 ",     "023 ",     "024 ",     "025 ",     "026 ",     "027 ",
    "030 ",     "031 ",     "032 ",     "033 ",     "034 ",     "035 ",     "036 ",     "037 ",
    "040 ",     "041 ",     "042 ",     "043 ",     "044 ",     "045 ",     "046 ",     "047 ",
    "050 ",     "051 ",     "052 ",     "053 ",     "054 ",     "055 ",     "056 ",     "057 ",
    "060 ",     "061 ",     "062 ",     "063 ",     "064 ",     "065 ",     "066 ",     "067 ",
    "070 ",     "071 ",     "072 ",     "073 ",     "074 ",     "075 ",     "076 ",     "077 ",
    "100 ",     "101 ",     "102 ",     "103 ",     "104 ",     "105 ",     "106 ",     "107 ",
    "110 ",     "111 ",     "112 ",     "113 ",     "114 ",     "115 ",     "116 ",     "117 ",
    "120 ",     "121 ",     "122 ",     "123 ",     "124 ",     "125 ",     "126 ",     "127 ",
    "130 ",     "131 ",     "132 ",     "133 ",     "134 ",     "135 ",     "136 ",     "137 ",
    "140 ",     "141 ",     "142 ",     "143 ",     "144 ",     "145 ",     "146 ",     "147 ",
    "150 ",     "151 ",     "152 ",     "153 ",     "154 ",     "155 ",     "156 ",     "157 ",
    "160 ",     "161 ",     "162 ",     "163 ",     "164 ",     "165 ",     "166 ",     "167 ",
    "170 ",     "171 ",     "172 ",     "173 ",     "174 ",     "175 ",     "176 ",     "177 ",
    "200 ",     "201 ",     "202 ",     "203 ",     "204 ",     "205 ",     "206 ",     "207 ",
    "210 ",     "211 ",     "212 ",     "213 ",     "214 ",     "215 ",     "216 ",     "217 ",
    "220 ",     "221 ",     "222 ",     "223 ",     "224 ",     "225 ",     "226 ",     "227 ",
    "230 ",     "231 ",     "232 ",     "233 ",     "234 ",     "235 ",     "236 ",     "237 ",
    "240 ",     "241 ",     "242 ",     "243 ",     "244 ",     "245 ",     "246 ",     "247 ",
    "250 ",     "251 ",     "252 ",     "253 ",     "254 ",     "255 ",     "256 ",     "257 ",
    "260 ",     "261 ",     "262 ",     "263 ",     "264 ",     "265 ",     "266 ",     "267 ",
    "270 ",     "271 ",     "272 ",     "273 ",     "274 ",     "275 ",     "276 ",     "277 ",
    "300 ",     "301 ",     "302 ",     "303 ",     "304 ",     "305 ",     "306 ",     "307 ",
    "310 ",     "311 ",     "312 ",     "313 ",     "314 ",     "315 ",     "316 ",     "317 ",
    "320 ",     "321 ",     "322 ",     "323 ",     "324 ",     "325 ",     "326 ",     "327 ",
    "330 ",     "331 ",     "332 ",     "333 ",     "334 ",     "335 ",     "336 ",     "337 ",
    "340 ",     "341 ",     "342 ",     "343 ",     "344 ",     "345 ",     "346 ",     "347 ",
    "350 ",     "351 ",     "352 ",     "353 ",     "354 ",     "355 ",     "356 ",     "357 ",
    "360 ",     "361 ",     "362 ",     "363 ",     "364 ",     "365 ",     "366 ",     "367 ",
    "370 ",     "371 ",     "372 ",     "373 ",     "374 ",     "375 ",     "376 ",     "377 ",
    };


static list             view_list = {
    "Hex",                      "view_sel hex",
    "Hex/ASCII fixed width",    "view_sel hexasc",
    "ASCII only",               "view_sel fwasc",
    "Octal",                    "view_sel octal",
    "EBCDIC",                   "view_sel ebcdic",
    "Literal",                  "view_sel literal",
    "EOL marker",               "view_sel eol",
    "Normal",                   "view_sel normal",
    };

/*
 *  Character maps identifiers.
 */
static int              hex_cmap     = -1;
static int              fw_cmap      = -1;
static int              hexasc_cmap  = -1;
static int              octal_cmap   = -1;
static int              ebcdic_cmap  = -1;
static int              literal_cmap = -1;
static int              eol_cmap     = -1;


/*
 *  view ---
 *      View selection.
 */
void
view(string arg)
{
    if (0 == strlen(arg) || "--select" == arg) {
        int viewbuf = inq_buffer(), viewwin = inq_window();

        UNUSED(viewbuf, viewwin);
        select_list("Views", "", 2, view_list, SEL_NORMAL | SEL_TOP_OF_WINDOW);
        return;
    }

    switch (arg) {
    case "hex":
        if (hex_cmap < 0) {
            hex_cmap = create_char_map(NULL, NULL, hex_view, NULL, "view::hex");
        }
        set_buffer_cmap(hex_cmap, inq_buffer());
        break;

    case "fwasc":
        if (fw_cmap < 0) {
            fw_cmap = create_char_map(NULL, NULL, fw_view, NULL, "view::fwasc");
        }
        set_buffer_cmap(fw_cmap, inq_buffer());
        break;

    case "hexasc":
        if (hexasc_cmap < 0) {
            hexasc_cmap = create_char_map(NULL, NULL, hexasc_view, NULL, "view::hexasc");
        }
        set_buffer_cmap(hexasc_cmap, inq_buffer());
//      set_buffer_flags(NULL, "hex_mode");
        break;

    case "octal":
        if (octal_cmap < 0) {
            octal_cmap = create_char_map(NULL, NULL, octal_view, NULL, "view::octal");
        }
        set_buffer_cmap(octal_cmap, inq_buffer());
//      set_buffer_flags(NULL, "oct_mode");
        break;

	  case "ebcdic":
		  if (ebcdic_cmap < 0) {
			   ebcdic_cmap = create_char_map(NULL, NULL, ebcdic_view, NULL, "view::ebcdic");
		  }
        set_buffer_cmap(ebcdic_cmap, inq_buffer());
		  break;

    case "literal":
        literal();
        break;

    case "eol":
		  if (eol_cmap < 0) {
            if (DC_UNICODE == ((DC_UNICODE|DC_ASCIIONLY) & inq_display_mode())) {
                eol_cmap = create_char_map(NULL, CMAP_EOL, quote_list(0x23CE), NULL, "view::eol");
            } else {
                eol_cmap = create_char_map(NULL, CMAP_EOL, quote_list("$"), NULL, "view::eol");
            }
		  }
        set_buffer_cmap(eol_cmap, inq_buffer());
		  break;

    case "normal":
        set_buffer_cmap(NULL, inq_buffer());
		  set_window_cmap(NULL, inq_window());
//      set_buffer_flags(NULL, NULL, "hex_mode,oct_mode");
        break;

    case "utf8":
    case "ucs2":
    case "ucs4":
        /* XXX/TODO */
        break;
    }
}


void
view_sel(string arg)
{
    extern int viewbuf, viewwin;
    int curbuf = inq_buffer(), curwin = inq_window();

    set_buffer(viewbuf);
    set_window(viewwin);
    view(arg);
    set_window(curwin);
    set_buffer(curbuf);
}


/*
 *  literal ---
 *      Toggle between literal display mode and normal mode.
 */
void
literal(void)
{
    if (literal_cmap < 0) {
        /*
         *  Create,
         *      push character map, plus EOL/EOF representation.
         *
         *  UNICODE/
         *      0x23CE -    RETURN SYMBOL.
         *      0x21A6 -    RIGHTWARDS ARROW FROM BAR.              |->
         *      0x2505 -    BOX DRAWINGS TRIPLE DASH HORIZONTIAL    ---
         *      0x290F -    RIGHTWARDS TRIPLE DASH ARROW            -->
         */
        literal_cmap = create_char_map(NULL, 0x00, literal_c0_view, NULL, "view::literal");
        create_char_map(literal_cmap, 0x80, literal_c1_view);

        if (DC_UNICODE == ((DC_UNICODE|DC_ASCIIONLY) & inq_display_mode())) {
            create_char_map(literal_cmap, 0, NULL, quote_list(0x09, CMAP_TAB));
            create_char_map(literal_cmap, CMAP_TABSTART, quote_list(0x21A6, 0x2505, 0x290F));
            create_char_map(literal_cmap, CMAP_EOL, quote_list(0x23CE));

        } else {
            create_char_map(literal_cmap, CMAP_EOL, quote_list("$"));
        }

        create_char_map(literal_cmap, CMAP_EOF, quote_list("[EOF]"));
    }

    if (inq_char_map() == literal_cmap) {       /* toggle */
        set_buffer_cmap(NULL, inq_buffer());
    } else {
        set_buffer_cmap(literal_cmap, inq_buffer());
    }
}

/*eof*/
