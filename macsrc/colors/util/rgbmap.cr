/* -*- mode: cr; indent-width: 8; -*- */
/* $Id: rgbmap.cr,v 1.5 2024/10/07 16:23:06 cvsuser Exp $
 * RGB color mapper.
 *
 */

#include "../../grief.h"
#include "rgbmap.h"

static void xterm_color_map(void);

static int colormap;

void
main(void)
{
        xterm_color_map();
}


//
// grey_number ---
//      Returns an approximate grey index for the given grey level.
//
static int
grey_number(int x)
{
        extern int colordepth;

        if (colordepth == 88) {
                if (x < 23) {
                        return 0;
                } else if (x < 69) {
                        return 1;
                } else if (x < 103) {
                        return 2;
                } else if (x < 127) {
                        return 3;
                } else if (x < 150) {
                        return 4;
                } else if (x < 173) {
                        return 5;
                } else if (x < 196) {
                        return 6;
                } else if (x < 219) {
                        return 7;
                } else if (x < 243) {
                        return 8;
                } else {
                        return 9;
                }
        } else {
                if (x < 14) {
                        return 0;
                } else {
                        float n = (x - 8) / 10;
                        int m = (x - 8) % 10;

                        if (m < 5) {
                                return n;
                        } else {
                                return n + 1;
                        }
                }
        }
}


//
// grey_level ---
//      Returns the actual grey level represented by the grey index.
//
static int
grey_level(int n)
{
        extern int colordepth;

        if (colordepth == 88) {
                if (n == 0) {
                        return 0;
                } else if (n == 1) {
                        return 46;
                } else if (n == 2) {
                        return 92;
                } else if (n == 3) {
                        return 115;
                } else if (n == 4) {
                        return 139;
                } else if (n == 5) {
                        return 162;
                } else if (n == 6) {
                        return 185;
                } else if (n == 7) {
                        return 208;
                } else if (n == 8) {
                        return 231;
                } else {
                        return 255;
                }
        } else {
                if (n == 0) {
                        return 0;
                } else {
                        return 8 + (n * 10);
                }
        }
}


//
// grey_colour ---
//      Returns the palette index for the given grey index.
//
static int
grey_colour(int n)
{
        extern int colordepth;

        if (colordepth == 88) {
                if (n == 0) {
                        return 16;
                } else if (n == 9) {
                        return 79;
                } else {
                        return 79 + n;
                }
        } else {
                if (n == 0) {
                        return 16;
                } else if (n == 25) {
                        return 231;
                } else {
                        return 231 + n;
                }
        }
}


//
// rgb_number ---
//      Returns an approximate colour index for the given colour level.
//
static int
rgb_number(int x)
{
        extern int colordepth;

        if (colordepth == 88) {
                if (x < 69) {
                        return 0;
                } else if (x < 172) {
                        return 1;
                } else if (x < 230) {
                        return 2;
                } else {
                        return 3;
                }
        } else {
                if (x < 75) {
                        return 0;
                } else {
                        float n = (x - 55) / 40;
                        int m = (x - 55) % 40;

                        if (m < 20) {
                                return n;
                        } else {
                                return n + 1;
                        }
                }
        }
}


//
// rgb_level ---
//      Returns the actual colour level for the given colour index.
//
static int
rgb_level(int n)
{
        extern int colordepth;

        if (colordepth == 88) {
                if (n == 0) {
                        return 0;
                } else if (n == 1) {
                        return 139;
                } else if (n == 2) {
                        return 205;
                } else {
                        return 255;
                }
        } else {
                if (n == 0) {
                        return 0;
                } else {
                        return 55 + (n * 40);
                }
        }
}


//
// rgb_colour ---
//      Returns the palette index for the given R/G/B colour indices.
//
static string
rgb_colour(int x, int y, int z)
{
        extern int colordepth;

        if (colordepth == 88) {
                return format("%d", 16 + (x * 16) + (y * 4) + z);
        } else {
                return format("%d", 16 + (x * 36) + (y * 6) + z);
        }
}


//
// RGBPaletteIndex ---
//      Returns the palette index to approximate the given R/G/B colour levels.
//
string
RGBPaletteIndex(int r, int g, int b, int colordepth)
{
        int gx = grey_number(r);
        int gy = grey_number(g);
        int gz = grey_number(b);

        // Get the closest colour
        int x = rgb_number(r);
        int y = rgb_number(g);
        int z = rgb_number(b);

        UNUSED(colordepth);

        if (gx == gy && gy == gz) { // r=g=b
                // Either grey-scale 235+ or color.
                //
                int dgr     = grey_level(gx) - r;
                int dgg     = grey_level(gy) - g;
                int dgb     = grey_level(gz) - b;
                int dgrey   = (dgr * dgr) + (dgg * dgg) + (dgb * dgb);
                int dr      = rgb_level(gx)  - r;
                int dg      = rgb_level(gy)  - g;
                int db      = rgb_level(gz)  - b;
                int drgb    = (dr * dr) + (dg * dg) + (db * db);

                if (dgrey < drgb) { // grey-scale
                        return grey_colour(gx);
                } else { // colour
                        return rgb_colour(x, y, z);
                }
        } else {
                // 6x6x6 color cube.
                return rgb_colour(x, y, z);
        }
}


