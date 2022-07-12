#include <edidentifier.h>
__CIDENT_RCSID(gr_spell_aspell_c,"$Id: spell_aspell.c,v 1.13 2022/07/12 15:30:56 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: spell_aspell.c,v 1.13 2022/07/12 15:30:56 cvsuser Exp $
 * Spell implementation - aspell driver.
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
#include <edenv.h>                              /* gputenvv(), ggetenv() */

#include <libstr.h>
#include "echo.h"


/*  autoconf:
 *      #define HAVE_LIBASPELL
 *      #define HAVE_ASPELL_ASPELL_H
 *      #define HAVE_ASPELL_H
 */

#if defined(HAVE_LIBASPELL)
#if defined(WIN32)
#define ASPELL_NO_EXPORTS
#if defined(HAVE_ASPELL_ASPELL_H)
#include <aspell/aspell.h>
#elif defined(HAVE_ASPELL_H)
#include <aspell.h>
#else
#include "..\\aspell\\aspell.h"
#endif

#else   /*!WIN32*/
#if defined(HAVE_ASPELL_ASPELL_H)
#include <aspell/aspell.h>
#elif defined(HAVE_ASPELL_H)
#include <aspell.h>
#else
#include "/usr/include/aspell.h"
#endif
#endif
#endif

#include "debug.h"
#include "file.h"
#include "spell.h"


#if defined(HAVE_LIBASPELL)
/*
 *  aspell implementation
 */
typedef struct {
#define AS_SUGGEST          12                  /* any more is generally pointless */
#define AS_DICTIONARIES     20

    Spell_t                 s_base;
    MAGIC_t                 s_magic;
#define SESSION_MAGIC           MKMAGIC('A','s','S','s')
    int                     s_open;
    int                     s_match;
    const char *            s_suggest[AS_SUGGEST + 1];
    const char *            s_dictionaries[AS_DICTIONARIES + 1];

    struct as_dictionary {
        MAGIC_t             magic;
#define INSTANCE_MAGIC          MKMAGIC('A','s','I','n')
        AspellConfig *      config;
        AspellSpeller *     speller;
        char                name[16];
        int                 unicode;
    } s_instances[AS_DICTIONARIES];
} SpellSession_t;


static int                  as_language(Spell_t *spell, const char *dict, int enable);
static const char **        as_dictionaries(Spell_t *spell, int flags);
static int                  as_check(Spell_t *spell, int flags, const char *word, int length);
static const char **        as_suggest(Spell_t *spell, int flags, const char *word, int length);
static int                  as_add(Spell_t *spell, int flags, const char *word, const char *affix);
static void                 as_close(Spell_t *spell);

static int                  as_dict_open(SpellSession_t *session, const char *name);
static void                 as_dict_dump(SpellSession_t *session, AspellConfig *config);
static void                 as_dict_close(SpellSession_t *session, int d);
static void                 as_dictionary_clear(SpellSession_t *session);
static void                 as_suggest_clear(SpellSession_t *session);


Spell_t *
spell_aspell_init(const char **langs)
{
    SpellSession_t *session;

    trace_log("aspell_init()\n");

    if (NULL != (session = (SpellSession_t *)chk_calloc(sizeof(SpellSession_t), 1))) {

        session->s_magic = SESSION_MAGIC;
        session->s_open = 0;

        if (langs && langs[0]) {
            while (*langs) {
                as_dict_open(session, *langs);
                ++langs;
            }
        } else {
            as_dict_open(session, NULL);
        }

        session->s_base.sf_description  = "aspell";
        session->s_base.sf_language     = as_language;
        session->s_base.sf_dictionaries = as_dictionaries;
        session->s_base.sf_check        = as_check;
        session->s_base.sf_suggest      = as_suggest;
        session->s_base.sf_add          = as_add;
        session->s_base.sf_close        = as_close;
    }
    return (Spell_t *)session;
}


static int
as_language(Spell_t *spell, const char *name, int enable)
{
    SpellSession_t *session = (SpellSession_t *)spell;
    int ret = -1;

    assert(SESSION_MAGIC == session->s_magic);
    if (session->s_open) {
        struct as_dictionary *dict = session->s_instances;
        unsigned dictcnt = 0, d;
                                                /* search existing */
        for (d = 0, dict = session->s_instances; d < session->s_open; ++d, ++dict) {

            assert(INSTANCE_MAGIC == dict->magic);

            if (0 == str_nicmp(name, dict->name, sizeof(dict->name))) {
                if (enable) {
                    if (d > 0) {
                        struct as_dictionary newtop = *dict;

                        while (d > 1) {         /* reorg */
                            session->s_instances[d] = session->s_instances[d - 1];
                            --d;
                        }
                        session->s_instances[0] = newtop;
                    }
                } else {
                    as_dict_close(session, d);  /* close, may reorg */
                }
                return 0;
            }
        }
    }

    if (enable && -1 == ret) {                  /* append */
        ret = as_dict_open(session, name);
    }
    return ret;
}


static const char **
as_dictionaries(Spell_t *spell, int flags)
{
    SpellSession_t *session = (SpellSession_t *)spell;

    assert(SESSION_MAGIC == session->s_magic);
    if (session->s_open) {
        return session->s_dictionaries;         /* see as_dict_dump() */
    }
    return NULL;
}


static int
as_check(Spell_t *spell, int flags, const char *word, int length)
{
    SpellSession_t *session = (SpellSession_t *)spell;
    int ret = -1;

    assert(SESSION_MAGIC == session->s_magic);
    if (session->s_open) {
        struct as_dictionary *dict = session->s_instances;
        unsigned d;

        for (d = 0, dict = session->s_instances; d < session->s_open; ++d, ++dict) {
            const int check =
                aspell_speller_check(dict->speller, word, length);

            assert(INSTANCE_MAGIC == dict->magic);
            if (0 == check) {
                ret = 1;                        /* bad */
            } else if (1 == check) {
                session->s_match = d;
                ret = 0;                        /* good */
                break;
            }
        }
    }
    return ret;
}


static const char **
as_suggest(Spell_t *spell, int flags, const char *word, int length)
{
    SpellSession_t *session = (SpellSession_t *)spell;
    int ret = -1;

    assert(SESSION_MAGIC == session->s_magic);
    if (session->s_open) {
        struct as_dictionary *dict = session->s_instances;
        const char **sugglist = session->s_suggest;
        unsigned suggcnt = 0, d;

        as_suggest_clear(session);
                                                /* foreach(dictionary) */
        for (d = 0, dict = session->s_instances; d < session->s_open && suggcnt < AS_SUGGEST; ++d, ++dict) {
            const AspellWordList *suggestions;
            int words;

            assert(INSTANCE_MAGIC == dict->magic);
            suggestions = aspell_speller_suggest(dict->speller, word, length);
            if ((words = (int)aspell_word_list_size(suggestions)) > 0) {
                AspellStringEnumeration *els = aspell_word_list_elements(suggestions);

                while (words > 0) {             /* foreach(suggestion) */
                    const char *sug;

                    if (NULL != (sug = aspell_string_enumeration_next(els))) {
                        sugglist[suggcnt] = chk_salloc(sug);
                        if (++suggcnt >= AS_SUGGEST) {
                            break;
                        }
                    }
                    --words;
                }
                delete_aspell_string_enumeration(els);
            }
        }

        if (suggcnt) {
            sugglist[suggcnt] = NULL;           /* terminate list */
            return sugglist;
        }
    }
    return NULL;
}


static int
as_add(Spell_t *spell, int flags, const char *word, const char *affix)
{
    SpellSession_t *session = (SpellSession_t *)spell;
    int ret = -1;

    assert(SESSION_MAGIC == session->s_magic);
    if (session->s_open) {
        struct as_dictionary *dict = session->s_instances;

        assert(INSTANCE_MAGIC == dict->magic);
        if (affix) {
            ret = aspell_speller_store_replacement(dict->speller, word, -1, affix, -1);
        } else {
            ret = aspell_speller_add_to_personal(dict->speller, word, -1);
        }
    }
    return ret;
}


static void
as_close(Spell_t *spell)
{
    SpellSession_t *session = (SpellSession_t *)spell;

    assert(SESSION_MAGIC == session->s_magic);
    while (session->s_open > 0) {
        as_dict_close(session, session->s_open - 1);
    }
    as_dictionary_clear(session);
    as_suggest_clear(session);
    session->s_magic = 0;
    chk_free(session);
}


static int
as_dict_open(SpellSession_t *session, const char *name)
{
    int ret = -1;

    if (session->s_open < AS_DICTIONARIES) {
        AspellConfig *config = NULL;
        AspellSpeller *speller = NULL;
        AspellCanHaveError *asret;

        config = new_aspell_config();
        assert(config);
#if defined(WIN32)
//      aspell_config_replace(config, "dict-dir", "/grief/aspell");
//      aspell_config_replace(config, "conf-dir", "/grief/aspell");
#endif
        aspell_config_replace(config, "encoding", "utf-8");
        if (name && 0 == strcmp(name, "default")) {
            name = NULL;
        }
        if (name) {
            aspell_config_replace(config, "language-tag", name);
        }
        if (0 == session->s_open) {
            as_dict_dump(session, config);
        }

        asret = new_aspell_speller(config);
        assert(asret);
        if (aspell_error(asret)) {
#if defined(WIN32)
            MessageBox(NULL, aspell_error_message(asret), "Spell Check", MB_OK);
#endif
            errorf("%s.", aspell_error_message(asret));
        } else {
            speller = to_aspell_speller(asret);
        }

        if (config) {
            if (speller) {
                struct as_dictionary *dict = session->s_instances + session->s_open;

                dict->magic   = INSTANCE_MAGIC;
                dict->config  = config;
                dict->speller = speller;
                dict->unicode = 1;
                strxcpy(dict->name, (name ? name : "default"), sizeof(dict->name));
                ret = session->s_open++;
            } else {
                delete_aspell_config(config);
            }
        }
    }

    trace_log("\taspell_open(%s) : %d\n", (name ? name : "default"), ret);
    return ret;
}


static void
as_dict_dump(SpellSession_t *session, AspellConfig *config)
{
    AspellKeyInfoEnumeration *keys = aspell_config_possible_elements(config, 0);
    AspellDictInfoList *dlist = get_aspell_dict_info_list(config);
    AspellDictInfoEnumeration *dels = aspell_dict_info_list_elements(dlist);
    const AspellKeyInfo *conf;
    const AspellDictInfo *dict;
    int dictcnt = 0, d;
                                                /* configuration elements */
    trace_log("\n\t%-29s %-49s %s\n", "KEY", "DESC", "VALUE (default)");
    while (NULL != (conf = aspell_key_info_enumeration_next(keys))) {
        const char *value = aspell_config_retrieve(config, conf->name);

        trace_log("\t%-29.29s %-49.49s %s(%s)\n", conf->name,
            (conf->desc ? conf->desc : ""), (value ? value : ""), conf->def);
    }
                                                /* dictionary list, order by code */
    trace_log("\n\t%-8s %-30s %-20s %-6s %-10s\n", "CODE", "NAME", "JARGON", "SIZE", "MODULE");
    while (NULL != (dict = aspell_dict_info_enumeration_next(dels))) {

        if ((d = dictcnt) < AS_DICTIONARIES) {
            while (--d >= 0) {
                if (0 == strcmp(session->s_dictionaries[d], dict->code)) {
                    break;
                }
            }
            if (d < 0) {                        /* en_GB etc */
                trace_log("\t%-8s %-30s %-20s %-6s %-10s\n",
                    dict->code, dict->name, dict->jargon, dict->size_str, dict->module->name);
                session->s_dictionaries[dictcnt++] = chk_salloc(dict->code);
            }
        }

        if (d >= 0) {
            trace_log("\t%-8s %-30s %-20s %-6s %-10s\n",
                "", dict->name, dict->jargon, dict->size_str, dict->module->name);
        }
    }
    session->s_dictionaries[dictcnt] = NULL;
}


static void
as_dict_close(SpellSession_t *session, int d)
{
    assert(d >= 0);
    assert(d <= session->s_open);

    if (session->s_open) {
        struct as_dictionary *dict = session->s_instances + d;

        assert(d < session->s_open);
        assert(INSTANCE_MAGIC == dict->magic);

        if (dict->speller) {
            aspell_speller_save_all_word_lists(dict->speller);
            delete_aspell_speller(dict->speller);
        }
        delete_aspell_config(dict->config);

        --session->s_open;

        while (d < session->s_open) {           /* reorg, move up */
            session->s_instances[d++] = *++dict;
        }

        memset(dict, 0, sizeof(*dict));         /* clear image */
    }
}


static void
as_dictionary_clear(SpellSession_t *session)
{
    const char **dictlist = session->s_dictionaries;
    unsigned dictcnt;
    void *word;

    assert(SESSION_MAGIC == session->s_magic);
    for (dictcnt = 0; NULL != (word = (void *)dictlist[dictcnt]); ++dictcnt) {
        dictlist[dictcnt] = NULL;
        chk_free(word);
    }
}


static void
as_suggest_clear(SpellSession_t *session)
{
    const char **sugglist = session->s_suggest;
    unsigned suggcnt;
    void *word;

    assert(SESSION_MAGIC == session->s_magic);
    for (suggcnt = 0; NULL != (word = (void *)sugglist[suggcnt]); ++suggcnt) {
        sugglist[suggcnt] = NULL;
        chk_free(word);
    }
}


#else   /* !LIBASPELL */

Spell_t *
spell_aspell_init(const char **langs)
{
    __CUNUSED(langs)
    return NULL;
}

#endif  /* !LIBSPELL */

