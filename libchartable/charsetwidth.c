#include <edidentifier.h>
__CIDENT_RCSID(gr_charsetwidth_c,"$Id: charsetwidth.c,v 1.17 2021/06/19 09:40:16 cvsuser Exp $")

/*
    ------------------------------------------------------------------------------
    The MIT License (MIT)

    Copyright (c) 2014 Jeff Quast <contact@jeffquast.com>

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.

    ------------------------------------------------------------------------------

    Markus Kuhn -- 2007-05-26 (Unicode 5.0)

    Permission to use, copy, modify, and distribute this software
    for any purpose and without fee is hereby granted. The author
    disclaims all warranties with regard to this software.

    ------------------------------------------------------------------------------
 */

#include <editor.h>
#include "libchartable.h"

struct width_interval {
        int start;
        int end;
};


#include "table_wide.h"
#include "table_zero.h"


#define _elementsof(__type) (sizeof(__type)/sizeof(__type[0]))

struct width_version {
        const char *label;
        unsigned long value;
        const struct width_interval *zero;
        int zero_elements;
        const struct width_interval *width;
        int width_elements;
};


static const struct width_version
VERSIONS[] = {
        { "4.1.0",      40100,  zero_4_1_0,     _elementsof(zero_4_1_0),    width_4_1_0,    _elementsof(width_4_1_0)  },
        { "5.0.0",      50000,  zero_5_0_0,     _elementsof(zero_5_0_0),    width_5_0_0,    _elementsof(width_5_0_0)  },
        { "5.1.0",      50100,  zero_5_1_0,     _elementsof(zero_5_1_0),    width_5_1_0,    _elementsof(width_5_1_0)  },
        { "5.2.0",      50200,  zero_5_2_0,     _elementsof(zero_5_2_0),    width_5_2_0,    _elementsof(width_5_2_0)  },
        { "6.0.0",      60000,  zero_6_0_0,     _elementsof(zero_6_0_0),    width_6_0_0,    _elementsof(width_6_0_0)  },
        { "6.1.0",      60100,  zero_6_1_0,     _elementsof(zero_6_1_0),    width_6_1_0,    _elementsof(width_6_1_0)  },
        { "6.2.0",      60200,  zero_6_2_0,     _elementsof(zero_6_2_0),    width_6_2_0,    _elementsof(width_6_2_0)  },
        { "6.3.0",      60300,  zero_6_3_0,     _elementsof(zero_6_3_0),    width_6_3_0,    _elementsof(width_6_3_0)  },
        { "7.0.0",      70000,  zero_7_0_0,     _elementsof(zero_7_0_0),    width_7_0_0,    _elementsof(width_7_0_0)  },
        { "8.0.0",      80000,  zero_8_0_0,     _elementsof(zero_8_0_0),    width_8_0_0,    _elementsof(width_8_0_0)  },
        { "9.0.0",      90000,  zero_9_0_0,     _elementsof(zero_9_0_0),    width_9_0_0,    _elementsof(width_9_0_0)  },
        { "10.0.0",    100000,  zero_10_0_0,    _elementsof(zero_10_0_0),   width_10_0_0,   _elementsof(width_10_0_0) },
        { "11.0.0",    110000,  zero_11_0_0,    _elementsof(zero_11_0_0),   width_11_0_0,   _elementsof(width_11_0_0) },
        { "12.1.0",    120100,  zero_12_1_0,    _elementsof(zero_12_1_0),   width_12_1_0,   _elementsof(width_12_1_0) },
        { "13.0.0",    130000,  zero_13_0_0,    _elementsof(zero_13_0_0),   width_13_0_0,   _elementsof(width_13_0_0) }
        };

static const struct width_version *version = VERSIONS + (_elementsof(VERSIONS) - 1);


static int intable(const struct width_interval *table, int table_length, int c) {
        // Binary search in table.
        int bot = 0;
        int top = table_length - 1;

        // First quick check for Latin1 etc. characters.
        if (c < table[0].start) return false;

        while (top >= bot) {
                int mid = (bot + top) / 2;
                if (table[mid].end < c) {
                        bot = mid + 1;
                } else if (table[mid].start > c) {
                        top = mid - 1;
                } else {
                        return true;
                }
        }
        return false;
}


int
charset_width_set_version(const char *label)
{
        if (version) {
                const struct width_version *cursor,
                        *end = VERSIONS + _elementsof(VERSIONS);
                unsigned a = 0, b = 0, c = 0;

                for (cursor = VERSIONS; cursor != end; ++cursor) {
                        if (0 == strcmp(label, cursor->label)) { //match
                                version = cursor;
                                return version->value;
                        }
                }

                if (sscanf(label, "%2u.%2u.%2u", &a, &b, &c) >= 1) { //closest match
                        const unsigned value = (a * 10000) + (b * 100) + c;
                        if (value) {
                                for (cursor = end; cursor-- != VERSIONS;) {
                                        if (value <= cursor->value) {
                                                version = cursor;
                                                return version->value;
                                        }
                                }
                        }
                }
        }
        return 0;
}


const char *
charset_width_version(void)
{
        return version->label;
}



int
charset_width_ucs(int32_t ucs, int bad)
{
#if (USYSTEM)
        return wcwidth(ucs);

#else
        // NOTE: created by hand, there isn't anything identifiable other than
        // general Cf category code to identify these, and some characters in Cf
        // category code are of non-zero width.
        if (ucs == 0 ||
                        ucs == 0x034F ||
                        (0x200B <= ucs && ucs <= 0x200F) ||
                        ucs == 0x2028 ||
                        ucs == 0x2029 ||
                        (0x202A <= ucs && ucs <= 0x202E) ||
                        (0x2060 <= ucs && ucs <= 0x2063)) {
                return 0;
        }

        // C0/C1 control characters.
        if (ucs < 32 || (0x07F <= ucs && ucs < 0x0A0)) return bad;

        // Combining characters with zero width.
        if (intable(version->zero, version->zero_elements, ucs))
                return 0;

        return intable(version->width, version->width_elements, ucs) ? 2 : 1;
#endif
}


int
charset_swidth_ucs(const int32_t *pwcs, size_t n)
{
        int w, width = 0;

        for (;*pwcs && n-- > 0; ++pwcs) {
                if ((w = charset_width_ucs(*pwcs, -1)) < 0) {
                        return -1;
                }
                width += w;
        }
        return width;
}

/*end*/
