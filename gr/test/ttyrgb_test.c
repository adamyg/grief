#include <edidentifier.h>
__CIDENT_RCSID(gr_ttyrgb_test_c,"$Id: ttyrgb_test.c,v 1.1 2024/11/18 13:42:22 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: ttyrgb_test.c,v 1.1 2024/11/18 13:42:22 cvsuser Exp $
 * Color RGB support.
 *
 *
 * This file is part of the GRIEF Editor.
 *
 * The GRIEF Editor is free software: you can redistribute it
 * and/or modify it under the terms of the GRIEF Editor License.
 *
 * The GRIEF Editor is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * License for more details.
 * ==end==
 */

#include <editor.h>
#include <math.h>
#include <libstr.h>                             /* str_...()/sxprintf() */

#include "../ttyrgb.h"

static const struct {
    char rgb[9];
    unsigned cterm;
} gui2cterm[] = {
    {"#000000",  16}, {"#00005f",  17}, {"#000087",  18}, {"#0000af",  19},
    {"#0000d7",  20}, {"#0000ff",  21}, {"#005f00",  22}, {"#005f5f",  23},
    {"#005f87",  24}, {"#005faf",  25}, {"#005fd7",  26}, {"#005fff",  27},
    {"#008700",  28}, {"#00875f",  29}, {"#008787",  30}, {"#0087af",  31},
    {"#0087d7",  32}, {"#0087ff",  33}, {"#00af00",  34}, {"#00af5f",  35},
    {"#00af87",  36}, {"#00afaf",  37}, {"#00afd7",  38}, {"#00afff",  39},
    {"#00d700",  40}, {"#00d75f",  41}, {"#00d787",  42}, {"#00d7af",  43},
    {"#00d7d7",  44}, {"#00d7ff",  45}, {"#00ff00",  46}, {"#00ff5f",  47},
    {"#00ff87",  48}, {"#00ffaf",  49}, {"#00ffd7",  50}, {"#00ffff",  51},
    {"#5f0000",  52}, {"#5f005f",  53}, {"#5f0087",  54}, {"#5f00af",  55},
    {"#5f00d7",  56}, {"#5f00ff",  57}, {"#5f5f00",  58}, {"#5f5f5f",  59},
    {"#5f5f87",  60}, {"#5f5faf",  61}, {"#5f5fd7",  62}, {"#5f5fff",  63},
    {"#5f8700",  64}, {"#5f875f",  65}, {"#5f8787",  66}, {"#5f87af",  67},
    {"#5f87d7",  68}, {"#5f87ff",  69}, {"#5faf00",  70}, {"#5faf5f",  71},
    {"#5faf87",  72}, {"#5fafaf",  73}, {"#5fafd7",  74}, {"#5fafff",  75},
    {"#5fd700",  76}, {"#5fd75f",  77}, {"#5fd787",  78}, {"#5fd7af",  79},
    {"#5fd7d7",  80}, {"#5fd7ff",  81}, {"#5fff00",  82}, {"#5fff5f",  83},
    {"#5fff87",  84}, {"#5fffaf",  85}, {"#5fffd7",  86}, {"#5fffff",  87},
    {"#870000",  88}, {"#87005f",  89}, {"#870087",  90}, {"#8700af",  91},
    {"#8700d7",  92}, {"#8700ff",  93}, {"#875f00",  94}, {"#875f5f",  95},
    {"#875f87",  96}, {"#875faf",  97}, {"#875fd7",  98}, {"#875fff",  99},
    {"#878700", 100}, {"#87875f", 101}, {"#878787", 102}, {"#8787af", 103},
    {"#8787d7", 104}, {"#8787ff", 105}, {"#87af00", 106}, {"#87af5f", 107},
    {"#87af87", 108}, {"#87afaf", 109}, {"#87afd7", 110}, {"#87afff", 111},
    {"#87d700", 112}, {"#87d75f", 113}, {"#87d787", 114}, {"#87d7af", 115},
    {"#87d7d7", 116}, {"#87d7ff", 117}, {"#87ff00", 118}, {"#87ff5f", 119},
    {"#87ff87", 120}, {"#87ffaf", 121}, {"#87ffd7", 122}, {"#87ffff", 123},
    {"#af0000", 124}, {"#af005f", 125}, {"#af0087", 126}, {"#af00af", 127},
    {"#af00d7", 128}, {"#af00ff", 129}, {"#af5f00", 130}, {"#af5f5f", 131},
    {"#af5f87", 132}, {"#af5faf", 133}, {"#af5fd7", 134}, {"#af5fff", 135},
    {"#af8700", 136}, {"#af875f", 137}, {"#af8787", 138}, {"#af87af", 139},
    {"#af87d7", 140}, {"#af87ff", 141}, {"#afaf00", 142}, {"#afaf5f", 143},
    {"#afaf87", 144}, {"#afafaf", 145}, {"#afafd7", 146}, {"#afafff", 147},
    {"#afd700", 148}, {"#afd75f", 149}, {"#afd787", 150}, {"#afd7af", 151},
    {"#afd7d7", 152}, {"#afd7ff", 153}, {"#afff00", 154}, {"#afff5f", 155},
    {"#afff87", 156}, {"#afffaf", 157}, {"#afffd7", 158}, {"#afffff", 159},
    {"#d70000", 160}, {"#d7005f", 161}, {"#d70087", 162}, {"#d700af", 163},
    {"#d700d7", 164}, {"#d700ff", 165}, {"#d75f00", 166}, {"#d75f5f", 167},
    {"#d75f87", 168}, {"#d75faf", 169}, {"#d75fd7", 170}, {"#d75fff", 171},
    {"#d78700", 172}, {"#d7875f", 173}, {"#d78787", 174}, {"#d787af", 175},
    {"#d787d7", 176}, {"#d787ff", 177}, {"#d7af00", 178}, {"#d7af5f", 179},
    {"#d7af87", 180}, {"#d7afaf", 181}, {"#d7afd7", 182}, {"#d7afff", 183},
    {"#d7d700", 184}, {"#d7d75f", 185}, {"#d7d787", 186}, {"#d7d7af", 187},
    {"#d7d7d7", 188}, {"#d7d7ff", 189}, {"#d7ff00", 190}, {"#d7ff5f", 191},
    {"#d7ff87", 192}, {"#d7ffaf", 193}, {"#d7ffd7", 194}, {"#d7ffff", 195},
    {"#ff0000", 196}, {"#ff005f", 197}, {"#ff0087", 198}, {"#ff00af", 199},
    {"#ff00d7", 200}, {"#ff00ff", 201}, {"#ff5f00", 202}, {"#ff5f5f", 203},
    {"#ff5f87", 204}, {"#ff5faf", 205}, {"#ff5fd7", 206}, {"#ff5fff", 207},
    {"#ff8700", 208}, {"#ff875f", 209}, {"#ff8787", 210}, {"#ff87af", 211},
    {"#ff87d7", 212}, {"#ff87ff", 213}, {"#ffaf00", 214}, {"#ffaf5f", 215},
    {"#ffaf87", 216}, {"#ffafaf", 217}, {"#ffafd7", 218}, {"#ffafff", 219},
    {"#ffd700", 220}, {"#ffd75f", 221}, {"#ffd787", 222}, {"#ffd7af", 223},
    {"#ffd7d7", 224}, {"#ffd7ff", 225}, {"#ffff00", 226}, {"#ffff5f", 227},
    {"#ffff87", 228}, {"#ffffaf", 229}, {"#ffffd7", 230}, {"#ffffff", 231},
    {"#080808", 232}, {"#121212", 233}, {"#1c1c1c", 234}, {"#262626", 235},
    {"#303030", 236}, {"#3a3a3a", 237}, {"#444444", 238}, {"#4e4e4e", 239},
    {"#585858", 240}, {"#626262", 241}, {"#6c6c6c", 242}, {"#767676", 243},
    {"#808080", 244}, {"#8a8a8a", 245}, {"#949494", 246}, {"#9e9e9e", 247},
    {"#a8a8a8", 248}, {"#b2b2b2", 249}, {"#bcbcbc", 250}, {"#c6c6c6", 251},
    {"#d0d0d0", 252}, {"#dadada", 253}, {"#e4e4e4", 254}, {"#eeeeee", 255},
    };


/*  Function:           rgb_exporttable
 *      Export the given RGB table to a structure.
 *
 *  Parameters:
 *      label - Optional table label.
 *      table - Table address.
 *      maxval - Under attribute value.
 *      base - Display base 8, 10 or 16.
 *
 *  Returns:
 *      nothing.
 */
static void
rgb_exporttable(const char *label, const struct rgbdef *table, int maxval, int base)
{
    unsigned color;

    printf("const struct rgbvalue %srgb_%dtable[%d] = {\n", label, maxval, maxval);

    if (10 == base) {
        for (color = 0; color < maxval; ++color) {
    printf("        { %d,%s  %3d, %3d, %3d },\n",
            color, (color < 10 ? "  " : (color < 100 ? " " : "")),
                table->red, table->green, table->blue);
            ++table;
        }

    } else {
        for (color = 0; color < maxval; ++color) {
    printf("        { %d,%s  0x%02x, 0x%02x, 0x%02x },\n",
            color, (color < 10 ? "  " : (color < 100 ? " " : "")),
                table->red, table->green, table->blue);
            ++table;
        }
    }

    printf("    };\n");
    printf("\n");
}


int
main(int argc, char *argv[])
{
    static const char *specs[] = {
        "#123",
        "#010203",
        "#001002003",
        "#000100020003",
        "",
        "123",
        "010203",
        "001002003",
        "000100020003",
        "",

        "rgb:12/34/45",
        "rgb: 12/34 / 45 ",
        "rgbi:.1/.2/.3",
        "rgbi: .1/.2 / .3 ",
        "rgb(255, 0, 0)",
        "rgb( 0, 255 , 0 )",
        "rgb(100%, 50%, 0%)",
        "rgb( 50%,100% , 0 %)",
        "",

        "hsl(0,   100%, 50%)",      /* red          #FF0000 */
        "hsl(360, 100%, 50%)",      /*  "              "    */
        "hsl(120, 100%, 50%)",      /* lime         #00FF00 */
        "hsl(480, 100%, 50%)",      /*  "              "    */
        "hsl(120, 100%, 25%)",      /* dark green   #006400 */
        "hsl(120, 100%, 75%)",      /* light green  #90EE90 */
        "hsl(120, 75%,  75%)",      /* pastel green #8F3F8F */
        "hsl(360, 75%,  75%)",
        "",

        /*invalid*/
        "#12",
        "#01023",
        "#00100203",
        "#00010002003",
        "12",
        "01023",
        "00100203",
        "00010002003",
        "rgb(110%, 50%, 0%)",
        "rgb(-110%, 50%, 0%)",
        "rgb(100%, 50, 0%)",
        NULL
        };
    struct rgbvalue rgb;
    const char *p;
    char buf[64] = {0};
    int cnt, i;

    if (argc > 1) {
        int base = 0;

        if (0 == strcmp(argv[1], "--dec")) {
            base = 10;

        } else if (0 == strcmp(argv[1], "--hex")) {
            base = 16;
        }

        if (base) {
            rgb_exporttable("win_",   rgb_win256table(), 256, base);
            rgb_exporttable("xterm_", rgb_xterm256table(), 256, base);
            rgb_exporttable("xterm_", rgb_xterm88table(), 88, base);
        }
    }

    for (cnt = i = 0; NULL != (p = specs[i]); ++i) {
        if (*p) {
            int ret = rgb_import(p, strlen(p), &rgb, 0xff);

            rgb_export(buf, sizeof(buf), &rgb, 0xff);
            printf("%02d: %-16s (%2d) == %2d/%02x/%02x/%02x [%s]\n",
                cnt++, p, ret, rgb.type, rgb.red, rgb.green, rgb.blue, buf);

        } else {
            printf("\n");
        }
    }

    printf("\n\nrgb2xterm\n");
    for (i = 0; i < 256; ++i) {
        rgb_256xterm(i, &rgb);

        printf("%03d: rgb:%02x/%02x/%02x, xterm=%3d, color=%3d\n",
            i, rgb.red, rgb.green, rgb.blue, rgb_xterm256(&rgb), rgb_color256(&rgb));
    }

    printf("\n\nrgb2cterm\n");
    for (i = 0; i < sizeof(gui2cterm)/sizeof(gui2cterm[0]); ++i) {
        const char *spec = gui2cterm[i].rgb;
        int ret = rgb_import(spec, strlen(spec), &rgb, 0xff);

        printf("%03d: %-15s (%2d) == rgb:%02x/%02x/%02x [xterm=%3d verses %3d]\n",
            16 + i, spec, ret, rgb.red, rgb.green, rgb.blue, rgb_color256(&rgb), gui2cterm[i].cterm);
    }

    return 0;
}

//end
