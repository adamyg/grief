/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: hpcalc.cr,v 1.8 2014/10/27 23:28:23 ayoung Exp $
 * HP48 calculator emulator.
 *
 *
 */

#include "grief.h"

static list          stack;
static int           stk_ptr = 0;
static int           hp_base = 16;              /* Default base for binary/real conversions */

extern int           popup_level;

static void          hp_func(string str);
static int           need_arg(int n);
static void          redraw_stack(void);
static int           getval(string str);
static float         getfval(string str);
static void          binop(string func);

/*
 *  List of functions which take a single argument
 */
static list          unary_fns =
   {
      "SIN",   "COS",   "TAN",   "ASIN",  "ACOS",  "ATAN",
      "SINH",  "COSH",  "TANH",
      "SQRT",  "EXP",   "LOG",   "LOG10"
   };


void
hpcalc(void)
{
    int curbuf, curwin, buf, win;

    curbuf = inq_buffer();
    curwin = inq_window();
    buf = create_buffer("Calculator", NULL, 1);
    win = sized_window(10, 23, "<Alt-H> for Help");
    set_window(win);
    set_buffer(buf);
    attach_buffer(buf);
    keyboard_push();
    keyboard_typeables();
    assign_to_key("<Esc>",              "exit");
    assign_to_key("<Enter>",            "hp_enter 1");
    assign_to_key("<Backspace>",        "hpk_drop");
    assign_to_key("<F10>",              "execute_macro");
    assign_to_key("<Right-arrow>",      "hpk_right");
    assign_to_key("+",                  "binop \"+\"");
    assign_to_key("-",                  "hpk_minus");
    assign_to_key("*",                  "binop \"*\"");
    assign_to_key("/",                  "binop \"/\"");
    assign_to_key("'",                  "hpk_alg");
    assign_to_key("<Left-arrow>",       "left");
    assign_to_key("<Right-arrow>",      "right");
    assign_to_key("<Del>",              "delete_character");
    assign_to_key("<Alt-B>",            "hpk_to_binary");
    assign_to_key("<Alt-R>",            "hpk_to_real");
    redraw_stack();

#define STACK_SIZE          8

    ++popup_level;
    process();
    --popup_level;

    delete_buffer(buf);
    delete_window();

    set_buffer(curbuf);
    set_window(curwin);
    attach_buffer(curbuf);
    keyboard_pop();
    refresh();
}


void
hpk_alg(void)
{
    insert("''");
    left();
}


void
hpk_drop(void)
{
    int col;

    inq_position(NULL, col);
    if (col == 1 && read(1) == "\n") {
        if (stk_ptr == 0) {
            beep();
            error("Stack is empty.");
        }
        else
        stk_ptr--;
        redraw_stack();
    } else {
        backspace();
    }
}


void
hp_enter(int dup)
{
    string str, s2;

    beginning_of_line();
    str = compress(trim(read()));
    delete_line();
    if (str == "") {
        if (dup && stk_ptr) {
            stack[stk_ptr] = stack[stk_ptr - 1];
            stk_ptr++;
        }
    } else {
        for (s2 = str; s2 != ""; str = s2) {
            int i = index(str, " ");

            if (i == 0) {
                s2 = "";
            } else {
                s2 = substr(str, i + 1);
                str = substr(str, 1, i - 1);
            }
            hp_func(str);
        }
    }
    redraw_stack();
}


static void
hp_func(string str)
{
    int j;
    float f;
    string buf;

    if ((j = re_search(NULL, "^" + str + "$", unary_fns)) >= 0) {
        if (need_arg(1))
            return;
        f = getfval(stack[stk_ptr - 1]);
    }

    switch (str)
    {
    case "DEPTH":
        stack[stk_ptr++] = stk_ptr;
        break;
    case "R->B":
        if (need_arg(1))
            return;
        j = atoi(stack[stk_ptr - 1], 1);
        switch (hp_base)
        {
        case 8:
            sprintf(buf, "#%loo", j);
            break;
        case 10:
            sprintf(buf, "#%ldd", j);
            break;
        case 16:
            sprintf(buf, "#%lxh", j);
            break;
        }
        stack[stk_ptr - 1] = buf;
        break;
    case "B->R":
        if (need_arg(1))
            return;
        j = getval(stack[stk_ptr - 1]);
        sprintf(buf, "%ld", j);
        stack[stk_ptr - 1] = buf;
        break;
    case "HEX":
        hp_base = 16;
        break;
    case "DEC":
        hp_base = 10;
        break;
    case "BIN":
        hp_base = 2;
        break;
    case "OCT":
        hp_base = 8;
        break;
    case "SIN":
        stack[stk_ptr - 1] = sin(f);
        break;
    case "COS":
        stack[stk_ptr - 1] = cos(f);
        break;
    case "TAN":
        stack[stk_ptr - 1] = tan(f);
        break;
    case "ASIN":
        stack[stk_ptr - 1] = asin(f);
        break;
    case "ACOS":
        stack[stk_ptr - 1] = acos(f);
        break;
    case "ATAN":
        stack[stk_ptr - 1] = atan(f);
        break;
    case "SINH":
        stack[stk_ptr - 1] = sinh(f);
        break;
    case "COSH":
        stack[stk_ptr - 1] = cosh(f);
        break;
    case "TANH":
        stack[stk_ptr - 1] = tanh(f);
        break;
 /*
  * Not currently supported. (ANSI C doesn't define these).
  *
  * case "ASINH":
  *     stack[stk_ptr - 1] = asinh(f);
  *     break;
  * case "ACOSH":
  *     stack[stk_ptr - 1] = acosh(f);
  *     break;
  * case "ATANH":
  *     stack[stk_ptr - 1] = atanh(f);
  *     break;
  */
    case "SQRT":
        stack[stk_ptr - 1] = sqrt(f);
        break;
    case "EXP":
        stack[stk_ptr - 1] = exp(f);
        break;
    case "LOG":
        stack[stk_ptr - 1] = log(f);
        break;
    case "LOG10":
        stack[stk_ptr - 1] = log10(f);
        break;
    default:
        stack[stk_ptr++] = str;
    }
}


