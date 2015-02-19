#ifndef GR_WORD_H_INCLUDED
#define GR_WORD_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_word_h,"$Id: word.h,v 1.15 2014/10/22 02:33:25 ayoung Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: word.h,v 1.15 2014/10/22 02:33:25 ayoung Exp $
 * External data representation.
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

struct CM;

extern const unsigned       sizeof_atoms[];
extern const char *         nameof_atoms[];

extern void                 cm_xdr_import(struct CM *cm);
extern void                 cm_xdr_export(struct CM *cm);

extern uint16_t             WPUT16(const uint16_t n);
extern uint32_t             WPUT32(const uint32_t n);
extern uint16_t             WGET16(const uint16_t n);
extern uint32_t             WGET32(const uint32_t n);
extern void                 WGET32_block(uint32_t *warray, int size);

extern void                 LPUT16(LIST *lp, const int16_t n);
extern int16_t              LGET16(const LIST *lp);

extern void                 LPUT32(LIST *lp, const int32_t n);
extern int32_t              LGET32(const LIST *lp);

#if (defined(HAVE_LONG_LONG_INT) && (SIZEOF_LONG_LONG >= 8)) \
        || (SIZEOF_INT >= 8)
extern void                 LPUT64(LIST *lp, const int64_t n);
extern int64_t              LGET64(const LIST *lp);
#endif

extern void                 LPUT_PTR(LIST *lp, const void *);
extern void *               LGET_PTR(const LIST *lp);

extern void                 LPUT_FLOAT(LIST *lp, const double val);
extern double               LGET_FLOAT(const LIST *lp);

#define                     LPUT_LEN(lp, n)     LPUT16((lp), (n))
#define                     LGET_LEN(lp)        LGET16(lp)

#define                     LPUT_ID(lp, i)      LPUT16((lp), (i))
#define                     LGET_ID(lp)         LGET16(lp)

#define                     LPUT_INT(lp, n)     LPUT32((lp), (int32_t)(n))
#define                     LGET_INT(lp)        LGET32(lp)

#define                     LGET_PTR2(type, lp) ((type *)LGET_PTR(lp))

__CEND_DECLS

#endif /*GR_WORD_H_INCLUDED*/
