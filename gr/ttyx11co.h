#ifndef GR_TTYX11CO_H_INCLUDED
#define GR_TTYX11CO_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_ttyx11co_h,"$Id: ttyx11co.h,v 1.5 2014/10/22 02:33:24 ayoung Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: ttyx11co.h,v 1.5 2014/10/22 02:33:24 ayoung Exp $
 * Color table ...
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

static const int        xgr_colormap[COLOR_NONE] = {
    /*
     *  GRIEF/BRIEF palette -> Color Table
     */
    0,                          /* BLACK     */
    12,                         /* BLUE      */
    10,                         /* GREEN     */
    14,                         /* CYAN      */
    9,                          /* RED       */
    13,                         /* MAGENTA   */
    130,                        /* BROWN     */
    248,                        /* WHITE     */

    7,                          /* GREY      */
    81,                         /* LTBLUE    */
    121,                        /* LTGREEN   */
    159,                        /* LTCYAN    */
    224,                        /* LTRED     */
    225,                        /* LTMAGENTA */
    11,                         /* YELLOW    */
    15,                         /* LTWHITE   */

    242,                        /* DKGREY    */
    4,                          /* DKBLUE    */
    2,                          /* DKGREEN   */
    6,                          /* DKCYAN    */
    1,                          /* DKRED     */
    5,                          /* DKMAGENTA */
    130,                        /* DKYELLOW  */
    229                         /* LTYELLOW  */
    };

