#include <edidentifier.h>
__CIDENT_RCSID(gr_m_ftp_c,"$Id: m_ftp.c,v 1.22 2025/02/07 03:03:21 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: m_ftp.c,v 1.22 2025/02/07 03:03:21 cvsuser Exp $
 * FTP/HTTP connection primitives -- beta/undocumented.
 *
 *
 * Copyright (c) 1998 - 2025, Adam Young.
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

#define  ED_ASSERT
#define  ED_LEVEL           1
#include <editor.h>
#include <edhandles.h>
#include <stdarg.h>
#include <stable.h>
#include <libstr.h>

#include "m_ftp.h"                              /* public interface */

#include "accum.h"                              /* acc_...() */
#include "debug.h"                              /* trace_...() */
#include "eval.h"                               /* get_...() */
#include "lisp.h"                               /* atom_...() */
#include "symbol.h"                             /* argv_assign_...() */

#if defined(HAVE_LIBSSH2)
static const char *keyfile1 = "~/.ssh/id_rsa.pub";
static const char *keyfile2 = "~/.ssh/id_rsa";

static int                  sftp_init(void);
#endif /*HAVE_LIBSSH2*/

#include "../libbsdfetch/fetch.h"               /* libfetch */

typedef struct {
#define IFTP_MAGIC          MKMAGIC('I','f','T','p')
    MAGIC_t         magic;
    int             ident;
    int             proto;
    const char *    cwd;
    const char *    sitename;
    struct url *    url;
#if defined(HAVE_LIBSSH2)
    struct ssh {
        LIBSSH2_SESSION *session;
        LIBSSH2_SFTP *sftp_session;
        LIBSSH2_SFTP_HANDLE *sftp_handle;
    } ssh;
#endif
} IFTP;

static IDENTIFIER_t         x_ftpseq;           /* identifier sequence */
static stype_t *            x_ftps = NULL;      /* and container */

static IFTP *               ftp_lookup(const int objid);


static IFTP *
iftplookup(const int id)
{
    sentry_t *st;

    if (x_ftps && id > 0 &&                     /* object lookup */
            NULL != (st = stype_lookup(x_ftps, (unsigned) id))) {
        IFTP *iftp = (IFTP  *)st->se_ptr;

        assert(IFTP_MAGIC == iftp->magic);
        assert(iftp->ident == id);
        ED_TRACE(("iftplookup(%d) : %d\n", id, iftp->proto))
        return iftp;
    }
    ED_TRACE(("iftplookup(%d) : NULL\n", id))
    return NULL;
}


static int
schememap(const char *scheme)
{
    int ret = -1;

    if (NULL == scheme || 0 == str_icmp(scheme, SCHEME_FTP)) {
        ret = PROTOCOL_FTP;
    } else if (0 == str_icmp(scheme, SCHEME_HTTP)) {
        ret = PROTOCOL_HTTP;
    } else if (0 == str_icmp(scheme, SCHEME_HTTPS)) {
        ret = PROTOCOL_HTTPS;
    } else if (0 == str_icmp(scheme, SCHEME_FILE)) {
        ret = PROTOCOL_FILE;
#if defined(HAVE_LIBSSH2)
    } else if (0 == str_icmp(scheme, SCHEME_SFTP)) {
        ret = PROTOCOL_SFTP;
#endif
//  } else if (0 == str_icmp(scheme, SCHEME_FTPS)) {
//      ret = PROTOCOL_FTPS;
    }
    ED_TRACE(("\tschememap(%s)=%d\n", scheme?scheme:"(null)", ret))
    return ret;
}


static const char *
schemename(int proto)
{
    const char *name = SCHEME_FTP;

    switch (proto) {
    case PROTOCOL_HTTP:  name = SCHEME_HTTP; break;
    case PROTOCOL_HTTPS: name = SCHEME_HTTPS; break;
    case PROTOCOL_FILE:  name = SCHEME_FILE; break;
    case PROTOCOL_FTP:
#if defined(HAVE_LIBSSH2)
    case PROTOCOL_SFTP:
#endif
//  case PROTOCOL_FTPS:
    default:
        break;
    }
    ED_TRACE(("\tschemename(%d)=%s\n", proto, name))
    return name;
}


static struct url *
makeURL(IFTP *iftp, const char *filename)
{
    const struct url *url = iftp->url;
    char *cursor, doc[1024] = {0};

    if (filename && !*filename) {
        filename = NULL;
    }

    if (!filename || (*filename != '/' && *filename != '\\')) {
        if (iftp->cwd) {                        /* current working directory */
            strxcat(doc, iftp->cwd, sizeof(doc));
        }
        strxcat(doc, "/", sizeof(doc));         /* delimiter or root directory */
    }

    if (filename) {
        strxcat(doc, filename, sizeof(doc));
    }

    for (cursor = doc; *cursor; ++cursor)  {
        if (*cursor == '\\') *cursor = '/';
    }

    return fetchMakeURL(schemename(iftp->proto),
                url->host, url->port, doc, url->user, url->pwd);
}


static const char *
unquotedURL(const struct url_ent *ent)
{
    struct url url = {0};
    url.doc = (char *)ent->doc;
    return fetchUnquotePath(&url);
}


/*<<GRIEF-BETA>>
    Macro: ftp_create - Reserved

        int
        ftp_create(..)

    Macro Description:
        The 'ftp_create()' primitive is reserved for future use.

    Macro Parameters:
        protocol - Optional protocol name.

    Macro Return:
        Upon successful completion, ftp_create() shall return a handle to
        the connection stream. Otherwise, -1 shall be returned, and
        'errno' shall be set to indicate the error.

    Macro Portability:
        A Grief extension.

    Macro See Also:
        ftp_create, ftp_connect, ftp_close
 */
void
do_ftp_create(void)                 /* int ([string|int protocol]) */
{
    const char *name = get_xstr(1);
    int proto = (NULL == name ? get_xinteger(1, -1) : -1);
    IFTP *iftp;
    int ret = -1;

    if (NULL == x_ftps) {
        fetchConnectionCacheInit(16, 2);        /* connection 16 max, 2 per host */
        x_ftps = stype_alloc();
        x_ftpseq = GRBASE_FTP;
    }

    if (proto >= 0 || (proto = schememap(name)) >= 0) {
        if (NULL != (iftp = chk_calloc(sizeof(IFTP), 1))) {
            const int ftpseq = x_ftpseq++;

            if (stype_insert(x_ftps, ftpseq, iftp) != -1) {
                iftp->magic = IFTP_MAGIC;
                iftp->ident = ftpseq;
                iftp->proto = proto;
                ret = ftpseq;
            } else {
                chk_free(iftp);
            }
        }
    }
    acc_assign_int(ret);
}


/*<<GRIEF-BETA>>
    Macro: ftp_connect - Reserved

        int
        ftp_connect(int id, string host_url, [int port],
                [string user], [string pass], [string callback])

    Macro Description:
        The 'ftp_connect()' primitive is reserved for future use.

    Macro Parameters:
        id - Connection identifier.
        host_url - Hostname or URL, if all other arguments are omitted.
        port - Optional integer port, if omitted the default for the
                connection protocol is applied.
        user - Optional user name.
        pwd - Optional password.
        callback - Application event callback.

    Macro Return:

    Macro See Also:
        ftp_create, ftp_close
 */
void
do_ftp_connect(void)                /* int (int id, string host, [int port],
                                              [string user], [string pwd], [string callback]) */
{
    const int id = get_xinteger(1, -1);
    const char *host = get_str(2);
    const int port = get_xinteger(3, -1);
    const char *user = get_xstr(4);
    const char *pwd = get_xstr(5);
//  const char *cb = get_xstr(6);
    IFTP *iftp = iftplookup(id);
    int ret = -1;

    if (iftp && host) {
        struct url *url;

        ED_TRACE(("ftp_connect(%s,%s)\n", schemename(iftp->proto), host))
        if (-1 == port && !user && !pwd) {      /* URL */
            if (NULL != (url = fetchParseURL(host))) {
                iftp->proto = schememap(url->scheme);
            }

        } else {
            const char *doc = strchr(host, '/');

            if (doc) {                          /* embedded path */
                char *thost = chk_snalloc(host, doc - host);
                url = fetchMakeURL(schemename(iftp->proto),
                        thost, (port > 0 ? port : 0), doc, (user ? user : ""), (pwd ? pwd : ""));
                chk_free(thost);

            } else {
                url = fetchMakeURL(schemename(iftp->proto),
                        host, (port > 0 ? port : 0), "/", (user ? user : ""), (pwd ? pwd : ""));
            }
        }

        if (url) {
            struct url_stat sb = {0};

            ED_TRACE(("ftp_connect(%s,host:%s,port:%d,user:%s,pwd:%s,doc:%s)\n", \
                 url->scheme, url->host, url->port, url->user, url->pwd, url->doc))
            if (0 == fetchStat(url, &sb, "v")) {
                if (iftp->url) {                /* reconnection */
                    fetchFreeURL(url);
                }
                iftp->url = url;
                ret = 0;
            }
        }
    }
    acc_assign_int(ret);
}


/*<<GRIEF-BETA>>
    Macro: ftp_timeout - Reserved

        int
        ftp_timeout(int id, int timeout, int connection)

    Macro Description:
        The 'ftp_timeout()' primitive is reserved for future use.

    Macro Parameters:
        id - Connection identifier.

    Macro Return:
        n/a

    Macro Portability:
        A Grief extension.

    Macro See Also:
        ftp_create, ftp_connect, ftp_close
 */
void
do_ftp_timeout(void)                /* int (int id, int timeout, int connection) */
{
    const int id = get_xinteger(1, -1);
    IFTP *iftp = iftplookup(id);
    int ret = -1;

    if (iftp) {
        /*TODO*/
    }
    acc_assign_int(ret);
}


/*<<GRIEF-BETA>>
    Macro: ftp_close - Reserved

        int
        ftp_close(int id)

    Macro Description:
        The 'ftp_close()' primitive is reserved for future use.

    Macro Parameters:
        id - Connection identifier.

    Macro Return:
        n/a

    Macro Portability:
        A Grief extension.

    Macro See Also:
        ftp_create, ftp_connect, ftp_close
 */
void
do_ftp_close(void)                  /* int (int id, [int keepalive = FALSE]) */
{
    const int id = get_xinteger(1, -1);
    const int keepalive = get_xinteger(2, 0);
    IFTP *iftp = iftplookup(id);
    int ret = -1;

    if (iftp) {
        struct url *url;

        if (NULL != (url = iftp->url)) {
            if (1 != keepalive) {
                fetchConnectionCacheFlush(url);
            }
            fetchFreeURL(url);
        }
        stype_delete(x_ftps, (unsigned) id);
        chk_free((char *)iftp->cwd);
        chk_free((char *)iftp->sitename);
        chk_free(iftp);
        ret = 0;
    }
    acc_assign_int(ret);
}


/*<<GRIEF-BETA>>
    Macro: ftp_connection_list - Reserved

        list
        ftp_connection_list()

    Macro Description:
        The 'ftp_connection_list()' primitive is reserved for future use.

    Macro Parameters:
        id - Connection identifier.

    Macro Return:
        n/a

    Macro Portability:
        A Grief extension.

    Macro See Also:
        ftp_create, ftp_connect, ftp_close
 */
void
do_ftp_connection_list(void)        /* list () */
{
    /*TODO*/
    acc_assign_int(-1);
}


/*<<GRIEF-BETA>>
    Macro: ftp_error - Reserved

        int
        ftp_error(int id, int errcode, string errmsg, string remotemsg)

    Macro Description:
        The 'ftp_error()' primitive is reserved for future use.

    Macro Parameters:
        id - Connection identifier.

    Macro Return:
        n/a

    Macro Portability:
        A Grief extension.

    Macro See Also:
        ftp_create, ftp_connect, ftp_close
 */
void
do_ftp_error(void)                  /* int (int id, int errcode, string errmsg, string remotemsg) */
{
    const int id = get_xinteger(1, -1);
    IFTP *iftp = iftplookup(id);
    int ret = -1;

    if (iftp) {
        /*TODO*/
    }
    acc_assign_int(ret);
}


/*<<GRIEF-BETA>>
    Macro: ftp_register - Reserved

        int
        ftp_register(int id, int eventid, string callback)

    Macro Description:
        The 'ftp_register()' primitive is reserved for future use.

    Macro Parameters:
        id - Connection identifier.

    Macro Return:
        n/a

    Macro Portability:
        A Grief extension.

    Macro See Also:
        ftp_create, ftp_connect, ftp_close
 */
void
do_ftp_register(void)               /* int (int id, int eventid, string callback) */
{
    const int id = get_xinteger(1, -1);
    IFTP *iftp = iftplookup(id);
    int ret = -1;

    if (iftp) {
        /*TODO*/
    }
    acc_assign_int(ret);
}


/*<<GRIEF-BETA>>
    Macro: ftp_put_file - Reserved

        int
        ftp_put_file(id, string remote, string local, int flags)

    Macro Description:
        The 'ftp_put_file()' primitive is reserved for future use.

    Macro Parameters:
        id - Connection identifier.

    Macro Return:
        n/a

    Macro Portability:
        A Grief extension.

    Macro See Also:
        ftp_create, ftp_connect, ftp_close
 */
void
do_ftp_put_file(void)               /* int (id, string remote, string local, int flags, [int offset]) */
{
    const int id = get_xinteger(1, -1);
    IFTP *iftp = iftplookup(id);
    int ret = -1;

    if (iftp) {
    }
    acc_assign_int(ret);
}


/*<<GRIEF-BETA>>
    Macro: ftp_get_file - Reserved

        int
        ftp_get_file(id, string remote, string local,
            [int flags], [int offset], [int mode = 0644])

    Macro Description:
        The 'ftp_get_file()' primitive is reserved for future use.

    Macro Parameters:
        id - Connection identifier.
        remote -
        local -
        flags -
        offset -
        mode -

    Macro Return:
        n/a

    Macro Portability:
        A Grief extension.

    Macro See Also:
        ftp_create, ftp_connect, ftp_close
 */
void
do_ftp_get_file(void)               /* int (id, string remote, string local,
                                                [int flags], [int offset], [int mode]) */
{
    const int id = get_xinteger(1, -1);
    const char *remote = get_str(2);
    const char *local = get_str(3);
        // accint_t flags = get_xinteger(4, 0);
    accint_t offset = get_xinteger(5, 0);
        // accint_t mode  = get_xinteger(6, 0);
    IFTP *iftp = iftplookup(id);
    int ret = -1;

    if (iftp) {
        struct url *url =
            (iftp->url ? makeURL(iftp, remote) : fetchParseURL(remote));

        if (url) {
            struct url_stat sb = {0};
            fetchIO *io;

            url->offset = (off_t)offset;        /* source offset */

            if (NULL != (io = fetchXGet(url, &sb, "v"))) {
                char buf[BUFSIZ * 4];
                ssize_t len, cnt;
                int fd = -1;
                                                /* TODO - flags and mode */
                if ((fd = open(local, O_CREAT|O_TRUNC|O_WRONLY, 0600)) >= 0) {
                    while ((len = fetchIO_read(io, buf, sizeof(buf))) > 0) {
                        if (len != (cnt = (ssize_t) write(fd, buf, (unsigned)len))) {
                            break;
                        }
                    }
                    close(fd);
                }
                fetchIO_close(io);
                ret = 0;
            }
            fetchFreeURL(url);
        }
    }
    acc_assign_int(ret);
}


/*<<GRIEF-BETA>>
    Macro: ftp_directory - Reserved

        int
        ftp_directory(int id, [string pattern], list files, int flags)

    Macro Description:
        The 'ftp_directory()' primitive is reserved for future use.

    Macro Parameters:
        id - Connection identifier.

    Macro Return:
        n/a

    Macro Portability:
        A Grief extension.

    Macro See Also:
        ftp_create, ftp_connect, ftp_close
 */
void
do_ftp_directory(void)              /* int (int id, [string pattern], list &files) */
{
    const int id = get_xinteger(1, -1);
//  const char *pattern = get_xstr(2);
    IFTP *iftp = iftplookup(id);
    accint_t ret = -1;

    if (iftp) {
        struct url *url;

        ED_TRACE(("ftp_directory(cwd:%s)\n", (iftp->cwd ? iftp->cwd : "")))
        if (NULL != (url = iftp->url)) {
            struct url_list list = {0};

            fetchInitURLList(&list);
            if (0 == fetchList(&list, url, NULL, NULL)) {
                const size_t items = list.length;
                const char **filenames = chk_alloc(sizeof(char *) * items);
                struct url_ent *urls = list.urls;
                size_t it, atoms = 0;

                for (it = 0; it < items; ++it) {
                    const char *filename = unquotedURL(urls + it);

                    ED_TRACE((" %3u: [%s] <%s>\n", (unsigned)it, urls[it].doc, (filename && *filename ? filename : "n/a")))
                    if (filename && *filename) {
                        filenames[atoms++] = filename;
                    }
                }

                if (atoms > 0) {
                    const size_t llen =
                        (atoms * sizeof_atoms[F_RSTR]) + sizeof_atoms[F_HALT];
                    LIST *newlp;

                    if (NULL != (newlp = lst_alloc(llen, (int)atoms))) {
                        LIST *lp = newlp;

                        for (it = 0; it < atoms; ++it) {
                            const char *filename = filenames[it];

                            lp = atom_push_str(lp, filename);
                            free((void *)filename);
                        }
                        atom_push_halt(lp);
                    }
                    argv_donate_list(3, newlp, llen);
                    ret = (int)list.length;
                }
                chk_free((void *)filenames);
            }
            fetchFreeURLList(&list);
        }
    }
    ED_TRACE(("ftp_directory() = %d\n", (int)ret))
    acc_assign_int(ret);
}


/*<<GRIEF-BETA>>
    Macro: ftp_getcwd - Reserved

        int
        ftp_getcwd(int id, string &dir)

    Macro Description:
        The 'ftp_getcwd()' primitive is reserved for future use.

    Macro Parameters:
        id - Connection identifier.

    Macro Return:
        n/a

    Macro Portability:
        A Grief extension.

    Macro See Also:
        ftp_create, ftp_connect, ftp_close
 */
void
do_ftp_getcwd(void)                 /* int (int id, string &dir) */
{
    const int id = get_xinteger(1, -1);
    IFTP *iftp = iftplookup(id);
    accint_t ret = -1;

    if (iftp) {
        ret = (accint_t) argv_assign_str(2, (iftp->cwd ? iftp->cwd : "/"));
    }
    acc_assign_int(ret);
}


/*<<GRIEF-BETA>>
    Macro: ftp_chdir - Reserved

        int
        ftp_chdir(int id, string dir)

    Macro Description:
        The 'ftp_chdir()' primitive is reserved for future use.

    Macro Parameters:
        id - Connection identifier.

    Macro Return:
        n/a

    Macro Portability:
        A Grief extension.

    Macro See Also:
        ftp_create, ftp_connect, ftp_close
 */
void
do_ftp_chdir(void)                  /* int (int id, string dir) */
{
    const int id = get_xinteger(1, -1);
    IFTP *iftp = iftplookup(id);
    int ret = -1;

    if (iftp) {
        /*TODO*/
    }
    acc_assign_int(ret);
}


/*<<GRIEF-BETA>>
    Macro: ftp_mkdir - Reserved

        int
        ftp_mkdir(int id, string dir)

    Macro Description:
        The 'ftp_mkdir()' primitive is reserved for future use.

        The ftp_mkdir() function creates the directory 'dir' on the
        server associated with connection 'id'.

    Macro Parameters:
        id - Connection identifier.

    Macro Return:
        The ftp_mkdir() function returns 0 on success, or -1 on error.

    Macro Portability:
        A Grief extension.

    Macro See Also:
        ftp_create, ftp_connect, ftp_close
 */
void
do_ftp_mkdir(void)                  /* int (int id, string dir) */
{
    const int id = get_xinteger(1, -1);
    const char *dir = get_str(2);
    IFTP *iftp = iftplookup(id);
    int ret = -1;

    if (iftp) {
        ED_TRACE(("ftp_mkdir(cwd:%s,new:%s)\n", (iftp->cwd ? iftp->cwd : ""), dir))
        if (NULL != iftp->url) {
            switch (iftp->proto) {
            case PROTOCOL_FTP: {
                    struct url *url = makeURL(iftp, dir);
                    if (url) {
                        ret = fetchMkdirFTP(url, "v");
                        fetchFreeURL(url);
                    }
                }
                break;
#if defined(HAVE_LIBSSH2)
            case PROTOCOL_SFTP:
                if (iftp->sftp_session) {
                    ret = libssh2_sftp_mkdir(sftp_session, dir,
                                LIBSSH2_SFTP_S_IRWXU|LIBSSH2_SFTP_S_IRGRP|LIBSSH2_SFTP_S_IXGRP|LIBSSH2_SFTP_S_IROTH|LIBSSH2_SFTP_S_IXOTH);
                }
                break;     
#endif //HAVE_LIBSSH2
            default:
                break;
            }
        }
    }
    acc_assign_int(ret);
}


/*<<GRIEF-BETA>>
    Macro: ftp_stat - Reserved

        int
        ftp_stat(int id, string file,
            [int &size], [int &mtime], [int &atime])

    Macro Description:
        The 'ftp_stat()' primitive is reserved for future use.

    Macro Parameters:
        id - Connection identifier.

    Macro Return:
        n/a

    Macro Portability:
        A Grief extension.

    Macro See Also:
        ftp_create, ftp_connect, ftp_close
 */
void
do_ftp_stat(void)                   /* int (int id, string filename,
                                                int &size, int &mtime, int &atime, int mode &mode) */
{
    const int id = get_xinteger(1, -1);
    const char *filename = get_xstr(2);
    IFTP *iftp = iftplookup(id);
    int ret = -1;

    if (iftp && iftp->url) {
        struct url *url = makeURL(iftp, filename);
        struct url_stat sb = {0};

        if (url) {
            if (0 == fetchStat(url, &sb, NULL)) {
                argv_assign_int(3, (accint_t) sb.size);
                argv_assign_int(4, (accint_t) sb.mtime);
                argv_assign_int(5, (accint_t) sb.atime);
                argv_assign_int(6, (accint_t) sb.mode);
                ret = 0;
            }
            fetchFreeURL(url);
        }
    }
    acc_assign_int(ret);
}


/*<<GRIEF-BETA>>
    Macro: ftp_remove - Reserved

        int
        ftp_remove(int id, string name)

    Macro Description:
        The 'ftp_remove()' primitive is reserved for future use.

        The ftp_remove() function removes the file or directory
        specified by 'name' on the server associated with the
        connection 'id'.

    Macro Parameters:
        id - Connection identifier.
        name - String specifying the file or directory to be removed.

    Macro Return:
        The ftp_remove() function returns 0 on success, or -1 on error.

    Macro Portability:
        A Grief extension.

    Macro See Also:
        ftp_create, ftp_connect, ftp_close
 */
void
do_ftp_remove(void)                 /* int (int id, string name) */
{
    const int id = get_xinteger(1, -1);
    const char *name = get_xstr(2);
    IFTP *iftp = iftplookup(id);
    int ret = -1;

    if (iftp) {
        ED_TRACE(("ftp_remove(cwd:%s,name:%s)\n", (iftp->cwd ? iftp->cwd : ""), name))
        if (NULL != iftp->url) {
            switch (iftp->proto) {
            case PROTOCOL_FTP: {
                    struct url *url = makeURL(iftp, name);
                    if (url) {
                        ret = fetchRemoveFTP(url, "v");
                        fetchFreeURL(url);
                    }
                }
                break;
#if defined(HAVE_LIBSSH2)
            case PROTOCOL_SFTP:
#endif
            default:
                break;
            }
        }
    }
    acc_assign_int(ret);
}


/*<<GRIEF-BETA>>
    Macro: ftp_rename - Reserved

        int
        ftp_rename(int id, string oldname, string newname)

    Macro Description:
        The 'ftp_rename()' primitive is reserved for future use.

    Macro Parameters:
        id - Connection identifier.
        oldname -
        newname -

    Macro Return:
        n/a

    Macro Portability:
        A Grief extension.

    Macro See Also:
        ftp_create, ftp_connect, ftp_close
 */
void
do_ftp_rename(void)                 /* int (int id, string oldname, string newname) */
{
    const int id = get_xinteger(1, -1);
    const char *oldname = get_xstr(2);
    const char *newname = get_xstr(3);
    IFTP *iftp = iftplookup(id);
    int ret = -1;

    if (iftp) {
        ED_TRACE(("ftp_rename(cwd:%s,old:%s,new:%s)\n", (iftp->cwd ? iftp->cwd : ""), oldname, newname))
        if (NULL != iftp->url) {
            switch (iftp->proto) {
            case PROTOCOL_FTP: {
                    struct url *url = makeURL(iftp, oldname),
                        *url2 = makeURL(iftp, newname);

                    if (url && url2) {
                        ret = fetchRenameFTP(url, url2->doc, "v");
                        fetchFreeURL(url2);
                        fetchFreeURL(url);
                    }
                }
                break;
#if defined(HAVE_LIBSSH2)
            case PROTOCOL_SFTP:
#endif
            default:
                break;
            }
        }
    }
    acc_assign_int(ret);
}


/*<<GRIEF-BETA>>
    Macro: ftp_connection - Reserved

        int
        ftp_find_connection(string host_or_site, [int site_flag])

    Macro Description:
        The 'ftp_connection()' primitive is reserved for future use.

    Macro Parameters:
        n/a

    Macro Return:
        n/a

    Macro Portability:
        A Grief extension.

    Macro See Also:
        ftp_create, ftp_connect, ftp_close
 */
void
do_ftp_find_connection(void)        /* int (string host_or_site, [int site_flag]) */
{
    /*TODO*/
    acc_assign_int(-1);
}


/*<<GRIEF-BETA>>
    Macro: ftp_protocol - Reserved

        int
        ftp_protocol(int id, [string &name])

    Macro Description:
        The 'ftp_protocol()' primitive is reserved for future use.

        The 'ftp_set_options()' primitive retrieves the protocol type
        associated with the connection 'id'.

    Macro Parameters:
        id - Connection identifier.

    Macro Return:
        n/a

    Macro Portability:
        A Grief extension.

    Macro See Also:
        ftp_create, ftp_connect, ftp_close
 */
void
do_ftp_protocol(void)               /* int (int id, [string &name]) */
{
    const int id = get_xinteger(1, -1);
    IFTP *iftp = iftplookup(id);
    int ret = -1;

    if (iftp) {
        argv_assign_str(2, schemename(iftp->proto));
        ret = iftp->proto;
    }
    acc_assign_int(ret);
}


/*<<GRIEF-BETA>>
    Macro: ftp_set_options - Reserved

        int
        ftp_set_options(int id, [int flags], [string cmd], [list alg])

    Macro Description:
        The 'ftp_set_options()' primitive is reserved for future use.

    Macro Parameters:
        n/a

    Macro Return:
        n/a

    Macro Portability:
        A Grief extension.

    Macro See Also:
        ftp_create, ftp_connect, ftp_close
 */
void
do_ftp_set_options(void)            /* int (int id, [int what], [string|list spec]) */
{
    /*TODO*/
    acc_assign_int(-1);
}


void
do_ftp_get_options(void)            /* int (int id, [int what], [string|list &spec]) */
{
    /*TODO*/
    acc_assign_int(-1);
}


/*<<GRIEF-BETA>>
    Macro: ftp_sitename - Reserved

        int
        ftp_sitename(int id, [string sitename])

    Macro Description:
        The 'ftp_sitename()' primitive is reserved for future use.

        The 'ftp_sitename()' primitive can be used to associate a
        unique user defined 'site-name' with an ftp connection.

        Sitenames can be utilised by the <ftp_find_connection>
        primitive to locate connections based on their name, rather
        than their URL.

    Macro Parameters:
        id - Connection identifier.
        sitename - Optional string containing the connection
            site-name to be associated with the connect; to clear
            assign an empty string. If omitted the name is not
            changed and the current site-name is returned.

    Macro Return:
        The 'ftp_sitename()' primitive returns the associated
        site-name, otherwise a null on error.

    Macro Portability:
        A Grief extension.

    Macro See Also:
        ftp_create, ftp_connect, ftp_close
 */
void
do_ftp_sitename(void)               /* int (int id, [string name]) */
{
    const int id = get_xinteger(1, -1);
    const char *sitename = get_xstr(2);
    IFTP *iftp = iftplookup(id);

    if (iftp && sitename) {

        if (*sitename) {                        /* unique check */
            stypecursor_t cursor = {0};
            sentry_t *st;

            for (st = stype_first(x_ftps, &cursor); st; st = stype_next(&cursor)) {
                const IFTP *t_iftp = (const IFTP *)st->se_ptr;

                assert(IFTP_MAGIC == t_iftp->magic);
                if (iftp != t_iftp) {
                    if (t_iftp->sitename) {
                        if (0 == strcmp(sitename, t_iftp->sitename)) {
                            sitename = NULL;
                            break;
                        }
                    }
                }
            }
        }

        if (sitename) {                         /* unique */
            chk_free((char *)iftp->sitename);
            if (*sitename) {
                iftp->sitename = chk_salloc(sitename);
            } else {
                iftp->sitename = NULL;
            }
        }
    }
    acc_assign_str(sitename ? sitename : "");
}


#if defined(HAVE_LIBSSH2)
static int
sftp_init(void)
{
    static int init;
    if (0 == init) {
        if (libssh2_init(0))
            return -1;
        ++init;
    }
    return 0;
}
#endif

/*end*/
