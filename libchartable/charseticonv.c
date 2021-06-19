#include <edidentifier.h>
__CIDENT_RCSID(gr_charseticonv_c,"$Id: charseticonv.c,v 1.20 2021/06/16 13:56:04 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* Conversion tables loader/interface.
 *
 *
 * Copyright (c) 2010 - 2021, Adam Young.
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

#include <config.h>
#include <edtypes.h>
#include <edpaths.h>
#include <edtrace.h>

#include <sys/stat.h>
#if defined(_AIX)
#include <sys/ldr.h>                            /* L_ERROR_... */
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <tailqueue.h>
#include <unistd.h>

#include <libstr.h>                             /* str_...()/sxprintf() */

#if defined(ENOSYS)
#define ENOTSUPPORTED       ENOSYS
#elif defined(ENOTSUP)
#define ENOTSUPPORTED       ENOTSUP
#elif defined(EOPNOTSUP)
#define ENOTSUPPORTED       EOPNOTSUP
#else
#error  Unknown value for ENOTSUPPORTED ...
#endif

#define CHARTABLE_DYNAMIC
#define CHARTABLES_7BIT
#define CHARTABLES_8BIT
#undef  CHARTABLES_16BIT

#include "chartable_module.h"
#include "charsettables.h"
#include "libchartable.h"

#if !defined(HAVE_DLOPEN)
#if defined(linux) || defined(unix) || defined(_AIX) || defined(__APPLE__) ||\
        defined(HAVE_DLFCN_H) || defined(_WIN32)
#define HAVE_DLOPEN
#endif  /*HAVE_DLOPEN*/
#endif  /*!HAVE_DLOPEN*/

#if defined(_WIN32)
#if !defined(_WIN32_WINNT) || (_WIN32_WINNT < 0x0500)
#undef  _WIN32_WINNT
#define _WIN32_WINNT        0x0500
#endif
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#if defined(__WATCOMC__)
#include <shellapi.h>                           /* SHSTDAPI */
#endif
#include <shlobj.h>
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "shfolder.lib")

static const char *         __charset_PATH_CXMOD(void);
static const char *         getpath(const char *application, const char *dir, char *buffer, const int buflen);
static int                  getexedir(char *buf, int maxlen);
static void                 dospath(char *path);

#undef  _PATH_GRIEF_CXMOD
#define _PATH_GRIEF_CXMOD   __charset_PATH_CXMOD()
#endif

#if defined(HAVE_DLFCN_H)
#include <dlfcn.h>                              /* dlopen() */
#elif defined(HAVE_LTDL_H)
#include <ltdl.h>                               /* lt_dlopen() */
#endif

typedef TAILQ_HEAD(ModuleList, dlmodule)
                            ModuleList_t;

typedef struct dlmodule {
    MAGIC_t                 dl_magic;
#define DLMODULE_MAGIC              0x1234
    TAILQ_ENTRY(dlmodule)   dl_node;
    void *                  dl_handle;
    void *                  dl_data;
    const char **           dl_names;
    unsigned                dl_refs;
    const char *            dl_module;
    const char *            dl_path;
} dlmodule_t;

static charset_iconv_t *    iconv_alloc(struct dlmodule *dlmod, int flags);

static const void *         ccs1_decode(struct charset_iconv *ic, const void *src, const void *cpend, int32_t *cooked, int32_t *raw);
static int                  ccs1_encode(struct charset_iconv *ic, const int32_t ch, void *buffer);
static size_t               ccs1_import(struct charset_iconv *ic, const char **inbuf, size_t *inbytes, char **outbuf, size_t *outbytes);
static size_t               ccs1_export(struct charset_iconv *ic, const char **inbuf, size_t *inbytes, char **outbuf, size_t *outbytes);

static int                  dlmod_path(char *buffer, int buflen, const char *dir, const char *name);
static int                  dlmod_open(const char *name, int flags, const char *path, struct dlmodule **result);
static int                  dlmod_push(const struct chartable_module *cm, void *handle,
                                    const char *path, const char *name, struct dlmodule **result);
