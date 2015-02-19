/* -*- indent-width: 4; -*- */
/* $Id: uspell.cr,v 1.8 2014/10/27 23:28:29 ayoung Exp $
 * Unix spell interface.
 *
 *  Macro to perform spell on the current buffer, and then allow user to
 *  move from word to word.  The 'uspell' interface allows both a command-line (old)
 *  and GUI (new) style interfaces.  The spell engine used is the standard
 *  of the target host.
 *
 *
 */

#include "grief.h"

static int              uspell_mode,        /* interface mode 0=command, 1=gui */
                        uspell_buf = -1,    /* result buffer */
                        uspell_line,        /* current line number */
                        uspell_dialog;      /* dialog resource */

static list             uspell_seen;        /* word alright processed */

#define DICT_NAME       ".grdict"           /* default dictionary name */

#if defined(sun) || defined(_AIX) || defined(linux)
#define SPELL_COMMANDP  "spell -l -x +%s <%s", mydict_name, tmp_file
#define SPELL_COMMAND   "spell -l -x <%s", tmp_file
#define IS_SPELL

#elif defined(MSDOS) || defined(OS2)        /* FIXME */
#define SPELL_COMMANDP  "C:\\Program Files\\Aspell\\bin\\aspell.exe -p %s -a < %s", mydict_name, tmp_file
#define SPELL_COMMAND   "\"C:\\Program Files\\Aspell\\bin\\aspell.exe\" -a < %s", tmp_file
#define IS_ASPELL

#else
#define SPELL_COMMANDP  "ispell -C -l -p %s <%s", mydict_name, tmp_file
#define SPELL_COMMAND   "ispell -C -l <%s", tmp_file
#define IS_ISPELL
#endif

#if defined(__PROTOTYPES__)
static void             uspell_run(void);
static void             uspell_next(void);
static void             uspell_prev(void);
static int              uspell_display(int line, int forward);
static int              uspell_sort(/*const*/ declare a, /*const*/ declare b);
static int              uspell_find(string word);
static int              uspell_command(void);
static int              uspell_gui(void);
static int              uspell_create(void);
static int              uspell_callback(int ident, string name, int p1, int p2);
#endif


void
uspell(void)
{
    uspell_mode = 1;
    uspell_run();
}


void
uspellcmd(void)
{
    uspell_mode = 0;
    uspell_run();
}


static void
uspell_run(void)
{
    string tmp_file, buf, mydict_name;
    int curbuf = inq_buffer();
    int mydict_buffer = -1;
    int lines;

    UNUSED(mydict_buffer);

    /* mark the entire buffer */
    save_position();
    end_of_buffer();
    drop_anchor(MK_LINE);
    end_anchor(1);
    restore_position();

    sprintf(tmp_file, "%s/sp%06d.cr", inq_tmpdir(), getpid());
    write_block(tmp_file);
    tmp_file = fixslash(tmp_file);

    /* If the user has a private dictionary, then use it.
     * The private dictionary is either specified by the BDICT
     * env variable or we default to $HOME / mydict_name.
     */
    mydict_name = getenv("BDICT");
    if (mydict_name == "") {
        sprintf(mydict_name, "%s/%s", inq_home(), DICT_NAME);
        if (! exist(mydict_name)) {
            mydict_name = "";                   /* doesnt exist :( */
        }
    }

    if (mydict_name != "") {
        sprintf(buf, SPELL_COMMANDP);
    } else {
        sprintf(buf, SPELL_COMMAND);
    }

    message("Checking for mis-spelled words...");
    if (uspell_buf >= 0) {
        delete_buffer(uspell_buf);
    }
    uspell_buf = perform_command(buf, "uspell-buffer" /*, -1, 0*/);
    remove(tmp_file);

    set_buffer(uspell_buf);
    end_of_buffer();
    insert("\n");
    top_of_buffer();
    lines = inq_lines();
    set_buffer(curbuf);

    if (lines <= 1) {
        delete_buffer(uspell_buf);
        uspell_buf = -1;
        error("No spelling mistakes.");
        return;
    }

    uspell_line = 0;
    uspell_seen = NULL;
    assign_to_key("<Ctrl-N>", "::uspell_next");
    assign_to_key("<Ctrl-P>", "::uspell_prev");
    uspell_next();
}


