#include <edidentifier.h>
__CIDENT_RCSID(gr_crautoload_c,"$Id: crautoload.c,v 1.14 2020/04/23 12:35:50 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: crautoload.c,v 1.14 2020/04/23 12:35:50 cvsuser Exp $
 * Autoload directive support.
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

#include "grunch.h"                             /* local definitions */

#include <rbtree.h>                             /* red-black tree */
#include <libstr.h>                             /* str_...()/sxprintf() */

typedef TAILQ_HEAD(ModuleList, Module)
                        ModuleList_t;

typedef TAILQ_HEAD(NameList, Name)
                        NameList_t;

typedef RB_HEAD(NameTree, Name) NameTree_t;

typedef struct Module {
    MAGIC_t             ml_magic;
#define MODULE_MAGIC        MKMAGIC('A', 'l', 'M', 'd')
    TAILQ_ENTRY(Module) ml_node;
    unsigned            ml_count;
    time_t              ml_mtime;
    NameList_t          ml_names;
    unsigned            ml_length;
    char                ml_name[1];
} Module_t;

typedef struct Name {
    MAGIC_t             an_magic;
#define NAME_MAGIC          MKMAGIC('A', 'l', 'N', 'm')
    RB_ENTRY(Name)      an_tree;
    TAILQ_ENTRY(Name)   an_list;
    Module_t *          an_owner;
    enum {STATUS_OLD, STATUS_NEW, STATUS_ORG}
                        an_status;
    unsigned            an_length;
    char                an_name[1];
} Name_t;

static int              autoload_import(void);

static Module_t *       module_new(const char *name, const int length, time_t mtime);
static Module_t *       module_find(const char *name, const int length);

static Name_t *         name_new(Module_t *m, const char *name, const int length, int status);
static void             name_delete(Name_t *n);
static Name_t *         name_find(Module_t *m, const char *name, const int length);
static int              name_compare(const struct Name *a, const struct Name *b);

RB_PROTOTYPE(NameTree, Name, an_tree, name_compare);

                                                /* module signature */
static const char       SIGNATURE[] = "// AUTOLOAD";

static ModuleList_t     x_modulelist;           /* autoload definitions */
static NameTree_t       x_nametree;             /* symbol tree */

static const char *     x_autoname;             /* autoload file name */
static FILE *           x_autofp;               /* autoload file stream */
static int              x_autostatus = 0;       /* update status */
static int              x_autoline;

static Module_t *       x_module;               /* current module */

RB_GENERATE(NameTree, Name, an_tree, name_compare);


void
autoload_init(void)
{
    TAILQ_INIT(&x_modulelist);
    RB_INIT(&x_nametree);
}


int
autoload_open(const char *filename)
{
    assert(NULL == x_autoname);
    x_autoname = chk_salloc(filename);

    errno = 0;
    if (NULL != (x_autofp = fopen(filename, "r+"))) {
        if (-1 == autoload_import()) {
            autoload_close();
            return 0;
        }
        x_autostatus = 0;

    } else {
        if (ENOENT != errno ||
                (NULL == (x_autofp = fopen(filename, "w+")))) {
            fprintf(x_errfp, "%s: cannot open '%s' : %s(%d)\n",
                x_progname, filename, strerror(errno), errno);
            autoload_close();
            return -1;

        } else if (fprintf(x_autofp, "%s\n", SIGNATURE) <= 0) {
            fprintf(x_errfp, "%s: cannot update '%s' : %s(%d)\n",
                x_progname, filename, strerror(errno), errno);
            autoload_close();
            unlink(filename);
            return -1;
        }
        x_autostatus = 1;
    }
    return 0;
}


static int
autoload_import(void)
{
    Module_t *module = NULL;
    char line[1024], *name, *end;
    int mtime;

    assert(x_autofp);

    if (!fgets(line, sizeof(line), x_autofp) ||
            0 != strncmp(line, SIGNATURE, sizeof(SIGNATURE)-1)) {
        fprintf(x_errfp, "%s (1): error: autoload SIGNATURE missing\n", x_autoname);
        return -1;
    }

    x_autoline = 2;

    while (fgets(line, sizeof(line), x_autofp)) {
        const char *cursor = line;

        while (' ' == *cursor || '\t' == *cursor) {
            ++cursor;                           /* consume leading white-space */
        }

        if ('/' == cursor[0] && '/' == cursor[1]) {
            cursor += 2;                        /* MTIME:<timestamp> */
            if (NULL != (name = strstr(cursor, "MTIME:"))) {
                if ((mtime = atoi(name + 6)) < 0) {
                    mtime = -1;
                }
            }
            continue;
        }
                                                /* autoload( "<module>" ... */
        if (NULL != (name = strstr(cursor, "autoload("))) {
            if (NULL != (name = strchr(name + 9, '"')) &&
                    NULL != (end = strchr(++name, '"'))) {
                const int namelen = (int)(end - name);

                if (namelen < 0) {
                    fprintf(x_errfp, "%s (%d): error: empty autoload module name.\n",
                        x_autoname, x_autoline);
                } else {
                    module = module_new(name, namelen, (time_t) mtime);
                }
                cursor = end + 1;
            } else {
                fprintf(x_errfp, "%s (%d): error: autoload module name missing.\n",
                    x_autoname, x_autoline);
                return -1;
            }
        }

        if (module && *cursor) {
            do {                                /* "<name>" ... ); */
                if (NULL == (name = strchr(cursor, '"')) ||
                        NULL == (end = strchr(++name, '"'))) {
                    if (strchr(cursor, ')')) {
                        module = NULL;
                    }
                    break;
                } else {
                    const int namelen = (int)(end - name);

                    if (namelen < 0) {
                        fprintf(x_errfp, "%s (%d): error: empty autoload name.\n",
                            x_autoname, x_autoline);
                    } else {
                        name_new(module, name, namelen, STATUS_OLD);
                    }
                }
                cursor = end + 1;
            } while (*cursor);
        }

        ++x_autoline;
    }

    x_autoline = -1;
    return 0;
}


static Module_t *
module_new(const char *name, const int length, time_t mtime)
{
    Module_t *m;

    if (length <= 0 ||
            NULL == (m = (Module_t *)chk_alloc(sizeof(Module_t) + length))) {
        return NULL;
    }

    m->ml_magic  = MODULE_MAGIC;
    m->ml_length = length;
    m->ml_count  = 0;
    m->ml_mtime  = mtime;

    TAILQ_INIT(&m->ml_names);
    memcpy(m->ml_name, name, length);
    m->ml_name[length] = 0;

    TAILQ_INSERT_TAIL(&x_modulelist, m, ml_node);
    return m;
}


static Module_t *
module_find(const char *name, const int length)
{
    ModuleList_t *modulelist = &x_modulelist;
    Module_t *m;

    TAILQ_FOREACH(m, modulelist, ml_node) {
        assert(MODULE_MAGIC == m->ml_magic);
        if ((unsigned)length == m->ml_length &&
                0 == memcmp(name, m->ml_name, length)) {
            return m;
        }
    }
    return NULL;
}


void
autoload_module(const char *source, time_t mtime)
{
    const char *dot;

    if (NULL != x_autofp &&
            NULL != (dot = strchr(source, '.'))) {
        const int length = (int)(dot - source);

        if (NULL == (x_module = module_find(source, length))) {
            x_module = module_new(source, length, mtime);
        } else if (mtime > 0) {
            x_module->ml_mtime = mtime;
        }
    }
}


void
autoload_push(const char *funcname)
{
    if (x_module) {
        const int length = strlen(funcname);
        Name_t *n;

        if (NULL != (n = name_find(x_module, funcname, length))) {
            n->an_status = STATUS_ORG;
        } else {
            name_new(x_module, funcname, length, STATUS_NEW);
        }
    }
}

        
int
autoload_export(int commit)
{
    const time_t now = time(NULL);
    NameList_t *list = &x_module->ml_names;
    int modifications = 0;
    Name_t *n;

    if (NULL == x_autofp || NULL == x_module) {
        return 0;
    }

    if (NULL != (n = TAILQ_FIRST(list))) {
        Name_t *nnext;

        do {
            nnext = TAILQ_NEXT(n, an_list);
            if (commit) {                   /* commit changes */
                switch (n->an_status) {
                case STATUS_OLD:
                    name_delete(n);
                    ++modifications;
                    continue;
                case STATUS_NEW:
                    ++modifications;
                    break;
                default:
                    break;
                }
            } else {                        /* todo modifiations */
                switch (n->an_status) {
                case STATUS_NEW:
                    name_delete(n);
                    continue;
                default:
                    break;
                }
            }
            n->an_status = STATUS_OLD;
        } while (NULL != (n = nnext));
    }

    if (commit && xf_verbose >= 2) {
        if (0 == modifications) {
            printf("%s: autoload, no change\n", x_module->ml_name);
        } else {
            printf("%s: autoload rebuilding\n", x_module->ml_name);
        }
    }

    if (modifications) {
        if (0 == x_autostatus++) {              /* rebuild, save backup image */
            size_t namelen = strlen(x_autoname);
            char *savname;

            fclose(x_autofp), x_autofp = NULL;

            if (NULL != (savname = chk_alloc(namelen += 6))) {
                sxprintf(savname, namelen, "%s.sav", x_autoname);
                (void) unlink(savname);
                (void) rename(x_autoname, savname);
                chk_free(savname);
            }

            if (NULL == savname || (NULL == (x_autofp = fopen(x_autoname, "w+")))) {
                fprintf(x_errfp, "%s: cannot update '%s' : %s(%d)\n",
                    x_progname, x_autoname, strerror(errno), errno);
                autoload_close();
                return -1;
            }
        } else {                                /* update */
            if (0 != fseek(x_autofp, 0, SEEK_SET) &&
                    0 != ftruncate(fileno(x_autofp), 0)) {
                fprintf(x_errfp, "%s: cannot update '%s' : %s(%d)\n",
                    x_progname, x_autoname, strerror(errno), errno);
                autoload_close();
                return -1;
            }
        }
    }

    if (modifications) {
        char buf[1024];
        ModuleList_t *modulelist = &x_modulelist;
        Module_t *m;

        errno = 0;
        sxprintf(buf, sizeof(buf),
            "%s - autogenerated on %.20s\n"
            "\n",
            SIGNATURE, ctime(&now) + 4);
        fputs(buf, x_autofp);

        fputs("void\nmain(void)\n{\n", x_autofp);
        TAILQ_FOREACH(m, modulelist, ml_node) {
            NameList_t *mlist = &m->ml_names;
            Name_t *mn;

            assert(MODULE_MAGIC == m->ml_magic);
            if (m->ml_count) {
                if (m->ml_mtime > 0) {
                    fprintf(x_autofp,
                        "    //MTIME:%d (%.20s)\n", (int) m->ml_mtime, ctime(&m->ml_mtime) + 4);
                }
                fprintf(x_autofp,
                        "    autoload(\"%s\"", m->ml_name);
                TAILQ_FOREACH(mn, mlist, an_list) {
                    assert(NAME_MAGIC == mn->an_magic);
                    fprintf(x_autofp,
                        ",\n        \"%s\"", mn->an_name);
                }
                fputs(");\n", x_autofp);
            }
        }
        fputs("}\n", x_autofp);

        if (errno) {
            fprintf(x_errfp, "%s: cannot update '%s' : %s(%d)\n",
                x_progname, x_autoname, strerror(errno), errno);
        }
    }
    return 0;
}


void
autoload_close(void)
{
    if (x_autofp) {
        fclose(x_autofp);
        x_autofp = NULL;
    }
    chk_free((void *)x_autoname);
    x_autoname = NULL;
}


static Name_t *
name_new(Module_t *m, const char *name, const int length, int status)
{
    NameTree_t *tree = &x_nametree;
    Name_t *n, *t_n;

    if (length <= 0 ||
            NULL == (n = (Name_t *)chk_alloc(sizeof(Name_t) + length))) {
        return NULL;
    }

    n->an_magic  = NAME_MAGIC;
    n->an_owner  = m;
    n->an_length = length;
    memcpy(n->an_name, name, length);
    n->an_name[length] = 0;
    n->an_status = status;

    TAILQ_INSERT_TAIL(&m->ml_names, n, an_list);
    if (NULL != (t_n = RB_INSERT(NameTree, tree, n))) {
        if (x_autoline >= 1) {
            fprintf(x_errfp, "%s (%d): autoload redefinition '%s', previous within module '%s'\n",
                x_autoname, x_autoline, n->an_name, t_n->an_owner->ml_name);
        } else {
            yywarningf("autoload redefinition '%s', previous within module '%s'",
                n->an_name, t_n->an_owner->ml_name);
        }
        name_delete(t_n);
        RB_INSERT(NameTree, tree, n);
    }
    ++m->ml_count;
    return n;
}


static void
name_delete(Name_t *n)
{
    Module_t *m;

    if (NULL != (m = n->an_owner)) {
        NameTree_t *tree = &x_nametree;
        NameList_t *list = &m->ml_names;

        assert(NAME_MAGIC == n->an_magic);
        RB_REMOVE(NameTree, tree, n);
        TAILQ_REMOVE(list, n, an_list);
        --m->ml_count;
    }
    chk_free(n);
}


static Name_t *
name_find(Module_t *m, const char *name, const int length)
{
    if (m) {
        NameList_t *list = &m->ml_names;
        Name_t *n;

        TAILQ_FOREACH(n, list, an_list) {
            assert(NAME_MAGIC == n->an_magic);
            if ((unsigned)length == n->an_length &&
                    0 == memcmp(name, n->an_name, length)) {
                return n;
            }
        }
    }
    return NULL;
}


static int
name_compare(const struct Name *a, const struct Name *b)
{
    return strcmp(a->an_name, b->an_name);
}

/*end*/
