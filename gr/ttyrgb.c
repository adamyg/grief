#include <edidentifier.h>
__CIDENT_RCSID(gr_ttyrgb_c,"$Id: ttyrgb.c,v 1.21 2025/02/07 03:03:22 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: ttyrgb.c,v 1.21 2025/02/07 03:03:22 cvsuser Exp $
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

#include "ttyrgb.h"

#define HEXVALUE(__c)   (isdigit(__c) ? (__c - '0') : (tolower(__c) - 'a') + 10);

enum {
    COLORRGB_BAD = -1,
    COLORRGB_RGB = 1,
    COLORRGB_RGBI,
    COLORRGB_RGBX,
    COLORRGB_RGBWI,
    COLORRGB_RGBWF,
    COLORRGB_HSL,
};

static int              rgblist(const char **cursor, int rgbmax);
static int              rgbhex(const char **cursor, int len, int rgbmax);

static int              rgb_searchtable(const struct rgbdef *table, int maxval, int greyscale, const struct rgbvalue *rgb);

void                    hsl2rgb(float h, float s, float l, struct rgbvalue *rgb);
static float            huetorgb(float m1, float m2, float h);
void                    rgb2hsl(const struct rgbvalue *rgb, float *h, float *s, float *l);

#define BASIC16         16
#define GREY24          24

static const struct rgbdef xterm_basic16[BASIC16] = {
    { 0,    0x00, 0x00, 0x00 },
    { 1,    0xCD, 0x00, 0x00 },
    { 2,    0x00, 0xCD, 0x00 },
    { 3,    0xCD, 0xCD, 0x00 },
    { 4,    0x00, 0x00, 0xEE },
    { 5,    0xCD, 0x00, 0xCD },
    { 6,    0x00, 0xCD, 0xCD },
    { 7,    0xE5, 0xE5, 0xE5 },
    { 8,    0x7F, 0x7F, 0x7F },
    { 9,    0xFF, 0x00, 0x00 },
    { 10,   0x00, 0xFF, 0x00 },
    { 11,   0xFF, 0xFF, 0x00 },
    { 12,   0x5C, 0x5C, 0xFF },
    { 13,   0xFF, 0x00, 0xFF },
    { 14,   0x00, 0xFF, 0xFF },
    { 15,   0xFF, 0xFF, 0xFF }
    };


static const struct rgbdef win_basic16[BASIC16] = {
    { 0,    0x00, 0x00, 0x00 },
    { 1,    0x00, 0x00, 0x80 },
    { 2,    0x00, 0x80, 0x00 },
    { 3,    0x00, 0x80, 0x80 },
    { 4,    0x80, 0x00, 0x00 },
    { 5,    0x80, 0x00, 0x80 },
    { 6,    0x80, 0x80, 0x00 },
    { 7,    0xc0, 0xc0, 0xc0 },
    { 8,    0x80, 0x80, 0x80 },
    { 9,    0x00, 0x00, 0xff },
    { 10,   0x00, 0xff, 0x00 },
    { 11,   0x00, 0xff, 0xff },
    { 12,   0xff, 0x00, 0x00 },
    { 13,   0xff, 0x00, 0xff },
    { 14,   0xff, 0xff, 0x00 },
    { 15,   0xff, 0xff, 0xff },
    };


#if (defined(__WATCOMC__) && (__WATCOMC__ < 1300)) || \
        (defined(_MSC_VER) && (_MSC_VER <= 1600))
static double
round(const double x)   //middle value point test
{
    if (ceil(x + 0.5) == floor(x + 0.5)) {
        int a = (int)ceil(x);
        if (a % 2 == 0) {return ceil(x);}
        return floor(x);
    }
    return floor(x + 0.5);
}
#endif


#if defined(__WATCOMC__) || \
        (defined(_MSC_VER) && (_MSC_VER <= 1600))
static int              //c99
nearbyintf(float x)
{
    return (int)round(x);
}
#endif


static __CINLINE int
Round(double x)
{
    if (x >= 0) return (int)(x + 0.5);
    return (int)(x - 0.5);
//  return (int)round(x);
}


int
rgb_import(const char *name, size_t length, struct rgbvalue *rgb, int rgbmax)
{
    int r = 0, g = 0, b = 0, ret = COLORRGB_BAD;

    if (name && length > 0) {
        if (0 == strncmp(name, "rgb:", 4)) {
            /*
             *  X11 RGB Device String Specification
             *
             *      rgb:<red>/<green>/<blue>
             *
             *  Red, green and blue can be of the form (h|h|hhh|hhhh) indicating the scaling factor.
             *  'h' indicates the value scaled in 4 bits, 'hh' scaled in 8 bits, 'hhh' scaled in 12 bits
             *  and 'hhhh' the value scaled in 16 bits.
             *
             *  Examples;
             *      rgb:es/75/53
             */
            const char *cursor = name + 4;

            r = rgblist(&cursor, rgbmax);
            g = rgblist(&cursor, rgbmax);
            b = rgblist(&cursor, rgbmax);
            if (b >= 0 && 0 == *cursor) {
                ret = COLORRGB_RGB;
            }

        } else if (0 == strncmp(name, "rgbi:", 5)) {
            /*
             *  X11 RGB Intensity String specification:
             *
             *      rgbi:<red>/<green>/<blue>
             *
             *  Red, green and blue are floating point values between the range 0.0 and 1.0 inclusive.
             */
            float fr, fg, fb;

            if (3 == sscanf(name + 5, "%f /%f /%f ", &fr, &fg, &fb)) {
                if (fr >= 0.0 && fr <= 1.0 && fg >= 0.0 && fg <= 1.0 && fb >= 0.0 && fb <= 1.0) {
                    r = Round(fr * (double)rgbmax);
                    g = Round(fg * (double)rgbmax);
                    b = Round(fb * (double)rgbmax);
                    ret = COLORRGB_RGBI;
                }
            }

        } else if (0 == strncmp(name, "rgb(", 4)) {
            /*
             *  WEB RGB Specification
             *
             *      rgb(255, 0, 0)          - integer range 0
             *      rgb(100%, 0%, 0%)       - float range 0.0% - 100.0%
             */
            float fr, fg, fb;

            if (3 == sscanf(name + 4, "%d ,%d ,%d )", &r, &g, &b)) {
                if (r >= 0 && r <= rgbmax && g >= 0 && g <= rgbmax && b >= 0 && b <= rgbmax) {
                    ret = COLORRGB_RGBWI;
                }

            } else if (3 == sscanf(name + 4, "%f %% ,%f %% ,%f %% )", &fr, &fg, &fb)) {
                if (fr >= 0 && fr <= 100 && fg >= 0 && fg <= 100 && fb >= 0 && fb <= 100) {
                    r = Round((fr * (double)rgbmax) / 100);
                    g = Round((fg * (double)rgbmax) / 100);
                    b = Round((fb * (double)rgbmax) / 100);
                    ret = COLORRGB_RGBWF;
                }
            }

        } else if (0 == strncmp(name, "hsl(", 4)) {
            /*
             *  HSL Specification, following CSS (http://www.w3.org/TR/css3-color)
             *
             *        hsl(0,   100%, 50%)   - red
             *        hsl(120, 100%, 50%)   - lime
             *        hsl(120, 100%, 25%)   - dark green
             *        hsl(120, 100%, 75%)   - light green
             *        hsl(120, 75%,  75%)   - pastel green
             */
            float h, s, l;

            if (3 == sscanf(name + 4, "%f ,%f %% ,%f %% )", &h, &s, &l)) {
                if (s >= 0 && s <= 100 && l >= 0 && l <= 100) {
                    struct rgbvalue t_rgb = {0};

                    hsl2rgb(h / 360, s / 100, l / 100, &t_rgb);
                    r   = t_rgb.red;
                    g   = t_rgb.green;
                    b   = t_rgb.blue;
                    ret = COLORRGB_HSL;
                }
            }

        } else if ('#' == name[0] || isxdigit((unsigned)name[0])) {
            /*
             *  RGB Device String Specification (older style)
             *
             *      #RGB, #RRGGBB, #RRRGGGBBB or #RRRRGGGGBBBB
             *
             *  The R, G and B represent single hex digits. When fewer then 16 bits are specificated
             *  they represent the most significant bit of the value (unlike the "rgb:" syntax in
             *  which values are scaled).
             *
             *  For example:
             *      #3a7    ==> #3000a0007000
             */
            const char *cursor = name;

            if ('#' == name[0]) {
                ++cursor, --length;
            }

            switch (length) {
            case 3:         /* 4 bits */
                if ('#' == name[0]) { /* #RGB only */
                    r = rgbhex(&cursor, 1, rgbmax);
                    g = rgbhex(&cursor, 1, rgbmax);
                    b = rgbhex(&cursor, 1, rgbmax);
                }
                break;
            case 6:         /* 8 bits */
                r = rgbhex(&cursor, 2, rgbmax);
                g = rgbhex(&cursor, 2, rgbmax);
                b = rgbhex(&cursor, 2, rgbmax);
                break;
            case 9:         /* 12 bits */
                r = rgbhex(&cursor, 3, rgbmax);
                g = rgbhex(&cursor, 3, rgbmax);
                b = rgbhex(&cursor, 3, rgbmax);
                break;
            case 12:        /* 16 bits */
                r = rgbhex(&cursor, 4, rgbmax);
                g = rgbhex(&cursor, 4, rgbmax);
                b = rgbhex(&cursor, 4, rgbmax);
                break;
            default:
                break;
            }

            if (b >= 0 && 0 == *cursor) {
                ret = COLORRGB_RGBX;
            }
        }
    }

    if (rgb) {
        if ((rgb->type = ret) > COLORRGB_BAD) {
            rgb->red    = r;
            rgb->green  = g;
            rgb->blue   = b;
        } else {
            rgb->red    = 0;
            rgb->green  = 0;
            rgb->blue   = 0;
        }
    }
    return ret;
}


static __CINLINE int
is_white(int ch)
{
    return (' ' == ch || '\t' == ch);
}


/*
 *  delimitered
 *      <red>/<green>/<blue>
 */
static int
rgblist(const char **cursor, int rgbmax)
{
    const char *t_cursor = *cursor;
    int digits = 0, value = 0, c;

    *cursor = NULL;
    if (NULL == t_cursor || 0 == *t_cursor) {
        return -1;
    }

    for (digits = 0; 0 != (c = (unsigned)*t_cursor);) {
        ++t_cursor;
        if ('/' == c) {
            break;
        }
        if (! isxdigit(c)) {
            if (is_white(c)) {
                continue;
            }
            return -1;
        }
        value = (value << 4) + HEXVALUE(c);
        ++digits;
    }
    if (!digits) {
        return -1;
    }

    /* scale */
    switch (digits) {
    case 1:             /* 4 bits   */
        if (0x000f != rgbmax) {
            value = value * rgbmax / 0x000f;
        }
        break;
    case 2:             /* 8 bits   */
        if (0x00ff != rgbmax) {
            value = value * rgbmax / 0x00ff;
        }
        break;
    case 3:             /* 12 bits  */
        if (0x0fff != rgbmax) {
            value = value * rgbmax / 0x0fff;
        }
        break;
    case 4:             /* 16 bits  */
        if (0xffff != rgbmax) {
            value = value * rgbmax / 0xffff;
        }
        break;
    default:
        return -1;
    }

    if (value > rgbmax) {
        return -1;
    }
    *cursor = t_cursor;
    return value;
}


/*
 *  specific length/
 *      #RRGGBB
 */
static int
rgbhex(const char **cursor, int len, int rgbmax)
{
    const char *t_cursor = *cursor;
    int digits = 0, value = 0, c;

    *cursor = NULL;
    if (NULL == t_cursor || 0 == *t_cursor) {
        return -1;
    }

    while (digits < len && 0 != (c = (unsigned)*t_cursor)) {
        ++t_cursor;
        c = tolower(c);
        if (! isxdigit(c)) {
            return -1;
        }
        value = (value << 4) + HEXVALUE(c);
        ++digits;
    }
    if (digits != len) {
        return -1;
    }

    /* shift-up most significant */
    switch (rgbmax) {
    case 0x00ff:        /* 8 bits   */
        digits -= 2;
        break;
    case 0x0fff:        /* 12 bits  */
        digits -= 3;
        break;
    case 0xffff:        /* 16 bits  */
        digits -= 4;
        break;
    }
    while (digits++ < 0) {
        value <<= 4;
    }

    if (value > rgbmax) {
        return -1;
    }
    *cursor = t_cursor;
    return value;
}


int
rgb_export(char *buf, size_t length, const struct rgbvalue *rgb, int rgbmax)
{
    int len = 0;

    if (buf && rgb) {
        switch(rgb->type) {
        case COLORRGB_RGB:          /* rgb:<hex>/<hex>/<hex> */
            len = sxprintf(buf, length, "rgb:%x/%x/%x",
                    rgb->red, rgb->green, rgb->blue);
            break;

        case COLORRGB_RGBI:         /* rgbi:<r%>/<g%>/<b%> */
            len = sxprintf(buf, length, "rgbi:%.2f/%.2f/%.2f",
                    (float)rgb->red   / (float)rgbmax,
                    (float)rgb->green / (float)rgbmax,
                    (float)rgb->blue  / (float)rgbmax);
            break;

        case COLORRGB_RGBX:         /* #<hex><hex><hex> */
            len = sxprintf(buf, length, "#%02x%02x%02x",
                    rgb->red, rgb->green, rgb->blue);
            break;

        case COLORRGB_RGBWI:        /* rgb(<r>,<b<,<g>) */
            len = sxprintf(buf, length, "rgb(%d, %d, %d)",
                    rgb->red, rgb->green, rgb->blue);
            break;

        case COLORRGB_HSL: {        /* hsl(<h>,<s%>,<l%>) */
                float h, s, l;

                rgb2hsl(rgb, &h, &s, &l);
                len = sxprintf(buf, length, "hsl(%.0f, %.0f%%, %.0f%%)",
                        nearbyintf(h * 360), nearbyintf(s * 100.0f), nearbyintf(l * 100.0f));
            }
            break;

        case COLORRGB_RGBWF:        /* rgb(<r%>,<g%>,<b%>) */
            len = sxprintf(buf, length, "rgb(%d%%, %d%%, %d%%)",
                    Round(((double)rgb->red   / (double)rgbmax) * 100.0f),
                    Round(((double)rgb->green / (double)rgbmax) * 100.0f),
                    Round(((double)rgb->blue  / (double)rgbmax) * 100.0f));
            break;

        case COLORRGB_BAD:
        default:
            len = sxprintf(buf, length, "<bad>");
            break;
        }
    }
    return len;
}


void
rgb_256win(const int color, struct rgbvalue *rgb)
{
    if (color < 0) {
        rgb->red    = 0;
        rgb->green  = 0;
        rgb->blue   = 0;

    } else if (color < 16) {
        rgb->red    = win_basic16[color].red;
        rgb->green  = win_basic16[color].green;
        rgb->blue   = win_basic16[color].blue;

    } else {
        rgb_256xterm(color, rgb);

    }
    rgb->type = 256;
}


void
rgb_256xterm(const int color, struct rgbvalue *rgb)
{
    /*
     *  System Colors:
     *
     *      A value between 0 and 7 is used for the traditional 8 system colors in
     *      normal intensity. A value between 8 and 15 is used for the 8 system colors
     *      in bright intensity.
     *
     *  RGB Colors:
     *
     *      A value between 16 and 231 is used for RGB colors with each color having 6
     *      intensities. Red has a value between 0 and 5 multiplied by 36, Green has a
     *      value between 0 and 5 multiplied by 6, and Blue has a value between 0 and 5.
     *
     *      To display xterm RGB colors as 24 bit RGB colors the following values are
     *      suggested for the 6 intensities:
     *
     *               0x00, 0x5F, 0x87, 0xAF, 0xD7, and 0xFF.
     *
     *  Grey Scale:
     *
     *      A value between 232 and 255 is used for a 24 value greyscale.
     *
     *      For the greyscale start out at 0x08 for value 232 and add 10 for each
     *      increment, which gives 0xF8 for value 255. For system colors use 0x00 and
     *      0xC0 for normal intensities, and 0x80 and 0xFF for bright intensities.
     *
     *  Reference:
     *
     *      256colres.pl/h - xterm source.
     */
    static const uint8_t  colorcube[] = {
        0x00, 0x5F, 0x87, 0xAF, 0xD7, 0xFF
        };

    if (color < 0) {                            /* lower range */
        rgb->red    = 0;
        rgb->green  = 0;
        rgb->blue   = 0;

    } else if (color < 16) {                    /* 16 basic colors */
        rgb->red    = xterm_basic16[color].red;
        rgb->green  = xterm_basic16[color].green;
        rgb->blue   = xterm_basic16[color].blue;

    } else if (color < 232) {                   /* color cube color 6x6x6 */
        const unsigned idx = (unsigned)color - 16;

        rgb->red    = colorcube[(idx/36)%6];
        rgb->green  = colorcube[(idx/6)%6];
        rgb->blue   = colorcube[idx%6];

    } else if (color < 256) {                   /* grey scale */
        const unsigned grey = (((unsigned)color - 232U) * 10) + 8;

        rgb->red    = grey;
        rgb->green  = grey;
        rgb->blue   = grey;

    } else {                                    /* upper range */
        rgb->red    = 0xff;
        rgb->green  = 0xff;
        rgb->blue   = 0xff;
    }
    rgb->type = 256;
}


void
rgb_88xterm(const int color, struct rgbvalue *rgb)
{
    /*
     *  System Colors:
     *
     *      A value between 0 and 7 is used for the traditional 8 system colors in
     *      normal intensity. A value between 8 and 15 is used for the 8 system colors
     *      in bright intensity.
     *
     *      Have seen a number of alternatives including xterm-88color.dat included
     *      with ncurses.
     *
     *  RGB Colors:
     *
     *      To display xterm RGB colors as 24 bit RGB colors the following values are
     *      suggested for the 4 intensities:
     *
     *              0, 139, 205, 255
     *
     *  Grey Scale:
     *
     *      A value between 80 (idx=0) and 87 (idx=7) are used for greyscale values, using
     *      a 46.36 base scaled by depth omitting black and white.
     *
     *              level = (idx * 23.181818) +
     *                          (0 == idx ? 46.363636 : 46.363636 + 23.181818);
     *
     *  Reference:
     *
     *      88colres.pl/h - xterm source
     */
    static const uint8_t colorcube[] = {
        0, 139, 205, 255
        };

    static const uint8_t greyscale[] = {
        46, 92, 113, 139, 162, 185, 208, 231
        };

    if (color < 0) {                            /* lower range */
        rgb->red    = 0;
        rgb->green  = 0;
        rgb->blue   = 0;

    } else if (color < 16) {                    /* 16 basic colors */
        rgb->red    = xterm_basic16[color].red;
        rgb->green  = xterm_basic16[color].green;
        rgb->blue   = xterm_basic16[color].blue;

    } else if (color < 80) {                    /* color cube color 4x4x4 */
        unsigned idx = (unsigned)color - 16;
        rgb->red    = colorcube[(idx/16)%4];
        rgb->green  = colorcube[(idx/4)%4];
        rgb->blue   = colorcube[idx%4];

    } else if (color < 88) {                    /* grey scale */
        const uint8_t grey = greyscale[color - 80];
        rgb->red    = grey;
        rgb->green  = grey;
        rgb->blue   = grey;

    } else {                                    /* upper range */
        rgb->red    = 0xff;
        rgb->green  = 0xff;
        rgb->blue   = 0xff;
    }
    rgb->type = 88;
}


/*
 *  rgb_color256 ---
 *      RGB to 256-color palette, ignoring system-colors.
 */
int
rgb_color256(const struct rgbvalue *rgb)
{
    return rgb_searchtable(rgb_xterm256table() + BASIC16, 256 - BASIC16, GREY24, rgb);
}


/*
 *  rgb_win256 ---
 *      RGB to 256-color palette, including default windows system-colors.
 */
int
rgb_win256(const struct rgbvalue *rgb)
{
    return rgb_searchtable(rgb_win256table(), 256, GREY24, rgb);
}


/*
 *  rgb_xterm256 ---
 *      RGB to 256-color palette, including default Xterm system-colors.
 */
int
rgb_xterm256(const struct rgbvalue *rgb)
{
    return rgb_searchtable(rgb_xterm256table(), 256, GREY24, rgb);
}


int
rgb_xterm88(const struct rgbvalue *rgb)
{
    return rgb_searchtable(rgb_xterm88table(), 88, 8, rgb);
}


int
rgb_xterm16(const struct rgbvalue *rgb)
{
    return rgb_searchtable(xterm_basic16, 16, 0, rgb);
}


const struct rgbdef *
rgb_win256table(void)
{
    static struct rgbdef table[256];

    if (0 == table[255].red) {
        struct rgbdef *def = table;
        struct rgbvalue rgb;
        int color;

        for (color = 0; color < 256; ++color) {
            rgb_256win(color, &rgb);
            def->color  = color;
            def->red    = (uint8_t)rgb.red;
            def->green  = (uint8_t)rgb.green;
            def->blue   = (uint8_t)rgb.blue;
            ++def;
        }
    }
    return table;
}


const struct rgbdef *
rgb_xterm256table(void)
{
    static struct rgbdef table[256];

    if (0 == table[255].red) {
        struct rgbdef *def = table;
        struct rgbvalue rgb;
        int color;

        for (color = 0; color < 256; ++color) {
            rgb_256xterm(color, &rgb);
            def->color  = color;
            def->red    = (uint8_t)rgb.red;
            def->green  = (uint8_t)rgb.green;
            def->blue   = (uint8_t)rgb.blue;
            ++def;
        }
    }
    return table;
}


const struct rgbdef *
rgb_xterm88table(void)
{
    static struct rgbdef table[88];

    if (0 == table[87].red) {
        struct rgbdef *def = table;
        struct rgbvalue rgb;
        int color;

        for (color = 0; color < 88; ++color) {
            rgb_88xterm(color, &rgb);
            def->color  = color;
            def->red    = (uint8_t)rgb.red;
            def->green  = (uint8_t)rgb.green;
            def->blue   = (uint8_t)rgb.blue;
            ++def;
        }
    }
    return table;
}


static int
rgb_searchtable(const struct rgbdef *table, int maxval, int greyscale, const struct rgbvalue *rgb)
{
    const int red   = (int)rgb->red;
    const int green = (int)rgb->green;
    const int blue  = (int)rgb->blue;

    double smallest = 100000000.0;
    int color = 0, minval, i;

    /*
     *  Table selection
     */
    if (greyscale > 0 && red == green && red == blue && red != 0x00 && red != 0xff) {
        minval  = maxval - greyscale;           /* grey-scale table; exclude black&white */
        table  += minval;
    } else {                                    /* non grey-scale colours */
        minval  = 0;
        maxval -= greyscale;
    }

    /*
     *  Euclidean distance-metric
     */
    for (i = minval; i < maxval; ++i) {
        double distance, tmp;

        tmp = red   - (int)table->red;
        distance  = tmp * tmp;
        tmp = green - (int)table->green;
        distance += tmp * tmp;
        tmp = blue  - (int)table->blue;
        distance += tmp * tmp;
        if (distance < smallest) {
            smallest = distance;
            color = table->color;
        }
        ++table;
    }
    return color;
}


/*
 *  Translate HSL to RGB is simple, which was used to generate the tables.) In these
 *  algorithms, all three values (H, S and L) have been normalized to fractions 0..1:
 *
 *  Notes:
 *      The advantage of HSL over RGB is that it is far more intuitive: you can guess at the
 *      colors you want, and then tweak. It is also easier to create sets of matching colors (by
 *      keeping the hue the same and varying the lightness/darkness, and saturation)
 *
 *      If saturation is less than 0%, implementations must clip it to 0%. If the resulting value
 *      is outside the device gamut, implementations must clip it to the device gamut. This
 *      clipping should preserve the hue when possible, but is otherwise undefined. (In other
 *      words, the clipping is different from applying the rules for clipping of RGB colors after
 *      applying the algorithm below for converting HSL to RGB.)
 *
 *  References:
 *      http://www.w3.org/TR/css3-color
 */
void
hsl2rgb(float h, float s, float l, struct rgbvalue *rgb)
{
    float m2 = (float)(l <= 0.5f ? l * (s + 1.0f) : l + s - l * s);
    float m1 = (float)(l * 2.0f - m2);

    rgb->red   = (uint8_t)(huetorgb(m1, m2, h + (1.0f/3)) * 255);
    rgb->green = (uint8_t)(huetorgb(m1, m2, h           ) * 255);
    rgb->blue  = (uint8_t)(huetorgb(m1, m2, h - (1.0f/3)) * 255);
}


static float
huetorgb(float m1, float m2, float h)
{
    if (h < 0) h += 1;
    if (h > 1) h -= 1;
    if (h*6 < 1) return m1 + (m2 - m1) * h * 6;
    if (h*2 < 1) return m2;
    if (h*3 < 2) return m1 + (m2 - m1) * ((2.0f/3) - h) * 6;
    return m1;
}


void
rgb2hsl(const struct rgbvalue *rgb, float *h, float *s, float *l)
{
    const float red   = (float)rgb->red   / 255.0f;
    const float green = (float)rgb->green / 255.0f;
    const float blue  = (float)rgb->blue  / 255.0f;

#ifndef MIN
#define MIN(__a, __b)   (__a < __b ? __a : __b)
#define MAX(__a, __b)   (__a > __b ? __a : __b)
#endif

    float min   = MIN(red, MIN(green, blue));
    float max   = MAX(red, MAX(green, blue));
    float delta = max - min;

    float _h = 0;
    float _l = (min + max) / 2.0f;
    float _s = 0;

    if (_l > 0 && _l < 1) {
        _s = delta / (_l < 0.5f ? (2.0f * _l) : (2.0f - 2.0f * _l));
    }
    if (delta > 0) {
        if (max == red   && max != green) _h +=         (green - blue)  / delta;
        if (max == green && max != blue)  _h += (2.0f + (blue  - red)   / delta);
        if (max == blue  && max != red)   _h += (4.0f + (red   - green) / delta);
        _h /= 6;
    }
    *h = _h, *s = _s; *l = _l;
}

/*end*/
