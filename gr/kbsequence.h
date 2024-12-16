#ifndef GR_KBSEQUENCE_H_INCLUDED
#define GR_KBSEQUENCE_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_kbsequence_h,"$Id: kbsequence.h,v 1.2 2024/11/26 15:51:25 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: kbsequence.h,v 1.2 2024/11/26 15:51:25 cvsuser Exp $
 * Keyboard input sequences.
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

typedef struct keyseq {
    KEY  ks_code;                               /* internal keycode */
    char ks_buf[1];                             /* array of characters; null terminated */
} keyseq_t;

extern void                 kbsequence_init(void);
extern void                 kbsequence_shutdown(void);

extern const keyseq_t *     kbsequence_lookup(const char *seq);
extern const keyseq_t *     kbsequence_update(const char *seq, KEY key);

extern const keyseq_t *     kbsequence_match(const char *seq, unsigned seqlen, unsigned *ambiguous, const keyseq_t **partial);

extern const keyseq_t * const * kbsequence_flatten(unsigned *count);

__CEND_DECLS

#endif /*GR_KBSEQUENCE_H_INCLUDED*/