static const char *     xgr_color256[256] = {
    /*
     *  Color Table
     */
    "Black",                    /* 0:  black          (#000000) */
    "Red3",                     /* 1:  red            (#CD0000) */
    "Green3",                   /* 2:  green          (#00CD00) */
    "Yellow3",                  /* 3:  yellow         (#CDCD00) */
    "Blue3",                    /* 4:  blue           (#0000CD) */
    "Magenta3",                 /* 5:  magenta        (#CD00CD) */
    "Cyan3",                    /* 6:  cyan           (#00CDCD) */
    "AntiqueWhite",             /* 7:  white          (#FAEBD7) */
    "Grey25",                   /* 8:  bright black   (#404040) */
    "Red",                      /* 9:  bright red     (#FF0000) */
    "Green",                    /* 10: bright green   (#00FF00) */
    "Yellow",                   /* 11: bright yellow  (#FFFF00) */
    "Blue",                     /* 12: bright blue    (#0000FF) */
    "Magenta",                  /* 13: bright magenta (#FF00FF) */
    "Cyan",                     /* 14: bright cyan    (#00FFFF) */
    "White",                    /* 15: bright white   (#FFFFFF) */
    "rgb:00/00/00",             /* 16-255 */
    "rgb:00/00/2a",
    "rgb:00/00/55",
    "rgb:00/00/7f",
    "rgb:00/00/aa",
    "rgb:00/00/d4",
    "rgb:00/2a/00",
    "rgb:00/2a/2a",
    "rgb:00/2a/55",
    "rgb:00/2a/7f",
    "rgb:00/2a/aa",
    "rgb:00/2a/d4",
    "rgb:00/55/00",
    "rgb:00/55/2a",
    "rgb:00/55/55",
    "rgb:00/55/7f",
    "rgb:00/55/aa",
    "rgb:00/55/d4",
    "rgb:00/7f/00",
    "rgb:00/7f/2a",
    "rgb:00/7f/55",
    "rgb:00/7f/7f",
    "rgb:00/7f/aa",
    "rgb:00/7f/d4",
    "rgb:00/aa/00",
    "rgb:00/aa/2a",
    "rgb:00/aa/55",
    "rgb:00/aa/7f",
    "rgb:00/aa/aa",
    "rgb:00/aa/d4",
    "rgb:00/d4/00",
    "rgb:00/d4/2a",
    "rgb:00/d4/55",
    "rgb:00/d4/7f",
    "rgb:00/d4/aa",
    "rgb:00/d4/d4",
    "rgb:2a/00/00",
    "rgb:2a/00/2a",
    "rgb:2a/00/55",
    "rgb:2a/00/7f",
    "rgb:2a/00/aa",
    "rgb:2a/00/d4",
    "rgb:2a/2a/00",
    "rgb:2a/2a/2a",
    "rgb:2a/2a/55",
    "rgb:2a/2a/7f",
    "rgb:2a/2a/aa",
    "rgb:2a/2a/d4",
    "rgb:2a/55/00",
    "rgb:2a/55/2a",
    "rgb:2a/55/55",
    "rgb:2a/55/7f",
    "rgb:2a/55/aa",
    "rgb:2a/55/d4",
    "rgb:2a/7f/00",
    "rgb:2a/7f/2a",
    "rgb:2a/7f/55",
    "rgb:2a/7f/7f",
    "rgb:2a/7f/aa",
    "rgb:2a/7f/d4",
    "rgb:2a/aa/00",
    "rgb:2a/aa/2a",
    "rgb:2a/aa/55",
    "rgb:2a/aa/7f",
    "rgb:2a/aa/aa",
    "rgb:2a/aa/d4",
    "rgb:2a/d4/00",
    "rgb:2a/d4/2a",
    "rgb:2a/d4/55",
    "rgb:2a/d4/7f",
    "rgb:2a/d4/aa",
    "rgb:2a/d4/d4",
    "rgb:55/00/00",
    "rgb:55/00/2a",
    "rgb:55/00/55",
    "rgb:55/00/7f",
    "rgb:55/00/aa",
    "rgb:55/00/d4",
    "rgb:55/2a/00",
    "rgb:55/2a/2a",
    "rgb:55/2a/55",
    "rgb:55/2a/7f",
    "rgb:55/2a/aa",
    "rgb:55/2a/d4",
    "rgb:55/55/00",
    "rgb:55/55/2a",
    "rgb:55/55/55",
    "rgb:55/55/7f",
    "rgb:55/55/aa",
    "rgb:55/55/d4",
    "rgb:55/7f/00",
    "rgb:55/7f/2a",
    "rgb:55/7f/55",
    "rgb:55/7f/7f",
    "rgb:55/7f/aa",
    "rgb:55/7f/d4",
    "rgb:55/aa/00",
    "rgb:55/aa/2a",
    "rgb:55/aa/55",
    "rgb:55/aa/7f",
    "rgb:55/aa/aa",
    "rgb:55/aa/d4",
    "rgb:55/d4/00",
    "rgb:55/d4/2a",
    "rgb:55/d4/55",
    "rgb:55/d4/7f",
    "rgb:55/d4/aa",
    "rgb:55/d4/d4",
    "rgb:7f/00/00",
    "rgb:7f/00/2a",
    "rgb:7f/00/55",
    "rgb:7f/00/7f",
    "rgb:7f/00/aa",
    "rgb:7f/00/d4",
    "rgb:7f/2a/00",
    "rgb:7f/2a/2a",
    "rgb:7f/2a/55",
    "rgb:7f/2a/7f",
    "rgb:7f/2a/aa",
    "rgb:7f/2a/d4",
    "rgb:7f/55/00",
    "rgb:7f/55/2a",
    "rgb:7f/55/55",
    "rgb:7f/55/7f",
    "rgb:7f/55/aa",
    "rgb:7f/55/d4",
    "rgb:7f/7f/00",
    "rgb:7f/7f/2a",
    "rgb:7f/7f/55",
    "rgb:7f/7f/7f",
    "rgb:7f/7f/aa",
    "rgb:7f/7f/d4",
    "rgb:7f/aa/00",
    "rgb:7f/aa/2a",
    "rgb:7f/aa/55",
    "rgb:7f/aa/7f",
    "rgb:7f/aa/aa",
    "rgb:7f/aa/d4",
    "rgb:7f/d4/00",
    "rgb:7f/d4/2a",
    "rgb:7f/d4/55",
    "rgb:7f/d4/7f",
    "rgb:7f/d4/aa",
    "rgb:7f/d4/d4",
    "rgb:aa/00/00",
    "rgb:aa/00/2a",
    "rgb:aa/00/55",
    "rgb:aa/00/7f",
    "rgb:aa/00/aa",
    "rgb:aa/00/d4",
    "rgb:aa/2a/00",
    "rgb:aa/2a/2a",
    "rgb:aa/2a/55",
    "rgb:aa/2a/7f",
    "rgb:aa/2a/aa",
    "rgb:aa/2a/d4",
    "rgb:aa/55/00",
    "rgb:aa/55/2a",
    "rgb:aa/55/55",
    "rgb:aa/55/7f",
    "rgb:aa/55/aa",
    "rgb:aa/55/d4",
    "rgb:aa/7f/00",
    "rgb:aa/7f/2a",
    "rgb:aa/7f/55",
    "rgb:aa/7f/7f",
    "rgb:aa/7f/aa",
    "rgb:aa/7f/d4",
    "rgb:aa/aa/00",
    "rgb:aa/aa/2a",
    "rgb:aa/aa/55",
    "rgb:aa/aa/7f",
    "rgb:aa/aa/aa",
    "rgb:aa/aa/d4",
    "rgb:aa/d4/00",
    "rgb:aa/d4/2a",
    "rgb:aa/d4/55",
    "rgb:aa/d4/7f",
    "rgb:aa/d4/aa",
    "rgb:aa/d4/d4",
    "rgb:d4/00/00",
    "rgb:d4/00/2a",
    "rgb:d4/00/55",
    "rgb:d4/00/7f",
    "rgb:d4/00/aa",
    "rgb:d4/00/d4",
    "rgb:d4/2a/00",
    "rgb:d4/2a/2a",
    "rgb:d4/2a/55",
    "rgb:d4/2a/7f",
    "rgb:d4/2a/aa",
    "rgb:d4/2a/d4",
    "rgb:d4/55/00",
    "rgb:d4/55/2a",
    "rgb:d4/55/55",
    "rgb:d4/55/7f",
    "rgb:d4/55/aa",
    "rgb:d4/55/d4",
    "rgb:d4/7f/00",
    "rgb:d4/7f/2a",
    "rgb:d4/7f/55",
    "rgb:d4/7f/7f",
    "rgb:d4/7f/aa",
    "rgb:d4/7f/d4",
    "rgb:d4/aa/00",
    "rgb:d4/aa/2a",
    "rgb:d4/aa/55",
    "rgb:d4/aa/7f",
    "rgb:d4/aa/aa",
    "rgb:d4/aa/d4",
    "rgb:d4/d4/00",
    "rgb:d4/d4/2a",
    "rgb:d4/d4/55",
    "rgb:d4/d4/7f",
    "rgb:d4/d4/aa",
    "rgb:d4/d4/d4",
    "rgb:08/08/08",
    "rgb:12/12/12",
    "rgb:1c/1c/1c",
    "rgb:26/26/26",
    "rgb:30/30/30",
    "rgb:3a/3a/3a",
    "rgb:44/44/44",
    "rgb:4e/4e/4e",
    "rgb:58/58/58",
    "rgb:62/62/62",
    "rgb:6c/6c/6c",
    "rgb:76/76/76",
    "rgb:80/80/80",
    "rgb:8a/8a/8a",
    "rgb:94/94/94",
    "rgb:9e/9e/9e",
    "rgb:a8/a8/a8",
    "rgb:b2/b2/b2",
    "rgb:bc/bc/bc",
    "rgb:c6/c6/c6",
    "rgb:d0/d0/d0",
    "rgb:da/da/da",
    "rgb:e4/e4/e4",
    "rgb:ee/ee/ee"
    };

#endif /*GR_TTYX11CO_H_INCLUDED*/
