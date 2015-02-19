#ifndef GR_LANGUAGE_H_INCLUDED
#define GR_LANGUAGE_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_language_h,"$Id: language.h,v 1.18 2014/10/22 02:32:59 ayoung Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: language.h,v 1.18 2014/10/22 02:32:59 ayoung Exp $
 * Language support, runtime/inline compiler.
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
#include <edcm.h>

__CBEGIN_DECLS

extern void                 cm_init(int cm);
extern void                 cm_shutdown(void);

extern int                  cm_push(const char *fname);
extern int                  cm_push2(const char *oname, const char *buf, unsigned length);
extern int                  cm_parse(void (*execute)(const LIST *lp, int size), const char **includes);
extern void                 cm_pop(void);

extern void                 cm_error(const char *str, ...) __ATTRIBUTE_FORMAT__((printf, 1, 2));

__CEND_DECLS

#endif /*GR_LANGUAGE_H_INCLUDED*/
