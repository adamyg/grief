/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: debug.cr,v 1.17 2020/04/20 23:13:00 cvsuser Exp $
 * Debugging package.
 *
 *
 */

#include "grief.h"
#include "debug.h"

#define DEBUG 0                                 /* Set to 1 if we are debugging debugger. */

static void         dbg_keylist(void);
static void         dbg_c(void);
static void         dbg_d(void);
static void         dbg_stack(string start_fn);
static void         var_dump(int vartype, declare object);
static void         var_print(int level, declare v);

static int          _dbg_saved_win;             /* If _dbg_win == -1, then _dbg_saved_win
                                                 * is real value of _dbg_win so we can delete
                                                 * window when we finish tracing macro
                                                 */

static int          _dbg_old_line = -1;         /* Old value of line number - stops us
                                                 * from keep tracing the same line
                                                 */

static int          _dbg_win = -1;
static int          _dbg_buf;

static int          _dbg_kbd;                   /* Keyboard map for debugger. */
static list         _dbg_bpt;                   /* List of breakpoints. */

static int          _dbg_nbrk = 0;              /* Number of breakpoints active. */
static int          _dbg_on = FALSE;            /* If TRUE, (debug) has been called. */

static list         __dbg_type_info;            /* Symbol types, see main() */


void
main(void)
{
    __dbg_type_info =
        debug_support(DBG_INQ_OPCODES, NULL, "");
}


void
trace(~string)
{
    string macro_name;

    get_parm(0, macro_name, "Macro to trace: ");
    if (macro_name == "") {
        return;
    }
    __dbg_init();
    execute_macro(macro_name);
    if (_dbg_win == -1) {
        _dbg_win = _dbg_saved_win;
    }
    set_window(_dbg_win);
    delete_window();
    _dbg_win = -1;
    message("Macro %s completed.", macro_name);
}


void
__dbg_init(void)
{
    extern int window_offset;

    window_offset += 40;
    _dbg_win = sized_window(20, 48, "<Alt-H> for help.");
    window_offset -= 40;
}


/*
 *  __dbg_trace__ ---
 *      callback executed by object debug produced by 'grunch -g'.
 */
void
__dbg_trace__(~int /*lineno*/, ~string /*filename*/, ~string /*function*/)
{
    int curbuf, curwin, msg_printed, line_no;
    string filename, macro_name;

    if (_dbg_on) {
        debug(DEBUG, FALSE);
    }
    get_parm(0, line_no);
    get_parm(2, macro_name);
    msg_printed = FALSE;
    if (_dbg_win == -1 || _dbg_old_line == line_no) {
        /*
         *  See if we have a breakpoint
         */
        if (_dbg_nbrk == 0 || _dbg_old_line == line_no ||
                re_search(NULL, "<" + macro_name + ">", _dbg_bpt) < 0) {
            if (_dbg_on) {
                debug(1);
            }
            return;
        }
        if (_dbg_win == -1) {
            _dbg_win = _dbg_saved_win;
        }
        msg_printed = TRUE;
        message("Breakpoint in %s", macro_name);
    }
    curbuf = inq_buffer();
    curwin = inq_window();

    _dbg_old_line = line_no;
    set_window(_dbg_win);
    get_parm(1, filename);
    if (edit_file(filename) == -1) {
        set_buffer(curbuf);
        set_window(curwin);
        attach_buffer(curbuf);

    } else {
        select_buffer(inq_buffer(), _dbg_win,
                SEL_NORMAL, "::dbg_listkeys", NULL, NULL, line_no, 1);
        if (msg_printed)
            message("");
        set_buffer(curbuf);
        set_window(curwin);
        attach_buffer(curbuf);
        if (_dbg_on) {
            debug(1);
        }
    }
}


static void
dbg_listkeys(void)
{
    assign_to_key("s", "exit");
    assign_to_key("b", "buffer_list 1");
    assign_to_key("c", "::dbg_c");
    assign_to_key("d", "::dbg_d");
    assign_to_key("k", "::dbg_stack \"__dbg_trace__\"");
}


static void
dbg_c(void)
{
    _dbg_saved_win = _dbg_win;
    _dbg_win = -1;
    exit();
}


static void
dbg_d(void)
{
    _dbg_on = !_dbg_on;
    if (_dbg_on) {
        message("Debug tracing turned on.");
    } else {
        message("Debug tracing turned off.");
    }
}


static void
dbg_stack(string start_fn)
{
    local list stack = debug_support(DBG_STACK_TRACE, NULL, start_fn);

    select_slim_list("Stack", "", stack, 0,  NULL, "::dbg_stackkeys");
}


/*
 *  dbg_stackkeys ---
 *      Function called to set up key assignments for the stack trace.
 */
static void
dbg_stackkeys(void)
{
    assign_to_key("<Enter>", "::dbg_s_enter");
    assign_to_key("<Alt-G>", "vars");
}


/*
 *  dbg_s_enter ---
 *      Function to look at variables defined at the current stack level.
 */
static void
dbg_s_enter(void)
{
    extern list stack;
    int line;

    inq_position(line);
    vars(length_of_list(stack) - line);
}


