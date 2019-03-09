#include <edidentifier.h>
__CIDENT_RCSID(gr_mac1_c,"$Id: mac1.c,v 1.69 2018/11/18 00:20:40 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: mac1.c,v 1.69 2018/11/18 00:20:40 cvsuser Exp $
 * Basic primitives.
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
#include <edpackage.h>
#include <libstr.h>                             /* str_...()/sxprintf() */

#include "accum.h"                              /* acc_...() */
#include "basic.h"
#include "buffer.h"
#include "builtin.h"
#include "debug.h"
#include "display.h"
#include "echo.h"
#include "eval.h"
#include "file.h"
#include "getkey.h"
#include "keyboard.h"
#include "keywd.h"
#include "line.h"
#include "lisp.h"
#include "mac1.h"
#include "macros.h"
#include "main.h"
#include "map.h"
#include "object.h"
#include "prntf.h"
#include "register.h"
#include "ruler.h"                              /* ruler_...() */
#include "symbol.h"                             /* sym_..,() */
#include "system.h"                             /* sys_...() */
#include "tty.h"                                /* tty_...() */
#include "undo.h"
#include "window.h"                             /* win_...() */
#include "word.h"

#include "m_pty.h"

static int              x_switch_level;
static int              x_switch_break;
static int              x_continue;
static int              x_loopcount;

int                     x_break = FALSE;
int                     x_selfinsert = FALSE;   /* used by undo collasper */

static void             loop_common(int cond_index, int post_index, int stmt_index);


/*  Function:           do_set_msg_level
 *      set_msg_level primitive.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 *
 *<<GRIEF>>
    Macro: set_msg_level - Set level of informational messages.

        int
        set_msg_level(int level)

    Macro Description:
        The 'set_msg_level()' primitive sets the message level for
        the duration of the current command.

        The message level control what type of messages are to be
        made visible on the status line; it allows macros to
        suppress messages from macros.

        By default the message level is a value of 1 whenever a
        command is invoked from the keyboard or a registered macro,
        and set to zero when the command completes.

        The specified 'level' is a value in the range 0-3 with the
        following effects:

            0 - All messages are enabled; the default value.

            1 - Normal messages are not displayed, error messages
                    still display.

            2 - Error messages are suppressed.

            3 - Suppress all messages, both message and error.

    Macro Parameters:
        level - Integer value specifying the new message level.

    Macro Returns:
        The 'set_msg_level()' primitive returns the previous message
        level.

    Macro Portability:
        n/a

    Macro See Also:
        inq_msg_level, error, message
 */
void
do_set_msg_level(void)          /* (int level) */
{
    const int omsglevel = x_msglevel;

    x_msglevel = get_xinteger(1, 0);
    acc_assign_int((accint_t) omsglevel);
}


/*  Function:           do_inq_msg_level
 *      inq_msg_level primitive.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 *
 *<<GRIEF>>
    Macro: inq_msg_level - Get the message level.

        int
        inq_msg_level()

    Macro Description:
        The 'inq_msg_level()' primitive retrieves the current message
        level.

            0 - All messages are enabled; the default value.

            1 - Normal messages are not displayed, error messages
                    still display.

            2 - Error messages are suppressed.

            3 - Suppress all messages, both message and error.


    Macro Parameters:
        none

    Macro Returns:
        The 'inq_msg_level()' primitive returns the message level.

    Macro Portability:
        n/a

    Macro See Also:
        set_msg_level
 */
void
inq_msg_level(void)             /* int () */
{
    acc_assign_int((accint_t) x_msglevel);
}


/*  Function:           do_backspace
 *      backspace primitive
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 *
 *<<GRIEF>>
    Macro: backspace - Delete character to the left of the cursor.

        void
        backspace([int num = 1])

    Macro Description:
        The 'backspace()' primitive moves the cursor and deletes the
        character to the left of the cursor in the current buffer.

        The actions of 'backspace' are dependent on the current
        insert mode.

    Insert Mode::

        If insert mode backspaces moves the cursor and deletes the
        previous character, all characters to the right move one
        character to the left.

        If the cursor is at the beginning of the line then the
        current line is appended to the end of the previous line.

    Overtype Mode::

        If overstrike mode backspaces moves the cursor and deletes the
        previous character, replacing it with a space.

        If the previous character is a tab, it moves over virtual
        spaces between the current position and the tab character
        when moving back.

        If the cursor is at the beginning of the line then the cursor
        is moved to end of the previous line.

    Macro Parameters:
        num - Optional integer, if stated specifies the number of
            characters the cursor to be moved backwards, if omitted
            only a single character position is moved.

    Macro Returns:
        The 'backspace()' primitive returns non-zero if successful
        impling the cursor moved, otherwise zero.

    Macro Portability:
        The 'num' option is a Grief extension.

    Macro See Also:
        delete_char, left
 */
void
do_backspace(void)              /* void ([int num = 1]) */
{
    int num = get_xinteger(1, 1);

    if (buf_imode(curbp)) {
        /*
         *  insert mode
         */
        while (num-- >= 1) {
            move_prev_char(1);
            if (0 == acc_get_ival()) {
                return;
            }
            ldeletec(1);                        /* MCHAR */
        }

    } else {
        /*
         *  overstrike mode
         */
        while (num-- >= 1) {
            const int oline = *cur_line;

            move_prev_char(1);
            if (acc_get_ival()) {
                if (oline == *cur_line) {
                    lwritec(' ');
                    mov_backchar(1, TRUE);

        //TODO  } else if (OVMODEF_EOL & xf_ovmode) {
                } else if (XF_TEST(3)) {
                    if (! BFTST(curbp, BF_SYSBUF)) {
                        ldelete(1);             /* join lines */
                    }
                }
            }
        }
    }
}


/*  Function:           do_delete_char
 *      delete_char primitive.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 *
 *<<GRIEF>>
    Macro: delete_char - Delete character.

        void
        delete_char([int num])

    Macro Description:
        The 'delete_char()' primitive deletes one or more characters at the
        current cursor position.

        This primitive is the default assignment for the <Delete> key on the
        keyboard.

    Macro Parameters:
        num - Optional integer, if stated specifies the number of
            characters to be deleted, if omitted only a single
            character is removed.

    Macro Returns:
        nothing

    Macro Portability:
        n/a

    Macro See Also:
        backspace, delete_block, delete_line
 */
void
do_delete_char(void)            /* void ([int num]) */
{
    if (isa_integer(1)) {
        /*
         *  delete current 'n' bytes
         */
        const accint_t n = get_xinteger(1, 0);

        if (n > 0) {                            /* region */
            line_current_offset(LOFFSET_FILL_VSPACE);
            ldelete(n);
        }
    } else {
        /*
         *  delete current 'character'
         */
        ldeletec(1);                            /* MCHAR */
    }
}


/*  Function:           do_delete_to_eol
 *      delete_to_eol primitive.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 *
 *<<GRIEF>>
    Macro: delete_to_eol - Delete to end-of-line

        void
        delete_to_eol()

    Macro Description:
        The 'delete_to_eol()' primitive deletes from the current
        cursor position to the end-of-line, not including the
        newline. The cursor position remains unchanged.

    Macro Parameters:
        none

    Macro Returns:
        nothing

    Macro Portability:
        n/a

    Macro See Also:
        delete_char, delete_line
 */
void
do_delete_to_eol(void)          /* void () */
{
    const LINENO cline = *cur_line, ccol = *cur_col;
    LINE_t *lp;
    int dot, n;

    lp = linep(cline);
    dot = line_offset2(lp, cline, ccol, LOFFSET_NORMAL);
    if ((n = llength(lp) - dot) > 0) {
        ldelete(n);
    }
    vm_unlock(cline);
}


/*  Function:           do_delete_line
 *      delete_line primitive.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 *
 *<<GRIEF>>
    Macro: delete_line - Delete current line

        void
        delete_line()

    Macro Description:
        The 'delete_line()' primitive deletes the current line,
        placing the cursor at the same column on the following line.

    Macro Parameters:
        none

    Macro Returns:
        nothing

    Macro Portability:
        n/a

    Macro See Also:
        delete_char, delete_to_eol
 */
void
do_delete_line(void)            /* void () */
{
    const LINENO cline = *cur_line,  ccol = *cur_col;
    LINE_t *lp;

    u_dot();
    *cur_col = 1;
    lp = linep(cline);
    ldelete(llength(lp) + 1);                   /* MCHAR, line plus EOL */
    *cur_col = ccol;
    vm_unlock(cline);
}


/*  Function:           do_goto_line
 *      goto_line primitive.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 *
 *<<GRIEF>>
    Macro: goto_line - Move to a particular line.

        int
        goto_line([int lineno])

    Macro Description:
        The 'goto_line()' primitive repositions the cursor to tbe
        beginning of the specified line 'lineno'.

    Macro Parameters:
        lineno - Specifies the line number which to relocate the
            cursor, if omitted the user is prompted.

    Macro Returns:
        The 'goto_line()' primitive returns *true* if successful,
        otherwise zero or less if unsuccessful.

    Macro Portability:
        n/a

    Macro See Also:
        goto_old_line, move_abs
 */
void
do_goto_line(void)              /* int ([int lineno]) */
{
    const LINENO cline = *cur_line;
    accint_t lineno;

    if (get_iarg1("Go to line: ", &lineno)) {
        acc_assign_int(-1);
        return;
    }
    mov_gotoline(lineno > 0 ? lineno : 1);
    acc_assign_int((accint_t) (cline != *cur_line));
}


/*  Function:           do_goto_old_line
 *      goto_old_line primitive.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 *
 *<<GRIEF>>
    Macro: goto_old_line - Move to line before buffer modification.

        int
        goto_old_line([int oldlineno])

    Macro Description:
        The 'goto_old_line()' primitive repositions the cursor as
        close as possible to the beginning of the specified line
        'oldlineno', representing the line prior to any buffer
        modifications.

        For each buffer the previous line numbers are retained for
        any one edit session, that is they are maintained until the
        buffer is saved, at which point line references are reset to
        the resulting new image.

        This primitive is provided for seeking lines within a buffer
        that are referred to in an external listing. For example
        within a previous compiler error report yet since that time
        inserts and/or line deletes have occurred to the source, yet
        despite these edits the original line can still be addressed.

    Macro Parameters:
        oldlineno - Specifies the old line number which to relocate
            the cursor, if omitted the user is prompted.

    Macro Returns:
        The 'goto_old_line()' primitive returns *true* if successful,
        otherwise zero or less if unsuccessful.

    Macro Portability:
        n/a

    Macro See Also:
        goto_line, move_abs
 */
void
do_goto_old_line(void)          /* int ([int oldlineno]) */
{
    register const LINE_t *lp;
    LINENO oldlineno, clineno = 1,
        closestdiff = 0x7ffffff, closestlineno = 0;
    accint_t t_oldlineno = 0;

    if (get_iarg1("Go to old line: ", &t_oldlineno)) {
        acc_assign_int(-1);
        return;
    }
    oldlineno = (LINENO) (t_oldlineno <= 0 ? 1 : t_oldlineno);

    TAILQ_FOREACH(lp, &curbp->b_lineq, l_node) {
        const LINENO lineno = lp->l_oldlineno;

        if (lineno == oldlineno) {
            closestdiff = 0;
            closestlineno = clineno;
            break;
        } else {
            const LINENO diff =
                    (lineno > oldlineno ? lineno - oldlineno : oldlineno - lineno);

            if (diff < closestdiff) {
                closestdiff = diff;
                closestlineno = clineno;
            }
        }
        ++clineno;
    }

    trace_ilog("goto_old_line(old:%d, closest:%d, diff:%d)\n", \
        (int)oldlineno, (int)closestlineno, (int)closestdiff);
    mov_gotoline(closestlineno ? closestlineno : oldlineno);
 /* acc_assign_int((accint_t) (cline != *cur_line)); */
    acc_assign_int((accint_t) closestdiff);     /* extension */
}


/*  Function:           do_execute_macro
 *      execute_macro primitive.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 *
 *<<GRIEF>>
    Macro: execute_macro - Invokes a command by name.

        declare
        execute_macro([string cmd], ...)

    Macro Description:
        The 'execute_macro()' primitive invokes the specified command
        'cmd', which if omitted the user shall be prompted as follows:

>           Command:

        When an invoked command is undefined, GRIEF shall attempt to
        load the associated macro file sourced from the directories
        listed in the <GRPATH>. If the macro file is successfully
        loaded and the command is found, the macro is then executed.

        Note!:
        The execute_macro primitive cannot be overloaded with a
        replacement macro.

    Macro Parameters:
        cmd -  Optional string buffer containing the command to be
            executed. Otherwise if omitted the user shall be prompted
            for the command.

        ... - Depending on the macro, the executed command may expect a
            sequence of additional arguments.

    Macro Returns:
        The 'execute_macro()' primitive returns the result of the
        executed command, otherwise upon an error -2 shall be
        returned..

    Macro Portability:
        n/a

    Macro See Also:
        load_macro
 */
void
do_execute_macro(void)          /* declare ([string cmd], ...) */
{
    LIST tmpl[LIST_SIZEOF(1)], *halt;           /* 1 atom */
    char buf[BUFSIZ] = {0};
    const char *arg;

    if (isa_undef(1)) {
        if (NULL == (arg = get_arg1("Command: ", buf, sizeof(buf)))) {
            acc_assign_int(-2);
            return;
        }
        eclear();
        execute_str(arg);
        return;
    }

    if (NULL == (arg = get_str(1)) || 0 == *arg) {
        acc_assign_int(0);
        return;
    }

    if (strchr(arg, ' ')) {                     /* others ??? */
        execute_str(arg);
        return;
    }

    halt = atom_push_sym(tmpl, arg);
    atom_push_halt(halt);
    if (++x_nest_level >= MAX_NESTING) {
        panic("Macro nesting overflow.");
    }
    execute_xmacro(tmpl, isa_list(2) ? get_list(2) : halt);
    sym_local_delete();
}


/*  Function:           do_nothing.
 *      nothing primitive.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 *
 *<<GRIEF>>
    Macro: nothing - Noop

        void
        nothing()

    Macro Description:
        The 'nothing()' primitive is a no operation, noop for short, This
        primitive it acts as a place holder when a dummy function is required,
        effectively does nothing at all.

    Macro Parameters:
        none

    Macro Returns:
        nothing

    Macro Portability:
        n/a

    Macro See Also:
        error
 */
void
do_nothing(void)                /* (...) */
{
}


/*  Function:           do_do
 *      do primitive.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 *
 *<<GRIEF>>
    Macro: do - do statement

        do statement; while (condition);

    Macro Description:
        The 'do' statement implements the 'do-while' loop construct.

>           do {
>               statements;
>           } while (condition);

        Executes statements one or more times until 'condition' is
        non-zero. The condition is tested after each iteration of the
        loop.

    Macro Portability:
        n/a

    Macro See Also:
        Iteration Statements, while, break, continue, for, if
 */
void
do_do(void)                     /* (statement, condition) */
{
    LISTV condition, statement, result;

    ++x_loopcount;
    statement = margv[1];
    condition = margv[2];

    x_break = FALSE;
    x_continue = FALSE;
    do {
        if (statement.l_flags != F_NULL) {
            execute_expr(&statement);
        }
        if (x_continue) {
            x_continue = x_break = FALSE;
        }
    } while (! x_return && ! x_break &&
                eval(condition.l_list, &result) == F_INT && result.l_int);

    --x_loopcount;
    x_break = x_continue = FALSE;
}


static void
loop_common(int cond_index, int post_index, int stmt_index)
{
    LISTV condition = {0}, post_op = {0}, statement = {0};
    LISTV result;

    ++x_loopcount;

    if (cond_index) {
        assert(cond_index < margc);
        condition = margv[cond_index];
        assert(condition.l_flags == F_LIST || condition.l_flags == F_NULL);
    } else {
        condition.l_flags = F_NULL;
    }

    if (post_index) {
        assert(post_index > 0);
        assert(post_index < margc);
        post_op = margv[post_index];
        assert(post_op.l_flags == F_LIST || post_op.l_flags == F_NULL);
    } else {
        post_op.l_flags = F_NULL;
    }

    if (stmt_index && stmt_index < margc) {
        statement = margv[stmt_index];
        assert(statement.l_flags == F_LIST || statement.l_flags == F_NULL);
    } else {
        statement.l_flags = F_NULL;
    }

    x_break = FALSE;
    x_continue = FALSE;

    while (!x_return && !x_break) {
        switch (eval(condition.l_list, &result)) {
        case F_INT:
            if (0 == result.l_int) {
                goto end_of_loop;
            }
            break;
        case F_FLOAT:
            if (0 == result.l_float) {
                goto end_of_loop;
            }
            break;
        default:
            goto end_of_loop;
        }

        if (statement.l_flags != F_NULL) {
            execute_expr(&statement);
            if (x_return)
                break;

            if (x_continue) {
                ;
            } else if (x_break) {
                break;
            }
        }

        if (post_op.l_flags != F_NULL) {
            (void)eval(post_op.l_list, &result);
        }

        if (x_continue) {
            x_continue = x_break = FALSE;
        }
    }

end_of_loop:;
    --x_loopcount;
    x_break = x_continue = FALSE;
}


/*  Function:           do_for
 *      for primitive.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 *
 *<<GRIEF>>
    Macro: for - for statement

        for ([initialise]; [condition]; [increment]) statements;

    Macro Description:
        The 'for' statement implements the 'for' loop construct.

>           for (initialise; condition; increment)
>           {
>               statements;
>           }

        Repeatedly performing 'statements' while the 'condition' is
        *true* (non-zero).

        When the for function starts, 'initialise' is evaluated once.
        It evaluates 'condition' and if the result is non-zero, it
        evaluates the 'statements'. Finally, 'increment' is evaluated,
        and control returns to the condition evaluation. The process
        is repeated from here until the condition is evaluated zero.

    Macro Portability:
        n/a

    Macro See Also:
        Iteration Statements, break, continue, while, do
 */
void
do_for(void)                    /* (pre, expr, post) */
{
    loop_common(2, 3, 4);
}


/*  Function:           do_while
 *      while primitive.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 *
 *<<GRIEF>>
    Macro: while - while statement

        while ([condition]) statements;

    Macro Description:
        The 'while' statement implements the 'while' loop construct.

>           while (condition)
>           {
>               statements;
>           }

        Repeatedly performs 'statements' while the 'condition' is
        *true* (non-zero). In each iteration of the loop, it first
        tests the condition, and if it is *true* it executes
        statement. This cycle is continued until condition is
        *false* (zero).

    Macro Portability:
        n/a

    Macro See Also:
        Iteration Statements, break, continue, for, do
 */
void
do_while(void)                  /* (expr) */
{
    loop_common(1, 0, 2);
}


/*  Function:           do_break
 *      break primitive.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 *
 *<<GRIEF>>
    Macro: break - break statement

        break;

    Macro Description:
        The 'break' statement is used to terminate 'switch',
        'while', 'do' and 'for' loops.

    Macro Returns:
        nothing

    Macro See Also:
        Jump Statements, switch, while, do, for
 */
void
do_break(void)                  /* () */
{
    if (x_loopcount <= 0) {
        errorf("break not executed in loop.");
        return;
    }
    trace_ilog("break\n");
    assert(x_break == FALSE);
    x_break = TRUE;
}


/*  Function:           do_breaksw
 *      breaksw primitive, switch break.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 *
 *<<GRIEF>>
    Macro: __breaksw - Switch break statement

        __breaksw;

    Macro Description:
        The '__breaksw' statement is used to implement the <break>
        statement within <switch> statements.

        Note!:
        The '__breaksw()' primitive is *internal* to the macro
        language. It is generated by the GRIEF macro compiler when a
        'break' statement is encountered within a switch statement;
        it not intended for direct use.

    Macro Returns:
        nothing

    Macro See Also:
        Jump Statements, switch, while, do, for
 */
void
do_breaksw(void)                /* () */
{
    if (x_switch_level <= 0) {
        errorf("__breaksw not executed inside switch.");
        return;
    }
    trace_ilog("__breaksw\n");
    assert(x_break == FALSE);
    x_break = TRUE;
    x_switch_break = TRUE;
}


/*  Function:           do_foreach
 *      foreach primitive.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 *
 *<<GRIEF>>
    Macro: foreach - Container iterator.

        void
        foreach(<body>,<expr>,<key>,<value>,<index>,<increment>)

    Macro Description:
        The 'foreach' statement is reserved for future use.

    Macro Parameters:
        n/a

    Macro Returns:
        The 'foreach' statement returns the element index retrieved,
        otherwise -1 upon an end-of-source condition or error.

    Macro Portability:
        n/a

    Macro See Also:
        list_each, dict_each
 */
void
do_foreach(void)                /* (<body> <expr> <key> <value> <index> <increment>) */
{
#if (TODO_FOREACH)
    LISTV statement;

    ++x_loopcount;
    statement = margv[1];
/*
 *  if (list) {
 *      foreach(list) {
 *          execute_expr(&statement);
 *      }
 *  } elif (dictionary) {
 *      foreach(dictionary) {
 *          execute_expr(&statement);
 *      }
 *  } else if (string) {
 *      foreach(character) {
 *          execute_expr(&statement);
 *      }
 *  }
 */
#endif

    --x_loopcount;
    x_break = x_continue = FALSE;
}


/*  Function:           do_continue
 *      continue primitive.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 *
 *<<GRIEF>>
    Macro: continue - Loop continuation.

        continue;

    Macro Description:
        The 'continue' statement forces transfer of control to the
        controlling expression of the smallest enclosing <do>,
        <for>, or <while >loop.

        A 'continue' statement may only appear within a loop body,
        and causes a jump to the inner-most loop-continuation
        statement (the end of the loop body).

        In a <while> loop, the jump is effectively back to the while.

        In a <do> loop, the jump is effectively down to the while

        In a <for> statement, the jump is effectively to the
        closing brace of the compound-statement that is the subject
        of the for loop. The third expression in the for statement,
        which is often an increment or decrement operation, is then
        executed before control is returned to the loop's
        conditional expression.

    Macro Parameters:
        none

    Macro Returns:
        nothing.

    Macro Portability:
        n/a

    Macro See Also:
        Jump Statements, while, for, foreach
 */
void
do_continue(void)               /* () */
{
    x_break = TRUE;
    x_continue = TRUE;
}


/*  Function:           do_try
 *      try/catch/finally primitives.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 *
 *<<GRIEF>> [macro]
    Macro: try - Try statement

        try statement; catch statement; finally statement;

    Macro Description:
        The 'try' statement is reserved for future use, which shall
        implement try, catch and finally exception constructs.

        Exceptions are used to indicate that an error has occurred
        while running the program. Exception objects that describe an
        error are created and then thrown with the <throw> primitive.
        The runtime then searches for the most compatible exception
        handler.

>           try {
>               statement;
>
>           } catch type {
>               statement;
>
>           } finally {
>               statement;
>
>           }

    Macro Parameters:
        none

    Macro Returns:
        nothing

    Macro Portability:
        A Grief extension.

    Macro See Also:
        try, catch, finally, throw

 *<<GRIEF>> [macro]
    Macro: catch - Catch statement

        catch statement;

    Macro Description:
        The 'catch' statement is reserved for future use, shall
        implement try, catch and finally constructs.

    Macro Parameters:
        none

    Macro Returns:
        nothing

    Macro Portability:
        A Grief extension.

    Macro See Also:
        try, catch, finally, throw

 *<<GRIEF>> [macro]
    Macro: finally - Finally statement

        finally statement;

    Macro Description:
        The 'finally' statement is reserved for future use, shall
        implement try, catch and finally constructs.

    Macro Parameters:
        none

    Macro Returns:
        nothing

    Macro Portability:
        A Grief extension.

    Macro See Also:
        try, catch, finally, throw

 */
void
do_try(void)                    /* (expr catch-list [finally]) */
{
    if (++x_nest_level >= MAX_NESTING) {
        panic("Macro nesting overflow.");
    }

#if (TODO_TRY_CATCH)
    execution body
    if (exception-stack)
        if (! exception-double)
            expr = exception-stack
            foreach () {
                if (NULL == condition || condition) {
                    execute body                // default or condition-match
                    break;
                }
            }
            if (! exception-rethrow)            // throw()
                pop exception-stack
            exception-rethrow = false
    }
    if (finally) {
        execute body
    }
#endif

    sym_local_delete();
    acc_assign_int(0);
}


