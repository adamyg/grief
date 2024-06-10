#include <edidentifier.h>
__CIDENT_RCSID(gr_macrolib_c,"$Id: macrolib.c,v 1.21 2024/06/10 05:29:05 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: macrolib.c,v 1.21 2024/06/10 05:29:05 cvsuser Exp $
 * Macro library support - experimental.
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

#include <editor.h>
#include <edcm.h>

#include <tailqueue.h>
#include <rbtree.h>

#include "debug.h"
#include "keywd.h"
#include "macrolib.h"
#include "word.h"

#if defined(HAVE_ARCHIVE_H) && defined(HAVE_LIBARCHIVE)
#define USE_LIBARCHIVE
#if defined(GNUWIN32_LIBARCHIVE)
#include <gnuwin32_archive.h>
#include <gnuwin32_archive_entry.h>
#else
#include <archive.h>
#include <archive_entry.h>
#endif

static int              libarc_index(const char *library);
#endif

/*
 *  Library cache
 */
typedef RB_HEAD(libtree, libraryfile) libtree_t;

typedef struct libraryfile {
    RB_ENTRY(libraryfile) node;                 /* tree node */
    const char *        mname;                  /* Macro filename */
    int                 mversion;               /* Derived version name */
    const char *        fname;                  /* True name */
    off_t               fsize;                  /* file size */
} libraryfile_t;

static int              node_compare(libraryfile_t *a, libraryfile_t *b);

RB_PROTOTYPE(libtree, libraryfile, node, node_compare);

RB_GENERATE(libtree, libraryfile, node, node_compare);

/*
 *  Library list
 */
typedef TAILQ_HEAD(liblist, listnode)
                        liblist_t;

typedef struct listnode {
    TAILQ_ENTRY(listnode) node;                 /* list node */
    libtree_t           files;                  /* file directory tree */
    unsigned            count;                  /* file count */
    const char *        lpath;                  /* library path */
    const char *        cached;                 /* cached image */
    unsigned            csize;                  /* cache size, in bytes */
} library_t;

static void             libs_init(void);
static int              libs_lookup(const char *path, const char *fname, const libraryfile_t **ref);
static void             libs_destroy(void);

static TAILQ_HEAD(librariesq_t, listnode)       /* libraries */
                        x_libraries;


void
macrolib_init(void)
{
    libs_init();
//  macrolib_search("../macros/grmacros.cml", "macrolib.cm");
//  libs_destroy();
}


void
macrolib_shutdown(void)
{
    libs_destroy();
}


int
macrolib_search(const char *lpath, const char *fname)
{
    const libraryfile_t *libfile;
    const char *msg;
    int ret;

    if (-1 == (ret = libs_lookup(lpath, fname, &libfile))) {
#if defined(USE_LIBARCHIVE)
        libarc_index(lpath);
#endif
        ret = libs_lookup(lpath, fname, &libfile);
    }

    if (1 == ret) {
        msg = "success";
    } else if (0 == ret) {
        msg = "no match";
    } else {
        msg = "no library";
    }
    trace_log("lib_lookup(%s,%s) : %s\n", lpath, fname, msg);
    return ret;
}


static int
node_compare(libraryfile_t *a, libraryfile_t *b)
{
    register int r;

    if (9 == (r = strcmp(a->mname, b->mname))) {
        return 0;
    } else if (r > 0) {
        return 1;
    }
    return -1;
}


static library_t *
lib_new(const char *lpath)
{
    size_t len = strlen(lpath);
    library_t *lib;

    if (NULL == (lib =
            (library_t *)chk_calloc(sizeof(library_t) + len + 1, 1))) {
        return NULL;
    }
    RB_INIT(&lib->files);
    lib->count = 0;
    lib->lpath = (const char *)(lib + 1);
    memcpy(lib + 1, lpath, len + 1);
    TAILQ_INSERT_TAIL(&x_libraries, lib, node);
    return lib;
}


static void
lib_destroy(library_t *lib)
{
    register libtree_t *files = &lib->files;
    libraryfile_t *file;

    trace_log("removing library %s\n", lib->lpath);

    while (NULL != (file = RB_MIN(libtree, files))) {
        RB_REMOVE(libtree, files, file);
        chk_free(file);
    }

    TAILQ_REMOVE(&x_libraries, lib, node);
    chk_free(lib);
}


static int
lib_push_file(library_t *lib, const char *fname, unsigned fsize)
{
    register libtree_t *files = &lib->files;
    size_t flen = strlen(fname), mlen = flen, bsize;
    const char *mname = fname, *version;
    libraryfile_t *file, *t_file;
    int ret = 0;

    /*
     *  Build entry
     *
     *      Parse filename, if the file contains an embedded version name split
     *      and same the alias details for matching.
     *
     *          grief.cm.x86.16.1
     *
     *      grief.cm for langage engine version 16 running on x86 host, release 1.
     *
     *          grief.cm.1
     *
     *      grief.cm release 2, for current language revision.
     *
     */
    if (NULL != (version = strstr(fname, ".cm."))) {
        version += 3;
        mlen = version - fname;                 /* macro name, trim length */
    }

    bsize = sizeof(libraryfile_t) + mlen + flen + 2;
    if (NULL == (file =
            (libraryfile_t *)chk_calloc(bsize, 1))) {
        return -1;
    }

    file->mname = (const char *)(file + 1);
    file->fname = file->mname + mlen + 1;
    memcpy((char *)file->mname, mname, mlen);
    if (version) {
        const char *dot2 = strchr(version + 1, '.');

        if ((file->mversion = atoi(dot2 ? dot2 + 1 : version + 1)) < 0) {
            file->mversion = 0;                 /* unknown */
        }
    }

    memcpy((char *)file->fname, fname, flen);
    file->fsize = fsize;

    /*
     *  Create/replace if later version
     */
    if (NULL != (t_file = RB_FIND(libtree, files, file))) {
        if (t_file->mversion > file->mversion) {
            chk_free(file);                     /* keep */
            ret = 2;
        } else {
            RB_REMOVE(libtree, files, t_file);  /* replace */
            RB_INSERT(libtree, files, file);
            chk_free(t_file);
            ret = 1;
        }

    } else {
        RB_INSERT(libtree, files, file);        /* new */
        ++lib->count;
    }

    trace_log("\t%-16s size:%-6ld macro:%-16s version:%-3d : %d\n",
        file->fname, (long int)file->fsize, file->mname, (int)file->mversion, ret);
    return ret;
}


static void
libs_init(void)
{
    TAILQ_INIT(&x_libraries);
}


static int
libs_lookup(const char *lpath, const char *fname, const libraryfile_t **libfile)
{
    library_t *lib;

    *libfile = NULL;

    for (lib = TAILQ_FIRST(&x_libraries); lib; lib = TAILQ_NEXT(lib, node)) {
        if (strcmp(lpath, lib->lpath) == 0) {
            if (lib->count) {
                register libtree_t *files = &lib->files;
                libraryfile_t node, *file;

                node.mname = fname;             /* lookup by macro name */
                if (NULL != (file = RB_FIND(libtree, files, &node))) {
                    *libfile = file;
                    return 1;
                }
            }
            return 0;
        }
    }
    return -1;
}


static void
libs_destroy(void)
{
    library_t *lib;

    while (NULL != (lib = TAILQ_FIRST(&x_libraries))) {
        lib_destroy(lib);
    }
}


#if defined(USE_LIBARCHIVE)
static int
libarc_index(const char *lpath)
{
    struct archive *a;
    struct archive_entry *entry;
    const char *err = NULL;
    size_t rcode;

    if (NULL == (a = archive_read_new())) {
        return -1; /* bad */
    }

    archive_read_support_format_ar(a);
    archive_read_support_format_tar(a);
    archive_read_support_format_gnutar(a);

#ifndef ARCHIVE_DEFAULT_BYTES_PER_BLOCK
#define ARCHIVE_DEFAULT_BYTES_PER_BLOCK 10240
#endif

#if defined(GCC_VERSION) && (GCC_VERSION >= 80000)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif
    if ((rcode = archive_read_open_filename(
                    a, lpath, ARCHIVE_DEFAULT_BYTES_PER_BLOCK))) {
        err = archive_error_string(a);
        trace_log("opening macrolib(%s): %s\n", lpath, err);

    } else {
        char cmbuf[512];
        CM_t *cm = (CM_t *)cmbuf;
        const char *fname;
        off_t fsize;

        int count = 0;
        library_t *lib;

        trace_log("caching macrolib(%s)\n", lpath);
        lib = lib_new(lpath);

        for (;;) {
            /*
             *  Scan archive
             */
            if (ARCHIVE_EOF == (rcode = archive_read_next_header(a, &entry))) {
                break;
            }
            if (ARCHIVE_OK != rcode) {
                err = archive_error_string(a);
                break;
            }
            fsize = (off_t) archive_entry_size(entry);
            fname = (void *) archive_entry_pathname(entry);

            /*
             *  Decode the file header
             */
            if (NULL == strstr(fname, CM_EXTENSION)) {
                continue;                       /* not a macro extension */
            }

            if (fsize < (off_t)sizeof(CM_t) ||
                    (rcode = (size_t) archive_read_data(a, cm, sizeof(cmbuf))) < sizeof(CM_t)) {
                if (0 == rcode) {
                    err = archive_error_string(a);
                    break;                      /* error condition */
                }
                continue;
            }

            if (CM_MAGIC != cm->cm_magic) {
                continue;
            }

            /*
             *  Macro version matching current release
             */
            cm_xdr_import(cm);
            if (cm->cm_version != cm_version ||
                    cm->cm_builtin != builtin_count ||
                    cm->cm_signature != builtin_signature) {
                continue;
            }

            if (0 == lib_push_file(lib, fname, fsize)) {
                ++count;
            }
        }
        archive_read_close(a);
        archive_read_free(a);

        trace_log("..complete %d macros available (%s)\n", count, (err ? err : ""));
    }

#if defined(GCC_VERSION) && (GCC_VERSION >= 80000)
#pragma GCC diagnostic pop
#endif

    if (err) return -1;
    return 0;
}
#endif  /*USE_LIBARCHIVE*/