static void
uspell_next(void)
{
    if (uspell_buf >= 0) {
        save_position();
        while (uspell_display(uspell_line + 1, TRUE) > 0) {
            continue;
        }
        restore_position();
    }
}


static void
uspell_prev(void)
{
    if (uspell_buf >= 0) {
        save_position();
        while (uspell_display(uspell_line - 1, FALSE) > 0) {
            continue;
        }
        restore_position();
    }
}


/* macro to display next incorrect word on screen and allow
 * user to type in a correction.
 * macro returns:-1 if an error occurs;
 *
 *      0 user doesnt want to correct word
 *      1 user corrected word.
 */
static int
uspell_display(int line, int forward)
{
    string word, new_word, buf;
    list suggestions;
    int curbuf;
    int ret;

    UNUSED(word, new_word, buf);

    curbuf = inq_buffer();

    /*  Read next word from list.
     *
     *  Two file structure are supported:
     *
     *      aspell:
     *              & word line ??: suggestions [, suggestions...]
     *
     *      spell:  (sun, aix and others)
     *          [=suggestion]
     *              :
     *          word
     */
    set_buffer(uspell_buf);
    if (line <= 0) {
        line = 1;
    }
    goto_line(line);

    while (1) {                                 /* word */
        if (line < 0 || line > inq_lines()) {
            set_buffer(curbuf);
            uspell_line = line;
            message("No further words to correct.");
            return -1;
        }
        goto_line(line);
        word = trim(compress(read()));

        if (strlen(word) > 0) {
#if defined(IS_ASPELL)
            if (substr(word, 1, 1) == "&") {    /* bust line, "& word line ??: suggestions, [suggestions]" */
                list aspell;
                int idx = 5;

                aspell = split(word, " ,");
                word = aspell[1];
                                                /* aspell report each occurence */
                if (search_list(NULL, word, uspell_seen) == -1) {
                    for (idx = 5; idx < length_of_list(aspell); ++idx) {
                        push(suggestions, aspell[idx]);
                    }
                    push(uspell_seen, word);
                    break;
                }
            }

#else   /*ISPELL || SPELL*/
            /* suggestions (ie =xxxxx) */
            if (substr(word, 1, 1) != "=") {    /* if walking backward, search for suggestions (if any) */
                if (!forward) {
                    int t_line = line;
                    string t_word;

                    while (t_line-- > 1) {
                        goto_line(t_line);
                        t_word = trim(compress(read()));
                        if (t_word == "" || substr(t_word, 1, 1) != "=") {
                            break;
                        }
                        push(suggestions, substr(t_word, 2));
                    }
                }
                break;
            }

            if (forward) {                      /* if walking forward, store suggestions */
                push(suggestions, substr(word, 2));
            }
#endif
        }
        line = (forward ? line+1 : line-1);     /* next line */
    }

    uspell_line = line;
    set_buffer(curbuf);

    /* Apply rules, skipping */
    if (strlen(word) < 2) {                     /* ... length < 2 */
        return 1;
    }

    if (strtol(word, ret) != 0 || ret > 1) {    /* ... number (hex, dec or oct) */
        return 1;
    }

    /* Cook suggestions (if any) */
    suggestions = sort_list(suggestions, "::uspell_sort");

    top_of_buffer();
    if (! uspell_find(word)) {
        return 1;                               /* skipped, try next word */
    }

    if (uspell_mode) {
        ret = uspell_gui();
    } else {
        ret = uspell_command();
    }

    return ret;
}


static int
uspell_sort(/*const*/ declare a, /*const*/ declare b)
{
    extern string word;

    return (spell_distance(a, word) <=> spell_distance(b, word));
}


