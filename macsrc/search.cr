/* -*- indent-width: 4; -*-
 * $Id: search.cr,v 1.11 2014/10/27 23:28:27 ayoung Exp $
 * Search support
 *
 *
 */

#include "grief.h"

#define MK_HILITE       -100

int                     searching_fwd   = TRUE; /* Set to direction of last search */
int                     translating_fwd = TRUE; /* Direction of last translate */
int                     search__wrap    = FALSE;    /* TRUE if search wraps around at end of */
                                                /* buffer, e.g. like vi */
int                     search__regexp  = TRUE;
int                     search__case    = TRUE;
int                     search__block   = FALSE;
int                     search__syntax  = re_syntax();
int                     search__flags;          /* Base flags to pass to re_search() and re_translate(). */

int                     search__hilite  = TRUE;
int                     search__increment = TRUE;
int                     search__timeout = 5;    /* Search hilite result timeout, in seconds */

static int              search_dialog;
static list             search_options_list;

extern string           search__pattern;
extern string           translate__pattern;
extern string           translate__replacement;

#if defined(__PROTOTYPES__)
static void             search_options_gui(void);
static int              search_callback(int ident, string name, int p1, int p2);
static void             search_options_text(void);
static void             setup_search_flags(void);
static void             isearch_hilite(string pat);
#endif


void
main()
{
    setup_search_flags();
}


void
search_options(void)
{
    if ((DC_ASCIIONLY & inq_display_mode())) {
        search_options_text();
    } else {
        search_options_gui();
    }
}


static void
search_options_gui(void)
{
    search_dialog = dialog_create( make_list(
        DLGA_TITLE,                             "Search Options",
        DLGA_CALLBACK,                          "::search_callback",

        DLGC_CONTAINER,
            DLGA_ATTACH_TOP,

            DLGC_CONTAINER,
                DLGA_ATTACH_LEFT,
                DLGA_ALIGN_W,
                DLGA_PADX,                      1,
                DLGC_LABEL,
                    DLGA_VALUE,                 "Regular Expressions:",
                    DLGA_ALIGN_W,
                    DLGA_COLS,                  22,
                DLGC_LABEL,
                    DLGA_VALUE,                 "Syntax mode:",
                    DLGA_ALIGN_W,
                DLGC_LABEL,
                    DLGA_VALUE,                 "Case sensitive:",
                    DLGA_ALIGN_W,
                DLGC_LABEL,
                    DLGA_VALUE,                 "Block selection:",
                    DLGA_ALIGN_W,
            DLGC_END,

            DLGC_CONTAINER,
                DLGA_ATTACH_LEFT,
                DLGA_PADX,                      1,

                DLGC_COMBO_FIELD,
                    DLGA_ATTACH_TOP,
                    DLGA_NAME,                  "regexp",
                    DLGA_COLS,                  16,
                    DLGA_ALIGN_W,
                    DLGA_LBELEMENTS,            quote_list("No", "Yes"),
                    DLGA_LBINDEXMODE,           TRUE,
                    DLGA_LBPAGEMODE,            TRUE,
                    DLGA_CBEDITABLE,            TRUE,
                    DLGA_CBPOPUPMODE,           2,          // Open
                    DLGA_CBAUTOCOMPLETEMODE,    2,          // Append

                DLGC_COMBO_FIELD,
                    DLGA_ATTACH_TOP,
                    DLGA_NAME,                  "syntax",
                    DLGA_COLS,                  16,
                    DLGA_ALIGN_W,
                    DLGA_LBELEMENTS,            quote_list("BRIEF", "Unix", "Extended", "Perl"),
                    DLGA_LBINDEXMODE,           TRUE,
                    DLGA_LBPAGEMODE,            TRUE,
                    DLGA_CBEDITABLE,            TRUE,
                    DLGA_CBPOPUPMODE,           2,          // Open
                    DLGA_CBAUTOCOMPLETEMODE,    2,          // Append

                DLGC_COMBO_FIELD,
                    DLGA_ATTACH_TOP,
                    DLGA_NAME,                  "case",
                    DLGA_COLS,                  16,
                    DLGA_ALIGN_W,
                    DLGA_LBELEMENTS,            quote_list("No", "Yes"),
                    DLGA_LBINDEXMODE,           TRUE,
                    DLGA_LBPAGEMODE,            TRUE,
                    DLGA_CBEDITABLE,            TRUE,
                    DLGA_CBPOPUPMODE,           2,          // Open
                    DLGA_CBAUTOCOMPLETEMODE,    2,          // Append

                DLGC_COMBO_FIELD,
                    DLGA_ATTACH_TOP,
                    DLGA_NAME,                  "block",
                    DLGA_COLS,                  16,
                    DLGA_ALIGN_W,
                    DLGA_LBELEMENTS,            quote_list("Off", "No"),
                    DLGA_LBINDEXMODE,           TRUE,
                    DLGA_LBPAGEMODE,            TRUE,
                    DLGA_CBEDITABLE,            TRUE,
                    DLGA_CBPOPUPMODE,           2,          // Open
                    DLGA_CBAUTOCOMPLETEMODE,    2,          // Append

            DLGC_END,
        DLGC_END,

        DLGC_CONTAINER,
            DLGA_ATTACH_BOTTOM,
            DLGC_PUSH_BUTTON,
                DLGA_ATTACH_LEFT,
                DLGA_LABEL,                     "&Done",
                DLGA_NAME,                      "done",
                DLGA_DEFAULT_BUTTON,
                DLGA_DEFAULT_FOCUS,
            DLGC_PUSH_BUTTON,
                DLGA_ATTACH_LEFT,
                DLGA_LABEL,                     "&Apply",
                DLGA_NAME,                      "apply",
            DLGC_PUSH_BUTTON,
                DLGA_ATTACH_LEFT,
                DLGA_LABEL,                     "&Help",
                DLGA_NAME,                      "help",
        DLGC_END
        ));

    widget_set(search_dialog, "regexp", search__regexp, DLGA_LBACTIVE);
    widget_set(search_dialog, "syntax", search__syntax, DLGA_LBACTIVE);
    widget_set(search_dialog, "case",   search__case,   DLGA_LBACTIVE);
    widget_set(search_dialog, "block",  search__block,  DLGA_LBACTIVE);
    dialog_run(search_dialog);
    dialog_delete(search_dialog);
    search_dialog = 0;
}


