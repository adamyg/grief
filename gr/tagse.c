#include <edidentifier.h>
__CIDENT_RCSID(gr_tagse_c,"$Id: tagse.c,v 1.13 2014/10/26 22:13:14 ayoung Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: tagse.c,v 1.13 2014/10/26 22:13:14 ayoung Exp $
 * tag (emac style) database access
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
#include <edtrace.h>
#include "tagse.h"                              /* private interface */
#include <assert.h>

typedef struct {
    MAGIC_t             magic;                  /* structure magic */
#define VSTRING_MAGIC       MKMAGIC('T','g','V','s')
#define VSTRING_BLOCK       256
    size_t              size;
    char *              buffer;
} vstring;

typedef struct {
    MAGIC_t             magic;                  /* structure magic */
#define TAGSE_MAGIC         MKMAGIC('T','g','S','e')
    FILE *              fp;                     /* file structure */
    vstring             tags;                   /* tags filename */

    vstring             line;                   /* last line read */
    vstring             file;                   /* filename of last tag in last line read */
    vstring             name;                   /* name of last tag in last line read */
    etagEntry           entry;                  /* current entry */

    struct {                                    /* defines tag search state */
        const char *    name;                   /* name of tag last searched for */
        size_t          length;                 /* length of name for partial matches */
        int             partial;                /* peforming partial match */
        int             ignorecase;             /* ignoring case */
    } s;
} etags;

static etags *          initialize(const char *const filePath);
static void             terminate(etags *const file);

#if defined(ED_LEVEL) && (ED_LEVEL)
#define DD(x)           trace_log x;
#else
#define DD(x)
#endif


/*
 *  Compare two strings, ignoring case.
 *  Return 0 for match, < 0 for smaller, > 0 for bigger
 *  Make sure case is folded to uppercase in comparison (like for 'sort -f')
 *  This makes a difference when one of the chars lies between upper and lower
 *  ie. one of the chars [ \ ] ^ _ ` for ascii. (The '_' in particular !)
 */
static int
struppercmp(const char *s1, const char *s2)
{
    int result;

    do {
        result = toupper(*((unsigned char *)s1)) - toupper(*((unsigned char *)s2));
    } while (0 == result && *s1++ && *s2++);
    return result;
}


static int
strnuppercmp(const char *s1, const char *s2, size_t n)
{
    int result;

    do {
        result = toupper(*((unsigned char *)s1)) - toupper(*((unsigned char *)s2));
    } while (0 == result && --n > 0 && *s1++ && *s2++);
    return result;
}


static int
strgrow(vstring *s)
{
    size_t size;
    char *buffer;

    if (0 == s->size) {
        size   = VSTRING_BLOCK;
        buffer = (char *) malloc(size);
    } else {
        assert(VSTRING_MAGIC == s->magic);
        size   =  2 * s->size;
        buffer = (char *) realloc(s->buffer, size);
    }
    if (NULL == buffer) {
        DD(("tagse: (E) string too large\n"))
        return -1;
    }
    buffer[s->size] = 0;
    s->magic = VSTRING_MAGIC;
    s->buffer = buffer;
    s->size = size;
    return 0;
}


static void
strfree(vstring *s)
{
    if (s->buffer) {
        assert(VSTRING_MAGIC == s->magic);
        free(s->buffer);
        s->buffer = NULL;
    }
    s->size = 0;
}


static void
strcopy(vstring *s, const char *buffer, char delim)
{
    const char *end;
    size_t length;

    if (!delim || NULL == (end = strchr(buffer, delim))) {
        if (NULL == (end = strchr(buffer, '\n'))) {
            end = strchr(buffer, '\r');
        }
    }

    length = (end ? (size_t)(end - buffer) : strlen(buffer));

    while (length >= s->size) {
        if (-1 == strgrow(s)) {
            length = (s->size - 1);
            break;
        }
    }

    assert(VSTRING_MAGIC == s->magic);
    strncpy(s->buffer, buffer, length);
    s->buffer[ length ] = '\0';
}


static etags *
initialize(const char *const tags)
{
    etags *file;

    if (NULL != (file = (etags *) malloc(sizeof (etags)))) {

        memset(file, 0, sizeof(etags));
        file->magic = TAGSE_MAGIC;

        strgrow(&file->tags);
        strgrow(&file->line);
        strgrow(&file->file);
        strgrow(&file->name);

        DD(("tagse: (D) opening <%s>\n", tags))

        if (NULL == (file->fp = fopen(tags, "rb"))) {
            DD(("tagse: (D) open error : %s\n", strerror(errno)))
            terminate(file);
            file = NULL;

        } else if (fgetc(file->fp) != '\f' || fgetc(file->fp) != '\n') {
            DD(("tagse: (D) not a tags file\n"))
            terminate(file);
            errno = EINVAL;
            file = NULL;

        } else {
            DD(("tagse: (D) opened\n"))
            rewind(file->fp);
            strcopy(&file->tags, tags, 0);
        }
    }
    return file;
}


static void
terminate(etags *const file)
{
    if (file) {
        assert(TAGSE_MAGIC == file->magic);
        if (file->fp) {
            fclose(file->fp), file->fp = NULL;
        }
        strfree(&file->line);
        strfree(&file->name);
        memset(file, 0, sizeof(etags));
        free(file);
    }
}


static int
readraw(etags *const file)
{
    int reread, result = 0;

    do {
        char *const ch = file->line.buffer + file->line.size - 2;
        char *line;
        off_t pos;

        reread = 0;
        *ch = '\0';

        pos = ftell(file->fp);
        line = fgets(file->line.buffer,(int) file->line.size, file->fp);

        if (line == NULL) {
            /* read error */
            result = -1;

        }  else if (*ch != '\0' && *ch != '\n'  &&  *ch != '\r') {
            /* buffer overflow */
            strgrow(&file->line);
            fseek(file->fp, pos, SEEK_SET);
            reread = 1;                         /* reread line */

        } else {
            /* done */
            size_t i = strlen(file->line.buffer);

            while (i > 0  &&
                    (file->line.buffer[i - 1] == '\n' || file->line.buffer[i - 1] == '\r')) {
                file->line.buffer[i - 1] = '\0';
                --i;
            }
        }
    } while (reread);
    return result;
}


static int
dofile(etags *file)
{
    const char *p = file->line.buffer;

    /*
     *  Format:
     *
     *      [\f] source-file , block-size \n
     */
#define DELIMFILE   '\f'                        /* new page */

    strcopy(&file->file, p, ',');
    DD(("tagse: (D) file=%s <%s>\n", file->file.buffer, p))
    return 0;
}


static int
doentry(etags *file)
{
    char *p = file->line.buffer;
    char *tag, *line, *ch;

    /*
     *  Format:
     *
     *      [\w] full-tag \177 short-tag \001 line-number , character-number \n
     */
#define DELIMTAG    '\177'
#define DELIMNUMBER '\001'
#define DELIMCHAR   ','

    while (*p == ' ' || *p == '\t') {
        ++p;                                    /* eat white space */
    }

    if (*p) {
        if ((tag = strchr(p, DELIMTAG)) != NULL &&
                (line = strchr(tag, DELIMNUMBER)) != NULL &&
                (ch = strchr(line, DELIMNUMBER)) != NULL) {

            *tag++ = '\0';                      /* NUL terminate definition */

            strcopy(&file->name, tag, DELIMNUMBER);
            file->entry.name    = file->name.buffer;
            file->entry.file    = file->file.buffer;
            file->entry.def     = p;
            file->entry.line    = atol(line + 1);
            file->entry.pattern = "";

            DD(("tagse: (D)  %s,%s,%ld\n", file->entry.name, file->entry.file, file->entry.line))
            return 0;
        }
    }

    DD(("tagse: (D) bas entry\n%s", file->line.buffer))
    return -1;
}


static int
tag(etags *const file)
{
    int result;

    while (1) {
        if ((result = readraw(file)) != 0) {
            return result;
        }

        if (file->line.buffer[0] == DELIMFILE) {
            if ((result = readraw(file)) != 0) {
                return result;
            }
            dofile(file);
            continue;
        }

        if (file->file.buffer[0] != '\0') {
            if (0 == (result = doentry(file))) {
                break;
            }
        }
    }
    return result;
}


static int
next(etags *const file, etagEntry *const entry)
{
    int result = -1;

    if (!file || !file->fp) {
        result = -1;
    } else {
        if (0 == (result = tag(file)) && entry) {
            *entry = file->entry;
        }
    }
    return result;
}


static int
compare(etags *const file)
{
    int result;

    if (file->s.ignorecase) {
        if (file->s.partial) {
            result = strnuppercmp(file->s.name, file->name.buffer, file->s.length);
        } else {
            result = struppercmp(file->s.name, file->name.buffer);
        }
    } else {
        if (file->s.partial) {
            result = strncmp(file->s.name, file->name.buffer, file->s.length);
        } else {
            result = strcmp(file->s.name, file->name.buffer);
        }
    }
    return result;
}


static int
search(etags *const file)
{
    while (0 == tag (file)) {
        if (0 == compare(file)) {
            return 0;
        }
    }
    return -1;
}


static int
first(etags *const file, etagEntry *const entry, const char *const name, const int options)
{
    int result;

    file->s.name = name;
    file->s.length = strlen(name);
    file->s.partial = ((options & TAG_FPARTIALMATCH) != 0);
    file->s.ignorecase = ((options & TAG_FIGNORECASE) != 0);

    rewind(file->fp);
    if (0 == (result = search(file)) && entry) {
        *entry = file->entry;
    }
    return result;
}


static int
cont(etags *const file, etagEntry *const entry)
{
    int result;

    if (0 == (result = search(file)) && entry) {
        *entry = file->entry;
    }
    return result;
}



/*
 *  Public interface
 *  ===========================================================================
 */

void *
etagsOpen(const char *const tags)
{
    return (void *)initialize(tags);
}


int
etagsSetSortType(void * const vfile, const int sorted)
{
    etags * const file = vfile;

    __CUNUSED(sorted)
    if (file) {
        assert(TAGSE_MAGIC == file->magic);
    }
    return -1;
}


int
etagsFirst(void * const vfile, etagEntry *const entry)
{
    etags * const file = vfile;
    int result = -1;

    if (file) {
        assert(TAGSE_MAGIC == file->magic);
        if (file->fp) {
            rewind(file->fp);
            result = next(file, entry);
        }
    }
    return result;
}


int
etagsNext(void * const vfile, etagEntry *const entry)
{
    etags * const file = vfile;
    int result = -1;

    if (file) {
        assert(TAGSE_MAGIC == file->magic);
        if (file->fp) {
            result = next(file, entry);
        }
    }
    return result;
}


int
etagsFind(
    void * const vfile, etagEntry *const entry, const char *const name, const int options )
{
    etags * const file = vfile;
    int result = -1;

    if (file) {
        assert(TAGSE_MAGIC == file->magic);
        if (file->fp) {
            result = first(file, entry, name, options);
        }
    }
    return result;
}


int
etagsFindNext(void * const vfile, etagEntry *const entry)
{
    etags * const file = vfile;
    int result = -1;

    if (file) {
        assert(TAGSE_MAGIC == file->magic);
        if (file->fp) {
            result = cont(file, entry);
        }
    }
    return result;
}


int
etagsClose(void * const vfile)
{
    etags * const file = vfile;
    int result = -1;

    if (file) {
        assert(TAGSE_MAGIC == file->magic);
        if (file->fp) {
            terminate(file);
            result = 0;
        }
    }
    return result;
}
/*end*/
