#ifndef GR_TTYCFG_H_INCLUDED
#define GR_TTYCFG_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_ttycfg_h,"$Id: ttycfg.h,v 1.1 2024/10/15 10:48:57 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: ttycfg.h,v 1.1 2024/10/15 10:48:57 cvsuser Exp $
 * TTY configuration.
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

__CBEGIN_DECLS

typedef struct {
    const char *        termfname;              /* function name */
    unsigned            userdef;                /* is userdef attribute */
    const char *        termcapname;            /* termcap name */
    const char *        terminfoname;           /* terminfo name */
    const char *        comment;                /* comment string */
#define TC_FNAME(__fname)       #__fname, 0
#define TC_UNAME(__uname)       "userdef_" #__uname, 1
#define TC_KNAME(__kname)       "keydef_" #__kname, 1
#define TC_DESC(__desc)         __desc
} TermVal_t;

typedef struct {
    TermVal_t           term;
    int                 key;
#define TC_TOKEN(__token)       __token
    const char *        svalue;                 /* runtime value */
} TermKey_t;

typedef struct {
    TermVal_t           term;
    const char **       stoken;
#define TC_STRING(__token)      __token
    const char *        svalue;                 /* runtime value */
} TermString_t;

typedef struct {
    TermVal_t           term;
    int *               itoken;
#define TC_FLAG(__token)        __token
    int                 ivalue;                 /* runtime value*/
} TermNumeric_t;

extern TermKey_t *	    ttcfgkeys(unsigned *count);

__CEND_DECLS

#endif /*GR_TTYCFG_H_INCLUDED*/

