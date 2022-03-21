#ifndef GR_INIPARSER_H_INCLUDED
#define GR_INIPARSER_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_iniparser_h,"$Id: iniparser.h,v 1.8 2022/03/21 14:55:28 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: iniparser.h,v 1.8 2022/03/21 14:55:28 cvsuser Exp $
 * INI parser.
 *
 *
 *
 * Copyright (c) 1998 - 2022, Adam Young.
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

#include <edsym.h>
#include <stdio.h>

#include <tailqueue.h>
#include <rbtree.h>

__CBEGIN_DECLS

struct IFILE;
struct _iniSection;

typedef RB_HEAD(_iniSectionTree, _iniSection)
                        ISectionTree_t;         /* section tree. */

typedef TAILQ_HEAD(_iniSectionList, _iniSection)
                        ISectionList_t;         /* section list. */

typedef struct IFILECURSOR {
#define IFILECURSOR_MAGIC   MKMAGIC('I','f','i','C')
    MAGIC_t             magic;
    const char *        sect;
    const char *        key;
    const char *        data;

#if defined(__IFILE_INTERNAL__)
#define IFILE_IFILE         0
#define IFILE_ISECT         1
#define IFILE_IPROP         2
#endif
    void *              __w[3];
} IFILECursor;

typedef void (*IniReporter_t)(struct IFILE *ifile, void *udata, unsigned line, const char *msg);

typedef struct IFILE {
    unsigned            i_magic;
#define IFILE_MAGIC         MKMAGIC('I','f','i','L')

/*--export--defines--*/
/*
 *  iniopen() flags
 */
#define IFILE_STANDARD          0x0001          /* standard comment syntax (;). */
#define IFILE_STANDARDEOL       0x0002          /* ; end-of-ine comments. */
#define IFILE_EXTENDED          0x0004          /* extended syntax (#) comments. */
#define IFILE_EXTENDEDEOL       0x0008          /* ## end-of-line comments. */
#define IFILE_COMMENTSEOL      (IFILE_STANDARDEOL|IFILE_EXTENDEDEOL)
                                                /* EOL comments */

#define IFILE_DUPLICATES        0x0010          /* allow duplicate sections. */
#define IFILE_COLON             0x0020          /* colon (:) as key/value delimiter, otherwise equal (=). */
                                                /* colon (:) and equal (=) as key/value delimiter. */
#define IFILE_EQUALCOLON       (0x0040|IFILE_COLON)
#define IFILE_BACKSLASH         0x0080          /* backslash quoting. */

#define IFILE_QUOTED            0x0100          /* quoted strings. */
#define IFILE_QUOTES            0x0200          /* preserve quotes. */
#define IFILE_CREATE            0x0400          /* create a new image. */
#define IFILE_COMMENTS          0x0800          /* preserve comments. */
/*--end--*/

#define IFILE_EOF               0x1000

    unsigned            i_flags;

    const char *        i_fname;
    FILE *              i_file;
    unsigned            i_line;

    IniReporter_t       i_report;
    void *              i_udata;
    int                 i_errors;
    int                 i_modifications;

#if defined(__IFILE_INTERNAL__)
#define IFILE_LINEMAX		(8 * 1028)
#define IFILE_SECTMAX		(128)
#endif

    ISectionList_t      i_sections;             /* sections */
    ISectionTree_t      i_lookup;               /* unique section tree */
    struct _iniSection *i_root;                 /* root section */
    char *              i_buffer;               /* working line buffer */
    IFILECursor         i_cursor;               /* cursor */

    int              (* i_getc)(struct IFILE *);/* get character implementation */
    int                 i_unget;                /* unget buffer */
} IFILE;

extern IFILE *          IniOpen(const char *filename, unsigned flags);
extern IFILE *          IniOpenx(const char *filename, unsigned flags, IniReporter_t reporter, void *udata);
extern const char *     IniFilename(IFILE *ifile);
extern int              IniErrors(IFILE *ifile);
extern int              IniModified(IFILE *ifile);

extern int              IniExport(IFILE *ifile, const char *filename);
extern void             IniClose(IFILE *ifile);

extern IFILECursor *    IniFirst(IFILE *ifile, const char *section);
extern IFILECursor *    IniNext(IFILECursor *ifile);

extern const char *     IniQuery(IFILE *ifile, const char *section, const char *key, const void *prev);
extern int              IniPush(IFILE *ifile, const char *section, const char *key,
                            const char *value, const char *comment, int flags);
extern int              IniRemove(IFILE *ifile, const char *section, const char *key, int keep);

__CEND_DECLS

#endif /*GR_INIPARSER_H_INCLUDED*/
