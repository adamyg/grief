#ifndef GR_ARG_H_INCLUDED
#define GR_ARG_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_arg_h,"$Id: arg.h,v 1.13 2024/05/17 16:42:32 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: arg.h,v 1.13 2024/05/17 16:42:32 cvsuser Exp $
 * Command line argument processing functionality.
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

#define arg_none            0
#define arg_required        1
#define arg_optional        2

struct argoption {
    const char *name;               /* long argument name */
    int has_arg;                    /* 0=no arg, 1=arg required, 2=optional arg */
    int *flag;                      /* var with 'val' value upon detection */
    int val;                        /* value returned on detection */
    const char *d1,*d2;             /* description (optional) */
    int udata;                      /* user specific data (read-only) */
};

struct argparms {
    int err;                        /* [in]     if error message should be printed (user override) */
    int ind;                        /* [out]    index into parent argv vector */
    int opt;                        /* [out]    character checked for validity */
    int lidx;                       /* [out]    long index */
    const char *val;                /* [out]    argument associated with option */
    int ret;                        /* [out]    return code */
    char *udata;                    /* [in/out] user specific data */

    const char *_prog;
    void (* _errout)(struct argparms *p, const char *msg);
    const char *_place;
    int _nargc;
    const char *const *_nargv;
    const char *_ostr;
    const struct argoption *_lopt;
    int _longonly;
};

typedef const char * (* arg_helper_t)(const struct argoption *, void *);

extern void                 arg_init(struct argparms *p, int nargc, const char *const *nargv, const char *ostr);
extern void                 arg_initl(struct argparms *p, int nargc, const char *const *nargv, 
                                    const char *ostr, const struct argoption *lopt, int longonly);
#define                     arg_errout(a, errout) \
                                (a)->_errout = errout
extern int                  arg_getopt(struct argparms *p);
#define                     arg_next(a) \
                                (a)->ind++, (a)->_place = NULL
extern void                 arg_close(struct argparms *p);
extern int                  arg_print(int ident, const struct argoption *options, arg_helper_t helper, void *udata);
extern int                  arg_split(char *cmd, const char **argv, int cnt);
extern int                  arg_subopt(char **optionp, const char * const *tokens, char **valuep);

/* 
 *  argz compatible functionality 
 */
extern int                  argx_create(const char **argv, char **argx, int *args);
extern int                  argx_create_sep(const char **argv, int sep, char **argx, int *args);
extern int                  argx_destroy(char **argx, int *args);
extern int                  argx_add(char **argx, int *args, const char *str);
extern int                  argx_add_sep(char **argx, int *args, const char *str, int sep);
extern int                  argx_append(char **argx, int *args, const char *buf, int buflen);
extern int                  argx_insert(char **argx, int *args, const char *cursor, const char *str);
extern int                  argx_replace(char **argx, int *args, const char *cursor, const char *str);
extern int                  argx_remove(char **argx, int *args, const char *cursor);
#define                     argx_first(__argx, __args) argx_next(__argx, __args, NULL)
extern const char *         argx_next(const char *argx, int args, const char *cursor);
extern int                  argx_count(const char *argx, int args);
extern int                  argx_dump(const char *argx, int args);
extern int                  argx_extract(const char *argx, const int args, char **argv, int argc);
extern char *               argx_stringify(char *argx, int args, int sep);

__CEND_DECLS

#endif /*GR_ARG_H_INCLUDED*/
