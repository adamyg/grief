/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: brace.cr,v 1.8 2014/10/27 23:28:18 ayoung Exp $
 * Brace matching.
 *
 *
 */

#include "grief.h"

/*  Function:       find_matching_brace
        Find the matching brace

    Description:
        This macro is an attempt at a "locate the matching brace" utility. Although it
        attempts to be fairly smart about things, it has an IQ of 4 or 5, so be careful
        before taking its word for something.

    Notes:
        It DOES NOT WORK if there are braces inside comments.

        The macro can, however, be modified to ignore everything inside these
        structures (and check for the appropriate mismatches).

**/
void
find_matching_brace()
{
#define C_BRACKETS  "[](){}"

    string bracket, mbracket, s, e, c;
    int i, lvl, incr, fwd;
    int ins, inq;                               /* in string or quote */

    save_position();
    bracket = read(1);
    if (index(bracket, C_BRACKETS) == 0) {
        if (re_search(SF_UNIX, "[()\\[\\]\\{\\}]") <= 0) {
            restore_position();
            beep();
            return;
        }
        bracket = rtrim(read(1));
    }

    message("Locating matching bracket...");
    i = index(C_BRACKETS, bracket) - 1;
    fwd = i & 1;
    incr = -1;
    if (fwd)
        incr = 1;
    i = 2 * i + 1;
    mbracket = substr("[][]()(){}{}", i, 2);
    s = "[" + quote_regexp(mbracket) + "\"\']"; /* match plus quote */
    e = substr(mbracket, 1, 1);                 /* end/closing */

    lvl = 1;
    while (lvl > 0)
    {
        if (fwd) {
            prev_char();
            if (re_search(SF_BACKWARDS, s) <= 0)
                break;

        } else {
            next_char();
            if (re_search(NULL, s) <= 0)
                break;
        }

        c = rtrim(read(1));
        if (c == "\"") {
            if (inq == 0) {
                prev_char();
                if (read(1) != "\\") {          /* quoted?? */
                    ins ^= 1;
                } else {
                    prev_char();
                    if (read(1) == "\\")
                        ins ^= 1;
                    next_char();
                }
                next_char();                    /* .. move past match */
            }

        } else if (c == "\'") {
            if (ins == 0) {
                prev_char();
                if (read(1) != "\\") {          /* quoted?? */
                    inq ^= 1;
                } else {
                    prev_char();
                    if (read(1) == "\\")
                        inq ^= 1;
                    next_char();
                }
                next_char();                    /* .. move past match */
            }

        } else if (ins == 0 && inq == 0) {
            if (c == e)
                lvl -= incr;
            else lvl += incr;
        }
    }

    if (lvl) {
        error("Matching %s not found ...", mbracket);
        restore_position();
        beep();
        return;
    }

    restore_position(0);
    message("");
}


#define P_BRACKETS      "BEGINEND"
#define P_SEARCHSTRING  "{{{<|[~0-z]}begin{[~0-z]|$}}|{{<|[~0-z]}end{[~0-z]|$}}}"

void
f_m_b()
{
    string bracket, mbracket, s, e;
    int lvl, incr, fwd;

    save_position();
    bracket = substr(trim(upper(read())), 1, 3);
    if (index(P_BRACKETS, bracket) == 0) {
        if (re_search(SF_IGNORE_CASE, P_SEARCHSTRING) <= 0) {
            restore_position();
            beep();
            return;
        }
        bracket = substr(trim(upper(read())), 1, 3);
    }

    message("Locating matching bracket...");
    if (bracket == "BEG") {
        fwd = 0;
        incr = -1;
        mbracket = "END";
    } else {
        fwd = 1;
        incr = 1;
        next_char(1);
        mbracket = "BEGIN";
    }
    s = P_SEARCHSTRING;
    e = "BEG";

    lvl = 1;
    while (lvl > 0)
    {
        if ((incr == 1) && (fwd == 1))
        {
            /*  This is because otherwise matching lvl gets to one
             *  more than reqd in case of finding a matching BEGIN
             */
            lvl = 0;
            fwd = 2;
        }

        if (fwd) {
            prev_char();
            if (re_search(SF_IGNORE_CASE | SF_BACKWARDS, s) <= 0)
                break;
        } else {
            next_char();
            if (re_search(SF_IGNORE_CASE, s) <= 0)
                break;
        }

        if (substr(trim(upper(read())), 1, 3) == e)
            lvl -= incr;
        else
            lvl += incr;
    }

    if (lvl)
    {
        error("Matching %s not found.", mbracket);
        restore_position();
        beep();
        return;
    }
    else
    {
        restore_position(0);
        message("Matching %s found.", mbracket);
    }
}


/*  Function:       check_braces
        Check for unmatched braces

    Description:
        This macro is an attempt at a "locate the unmatched braces" utility. Although
        it attempts to be fairly smart about things, it has an IQ of 4 or 5, so be
        careful before taking its word for something.

    Notes:
        It DOES NOT WORK if there are braces inside quotes, apostrophes, or comments.
        The macro can, however, be modified to ignore everything inside these
        structures (and check for the appropriate mismatches).

**/

#define FWD             0
#define BACK            1
#define DONE            2

static string           brace_pattern = "[\\{\\}]";

static int
_brace_charsearch(int direction)
{
    return (direction) ?
        search_back(brace_pattern) : search_fwd(brace_pattern);
}


void
check_braces(void)
{
    int tot_count, start_line, start_col, mismatch, char_we_got, direction;
    string char_holder, msg_pattern, msg_text;

    msg_pattern = "Checking braces, %d unmatched {s.";
    inq_position(start_line, start_col);
    top_of_buffer();
    tot_count = 0;
    mismatch = FALSE;
    direction = FWD;
    while (direction != DONE)
    {
        while (_brace_charsearch(direction) && !mismatch)
        {
            sprintf(msg_text, msg_pattern, tot_count);
            message(msg_text);
            char_holder = read(1);
            if (direction == BACK)
                char_we_got = index("}{", char_holder);
            else
                char_we_got = index("{}", char_holder);

            if (char_we_got == 1)
                ++tot_count;
            else
            {
                if (char_we_got == 2)
                {
                    if (tot_count)
                        --tot_count;
                    else
                    {
                        if (direction == BACK)
                            message("Mismatched opening brace.");
                        else
                            message("Mismatched closing brace.");
                        mismatch = TRUE;
                    }
                }
            }

            if (!mismatch)
            {
                if (direction == BACK)
                    prev_char(1);
                else
                    next_char(1);
            }
        }

        if (!mismatch)
        {
            if (tot_count)
            {
                end_of_buffer();
                tot_count = 0;
                direction = BACK;
                msg_pattern = "Locating mismatch, %d unmatched }s.";
            }
            else
                direction = DONE;
        }
        else
            direction = DONE;
    }

    if (!mismatch)
    {
        message("All braces match.");
        move_abs(start_line, start_col);
    }
}

/*end*/
