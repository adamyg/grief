#include <edidentifier.h>
__CIDENT_RCSID(gr_spell_enchant_c,"$Id: spell_enchant.c,v 1.13 2022/08/10 15:44:58 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: spell_enchant.c,v 1.13 2022/08/10 15:44:58 cvsuser Exp $
 * Spell implementation - enchant driver.
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
#include <libstr.h>                             /* str_...()/sxprintf() */

#if defined(HAVE_LIBENCHANT)
#if defined(HAVE_ENCHANT_ENCHANT_H)
#include <enchant/enchant.h>
#else
#include <enchant.h>
#endif
#endif

#include "builtin.h"
#include "debug.h"
#include "eval.h"
#include "file.h"
#include "lisp.h"
#include "spell.h"


#if defined(HAVE_LIBENCHANT)

typedef struct en_session {
#define EN_SUGGEST          12                  /* any more is generally pointless */
#define EN_DICTIONARIES     20

    Spell_t                 s_base;
    EnchantBroker *         s_broker;
    unsigned                s_open;
    unsigned                s_match;
    
    unsigned                s_suggestions;
    const char *            s_sugglist[EN_SUGGEST+1];
    
    unsigned                s_dictionaries;
    const char *            s_dictlist[EN_DICTIONARIES+1];
    
    struct en_dictionary {
        EnchantDict *       handle;
        char                name[16];
        const char **       sugglist;
        size_t              suggcnt;
    } s_instances[EN_DICTIONARIES];
} SpellSession_t;


static int                  en_language(Spell_t *spell, const char *dict, int enable);
static const char **        en_dictionaries(Spell_t *spell, int flags);
static int                  en_check(Spell_t *spell, int flags, const char *word, int length);
static const char **        en_suggest(Spell_t *spell, int flags, const char *word, int length);
static int                  en_add(Spell_t *spell, int flags, const char *word, const char *affix);
static int                  en_remove(Spell_t *spell, int flags, const char *word);
static void                 en_close(Spell_t *spell);

static void                 brokerdescription(const char * const provider_name, const char * const provider_desc,
                                    const char * const provider_file, void *user_data);

static void                 dictionarydescriptions(const char * const lang_tag,
                                    const char * const provider_name, const char * const provider_desc,
                                        const char * const provider_file, void * user_data);

static int                  en_dict_open(SpellSession_t *session, const char *name);
static void                 en_dict_close(SpellSession_t *session, int d);


Spell_t *
spell_enchant_init(const char **langs)
{
    SpellSession_t *session;

    trace_log("enchant_init(version:%s)\n", enchant_get_version());

    if (NULL != (session = (SpellSession_t *)chk_calloc(sizeof(SpellSession_t), 1))) {

        if (NULL != (session->s_broker = enchant_broker_init())) {
            enchant_broker_describe(session->s_broker, brokerdescription, (void *)session);
            enchant_broker_list_dicts(session->s_broker, dictionarydescriptions, (void *)session);

//TODO      if (personal) {
//              enchant_broker_request_pwl_dict(session->s_broker, personal);
//          }
//          enchant_broker_set_ordering(session->s_broker, "en", "xxxx,xxxx");
            if (langs && *langs) {
                while (*langs) {
                    en_dict_open(session, *langs);
                    ++langs;
                }
            } else {
                if (-1 == en_dict_open(session, "en_GB")) {
                    en_dict_open(session, "en");
                }
            }
        }

        session->s_base.sf_description  = "enchant";
        session->s_base.sf_language     = en_language;
        session->s_base.sf_dictionaries = en_dictionaries;
        session->s_base.sf_check        = en_check;
        session->s_base.sf_suggest      = en_suggest;
        session->s_base.sf_add          = en_add;
        session->s_base.sf_remove       = en_remove;
        session->s_base.sf_close        = en_close;
    }
    return (Spell_t *)session;
}


static int
en_language(Spell_t *spell, const char *name, int enable)
{
    SpellSession_t *session = (SpellSession_t *)spell;
    int ret = -1;

    if (session->s_open) {
        struct en_dictionary *dict = session->s_instances;
        unsigned d;

        for (d = 0, dict = session->s_instances; d < session->s_open; ++d, ++dict) {
            if (0 == str_nicmp(name, dict->name, sizeof(dict->name))) {
                if (enable) {
                    if (d > 0 && session->s_open > 1) {
                        struct en_dictionary newtop = *dict;

                        while (d > 1) {         /* reorg */
                            session->s_instances[d] = session->s_instances[d - 1];
                            --d;
                        }
                        session->s_instances[0] = newtop;
                    }
                } else {
                    en_dict_close(session, d);  /* close, may reorg */
                }
                return 0;
            }
        }
    }

    if (enable && -1 == ret) {                  /* append */
        ret = en_dict_open(session, name);
    }
    return ret;
}


static const char **
en_dictionaries(Spell_t *spell, int flags)
{
    SpellSession_t *session = (SpellSession_t *)spell;

    __CUNUSED(flags)

    if (session->s_open) {
        if (session->s_dictionaries) {
            session->s_dictlist[session->s_dictionaries] = NULL;
            return session->s_dictlist;
        }
    }
    return NULL;
}


static int
en_check(Spell_t *spell, int flags, const char *word, int length)
{
    SpellSession_t *session = (SpellSession_t *)spell;

    __CUNUSED(flags)

    if (session->s_open) {
        struct en_dictionary *dict = session->s_instances;
        unsigned d;

        for (d = 0, dict = session->s_instances; d < session->s_open; ++d, ++dict) {
            if (0 == enchant_dict_check(dict->handle, word, length)) {
                session->s_match = d;
                return 0;                       /* good */
            }
        }                          
        return 1;                               /* bad */
    }
    return -1;                                  /* error */
}


static const char **
en_suggest(Spell_t *spell, int flags, const char *word, int length)
{
    SpellSession_t *session = (SpellSession_t *)spell;

    __CUNUSED(flags)

    if (session->s_open) {
        struct en_dictionary *dict = session->s_instances;
        unsigned total = 0, d;

        for (d = 0, dict = session->s_instances; d < session->s_open; ++d, ++dict) {
            const char **sugglist;
            size_t suggcnt = 0;

            if (dict->sugglist) {
                enchant_dict_free_suggestions(dict->handle, (char **)dict->sugglist);
                dict->sugglist = NULL;
                dict->suggcnt = 0;
            }
            sugglist = (const char **)enchant_dict_suggest(dict->handle, word, length, &suggcnt);
            if (sugglist && suggcnt > 0) {
                dict->sugglist = sugglist;
                dict->suggcnt = suggcnt;
                total += (unsigned) suggcnt;
            }
        }

        if (total) {
            const char **sugglist = session->s_sugglist;
            unsigned suggcnt = 0, s, w;

            for (d = 0, dict = session->s_instances; d < session->s_open && suggcnt < EN_SUGGEST; ++d, ++dict) {
                for (s = 0; s < dict->suggcnt; ++s) {
                    const char *suggest = dict->sugglist[s];

                    for (w = 0; w < suggcnt; ++w) {
                        if (0 == strcmp(suggest, sugglist[w])) {
                            break;              /* duplicate, ignore */
                        }
                    }
                    if (w >= suggcnt) {         /* push */
                        sugglist[suggcnt] = suggest;
                        if (++suggcnt >= EN_SUGGEST) {
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
en_add(Spell_t *spell, int flags, const char *word, const char *affix)
{
    SpellSession_t *session = (SpellSession_t *)spell;

    __CUNUSED(flags)
    __CUNUSED(word)
    __CUNUSED(affix)

    if (session->s_broker) {
//      enchant_dict_add_to_session(EnchantDict *dict, word, strlen(word));
    }
    return -1;
}


static int
en_remove(Spell_t *spell, int flags, const char *word)
{
    SpellSession_t *session = (SpellSession_t *)spell;

    __CUNUSED(flags)
    __CUNUSED(word)

    if (session->s_broker) {
//      enchant_dict_remove_from_session(EnchantDict *dict, const char *const word, ssize_t len);
    }
    return -1;
}


static void
en_close(Spell_t *spell)
{
    SpellSession_t *session = (SpellSession_t *)spell;

    if (session->s_broker) {
        if (session->s_open) {
            while (session->s_open > 0) {
                en_dict_close(session, 0);
            }
        }

        while (session->s_dictionaries > 0) {
            const int idx = --session->s_dictionaries;

            chk_free((void *)session->s_dictlist[idx]);
            session->s_dictlist[idx] = NULL;
        }

        enchant_broker_free(session->s_broker);
        session->s_broker = NULL;
    }
    chk_free(spell);
}


static void
brokerdescription(
    const char * const provider_name, const char * const provider_desc, const char * const provider_file, void *user_data)
{   
    __CUNUSED(user_data)
    trace_log("\tbroker(name:%s, desc:%s, file:%s)\n", provider_name, provider_desc, provider_file);
}


static void
dictionarydescriptions(const char * const lang_tag,
    const char * const provider_name, const char * const provider_desc, const char * const provider_file, void * user_data)
{
    __CUNUSED(provider_desc)
    __CUNUSED(provider_file)
    trace_log("\tdictionary(%8s, %s)\n", lang_tag, provider_name);
    if (user_data) {
        SpellSession_t *session = (SpellSession_t *)user_data;

        if (session->s_dictionaries < EN_DICTIONARIES) {
            session->s_dictlist[session->s_dictionaries++] = chk_salloc(lang_tag);
        }
    }
}


static int
en_dict_open(SpellSession_t *session, const char *name)
{
    int ret = -1;

    if (session->s_open < EN_DICTIONARIES) {
        if (enchant_broker_dict_exists(session->s_broker, name)) {
            struct en_dictionary *dict = session->s_instances + session->s_open;
            void *handle;

            if (NULL != (handle = enchant_broker_request_dict(session->s_broker, name))) {
                (void) strxcpy(dict->name, name, sizeof(dict->name));
                dict->handle = handle;
                ret = session->s_open++;
            }
        }
    }
    trace_log("\tenchant::dict_open(%s) ; %d\n", name, ret);
    return ret;
}


static void
en_dict_close(SpellSession_t *session, int d)
{
    assert(d >= 0);
    assert((unsigned)d <= session->s_open);

    if (session->s_open && (unsigned)d <= session->s_open) {
        struct en_dictionary *dict = session->s_instances + d;

        assert(dict->name[0]);
        assert(dict->handle);

        trace_log("\tenchant::dict_close(%s)\n", dict->name);

        if (dict->sugglist) {
            enchant_dict_free_suggestions(dict->handle, (char **)dict->sugglist);
        }
        enchant_broker_free_dict(session->s_broker, dict->handle);

        --session->s_open;
        while ((unsigned)d < session->s_open) { /* reorg, move up */
            session->s_instances[d++] = *++dict;
        }
        memset(dict, 0, sizeof(*dict));
    }
}


#else       /*HAVE_LIBENCHANT*/

Spell_t *
spell_enchant_init(const char **langs)
{
    __CUNUSED(langs)
    return NULL;
}

#endif      /*HAVE_LIBENCHANT*/
