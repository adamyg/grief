#include <edidentifier.h>
__CIDENT_RCSID(gr_dict_c,"$Id: dict.c,v 1.21 2014/11/16 17:28:38 ayoung Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: dict.c,v 1.21 2014/11/16 17:28:38 ayoung Exp $
 * Dictionary functionality.
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
#include <edhandles.h>
#include <assert.h>

#include <tailqueue.h>
#include <stable.h>
#include <stype.h>

#include "dict.h"

#include "accum.h"
#include "builtin.h"
#include "debug.h"
#include "echo.h"
#include "eval.h"
#include "lisp.h"
#include "main.h"
#include "object.h"
#include "symbol.h"
#include "word.h"

#define TABLESIZE           16
#define TABLEFACTOR         2

typedef struct {
    MAGIC_t                 d_magic;            /* structure magic */
#define DICTMAGIC               MKMAGIC('d','i','c','t')

    IDENTIFIER_t            d_ident;            /* identifier */
    const char *            d_name;             /* name */
    const char *            d_blessed;          /* blessed */
    unsigned                d_attributes;       /* attributes */
#define DICT_READONLY           0x0001          /* read-only */
#define DICT_NOCREATE           0x0002          /* no create */
#define DICT_BLESSSED           0x1000
#define DICT_AUTOPTR            0x2000

    unsigned                d_references;       /* reference count */
    stable_t                d_stable;           /* hash table */
    int                     d_eachstate;        /* dict_each index */
    stblcursor_t            d_eachcursor;       /* dict_each stbl cursor */
} DICT;

static IDENTIFIER_t         x_dictseq;          /* Identifier sequence/identifier */
static stype_t *            x_dicts = NULL;     /* and container */

static DICT *               dict_lookup(int objid);


/*<<GRIEF>>
    Macro: create_dictionary - Create a dictionary.

        int
        create_dictionary(string ~name,
                int ~tablesize, int ~tablefactor)

    Macro Description:
        The 'create_dictionary()' primitive creates a new dictionary
        resource.

        A dictionary is a collections of associations. Dictionaries
        consist of pairs of keys and their corresponding values,
        both arbitrary data values. Dictionaries are also known as
        associative arrays or hash tables.

    Macro Parameters:
        name - Optional string containing the unique
            dictionary name, if omitted the dictionary shall be
            unnamed.

        size - Optional position integer specifying the size of
            the underlying table.

        factor - Optional position integer specifying the table
            fill factor.

    Macro Returns:
        The 'create_dictionary()' primitive returns the identifier
        associated with the new dictionary.

    Macro Portability:
        A Grief extension.

    Macro See Also:
        create_dictionary, delete_dictionary, set_property,
        get_property,

*/
void
do_create_dictionary(void)      /* (string ~name, int ~tablesize, int ~tablefactor) */
{
    const char *name = get_xstr(1);
    int elements = get_xinteger(2, TABLESIZE);
    int factor = get_xinteger(3, TABLEFACTOR);
    int dictseq, ret = -1;
    DICT *dict;

    /*
     *  Prime the interface.
     */
    if (NULL == x_dicts) {
        x_dicts = stype_alloc();
        x_dictseq = GRBASE_DICT;
    }

    /*
     *  create dictionary/
     *      Current usage is assumed to be for low volume data sets.
     */
    if (elements && factor > 0) {
        elements /= factor;
    }

    if (name) {
        name = chk_salloc(name);                /* clone name (if any) */
    }

    dictseq = x_dictseq++;

    if (NULL != (dict = chk_calloc(sizeof(DICT),1)) &&
            stbl_open(&dict->d_stable, (unsigned)elements, (unsigned)factor) != -1 &&
                stype_insert(x_dicts, dictseq, dict) != -1) {
        dict->d_magic = DICTMAGIC;
        dict->d_ident = dictseq;
        dict->d_name = name;                    /* optional name */
        dict->d_eachstate = 0;                  /* dict_each/keys/values() state */
        ret = dictseq;                          /* obj_id */
    }

    trace_log("create_dict(%s, %d, %d) : %d\n", (name?name:""), elements, factor, ret);
    acc_assign_int(ret);
}


static DICT *
dict_lookup(int objid)
{
    sentry_t *st;

    if (x_dicts && objid > 0 && (st = stype_lookup(x_dicts, (unsigned) objid)) != NULL)
        if (NULL != st->se_ptr) {
            DICT *dict = (DICT *)st->se_ptr;

            assert(DICTMAGIC == dict->d_magic);
            return dict;
        }
    return NULL;
}


/*<<GRIEF>>
    Macro: delete_dictionary - Destroy a dictionary.

        int
        delete_dictionary(int obj_id)

    Macro Description:
        The 'delete_dictionary()' primitive destroys the specified
        dictionary 'obj_id'.

    Macro Parameters:
        obj_id - Dictionary identifier.

    Macro Returns:
        The 'delete_dictionary()' primitive returns 0 on success,
        otherwise -1 on error.

    Macro Portability:
        A Grief extension.

    Macro See Also:
        create_dictionary
*/
void
do_delete_dictionary(void)      /* int (int obj_id) */
{
    const int objid = get_xinteger(1, -1);
    DICT *dict = dict_lookup(objid);            /* objid lookup */
    int ret = -1;

    if (dict) {
        stable_t *stbl = &dict->d_stable;
        stblcursor_t cursor;
        stblnode_t *node;
                                                /* delete all keys */
        if (NULL != (node = stbl_first(stbl, &cursor))) {
            do {
                obj_free((object_t *)node->stbl_uptr);
                node->stbl_uptr = NULL;
            } while (NULL != (node = stbl_next(&cursor)));
        }

        /*destroy dictionary*/
        stbl_close(stbl);
        stype_delete(x_dicts, (unsigned) objid);
        dict->d_magic = 0;
        chk_free(dict);
        ret = 0;
    }
    trace_log("delete_dictionary(%d) ; %d\n", objid, ret);
    acc_assign_int(ret);
}


/*<<GRIEF>>
    Macro: get_property - Retrieve a dictionary item.

        declare
        get_property(int obj_id, string key)

    Macro Description:
        The 'get_property()' primitive retrieves the value of an item
        from the specified dictionary 'obj_id'.

        This primitive is generally not required within the GRIEF
        Macro Language. When the user accesses an object member, the
        compiler converts the construct into calls to this macro.

	The following are equivalent;

>		get_property(object, "member");
>       and:
>	        object.member;

    Macro Parameters:
        obj_id - Dictionary identifier.
        key - Item key.

    Macro Returns:
        The 'get_property()' primitive returns the item value
        otherwise NULL.

    Macro Portability:
        A Grief extension.

    Macro See Also:
        create_dictionary, set_property, dict_exists, dict_each
*/
void
do_get_property(void)           /* (int obj_id, string key) */
{
    const int objid = get_xinteger(1, -1);      /* objid */
    DICT *dict = dict_lookup(objid);            /* objid lookup */
    const char *key = get_str(2);               /* key */
    int assigned = FALSE;

    trace_log("get_property(%d, %s)\n", objid, key);

    if (dict) {
        stable_t *stbl = &dict->d_stable;
        stblnode_t *node;

        if (NULL != (node = stbl_find(stbl, key))) {
            object_t *obj = (object_t *)node->stbl_uptr;

            if (obj) {
                acc_assign_object(obj);
                assigned = TRUE;
            }
        }
    }

    if (! assigned) {
        acc_assign_null();
    }
}


/*<<GRIEF>>
    Macro: set_property - Set a dictionary item.

        int
        set_property(
            int obj_id, string key, [declare value])

    Macro Description:
        The 'set_property()' primitive sets the value of an item
        within the specified dictionary 'obj_id'.

        This primitive is generally not required within the GRIEF
        Macro Language. When the user accesses an object member, the
        compiler converts the construct into calls to this macro.

	The following are equivalent;

>               set_property(object, "member", value);
>	and:
>		object.member = value;

    Macro Parameters:
        obj_id - Dictionary identifier.
        key - Item key.
        value - Item value.

    Macro Returns:
        The 'set_property()' primitive returns TRUE on success
        otherwise FALSE.

    Macro Portability:
        A Grief extension.

    Macro See Also:
        create_dictionary, get_property, dict_exists, dict_each
*/
void
do_set_property(void)           /* (int obj_id, string key, [declare value]) */
{
    const int objid = get_xinteger(1, -1);      /* objid */
    DICT *dict = dict_lookup(objid);            /* objid lookup */
    const char *key = get_str(2);               /* key */
    const LISTV *lvp = margv + 3;               /* value */

    trace_log("set_property(%d, %s)\n", objid, key);

    if (dict) {
        stable_t *stbl = &dict->d_stable;
        stblnode_t *node;
        object_t *obj;

        if (dict->d_attributes & DICT_READONLY) {
            errorf("cannot assign to read-only dictionary '%s'.", key);
            acc_assign_int(0);
            return;
        }

        /*
         *  Find object
         */
        if (NULL != (node = stbl_find(stbl, key))) {
            /*
             *  Existing property.
             */
            obj = (object_t *)node->stbl_uptr;

        } else {
            /*
             *  Create a new property (if allowed).
             */
            if (dict->d_attributes & DICT_NOCREATE) {
                errorf("cannot create new property '%s'.", key);
                acc_assign_int(0);
                return;
            }
            node = stbl_new(stbl, key);
            obj = obj_alloc();
            node->stbl_uptr = (void *)obj;
        }

        /*
         *  Assign
         */
        if (OBJECT_FREADONLY & obj_attributes(obj)) {
            errorf("cannot assign to read-only property '%s'.", key);
            acc_assign_int(0);

        } else {
            obj_assign_argv(obj, lvp);
            obj_trace(obj);
        }
    }
}


/*<<GRIEF>>
    Macro: dict_exists - Dictionary item existence check.

        int
        dict_exists(
            int obj_id, string key)

    Macro Description:
        The 'dict_exists()' primitive determines the existence of an
        item within the specified dictionary 'obj_id'.

    Macro Parameters:
        obj_id - Dictionary identifier.
        key - Item key.

    Macro Returns:
        The 'dict_exists()' primitive returns TRUE on success
        otherwise FALSE.

    Macro Portability:
        A Grief extension.

    Macro See Also:
        create_dictionary, dict_each, dict_keys, dict_values
*/
void
do_dict_exists(void)            /* (int obj_id, [string key]) */
{
    const int objid = get_xinteger(1, -1);      /* objid */
    DICT *dict = dict_lookup(objid);            /* objid lookup */
    const char *key = get_xstr(2);              /* key */
    int ret = -1;

    if (dict) {
        if (NULL == key) {
            ret = 1;                            /* testing object identifier */

        } else {
            stable_t *stbl = &dict->d_stable;
            stblnode_t *node;

            ret = 0;
            if ((node = stbl_find(stbl, key)) != NULL) {
                ret = 1;                        /* testing property */
            }
        }
    }
    acc_assign_int(ret);
}


/*<<GRIEF>>
    Macro: dict_name - Retrieve a dictionary name.

        string
        dict_name(int obj_id)

    Macro Description:
        The 'dict_name()' primitive retrieves the name associated
        with the specified dictionary 'obj_id'.

    Macro Parameters:
        obj_id - Dictionary identifier.

    Macro Returns:
        The 'dict_name()' primitive returns the name of the
        dictionary otherwise an empty string.

    Macro Portability:
        A Grief extension.

    Macro See Also:
        create_dictionary
*/
void
do_dict_name(void)              /* string (int obj_id) */
{
    const int objid = get_xinteger(1, -1);      /* objid */
    DICT *dict = dict_lookup(objid);            /* objid lookup */
    const char *ret = NULL;

    if (dict) {
        ret = dict->d_name;                     /* string or NULL */
    }
    acc_assign_str(ret?ret:"", -1);
}


/*<<GRIEF>>
    Macro: dict_clear - Clear a dictionary.

        int
        dict_clear(int obj_id)

    Macro Description:
        The 'dict_clear()' primitive removes all items of the
        specified dictionary 'obj_id'.

    Macro Parameters:
        obj_id - Dictionary identifier.

    Macro Returns:
        The 'dict_clear()' primitive returns 0 on success, otherwise
        -1 on error.

    Macro Portability:
        A Grief extension.

    Macro See Also:
        create_dictionary, dict_delete, dict_each, dict_keys, dict_values
*/
void
do_dict_clear(void)             /* int (int obj_id) */
{
    const int objid = get_xinteger(1, -1);      /* objid */
    DICT *dict = dict_lookup(objid);
    int ret = -1;

    if (dict) {
        stable_t *stbl = &dict->d_stable;
        stbl_clear(stbl);
        ret = 0;
    }
    acc_assign_int(ret);
}


/*<<GRIEF>>
    Macro: dict_delete - Remove a dictionary item.

        int
        dict_delete(int obj_id, string key)

    Macro Description:
        The 'dict_delete()' primitive removes the item associated
        with the item 'key' from the specified dictionary 'obj_id'.

    Macro Parameters:
        obj_id - Dictionary identifier.
        key - Item key.

    Macro Returns:
        The 'dict_clear()' primitive returns 0 on success, otherwise
        -1 on error.

    Macro Portability:
        A Grief extension.

    Macro See Also:
        create_dictionary, dict_each, dict_keys, dict_values
*/
void
do_dict_delete(void)            /* int (int obj_id) */
{
    const int objid = get_xinteger(1, -1);      /* objid */
    DICT *dict = dict_lookup(objid);
    const char *key = get_str(2);               /* key */
    int ret = -1;

    if (dict) {
        stable_t *stbl = &dict->d_stable;
        stblnode_t *node;

        if ((node = stbl_find(stbl, key)) != NULL) {
            object_t *obj = node->stbl_uptr;        /* find and delete */

            if (0 == stbl_delete(stbl, key)) {
                obj_free(obj);
                ret = 0;
            }
        }
    }
    acc_assign_int(ret);
}


/*<<GRIEF>>
    Macro: dict_list - Retrieve dictionary items.

        list
        dict_list(int obj_id)

    Macro Description:
        The 'dict_list()' primitive retrieves the keys or values
        contained within the specified dictionary 'obj_id'.

    Macro Parameters:
        obj_id - Dictionary identifier.
        keys - Optional boolean value unless *FALSE* key values are
            retrieved otherwise values are retrieved.

    Macro Returns:
        The 'dict_list()' primitive returns a list of values
        containing all the keys or values within the specified
        dictionary, otherwise a null list.

    Macro Portability:
        A Grief extension.

    Macro See Also:
        create_dictionary
*/
void
do_dict_list(void)              /* list (int obj_id, [int value = FALSE]) */
{
    const int objid = get_xinteger(1, -1);      /* objid */
    const int keys = get_xinteger(2, 1);        /* 1=keys, 0=values */

    DICT *dict = dict_lookup(objid);
    LIST *dictlist = NULL;
    int length = 0;

    if (dict) {
        stable_t *stbl = &dict->d_stable;
        int count = stbl_count(stbl);

        if (count) {
            stblcursor_t cursor;
            stblnode_t *node;

            if (keys) {                         /* item keys */
                length = (count * sizeof_atoms[F_RSTR]) + sizeof_atoms[F_HALT];
                if (NULL != (dictlist = lst_alloc(length, count))) {
                    LIST *lp = dictlist;

                    if (NULL != (node = stbl_first(stbl, &cursor))) {
                        do {
                            lp = atom_push_str(lp, node->stbl_key);
                            --count;
                        } while (NULL != (node = stbl_next(&cursor)));
                    }
                    assert(0 == count);
                    atom_push_halt(lp);
                }

            } else {                            /* item values */
                length = sizeof_atoms[F_HALT];

                if (NULL != (node = stbl_first(stbl, &cursor))) {
                    do {
                        const object_t *obj = (object_t *)node->stbl_uptr;
                        const OPCODE type = obj_get_type(obj);

                        length += sizeof_atoms[type];
                    } while (NULL != (node = stbl_next(&cursor)));
                }

                if (NULL != (dictlist = lst_alloc(length, count))) {
                    LIST *lp = dictlist;

                    if (NULL != (node = stbl_first(stbl, &cursor))) {
                        do {
                            const object_t *obj = (object_t *)node->stbl_uptr;
                            const OPCODE type = obj_get_type(obj);

                            switch(type) {
                            case F_INT:
                                lp = atom_push_int(lp, obj_get_ival(obj));
                                break;
                            case F_FLOAT:
                                lp = atom_push_float(lp, obj_get_fval(obj));
                                break;
                            case F_RLIST:
                            case F_RSTR:
                                lp = atom_push_ref(lp, obj_get_ref(obj));
                                break;
                            case F_STR:
                            case F_LIT:
                                lp = atom_push_str(lp, obj_get_sval(obj));
                                break;
                            case F_NULL:
                                lp = atom_push_halt(lp);
                                break;
                            default:
                                panic("dict_list: what type? (%d)", type);
                                break;
                            }
                            --count;
                        } while (NULL != (node = stbl_next(&cursor)));
                    }
                    assert(0 == count);
                    atom_push_halt(lp);
                }    
            }
        }    
    }

    if (dictlist && length) {
        acc_donate_list(dictlist, length);
    } else {
        acc_assign_null();
    }
}


/*  Function:           do_list_of_dictionaries
 *      list_of_dictionaries primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      none
 *
 *<<GRIEF>>
    Macro: list_of_dictionaries - List of created dictionaries.

        list
        list_of_dictionaries(
            [bool nonempty = false], [bool named = false])

    Macro Description:
        The 'list_of_dictionaries()' primitive retrieves a list of
        dictionary identifiers created by the <create_dictionary>
        primitive.

    Macro Parameters:
        nonempty - Empty diction filter. Optional integer boolean
            flag. If specified and is not-zero, then empty
            directionaries shall be filtered.

        named - Named dictionary filter. Optional integer boolean
            flag. If specified and is not-zero, then unnamed
            directionaries shall be filtered.

    Macro Returns:
        The 'list_of_dictionaries()' primitive retrieves a list of
        dictionaries.

    Macro Portability:
        A Grief extension.

    Macro See Also:
        create_dictionary
*/
void
do_list_of_dictionaries(void)   /* ([bool nonempty = false, bool named = false]) */
{
    const int non_empty = get_xinteger(1, FALSE);
    int named = get_xinteger(2, FALSE);         /* extension */
    LIST *dictlist = NULL;
    int length = 0;

    if (x_dicts && stype_used(x_dicts)) {
        int count = 0;
        stypecursor_t cursor;
        sentry_t *st;

        /* remove empty dictionaries (if any) */
        if (NULL != (st = stype_first(x_dicts, &cursor)))
            do {
                DICT *dict = (DICT *)st->se_ptr;
                stable_t *stbl = &dict->d_stable;

                assert(DICTMAGIC == dict->d_magic);
                if ((! non_empty || stbl_count(stbl)) || (! named || dict->d_name)) {
                    ++count;
                }
            } while (NULL != (st = stype_next(&cursor)));

        /* build list */
        if (count) {
            length = (count * sizeof_atoms[F_INT]) + sizeof_atoms[F_HALT];

            if (NULL != (dictlist = lst_alloc(length, count))) {
                LIST *lp = dictlist;

                if (NULL != (st = stype_first(x_dicts, &cursor)))
                    do {
                        DICT *dict = (DICT *)st->se_ptr;
                        stable_t *stbl = &dict->d_stable;

                        assert(DICTMAGIC == dict->d_magic);
                        if ((! non_empty || stbl_count(stbl)) || (! named || dict->d_name)) {
                            if (! non_empty || stbl_count(stbl)) {
                                lp = atom_push_int(lp, dict->d_ident);
                                --count;
                            }
                        }
                    } while (NULL != (st = stype_next(&cursor)));

                assert(0 == count);
                atom_push_halt(lp);
            }
        }
    }

    if (dictlist && length) {
        acc_donate_list(dictlist, length);
    } else {
        acc_assign_null();
    }
}


/*  Function:           do_dict_each
 *      dict_each primitive
 *
 *<<GRIEF>>
    Macro: dict_each - Iterator a dictionary

        int
        dict_each(int obj_id,
            [string key], [declare value])

    Macro Description:
        The 'dict_each()' primitive iterates thru a dictionary.

        Returns the element index started at one and populates
        'key' and 'value' with the entry details, so one may
        iterate over all elements within the dictionary.

        There is no guarantee of order, entries are returned in an
        apparently random order; driven by the underlying hashing
        value.

        When the dictionary is entirely read, a zero or *FALSE*
        value is returned.

        You must not modify the dictionary whilst iterating over
        it. There is a single iterator for each dictionary, shared
        by <dict_each>, <dict_keys> and <dict_values> primitives, 
        as such care should be taken not to intermix their usage.

        The following prints out your environment like the printenv
        program, only in a different order:

>           int index;
>           while ((index = dict_each(dict, key, value)) >= 0) {
>               insertf("%d:%s=%s\n", index, key, value);
>           }

    Macro Parameters:
        obj_id - Dictionary identifier.
        key - Retrieval key.
        value - Value.

    Macro Returns:
        The primitive returns the positive element index on success, 
        otherwise zero at the end of the dictionary.

    Macro Portability:
        A Grief extension.

    Macro See Also:
        create_dictionary, dict_each, dict_keys, dict_values

 *<<GRIEF>>
    Macro: dict_keys - Iterator dictionary keys.

        int
        dict_keys(int obj_id,
            [string key])

    Macro Description:
        The 'dict_keys()' primitive iterates thru a dictionary.

        Returns the element index started at one and populates
        'key' with the entry details, so one may iterate over all
        elements within the dictionary.

        There is no guarantee of order, entries are returned in an
        apparently random order; driven by the underlying hashing
        value.

        When the dictionary is entirely read, a zero or *FALSE*
        value is returned.

        You must not modify the dictionary whilst iterating over
        it. There is a single iterator for each dictionary, shared
        by <dict_each>, <dict_keys> and <dict_values> primitives,
        as such care should be taken not to intermix their usage.

        The following prints out your environment like the printenv
        program, only in a different order:

>           int index;
>           while ((index = dict_keys(dict, key)) >= 0) {
>               insertf("%d:%s\n", index, key);
>           }

    Macro Parameters:
        obj_id - Dictionary identifier.
        key - Retrieval key.

    Macro Returns:
        The primitive returns the positive element index on success,
        otherwise zero at the end of the dictionary.

    Macro Portability:
        A Grief extension.

    Macro See Also:
        create_dictionary, dict_each, dict_keys, dict_values

 *<<GRIEF>>
    Macro: dict_values - Iterator dictionary values.

        int
        dict_values(int obj_id,
            [declare value])

    Macro Description:
        The 'dict_values()' primitive iterates thru a dictionary.

        Returns the element index started at one and populates
        'value' with the entry details, so one may iterate over all
        elements within the dictionary.

        There is no guarantee of order, entries are returned in an
        apparently random order; driven by the underlying hashing
        value.

        When the dictionary is entirely read, a zero or *FALSE*
        value is returned.

        You must not modify the dictionary whilst iterating over
        it. There is a single iterator for each dictionary, shared
        by <dict_each>, <dict_keys> and <dict_values> primitives,
        as such care should be taken not to intermix their usage.

        The following prints out your environment like the printenv
        program, only in a different order:

>           int index;
>           while ((index = dict_each(dict, value)) >= 0) {
>               insertf("%d=%s\n", index, value);
>           }

    Macro Parameters:
        obj_id - Dictionary identifier.
        value - Value.

    Macro Returns:
        The primitive returns the positive element index on success,
        otherwise zero at the end of the dictionary.

    Macro Portability:
        A Grief extension.

    Macro See Also:
        create_dictionary, dict_each, dict_keys, dict_values
*/
void
do_dict_each(int what)          /* (int obj_id, [string key], [declare value]) */
{
    const int objid = get_xinteger(1, -1);      /* objid */
    DICT *dict = dict_lookup(objid);            /* objid lookup */
    int ret = -1;

    if (dict) {
        stable_t *stbl = &dict->d_stable;

        if (0 == stbl_count(stbl)) {
            dict->d_eachstate = 0;              /* empty */

        } else {
            stblnode_t *node;

            if (0 == dict->d_eachstate) {       /* (re)start */
                node = stbl_first(stbl, &dict->d_eachcursor);
            } else {                            /* next */
                node = stbl_next(&dict->d_eachcursor);
            }

            if (NULL == node) {
                dict->d_eachstate = 0;

            } else {
                SYMBOL *key, *value;

                switch (what) {                 /* mode? */
                case DICT_EACH_PAIR:
                    key = get_symbol(2);
                    value = get_symbol(3);
                    break;
                case DICT_EACH_KEY:
                    key = get_symbol(2);
                    value = NULL;
                    break;
                case DICT_EACH_VALUE:
                    key = NULL;
                    value = get_symbol(2);
                    break;
                default:
                    panic("dict_each(%d): unknown type\n", what);
                    key = value = NULL;
                    break;
                }

                if (key) {
                    sym_assign_str(key, node->stbl_key);
                }

                if (value) {
                    const object_t *obj = (object_t *)node->stbl_uptr;
                    const OPCODE type = obj_get_type(obj);

                    switch(type) {
                    case F_INT:
                        sym_assign_int(value, obj_get_ival(obj));
                        break;
                    case F_FLOAT:
                        sym_assign_float(value, obj_get_fval(obj));
                        break;
                    case F_RLIST:
                    case F_RSTR:
                        sym_assign_ref(value, obj_get_ref(obj));
                        break;
                    case F_STR:
                    case F_LIT:
                        sym_assign_str(value, obj_get_sval(obj));
                        break;
                    case F_NULL:
                        break;
                    default:
                        panic("dict_each: what type? (%d)", type);
                        break;
                    }
                }
                ret = dict->d_eachstate++;      /* index 0.... */
            }
        }
    }
    acc_assign_int(ret);
}
/*end*/
