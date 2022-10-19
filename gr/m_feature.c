#include <edidentifier.h>
__CIDENT_RCSID(gr_m_feature_c,"$Id: m_feature.c,v 1.28 2022/08/10 15:44:57 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: m_feature.c,v 1.28 2022/08/10 15:44:57 cvsuser Exp $
 * Features.
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

#if defined(HAVE_ARCHIVE_H) && defined(HAVE_LIBARCHIVE)
#if defined(GNUWIN32_LIBARCHIVE)
#include <gnuwin32_archive.h>
#include <gnuwin32_archive_entry.h>
#else
#include <archive.h>
#include <archive_entry.h>
#endif
#endif

#include "m_feature.h"

#ifndef STRINGIZE
#define _STRINGIZE(_s2)         #_s2
#define STRINGIZE(_s1)          _STRINGIZE(_s1)
#endif

#if defined(USE_VIO_BUFFER)
#elif defined(HAVE_LIBNCURSES)
#if defined(HAVE_NCURSES_CURSES_H)
#include <ncurses/curses.h>
#elif defined(HAVE_NCURSES_H)
#include <ncurses.h>
#elif defined(HAVE_CURSES_H)
#include <curses.h>
#endif
#elif defined(HAVE_LIBNCURSESW)
#if defined(HAVE_NCURSESW_CURSES_H)
#include <ncursesw/curses.h>
#elif defined(HAVE_NCURSESW_H)
#include <ncursesw.h>
#elif defined(HAVE_CURSES_H)
#include <curses.h>
#endif
#elif defined(HAVE_LIBCURSES)
#if defined(HAVE_CURSES_H)
#include <curses.h>
#if defined(HAVE_TERM_H)
#include <term.h>
#endif
#endif
#endif

#if defined(HAVE_LIBICU)
#include <unicode/uversion.h>
#endif

#include <edbuildinfo.h>

#include "accum.h"
#include "eval.h"
#include "lisp.h"
#include "main.h"
#include "regprog.h"
#include "wild.h"

struct ft_map {
    const char *        name;                   /* Feature name */
    unsigned            type;                   /* Value type */
#define FT_STR                  1   /* String */
#define FT_INT                  2   /* Numeric */
#define FT_FLG                  3   /* Flag/boolean */
    void *              ptr;                    /* Storage pointer */
    const char *        desc;                   /* Description */
};

int                     xf_fsync = FALSE;       //TODO
int                     x_history_depth = -1;   //TODO

#if (TODO)
static struct ft_map    ft_features[] = {
#define FT_MKSTR(__x)           FT_STR, (void *)&__x
#define FT_MKINT(__x)           FT_INT, (void *)&__x
#define FT_MKFLG(__x)           FT_FLG, (void *)&__x

//  { "autoindent",         FT_MKFLG(xf_autoindent),        "" },
    { "autosave",           FT_MKFLG(xf_autosave),          "" },
//  { "autoread",           FT_MKFLG(xf_autoread),          "" },
//  { "autowrite",          FT_MKFLG(xf_autowrite),         "" },
    { "backups",            FT_MKFLG(xf_backups),           "" },
    { "buftype",            FT_MKSTR(x_bftype_default),     "" },
    { "encodingdefault",    FT_MKSTR(x_encoding_default),   "" },
    { "encodingguess",      FT_MKSTR(x_encoding_guess),     "" },
    { "fsync",              FT_MKFLG(xf_fsync),             "" },
    { "hilite",             FT_MKFLG(xf_syntax_flags),      "" },
    { "history",            FT_MKINT(x_history_depth),      "command-line history depth" },
    { "lazyvt",             FT_MKFLG(xf_lazyvt),            "" },
    { "profile",            FT_MKFLG(xf_profile),           "" },
    { "readonly",           FT_MKFLG(xf_readonly),          "" },
    { "spell",              FT_MKFLG(xf_spell),             "" },
    { "visbell",            FT_MKFLG(xf_visbell),           "visible bell" },
    { "wait",               FT_MKFLG(xf_wait),              "" },
    { "warnings",           FT_MKFLG(xf_warnings),          "" },
//  { "escdelay",           FT_MKFLG(xf_escdelay),          "" },
//  { "backspace",          FT_MKFLG(xf_backspace),         "how backspace works at start of line" },
//  { "confirm",            FT_MKFLG(xf_confirm),           "ask what to do about unsaved/read-only files" },
//  { "swapfile",           FT_MKFLG(xf_swapfile),          "whether to use a swapfile for a buffer" },
//  { "swapsync",           FT_MKFLG(xf_swapsync,           "how to sync the swap file" },
//  { "terse",              FT_MKFLG(xf_terse),             "shorten some messages" },
//  { "verbose",            FT_MKFLG(xf_verbose),           "give informative messages" },

#undef  FT_MKSTR
#undef  FT_MKINT
#undef  FT_MKFLG
    };
#endif  /*TODO*/


/*
 *  Build features
 */
const char * const      x_features[] = {
        /* compiler */
#if defined(__WATCOMC__)
        "+wcc" STRINGIZE(__WATCOMC__)
#elif defined(_MSC_VER)
        "+msc" STRINGIZE(_MSC_VER)
#elif defined(__GNUC__)
        "+gcc" STRINGIZE(__GNUC__) "." STRINGIZE(__GNUC_MINOR__) "." STRINGIZE(__GNUC_PATCHLEVEL__)
#if defined(__CYGWIN__)
            "-cygwin"
#endif
#else
        "+cc"
#endif
#if defined(__STDC_VERSION__)
            "-stdc" STRINGIZE(__STDC_VERSION__)
#endif
        ,

#if !defined(NDEBUG)
        "+assert",
#endif

#if defined(BUILD_TYPE_RELEASE)
        "+release",                             /*release*/
#elif defined(BUILD_TYPE_DEBUG)
        "+debug",                               /*debug*/
#else
#error edbuildinfo.h: unknown BUILD_TYPE
#endif

        /* video/terminal implementation */
#if defined(USE_VIO_BUFFER)
        "+libvio",
#elif defined(HAVE_LIBNCURSES)
#if defined(NCURSES_VERSION)
        "+libncurses (" NCURSES_VERSION ")",
#elif defined(NCURSES_VERSION_MAJOR) && defined(NCURSES_VERSION_MINOR)
        "+libncurses (" STRINGIZE(NCURSES_VERSION_MAJOR) "." STRINGIZE(NCURSES_VERSION_MINOR) ")",
#else
        "+libncurses",
#endif
#elif defined(HAVE_LIBNCURSESW)
#if defined(NCURSES_VERSION)
        "+libncursesw (" NCURSES_VERSION ")",
#elif defined(NCURSES_VERSION_MAJOR) && defined(NCURSES_VERSION_MINOR)
        "+libncursesw (" STRINGIZE(NCURSES_VERSION_MAJOR) "." STRINGIZE(NCURSES_VERSION_MINOR) ")",
#else
        "+libncursesw",
#endif
#elif defined(HAVE_LIBCURSES)
        "+libcurses",
#elif defined(HAVE_LIBTERMCAP)
        "+libtermcap",
#endif
#if defined(HAVE_TERMINFO)
        "+terminfo",
#endif
#if defined(HAVE_TERMCAP)
        "+termcap",
#endif

        /* mouse support */
#if defined(HAVE_MOUSE)
        "+mouse",
#if defined(HAVE_LIBGPM)
        "+libgpm",
#endif
#endif

        /* filesystem options */
#if defined(HAVE_LONG_FILE_NAMES)
        "+filename-long",
#endif
#if defined(NOCASE_FILENAMES)
        "+filename-nocase",
#endif
#if defined(MONOCASE_FILENAMES)
        "+filename-monocase",
#endif
        "+vfs",                                 /* configure ?? */
    /*  "+vfsarchive",  */
    /*  "+vfsftp",      */
    /*  "+vfsplugin",   */

        /* build options */
#if defined(HAVE_LIBENCHANT)
        "+libenchant",
#elif defined(HAVE_LIBHUNSPELL)
        "+libhunspell",
#elif defined(HAVE_LIBASPELL)
        "+libaspell",
#endif

#if defined(HAVE_LIBCLANG)
        "+libclang",                            /* TODO */
#endif
#if defined(HAVE_LIBTRANSLATE)
        "+libtranslate",                        /* TODO */
#endif
#if defined(HAVE_LIBEXPLAIN)
        "+libexplain",                          /* TODO */
#endif

        "+wchar",                               /* configure ?? */
#if defined(HAVE_LIBICU)
#if defined(U_ICU_VERSION)
        "+libicu (" U_ICU_VERSION ")",
#elif defined(U_ICU_VERSION_MAJOR_NUM) && defined(U_ICU_VERSION_MINOR_NUM)
        "+libicu (" STRINGIZE(U_ICU_VERSION_MAJOR_NUM) "." STRINGIZE(U_ICU_VERSION_MINOR_NUM) ")",
#else
        "+libicu",
#endif
#elif defined(HAVE_APRICONV)
        "+libapriconv",
#elif defined(HAVE_LIBICONV)
        "+libiconv",
#endif
#if defined(HAVE_LIBCHARTABLE)
        "+libchartable",
#endif
#if defined(HAVE_LIBCHARDET)
        "+libchardet",                          /* Mozilla character detector */
#endif
#if defined(HAVE_LIBENCA)
        "+libenca",                             /* TODO */
#endif
#if (defined(HAVE_LIBGUESS_LIBGUESS_H) || defined(HAVE_LIBGUESS_H)) && \
            defined(HAVE_LIBGUESS)
        "+libguess",                            /* www.atheme.org */
#endif
#if defined(HAVE_MAGIC_H) && defined(HAVE_LIBMAGIC)
        "+libmagic",
#endif

#if defined(HAVE_ARCHIVE_H) && defined(HAVE_LIBARCHIVE)
#if defined(ARCHIVE_VERSION_STRING)
        "+" ARCHIVE_VERSION_STRING,             /* libarchive 2.4.12 */
#else
        "+libarchive",
#endif
#endif

#if defined(HAVE_CURL_CURL_H) && defined(HAVE_LIBCURL)
        "+libcurl",
#endif
#if defined(HAVE_OPENSSL)
        "+libopenssl",
#endif

        /* regular expressions */
#if defined(ONIGURUMA_VERSION_MAJOR)
        "+libonigurma (" STRINGIZE(ONIGURUMA_VERSION_MAJOR) "." STRINGIZE(ONIGURUMA_VERSION_MINOR) "." STRINGIZE(ONIGURUMA_VERSION_TEENY) ")",
#endif
#if defined(TRE_VERSION)
        "+libtre (" TRE_VERSION ")",
#endif

        /* malloc */
#if defined(HAVE_LIBDLMALLOC)
        "+libdlmalloc",
#elif defined(HAVE_LIBDBMALLOC)
        "+libdbmalloc",
#elif defined(HAVE_LIBTCMALLOC)
        "+libtcmalloc",
#endif

        /* general/misc */
#if defined(HAVE_LTDL_H) && defined(HAVE_LIBLTDL)
        "+libltdl",
#endif

        /* maths */
#if defined(NO_FLOAT_MATH)
        "-float",
#else
        "+float",
#endif

        NULL
    };


/*  Function:           set_feature
 *      set_feature primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: set_feature - Config an editor feature.

        int
        set_feature([int|string feature], [string value])

    Macro Description:
        The 'set_feature()' primitive sets the status of the specific
        feature 'feature'.

        Warning!:
        The 'set_feature()' primitive is an experimental interface
        and may change without notice.

    Macro Parameters:
        feature - Name of the feature.
        value - Configuration value.

    Macro Return:
        The 'set_feature()' primitive returns non-zero on success,
        otherwise zero on error.

    Macro Portability;
        A Grief extension.

    Macro See Also:
        inq_feature
 */
void
do_set_feature(void)            /* (string feature, declare value) */
{
    /*TODO*/
}


/*  Function:           inq_feature
 *      inq_feature primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: inq_feature - Retrieve an editor feature.

        int|list
        inq_feature([int|string feature], [string value])

    Macro Description:
        The 'inq_feature()' primitive tsets the status of the
        specific feature 'feature', returning its current
        configuration value.

        Warning!:
        The 'inq_feature()' primitive is an experimental interface
        and may change without notice.

    Macro Parameters:
        feature - Name of the feature.
        value - Configuration value.

    Macro Return:
        The 'inq_feature()' primitive returns non-zero on success,
        otherwise zero on error.

    Macro Portability;
        A Grief extension.

    Macro See Also:
        set_feature, inq_display_mode, set
 */
void
inq_feature(void)               /* ([string pattern|int index], [string value]) */
{
    const int count = (sizeof(x_features)/sizeof(x_features[0]));
    const char *pattern = get_xstr(1);          /* optional pattern */
    LIST *newlp = NULL;
    int i, results = 0;
//  int first = -1;

//  if (! isa_undef(1)) {                       /* pattern or index, TODO */

        if (pattern) {                          /* filter */
            for (i = 0; i < count; ++i) {
                if (wild_match(x_features[i], pattern, MATCH_NOCASE)) {
                    if (1 == ++results) {
//                      first = i;
                    }
                }
            }
        } else {
            results = count;
        }
//  }

//  if (! isa_undef(2)) {                       /* second form, results value */
//      if (first >= 0) {
//          sys_assign_string(2, x_features_value[first]);
//      }
//      acc_assign_int(results);
//      return;
//  }

    if (results) {                              /* return results */
        const int llen = (results * sizeof_atoms[F_LIT]) + sizeof_atoms[F_HALT];

                                                /* push each feature */
        if (NULL != (newlp = lst_alloc(llen, results))) {
            LIST *lp = newlp;

            for (i = 0; i < count; ++i) {
                if (!pattern || wild_match(x_features[i], pattern, MATCH_NOCASE)) {
                    lp = atom_push_const(lp, x_features[i]);
                    --results;
                }
            }
            assert(0 == results);
            atom_push_halt(lp);
            acc_donate_list(newlp, llen);
            return;
        }
    }
    acc_assign_null();
}

/*eof*/
