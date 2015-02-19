#ifndef GR_SEARCH_H_INCLUDED
#define GR_SEARCH_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_search_h,"$Id: search.h,v 1.11 2014/10/22 02:33:18 ayoung Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: search.h,v 1.11 2014/10/22 02:33:18 ayoung Exp $
 * Search interface.
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
#include "regexp.h"                             /* regopts */

__CBEGIN_DECLS

extern void                 search_init(void);
extern void                 search_options(struct regopts *regopts, const int fwd_search, const int flags);
extern void                 search_defoptions(struct regopts *regopts);

__CEND_DECLS

#endif /*GR_SEARCH_H_INCLUDED*/