/*  Function:           do_throw
 *      throw primitives.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 *
 *<<GRIEF>>
    Macro: throw - Throw an exception.

        throw expr;

    Macro Description:
        The 'throw' statement is reserved for future use.

        Exceptions are used to indicate that an error has occurred
        while running the program. Exception objects that describe
        an error are created and then thrown with the 'throw'
        primitive. The runtime then searches for the most
        compatible exception handler.

    Macro Parameters:
        expr - Exception value.

    Macro Returns:
        nothing

    Macro Portability:
        A Grief extension.

    Macro See Also:
        try, catch, finally, throw
 */
void
do_throw(void)                  /* void (int|string expr) */
{
#if (TODO_TRY_CATCH)
    if (expr) {
        push exception-stack, <expr>
        if (exception-stack) {
            ++exception-double;
            exception-rethrow = true;
        }
    } else {
        if (exception-stack) {
            exception-rethrow = true;
        } else {
            error
        }
    }
#endif
}


/*  Function:           do_switch
 *      switch primitive.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 *
 *<<GRIEF>>
    Macro: switch - Switch statement

        switch( expr ) statement

    Macro Description:
        The 'switch' and <case> statements help control complex
        conditional and branching operations. The switch statement
        transfers control to a statement within its body. The body
        of a switch statement may have an arbitrary number of
        unique <case> labels, for example.

>           switch (value) {
>           case 1:
>               w = "one";
>               break;
>           case 2:
>               w = "two";
>               break;
>           default:
>               w = "others";
>               break;
>           }

        If condition evaluates to the value that is equal to the
        value of one of case expressions, then control is
        transferred to the statement that is labelled with that
        expression.

        If condition evaluates to the value that does not match any
        of the 'case' labels, and the 'default' label is present,
        control is transferred to the statement labelled as the
        default label.

        The <break> statement, when encountered in statement exits
        the switch statement.

    Macro Parameters:
        expr - Switch value, which can be an integer, float or string
            expression.

    Macro Returns:
        nothing

    Macro Portability:
        n/a

    Macro See Also:
        Selection Statements, case, break

 *<<GRIEF>>
    Macro: case - Switch case statement

        case expr:

    Macro Description:
        The <switch> and 'case' statements help control complex
        conditional and branching operations.

        The body of a switch statement may have an arbitrary number
        of unique case labels.

        If condition evaluates to the value that is equal to the
        value of one of case expressions, then control is
        transferred to the statement that is labelled with that
        expression.

        If condition evaluates to the value that does not match any
        of the 'case' labels, and the 'default' label is present,
        control is transferred to the statement labelled as the
        default label.

    Macro Parameters:
        expr - Case value, which can be an integer, float or string
            expression.

    Macro Returns:
        nothing

    Macro See Also:
        Selection Statements, switch
 */
