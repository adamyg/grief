/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: abbrev.cr,v 1.11 2014/10/27 23:28:16 ayoung Exp $
 * Abbreviation mode handling.
 *
 *
 */

#include "grief.h"
#include "mode.h"

#if defined(__PROTOTYPES__)
void                    _griset_abbrev(string arg);
string                  _griget_abbrev(void);
void                    _abbrev_enable(void);
void                    _abbrev_set(int mode);
int                     _abbrev_get(void);
void                    _abbrev_check(void);
#endif

list                    _abbrev_list;           /* Abbreviations currently defined */
string                  _abbrev_init;           /* Init arguments */

extern int              _bvar_abbrev;           /* Buffer abbrev status. */


/*
 *  main ---
 *      Runtime initialisation.
 */
void
main(void)
{
    autoload("objects", "default_word_left");
}


/*
 *  _griset_abbrev, _griget_abbrev ---
 *      Runtime configuration.
 */
void
_griset_abbrev(string arg)
{
    list args;
    int i;

    _abbrev_init = args;
    args = split(arg, " =", 1);
    for (i = 0; i < length_of_list(args); i += 2) {
        if (args[i] == "file") {
            abbrev_load( args[i+1] );
        }
    }
}


string
_griget_abbrev(void)
{
    return (_abbrev_init);
}


/*
 *  abbrev ---
 *      Macro called to define a new abbreviation and to turn on abbreviation mode.
 */
void
abbrev(~string)
{
    string abbr, expr, w;
    int i;

    if (get_parm(0, abbr, "Abbreviation string: ") <= 0 || abbr == "") {
        return;
    }

    /*  If the abbreviation was typed on the command line, and it
     *  consists of more than one word, then these words will
     *  be passed as separate arguments so we need to string them
     *  all back together again
     */
    for (i = 1;; i++) {
        if (get_parm(i, w) <= 0) {
            break;
        }
        expr += w + " ";
    }
    if (i == 1 && get_parm(NULL, expr, "Expansion string: ") <= 0) {
        return;
    }

    /*
     *  See if we already have definition for this entry
     */
    if ((i = re_search(NULL, "<" + quote_regexp(abbr) + ":", _abbrev_list)) < 0) {
        _abbrev_list += abbr + ":" + trim(expr);
    } else {
        _abbrev_list[i] = abbr + ":" + trim(expr);
    }
    _abbrev_enable();
}


/*
 *  abbrev_load ---
 *      Macro to load up a set of abbreviations from a file.
 */
void
abbrev_load(string arg)
{
    int curbuf, buf;
    string s;

    curbuf = inq_buffer();
    if ((buf = create_buffer("-abbrev-", arg, TRUE)) != -1) {
        set_buffer(buf);
        top_of_buffer();
        while (1) {
            if ((s = ltrim(trim(read()))) == "") {
                break;
            }

            /*
             *  Allow for comments and ignore lines without ':' in them
             */
            if (substr(s, 1, 1) != "#" && index(s, ":") > 0) {
                _abbrev_list += s;
            }
            down();
        }
        set_buffer(curbuf);
        delete_buffer(buf);
        _abbrev_enable();
    }
}


/*
 *  _abbrev_enable ---
 *      Enable abbrev support on the current buffer.
 *
 *      Set the buffer variable '_bvar_abbrev' and if template processing
 *      isn't active on the buffer reassign the <Space> key.
 */
void
_abbrev_enable(void)
{
    if (_abbrev_get() == -1) {
        if (_mode_attr_get("template") == "") { /* templates enabled? */
            assign_to_key("<Space>", "_abbrev_check");
        }
        _abbrev_set(1);
    }
}


/*
 *  _abbrev_set ---
 *      Set the current 'abbrev' mode.
 */
void
_abbrev_set(int mode)
{
    local int _bvar_abbrev = mode;

    make_local_variable(_bvar_abbrev);          /* buffer local variable */
}


/*
 *  _abbrev_get ---
 *      Retrieve the current 'abbrev' mode.
 */
int
_abbrev_get(void)
{
    if (inq_symbol("_bvar_abbrev")) {           /* standard abbrev support */
        return _bvar_abbrev;
    }
    return -1;
}


/*
 *  _abbrev_check ---
 *      Macro called when <Space> hit in buffer with abbreviations active.
 *      Check previous word to see if we have a valid abbreviation.
 */
void
_abbrev_check(void)
{
    string word, new_word;
    int i = -1;

    save_position();
    drop_anchor(MK_NORMAL);
    default_word_left();
    word = trim(read(inq_mark_size() - 1));
    if (word != "") {
        word = quote_regexp(word) + ":";
        i = re_search(NULL, word, _abbrev_list);
    }

    if (i < 0) {
        raise_anchor();
        restore_position();
        self_insert(' ');
    } else {
        /*
         *  Delete abbrev without using scrap so we don't corrupt it
         */
        delete_char(inq_mark_size() - 1);
        raise_anchor();
        restore_position(0);
        new_word = substr(_abbrev_list[i], strlen(word) + 1);
        insert(new_word);
    }
}

/*end*/
