/* -*- mode: cr; indent-width: 4; -*- */
/*
 *  Determine the console type, and initialise the terminal characteristics.
 *
 *  Firstly utilise GRTERM otherwise system TERM.
 *  If "tty/$TERM" exists load, otherwise, default to "tty/tty".
 *
 *  If the GRTERM environment variables ate of the form:
 *
 *      type-type1-type2,
 */

#include "tty.h"

#if defined(__PROTOTYPES__)
static string iscolor(string);
extern void ansi_arrows(void);
#endif

void
console_detect()
{
    string btermspec, termspec, term, name;
    list btermpts, termpts;

    /*
     *  GRTERM/TERM
     */
    btermspec = lower(getenv("GRTERM"));
    if (strlen(btermspec)) {
        btermpts = split(btermspec, "-");
        if (length_of_list(btermpts)) {             /* GRTERM primary */
           term = btermpts[0];
        }
    }

    termspec = lower(getenv("TERM"));
    if (strlen(termspec)) {
        termpts = split(termspec, "-");
        if (term == "") {                           /* TERM secondary */
            if (length_of_list(termpts)) {
                term = termpts[0];
            }
        }
    }

    if (0 == load_macro("tty/" + term)) {           /* xterm etc */
        load_macro("tty/tty");
        term = "tty";

    } else { /* retrieve derived/detected name */
        if (0 == get_term_feature(TF_NAME, name)) {
            name = re_translate(SF_GLOBAL, "[ -]+", "_", name);
        }
    }

    /*
     *  Color attribute.
     */
    const string colorterm = getenv("COLORTERM");
    string colorname, var, fn;

    if (colorterm == "truecolor" || colorterm == "24bit") {
        colorname = "256color";

    } else {
        int i;

        for (i = 1; (i < length_of_list(btermpts)) && (colorname == ""); i++) {
            colorname = iscolor(btermpts[i]);
        }

        for (i = 1; (i < length_of_list(termpts)) && (colorname == ""); i++) {
            colorname = iscolor(termpts[i]);
        }
    }

    if (strlen(colorname)) {
        fn = name + "_" + colorname;                /* xterm_gnome_xxx */
        if (inq_macro(fn) > 0) {
            execute_macro(fn);

        } else if (term != name) {
            fn = term + "_" + colorname;            /* xterm_xxx */
            if (inq_macro(fn) > 0) {
                execute_macro(fn);
            }
        }
    }

    /*
     *  Additional attributes, example "GRTERM=xterm-arrows"
     *
     *  Note:
     *  tty/xterm_xxx macros contain a void xxx macro, as such term_xxx calls shall have no effect.
     */
    while (list_each(btermpts, var) >= 0) {
        if (var == term || iscolor(var) != "")
            continue;

        fn = name + "_" + var;                      /* xterm_gnome_xxx */
        if (inq_macro(fn, 0x01) > 0) {
            execute_macro(fn);
            continue;
        }

        if (term != name) {
            fn = term + "_" + var;                  /* xterm_xxx */
            if (inq_macro(fn, 0x01) > 0) {
                execute_macro(fn);
                continue;
            }
        }

        if (var == "ansi_arrows" > 0) {             /* global options, restricted set */
            ansi_arrows();
        }
    }
}


/*
 *  iscolor ---
 *      Determine whether a colorname attribute.
 */
static string
iscolor(string name)
{
    if (name == "truecolor") {
        return "256color";

    } else if (name == "24bit" || name == "24bits" /*emac*/) {
        return "256color";

    } else if (name == "256color" || name == "256" /*legacy*/) {
        return "256color";

    } else if (name == "88color" || name == "88" /*legacy*/) {
        return "88color";

    } else if (name == "color" || name == "16color" || name == "16" /*legacy*/) {
        return "color";

    } else if (name == "mono" || name == "m") {
        return "mono";
    }

    return "";  /*not-color*/
}


void
console()
{
    /*NOTHING*/
}


/*
 *  ansi_arrows ---
 *      Following macro used for setting arrow keys to standard ANSI sequences.
 */
void
ansi_arrows(void)
{
    set_term_keyboard(
        KEY_UP,     "\x1b[A",   KEY_DOWN,   "\x1b[B",
        KEY_LEFT,   "\x1b[D",   KEY_RIGHT,  "\x1b[C");
}

/*end*/