void
do_switch(void)                 /* (declare switch, list expr) */
{
    const LIST *lp = get_list(2);
    int fallthru = FALSE;
    accint_t ival;
    accfloat_t fval;
    const char *sval;
    OPCODE argtype;
    OPCODE type;
    LISTV result;

    /*
     *  decode arguments
     */
    if (NULL == lp) {
        ewprintf("switch: missing expression body");
        return;
    }

    if (listv_int(margv + 1, &ival)) {
        fval = 0; sval = "";
        argtype = F_INT;

    } else if (listv_float(margv + 1, &fval)) {
        ival = 0; sval = "";
        argtype = F_FLOAT;

    } else if (NULL != (sval = listv_xstr(margv + 1))) {
        ival = -1; fval = 0;
        argtype = F_STR;

    } else {
        ewprintf("switch: invalid argument type");
        return;
    }

    /*
     *  Walk down the switch statement/
     *      trying to find a match or the NULL representing
     *      the default case.
     *
     *  list/
     *      case + statement
     */
    ++x_switch_level;
    for (; F_HALT != *lp; lp = atom_next(lp)) {
        /*
         *  default/fallthru
         */
        if (fallthru || F_NULL == *lp) {
            lp = atom_next(lp);
            goto eval;
        }

        /*
         *  case
         */
        type = eval(lp, &result);
        if (x_switch_break) {
            break;
        }
        lp = atom_next(lp);
        switch (argtype) {
        case F_INT:         /* switch(<int>) */
            if (F_INT == type) {                /* case <int>: */
                if (ival != result.l_int) {
                    continue;
                }
            } else if (F_FLOAT == type) {       /* case <float>: */
                if (fval != result.l_float) {
                    continue;
                }
            } else {                            /* case <str>: -- extension */
                const char *sresult;

                if (NULL == (sresult = listv_xstr(&result)) ||
                        (int)strlen(sresult) != ival) {
                    continue;
                }
            }
            break;

        case F_FLOAT:       /* switch (<float>) */
            if (F_INT == type) {                /* case <int>: */
                if (fval != (accfloat_t) result.l_int) {
                    continue;
                }
            } else if (F_FLOAT == type) {       /* case <float>: */
                if (fval != result.l_float) {
                    continue;
                }
            } else {
                continue;
            }
            break;

        case F_STR:         /* switch (<str>) */
            if (F_INT == type) {                /* case <int>: -- extension */
                if (ival < 0) ival = (accint_t) strlen(sval);
                if (ival != result.l_int) {
                    continue;
                }
            } else {
                const char *sresult;
                                                /* case <str>: */
                if (NULL == (sresult = listv_xstr(&result)) ||
                        0 != strcmp(sresult, sval)) {
                    continue;
                }
            }
            break;

        default:
            continue;
        }

        /*
         *  match execute
         */
eval:;  if (F_NULL == *lp) {
            fallthru = TRUE;
            continue;
        }
        (void)eval(lp, &result);
        break;
    }
    x_switch_break = x_break = FALSE;
    --x_switch_level;
}


