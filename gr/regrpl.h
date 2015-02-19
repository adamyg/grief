#ifndef GR_REGRPL_H_INCLUDED
#define GR_REGRPL_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_regrpl_h,"$Id: regrpl.h,v 1.3 2014/10/22 02:33:17 ayoung Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: regrpl.h,v 1.3 2014/10/22 02:33:17 ayoung Exp $
 * Regular expression replacement.
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

typedef uint8_t REGRPL;                         /* Replacement program unit type. */

typedef struct regrpl {
    REGRPL *                prog;               /* Compiled replacement expression. */
    const char *            buf;                /* Buffer. */
    size_t                  len;                /* Length, in bytes. */
    size_t                  _dsiz;              /* Data buffer size, in bytes. */
    size_t                  _psiz;              /* Program buffer size, in bytes. */
    void *                  _dmem;              /* Data memory buffer. */
    void *                  _pmem;              /* Program memory buffer. */
} regrpl_t;

struct regprog;

extern int                  regrpl_comp(regrpl_t *rpl, const char *replace, int mode, int flags);
extern int                  regrpl_exec(const struct regprog *prog, regrpl_t *rpl);
extern void                 regrpl_print(const regrpl_t *rpl);
extern void                 regrpl_reset(regrpl_t *prog);
extern void                 regrpl_free(regrpl_t *rpl);

__CEND_DECLS

#endif /*GR_REGRPL_H_INCLUDED*/