static int
search_callback(int ident, string name, int p1, int p2)
{
    UNUSED(ident, p2);
    switch (p1) {
    case DLGE_BUTTON:
        switch(name) {
        case "done":
            dialog_exit();
            break;
        case "apply":
            search__regexp = widget_get(search_dialog, "regexp", DLGA_LBACTIVE);
            search__syntax = widget_get(search_dialog, "syntax", DLGA_LBACTIVE);
            search__case   = widget_get(search_dialog, "case",   DLGA_LBACTIVE);
            search__block  = widget_get(search_dialog, "block",  DLGA_LBACTIVE);
            dialog_exit();
            break;
        case "help":
            execute_macro("explain search_options");
            break;
        }
    }
    return TRUE;
}


static void
search_options_text(void)
{
    list results;

    if (0 == length_of_list(search_options_list)) {
        search_options_list = make_list(
                "Regular Expressions   : ", quote_list("No", "Yes"),
                "Case sensitive        : ", quote_list("No", "Yes"),
                "Block selection       : ", quote_list("Off", "On"),
                "Syntax mode           : ",
                    quote_list("BRIEF", "Unix", "Extended", "Perl")
                );
    }

    results[0] = search__regexp;
    results[1] = search__case;
    results[2] = search__block;
    results[3] = search__syntax;
    results = field_list("Search Parameters", results, search_options_list, TRUE, TRUE);
    if (length_of_list(results) <= 0) {
        return;
    }
    search__regexp = results[0];
    search__case   = results[1];
    search__block  = results[2];
    search__syntax = results[3];
    setup_search_flags();
}


string
_griget_case_sensitive(void)
{
    return search__case ? "yes" : "no";
}


void
_griset_case_sensitive(string arg)
{
    if ("yes" == arg) {
        search__case = 1;
    } else {
        search__case = 0;
    }
}


static void
setup_search_flags(void)
{
    search__flags = 0;
    if (0 == search__regexp) {
        search__flags |= SF_NOT_REGEXP;
    }
    if (0 == search__case) {
        search__flags |= SF_IGNORE_CASE;
    }
    if (search__block) {
        search__flags |= SF_BLOCK;
    }
    if (search__syntax) {
        search__flags |= SF_UNIX;
    }
    re_syntax(search__syntax);
}