/*  Function:           do_if
 *      if primitive.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 *
 *<<GRIEF>>
    Macro: if - if statement

        if (expr) true-body

    Macro Description:
        The 'if' statement implements the 'if-then-else' construct.
        The else is optional. Evaluates the condition 'expr'; if the
        condition is non-zero, executes the if_statements; otherwise,
        it executes the else_statements.

>           if (condition)
>               { if_statements }
>           [else]
>               { else_statements }

    Macro See Also:
        Selection Statements, else, while

 *<<GRIEF>>
    Macro: else - else statement

        if (expr) true-body else false-body

    Macro Description:
        The 'else' statement implements the secondary condition in
        the if-then-else construct.

    Macro See Also:
        Selection Statements, else
 */
void
do_if(void)                     /* (declare expr, list true-expr, [list false-expr]) */
{
    const LIST *lp = NULL;
    LISTV result = {0};

    switch (margv[1].l_flags) {
    case F_INT:
        if (margv[1].l_int) {                   /* if (int != 0) */
            lp = margv[2].l_list;
        } else {
            lp = get_xlist(3);
        }
        break;

    case F_FLOAT:
        if (margv[1].l_float) {                 /* if (float != 0) */
            lp = margv[2].l_list;
        } else {
            lp = get_xlist(3);
        }
        break;

    case F_STR:
    case F_LIT:
    case F_RSTR: {
            const char *s = get_str(1);
            if (*s) {                           /* if (strlen(xx)) */
                lp = margv[2].l_list;
            } else {
                lp = get_xlist(3);
            }
        }
        break;

    case F_LIST:        /*extension*/
    case F_RLIST: {
            const LIST *l = get_list(1);
            if (lst_atoms_get(l)) {             /* if (length_of_list(xx)) */
                lp = margv[2].l_list;
            } else {
                lp = get_xlist(3);
            }
        }
        break;

    case F_NULL:        /*extension*/
        lp = get_xlist(3);                      /* if (NULL) */
        break;

    default:
        break;
    }

    if (lp) {
        if (F_RSTR == eval(lp, &result)) {
            /*
             *  Make sure there are > 1 reference to the result. If there's only one ref,
             *  it may be the accumulators but eval doesn't tell us how it derived the
             *  result and we could screw up badly if we dont do this.
             */
            r_inc(result.l_ref);
            acc_assign_ref(result.l_ref);
            r_dec(result.l_ref);
        }
    }
}