static int /*bool*/
uspell_find(string word)
{
    string line;
    int len, col;

    len = strlen(word);
    do {
        if (re_search(SF_NOT_REGEXP, word) <= 0) {
            return 0;                           /* not found */
        }

        inq_position(NULL, col);
        if (col > 1) {
            left();
            line = read(len + 2);
            right();
            if (!iscsym(line, 1) && !iscsym(line, len + 2)) {
                break;                          /* stop substring */
            }

        } else {
            line = read(len+1);
            if (!iscsym(line, len + 1)) {
                break;                          /* stop substring */
            }
        }
        right(len);                             /* skip word */

    } while (1);

    drop_anchor(MK_NORMAL);
    right(len);
 /* set_top_of_window(); */
    set_bottom_of_window();
    refresh();
    return 1;
}


static int
uspell_command(void)
{
    extern string word;
    string buf, new_word;

    sprintf(buf, "Correct %s:", word);
    if (get_parm(NULL, new_word, buf, NULL, word) <= 0 || new_word == word) {
        left(strlen(word));
        raise_anchor();
        message("<Ctrl-N>/<Ctrl-P> checks next or previous word.");
        return -1;
    }
    raise_anchor();
    translate(word, new_word, NULL, 0, 1);
    return 0;
}


static int
uspell_gui(void)
{
    extern string word;
    int ret;

    message("Correct %s ...", word);
    uspell_create();
    ret = dialog_run(uspell_dialog);
    dialog_delete(uspell_dialog);
    uspell_dialog = 0;
    if (ret <= 0) {
        raise_anchor();
    }
    return ret;
}


static int
uspell_create(void)
{
    extern list suggestions;

    if (uspell_dialog) {
        dialog_delete(uspell_dialog);
    }

    uspell_dialog =
        dialog_create( make_list(
            DLGA_TITLE,                         "Unix Spell",
            DLGA_CALLBACK,                      "::uspell_callback",

            DLGC_CONTAINER,
                DLGA_ATTACH_LEFT,
                DLGC_CONTAINER,
                    DLGA_ATTACH_LEFT,
                    DLGA_ALIGN_NW,
                    DLGC_LABEL,
                        DLGA_LABEL,             "Word",
                        DLGA_ALIGN_W,
                    DLGC_LABEL,
                        DLGA_LABEL,             "Replace",
                        DLGA_ALIGN_W,
                    DLGC_LABEL,
                        DLGA_LABEL,             "Suggestions",
                        DLGA_COLS,              12,
                        DLGA_ALIGN_W,
                DLGC_END,

                DLGC_CONTAINER,
                    DLGA_ATTACH_RIGHT,
                    DLGA_ALIGN_NE,
                    DLGC_EDIT_FIELD,
                        DLGA_ALIGN_E,
                        DLGA_NAME,              "word",
                        DLGA_ROWS,              1,
                        DLGA_GREYED,
                        DLGA_ALLOW_FILLX,
                    DLGC_EDIT_FIELD,
                        DLGA_ALIGN_E,
                        DLGA_NAME,              "replace",
                        DLGA_ROWS,              1,
                        DLGA_ALLOW_FILLX,
                    DLGC_LIST_BOX,
                        DLGA_ALIGN_E,
                        DLGA_NAME,              "suggestions",
                        DLGA_ROWS,              11,
                        DLGA_COLS,              30,
                        DLGA_LBELEMENTS,        suggestions,
                        DLGA_LBINDEXMODE,       TRUE,
                        DLGA_LBPAGEMODE,        TRUE,
                DLGC_END,
            DLGC_END,

            DLGC_CONTAINER,
                DLGA_ATTACH_RIGHT,
                DLGC_PUSH_BUTTON,
                    DLGA_LABEL,                 "&Ignore",
                    DLGA_NAME,                  "ignore",
                    DLGA_ALLOW_FILLX,
                DLGC_PUSH_BUTTON,
                    DLGA_LABEL,                 "I&gnore all",
                    DLGA_NAME,                  "ignoreall",
                    DLGA_ALLOW_FILLX,
                DLGC_PUSH_BUTTON,
                    DLGA_LABEL,                 "Ignore &add",
                    DLGA_NAME,                  "add",
                    DLGA_ALLOW_FILLX,
                DLGC_PUSH_BUTTON,
                    DLGA_LABEL,                 "&Change",
                    DLGA_NAME,                  "change",
                    DLGA_ALLOW_FILLX,
                    DLGA_DEFAULT_BUTTON,
                DLGC_PUSH_BUTTON,
                    DLGA_LABEL,                 "Change A&ll",
                    DLGA_NAME,                  "changeall",
                    DLGA_ALLOW_FILLX,
                DLGC_PUSH_BUTTON,
                    DLGA_LABEL,                 "&Done",
                    DLGA_NAME,                  "done",
                    DLGA_ALLOW_FILLX,
                    DLGA_CANCEL_BUTTON,
                DLGC_PUSH_BUTTON,
                    DLGA_LABEL,                 "&Help",
                    DLGA_NAME,                  "help",
                    DLGA_ALLOW_FILLX,
            DLGC_END
            ));

    return uspell_dialog;
}