/*
 *  Macro to hilite a group of characters until a key is pressed.
 *  Used by search-fwd and search-back macros.
 *
 *  Notes:
        If search is successful, hilite the matched string but only if the matched
        string len is at least 2 chars wide, otherwise we have real problems on a mono
        screen.
 */
int
search_hilite(int match_len)
{
    if (match_len > 2) {
        hilite_destroy(NULL, MK_HILITE);

        if (search__timeout > 0) {              // create temporaray hilite region
            hilite_create(NULL, MK_HILITE,
                search__timeout, NULL, current_col, NULL, current_col + (match_len-2), "search");
            raise_anchor();

        } else {                                // old school, blocking
            int ch;

            next_char(match_len - 1);
            drop_anchor(MK_NONINC);
            prev_char(match_len - 1);
            refresh();
            while ((ch = read_char()) == -1) {
                ;
            }
            push_back(ch);
            raise_anchor();
        }
    }
    return match_len;
}


void
translate__fwd(void)
{
    int old_msg_level;

    translating_fwd = TRUE;
    if (get_parm(NULL, translate__pattern, "Translate: ", NULL, translate__pattern) <= 0)
        return;
    if (translate__pattern == "")
        return;
    if (get_parm(NULL, translate__replacement, "Replacement: ", NULL, translate__replacement) <= 0)
        return;
    old_msg_level = set_msg_level(0);
    re_translate(search__flags | SF_PROMPT, translate__pattern, translate__replacement);
    set_msg_level(old_msg_level);
}


void
translate__back(void)
{
    int old_msg_level;

    translating_fwd = FALSE;
    if (get_parm(NULL, translate__pattern, "Translate back: ", NULL, translate__pattern) <= 0)
        return;
    if (translate__pattern == "")
        return;
    if (get_parm(NULL, translate__replacement, "Replacement: ", NULL, translate__replacement) <= 0)
        return;
    old_msg_level = set_msg_level(0);
    re_translate(search__flags | SF_PROMPT | SF_BACKWARDS, translate__pattern,
            translate__replacement);
    set_msg_level(old_msg_level);
}


/*
 *  Function to apply a translate again.
 */
void
translate_again(void)
{
    int old_msg_level;

    if ("" == translate__pattern) {
        error("No previous translate pattern.");
        return;
    }
    old_msg_level = set_msg_level(0);
    re_translate(search__flags | SF_PROMPT | (translating_fwd ? 0 : SF_BACKWARDS),
            translate__pattern, translate__replacement);
    set_msg_level(old_msg_level);
}


void
search__fwd(void)
{
    int old_msg_level, match_len;

    if (get_parm(NULL, search__pattern, "Search fwd: ", NULL, search__pattern) <= 0) {
        return;
    }
    searching_fwd = TRUE;
    old_msg_level = set_msg_level(0);
    match_len = re_search(search__flags, search__pattern);
    set_msg_level(old_msg_level);
    search_hilite(match_len);
}


void
search__back(void)
{
    int old_msg_level, match_len;

    if (get_parm(NULL, search__pattern, "Search back: ", NULL, search__pattern) <= 0) {
        return;
    }
    searching_fwd = FALSE;
    old_msg_level = set_msg_level(0);
    match_len = re_search(SF_BACKWARDS | search__flags, search__pattern);
    set_msg_level(old_msg_level);
    search_hilite(match_len);
}


void
search_next(void)
{
    int old_msg_level, match_len;

    if (search__pattern == "") {
        error("No previous search string.");
        return;
    }

    save_position();
    next_char(searching_fwd ? 1 : -1);
    old_msg_level = set_msg_level(0);

    if (searching_fwd) {
        match_len = re_search(search__flags, search__pattern);
    } else {
        match_len = re_search(SF_BACKWARDS | search__flags, search__pattern);
    }

    if (match_len <= 0) {
        restore_position();
    } else {
        restore_position(0);
    }

    set_msg_level(old_msg_level);
    search_hilite(match_len);
}


void
search_prev(void)
{
    int old_msg_level, match_len;

    save_position();
    prev_char();
    old_msg_level = set_msg_level(0);

    match_len = re_search(SF_BACKWARDS | search__flags, search__pattern);
    if (match_len <= 0) {
        restore_position();
    } else {
        restore_position(0);
    }

    set_msg_level(old_msg_level);
    search_hilite(match_len);
}


/*
 *  Function callable to toggle or set the regular expression character.
 */
