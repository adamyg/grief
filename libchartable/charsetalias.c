#include <edidentifier.h>
__CIDENT_RCSID(gr_charsetalias_c,"$Id: charsetalias.c,v 1.12 2018/10/01 22:10:52 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* Locale/multibyte character information.
 *
 *
 * Copyright (c) 2010 - 2018, Adam Young.
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include <unistd.h>

#include <libstr.h>                             /* str_...()/sxprintf() */
#include "libchartable.h"

#if defined(DO_DEBUG)
#define __DEBUG(x)          printf x;
#else
#define __DEBUG(x)
#endif


/*
 *  Character encoding --- common character sets.
 *
 *      References:
 *          UNICODE/ICU/IANA/GLIB
 *          http://en.wikipedia.org/wiki/Character_encoding
 *          http://en.wikipedia.org/wiki/ISO_8859
 */

struct charset {
    MAGIC_t                 cs_magic;
#ifndef MKMAGIC
#define MKMAGIC(a, b, c,d) \
              (((unsigned)(a))<<24|((unsigned)(b))<<16|(c)<<8|(d))
#endif
#define CS_MAGIC                MKMAGIC('C','h','S','t')
    const char *            cs_name;
    unsigned                cs_namelen;
    char *                  cs_aliases;
    unsigned                cs_alias_len;
    unsigned                cs_alias_alloced;
};

static int                  charset_load(struct charsetmap *map, int mode, const char *aliasset);

static struct charset *     charset_get(struct charsetmap *map, const char *name, int namelen);
static struct charset *     charset_map(struct charsetmap *map, const char *name, int namelen);
static struct charset *     charset_new(struct charsetmap *map, const char *name, int namelen);

static const char *         charset_lookup(struct charsetmap *map, const char *name, int namelen);
static void                 charset_dump(struct charsetmap *map);

static const char *         skipwhite(const char *cursor);
static int                  iswhite(int c);

static int                  alias_push(struct charset *map, const char *name);
static const char *         alias_map(const char *name, char *buffer, int bufsiz);
static int                  alias_compare(const char *primary, const char *name, int namelen);

static struct charsetmap    x_charsetmap;


void
charset_alias_init(void)
{
    charset_alias_open(CHARSET_MODE_X11, -1, NULL, "charset.alias");
}


void
charset_alias_shutdown(void)
{
    /*TODO*/
}


int
charset_alias_open(int mode, int paths, const char **dirs, const char *aliasset)
{
    const char *t_dirs[6] = {0};
    int ret = -1;

    if (-1 == paths) {
        paths = 0;
        if (NULL != (t_dirs[paths] = getenv("CHARSETALIASDIR"))) ++paths;
        if (NULL != (t_dirs[paths] = getenv("GRPATH"))) ++paths;
        if (NULL != (t_dirs[paths] = getenv("GRHELP"))) ++paths;
#if defined(unix) || defined(__APPLE__)
        t_dirs[paths++] = "/usr/lib";
#endif
        dirs = t_dirs;
    }

    if (paths > 0 && dirs && aliasset) {
        const int aliaslen = strlen(aliasset);
        char *path = NULL;
        int pathlen = 0, p;

        for (p = 0; -1 == ret && (paths >= 0 && p < paths) || (paths < 0 && dirs[p]); ++p) {
            const char *dir;

            if (NULL != (dir = dirs[p]) && ('/' == *dir || '\\' == *dir)) {
                const int dirlen = strlen(dir);
                int t_pathlen = dirlen + aliaslen + 2;

                if ((path && t_pathlen <= pathlen) ||
                        (free((void *)path), NULL != (path = malloc(pathlen = t_pathlen)))) {

                    memcpy(path, dir, dirlen);
                    path[dirlen] = *dir;
                    memcpy(path + dirlen + 1, aliasset, aliaslen + 1);

                    __DEBUG(("alias: open '%s'\n", path))
                    if (0 == access(path, R_OK)) {
                        ret = charset_alias_load(mode, path);
                    }
                }
            }
        }
        free((void *)path);
    }
    return ret;
}


/*  Function:               charset_alias_load
 *      Character-set alias loader.
 *
 *  Parameters:
 *      maps -                  Primary maps base address.
 *      count -                 Map count.
 *      mode -                  Specification mode.
 *
 *  Modes:
 *      The mode specifies the definition format, the following are currently supported.
 *
 *          CHARSET_MODE_BASIC/1
 *              Basic list of supported character-set, without aliases.
 *              Generally sourced from 'locale -m'.
 *
 *>                 <character-set>
 *
 *          CHARSET_MODE_INI/2
 *              Labelled/ini style
 *
 *>                 :<primary>  or [<primary>]
 *>                 desc[ription]=<text>
 *>                 [aliases]=<secondary> ...
 *>                     :
 *
 *          CHARSET_MODE_X11/3
 *              X11/glib charset.alias
 *
 *>                 <primary>[whitespace]<secondary> ....
 *
 *          CHARSET_MODE_ICONV/4
 *              iconv
 *
 *>                 <secondary>[whitespace]<primary>
 *
 *          CHARSET_MODE_MOZILLA/5
 *              Mozilla charsetalias.properties
 *
 *>                 <secondary>=<primary>
 *
 *          CHARSET_MODE_IANA/6
 *
 *>                 Name:   <name>
 *>                 Alias:  <alias>     [(perferred MIME name)]
 *
 *  Returns:
 *      Load aliases count, otherwise -1 on error.
 */
int
charset_alias_load(int mode, const char *aliasset)
{
    if (aliasset) {
        return charset_load(&x_charsetmap, mode, aliasset);
    }
    return -1;
}


const char *
charset_alias_lookup(const char *name, int namelen)
{
    if (name) {
        return charset_lookup(&x_charsetmap, name, (namelen > 0 ? namelen : strlen(name)));
    }
    return NULL;
}


/*  Functions:              charset_alias_dump
 *      Dump the current character-set alias table.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing.
 */
void
charset_alias_dump(void)
{
    charset_dump(&x_charsetmap);
}


static int
charset_load(struct charsetmap *map, int mode, const char *aliasset)
{
    FILE *file = NULL;
    struct charset *cs = NULL;
    char line[1025], name[65], name2[65];
    int result = 0;

    __DEBUG(("alias: loading '%s'\n", aliasset))

    if (NULL == (file = fopen(aliasset, "r"))) {
        perror(aliasset);
        return -1;
    }

    while (! feof(file)) {
        const char *cursor = line;

        if (NULL == fgets(line, sizeof(line), file)) {
            if (ferror(file)) {
                perror(aliasset);
            }
            break;
        }

        cursor = skipwhite(cursor);
        if (0 == *cursor) {
            cs = NULL;
            continue;
        }

        switch (*cursor) {
        case '#':               /* comments */
        case ';':
        case '/':
            break;

        case ':':   case '[':   /* section */
            if (CHARSET_MODE_INI == mode) {     /* :primary or [primary] */
                const char delimiter = *cursor;
                char name[201];

                cs = NULL;
                if (1 == sscanf(cursor + 1, "%64[^ \t\n\r]", name)) {
                    if (name[0]) {
                        if ('[' == delimiter) {
                            char *end = strchr(name, ']');
                            if (*end) *end = 0;
                        }
                        cs = charset_get(map, name, -1);
                    }
                }
            }
            break;

        default:  {             /* alias or alias-list */
                char *comment;
                                                /* name */
                if (1 != sscanf(cursor, "%64[^ \t\r\n]", name) ||
                        0 == name[0] || !isalpha(name[0])) {
                    break;
                }
                                                /* remove trailing comments */
                if (NULL != (comment = strchr(name, '#')) && iswhite(comment[-1])) {
                    while (comment > name && iswhite(comment[-1])) {
                        --comment;
                    }
                    *comment = 0;
                }

                switch (mode)  {
                case CHARSET_MODE_BASIC:        /* <primary> */
                    charset_get(map, name, -1);
                    break;

                case CHARSET_MODE_INI:          /* <tag>=<name> */
                    if (cs) {
                        if (0 == strncmp("aliases=", cursor, 6) || ('=' == *cursor)) {
                            int namelen = ('=' == *cursor ? 1 : 6);
                            for (;;) {
                                cursor = skipwhite(cursor + namelen);
                                if (1 != sscanf(cursor, "%64[^ \t\r\n]", name) ||
                                        0 == (namelen = strlen(name))) {
                                    break;
                                }
                                if (alias_push(cs, name)) {
                                    ++result;
                                }
                            }
                        }
                    }
                    break;

                case CHARSET_MODE_X11: {        /* <primary> <secondary> .... */
                        int namelen = strlen(name);

                        if (NULL != (cs = charset_get(map, name, namelen))) {
                            for (;;) {
                                cursor = skipwhite(cursor + namelen);
                                if (1 != sscanf(cursor, "%64[^ \t\r\n]", name) ||
                                        0 == (namelen = strlen(name))) {
                                    break;
                                }
                                if (alias_push(cs, name)) {
                                    ++result;
                                }
                            }
                            cs = NULL;
                        }
                    }
                    break;

                case CHARSET_MODE_ICONV: {      /* <secondary> <primary> */
                        cursor = skipwhite(cursor + strlen(name));
                        if (1 == sscanf(cursor, "%64[^ \t\r\n]", name2) &&
                                        name2[0] && NULL == strchr(name2, '.')) {
                            if (NULL != (cs = charset_get(map, name2, -1))) {
                                if (alias_push(cs, name)) {
                                    ++result;
                                }
                                cs = NULL;
                            }
                        }
                    }
                    break;

                case CHARSET_MODE_MOZILLA: {    /* <secondary>=<primary> */
                        char *eq;
                                                /* ignore private/mime, x- */
                        if (NULL != (eq = strchr(name, '='))) {
                            if ('x' != eq[1] && '-' != eq[2])
                                if (NULL != (cs = charset_get(map, eq + 1, -1))) {
                                    eq[0] = 0;
                                    if (alias_push(cs, name)) {
                                        ++result;
                                    }
                                    cs = NULL;
                                }
                        }
                    }
                    break;

                case CHARSET_MODE_IANA: {       /* Name: or Alias: */
                        if (0 == strncmp("Name:", cursor, 5)) {
                            cs = NULL;
                            cursor = skipwhite(cursor + 5);
                            if (1 == sscanf(cursor, "%64[^ \t\r\n]", name) && name[0] &&
                                        NULL == strchr(name, '.')) {
                                const char *colon;

                                if (NULL != (colon = strchr(name, ':'))) {
                                    cs = charset_get(map, name, colon - name);
                                    if (cs && alias_push(cs, name)) {
                                        ++result;
                                    }
__DEBUG(("alias: iana,  CS= '%.*s'\n", colon - name, name))
__DEBUG(("alias: iana, MAP= '%s'\n", name))
                                } else {
                                    cs = charset_get(map, name, -1);
__DEBUG(("alias: iana,  cs= '%s'\n", name))
                                }
                            }

                        } else if (cs && 0 == strncmp("Alias:", cursor, 6)) {
                            cursor = skipwhite(cursor + 6);
                            if (1 == sscanf(cursor, "%64[^ \t\r\n]", name) && name[0] &&
                                        str_icmp("none", name)) {
                                if (alias_push(cs, name)) {
                                    ++result;
                                }
__DEBUG(("alias: iana, map= '%s'\n", name))
                            }
                        }
                    }
                    break;
                }
            }
            break;
        }
    }
    fclose(file);
    return result;
}


static struct charset *
charset_get(struct charsetmap *map, const char *name, int namelen)
{
    char canonicalize[CS_NAMELEN+1];
    struct charset *cs;

    if (namelen < 0) {
        namelen = strlen(name);
    }

    if (NULL == (cs = charset_map(map, name, namelen))) {
        /*
         *  normalise name/
         *      some alias tables (for example sun/iconv) use short forms of several codsets,
         *      which shall cause inconsistent lookups, canonicalize prior.
         */
        if (NULL == charset_canonicalize(name, namelen, canonicalize, sizeof(canonicalize))) {
            cs = charset_new(map, name, namelen);

        } else {
            const int canonicalizelen = strlen(canonicalize);

            __DEBUG(("alias: charset canonicalize '%s' ==> '%s'\n", name, canonicalize))
            if (NULL == (cs = charset_map(map, canonicalize, canonicalizelen))) {
                cs = charset_new(map, canonicalize, canonicalizelen);
            }
        }
    }
    __DEBUG(("alias: charset '%s' ==> %p\n", cs->cs_name, cs))
    return cs;
}


static struct charset *
charset_map(struct charsetmap *map, const char *name, int namelen)
{
    if (name && namelen > 0) {
        const int count = map->cs_count;

        if (count) {
            struct charset *cs = map->cs_table;
            int idx;

            for (idx = 0; idx < count; ++cs, ++idx) {
                assert(CS_MAGIC == cs->cs_magic);
                if (0 == charset_compare(cs->cs_name, name, namelen)) {
                    return cs;
                }
            }
        }
    }
    return NULL;
}


static struct charset *
charset_new(struct charsetmap *map, const char *name, int namelen)
{
    struct charset *cs = NULL;

    if (name && namelen > 0) {
        struct charset *table;
        char *nname;

        __DEBUG(("alias: new charset '%*s'\n", namelen, name))
        if (NULL == (table = map->cs_table) || (map->cs_count + 1) >= map->cs_alloced) {
            if (NULL == (table = realloc(table, sizeof(struct charset) * (map->cs_alloced + 64)))) {
                return NULL;
            }
            map->cs_alloced += 64;
            map->cs_table = table;
            __DEBUG(("alias: table expanded +64 %d ==> %p\n", map->cs_alloced, table))
        }

        cs = table + map->cs_count++;
        memset(cs, 0, sizeof(struct charset));
        cs->cs_magic = CS_MAGIC;
        cs->cs_name = nname = malloc(namelen + 1);
        cs->cs_namelen = namelen;
        memcpy(nname, name, namelen);
        nname[namelen] = 0;
        return cs;
    }
    return NULL;
}


static const char *
charset_lookup(struct charsetmap *map, const char *name, int namelen)
{
    const int count = map->cs_count;
    
    if (count) {
        struct charset *cs = map->cs_table;
        int idx;

        for (idx = 0; idx < count; ++cs, ++idx) {
            assert(CS_MAGIC == cs->cs_magic);
            
            if (0 == charset_compare(cs->cs_name, name, namelen)) {
                return cs->cs_name;
            }

            if (cs->cs_aliases) {
                const char *aliases = cs->cs_aliases;
                size_t length;

                while (aliases[0] && (length = strlen(aliases)) > 0) {
                    if (0 == alias_compare(aliases, name, namelen)) {
                        return cs->cs_name;
                    }
                    aliases += length + 1;
                }
            }
        }
    }
    return NULL;
}


static void
charset_dump(struct charsetmap *map)
{
    const int count = map->cs_count;

    printf("<CHARSET DUMP=%d>", count);
    if (count) {
        struct charset *cs = map->cs_table;
        int idx;

        for (idx = 0; idx < count; ++cs, ++idx) {
            assert(CS_MAGIC == cs->cs_magic);
            
            printf("\n[%s]\n", cs->cs_name);
            
            if (cs->cs_aliases) {
                const char *aliases = cs->cs_aliases;
                size_t length;

                while (aliases[0] && (length = strlen(aliases)) > 0) {
                    printf("=%s\n", aliases);
                    aliases += length + 1;
                }
            }
        }
    }
    printf("\n<END>\n");
}


static const char *
skipwhite(const char *cursor)
{
    while (iswhite(*cursor)) {
        ++cursor;
    }
    return cursor;
}


static int
iswhite(const int c)
{
    return (' ' == c || '\t' == c || '\r' == c || '\n' == c);
}


static int
alias_push(struct charset *cs, const char *name)
{
    char *t_aliases, t_name[CS_NAMELEN+1];
    int namelen = 0;

    assert(CS_MAGIC == cs->cs_magic);

    /* copy and remove escape sequences */
    while (*name && namelen < CS_NAMELEN) {
        if ('\\' == (t_name[namelen] = *name++)) {
            if (*name) {                        /* consume escaped characters */
                t_name[namelen] = *name++;
            }
        }
        ++namelen;
    }
    t_name[namelen] = 0;
    name = t_name;

    /* unique check */
    if (0 == charset_compare(cs->cs_name, name, namelen)) {
        __DEBUG(("alias: equ alias %s ==> %*s\n", cs->cs_name, namelen, name))
        return 0;
    }

    if (cs->cs_aliases) {
        const char *aliases = cs->cs_aliases;
        int length;

        while (aliases[0] && (length = strlen(aliases)) > 0) {
            if (0 == alias_compare(aliases, name, namelen)) {
                __DEBUG(("alias: dup alias %s ==> %*s\n", cs->cs_name, namelen, name))
                return 0;
            }
            aliases += length + 1;
        }
    }

    __DEBUG(("alias: new alias %s ==> %*s\n", cs->cs_name, namelen, name))

    /* push */
    ++namelen;                                  /* NUL */

    if (NULL == (t_aliases = cs->cs_aliases) ||
            (cs->cs_alias_len + namelen + 1) >= cs->cs_alias_alloced) {
        if (NULL == (t_aliases =
                realloc(cs->cs_aliases, cs->cs_alias_alloced + 128 + 1))) {
            return 0;
        }
        cs->cs_alias_alloced += 128;
        cs->cs_aliases = t_aliases;
        __DEBUG(("alias: alias table expanded %d ==> %p\n", cs->cs_alias_alloced, cs->cs_aliases))
    }
    memcpy(cs->cs_aliases + cs->cs_alias_len, name, namelen);
    cs->cs_alias_len += namelen;
    cs->cs_aliases[cs->cs_alias_len] = 0;
    cs->cs_aliases[cs->cs_alias_len+1] = 0;
    return 1;
}


/*  Function:               alias_compare
 *      Compare two character-set aliases, following glib with a case sensitive match.
 *
 *  Parameters:
 *      primary -               Primary character set name.
 *      name -                  Name against which to comare.
 *      namelen -               Name length in bytes, -1 length is derived.
 *
 *  Returns:
 *      0 if matched, otherwise a 1.
 */
static int
alias_compare(const char *primary, const char *name, int namelen)
{
    return charset_compare(primary, name, namelen);
}

/*end*/