/*  Function:           do_refresh
 *      refresh primitive.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 *
 *<<GRIEF>>
    Macro: refresh - Update the display.

        void
        refresh()

    Macro Description:
        The 'refresh()' primitive updates the screen flushing any
        pending changes.

    Macro Parameters:
        none

    Macro Returns:
        nothing

    Macro Portability:
        n/a

    Macro See Also:
        redraw
 */
void
do_refresh(void)                /* () */
{
    vtupdate();
}


/*  Function:           do_redraw
 *      redraw primitive.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 *
 *<<GRIEF>>
    Macro: redraw - Redraw screen.

        void
        redraw([int winch])

    Macro Description:
        The 'redraw()' primitive redisplays the screen.

    Macro Parameters:
        none

    Macro Returns:
        nothing

    Macro Portability:
        n/a

    Macro See Also:
        refresh
 */
void
do_redraw(void)                 /* ([int winch]) */
{
    const int winch = get_xinteger(1, FALSE);   /* extension. force winch logic */

    vtgarbled();
    if (! winch || 1 != ttresize()) {
        vtwinch(ttcols(), ttrows());
        vtupdate();
    }
}


/*  Function:           do_insert
 *      insert and insert_process primitives.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 *
 *<<GRIEF>>
    Macro: insert - Insert string into current buffer.

        int
        insert(string str, [int num = 1])

    Macro Description:
        The 'insert()' primitive inserts the specified string 'str'
        into the current buffer. The string shall be inserted 'num'
        times, which if omitted defaults to 1.

    Macro Parameters:
        str - String value to be inserted.

        num - Option integer number stating the repeat count, if
            specified then the string is inserted the given number of
            times. If omitted, it defaults to 1.

    Macro Returns:
        The 'insert()' primitive returns the number of characters
        inserted.

    Macro Portability:
        The standard function has a void declaration and returns
        nothing.

    Macro See Also:
        insert, insertf, insert_buffer, insert_process

 *<<GRIEF>>
    Macro: insert_process - Send string to a attached process.

        int
        insert_process(string str, [int num = 1])

    Macro Description:
        The 'insert_process()' primitive inserts the specified string
        'str' into the process attached to the current buffer. The
        string shall be inserted 'num' times, which if omitted
        defaults to 1.

    Macro Parameters:
        str - String value to be inserted.

        num - Option integer number stating the repeat count, if
            specified then the string is inserted the given number
            of times. If omitted, it defaults to 1.

    Macro Returns:
        The 'insert_process()' primitive returns the number of
        characters inserted.

    Macro Portability:
        n/a

    Macro See Also:
        insert, insertf, insert_buffer, insert_process
 */
