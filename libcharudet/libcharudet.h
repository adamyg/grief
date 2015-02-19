#ifndef GR_LIBCHARUDET_H_INCLUDED
#define GR_LIBCHARUDET_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_libchardet_h,"$Id: libcharudet.h,v 1.3 2014/10/22 02:33:39 ayoung Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: libcharudet.h,v 1.3 2014/10/22 02:33:39 ayoung Exp $
 * libchardet interface
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

#ifdef  __cplusplus
extern "C" {
#endif

extern int              chardet_analysis(const char *buffer, int length, char *encoding, int encoding_length);

#ifdef  __cplusplus
}
#endif

#endif /*GR_LIBCHARUDET_H_INCLUDED*/
