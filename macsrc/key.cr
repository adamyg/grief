/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: key.cr,v 1.27 2024/11/24 14:38:04 cvsuser Exp $
 * Key definition tools.
 *
 *
 */

#include "grief.h"
#include "mode.h"
#include "alt.h"

#define KEYS_COLUMNS        5

static void             _key_learn_print(list def, string prefix, int quote);
static list             _key_learn_aa(int alpha);
static list             _key_learn_kp(~string);
static list             _key_learn_sp(string what);

static void             tohex(string seq);

#define IsMouse(_k)     (((_k) & RANGE_MASK) == RANGE_BUTTON)

static string           x_lastkey = "";
static int              x_lastmatch = 0;

static list             x_keys_2_learn =
    {
#define KEYS_ELEM           2
#define KEYS_LABEL          0
#define KEYS_COMMAND        1

        "F1_F12",           "fn",
        "SHIFT_F1_F12",     "fn Shift",
        "CTRL_F1_F12",      "fn ctrl",
        "CTRLSHIFT_F1_F12", "fn ctrlshift",
        "ALT_F1_F12",       "fn Alt",
        "ALT_A_Z",          "aa 1",             /* upper case */
        "ALT_A_Z",          "aa 2",             /* lower case */
        "ALT_0_9",          "aa",
        "KEYPAD_0_9",       "kp",
        "SHIFT_KEYPAD_0_9", "kp shift",
        "CTRL_KEYPAD_0_9",  "kp ctrl",
        "ALT_KEYPAD_0_9",   "kp alt",
        "BACK_TAB",         "sp <Shift-Tab>",
    };

static list             x_keys_kp_names =
    {
        "Ins",              /* 0  */
        "End",              /* 1  */
        "Down",             /* 2  */
        "PgDn",             /* 3  */
        "Left",             /* 4  */
        "5",                /* 5  */
        "Right",            /* 6  */
        "Home",             /* 7  */
        "Up",               /* 8  */
        "PgUp",             /* 9  */
        "Del",              /* 10 */
        "Plus",             /* 11 */
        "Minus",            /* 12 */
        "Star",             /* 13 */
        "Divide",           /* 14 */
        "Equals",           /* 15 */
        "Enter",            /* 16 */
        "Pause",            /* 17 */
        "PrtSc",            /* 18 */
        "Scroll",           /* 19 */
        "NumLock"           /* 20 */
    };


/*  Function:           key
 *      Retrieve the "key-code" for the next typed key.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      none
 */
void
key(void)
{
    string buf, kseq = "", rseq = "";
    int ch, keycode = 0;

    /*
     *  keycode ...
     */
    while (1) {
        message("key: %s", kseq);
        if ((ch = read_char(kseq == "" ? 0 : 250, TRUE)) < 0) {
           break;
        }

        if (ch & ~0xff) {                       // function key?
            keycode = ch;
            break;
        }

        sprintf(buf, "%c", ch);                 // raw sequence
        rseq += buf;

        if (ch < ' ') {                         // printable version
            sprintf(buf, "\\x%02x", ch);
        } else {
            sprintf(buf, "%c", ch);
        }
        kseq += buf;
    }

    /*
     *  apply action to key ...
     */
    message("<Ins> to insert or <Enter> for help: ");
    ch = read_char();
    if (ch == key_to_int("<Ins>")) {
        if (keycode) {
            insert("\"" + int_to_key(keycode) + "\"");
        } else {
            insert("\"" + kseq + "\"");
        }
        return;
    }

    if (ch == key_to_int("<Esc>")) {
        return;
    }

    if (keycode) {
        explain(inq_assignment(int_to_key(keycode)));
    } else {
        explain(inq_assignment(key_to_int(rseq, 1)));
    }
}


/*  Function:           key_code
 *      Retrieve the "key-code" for the next input event, including mouse.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      none
 */
void
key_code()
{
    string description;
    int seq = 7;

    while (--seq > 0) {
        message("key_code (%d): %s", seq, description);
        int key = read_char(1000, -1);          /* next event */
        if (key >= 0) {
            if (IsMouse(key)) {
                int x, y;
                get_mouse_pos(x, y);
                sprintf(description, "keyx=%s (%d,%d)", int_to_key(key), x, y);

            } else {
                sprintf(description, "keyx=" + int_to_key(key));
            }
            seq = 7;

        } else {
            description = "";
        }
    }
    message("");
}