void
do_insert(int proc)             /* int (string str, [int num]) */
{
    const char *cp;
    accint_t num;
    int len;

    if ((num = get_xinteger(2, 1)) < 0) {
        num = 0;                                /* count */
    }

    /*
     *  If first argument is numeric then insert that character into the
     *  line, i.e. dont treat '\n' as a newline (extension).
     */
    if (isa_integer(1)) {
        const int value1 = get_xinteger(1, 0);

        if (value1 >= 0) {
            acc_assign_int((accint_t) num);
            while (num-- > 0) {
                linsertc(value1);
            }
            lchange(WFEDIT, 0);
        }
        return;
    }

    /*
     *  Otherwise insert the specified string
     */
    cp = get_str(1);
    len = (int)strlen(cp);
    acc_assign_int(num * len);
    if (len > 0) {
        while (num-- > 0) {                     /* MCHAR??? */
            if (proc) {
                pty_write(cp, len);             /* insert_process() */
            } else {
                linserts(cp, len);              /* insert() */
            }
        }
    }
}


/*  Function:           do_insertf
 *      insertf primitive.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 *
 *<<GRIEF>>
    Macro: insertf - Insert a formatted string.

        int
        insertf(string format, ...)

    Macro Description:
        The 'insertf()' primitive behaves like the C printf()
        function. It inserts the string 'format' into the current buffer
        and applies the printf style formatting commands as specified
        in fmt, and using the argument list.

        When more than one argument is specified then 'expr' is treated
        as a printf-style format specification. (ie. '%' sequences are
        interpreted, otherwise '%' characters are inserted literally).

    Macro Parameters:
        format - String that contains the text to be written. It can
            optionally contain an embedded <Format Specification>
            that are replaced by the values specified in subsequent
            additional arguments and formatted as requested.

        ... - Optional format specific arguments, depending on the
            format string, the primitive may expect a sequence of
            additional arguments, each containing a value to be used
            to replace a format specifier in the format string.

            There should be at least as many of these arguments as
            the number of values specified in the format specifiers.
            Additional arguments are ignored by the primitive.

    Macro Returns:
        The 'insertf()' primitive returns the number of characters
        written to the referenced buffer, otherwise -1 on error.

    Macro Portability:
        A Grief extension.

    Macro See Also:
        insert, insert_buffer
 */
void
do_insertf(void)                /* int (string fmt, ...) */
{
    const char *cp;
    int len = 0;

    if (margc > 1) {
        cp = print_formatted(0, &len);          /* sprintf style output */
    } else {
        cp = get_str(1);
    }
    if (cp) {
        linserts(cp, len);
    }
    acc_assign_int(len);
}


/*  Function:           do_insert_buffer
 *      insert_buffer primitive
 *
 *      If multiple arguments, treat as a format specification
 *      otherwise treat as a string.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 *
 *<<GRIEF>>
    Macro: insert_buffer - Insert format text into a buffer.

        int
        insert_buffer(int bufnum, string format, ...)

    Macro Description:
        The 'insert_buffer()' primitive behaves like the C printf()
        function. It inserts the string 'format' into the specified
        buffer and applies the printf style formatting commands as
        specified in 'format', and using the argument list.

        When more than one argument is specified then 'format' is
        treated as a printf-style format specification. (ie. '%'
        sequences are interpreted, otherwise '%' characters are
        inserted literally).

    Macro Parameters:
        bufnum - Buffer number.

        format - String that contains the text to be written. It can
            optionally contain an embedded <Format Specification>
            that are replaced by the values specified in subsequent
            additional arguments and formatted as requested.

        ... - Optional format specific arguments, depending on the
            format string, the primitive may expect a sequence of
            additional arguments, each containing a value to be used
            to replace a format specifier in the format string.

            There should be at least as many of these arguments as
            the number of values specified in the format specifiers.
            Additional arguments are ignored by the primitive.

    Macro Returns:
        The 'insert_buffer()' primitive returns the number of
        characters written to the referenced buffer, otherwise -1 on
        error.

    Macro Portability:
        The CRiSPEdit(tm) version has a void declaration and returns
        nothing.

    Macro See Also:
        insert, insertf
 */
void
do_insert_buffer(void)          /* int (int bufnum, string | expr ....) */
{
    BUFFER_t *bp;
    const char *cp;
    int len = 0;

    if (NULL == (bp = buf_lookup(get_xinteger(1, -1)))) {
        ewprintf("insert_buffer: no such buffer");
        acc_assign_int(-1);
        return;
    }

    if (margc > 2) {
        cp = print_formatted(1, &len);          /* sprintf style output */
    } else {
        cp = get_str(2);                        /* just quote string */
    }

    if (cp) {
        BUFFER_t *saved_bp = curbp;

        if (bp != curbp) {
            curbp = bp;
            set_hooked();
        }
        linserts(cp, len);
        curbp = saved_bp;
        set_hooked();
    }
    acc_assign_int(len);
}


/*  Function:           do_self_insert
 *      self_insert primitive
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 *
 *<<GRIEF>>
    Macro: self_insert - Insert a character as if it was typed.

        void
        self_insert([int character])

    Macro Description:
        The 'self_insert()' primitive insert a character into the
        current buffer. If 'character' is specified, then the
        character whose ASCII value is 'character' is inserted into
        the current buffer instead of the last character typed.

        The majority of characters are directly inserted yet the
        following infer special processing.

            tab - Cursor is moved the next tab stop, and space
                backfilled if at the end of the line.

            newlines - Cursor is repositioned on the next line.

        This primitive is normally used in conjunction with
        <assign_to_key> to unassign an ASCII key by making it a
        normal, typeable character.

    Macro Parameters:
        character - Optional integer character code to be inserted.
            If omitted, the value of the last key typed is inserted
            into the buffer.

    Macro Returns:
        nothing

    Macro Portability:
        n/a

    Macro See Also:
        insert, insertf, assign_to_key, keyboard_typeables
 */