/*
 *  brk ---
 *      Macro to set a breakpoint at a certain line/file or macro.
 */
void
brk(string macro_name)
{
    if (macro_name == "") {
        if (get_parm(NULL, macro_name, "Macro name: ") <= 0 ||
                (macro_name = trim(macro_name)) == "") {
            if (_dbg_nbrk == 0) {
                message("No breakpoints active.");
                return;
            }
            select_list("Breakpoint List", "", 1, _dbg_bpt, SEL_NORMAL);
            return;
        }
    }
    _dbg_bpt[_dbg_nbrk] = macro_name;
    ++_dbg_nbrk;
}


/*
 *  Macro to evaluate argument and print result.
 */
void
evaluate()
{
    declare arg;
    int ret, p;

    if (get_parm(0, arg, "Argument: ") <= 0)
        return;
    p = pause_on_error();
    ret = execute_macro(arg);
    pause_on_error(p);
    message("Result=%d", ret);
}


/*
 *  inq_nest_level --
 *      Echo the current execution nesting level.
 */
int
inq_nest_level(void)
{
    int level = debug_support(DBG_NEST_LEVEL);

    message("Nesting level=%d", level);
    return level;
}


/*
 *  vars ---
 *      Display all global variables.
 */
void
vars(~int)
{
    int level;

    if (get_parm(0, level) <= 0)
        level = -1;                             /* global */
    var_dump(DBG_INQ_VARS, level);
}


/*
 *  bvars ---
 *      Display all buffer variables.
 */
void
bvars(~int)
{
    int level;

    if (get_parm(0, level) <= 0) {
        level = inq_buffer();                   /* current buffer */
    }
    var_dump(DBG_INQ_BVARS, level);
}


/*
 *  mvars ---
 *      Display module variables.
 */
void
mvars(~string)
{
    string modname;

    if (get_parm(0, modname) <= 0) {
        modname = "";                           /* XXX - need list 'all' functionality */
    }
    var_dump(DBG_INQ_MVARS, modname);
}


/*
 *  var_dump ---
 *      Variable dumper.
 */
static void
var_dump(int vartype, declare object)
{
    int buf, curbuf;
    int win, curwin;
    list vars, vari;
    int len, i;
    string name;

    curbuf = inq_buffer();
    curwin = inq_window();

    vars = debug_support(vartype, object);
    if ((len = length_of_list(vars)) == 0) {
        message("No variables defined.");
        return;
    }
    message("");

    /*
     *  If we have a stack level then use the name of that function as the window title
     */
    if (DBG_INQ_BVARS == vartype) {
        vartype = DBG_INQ_BVAR_INFO;
        name = "Buffer Variables";

    } else if (DBG_INQ_MVARS == vartype) {
        vartype = DBG_INQ_MVAR_INFO;
        name = "Module Variables";

    } else {
        if (object >= 0) {
            vari = debug_support(DBG_STACK_TRACE, NULL, "");
            name = vari[length_of_list(vari) - object - 1] + " #" + object;

        } else {
            name = "Global Variables";
        }
        vartype = DBG_INQ_VAR_INFO;
    }

    /*
     *  Display each variable
     */
    if ((buf = create_buffer(name, NULL, TRUE)) >= 0) {
        set_buffer(buf);
        for (i = 0; i < len;) {
            string var = vars[i++];

            vari = debug_support(vartype, object, var);
            insert(__dbg_type_info[vari[0]]);   /* type */
            insert(var + " = ");                /* name */
            var_print(0, vari[1]);              /* value */
        }

        win = sized_window(inq_lines(), inq_line_length());
        select_buffer(buf, win);

        delete_buffer(buf);
        set_buffer(curbuf);
        set_window(curwin);
        attach_buffer(curbuf);
    }
}


static void
var_print(int level, declare v)
{
    list lst;
    int len, i;

    for (i = 0; i < level; i++) {
        insert("\t");
    }

    switch (typeof(v)) {
    case "array":
    case "list":
        if ((len = length_of_list(v)) == 0) {   /* empty */
            insert("NULL");

        } else if (level == -1) {               /* dont expand */
            insert("{list}");

        } else {                                /* expand var */
            lst = v;
            insert("{\n");
            for (i = 0; i < len; i++) {
                var_print(level+1, lst[i]);
            }
            for (i = 0; i < level; i++)
                insert("\t");
            insert("}");
        }
        break;

    case "string":
        v = gsub("\"", "\\\\\"", v);
        insert("\"" + v + "\"");
        break;

    case "NULL":
        insert("NULL");
        break;

    case "undef":
        insert("undef");
        break;

    default:                                    /* int's and float's */
        insert("" + v);
        break;
    }

    if (level > 0) {
        insert(",");
    } else {
        insert(";");
    }
    insert("\n");
}


/*
 *  Useful macro for debugging.
 */
void
debug_pause(string msg)
{
    int save_buffer = inq_buffer(), save_window = inq_window();
    string arg;

    get_parm(1, arg, msg + " [enter] ");
    message("");
    set_buffer(save_buffer);
    set_window(save_window);
}

/*eof*/