//
// RGBMap ---
//      RGB to XTern color for the specified 'colordepth'.
//
// Returns:
//      Palette index to approximate the RRGGBB color.
//
string
RGBMap(string rgb, int colordepth)
{
        int r,g,b;

        if (characterat(rgb, 1) == '#') { // #RRGGBB
                if (colordepth >= 256) {
                        declare value = get_property(colormap, rgb);
                        if (! is_null(value))
                                return value;
                }
                sscanf(rgb, "#%2x%2x%2x", r, g, b);

        } else { // RRGGBB
                if (colordepth >= 256) {
                        declare value = get_property(colormap, "#" + rgb);
                        if (! is_null(value))
                                return value;
                }
                sscanf(rgb, "%2x%2x%2x", r, g, b);
        }

        return RGBPaletteIndex(r, g, b, colordepth);
}


//
// RGBMap256 --
//      RGB to XTern color for the specified 'colordepth'.
//
// Returns:
//      Palette index to approximate the RRGGBB color.
//
string
RGBMap256(string rgb)
{
        int r,g,b;

        if (characterat(rgb, 1) == '#') { // #RRGGBB
                declare value = get_property(colormap, rgb);
                if (! is_null(value)) {
                        return value;
                }
                sscanf(rgb, "#%2x%2x%2x", r, g, b);

        } else { // RRGGBB
                declare value = get_property(colormap, "#" + rgb);
                if (! is_null(value)) {
                        return value;
                }
                sscanf(rgb, "%2x%2x%2x", r, g, b);
        }

        return RGBPaletteIndex(r, g, b, 256);
}