void
do_self_insert(void)            /* void ([int character]) */
{
    int32_t character = x_character;            /* last character */
    int typed = TRUE;

    if (isa_integer(1)) {                       /* parameterised otherwise global */
        character = get_xinteger(1, 0);
    }

    if (character <= 0) {
        return;
    }

    trace_log("self_insert(%s, 0x%x/%d)\n",
        key_code2name(character), (unsigned)character, (int)character);

    x_selfinsert = TRUE;                        /* inform undo engine */

    if (buf_imode(curbp)) {
        /*
         *  insert mode
         */
        switch (character) {
        case '\t': {            /* next tab stop */
                const int ntab = ruler_next_indent(curbp, *cur_col) + 1;
                int success = TRUE;

                while (*cur_col < ntab) {       /* insert spaces */
                    if (FALSE == (success = linsertc(' '))) {
                        break;                  /* error, most likely read-only */
                    }
                }

                if (success) {
                    if (BFTST(curbp, BF_TABS)) {
                        trace_log("backfill\n");
                        line_tab_backfill();    /* backfill with tabs */
                    }
                }
            }
            break;

        case '\r':              /* newline */
        case '\n':
            lnewline();
            break;

        default:                /* others */
            linsertc(character);
            break;
        }

    } else {
        /*
         *  overstrike mode
         */
        switch (character) {
        case '\t':              /* next tab stop */
            u_dot();
            *cur_col = ruler_next_indent(curbp, *cur_col);
            typed = FALSE;
            break;

        case '\r':              /* newline */
        case '\n':
//TODO      if (OVMODEF_NL & xf_ovmode) {
            if (XF_TEST(4) && !BFTST(curbp, BF_SYSBUF)) {
                lnewline();
            } else {
                mov_forwline(1);
                *cur_col = 1;
                typed = FALSE;
            }
            break;

        default:                /* others */
            lwritec(character);
            break;
        }
    }

    x_selfinsert = FALSE;
    curbp->b_uchar = character;
    if (typed) {
        trigger(REG_TYPED);
    }
}


/*  Function:           do_version
 *      version primitive.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 *
 *<<GRIEF>>
    Macro: version - Version information

        int
        version([int major | string machtype],
                [int min], [int edit], [int release], [string machtype],
                    [string compile], [int cmversion],
                    [string features], [string build])

    Macro Description:
        The 'version()' primitive retrieves the version information
        associated with the current GRIEF installation.

        If the first argument is omitted, displays the current
        version and build information on the command prompt, for
        example:

>           GRIEF v3.2.0 compiled Aug 20 2014 20:05:04

    Macro Parameters:
        The first parameter may be either an integer or string
        variable, which shall be populated with the major version or
        the machine type representatively.

        All additional parameters are either integer or string
        variable references which are populated with their associated
        value.

            major - Integer major version number.

            min - Integer minor version number.

            edit - Integer sub version number.

            machtype - Machine type labels, value include
                "DOS", "OS/2", "UNIX" and "VMS".

            release - Reserved for future use.

            compiled - GRIEF engine compilation timestamp.

            cmversion - Macro compiler language version.

            features - String of comma separated built-in features
                (reserved for future use).

            build - Populated with the host build label.

    Macro Returns:
        The 'version()' primitive returns the current version multiplied 
	by 100, plus the minor; for example '301' represents version '3.1'.

    Macro Portability:
        All the arguments are extensions.

    Macro See Also:
        grief_version
 */
void
do_version(void)                /* int ([maj|mach], [min], [edit], [release], [machtype],
                                         [compile], [cmversion], [features], [build]) */
{
   /*   argument1:
    *       NULL        - echo
    *       INT         - returns 'major'.
    *       STRING      - returns 'machtype'.
    */
    if (isa_undef(1)) {
        ewprintf("%s %s compiled %s", x_appname, x_version, x_compiled);
        return;
    }

    if (isa_integer(1)) {
        argv_assign_int(1, (accint_t) x_major_version);

    } else if (isa_string(1)) {
        argv_assign_str(1, x_machtype);
    }

    argv_assign_int(2,  (accint_t) x_minor_version);
    argv_assign_int(3,  (accint_t) x_edit_version);
    argv_assign_int(4,  (accint_t) 0);          /* todo - o/s release */
    argv_assign_str(5,  x_machtype);            /* DOS, OS/2, UNIX and VMS. */
    argv_assign_str(6,  x_compiled);
    argv_assign_int(7,  (accint_t) cm_version);
    argv_assign_str(8,  "");                    /* todo - features */
    argv_assign_str(9,  GR_BUILD);              /* see edpackage.h */

    acc_assign_int((accint_t) x_major_version * 100 + x_minor_version);
}


/*<<GRIEF>>
    Macro: grief_version - GRIEF version

        int
        grief_version()

    Macro Description:
        The 'grief_version()' primitive retrieves the current GRIEF
        version.

    Macro Parameters:
        none

    Macro Returns:
        The 'grief_version' primitive returns the current version
        multiplied by 100 plus the minor; for example '101'
        represents version '1.1'.

    Macro Portability:
        A Grief extension.

    Macro See Also:
        version
 */
void
do_grief_version(void)
{
    acc_assign_int(101);
}


/*  Function:           do_return
 *      return and returns primitive
 *
 *  Parameters:
 *      returns -           TRUE if returns otherwise return.
 *
 *  Returns:
 *      nothing.
 *
 *<<GRIEF>>
    Macro: return - Return from a macro.

        return [<expression>];

    Macro Description:
        The 'return()' primitive terminate the current macro and
        returns the given value 'expression'. If specified
        'expression' may be an integer, string or list expression
        which matches the return type of the associated macro.

        If <return> or <returns> is not called, the return value for
        a macro is indeterminate.

    Macro Parameters:
        expression - Optional value to be returned, which should
            match the return type of the associated macro.

    Macro Returns:
        nothing

    Macro See Also:
        returns, exit
 *
 *<<GRIEF>>
    Macro: returns - Return an expression from a macro.

        returns (expression);

    Macro Description:
        The 'returns()' primitive sets the return value of the
        current macro to 'expression', which may be an integer,
        string or list expression which matches the return type of
        the associated macro.

        This primitive is similar to the <return> statement, except
        it does not cause the current macro to terminate. It simply
        sets *GRIEF's* internal accumulator with the value of the
        expression; as such any following may overwrite the value.

        If <return> or <returns> is not called, the return value for
        a macro is indeterminate.

    Note!:
        This primitive is not strictly compatible with the 'returns'
        macro of BRIEF and is not generally recommended as statements
        following may have side effects.

    Macro Parameters:
        expression - Value to be returned, which should match the
            return type of the associated macro.

    Macro Returns:
        nothing

    Macro See Also:
        Jump Statements, return, exit
 */