/*  Function:           key_test
 *      Test the current keyboard mapping.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      none
 */
void
key_test(void)
{
    list   lst;
    string ascii, tmp;
    int    llen, l;
    int    ch, t;

    lst = get_term_keyboard();                  /* retrieve list */
    llen = length_of_list(lst);

    t = 6;                                      /* idle timeout, initial */
    while (1) {
        while (1) {
            message("Press key (in %d): %s", t, ascii);
            if ((ch = read_char(1000, TRUE)) >= 0) {
                break;
            }
            if (--t <= 0) {                     /* idle terminate */
                message( "" );
                return;
            }
        }
        t = 4;                                  /* idle timeout, secondary */

        if (llen == 0 || (ch & ~0xff)) {        /* !TERM or 16 bit */
            if (ch < ' ') {
                sprintf(tmp, "\\x%02x", ch);
            } else if (ch < 127) {
                sprintf(tmp, "%c", ch);
            } else {
                sprintf(tmp, "#%d", ch);
            }
            ascii = "<" + tmp + "> = " + int_to_key(ch);

        } else {                                /* multi-character ? */
            string escseq;
            int kcode;

            ascii = "";
            while (1) {
                // TODO: CSI/OSC detection
                sprintf(tmp, "%c", ch);
                escseq += tmp;                  /* raw esc sequence */

                if (ch < ' ' || ch > 127) {
                    sprintf(tmp, "\\x%X", ch);
                }
                ascii += tmp;                   /* ascii version */

                if ((ch = read_char(150, TRUE)) < 0) {
                    break;
                }
            }
            ascii = "<" + ascii + ">";

            l = re_search(NULL, "<" + quote_regexp(escseq) + ">", lst);
            if (l >= 0) {
                ascii += " := " + int_to_key(lst[l-1]);

            } else if (strlen(escseq) > 1) {
                /*
                 *  Esc => KeyCode, for example cygwin specials
                 */
                if ((kcode = key_to_int(escseq, TRUE)) > 0) {
                    ascii += " ~= " + int_to_key(kcode);
                } else {
                    ascii += " ?= <undefined>";
                }
            }
        }
    }
}


/*  Function:           key_trace
 *      Keyboard diagnostics
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      none
 */
void
key_trace(~ string arg)
{
    int buf, curbuf;

    if ((buf = create_buffer("key_trace")) < 0) {
         return;
    }

    curbuf = inq_buffer();
    set_buffer(buf);
    attach_buffer(buf);

    keyboard_push();
    if (arg == "" || arg == "--esc")
         assign_to_key("<ESC>", "exit");
    if (arg == "" || arg == "--f10")
         assign_to_key("<F10>", "exit");
    assign_to_key("<unassigned>", "::_key_trace {}"); // <esc> <key>

    if (arg == "--esc") {
         message("key_trace: <ESC> to exit");
    } else if (arg == "--f10") {
         message("key_trace: <F10> to exit");
    } else {
         message("key_trace: <ESC> or <F10> to exit");
    }
    process();
    message("");
    keyboard_pop(1);

    delete_buffer(buf);
    set_buffer(curbuf);
    attach_buffer(curbuf);
}


static void
_key_trace(string seq, int key)
{
    int hour, min, sec, msec, line;

    inq_position(line);
    time(hour, min, sec, msec);

    insertf("%6u: %02d:%02d:%02d.%03d: ", line, hour, min, sec, msec);
    if (IsMouse(key)) {
        int x, y, where, region, event;

        get_mouse_pos(x, y, NULL, NULL, NULL, where, region, event);
        insertf("0x%08x/%-9u %-32s (x=%u, y=%u, where=%u, region=%u, event=0x%x) [%s]\n",
            key, key, int_to_key(key), x, y, where, region, event, seq);

    } else {
        insertf("0x%08x/%-9d %-32s [%s]\n",
            key, key, int_to_key(key), seq);
    }
}



/*  Function:           key_map
 *      Display the current keyboard mapping
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      none
 */
