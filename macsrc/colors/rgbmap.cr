/* -*- mode: cr; indent-width: 8; -*- */
/* $Id: rgbmap.cr,v 1.1 2014/11/25 04:44:50 ayoung Exp $
 * RGB color mapper.
 *
 */

#include "../grief.h"
#include "rgbmap.h"


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
// colour ---
//      Returns the palette index to approximate the given R/G/B colour levels.
//
static string
colour(int r, int g, int b)
{
        int gx = grey_number(r);
        int gy = grey_number(g);
        int gz = grey_number(b);

        // Get the closest colour
        int x = rgb_number(r);
        int y = rgb_number(g);
        int z = rgb_number(b);

        if (gx == gy && gy == gz) {
                //  There are two possibilities
                int dgr     = grey_level(gx) - r;
                int dgg     = grey_level(gy) - g;
                int dgb     = grey_level(gz) - b;
                int dgrey   = (dgr * dgr) + (dgg * dgg) + (dgb * dgb);
                int dr      = rgb_level(gx)  - r;
                int dg      = rgb_level(gy)  - g;
                int db      = rgb_level(gz)  - b;
                int drgb    = (dr * dr) + (dg * dg) + (db * db);

                if (dgrey < drgb) {
                        // Use the grey
                        return grey_colour(gx);
                } else {
                        // Use the colour
                        return rgb_colour(x, y, z);
                }
        } else {
                // Only one possibility
                return rgb_colour(x, y, z);
        }
}


//
// RGBMap ---
//      Returns the palette index to approximate the 'rrggbb' hex string
//      for the specified 'colordepth'.
//
string
RGBMap(string rgb, int colordepth)
{
        int r,g,b;

        UNUSED(colordepth);
        sscanf(rgb, "%2x%2x%2x", r, g, b);
        return colour(r, g, b);
}


