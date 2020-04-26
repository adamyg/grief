#ifndef GR_M_FLOAT_H_INCLUDED
#define GR_M_FLOAT_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_m_float_h,"$Id: m_float.h,v 1.8 2020/04/21 00:01:56 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: m_float.h,v 1.8 2020/04/21 00:01:56 cvsuser Exp $
 * Floating point primitives.
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

extern void                 do_acos(void);
extern void                 do_asin(void);
extern void                 do_atan(void);
extern void                 do_atan2(void);
extern void                 do_ceil(void);
extern void                 do_cos(void);
extern void                 do_cosh(void);
extern void                 do_exp(void);
extern void                 do_fabs(void);
extern void                 do_floor(void);
extern void                 do_fmod(void);
extern void                 do_frexp(void);
extern void                 do_isclose(void);
extern void                 do_isfinite(void);
extern void                 do_isinf(void);
extern void                 do_isnan(void);
extern void                 do_ldexp(void);
extern void                 do_log(void);
extern void                 do_log10(void);
extern void                 do_modf(void);
extern void                 do_pow(void);
extern void                 do_sin(void);
extern void                 do_sinh(void);
extern void                 do_sqrt(void);
extern void                 do_tan(void);
extern void                 do_tanh(void);

__CEND_DECLS

#endif /*GR_M_FLOAT_H_INCLUDED*/