void
key_map()
{
    int curbuf = inq_buffer(), curwin = inq_window();
    int i, len, buf, win;
    list lst;

    lst = key_list(NULL, NULL, NULL);
    len = length_of_list(lst);
    buf = create_buffer("Key Map", NULL, 1);
    set_buffer(buf);
    for (i = 0; i < len; i += 2) {
        insert(lst[i]);
        move_abs(0, 24);
        insert(lst[i + 1]);
        insert("\n");
    }
    delete_line();

    win = sized_window(inq_lines(), inq_line_length() + 1);
    select_buffer(buf, win, SEL_NORMAL);
    delete_buffer(buf);
    set_buffer(curbuf);
    set_window(curwin);
    attach_buffer(curbuf);
}


/*  Function:           key_val
 *      Display a key value.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      none
 */
void
key_val(void)
{
    string key;
    int val;

    get_parm(0, key, "key text: ");
    val = key_to_int("<" + key + ">");
    message("key_val: %d (%x)", val, val);
}


/*  Function:           key_learn_menu
 *      Learn keys menu interface.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      none
 */
void
key_learn_menu()
{
    list sellist;
    int l, i;

    sellist += "ALL";
    sellist += "key_learn";

    l = length_of_list(x_keys_2_learn);
    for (i = 0; i < l; i += KEYS_ELEM) {
        string label = x_keys_2_learn[i+KEYS_LABEL];

        sellist += label;
        sellist += "key_learn " + label;
    }
    select_list("key learn", "", 2, sellist, SEL_CENTER);
}


/*  Function:           key_learn
 *      Learn keys.
 *
 *  Parameters:
 *      what -              optional label to restrict functionality.
 *
 *  Generates the following:
 *
 *      set_term_keyboard(
 *          F1_F12, quote_list( ... ),
 *          SHIFT_F1_F12, quote_list( ... ),
 *          CTRL_F1_F12, quote_list( ... ),
 *          CTRLSHIFT_F1_F12, quote_list( ... ),
 *          ALT_F1_F12, quote_list( ... ),
 *          ALT_A_Z, quote_list( ... ),
 *          ALT_0_9, quote_list( ... ),
 *          KEYPAD_0_9, quote_list( ... ),
 *          SHIFT_KEYPAD_0_9, quote_list( ... ),
 *          CTRL_KEYPAD_0_9, quote_list( ... ),
 *          BACK_TAB
 *          BACKSPACE
 *          );
 *
 */
void
key_learn(~ string what)
{
    int curbuf, buf, win;
    int l, i, defs;
    list def;

    x_lastkey = "";
    x_lastmatch = 0;

    if ((buf = create_buffer("keygen")) < 0) {
        return;
    }
    curbuf = inq_buffer();
    set_buffer(buf);
    insert("/* -*- mode: cr; -*- */\n\n");

    insert("void\nkeymap(void)\n{\n");
    insert("    set_term_keyboard(\n");

    l = length_of_list(x_keys_2_learn);
    for (i = 0; i < l; i += KEYS_ELEM) {
        string label = x_keys_2_learn[i+KEYS_LABEL];

        if (what == "" || what == label) {
            def = execute_macro("_key_learn_" + x_keys_2_learn[i+KEYS_COMMAND]);
            if (0 == (defs = length_of_list(def))) {
                break;
            }
            insertf("        %s, ", label);
            if (defs > 1) {                     /* list */
                insert("quote_list(\n");
            }
            _key_learn_print(def, "", TRUE);
            if (defs > 1) {
                insert(")");
            }
            insert(",\n");
            if (defs > 1) {
                insert("\n");
            }
        }
    }
    insert("    );\n}\n");
    mode("cr");

    set_buffer(curbuf);                         /* restore buffer */

    win = sized_window(inq_lines(buf), inq_line_length(buf));
    select_buffer(buf, win, 0);
}


static void
_key_learn_print(list def, string prefix, int quote)
{
    int defs = length_of_list(def);
    int d, w;

    if (defs > 1) {                             /* list? */
        insert(" ", 12 - strlen(prefix));
        insert(prefix);
    }

    if (def[0] == "") {                         /* arg1 */
        w = insert("NULL");
    } else if (quote) {
        w = insertf("\"%s\"", def[0]);
    } else {
        w = insert(def[0]);
    }

    for (d = 1; d < defs; d++) {                /* arg2 ... x */
        if ((d % KEYS_COLUMNS) == 0) {
            insert(",\n");
            insert(" ", 12 - strlen(prefix));
            insert(prefix );
        } else {
            w += insert(", ");
            insert(" ", 16 - w);
        }

        if (def[d] == "") {
            w = insert("NULL");
        } else if (quote) {
            w = insertf("\"%s\"", def[d]);
        } else {
            w = insert(def[d]);
        }
    }
}


