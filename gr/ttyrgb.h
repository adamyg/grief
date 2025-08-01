#ifndef GR_TTYRGB_H_INCLUDED
#define GR_TTYRGB_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_ttyrgb_h,"$Id: ttyrgb.h,v 1.11 2025/02/07 03:03:22 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: ttyrgb.h,v 1.11 2025/02/07 03:03:22 cvsuser Exp $
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

#include <edsym.h>

__CBEGIN_DECLS

struct rgbvalue {
    int         type;
    unsigned    red;
    unsigned    green;
    unsigned    blue;
};

struct rgbdef {
    int         color;
    uint8_t     red;
    uint8_t     green;
    uint8_t     blue;
};

extern int                  rgb_import(const char *name, size_t length, struct rgbvalue *rgb, int rgbmax);
extern int                  rgb_export(char *buf, size_t length, const struct rgbvalue *rgb, int rgbmax);

extern void                 rgb_256win(const int color, struct rgbvalue *rgb);
extern void                 rgb_256xterm(const int color, struct rgbvalue *rgb);
extern void                 rgb_88xterm(const int color, struct rgbvalue *rgb);

extern int                  rgb_color256(const struct rgbvalue *rgb);
extern int                  rgb_win256(const struct rgbvalue *rgb);
extern int                  rgb_xterm256(const struct rgbvalue *rgb);
extern int                  rgb_xterm88(const struct rgbvalue *rgb);
extern int                  rgb_xterm16(const struct rgbvalue *rgb);

extern const struct rgbdef *rgb_win256table(void);
extern const struct rgbdef *rgb_xterm256table(void);
extern const struct rgbdef *rgb_xterm88table(void);

__CEND_DECLS

#endif /*GR_TTYRGB_H_INCLUDED*/