void
do_returns(int returns)         /* (expr) */
{
    if (returns) {                              /* returns; */
        if (NULL == x_returns) {
            x_returns = obj_alloc();
        }
        obj_assign_argv(x_returns, margv + 1);

    } else {                                    /* return; */
        acc_assign_argv(margv + 1);
        x_return = TRUE;
    }
}

/*  Function:           do_first_time
 *      first_time primitive
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 *
 *<<GRIEF>>
    Macro: first_time - Determine a macros initialisation status.

        int
        first_time()

    Macro Description:
        The 'first_time()' primitive returns *true* if this is the
        first invocation of the calling macro since loading.

        Each macro maintains a 'first_time' status, which during the
        first execution of the macro is *true* and all subsequent
        calls shall be *false. This primitive is generally used to
        perform run-time subsystem initialisation, for example.

>           if (first_time()) {
>               initialise();
>           }

        The loading of macro resets the first_time flag, is effect
        deleting the macro and reloading will look like it was never
        loaded.

        Note!:
        A GRIEF macro compiler utilises this primitive to perform
        run-time initialisation of function <static> variables.

    Macro Parameters:
        none

    Macro Returns:
        The 'first_time()' primitive returns *true* (non-zero) if the
        first invocation of the associated macro, zero otherwise.

    Macro Portability:
        n/a

    Macro See Also:
        static
 */
void
do_first_time(void)             /* int () */
{
    MACRO *mp;

    if (0 == ms_cnt) {
        acc_assign_int((accint_t) 0);
        return;
    }
    mp = macro_lookup(mac_stack[ms_cnt - 1].name);
    acc_assign_int((accint_t) mp->m_ftime);
}


/*  Function:           do_read
 *      read primitive
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 *
 *<<GRIEF>>
    Macro: read - Read characters from the buffer.

        string
        read([int number], [int &status])

    Macro Description:
        The 'read()' primitive reads characters up to the specified
        number of characters 'length', if omitted characters are read
        up to the end of the current buffer including the new-line
        terminator.

        The current buffer position remains unchanged.

    Macro Parameters:
        number - An optional number of characters to be read.

        status - Optional integer variable reference to be populated
            with the read completion status, representing the
            position of the cursor at the end of the read operation;

                o 0   - Partial, within a line.
                o 1   - End of line.
                o -1  - End of file.

    Macro Returns:
        The 'read()' primitive returns the string read.

    Macro Portability:
        n/a

    Macro See Also:
        insert, insertf
 */
void
do_read(void)                   /* string ([int number], [int &status]) */
{
    const LINENO cline = *cur_line, ccol = *cur_col;
    int status = 0;                             /* extension, status being -1=eof,1=eol,0=partial */
    LINE_t *lp;

    if (NULL == (lp = vm_lock_linex(curbp, cline))) {
        trace_ilog("read(line:%d) : -1\n", (int)cline);
        acc_assign_str("", 0);                  /* eof */
        status = -1;

    } else {
        int dot, len, copy;

        dot = line_offset2(lp, cline, ccol, LOFFSET_NORMAL);
        len = llength(lp) - dot;

        trace_ilog("read(line:%d,col:%d,dot:%d,length:%d) : %d\n", \
            (int)cline, (int)ccol, dot, llength(lp), len);

        if (isa_undef(1)) {
            copy = len;                         /* EOL */

        } else {
            int value1 = get_xinteger(1, 0);    /* user length, min(user/length) */

            if (value1 <= 0) value1 = 1;
            copy = (value1 > len ? len : value1);
        }

        if (copy <= 0) {
            acc_assign_str("\n", 1);            /* empty line */

        } else if (copy < len) {                /* sub-line */
            acc_assign_str((const char *) ltext(lp) + dot, copy);
            status = 0;

        } else {                                /* copied to EOL, must add \n */
            acc_assign_str2((const char *) ltext(lp) + dot, copy, "\n", 1);
        }

        vm_unlock(cline);
    }

    argv_assign_int(2, (accint_t) status);
}


/*  Function:           do_sleep
 *      sleep primitive.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 *
 *<<GRIEF>>
    Macro: sleep - Suspend execution for an interval of time.

        void
        sleep([int seconds = 1], [int milliseconds = 0])

    Macro Description:
        The 'sleep()' primitive causes the caller to be suspended
        from execution until either the specified interval specified
        by 'seconds' and 'milliseconds' has elapsed.

        Note!:
        The suspension time may be longer than requested due to the
        scheduling of other activity by the system.

    Macro Parameters:
        seconds - Optional positive integer stating the time-out
            interval seconds component, if omitted defaults to 1
            second.

        milliseconds - Option positive integer stating the time-out
            interval milliseconds component.

    Macro Returns:
        nothing

    Macro Portability:
        n/a

    Macro See Also:
        inq_clock, time
 */
void
do_sleep(void)                  /* void ([int seconds = 1], [int milliseconds]) */
{
    accint_t ms = get_xaccint(2, 0);            /* extension */
    accint_t seconds = get_xaccint(1, (ms > 0 ? 0 : 1));

    if (seconds > 0 || ms > 0) {
        if (seconds < 0) seconds = 0;
        if (ms < 0) ms = 0;
        io_get_key((EVT_SECOND(seconds)) + ms); /* FIXME - will unblock on any i/o */
    }
}


/*  Function:           do_unimp
 *      unimpl, internal method for unimplemented primitives.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 */
void
do_unimp(void)                  /* void (...) */
{
    ewprintf("*** <%s> not yet implemented.", x_command_name);
}


/*  Function:           do_input_mode
 *      input_mode primitive.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 *
 *<<GRIEF>>
    Macro: input_mode - Effect of certain system keys.

        int
        input_mode(int char, int flag)

    Macro Description:
        The 'input_mode()' primitive controls the effect of certain
        characters when typed, setting the actions of character
        'char' to the desired 'state'.

        Normally the keyboard driver acts internal on specific
        characters which perform the control actions of XON/XOFF and
        the Job control stop; specifically Ctrl-S (0x13), Ctrl-Q (0x11)
        and Ctrl-Z (0x1a).

    Macro Parameters:
        char - Integer value of the ascii character to be effected.

        flags - Boolean flag, if 'true' enable the specified character
            to flow thru to GRIEF otherwise *false* to reset the default
            behaviour.

    Macro Returns:
        The 'input_mode()' primitive returns 1 if character previously
        enabled; zero otherwise.

    Macro Portability:
        n/a

    Macro See Also:
        inq_keyboard
 */
void
do_input_mode(void)             /* int (int char, int flag) */
{
    const int ch = get_integer(1);
    const int value = get_integer(2);

    acc_assign_int((accint_t)sys_enable_char(ch, value));
}

/*end*/
