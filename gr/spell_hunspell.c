#include <edidentifier.h>
__CIDENT_RCSID(gr_spell_hunspell_c,"$Id: spell_hunspell.c,v 1.19 2022/08/10 15:44:58 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: spell_hunspell.c,v 1.19 2022/08/10 15:44:58 cvsuser Exp $
 * Spell implementation - hunspell driver.
 *
 *  http://sourceforge.net/hunspell/projects
 *  http://hunspell.sourceforge.net/
 *      OpenOffice 1.x - based on myspell
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
#include <eddir.h>
#include <libstr.h>                             /* str_...()/sxprintf() */

#if defined(WIN32) && defined(HAVE_HUSPELLDLL_H)
#include <hunspelldll.h>
#define HAVE_HUNSPELLDLL
#elif defined(WIN32)
#define HAVE_HUNSPELLDLL
#define WIN32_HUNSPELL_MAP                      /* see: win32_hunspell.h */
#include "../libw32/win32_hunspell.h"
#elif defined(HAVE_LIBHUNSPELL)
#if defined(HAVE_HUNSPELL_HUNSPELL_H)
#include <hunspell/hunspell.h>

#else
#include <hunspell.h>
#endif
#endif

#include "spell.h"
#include "system.h"
#include "debug.h"
#include "file.h"


//  Obtaining dictionaries
//      *Grief* does not currently include any bundled spelling dictionaries. Therefore, if you want
//      spell-checking to be enabled in the editor, you will need to install dictionaries separately.
//
//      The editor uses the hunspell spell-checker; more information is available at that project's site.
//      Hunspell also supports dictionaries from the older myspell program.
//
//      One easy source of dictionaries is the OpenOffice.org project, which uses the same spelling engine.
//      There is a list of available dictionaries at http://extensions.services.openoffice.org/en/dictionaries.
//
//      Note that Grief shall only use the spelling dictionaries (*.dic and *.aff) for each language;
//      to install them, rename the downloaded .oxt file to .zip and extract the spelling dictionary files
//      (they may be contained in a subdirectory).
//
//  Installing
//      The dictionaries are installed in different locations depending on your platform. Note: The paths given
//      below apply to the default configuration of the versions distributed here. If you use Grief, the
//      paths may differ.
//
//  Windows
//      To find the correct folder, use "Help -> Settings and Resources...", navigate to the "Resources" folder
//      given there, and finally put the spelling dictionaries in the dictionaries folder.
//
//      By default, Grief resources directory should be located in your "user profile" directory,
//      typically C:\Documents and Settings\Your Name\. This is automatically created the first time you run
//      Grief, and some default resources are copied there. There should be a 'dictionaries' subdirectory,
//      but it is initially empty. Place the *.dic and *.aff files there, and then re-start TeXworks.
//
//  Mac OS X
//      To find the correct folder, use "Help -> Settings and Resources...", navigate to the "Resources" folder
//      given there, and finally put the spelling dictionaries in the 'dictionaries' folder.
//
//      By default, the Grief resources folder should be located inside the Library folder of your
//      Home (user) folder. This is automatically created the first time you run Grief, and some default
//      resources are copied there. There should be a 'dictionaries' subdirectory, but it is initially empty.
//      Place the *.dic and *.aff files there, and then re-start TeXworks.
//
//  Cygwin/Linux/Unix
//      On these systems, it is quite likely that appropriate dictionaries are already present, thanks to
//      other programs using the same spell-check engine. However, if you need to add dictionaries, they
//      should be placed in /usr/share/myspell/dicts (*).
//
//      (*) This will typically require administrative privileges.
//
#if defined(HAVE_LIBHUNSPELL) || defined(HAVE_HUNSPELLDLL)
static const char *x_paths[] = {                /* search paths */
#if defined(WIN32)
    "$(USERPROFILE)\\Application\\OpenOffice.org 2\\user\\wordbook",
    "C:\\Hunspell\\",

#elif defined(__APPLE__)
    "$(HOME)/Library/Spelling",
    "/Library/Spelling",

#else   /*cygwin/unix|linux*/
    "$(HOME)/openoffice.org/3/user/wordbook",
    "/usr/share/hunspell",
    "/usr/share/myspell/dicts",
    "/usr/share/myspell",
#endif

    NULL
    };

#if defined(HAVE_HUNSPELLDLL) &&                /* libw32/win32_huspell.h */ \
            !defined(GR_LIBW32_WIN32_HUNSPELL_H_INCLUDED) && \
            !defined(GR_WIN32_HUNSPELL_H_INCLUDED)
#define Hunspell_create     hunspell_initialize
#define Hunspell_create_key hunspell_initialize_key
#define Hunspell_destroy    hunspell_uninitialize
#define Hunspell_get_dic_encoding \
                            hunspell_get_dic_encoding
#define Hunspell_spell      hunspell_spell
#define Hunspell_suggest    hunspell_suggest
#define Hunspell_free_list  hunspell_free_list
#define Hunspell_add        hunspell_add
#define Hunspell_add_with_affix \
                            hunspell_add_with_affix
#else
#define HUNHANDLE           Hunhandle *
#endif

typedef struct hs_session {
#define HS_SUGGEST          12                  /* any more is generally pointless */
#define HS_DICTIONARIES     32

    Spell_t             s_base;                 /* base spell object */
    int                 s_open;                 /* dictionary open count */
    int                 s_match;                /* last directionary matched */
    const char *        s_suggest[HS_SUGGEST + 1];
    const char *        s_dictionaries[HS_DICTIONARIES + 1];

    struct hs_dictionary {
        HUNHANDLE       handle;
        char            name[32];
        const char *    aff;
        const char *    dic;
        int             unicode;                /* UTF-8, otherwise ASCII */
        char **         sugglist;
        int             suggcnt;
    } s_instances[HS_DICTIONARIES];
} SpellSession_t;

static int              hs_language(Spell_t *spell, const char *dict, int enable);
static const char **    hs_dictionaries(Spell_t *spell, int flags);
static int              hs_check(Spell_t *spell, int flags, const char *word, int length);
static const char **    hs_suggest(Spell_t *spell, int flags, const char *word, int length);
static int              hs_add(Spell_t *spell, int flags, const char *word, const char *affix);
static void             hs_close(Spell_t *spell);

static int              hs_dict_open(SpellSession_t *session, const char *dict, const char *aff, const char *dic, const char *key);
static void             hs_dict_close(SpellSession_t *session, int idx);
static char *           hs_dict_resolve(const char **paths, const char *name, int length, const char *ext);
static char *           hs_dict_name(const char *aff, int ext);
static int              hs_dict_cmp(const char *a, const char *b);


Spell_t *
spell_hunspell_init(const char **langs, const char **bdictionaries)
{
    SpellSession_t *session;

    trace_log("hunspell_init()\n");
    if (NULL != (session = (SpellSession_t *)chk_calloc(sizeof(SpellSession_t), 1))) {
        while (*langs) {
            const char *name = *langs;
            char *aff;

            if ((bdictionaries && NULL != (aff = hs_dict_resolve(bdictionaries, name, strlen(name), ".aff"))) ||
                    (NULL != (aff = hs_dict_resolve(x_paths, name, strlen(name), ".aff")))) {
                if (hs_dict_open(session, name, aff, NULL, NULL) >= 0) {
                    break;                      /* stop at first */
                }
            }
            ++langs;
        }

        session->s_base.sf_description  = "hunspell";
        session->s_base.sf_language     = hs_language;
        session->s_base.sf_dictionaries = hs_dictionaries;
        session->s_base.sf_check        = hs_check;
        session->s_base.sf_suggest      = hs_suggest;
        session->s_base.sf_add          = hs_add;
//      session->s_base.sf_remove       = hs_remove;
        session->s_base.sf_close        = hs_close;
    }
    return (Spell_t *)session;
}


static int
hs_language(Spell_t *spell, const char *name, int enable)
{
    SpellSession_t *session = (SpellSession_t *)spell;
    int ret = -1;

    if (session->s_open) {
        struct hs_dictionary *dict = session->s_instances;
        int d;

        for (d = 0, dict = session->s_instances; d < session->s_open; ++d, ++dict) {
            if (0 == str_nicmp(name, dict->name, sizeof(dict->name))) {
                if (enable) {
                    if (d > 0 && session->s_open > 1) {
                        struct hs_dictionary newtop = *dict;

                        while (d > 1) {         /* reorg */
                            session->s_instances[d] = session->s_instances[d - 1];
                            --d;
                        }
                        session->s_instances[0] = newtop;
                    }
                } else {
                    hs_dict_close(session, d);  /* close, may reorg */
                }
                return 0;
            }
        }
    }

    if (enable && -1 == ret) {                  /* append */
        char *aff;

        if (NULL != (aff = hs_dict_resolve(x_paths, name, strlen(name), ".aff"))) {
            hs_dict_open(session, name, aff, NULL, NULL);
        }
    }
    return ret;
}


const char **
hs_dictionaries(Spell_t *spell, int flags)
{
    SpellSession_t *session = (SpellSession_t *)spell;

    __CUNUSED(flags)
    if (session->s_open) {
        struct hs_dictionary *dict = session->s_instances;
        int dictcnt = 0, d;

        for (d = 0, dict = session->s_instances; d < session->s_open; ++d, ++dict) {
            if (dict->handle) {
                session->s_dictionaries[ dictcnt++ ] = dict->name;
                if (dictcnt >= HS_DICTIONARIES) {
                    break;
                }
            }
        }

        if (1 || flags) {                       /* FIXME */
            char expand[MAX_PATH];
            const char *path;
            int i, k;

            for (i = 0; NULL != (path = x_paths[i]); ++i) {
                if (*path) {
                    const char *dictname;
                    struct dirent *dent;
                    DIR *dir;

                    if (NULL != (path = file_expand(path, expand, sizeof(expand))) &&
                            NULL != (dir = opendir(path))) {
                        while (NULL != (dent = readdir(dir))) {
                            if (NULL != (dictname = hs_dict_name(dent->d_name, FALSE))) {
                                for (k = 0; k < dictcnt; ++k) {
                                    if (hs_dict_cmp(dictname, session->s_dictionaries[ k ])) {
                                        chk_free((void *)dictname);
                                        dictname = NULL;
                                        break;
                                    }
                                }
                                if (dictname) {
                                    session->s_dictionaries[ dictcnt++ ] = dictname;
                                    if (dictcnt >= HS_DICTIONARIES) {
                                        break;
                                    }
                                }
                            }
                        }
                        closedir(dir);
                    }
                }
            }
        }

        session->s_dictionaries[ dictcnt++ ] = NULL;
        return session->s_dictionaries;
    }
    return NULL;
}


static int
hs_check(Spell_t *spell, int flags, const char *word, int length)
{
    SpellSession_t *session = (SpellSession_t *)spell;

    __CUNUSED(flags)
    __CUNUSED(length)
    if (session->s_open) {
        struct hs_dictionary *dict = session->s_instances;
        int d;

        for (d = 0, dict = session->s_instances; d < session->s_open; ++d, ++dict) {
            if (Hunspell_spell(dict->handle, word)) {
                session->s_match = d;
                return 0;                       /* good */
            }
        }
        return 1;                               /* bad */
    }
    return -1;                                  /* error */
}


static const char **
hs_suggest(Spell_t *spell, int flags, const char *word, int length)
{
    SpellSession_t *session = (SpellSession_t *)spell;

    __CUNUSED(flags)
    __CUNUSED(length)
    if (session->s_open) {
        struct hs_dictionary *dict = session->s_instances;
        int total = 0, d;

        for (d = 0, dict = session->s_instances; d < session->s_open; ++d, ++dict) {
            int cnt;

            if (dict->sugglist) {
                Hunspell_free_list(dict->handle, &dict->sugglist, dict->suggcnt);
                dict->sugglist = NULL;
                dict->suggcnt = 0;
            }
            if ((cnt = Hunspell_suggest(dict->handle, &dict->sugglist, word)) > 0) {
                dict->suggcnt = cnt;
                total += cnt;
            }
        }

        if (total) {
            const char **sugglist = session->s_suggest;
            int suggcnt = 0, s, w;
                                                /* foreach(dictionary) */
            for (d = 0, dict = session->s_instances; d < session->s_open && suggcnt < HS_SUGGEST; ++d, ++dict) {
                for (s = 0; s < dict->suggcnt; ++s) {
                    const char *suggest = dict->sugglist[s];

                    for (w = 0; w < suggcnt; ++w) {
                        if (0 == strcmp(suggest, sugglist[w])) {
                            break;              /* duplicate, ignore */
                        }
                    }
                    if (w >= suggcnt) {         /* push */
                        sugglist[suggcnt] = suggest;
                        if (++suggcnt >= HS_SUGGEST) {
                            break;
                        }
                    }
                }
            }
            sugglist[suggcnt] = NULL;           /* terminate list */
            return sugglist;
        }
    }
    return NULL;
}


static int
hs_add(Spell_t *spell, int flags, const char *word, const char *affix)
{
    SpellSession_t *session = (SpellSession_t *)spell;
    int ret = -1;

    __CUNUSED(flags)
    if (session->s_open) {
        struct hs_dictionary *dict = session->s_instances;

        if (affix) {
            ret = Hunspell_add_with_affix(dict->handle, word, affix);
        } else {
            ret = Hunspell_add(dict->handle, word);
        }
    }
    return ret;
}


static void
hs_close(Spell_t *spell)
{
    SpellSession_t *session = (SpellSession_t *)spell;

    if (session) {
        while (session->s_open > 0) {
            hs_dict_close(session, session->s_open - 1);
        }
    }
    chk_free(session);
}


/*  Function:           hs_dict_open
 *      Dictionary open.
 *
 *  Parameters:
 *      session -           Spell-check session.
 *      name -              Dictionary base name.
 *      aff -               aff path.
 *      dic -               dic path, otherwise NULL derived.
 *      key -               Optional key.
 *
 *  Returns:
 *      Session handle otherwise -1.
 */
static int
hs_dict_open(SpellSession_t *session, const char *name, const char *aff, const char *dic, const char *key)
{
    if (session->s_open < HS_DICTIONARIES &&
            aff && (dic || (NULL != (dic = hs_dict_name(aff, TRUE))))) {
        struct hs_dictionary *dict = session->s_instances + session->s_open;
        HUNHANDLE handle;

        if (NULL != (handle = (key ? Hunspell_create_key(aff, dic, key) : Hunspell_create(aff, dic)))) {
            const int unicode =
                (0 == str_icmp(Hunspell_get_dic_encoding(handle), "UTF-8") ? 1 : 0);

            dict->handle = handle;
            dict->unicode = unicode;
            (void) strxcpy(dict->name, name, sizeof(dict->name));
            dict->aff = aff;
            dict->dic = dic;
            trace_log("hunspell_open(%s, %s) ; %d\n", aff, dic, session->s_open);
            return session->s_open++;
        }
    }
    chk_free((void *)aff), chk_free((void *)dic);
    return -1;
}


/*  Function:           hs_dict_close
 *      Dictionary close.
 *
 *  Parameters:
 *      session -           Spell-check session.
 *      idx -               Dictionary index.
 *
 *  Returns:
 *      nothing
 */
static void
hs_dict_close(SpellSession_t *session, int d)
{
    struct hs_dictionary *dict = session->s_instances + d;

    assert(d >= 0);
    assert(d <= session->s_open);

    if (session->s_open > 0) {
        if (dict->sugglist) {
            (void) Hunspell_free_list(dict->handle, &dict->sugglist, dict->suggcnt);
        }
        chk_free((void *)dict->aff);
        chk_free((void *)dict->dic);
        (void) Hunspell_destroy(dict->handle);

        --session->s_open;
        while (d < session->s_open) {           /* reorg, move up */
            session->s_instances[d++] = *++dict;
        }
        memset(dict, 0, sizeof(*dict));
    }
}


/*  Function:           hs_dict_resolve
 *      Resolve a dictionary path.
 *
 *  Parameters:
 *      paths -             Search path vector.
 *      name -              Base name.
 *      len -               Length of the base name, in characters.
 *      ext -               Extension.
 *
 *  Returns:
 *      Resolved path, otherwise NULL.
 */
static char *
hs_dict_resolve(const char **paths, const char *name, int len, const char *ext)
{
    const char *path, *expanded;
    char buf[MAX_PATH], expand[MAX_PATH];
    unsigned i;

    for (i = 0; NULL != (path = paths[i]); ++i) {
        if (path[0]) {
            sxprintf(buf, sizeof(buf), "%s%s%.*s%s", path, sys_delim(), len, name, ext);
        } else {
            sxprintf(buf, sizeof(buf), "%.*s%s", len, name, ext);
        }
        if (NULL != (expanded = file_expand(buf, expand, sizeof(expand)))) {
            trace_log("=> resolve [%s -> %s]\n", buf, expanded);
            if (0 == sys_access(expanded, R_OK)) {
                return chk_salloc(expanded);
            }
        }
    }
    return NULL;
}


static char *
hs_dict_name(const char *aff, int ext)
{
    const int len = strlen(aff);
    char *dic = NULL;

    if (len > 4 && 0 == strcmp(aff + len - 4, ".aff"))
        if (NULL != (dic = chk_salloc(aff))) {
            if (ext) {
                strcpy(dic + len - 3, "dic");
            } else {
                dic[len - 4] = 0;
            }
        }
    return dic;
}


static int
hs_dict_cmp(const char *a, const char *b)
{
    int ta, tb;

    for (ta = *a++, tb = *b++; ta && tb; ta = *a++, tb = *b++) {
        if (tolower(ta) == tolower(tb) ||
                (('-' == ta || '_' == ta) && ('-' == tb || '_' == tb))) {
            continue;
        }
        return FALSE;
    }
    return (ta == tb);
}

#else   /*!LIBHUNSPELL*/

Spell_t *
spell_hunspell_init(const char **langs, const char **bdictionaries)
{
    __CUNUSED(langs)
    return NULL;
}

#endif