/*
 *  Check we have enough arguments on the stack
 */
static int
need_arg(int n)
{
    if (stk_ptr == 0) {
        error("Stack is empty.");
        beep();
        return 1;
    }
    if (stk_ptr < n) {
        error("Not enough arguments on stack.");
        beep();
        return 1;
    }
    return 0;
}


void
hpk_right(void)
{
    declare p1, p2;

    if (read(1) == "\n") {
        if (stk_ptr > 1) {
            p1 = stack[stk_ptr - 1];
            p2 = stack[stk_ptr - 2];
            stack[stk_ptr - 1] = p2;
            stack[stk_ptr - 2] = p1;
            redraw_stack();
        }
    } else {
        right();
    }
}


static void
redraw_stack(void)
{
    int i, j, llen;
    string buf;

    clear_buffer();
    llen = length_of_list(stack);
    for (i = 1; i <= STACK_SIZE; i++) {
        j = stk_ptr - i;
        if (j < 0) {
            sprintf(buf, "%d:\n", i);
        } else {
            sprintf(buf, "%d: %19t\n", i, stack[j]);
        }
        insert(buf);
        up();
    }
    end_of_buffer();
    message("");
}


void
hpk_minus(void)
{
    int col;
    string ch;

    inq_position(NULL, col);

    /* If we're at the start of the line, then minus is an operator */
    if (col == 1) {
        binop("-");
        return;
    }

    /* If the previous character is an alphabetic then '-' is part of the
     * symbol/function name
     */
    left();
    ch = upper(read(1));
    right();
    if (ch >= "A" && ch <= "Z")
        insert("-");
    else
        binop("-");
}


/*
 *  Parse string as an integer number, possibly in a different base
 */
static int
getval(string str)
{
    int val = 0;
    int base = hp_base;
    string ch;

    /* If it looks like a C hex number then convert to HP style */
    if (substr(str, 2, 1) == "x" || substr(str, 2, 1) == "X")
        str = "#" + substr(str, 3) + "h";

    if (substr(str, 1, 1) != "#")
        return atoi(str, 1);

    switch (substr(str, strlen(str)))
    {
    case "b":
        base = 2;
        str = substr(str, 2, strlen(str) - 2);
        break;
    case "o":
        base = 8;
        str = substr(str, 2, strlen(str) - 2);
        break;
    case "d":
        base = 10;
        str = substr(str, 2, strlen(str) - 2);
        break;
    case "h":
        base = 16;
        str = substr(str, 2, strlen(str) - 2);
        break;
    default:
        str = substr(str, 2);
        break;
    }

    while (str != "") {
        ch = substr(str, 1, 1);
        if (ch >= "0" && ch <= "9")
            val = val * base + atoi(ch, 0) - '0';
        else
            val = val * base + atoi(ch, 0) - 'a' + 10;
        str = substr(str, 2);
    }
    return val;
}


/*
 *  Parse string as a floating point number
 */
static float
getfval(string str)
{
    float zero = 0;

    /* If no decimal point, then treat it as an integer but cast to a float */
    if (index(str, ".") == 0) {
        return zero + getval(str);
    }
    return cvt_to_object(str);
}


static void
binop(string func)
{
    string buf, tmp;
    declare s1, s2;
    int i1;
    float f1;
    int base_flag = FALSE;

    hp_enter(0);
    if (need_arg(2))
        return;

    /* Get first operand and convert to float */
    s1 = stack[stk_ptr - 1];
    if (substr(s1, 1, 1) == "#") {
        base_flag = TRUE;
        sprintf(s1, "0x%s", substr(s1, 2, strlen(s1) - 2));
    }
    s1 = execute_macro("+ " + s1 + " 0.0");

    /* Get 2nd operand and convert to float */
    s2 = stack[stk_ptr - 2];
    if (substr(s2, 1, 1) == "#") {
        base_flag = TRUE;
        sprintf(s2, "0x%s", substr(s2, 2, strlen(s2) - 2));
    }
    s2 = execute_macro("+ " + s2 + " 0.0");

    /* Pop args from stack */
    stk_ptr -= 2;
    sprintf(tmp, "%s %f %f", func, s2, s1);
    sprintf(buf, "%t", execute_macro(tmp));

    /* If either operand was in binary then convert result to binary */
    if (base_flag) {
        f1 = cvt_to_object(buf);
        i1 = f1;

        sprintf(buf, hp_base == 8 ? "#%loo" :
            hp_base == 10 ? "#%ldd" : "#%lxh", i1);
    }

    /* Save result on stack */
    stack[stk_ptr++] = buf;

    redraw_stack();
}


void
hpk_to_binary()
{
    insert(" R->B");
    hp_enter(1);
}


void
hpk_to_real()
{
    insert(" B->R");
    hp_enter(1);
}

/*eof*/
