#include <edidentifier.h>
__CIDENT_RCSID(gr_tags_c,"$Id: tags.c,v 1.23 2022/08/10 15:44:58 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: tags.c,v 1.23 2022/08/10 15:44:58 cvsuser Exp $
 * tag database access.
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

#define ED_LEVEL 1
#define ED_ASSERT
#include <editor.h>
#include <edfileio.h>
#include <edthreads.h>
#include <eddebug.h>
#include <edassert.h>

#include <tailqueue.h>
#include <bsd_ndbm.h>
#include <stable.h>
#include <libstr.h>                             /* str_...()/sxprintf() */

#include "tags.h"                               /* public header */
#include "tagsex.h"                             /* extags interface functions */
#include "tagse.h"                              /* emacs */

#include "accum.h"
#include "builtin.h"
#include "debug.h"
#include "eval.h"
#include "lisp.h"
#include "sysinfo.h"
#include "system.h"
#include "word.h"

#define TAG_CTAGS               0x01
#define TAG_ETAGS               0x02

#define TABLESIZE               512
#define TABLEFACTOR             4

#define EXTAG_HANDLE            0x42
#define ETAG_HANDLE             0x69

typedef TAILQ_HEAD(tlist, tag)
                        TagList_t;

typedef struct tag {
    TAILQ_ENTRY(tag)    t_node;
    const char *        t_name;
    const char *        t_file;
    int                 t_flags;
#define TAG_FLINENO             0x01
#define TAG_FLOCAL              0x02
    union {
        const char *    pattern;
        int32_t         line;
    } t_addr;
} Tag_t;

struct tags;

typedef int (*TagsSearch_t)(struct tags *tags, const char *pattern, int options);
typedef int (*TagsIndex_t)(struct tags *tags, DBM *db);
typedef int (*TagsClose_t)(void *);

typedef struct tags {
    uint32_t            tg_magic;               /* structure magic */
#define TAGS_MAGIC              MKMAGIC('T','a','G','S')
    mtx_t               tg_guard;               /* access guard */
    TAILQ_ENTRY(tags)   tg_node;
    IDENTIFIER_t        tg_ident;               /* handle identifier */
    void *              tg_file;                /* underlying tag file handle */
    const char *        tg_idxname;
    DBM *               tg_idxdbm;
    int                 tg_idxntags;
    unsigned            tg_status;
#define TAGS_STATUS_BUILDING    0x01
#define TAGS_STATUS_INDEXED     0x02
#define TAGS_STATUS_DESTROYED   0x04
    TagList_t           tg_tags;
    stable_t            tg_strings;
    TagsSearch_t        tg_search;              /* search implementation tags database */
    TagsIndex_t         tg_index;               /* index tags database */
    TagsClose_t         tg_close;               /* close tags */
} Tags_t;

typedef struct dbrec {
    uint16_t            flags;                  /* flags */
    uint16_t            kind;                   /* element kind */
    uint32_t            line;                   /* line number */
    uint32_t            filelength;             /* length of the filename, in bytes */
    char                filename[1];            /* first byte of the filename */
} dbrec_t;

static Tags_t *             tags_new(void);
static void                 tags_destroy(Tags_t *tags);
static void                 tags_delete(Tags_t *tags);
static Tags_t *             tags_find(int ident);

static void                 tagidx_build(Tags_t *tags, int background);
static int                  tagidx_lookup(Tags_t *tags, const char *word);
static void                 tagidx_close(Tags_t *tags);

static int                  extag_search(Tags_t *tags, const char *pattern, int options);
static int                  extag_index(Tags_t *tags, DBM *db);

static int                  etag_search(Tags_t *tags, const char *pattern, int options);
static int                  etag_index(Tags_t *tags, DBM *db);

static int                  tagl_assign(TagList_t *, int);
static void                 tagl_free(TagList_t *);

static IDENTIFIER_t         x_tagsseq;          /* identifier sequence */
static TAILQ_HEAD(tagslist, tags)               /* and container */
                            x_tagslist;


int
tags_check(const char *word, int wordlen)
{
    if (x_tagsseq) {
        struct tagslist *tagslist = &x_tagslist;
        const Tags_t *tags;
        datum key;

        TAILQ_FOREACH(tags, tagslist, tg_node) {
            if (TAGS_STATUS_INDEXED & tags->tg_status) {
                key.dptr = (void *)word;        /* tags symbol lookup */
                key.dsize = wordlen;
                key = bsddbm_fetch(tags->tg_idxdbm, key);
                if (key.dptr) {
                    return 1;
                }
            }
        }
    }
    return -1;
}


void
tags_shutdown(void)
{
    if (x_tagsseq) {
        struct tagslist *tagslist = &x_tagslist;
        Tags_t *tags;

        while (NULL != (tags = TAILQ_FIRST(tagslist))) {
            mtx_lock(&tags->tg_guard);          /* -- lock */
            tags_destroy(tags);
        }
    }
}


/*<<GRIEF>>
    Macro: tagdb_open - Tag database open

        int
        tagdb_open(
            string file, [int options], [int background])

    Macro Description:
        The 'tagdb_open()' primitive given a pathname for a ctags
        database, returns a handle being a non-negative integer for
        use in subsequent database search operations using
        <tagdb_search>. The tag database handle shall remain open
        until <tagtb_close> is executed against the handle.

        ctags is a tool which permit easy navigation thru a large
        set of source files. ctags supports many languages
        including 'c', 'c++' and 'Java' just to name a few.

        Note!:
        Grief relies on an external tag file generator. There are
        many versions of ctags; however, the recommended version is
        "Exuberant Ctags" available from
            http://ctags.sourceforge.net/.

        Grief is generally bundled with a recent version within the
        bin installation folder as 'extags'. Therefore, you would
        not need to download/install a tag binary to use this
        feature.

    Macro Parameters:
        file - tag database path.
        options - Optional integer flags, being one or more of the
            following constants OR'ed together forming open options.

            o TAG_ETAGS
            o TAG_CTAGS

        background - Optional integer boolean value, if *true* the database
            loading shall be moved into the background.

    Macro Returns:
        The 'tagdb_open()' primitive returns the new database descriptor,
        otherwise -1 if an error occurred.

    Macro Portability:
        A Grief extension.

    Macro See Also:
        tagdb_search, tagdb_close
*/
void
do_tagdb_open(void)             /* (string file, [int options], [int background]) */
{
    const char *file = get_str(1);
    const int options = get_xinteger(2, TAG_ETAGS|TAG_CTAGS);
//  const int background = get_xinteger(3, FALSE);
    Tags_t *tags;
    int ret = -1;

    if (NULL != (tags = tags_new())) {          /* --- lock */

        if (options & TAG_ETAGS)
                                                /* etags */
            if (NULL != (tags->tg_file = etagsOpen(file))) {
                ED_ITRACE(("=> etags\n"))
                tags->tg_search = etag_search;
                tags->tg_index = etag_index;
                tags->tg_close = etagsClose;
            }

        if (NULL == tags->tg_file) {            /* extags/ctags */
            if (options & TAG_CTAGS) {
                extagFileInfo info = {0};

                if (NULL != (tags->tg_file = extagsOpen(file, &info))) {
                    ED_ITRACE(("=> extags\n"))
                    tags->tg_search = extag_search;
                    tags->tg_index = extag_index;
                    tags->tg_close = (TagsClose_t) extagsClose;
                }
            }
        }

        if (tags->tg_search) {
            ret = tags->tg_ident;
            if (tags->tg_index) {
                tagidx_build(tags, FALSE);
            }
            mtx_unlock(&tags->tg_guard);        /* -- unlock */

        } else {
            tags_destroy(tags);                 /* -- destroy */
        }
    }
    acc_assign_int(ret);
}


/*<<GRIEF>>
    Macro: tagdb_search - Tag database search

        int
        tagdb_search(
            int handle, string word, [int flags])

    Macro Description:
        The 'tagdb_search()' primitive searches the tag database for
        symbols matching 'pattern'.

        Note!:
        Consult the 'tags' macro source for an example.

    Macro Parameters:
        handle - Tag database handle.
        pattern - String containing the search pattern.
        flags - Optional integer flags.

    Macro Returns:
        The 'tagdb_search' returns a list containing the search
        results, otherwise a NULL list on error or no match was found.

    Macro Portability:
        A Grief extension.

    Macro See Also:
        tagdb_open, tagdb_close
*/
void
do_tagdb_search(void)           /* (int handle, string word, [int flags] */
{
    const int ident = get_xinteger(1, -1);
    const char *pattern = get_str(2);
    const int flags = get_xinteger(3, 0);
    int ntags = -1;
    Tags_t *tags;

    if (NULL != (tags = tags_find(ident))) {    /* -- lock */
        tagl_free(&tags->tg_tags);
        stbl_clear(&tags->tg_strings);
        ntags = (*tags->tg_search)(tags, pattern, flags);
    }

    if (ntags <= 0 || -1 == tagl_assign(&tags->tg_tags, ntags)) {
        if (tags) {
            tagl_free(&tags->tg_tags);
            stbl_clear(&tags->tg_strings);
        }
        acc_assign_null();
    }

    if (tags) {
        mtx_unlock(&tags->tg_guard);            /* -- unlock */
    }
}


/*<<GRIEF>>
    Macro: tagdb_close - Tag database close

        int
        tagdb_close(int handle)

    Macro Description:
        The 'tagdb_close()' primitive closes a tag database handle, so that
        it no longer refers to any resources and may be reused.

    Macro Parameters:
        handle - Tag database handle.

    Macro Returns:
        nothing

    Macro Portability:
        A Grief extension.

    Macro See Also:
        tagdb_open, tagdb_search
*/
void
do_tagdb_close(void)            /* (int handle) */
{
    const int ident = get_xinteger(1, -1);
    Tags_t *tags;

    if (NULL != (tags = tags_find(ident))) {
        tags_destroy(tags);                     /* -- destroy */
    }
}


static Tags_t *
tags_new(void)
{
    Tags_t *tags;

    if (0 == x_tagsseq) {
        TAILQ_INIT(&x_tagslist);
        x_tagsseq = 8999;                       /* seed sequence 9000+ */
    }
    if (NULL == (tags = chk_alloc(sizeof(Tags_t)))) {
        return NULL;
    }
    memset(tags, 0, sizeof(Tags_t));
    tags->tg_magic = TAGS_MAGIC;
    mtx_init(&tags->tg_guard, mtx_plain);
    mtx_lock(&tags->tg_guard);                  /* -- lock */
    tags->tg_ident = ++x_tagsseq;
    TAILQ_INSERT_HEAD(&x_tagslist, tags, tg_node);
    TAILQ_INIT(&tags->tg_tags);
    stbl_open(&tags->tg_strings, TABLESIZE, TABLEFACTOR);
    return tags;
}


static void
tags_destroy(Tags_t *tags)
{
    if (tags->tg_close) {
        (*tags->tg_close)(tags->tg_file);
    }

    TAILQ_REMOVE(&x_tagslist, tags, tg_node);

    if (TAGS_STATUS_BUILDING & tags->tg_status) {
        tags->tg_status |= TAGS_STATUS_DESTROYED;
        mtx_unlock(&tags->tg_guard);           /* -- unlock */
        return;
    }

    tagidx_close(tags);
    tags_delete(tags);                         /* -- delete */
}


static void
tags_delete(Tags_t *tags)
{
    assert(TAGS_MAGIC == tags->tg_magic);
    tagl_free(&tags->tg_tags);
    stbl_close(&tags->tg_strings);
    mtx_unlock(&tags->tg_guard);               /* -- unlock */
    mtx_destroy(&tags->tg_guard);
    chk_free(tags);
}


static Tags_t *
tags_find(int ident)
{
    Tags_t *tags;

    for (tags = TAILQ_FIRST(&x_tagslist); tags; tags = TAILQ_NEXT(tags, tg_node)) {
        assert(TAGS_MAGIC == tags->tg_magic);
        mtx_lock(&tags->tg_guard);              /* -- lock */
        if (tags->tg_ident == ident) {
            return tags;
        }
    }
    return NULL;
}


/*
 *  Tag index
 */
static int
builder_task(void *p)
{
    Tags_t *tags = (Tags_t *)p;
    int ntags;

    assert(TAGS_MAGIC == tags->tg_magic);
    ntags = (*tags->tg_index)(tags, tags->tg_idxdbm);
    mtx_lock(&tags->tg_guard);                  /* -- lock */
    if (TAGS_STATUS_DESTROYED & tags->tg_status) {
        tagidx_close(tags);
        tags_delete(tags);                      /* -- delete */
        return 0;
    }
    tags->tg_idxntags = ntags;
    tags->tg_status = TAGS_STATUS_INDEXED;
    mtx_unlock(&tags->tg_guard);                /* -- unlock */
    return 1;
}


static void
tagidx_build(Tags_t *tags, int background)
{
    const char *home = sysinfo_homedir(NULL, -1);
    char *name, t_name[MAX_PATH];
    int namelen;
    DBM *db;

    assert(TAGS_MAGIC == tags->tg_magic);
    assert(NULL != tags->tg_index);
    assert(NULL == tags->tg_idxdbm);

    namelen = sxprintf(t_name, sizeof(t_name), "%s%c.crtdx%d-%d",
                    home, FILEIO_PATHSEP, tags->tg_ident, sys_getpid());

    ED_TRACE(("tagidx_build(baclground:%d,name:%s)\n", background, t_name))

    if (NULL != (name = chk_alloc(namelen + sizeof(DBM_SUFFIX))) &&
            NULL != (db = bsddbm_open(t_name, O_RDWR|O_CREAT, 0666))) {

        tags->tg_idxname = name;
        memcpy(name, (const char *)t_name, namelen);
        memcpy(name + namelen, DBM_SUFFIX, sizeof(DBM_SUFFIX));
        unlink(name);                           /* temporary resource */

        tags->tg_status = TAGS_STATUS_BUILDING;
        tags->tg_idxdbm = db;
        if (background) {                       /* build database as background task */
            thrd_t thr /*= 0*/;
            if (thrd_success == thrd_create(&thr, builder_task, (void *)tags)) {
                thrd_detach(thr);
                return;
            }
        }
                                                /* otherwise direct */
        tags->tg_idxntags = (*tags->tg_index)(tags, db);
        tags->tg_status = TAGS_STATUS_INDEXED;
    }
}


static void
tagidx_close(Tags_t *tags)
{
    assert(TAGS_MAGIC == tags->tg_magic);
    if (tags->tg_idxname) {
        if (tags->tg_idxdbm) {
            bsddbm_close(tags->tg_idxdbm);
#if defined(DOSISH)
            remove(tags->tg_idxname);           /* temporary resource */
#endif
            tags->tg_idxdbm = NULL;
        }
        chk_free((void *)tags->tg_idxname);
        tags->tg_idxname = NULL;
    }
}


/*
 *  Scan Tags_t database
 */
static int
extag_search(Tags_t *tags, const char *pattern, int options)
{
    void *const file = tags->tg_file;
    extagEntry entry;
    int ntags = 0;

    if (options == -1) {
        options = 0;
    }
    if (TagSuccess == extagsFind(file, &entry, pattern, options)) {
        do {
            struct tag *t;

            /* new node */
            if (NULL == (t = (struct tag *)chk_alloc(sizeof(*t))) ||
                    NULL == (t->t_name = chk_salloc(entry.name)) ||
                        NULL == (t->t_file = stbl_insert(&tags->tg_strings, entry.file))) {
                if (t) chk_free((void *)t->t_name);
                chk_free((void *)t);
                break;
            }

            t->t_flags = 0;
            if ((t->t_addr.line = entry.address.lineNumber) != 0) {
                t->t_flags |= TAG_FLINENO;
            } else {
                t->t_addr.pattern = chk_salloc(entry.address.pattern);
            }
            if (entry.fileScope) {
                t->t_flags |= TAG_FLOCAL;
            }

            /* enqueue */
            TAILQ_INSERT_TAIL(&tags->tg_tags, t, t_node);
            ++ntags;

        } while (TagSuccess == extagsFindNext(file, &entry));
    }
    return ntags;
}


static int
extag_index(Tags_t *tags, DBM *db)
{
    void *file = tags->tg_file;
    dbrec_t *dbrec;
    datum key, data;
    extagEntry entry;
    int ntags = 0;

    ED_TRACE(("extag_index()\n"))

    if (NULL != (dbrec = chk_alloc(sizeof(dbrec) + 1024))) {

        if (TagSuccess == extagsFirst(file, &entry)) {
            do {
                const size_t filelength = strlen(entry.file);
                size_t namelength = strlen(entry.name);
                char kind = 0;

                if (entry.kind) {
                    kind = *entry.kind;
                    switch (kind) {
                    case 'v':   // variable
                    case 'x':   // extern var's
                        namelength = 0;
                        break;
                    case 'm': { // members (class, struct)
                        //  int count = entry.fields.count;
                        //  extagExtensionField *list = entry.fields.list;
                        //
                        //  for (;count-- > 0; ++list) {
                        //      const char *key = list->key;
                        //      if (0 == strcmp("class", key)) {
                        //          //TODO
                        //      } else if (0 == strcmp("struct", key)) {
                        //          //TODO
                        //      }
                        //   }
                            namelength = 0;
                        }
                        break;
                    }
                }

                if (namelength && filelength < 1024) {
                    dbrec->kind  = kind;
                    dbrec->flags = 0;
                    dbrec->line  = (uint32_t)entry.address.lineNumber;
                    dbrec->filelength = (uint32_t)filelength;
                    memcpy(dbrec->filename, (const char *)entry.file, filelength + 1);

                    key.dptr = (void *)entry.name;
                    key.dsize = namelength;
                    data.dptr = (void *)dbrec;
                    data.dsize = sizeof(dbrec_t) + filelength;
                    (void) bsddbm_store(db, key, data, DBM_REPLACE);
                    ++ntags;
                }

            } while (0 == (TAGS_STATUS_DESTROYED & ((volatile Tags_t *)tags)->tg_status) &&
                        TagSuccess == extagsNext(file, &entry));
        }
    }

    chk_free(dbrec);
    ED_TRACE(("extag_index()=%d\n", ntags))
    return ntags;
}


static int
etag_search(Tags_t *tags, const char *pattern, int options)
{
    etagEntry entry;
    int ntags = 0;

    if (0 == etagsFind (tags->tg_file, &entry, pattern, options)) {
        do {
            struct tag *t;

            /* new node */
            if (NULL == (t = (struct tag *)chk_alloc(sizeof(*t))) ||
                    NULL == (t->t_name = chk_salloc(entry.name)) ||
                        NULL == (t->t_file = stbl_insert(&tags->tg_strings, entry.file))) {
                if (t) chk_free((void *)t->t_name);
                chk_free((void *)t);
                break;
            }
            t->t_flags = 0;
            if ((t->t_addr.line = entry.line) != 0) {
                t->t_flags |= TAG_FLINENO;
            } else {
                t->t_addr.pattern = chk_salloc(entry.pattern);
            }

            /* enqueue */
            TAILQ_INSERT_TAIL(&tags->tg_tags, t, t_node);
            ++ntags;

        } while (0 == etagsFindNext (tags->tg_file, &entry));
    }
    return ntags;
}


static int
etag_index(Tags_t *tags, DBM *db)
{
    void *file = tags->tg_file;
    dbrec_t *dbrec;
    datum key, data;
    etagEntry entry;
    int ntags = 0;

    ED_TRACE(("etag_index()\n"))

    if (NULL != (dbrec = chk_alloc(sizeof(dbrec) + 1024))) {

        if (0 == etagsFirst(file, &entry)) {
            do {
                const size_t filelength = strlen(entry.file);
                const size_t namelength = strlen(entry.name);

                if (namelength && filelength < 1024) {
                    dbrec->kind = 0;
                    dbrec->flags = 0;
                    dbrec->line = (uint32_t)entry.line;
                    dbrec->filelength = (uint32_t)filelength;
                    memcpy(dbrec->filename, (const char *)entry.file, filelength + 1);

                    key.dptr = (void *)entry.name;
                    key.dsize = namelength;
                    data.dptr = (void *)dbrec;
                    data.dsize = sizeof(dbrec_t) + filelength;
                    (void) bsddbm_store(db, key, data, DBM_REPLACE);
                    ++ntags;
                }

            } while (0 == (TAGS_STATUS_DESTROYED & ((volatile Tags_t *)tags)->tg_status) &&
                        0 == etagsNext(file, &entry));
        }
    }

    chk_free(dbrec);
    ED_TRACE(("etag_index()=%d\n", ntags))
    return ntags;
}


static int
tagl_assign(TagList_t *tagl, int ntags)
{
    LIST *newlp, *lp;
    struct tag *t;
    int llen;

#define FIELDS          4

    /* Allocate memory the list. */
    if (ntags > 3200) ntags = 3200;             /* FIXME - limit to 64k result */
    assert(sizeof_atoms[F_RSTR] == sizeof_atoms[F_INT]);
    llen = ((ntags * FIELDS) * sizeof_atoms[F_RSTR]) + sizeof_atoms[F_HALT];
    if (NULL == (newlp = lst_alloc(llen, ntags * FIELDS))) {
        acc_assign_null();
        return -1;
    }

    /* Assign */
    lp = newlp;
    for (t = TAILQ_FIRST(tagl); t && (ntags-- > 0); t = TAILQ_NEXT(t, t_node)) {
        lp = atom_push_const(lp, t->t_name);
    /*- lp = atom_push_const(lp, t->t_prototype); -*/
        lp = atom_push_const(lp, t->t_file);
        lp = atom_push_int(lp, t->t_flags);     /* TODO - publish usage */
        if (t->t_flags & TAG_FLINENO) {
            lp = atom_push_int(lp, t->t_addr.line);
        } else {
            lp = atom_push_const(lp, t->t_addr.pattern);
        }
    }
    atom_push_halt(lp);
    lst_check(newlp);
    acc_donate_list(newlp, llen);
    return 0;
}


static void
tagl_free(TagList_t *tagl)
{
    struct tag *t;

    while ((t = TAILQ_FIRST(tagl)) != NULL) {
        TAILQ_REMOVE(tagl, t, t_node);
        chk_free((void *)t->t_name);
        if ((t->t_flags & TAG_FLINENO) == 0) {
            chk_free((void *)t->t_addr.pattern);
        }
        chk_free(t);
    }
}

/*end*/
