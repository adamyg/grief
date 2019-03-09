#include <edidentifier.h>
__CIDENT_RCSID(gr_m_ini_c,"$Id: m_ini.c,v 1.6 2019/01/28 00:24:41 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: m_ini.c,v 1.6 2019/01/28 00:24:41 cvsuser Exp $
 * INI interface.
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

#define  ED_ASSERT
#include <editor.h>
#include <edhandles.h>

#include <stable.h>
#include <iniparser.h>

#include "m_ini.h"                              /* public interface */

#include "accum.h"                              /* acc_...() */
#include "debug.h"                              /* trace_...() */
#include "eval.h"                               /* get_...() */
#include "lisp.h"                               /* atom_...() */
#include "symbol.h"                             /* argv_assign_...() */

typedef struct {
#define IIFILE_MAGIC        MKMAGIC('I','i','F','l')
    MAGIC_t         magic;
    int             ident;
    IFILE          *file;
    IFILECursor    *cursor;
} IIFILE;

static IDENTIFIER_t         x_iniseq;           /* identifier sequence */
static stype_t *            x_inis = NULL;      /* and container */

static IIFILE *             ini_lookup(int objid);
static const char *         section_argument(int argi);


static IIFILE *
ini_lookup(int objid)
{
    sentry_t *st;

    if (x_inis && objid > 0 &&
            NULL != (st = stype_lookup(x_inis, (unsigned) objid))) {
        IIFILE *iifile = (IIFILE  *)st->se_ptr;

        assert(IIFILE_MAGIC == iifile->magic);
        assert(iifile->ident == objid);
        return iifile;
    }
    return NULL;
}


static const char *
section_argument(int argi)
{
    const char *section = get_xstr(argi);
    return (section ? section : "");            /* name otherwise default "" */
}


void
do_iniopen(void)                /* int ([string filename = NULL], [int flags = IFILE_STANDARD]) */
{
    const char *filename = get_xstr(1);
    int flags = get_xinteger(2, IFILE_STANDARD);
    IIFILE *iifile;
    int ret = -1;

    if (NULL == x_inis) {
        x_inis = stype_alloc();
        x_iniseq = GRBASE_INI;
    }

    if (flags & IFILE_STANDARDEOL) flags |= IFILE_STANDARD;
    if (flags & IFILE_EXTENDEDEOL) flags |= IFILE_EXTENDED;

    if (NULL != (iifile = chk_calloc(sizeof(IIFILE), 1))) {
        if (NULL != (iifile->file = IniOpen(filename, flags|IFILE_CREATE))) {
            const int iniseq = x_iniseq++;

            if (stype_insert(x_inis, iniseq, iifile) != -1) {
                iifile->magic = IIFILE_MAGIC;
                iifile->ident = iniseq;
                ret = iniseq;

            } else {
                IniClose(iifile->file);
                chk_free(iifile);
            }

        } else {
            chk_free(iifile);
            ret = -1;
        }
    }
    acc_assign_int(ret);
}


void
do_iniclose(void)               /* int (int ifd) */
{
    const int objid = get_xinteger(1, -1);
    IIFILE *iifile = ini_lookup(objid);         /* objid lookup */
    int ret = -1;

    if (iifile) {
        IniClose(iifile->file);
        stype_delete(x_inis, (unsigned) objid);
        chk_free(iifile);
        ret = 0;
    }
    acc_assign_int(ret);
}


void
do_iniexport(void)              /* int (int ifd, [string filename]) */
{
    const int objid = get_xinteger(1, -1);
    IIFILE *iifile = ini_lookup(objid);         /* objid lookup */
    int ret = -1;

    if (iifile) {
        const char *filename = get_xstr(2);

        ret = (NULL == filename && 0 == IniModified(iifile->file) ? 2 :
                IniExport(iifile->file, (filename ? filename : IniFilename(iifile->file))));
    }
    acc_assign_int(ret);
}


void
do_inifirst(void)               /* int (int ifd, [string &sect], [string &key], [string &val], [string section = NULL]) */
{
    const int objid = get_xinteger(1, -1);
    IIFILE *iifile = ini_lookup(objid);         /* objid lookup */
    int ret = -1;

    if (iifile) {
        IFILECursor *icursor = iifile->cursor;

        ret = 0;
        if (NULL != (icursor = IniFirst(iifile->file, get_xstr(5)))) {
            argv_assign_str(2, (icursor->sect ? icursor->sect : ""));
            argv_assign_str(3, (icursor->key  ? icursor->key  : ""));
            argv_assign_str(4, (icursor->data ? icursor->data : ""));
            ret = 1;
        }
        iifile->cursor = icursor;
    }
    acc_assign_int(ret);
}


void
do_ininext(void)                /* int (int ifd, [string &sect], [string &key], [string &val]) */
{
    const int objid = get_xinteger(1, -1);
    IIFILE *iifile = ini_lookup(objid);         /* objid lookup */
    int ret = -1;

    if (iifile) {
        IFILECursor *icursor = iifile->cursor;

        ret = 0;
        if (icursor) {
            if (NULL != (icursor = IniNext(icursor))) {
                argv_assign_str(2, (icursor->sect ? icursor->sect : ""));
                argv_assign_str(3, (icursor->key  ? icursor->key  : ""));
                argv_assign_str(4, (icursor->data ? icursor->data : ""));
                ret = 1;
            }
            iifile->cursor = icursor;
        }
    }
    acc_assign_int(ret);
}


void
do_iniproperties(void)          /* list (int ifd, [string sect = NULL) */
{
    const int objid = get_xinteger(1, -1);
    IIFILE *iifile = ini_lookup(objid);         /* objid lookup */
    int ret = -1;

    if (iifile) {
        const char *sect = get_xstr(2);
        IFILECursor *icursor = IniFirst(iifile->file, sect);
        unsigned sections = 0, properties = 0;
        const char *section = NULL;

        if (icursor) {
            section = icursor->sect;
            ++sections;
            do {
                if (icursor->sect != section) {
                    section = icursor->sect;
                    ++sections;
                }
                ++properties;
            } while (NULL != (icursor = IniNext(icursor)));
        }

        //TODO
    }
    acc_assign_int(ret);
}


void
do_iniquery(void)               /* string (int ifd, [string sect = ""], string key) */
{
    const int objid = get_xinteger(1, -1);
    IIFILE *iifile = ini_lookup(objid);         /* objid lookup */
    const char *ret = NULL;

    if (iifile) {
        const char *key = get_str(3);

        if (key && *key) {
            ret = IniQuery(iifile->file, section_argument(2), key, NULL);
        }
    }
    acc_assign_str(ret ? ret : "", -1);
}


void
do_inipush(void)                /* int (int ifd, [string sect = ""], [string key = NULL], [string val = ""], [string comment = NULL], [unique = TRUE]) */
{
    const int objid = get_xinteger(1, -1);
    IIFILE *iifile = ini_lookup(objid);         /* objid lookup */
    int ret = -1;

    if (iifile) {
        const char *key = get_xstr(3),
                *val = get_xstr(4),
            *comment = get_xstr(5);

        if ((key && *key) || comment) {         /* property+[comment] or comment */
            ret = IniPush(iifile->file, section_argument(2),
                        key, (val ? val : ""), comment, get_xinteger(6, TRUE));
        }
    }
    acc_assign_int(ret);
}


void
do_iniremove(void)              /* int (int ifd, [string sect = ""], [string key], [int keep = FALSE]) */
{
    const int objid  = get_xinteger(1, -1);
    IIFILE *iifile = ini_lookup(objid);         /* objid lookup */
    int ret = -1;

    if (iifile) {
        ret = IniRemove(iifile->file, section_argument(2), get_xstr(3), get_xinteger(4, FALSE));
    }
    acc_assign_int(ret);
}

/*end*/