//
// xterm_color_map --
//      Build the XTerm color-map for 6x6x6 colorcube plus grey-scale attributes.
//
static void
xterm_color_map(void)
{
        // color-map 6x6x6
        colormap = create_dictionary("xterm-color-map", 256);

        colormap.fg = "fg";
        colormap.bg = "bg";
        colormap.NONE = "NONE";

#define CM(__n, __v) set_property(colormap, __n, __v)

        CM("#000000",  16); CM("#00005f",  17); CM("#000087",  18); CM("#0000af",  19);
        CM("#0000d7",  20); CM("#0000ff",  21); CM("#005f00",  22); CM("#005f5f",  23);
        CM("#005f87",  24); CM("#005faf",  25); CM("#005fd7",  26); CM("#005fff",  27);
        CM("#008700",  28); CM("#00875f",  29); CM("#008787",  30); CM("#0087af",  31);
        CM("#0087d7",  32); CM("#0087ff",  33); CM("#00af00",  34); CM("#00af5f",  35);
        CM("#00af87",  36); CM("#00afaf",  37); CM("#00afd7",  38); CM("#00afff",  39);
        CM("#00d700",  40); CM("#00d75f",  41); CM("#00d787",  42); CM("#00d7af",  43);
        CM("#00d7d7",  44); CM("#00d7ff",  45); CM("#00ff00",  46); CM("#00ff5f",  47);
        CM("#00ff87",  48); CM("#00ffaf",  49); CM("#00ffd7",  50); CM("#00ffff",  51);
        CM("#5f0000",  52); CM("#5f005f",  53); CM("#5f0087",  54); CM("#5f00af",  55);
        CM("#5f00d7",  56); CM("#5f00ff",  57); CM("#5f5f00",  58); CM("#5f5f5f",  59);
        CM("#5f5f87",  60); CM("#5f5faf",  61); CM("#5f5fd7",  62); CM("#5f5fff",  63);
        CM("#5f8700",  64); CM("#5f875f",  65); CM("#5f8787",  66); CM("#5f87af",  67);
        CM("#5f87d7",  68); CM("#5f87ff",  69); CM("#5faf00",  70); CM("#5faf5f",  71);
        CM("#5faf87",  72); CM("#5fafaf",  73); CM("#5fafd7",  74); CM("#5fafff",  75);
        CM("#5fd700",  76); CM("#5fd75f",  77); CM("#5fd787",  78); CM("#5fd7af",  79);
        CM("#5fd7d7",  80); CM("#5fd7ff",  81); CM("#5fff00",  82); CM("#5fff5f",  83);
        CM("#5fff87",  84); CM("#5fffaf",  85); CM("#5fffd7",  86); CM("#5fffff",  87);
        CM("#870000",  88); CM("#87005f",  89); CM("#870087",  90); CM("#8700af",  91);
        CM("#8700d7",  92); CM("#8700ff",  93); CM("#875f00",  94); CM("#875f5f",  95);
        CM("#875f87",  96); CM("#875faf",  97); CM("#875fd7",  98); CM("#875fff",  99);
        CM("#878700", 100); CM("#87875f", 101); CM("#878787", 102); CM("#8787af", 103);
        CM("#8787d7", 104); CM("#8787ff", 105); CM("#87af00", 106); CM("#87af5f", 107);
        CM("#87af87", 108); CM("#87afaf", 109); CM("#87afd7", 110); CM("#87afff", 111);
        CM("#87d700", 112); CM("#87d75f", 113); CM("#87d787", 114); CM("#87d7af", 115);
        CM("#87d7d7", 116); CM("#87d7ff", 117); CM("#87ff00", 118); CM("#87ff5f", 119);
        CM("#87ff87", 120); CM("#87ffaf", 121); CM("#87ffd7", 122); CM("#87ffff", 123);
        CM("#af0000", 124); CM("#af005f", 125); CM("#af0087", 126); CM("#af00af", 127);
        CM("#af00d7", 128); CM("#af00ff", 129); CM("#af5f00", 130); CM("#af5f5f", 131);
        CM("#af5f87", 132); CM("#af5faf", 133); CM("#af5fd7", 134); CM("#af5fff", 135);
        CM("#af8700", 136); CM("#af875f", 137); CM("#af8787", 138); CM("#af87af", 139);
        CM("#af87d7", 140); CM("#af87ff", 141); CM("#afaf00", 142); CM("#afaf5f", 143);
        CM("#afaf87", 144); CM("#afafaf", 145); CM("#afafd7", 146); CM("#afafff", 147);
        CM("#afd700", 148); CM("#afd75f", 149); CM("#afd787", 150); CM("#afd7af", 151);
        CM("#afd7d7", 152); CM("#afd7ff", 153); CM("#afff00", 154); CM("#afff5f", 155);
        CM("#afff87", 156); CM("#afffaf", 157); CM("#afffd7", 158); CM("#afffff", 159);
        CM("#d70000", 160); CM("#d7005f", 161); CM("#d70087", 162); CM("#d700af", 163);
        CM("#d700d7", 164); CM("#d700ff", 165); CM("#d75f00", 166); CM("#d75f5f", 167);
        CM("#d75f87", 168); CM("#d75faf", 169); CM("#d75fd7", 170); CM("#d75fff", 171);
        CM("#d78700", 172); CM("#d7875f", 173); CM("#d78787", 174); CM("#d787af", 175);
        CM("#d787d7", 176); CM("#d787ff", 177); CM("#d7af00", 178); CM("#d7af5f", 179);
        CM("#d7af87", 180); CM("#d7afaf", 181); CM("#d7afd7", 182); CM("#d7afff", 183);
        CM("#d7d700", 184); CM("#d7d75f", 185); CM("#d7d787", 186); CM("#d7d7af", 187);
        CM("#d7d7d7", 188); CM("#d7d7ff", 189); CM("#d7ff00", 190); CM("#d7ff5f", 191);
        CM("#d7ff87", 192); CM("#d7ffaf", 193); CM("#d7ffd7", 194); CM("#d7ffff", 195);
        CM("#ff0000", 196); CM("#ff005f", 197); CM("#ff0087", 198); CM("#ff00af", 199);
        CM("#ff00d7", 200); CM("#ff00ff", 201); CM("#ff5f00", 202); CM("#ff5f5f", 203);
        CM("#ff5f87", 204); CM("#ff5faf", 205); CM("#ff5fd7", 206); CM("#ff5fff", 207);
        CM("#ff8700", 208); CM("#ff875f", 209); CM("#ff8787", 210); CM("#ff87af", 211);
        CM("#ff87d7", 212); CM("#ff87ff", 213); CM("#ffaf00", 214); CM("#ffaf5f", 215);
        CM("#ffaf87", 216); CM("#ffafaf", 217); CM("#ffafd7", 218); CM("#ffafff", 219);
        CM("#ffd700", 220); CM("#ffd75f", 221); CM("#ffd787", 222); CM("#ffd7af", 223);
        CM("#ffd7d7", 224); CM("#ffd7ff", 225); CM("#ffff00", 226); CM("#ffff5f", 227);
        CM("#ffff87", 228); CM("#ffffaf", 229); CM("#ffffd7", 230); CM("#ffffff", 231);
        CM("#080808", 232); CM("#121212", 233); CM("#1c1c1c", 234); CM("#262626", 235);
        CM("#303030", 236); CM("#3a3a3a", 237); CM("#444444", 238); CM("#4e4e4e", 239);
        CM("#585858", 240); CM("#626262", 241); CM("#6c6c6c", 242); CM("#767676", 243);
        CM("#808080", 244); CM("#8a8a8a", 245); CM("#949494", 246); CM("#9e9e9e", 247);
        CM("#a8a8a8", 248); CM("#b2b2b2", 249); CM("#bcbcbc", 250); CM("#c6c6c6", 251);
        CM("#d0d0d0", 252); CM("#dadada", 253); CM("#e4e4e4", 254); CM("#eeeeee", 255);
}

//end