static string
_key_learn_get(string what, int waitms = 3000)
{
    string buf, kseq = "";
    int ch;

    if (x_lastmatch > 3) {
        return "";
    }

    while (1) {
        message("Key %s %s (waitms %d)", what, kseq, waitms);

        if ((ch = read_char(waitms, TRUE)) < 0) {
            break;
        }
        if (ch < ' ' || ch >= 127) {
            if (ch >= 256) {
                sprintf(buf, "#0x%0x", ch);
            } else {
                sprintf(buf, "\\x%02x", ch);
            }
        } else {
            sprintf(buf, "%c", ch);
        }
        kseq += buf;
        waitms = 250;                           /* inq_escape() */
    }

    if ("quit" == kseq) {
        x_lastmatch = 99;
    } else if (x_lastkey == kseq) {
        if (kseq != "") {
            ++x_lastmatch;
        }
    } else {
        x_lastkey = kseq;
        x_lastmatch = 0;
    }
    return kseq;
}


static list
_key_learn_fn(~string)
{
    string label, what;
    list defs;
    int k;

    if (get_parm(0, label)) {
        label += "-";
    }

    for (k = 0; k < 12; k++) {
        sprintf(what, "<%sF%d>", label, k+1);
        defs[k] = _key_learn_get(what);
    }
    return defs;
}


static list
_key_learn_aa(int alpha)
{
    string what;
    list labels, defs;
    int kn, ch, k;

    if (alpha) {
        kn = 26, ch = (alpha == 1 ? 'A' : 'a');
    } else {
        kn = 9, ch = '0';
    }

    for (k = 0; k < kn; k++) {
        sprintf(what, "<Alt-%c>", ch++);
        labels[k] = what;
        defs[k] = _key_learn_get(what);
    }

    /* labels */
    _key_learn_print(labels, "//  ", FALSE);
    insert("\n");
    return defs;
}


static list
_key_learn_kp(~string)
{
    string label, what;
    list defs;
    int k;

    if (get_parm(0, label)) {
        label += "-";
    }

    for (k = 0; k < length_of_list(x_keys_kp_names); k++) {
        sprintf(what, "<%sKeypad-%s>", label, x_keys_kp_names[k]);
        defs[k] = _key_learn_get(what, 4000);
    }

    /* labels */
    _key_learn_print(x_keys_kp_names, "//  ", FALSE);
    insert("\n");
    return defs;
}


static list
_key_learn_sp(string what)
{
    list defs;

    defs += _key_learn_get(what);
    return defs;
}


/*  Function:           key_termmap
 *      Generate a keyboard terminal mapping summary.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      none
 */
void
key_termmap(void)
{
    extern int window_offset;
    int curbuf, buf, win;
    list lst;
    int llen, l;

    lst = get_term_keyboard();                  /* retrieve list */
    if ((llen = length_of_list(lst)) == 0) {
        message("term mapping not available/required");
        return;                                 /* MSDOS etc */
    }

    if ((buf = create_buffer("Terminal Key Mapping", NULL, 1)) < 0) {
        return;
    }

    curbuf = inq_buffer();
    set_buffer(buf);
    for (l = 0; l < llen; l += 2) {
        insert(int_to_key(lst[l]));             /* keycode */
        tohex(lst[l+1]);
    }
    delete_line();
    sort_buffer();                              /* ??? */

    set_buffer(curbuf);                         /* restore buffer */
    window_offset += 10;
    win = sized_window(inq_lines(buf), inq_line_length(buf), "<F10> exit.");
    select_buffer(buf, win, SEL_NORMAL, NULL, NULL, NULL);
    window_offset -= 10;

    delete_buffer(buf);                         /* release local buffer */
}


static void
tohex(string seq)
{
    int i, seqlen = strlen(seq);
    string h, a;

    for (i = 1; i <= seqlen; ++i) {
        int ch = characterat(seq, i);
        h += format("%02X ", ch);
        a += format("%c", (ch > ' ' && ch < 0xff ? ch : '.'));
    }
    move_abs(0, 32);
    insertf("| %-*s | %-*s \n", 14, a, 14*3, h);
}

/*end*/
