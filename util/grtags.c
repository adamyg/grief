#include <edidentifier.h>
__CIDENT_RCSID(grtags_c,"$Id: grtags.c,v 1.7 2022/05/27 03:33:16 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: grtags.c,v 1.7 2022/05/27 03:33:16 cvsuser Exp $
 * tags command line util.
 *
 *
 */

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <errno.h>

#include "../gr/tagsex.h"
#include "../gr/tagse.h"
#include "../gr/arg.h"

#ifndef FALSE
#define FALSE           0
#define TRUE            1
#endif

static void             usage(void);

static void             tag_find(const char *const name);
static void             tag_list(void);

static void             exprint(const extagEntry *entry);

static const char *     x_progname;

static const char *     o_tags = "grtags";
static int              o_ext;
static int              o_icase;
static int              o_listall;
static int              o_partial;
static int              o_sort = -1;

static struct argoption options[] = {
    { "ext",        arg_none,       NULL,   'e',    "display extension fields in output." },
    { "icase",      arg_none,       NULL,   'i',    "case-insensitive matching." },
    { "list",       arg_none,       NULL,   'l',    "list all tags." },
    { "partial",    arg_none,       NULL,   'p',    "partial matching." },
    { "sort",       arg_optional,   NULL,   's',    "sort order.", "[1|2|3]" },
    { "file",       arg_required,   NULL,   't',    "specified tag file (default: \"tags\").", "file-name" },
    { "debug",      arg_none,       NULL,   'd',    "debugging" },
    { "help",       arg_none,       NULL,   '?',    "usage." },
    { NULL }
    };


int
main(int argc, char **argv)
{
    struct argparms args;
    int c, errflag = 0;
    char *what = NULL;

    x_progname = argv[0];
    arg_initl(&args, argc, (const char * const *)argv, (const char *)-1, options, FALSE);

    while ((c = arg_getopt(&args)) != EOF) {
        switch(c) {
        case 'e':       /* include extension fields */
            ++o_ext;
            break;

        case 'i':       /* case-insensitive matching */
            ++o_icase;
            break;

        case 'l':       /* list all */
            ++o_listall;
            break;

        case 'p':       /* partial matching */
            ++o_partial;
            break;

        case 's':       /* sort */
            if (args.val == NULL) {
                o_sort = 1;
            } else if (strchr("012", args.val[0]) != NULL && args.val[1] == '\0') {
                o_sort = (args.val[0] - '0');
            } else {
                ++errflag;
            }
            break;

        case 't':       /* tag file */
            o_tags = args.val;
            break;

        default:
            ++errflag;
            break;
        }
    }
    argc -= args.ind;
    argv += args.ind;

    /* allow trailing wild-cards */
    if (1 == argc) {
        int l;

        what = argv[0];
        if ((l = strlen(what)) > 0) {
            if (what[l-1] == '*' || what[l-1] == '?' ) {
                if (l == 1) {                   /* special */
                    ++o_listall;
                    --argc;
                } else {                        /* strip & force partial */
                    what = strdup(what);
                    what[l-1] = '\0';
                    ++o_partial;
                }
            }
        }
    }

    if (errflag || (argc != 1 && !o_listall)) {
        usage();
    }

    /* execute */
    if (o_listall) {
        tag_list();

    } else {
        tag_find(what);
    }
    return 0;
}


static void
usage(void)
{
    fprintf(stderr,
        "Find tag file entries matching specified names.\n"
        "\n"
        "Usage: %s [-ilp] [-s[0|1]] [-t file] [name(s)]\n"
        "\n"
        "Options:\n", x_progname);

    arg_print(5, options);

    exit(3);
}


static void
tag_find(const char *const name)
{
    extagFileInfo info;
    extagEntry entry;
    void *file;

    if (NULL == (file = extagsOpen(o_tags, &info))) {
        fprintf(stderr, "%s: %s: %s\n",
            x_progname, strerror (errno), o_tags);
        exit(1);

    } else {
        int opts = 0;

        if (o_sort >= 0) {
            extagsSetSortType(file, o_sort);
        }

        if (o_partial) {
            opts |= TAG_FPARTIALMATCH;
        }

        if (o_icase) {
            opts |= TAG_FIGNORECASE;
        }

        if (extagsFind(file, &entry, name, opts) == TagSuccess) {
            do {
                exprint(&entry);
            } while (extagsFindNext(file, &entry) == TagSuccess);

        } else {
            fprintf (stderr, "%s: no match: %s\n", x_progname, name);
        }

        extagsClose(file);
    }
}


static void
tag_list (void)
{
    extagFileInfo info;
    extagEntry entry;
    void * file;

    if ((file = extagsOpen (o_tags, &info)) == NULL) {
        fprintf (stderr, "%s: %s: %s\n",
            x_progname, strerror (errno), o_tags);
        exit (1);

    } else {
        if (extagsNext (file, &entry) == TagSuccess) {
            do {
                exprint (&entry);
            } while (extagsNext (file, &entry) == TagSuccess);

        } else {
            fprintf (stderr, "%s: no match\n", x_progname);
        }
        extagsClose (file);
    }
}


static void
exprint (const extagEntry *entry)
{
    const char* separator = ";\"";
    const char* const empty = "";
    int i, first = 1;

/* "sep" returns a value only the first time it is evaluated */
#define sep (first ? (first = 0, separator) : empty)

    printf ("%s\t%s\t%s",
        entry->name, entry->file, entry->address.pattern);

    if (o_ext)
    {
        if (entry->kind != NULL  &&  entry->kind [0] != '\0')
            printf ("%s\tkind:%s", sep, entry->kind);
        if (entry->fileScope)
            printf ("%s\tfile:", sep);
        for (i = 0  ;  i < entry->fields.count  ;  ++i)
            printf ("%s\t%s:%s", sep, entry->fields.list [i].key,
                entry->fields.list [i].value);
    }
    putchar ('\n');

#undef sep
}
