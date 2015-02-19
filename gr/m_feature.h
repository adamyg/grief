#ifndef GR_M_FEATURE_H_INCLUDED
#define GR_M_FEATURE_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_m_feature_h,"$Id: m_feature.h,v 1.7 2014/10/22 02:33:03 ayoung Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: m_feature.h,v 1.7 2014/10/22 02:33:03 ayoung Exp $
 * Feature support.
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

extern void                 do_set_feature(void);
extern void                 inq_feature(void);

extern const char * const   x_features[];

__CEND_DECLS

#endif /*GR_M_FEATURE_H_INCLUDED*/