void
toggle_re(void)
{
    search__regexp = !search__regexp;
    message("Regular expressions %s.", search__regexp ? "on" : "off");
    setup_search_flags();
}


/*
 *  Toggle whether searches are case sensitive or not.
 */
void
toggle_re_case(void)
{
    search__case = !search__case;
    message("Case sensitivity %s.", search__case ? "on" : "off");
    setup_search_flags();
}


/*  Function:           isearch
        Intelligent search macro. Searches for text as user types it
        in. Needs to handle command line editing on its own, so its
        very simple.

        <Backspace> can be used to delete characters that were typed
        in and backup the search. Hit <Enter> to accept the entry or
        <Alt-S> to go to next entry. Hit <Esc> to abort the search.
 */
void
isearch(void)
{
    int ch, aborted = FALSE, save_level = 1;
    string pat, mac;

    save_position();
    while (1) {
        refresh();
        message("I-search for: %s..", pat);
        ch = read_char();

        if (key_to_int("<Enter>") == ch) {      /* completion */
            message("");
            break;
        }

        if (key_to_int("<Esc>") == ch) {        /* abort */
            message("Search aborted.");
            aborted = TRUE;
            break;
        }

        if (key_to_int("<Backspace>") == ch) {
            if (save_level > 1 && strlen(pat) < save_level) {
                restore_position();             /* remove <next> */
                --save_level;
            } else {
                pat = substr(pat, 1, strlen(pat)-1);
                if (save_level > 1) {
                    restore_position();
                    if (--save_level == 0) {
                        save_position();
                        save_level = 1;
                    }
                }
                isearch_hilite(pat);
            }
            continue;
        }

        if (key_to_int("<Ctrl-N>") == ch ||
                key_to_int("<Next>") == ch) {   /* <next> */
            save_position();
            next_char();
            if (re_search(search__flags | SF_NOT_REGEXP, pat) <= 0) {
                restore_position();
                beep();
            } else {
                restore_position(0);
                isearch_hilite(pat);
            }
            continue;
        }

        if (key_to_int("<Ctrl-P>") == ch ||
                key_to_int("<Prev>") == ch) {   /* <previous> */
            save_position();
            prev_char();
            if (re_search(search__flags | SF_BACKWARDS | SF_NOT_REGEXP, pat) <= 0) {
                restore_position();
                beep();
            } else {
                restore_position(0);
                isearch_hilite(pat);
            }
            continue;
        }

        mac = inq_assignment(ch);
        if (substr(mac, 1, 6) == "search") {    /* <search>/<Alt-S> */
            ++save_level;
            save_position();
            next_char();
            if (re_search(search__flags | SF_NOT_REGEXP, pat) <= 0) {
                restore_position();
                --save_level;
                beep();
            }
            continue;
        }

        if ("undo" == mac) {                    /* <undo> */
            if (save_level > 1) {
                restore_position();
                if (--save_level == 0) {
                    save_position();
                    save_level = 1;
                }
            }
            continue;
        }

        sprintf(pat, "%s%c", pat, ch);          /* add character to pattern. */

        ++save_level;
        save_position();                        /* find next or undo */
        if (re_search(search__flags | SF_NOT_REGEXP, pat) <= 0) {
            pat = substr(pat, 1, strlen(pat) - 1);
            restore_position();
            --save_level;
            beep();
            continue;
        }

        isearch_hilite(pat);
    }

    while (save_level-- > 0) {
        restore_position(0);
    }
    hilite_destroy(NULL, MK_HILITE);
    refresh();
}


static void
isearch_hilite(string pat)
{
    int len, count = 0;

    hilite_destroy(NULL, MK_HILITE);

    if (0 == (len = strlen(pat)))
        return;
    --len;

    save_position();
    do {
        hilite_create(NULL, MK_HILITE, search__timeout, NULL, current_col, NULL, current_col + len, "search_inc");
        right();
    } while (count++ < 15 && re_search(search__flags | SF_NOT_REGEXP, pat) > 0);
    restore_position();

    count = 0;
    save_position();
    left();
    while (count++ < 15 && re_search(search__flags | SF_BACKWARDS | SF_NOT_REGEXP, pat) > 0) {
        hilite_create(NULL, MK_HILITE, search__timeout, NULL, current_col, NULL, current_col + len, "search_inc");
        left();
    }
    restore_position();
}

/*end*/