static const char *         dlmod_error(int xerrno);
static struct dlmodule *    dlmod_byname(const char *name, int namelen);
static void                 dlmod_close(void *handle, struct dlmodule *mod);

static ModuleList_t         x_modlist;

static const char *         x_modpaths[] = {
    "$(HOME)/.grief/ctbl",
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    };


/*  Function:           charset_iconv_init
 *      Runtime initialisation.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing.
 */
void
charset_iconv_init(void)
{
    const struct charsettables *ct;
    unsigned i;

    TAILQ_INIT(&x_modlist);
#if defined(HAVE_DLOPEN)
#elif defined(HAVE_LTDL) && defined(HAVE_LTDL_H)
    lt_dlinit();
#endif
                                                /* static bindings */
    for (i = 0, ct = charsettables; i < (sizeof(charsettables)/sizeof(charsettables[0])); ++i, ++ct) {
        if (ct->module) {
            dlmod_push(ct->module, NULL, "", "local", NULL);
        }
    }
}


/*  Function:           charset_iconv_home
 *      Configure current home directory, using to expand ~/ references within paths.
 *
 *  Parameters:
 *      path -              Path specification.
 *
 *  Returns:
 *      nothing.
 */
void
charset_iconv_home(const char *path)
{
}


/*  Function:           charset_iconv_path
 *      Push a path into the conversion library path.
 *
 *  Parameters:
 *      path -              Path specification. If NULL the current
 *                          configuration shall be cleared.
 *
 *  Returns:
 *      nothing.
 */
void
charset_iconv_path(const char *path)
{
}


/*  Function:           charset_iconv_open
 *      Open a conversion descriptor for the specified external encoding.
 *
 *  Parameters:
 *      name -              External encoding.
 *
 *  Returns:
 *      Conversion descriptor, otherwise NULL.
 */
