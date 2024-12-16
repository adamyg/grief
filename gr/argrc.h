#ifndef GR_ARGRC_H_INCLUDED
#define GR_ARGRC_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_argrc_h,"$Id: argrc.h,v 1.3 2024/12/05 17:26:52 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: argrc.h,v 1.3 2024/12/05 17:26:52 cvsuser Exp $
 * Command line resource processing.
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

#include <edtypes.h>

__CBEGIN_DECLS

struct argrc {
    unsigned argc;                  /* count */
    const char * const *argv;       /* argument vector, NULL terminated */
    void *data;                     /* data buffer */
};

extern int          argrc_load(const char *file, struct argrc *rc);
extern int          argrc_load2(const char *argv0, const char *file, struct argrc *rc);

extern int          argrc_parse(const char *buffer, struct argrc *rc);
extern int          argrc_parse2(const char *argv0, const char* buffer, struct argrc* rc);

extern void         argrc_free(struct argrc *ret);

__CEND_DECLS

#endif /*GR_ARGRC_H_INCLUDED*/
