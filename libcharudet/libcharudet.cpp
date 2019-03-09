#include <edidentifier.h>
__CIDENT_RCSID(cr_libchardet_cpp,"$Id: libcharudet.cpp,v 1.6 2018/10/04 14:43:24 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: libcharudet.cpp,v 1.6 2018/10/04 14:43:24 cvsuser Exp $
 *
 * libchardet interface.
 *
 * Copyright (c) 1998 - 2018, Adam Young.
 * All rights reserved.
 *
 * This file is part of the GRIEF Editor.
 *
 * The GRIEF Editor is free software: you can redistribute it
 * and/or modify it under the terms of the GRIEF Editor License.
 *
 * Redistributions of source code must retain the above copyright
 * notice, and must be distributed with the license document above.
 *
 * Redistributions in binary form must reproduce the above copyright
 * notice, and must include the license document above in
 * the documentation and/or other materials provided with the
 * distribution.
 *
 * The GRIEF Editor is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * License for more details.
 * ==end==
 */

#include <edsym.h>
#include <edtrace.h>
#include <string.h>

#include "src/nscore.h"
#include "src/nsUniversalDetector.h"

#include "libcharudet.h"

class nschardet : private nsUniversalDetector {
public:
    nschardet(int _filter = NS_FILTER_ALL)
            : nsUniversalDetector(_filter)
        { }

    const char *        analysis(const char *buffer, int length)
        {
            const int ret = nsUniversalDetector::HandleData(buffer, length);
            nsUniversalDetector::DataEnd();

            trace_log("chardet (buffer:%p,length:%d) = %d (%s)\n", \
                            buffer, length, ret, (mDetectedCharset ? mDetectedCharset : ""));
            if (NS_OK == ret) {
                return mDetectedCharset;
            }
            return 0;
        }

protected:
    virtual void        Report(const char *msg)
        {
            trace_log("chardet <%s>\n", msg);
        }
};


extern "C" int
chardet_analysis(const char *buffer, int length, char *encoding, int encoding_length)
{
    nschardet chardet;
    const char *result;

    if (0 != (result = chardet.analysis(buffer, length))) {
        if (encoding && encoding_length > 0) {
            strncpy(encoding, result, encoding_length);
            encoding[encoding_length - 1] = 0;
        }
        return 1;
    }
    return 0;
}
/*end*/