struct charset_iconv *
charset_iconv_open(const char *name, int flags)
{
    int namelen = strlen(name);
    const char *mod;

    if (NULL == name || 0 == name[0]) {
        return NULL;
    }
    namelen = strlen(name);

    if (NULL != (mod = strchr(name, '/'))) {    /* giconv style modifiers */
        if (0 == str_icmp(mod, "//TRANSLIT")) {
            flags |= CS_ICONV_TRANSLIT;

        } else if (0 == str_icmp(mod, "//IGNORE")) {
            flags |= CS_ICONV_IGNORE;
        }
        namelen = mod - name;                   /* remove modifier */
    }

    if (namelen) {
        struct dlmodule *dlmod = NULL;
        const struct charsettables *ct;
        char canon_buffer[64];
        const char *charset;
        int charsetlen = 0;
        unsigned i;

        if (NULL != (charset = charset_canonicalize(name, namelen, canon_buffer, sizeof(canon_buffer))) ||
                    NULL != (charset = charset_alias_lookup(name, namelen))) {
            charsetlen = strlen(charset);
            if (charsetlen == namelen && 0 == charset_compare(charset, name, namelen)) {
                charset = NULL;
            }
        }

        if (NULL != (dlmod = dlmod_byname(name, namelen)) ||
                (charset && NULL != (dlmod = dlmod_byname(charset, charsetlen)))) {
            return iconv_alloc(dlmod, flags);   /* pre-existing */
        }

        for (i = 0, ct = charsettables; i < (sizeof(charsettables)/sizeof(charsettables[0])); ++i, ++ct) {

            if (NULL == ct->module && ct->container) {
                                                /* dynamic binding */
                if (0 == charset_compare(ct->name, name, namelen) ||
                        (charset && 0 == charset_compare(ct->name, charset, charsetlen))) {

                    const char *cxmod = _PATH_GRIEF_CXMOD;
                    char path[1024];
                    unsigned i;

                    for (i = 0; i < (sizeof(x_modpaths)/sizeof(x_modpaths[0])); ++i) {
                        const char *modpath;

                        if (NULL == (modpath = x_modpaths[i])) {
                            modpath = cxmod;
                            cxmod = NULL;
                        }

                        if (modpath) {
                            if (0 == dlmod_path(path, sizeof(path), modpath, ct->container) &&
                                        0 == dlmod_open(ct->container, ct->flag, path, &dlmod)) {
                                charsettables[i].module = NULL;
                                if (dlmod) {
                                    return iconv_alloc(dlmod, flags);
                                }

                                if (NULL != (dlmod = dlmod_byname(name, namelen)) ||
                                        (charset && NULL != (dlmod = dlmod_byname(charset, charsetlen)))) {
                                    return iconv_alloc(dlmod, flags);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return NULL;
}


/*  Function:           charset_iconv_close
 *      Close the previously open conversion descriptor for the specified external encoding.
 *
 *  Parameters:
 *      ic -                Conversion descriptor.
 *
 *  Returns:
 *      nothing.
 */
void
charset_iconv_close(struct charset_iconv *ic)
{
    if (ic) {
        struct dlmodule *dlmod;

        assert(0x5421 == ic->ic_magic);
        if (NULL != (dlmod = ic->ic_module)) {
            --dlmod->dl_refs;
        }
        free(ic);
    }
}


const void *
charset_iconv_decode(struct charset_iconv *ic, const void *src, const void *cpend, int32_t *cooked, int32_t *raw)
{
    if (ic && src < cpend) {
        return (*ic->ic_decode)(ic, src, cpend, cooked, raw);
    }
    return NULL;
}


int
charset_iconv_encode(struct charset_iconv *ic, const int32_t ch, void *buffer)
{
    if (ic && buffer) {
        return (*ic->ic_encode)(ic, ch, buffer);
    }
    return 0;
}


int
charset_iconv_length(struct charset_iconv *ic, const int32_t ch)
{
    if (ic) {
        char buffer[32];
        return (*ic->ic_encode)(ic, ch, buffer);
    }
    return 0;
}


size_t
charset_iconv_import(struct charset_iconv *ic,
        const char **inbuf, size_t *inbytes, char **outbuf, size_t *outbytes)
{
    if (ic) {
        return (*ic->ic_import)(ic, inbuf, inbytes, outbuf, outbytes);
    }
    errno = EBADF;                              /* bad arguments */
    return (size_t)-1;
}


size_t
charset_iconv_export(struct charset_iconv *ic,
        const char **inbuf, size_t *inbytes, char **outbuf, size_t *outbytes)
{
    if (ic) {
        return (*ic->ic_export)(ic, inbuf, inbytes, outbuf, outbytes);
    }
    errno = EBADF;                              /* bad arguments */
    return (size_t)-1;
}


static charset_iconv_t *
iconv_alloc(struct dlmodule *dlmod, int flags)
{
    charset_iconv_t *ic = NULL;

    if (NULL != (ic = malloc(sizeof(charset_iconv_t)))) {
        if (dlmod->dl_data)  {
            const struct chartable_module *cm = dlmod->dl_data;
            const uint32_t signature = cm->cm_signature;

            memset(ic, 0, sizeof(charset_iconv_t));
            if (CHARTABLE_SIGNATURE(CHARTABLE_CCS, 0x100) == signature) {
                ic->ic_desc = dlmod->dl_names[0];
                ic->ic_data = cm->cm_desc;
                ic->ic_decode = ccs1_decode;
                ic->ic_encode = ccs1_encode;
                ic->ic_import = ccs1_import;
                ic->ic_export = ccs1_export;
            }
            ic->ic_magic = 0x5421;
            ic->ic_flags = flags;
            ic->ic_module = dlmod;
        }
        ++dlmod->dl_refs;
    }
    return ic;
}


static const void *
ccs1_decode(struct charset_iconv *ic, const void *src, const void *cpend, int32_t *cooked, int32_t *raw)
{
    struct chartable_ccs1 *ccs1 = (struct chartable_ccs1 *) ic->ic_data;
    const unsigned char *cursor = (const unsigned char *)src;
    *cooked = *raw = (*ccs1->unicode)(ccs1, *cursor);
    return ++cursor;
}


static int
ccs1_encode(struct charset_iconv *ic, const int32_t ch, void *buffer)
{
    struct chartable_ccs1 *ccs1 = (struct chartable_ccs1 *) ic->ic_data;
    unsigned char *cursor = (unsigned char *)buffer;
    *cursor = (*ccs1->native)(ccs1, ch);
    return 1;
}


static size_t
ccs1_import(struct charset_iconv *ic, const char **inbuf, size_t *inbytes, char **outbuf, size_t *outbytes)
{
    struct chartable_ccs1 *ccs1 = (struct chartable_ccs1 *) ic->ic_desc;
    unsigned char *ocursor;
    size_t ocount, res = 0;

    if (NULL == outbuf || NULL == (ocursor = ((unsigned char *) *outbuf)) ||
            NULL == outbytes || (ocount = *outbytes) == 0) {
        errno = EBADF;                          /* bad arguments */
        res = (size_t)-1;

    } else {
        const unsigned char *icursor;
        size_t icount;

        if (NULL != inbuf || NULL != (icursor = ((const unsigned char *) *inbuf))) {
            if (NULL != inbytes || (icount = *inbytes) > 0) {
                while (icount > 0) {
                    const int32_t ch = (*ccs1->native)(ccs1, *icursor++);
                    int len = charset_utf8_length(ch);

                    if ((size_t)len > ocount) {
                        errno = E2BIG;          /* insuffient space within out-buffer */
                        res = (size_t)-1;
                        break;
                    }
                    (void) charset_utf8_encode(ch, ocursor);
                    ocursor += len;
                    ocount -= len;
                }
                *inbuf    = (const char *)icursor;
                *inbytes  = icount;
                *outbuf   = (char *)ocursor;
                *outbytes = ocount;
            }
        }
    }
    return res;
}


static size_t
ccs1_export(struct charset_iconv *ic, const char **inbuf, size_t *inbytes, char **outbuf, size_t *outbytes)
{
    struct chartable_ccs1 *ccs1 = (struct chartable_ccs1 *) ic->ic_desc;
    unsigned char *ocursor;
    size_t ocount, res = 0;

    if (NULL == outbuf || NULL == (ocursor = ((unsigned char *) *outbuf)) ||
            NULL == outbytes || (ocount = *outbytes) == 0) {
        errno = EBADF;                          /* bad arguments */
        res = (size_t)-1;

    } else {
        const char *icursor;
        size_t icount;

        if (NULL != inbuf || NULL != (icursor = *inbuf)) {
            if (NULL != inbytes || (icount = *inbytes) > 0) {
                while (icount > 0) {
                    if (ocount == 0) {          /* insuffient space within out-buffer */
                        errno = E2BIG;
                        res = (size_t)-1;

                    } else {
                        const char *end;
                        int32_t raw, ch;

                        if (NULL == (end = charset_utf8_decode(icursor, icursor + icount, &ch, &raw))) {
                            errno = EINVAL;     /* an complete sequence */
                            res = (size_t)-1;
                            break;
                        }
                        if (ch != raw) {
                            errno = EILSEQ;     /* bad sequence */
                            res = (size_t)-1;
                            break;
                        }
                         icount -= (end - icursor);
                        *ocursor++ = (*ccs1->native)(ccs1, ch);
                        --ocount;
                    }
                }
                *inbuf    = icursor;
                *inbytes  = icount;
                *outbuf   = (char *)ocursor;
                *outbytes = ocount;
            }
        }
    }
    return res;
}


static int
dlmod_path(char *path, int pathlen, const char *dir, const char *name)
{
    struct stat sb;
    int rc;

#if defined(_WIN32) || defined(WIN32)
    _snprintf(path, pathlen, "%s/cx%s.dll", dir, name);
    path[pathlen - 1] = 0;
    dospath(path);
#else
    snprintf(path, pathlen, "%s/cx%s.so", dir, name);
    path[pathlen - 1] = 0;
#endif
    if (0 == (rc = stat(path, &sb))) {
        if (! S_ISREG(sb.st_mode)) {
            rc = EINVAL;
        }
    }
    trace_log("dlmod_path(%s, %s) : %d\n", name, path, rc);
    return rc;
}


static int
dlmod_open(const char *module, int flag, const char *path, struct dlmodule ** result)
{
    const char *symbol =
            (CHARTABLE_PACKAGE & flag ? "chartable_module_package" : "chartable_module");
    int ret = ENOTSUPPORTED;
    void *data, *handle;

    if (result) *result = NULL;
    trace_log("dlmod_open(%s->%s, %s)\n", module, path, symbol);
    errno = 0;

#if defined(HAVE_DLOPEN)
#ifndef RTLD_LOCAL
#define RTLD_LOCAL 0
#endif
#ifndef RTLD_LAZY
#define RTLD_LAZY  0
#endif
    if (NULL != (handle = dlopen(path, RTLD_LOCAL|RTLD_LAZY))) {
        if (NULL != (data = dlsym(handle, symbol))) {
            ret = 0;
        } else {
            ret = (errno ? errno : ENOENT);
        }
    } else {
        ret = (errno ? errno : ENOENT);
    }

#elif defined(HAVE_LIBLTDL) && defined(HAVE_LTDL_H)
    if (NULL != (handle = lt_dlopen(path))) {
        if (NULL != (data = lt_dlsym(handle, symbol))) {
            ret = 0;
        } else {
            ret = (errno ? errno : ENOENT);
        }
    } else {
        ret = (errno ? errno : ENOTSUPPORTED);
    }

#elif defined(_WIN32) || defined(WIN32)
    ret = ENOTSUPPORTED;                        /* dlfcn emulation */

#else
#error unknown environment ...
#endif

    if (0 == ret) {
        struct dlmodule *dlmod;

        if (CHARTABLE_PACKAGE & flag) {
            const struct chartable_package *pkg =
                    (const struct chartable_package *) data;

            if (CHARTABLE_PACKAGE_MAGIC == pkg->cp_magic) {
                const struct chartable_module * const *modules = pkg->cp_modules;
                uint32_t count = pkg->cp_count;
                int t_ret, cnt = 0, ret = 0;

                while (*modules && count-- > 0) {
                    if (0 == (t_ret = dlmod_push(*modules, handle, module, path, &dlmod))) {
                        ret = 0;
                        ++cnt;
                    } else if (0 == ret && 0 == cnt) {
                        ret = t_ret;
                    }
                    ++modules;
                }
            }
        } else {
            if (0 == (ret = dlmod_push((const struct chartable_module *)data, handle, module, path, &dlmod))) {
                if (result) *result = dlmod;
            }
        }
    }

    if (ret) {
        dlmod_close(handle, NULL);
        trace_log("==> %s (%d)\n", dlmod_error(ret), ret);
    }
    return ret;
}


static int
dlmod_push(const struct chartable_module *cm, void *handle,
        const char *module, const char *path, struct dlmodule **result)
{
    const int modulelen = strlen(module), pathlen = strlen(path);
    struct dlmodule *dlmod = NULL;
    int ret = 0;

    if (CHARTABLE_MODULE_MAGIC == cm->cm_magic) {
        if (NULL == (dlmod = malloc(sizeof(*dlmod) + modulelen + pathlen + 2))) {
            ret = ENOMEM;

        } else {
            const uint32_t signature = cm->cm_signature;

            memset(dlmod, 0, sizeof(*dlmod));
            TAILQ_INSERT_HEAD(&x_modlist, dlmod, dl_node);
            dlmod->dl_data   = (void *)cm;
            dlmod->dl_magic  = DLMODULE_MAGIC;
            dlmod->dl_handle = handle;
            dlmod->dl_module = (const char *)(dlmod + 1);
            dlmod->dl_path   = dlmod->dl_module + modulelen + 1;
            memcpy((char *)dlmod->dl_path, path, pathlen + 1);
            memcpy((char *)dlmod->dl_module, module, modulelen + 1);
            trace_log("==> loaded (signature:0x%08x)\n", (unsigned) signature);

            if (CHARTABLE_SIGNATURE(CHARTABLE_CCS, 0x100) == signature) {
                struct chartable_ccs1 *ccs1 = (struct chartable_ccs1 *)cm->cm_desc;

                dlmod->dl_names = ccs1->names;
                if (ccs1->init) {
                    (*ccs1->init)(ccs1);
                }
            }
            ret = 0;
        }
    } else {
        ret = EINVAL;
    }
    if (result) *result = dlmod;
    return ret;
}


static const char *
dlmod_error(int xerrno)
{
    const char *ret = NULL;

#if defined(HAVE_DLOPEN)
#if defined(_AIX)
    if (ENOEXEC == xerrno) {
        /*
         *  On AIX if reported as ENOEXEC, can query loader for further details.
         */
        static char buffer[1024];
        char *msgs[BUFSIZ/sizeof(char*)];
        int n;

        buffer[0] = 0;
        if ((n = loadquery(L_GETMESSAGE, msgs, sizeof(msgs))) > 0) {
            int m, len;

            len = strpush(0, buffer, "dlopen: ");
            for (m = 0; msgs[m] && len < buffer; ++m) {
                const char *msg = msgs[m], *trailing = msg;

                while (isdigit(*trailing))
                    ++trailing;
                switch (atoi(msg)) {
                case L_ERROR_TOOMANY: msg = "too many errors, remaining truncated"; trailing = NULL; break;
                case L_ERROR_NOLIB:   msg = "cannot load dependent library"; break;
                case L_ERROR_FORMAT:  msg = "bad exec format in"; break;
                case L_ERROR_ERRNO:
                    while (! isdigit(*trailing))
                        ++trailing;
                    msg = strerror(atoi(trailing));
                    trailing = NULL;
                    break;
                default:
                    trailing = NULL;
                    break;
                }
                len = strpush(len, buffer, msg);
                if (trailing) len = strpush(len, buffer, trailing);
            }
            return buffer;
        }
    }
#endif  /*_AIX*/
    ret = dlerror();

#elif defined(HAVE_LIBLTDL) && defined(HAVE_LTDL_H)
    ret = lt_dlerror();

#else
    ret = "not supported";

#endif

    return (ret ? ret : (xerrno ? strerror(xerrno) : "unknown error"));
}


static struct dlmodule *
dlmod_byname(const char *name, int namelen)
{
    ModuleList_t *modlist = &x_modlist;
    dlmodule_t *dlmod = NULL;

    TAILQ_FOREACH(dlmod, modlist, dl_node) {
        const char **names = dlmod->dl_names;

        if (names) {
            const char *charset;

            while (NULL != (charset = *names++)) {
                if (0 == charset_compare(charset, name, namelen)) {
                    trace_log("==> preloaded(%s, %s)\n", charset, dlmod->dl_path);
                    return dlmod;
                }
            }
        }
    }
    return NULL;
}


static void
dlmod_close(void *handle, struct dlmodule *dlmod)
{
    if (dlmod) {
        if (NULL == handle) {
            if (dlmod) handle = dlmod->dl_handle;
        }

#if defined(HAVE_DLOPEN)
        if (handle) {
            dlclose(handle);
        }

#elif defined(HAVE_LIBLTDL) && defined(HAVE_LTDL_H)
        if (handle) {
            lt_dlclose(handle);
        }

#endif

        if (dlmod) {
            TAILQ_REMOVE(&x_modlist, dlmod, dl_node);
            memset(dlmod, 0, sizeof(*dlmod));
            free((void *)dlmod);
        }
    }
}


#if defined(_WIN32) || defined(WIN32)
/*
 *  Retrieve cx library directory, equivalent to '/usr/local/lib/grief'
 *
 *      <EXEPATH>
 *          <exepath>\ctbl\
 *
 *      <INSTALLPATH>
 *          X:\Program Files\<Grief>\ctbl\
 *
 *              SHGetFolderPath(CSIDL_PROGRAM_FILES)
 *              or getenv(ProgramFiles)
 *
 *      <APPDATA>
 *          X:\Documents and Settings\All Users\Application Data\<Grief>\ctbl\
 *
 *              SHGetFolderPath(CSIDL_COMMON_APPDATA)
 *              or getenv(ALLUSERSPROFILE)
 */
static const char *
__charset_PATH_CXMOD(void)
{
    static char x_buffer[MAX_PATH];

    if (0 == x_buffer[0]) {
        getpath(_PATH_GRIEF_NAME, "ctbl", x_buffer, sizeof(x_buffer));
    }
    return x_buffer;
}


static const char *
getpath(const char *application, const char *dir, char *buffer, const int buflen)
{
    int len, done = FALSE;

    // <EXEPATH>, generally same as INSTALLDIR
    if ((len = getexedir(buffer, buflen)) > 0) {
        _snprintf(buffer + len, buflen - len, "/%s", dir);
        buffer[buflen - 1] = 0;
        if (0 == _access(buffer, 0)) {
            done = TRUE;
        }
    }

    // <INSTALLPATH>
    if (! done) {
        if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_PROGRAM_FILES, NULL, 0, buffer))) {
            len = strlen(buffer);
            _snprintf(buffer + len, buflen - len, "/%s/%s", application, dir);
            buffer[buflen - 1] = 0;
            if (0 == _access(buffer, 0)) {
                done = TRUE;
            }
        }
    }

    // <APPDATA>
    if (! done)  {
        if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_COMMON_APPDATA, NULL, 0, buffer))) {
            len = strlen(buffer);
            _snprintf(buffer + len, buflen - len, "/%s/%s", application, dir);
            buffer[buflen - 1] = 0;
            if (0 == _access(buffer, 0)) {
                done = TRUE;
            }
        }
    }

    // default - INSTALLPATH
    if (! done) {
        const char *env;

        if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_PROGRAM_FILES, NULL, 0, buffer))) {
            len = strlen(buffer);
            _snprintf(buffer + len, buflen - len, "/%s/%s", application, dir);

        } else if (NULL != (env = getenv("ProgramFiles"))) {
            _snprintf(buffer, buflen, "%s/%s/%s/", env, application, dir);

        } else {
            _snprintf(buffer, buflen, "c:/Program Files/%s/%s", application, dir);
        }
        buffer[buflen - 1] = 0;
        w32_mkdir(buffer, 0660);
    }

    dospath(buffer);
    return buffer;
}


static int
getexedir(char *buf, int maxlen)
{
    if (GetModuleFileName(NULL, buf, maxlen)) {
        const int len = strlen(buf);
        char *cp;

        for (cp = buf + len; (cp > buf) && (*cp != '\\'); cp--)
            /*cont*/;
        if ('\\' == *cp) {
            cp[1] = '\0';                       // remove program
            return (cp - buf) + 1;
        }
        return len;
    }
    return -1;
}


static void
dospath(char *path)
{
    const char *in = path;

    while (*in) {
        if ('/' == *in || '\\' == *in) {
            ++in;
            while ('/' == *in || '\\' == *in) {
                ++in;                           // compress
            }
            *path++ = '\\';                     // convert
        } else {
            *path++ = *in++;
        }
    }
    *path = 0;
}
#endif  /*WIN32*/
/*end*/
