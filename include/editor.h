#ifndef GR_EDITOR_H_INCLUDED
#define GR_EDITOR_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_editor_h,"$Id: editor.h,v 1.33 2015/02/19 00:16:55 ayoung Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: editor.h,v 1.33 2015/02/19 00:16:55 ayoung Exp $
 * GRIEF Editor global definitions.
 *
 *
 *
 * Copyright (c) 1998 - 2015, Adam Young.
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

#include <edstruct.h>
#include <edsym.h>

#include <libmisc.h>                            /* Miscellaneous functions */
#include <libsplay.h>                           /* splay binary trees. */
#include <stype.h>                              /* Integer object store */
#include <vm_alloc.h>                           /* Memory allocation interface */

#include <edmacros.h>                           /* grunch/gm macro constants */
#include <edrefobj.h>                           /* Reference object utilities */

__CBEGIN_DECLS

/*
 *  config.c definitions
 */

    /*FIXME: edpackage.h.in*/

#define ApplicationName         "GRIEF"
#define ApplicationEmail        "griefedit"/**/"@"/**/"gmail"/**/".com"

extern const char *             x_machtype;

extern const char *             x_grpath;
extern const char *             x_grfile;
extern const char *             x_grhelp;
extern const char *             x_grflags;

extern const char *             x_default_term;
extern const char *             x_grtermcap;

extern const char *             x_col_table_dark[];
extern const char *             x_col_table_light[];
extern const char *             x_col_windows[];

extern const struct k_tbl       x_key_table[];

__CEND_DECLS

#endif  /*GR_EDITOR_H_INCLUDED*/
