#ifndef GR_M_REGION_H_INCLUDED
#define GR_M_REGION_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_m_region_h,"$Id: m_region.h,v 1.3 2014/10/22 02:33:06 ayoung Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: m_region.h,v 1.3 2014/10/22 02:33:06 ayoung Exp $
 * Block primitives.
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

extern void                 do_delete_block(void);
extern void                 do_write_block(void);

__CEND_DECLS

#endif /*GR_M_REGION_H_INCLUDED*/
