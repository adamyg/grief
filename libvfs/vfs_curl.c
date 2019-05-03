#include <edidentifier.h>
__CIDENT_RCSID(gr_vfs_curl_c,"$Id: vfs_curl.c,v 1.13 2019/03/15 23:23:01 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: vfs_curl.c,v 1.13 2019/03/15 23:23:01 cvsuser Exp $
 * Virtual file system interface - libcurl driver.
 *
 *
 * Copyright (c) 1998 - 2019, Adam Young.
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

#include <editor.h>
#include <errno.h>
#include <sys/stat.h>

#include "vfs_internal.h"
#include "vfs_class.h"
#include "vfs_mount.h"
#include "vfs_node.h"
#include "vfs_handle.h"
#include "vfs_tree.h"

#if defined(HAVE_LIBCURL) && \
        (defined(HAVE_CURL_CURL_H) || defined(HAVE_CURL_H))

#if defined(HAVE_CURL_CURL_H)
#include <curl/curl.h>                  /* standard */
#else
#include <curl.h>                       /* non-standard */
#endif

typedef struct {
    enum {PREFIX_NONE, PREFIX_FTP, PREFIX_TFTP, PREFIX_SFTP, PREFIX_HTTP, PREFIX_HTTPS, PREFIX_RSH, PREFIX_SSH}
                        prefix;
    unsigned            enabled;        /* *TRUE* if the protocol is enabled */
    const char *        user;
    const char *        passwd;
    const char *        host;
    const char *        path;
    unsigned            port;
} ftpurl_t;

static int              vfsftp_mount(struct vfs_mount *vmount, const char *arguments);
static int              vfsftp_unmount(struct vfs_mount *vmount);
static int              vfsftp_url_split(const char *path, ftpurl_t *url);
static void             vfsftp_url_destroy(ftpurl_t *url);
static char *           url_dup(const char *str, unsigned length);

static unsigned         x_curl_protocols = 0xffff;


struct vfs_class *
vfscurl_init(void)
{
    struct vfs_class *vclass;

    vclass = vfs_class_new("libcurl - ftp", "ftp", NULL);
    vfs_tree_vops(vclass);
    vclass->v_impl.i_mount = vfsftp_mount;
    vclass->v_impl.i_unmount = vfsftp_unmount;
    return vclass;
}


static int
vfsftp_mount(struct vfs_mount *vmount, const char *arguments)
{
    ftpurl_t url;
    struct vfs_tree *tree = NULL;
    int error = 0;

    VFS_TRACE(("vfsftp_mount(%s, %s)\n", vmount->mt_mount, (arguments?arguments:"")))
    if (NULL == arguments ||
            0 != vfsftp_url_split(arguments, &url)) {
        errno = EINVAL;                         /* invalid arguments */

    } else if (!error) {
        if (NULL == (tree = vfs_tree_new(vmount, NULL))) {
            error = errno;
        } else {
//          vfs_tree_vop(tree, VTREE_VOP_LOCALGET, (vfs_treevfunc_t)vfsarc_tree_vlocalget);
//          vfs_tree_vop(tree, VTREE_VOP_LOCALPUT, (vfs_treevfunc_t)vfsarc_tree_vlocalput);
        }
    }

    if (!error)  {
        /* XXX - incomplete support/initialisation */
        error = -1;
    }

    if (error) {
        if (tree) {
            vfs_tree_destroy(tree);
        }
        vfsftp_url_destroy(&url);
        errno = error;
    }

    VFS_TRACE(("\tvfsftp_mount(%s, %s) : %d\n", vmount->mt_mount, (arguments?arguments:""), error))
    return (error ? -1 : 0);
}


static int
vfsftp_unmount(struct vfs_mount *vmount)
{
    return -1;
}


/*  Function:           vfsftp_url_split
 *      Extract the components from the specified 'url'
 *
 *  Parameters:
 *      path -              Buffer containing the URL specification.
 *      url -               Target URL object poluated with the individual components.
 *
 *  Syntax:
 *>     [ftp://][user[[:,]["]passwd["]]@]hostname:port/[remote-dir]
 *
 *  *Examples*
 *>     myself@localhost:9090/home/myself
 *>     myself:password@localhost:9090/home/myself
 *>     myself,"password"@localhost:9090/home/myself
 *
 *  Returns:
 *      Zero on success and populates the given url structure,
 *      otherwise -1 on error.
 */
static int
vfsftp_url_split(const char *path, ftpurl_t *url)
{
    static const struct protocol {
        unsigned        ident;
        const char     *name;
        unsigned        length;
    } protocols[] = {
#define PROTOCOL_ELEMENT(i, n)  {i, n, sizeof(n)-1 }
            PROTOCOL_ELEMENT(PREFIX_FTP,   "ftp"),
            PROTOCOL_ELEMENT(PREFIX_TFTP,  "tftp"),
            PROTOCOL_ELEMENT(PREFIX_SFTP,  "sftp"),
            PROTOCOL_ELEMENT(PREFIX_HTTP,  "http"),
            PROTOCOL_ELEMENT(PREFIX_HTTPS, "https"),
            PROTOCOL_ELEMENT(PREFIX_SSH,   "ssh"),
            PROTOCOL_ELEMENT(PREFIX_RSH,   "rsh"),
#undef PROTOCOL_ELEMENT
            };

    enum {URL_USER, URL_PASSWD, URL_HOST, URL_PORT, URL_PATH, URL_MODES}
        mode = URL_USER;
    unsigned i, length[URL_MODES] = {0};
    const unsigned char *cursor = (unsigned char *)path, *port = NULL;
    int quote = -1, ch;

    assert(path);
    assert(url);

    memset(url, 0, sizeof(*url));

    /*  leading optional prefix/protocol, of one the following -
     *
     *      [/][s]ftp[;#][//]
     *      [/]http[;#][//]
     */
    url->prefix = PREFIX_NONE;
    while ('/' == *cursor) {
        ++cursor;
    }
    for (i = 0; i < (sizeof(protocols)/sizeof(protocols[0])); ++i) {
        const struct protocol *p = protocols + i;

        if (0 == strncmp(cursor, p->name, p->length) &&
                (':' == cursor[p->length] || '"' == cursor[p->length])) {
            /*
             *  matched
             */
            url->prefix = p->ident;
            url->enabled = (x_curl_protocols & (1 << p->ident)) ? 1 : 0;
            cursor += p->length + 1;

            if ('/' != *path) {                 /* leading or trailing only */
                if ('/' == *cursor && '/' == cursor[1]) {
                    cursor += 2;
                }
            }
            break;
        }
    }

    /*  pull components
     *
     *      [user[,['"]password['"]][@hostname][;port][/path]
     */
#define ISQUOTE(c)  ('\'' == ch || '"' == ch)   /* denotes quoted text */

    port = NULL;
    url->user = cursor;
    while (0 != (ch = *cursor++)) {
        /* invalid characters */
        if (' ' == ch || '\t' == ch || '\r' == ch || '\n' == ch) {
            break;
        }

        /* quoted text */
        if (quote > 0) {
            if (ch != quote) {
                ++length[mode];
            } else {
                quote = 0;                      /* .. complete */
            }
            continue;
        }

        if (ISQUOTE(ch) && 0 == length[mode]) {
            if (URL_USER == mode) {
                url->user = cursor;
                quote = ch;
                continue;
            } else if (URL_PASSWD == mode) {
                url->passwd = cursor;
                quote = ch;
                continue;
            }
        }

        /* next component or concat to existing */
        if ((':' == ch || ',' == ch) && mode == URL_USER) {
            url->passwd = cursor;
            mode = URL_PASSWD;
            quote = -1;

        } else if ('@' == ch && mode < URL_HOST) {
            url->host = cursor;
            mode = URL_HOST;
            quote = -1;

        } else if (':' == ch && mode == URL_HOST) {
            port = cursor;
            mode = URL_PORT;
            quote = -1;

        } else if ('/' == ch && mode < URL_PATH) {
            url->path = cursor;
            mode = URL_PATH;
            quote = -1;

        } else {
            if (0 == quote) {
                break;                          /* next component missing */

            } else if (URL_PORT == mode && (ch < '0' || ch > '9')) {
                break;                          /* not digit */
            }
            ++length[mode];                     /* component length */
        }
    }

    /*
     *  assign results
     *      no default as assumed here, all are implemented on a protocol specific case.
     */
    url->user   = url_dup(url->user,   length[URL_USER]);
    url->passwd = url_dup(url->passwd, length[URL_PASSWD]);
    url->host   = url_dup(url->host,   length[URL_HOST]);
    url->port   = (port && length[URL_PORT] ? atoi(port) : 0);
    if (url->port >= 65536) {
        url->port = 0;                          /* out-of-range */
    }
    url->path   = url_dup(url->path, length[URL_PATH]);

    if (ch) {
        vfsftp_url_destroy(url);
        VFS_TRACE(("\tvfsftp_urlsplit(%s => %s) : -1\n", path, cursor-1))
        return -1;
    }

    VFS_TRACE(("\tvfsftp_urlsplit(%s) : 0\n", path))
    VFS_TRACE(("\t\tuser:%s, passwd:%s, host:%s, port:%d, path:%s\n", (url->user?url->user:""), \
        (url->passwd?url->passwd:""), (url->host?url->host:""), url->port,  (url->path?url->path:"")))
    return 0;
}


static char *
url_dup(const char *str, unsigned length)
{
    if (str && length) {
        char *t_str;

        if (NULL != (t_str = chk_alloc(length+1))) {
            memcpy(t_str, str, length);
            t_str[length]=0;
            return t_str;
        }
    }
    return NULL;
}


/*  Function:           vfsftp_url_destroy
 *      Destroy a previously parsed url object.
 *
 *  Parameters:
 *      url -               URL object reference.
 *
 *  Returns:
 *      nothing
 */
static void
vfsftp_url_destroy(ftpurl_t *url)
{
    assert(url);
    chk_free((char *)url->user);
    chk_free((char *)url->passwd);
    chk_free((char *)url->host);
    chk_free((char *)url->path);
    memset(url, 0, sizeof(*url));
}

#else   /* HAVE_LIBCURL*/

struct vfs_class *
vfscurl_init(void)
{
    return NULL;
}

#endif  /* !HAVE_LIBCURL*/