static int
uspell_callback(int ident, string name, int p1, int p2)
{
    extern string word;
    extern list suggestions;
    extern int mydict_buffer;
    extern string mydict_name;
    string new_word;

    UNUSED(ident, p2);

//  message("ident=%d,name=%s,p1=%d,p2=%d", ident, name, p1, p2);
    switch (p1) {
    case DLGE_INIT:
        widget_set(NULL, "word", word);
        widget_set(NULL, "replace", length_of_list(suggestions) ? suggestions[0] : "");
        widget_set(NULL, "suggestions", suggestions);
        break;

    case DLGE_CHANGE:
        switch (name) {
        case "replace":
            new_word = widget_get(NULL, "replace");

            raise_anchor();
//          left(strlen(word));
//          if (new_word != word) {
//              translate(word, new_word, /*one*/0, /re*/0, /*case*/1);
//          }
            message("value '%s' => '%s'", word, new_word);
            if (!uspell_find(word)) {
                dialog_exit(1);
            }
            break;

        case "suggestions":
//          new_word = suggestions[p2];
            new_word = widget_get(NULL, "suggestions");
            widget_set(NULL, "replace", new_word);
            break;
        }
        break;

    case DLGE_BUTTON:
        switch(name) {
        case "ignore":
            raise_anchor();
            if (!uspell_find(word)) {
                dialog_exit(1);
            }
            break;

        case "ignoreall":
            raise_anchor();
            dialog_exit(1);
            break;

        case "add":
            raise_anchor();
            if (-1 == mydict_buffer) {
                if ("" == mydict_name) {
                    sprintf(mydict_name, "%s/%s", inq_home(), DICT_NAME);
                }
                if (exist(mydict_name)) {
                    mydict_buffer = create_buffer("uspell-mydict", mydict_name, TRUE);
                } else {
                    mydict_buffer = create_buffer("uspell-mydict", NULL, TRUE);
                }
            }

            if (mydict_buffer >= 0) {
                set_buffer(mydict_buffer);
                end_of_buffer();
                insert(word + "\n");
                write_buffer(mydict_name);
            }
            dialog_exit(1);
            break;

        case "change":
            new_word = widget_get(NULL, "replace");

            raise_anchor();
//          left(strlen(word));
//          if (new_word != word) {
//              translate(word, new_word, /*one*/0, /re*/0, /*case*/1);
//          }
            message("change '%s' => '%s'", word, new_word);
            if (!uspell_find(word)) {
                dialog_exit(1);
            }
            break;

        case "changeall":
            new_word = widget_get(NULL, "replace");

            raise_anchor();
//          left(strlen(word));
//          if (new_word != word) {
//              translate(word, new_word, /*all*/1, /*re*/0, /*case*/1);
//          }
            message("changeall '%s' =>'%s'", word, new_word);
            dialog_exit(1);
            break;

        case "done":
            dialog_exit(-1);
            break;

        case "help":
            execute_macro("explain uspell");
            break;
        }
        break;
    }
    return TRUE;
}

/*end*/
