/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: regress.cr,v 1.37 2021/06/19 09:39:45 cvsuser Exp $
 *
 *  This set of macros are used when debugging and fixing CRISP to aid in regression testing and
 *  catching bugs introduced inadvertently. These tests dont attempt an exhaustive test, yet
 *  tests are added whenever a bug is found and upon the development or extension of key festures,
 *  to ensure the condition does not get overlooked in the future.
 *
 *  The tests in this file are mainly to do with testing the interpreter and simple aspects of
 *  the language and primitives. No attempt is made to test the correctnesss of the display, or
 *  reading/writing files.
 *
 *  This file can also be run after porting the editor, to ensure that these tests work as expected.
 *  If anything doesn't work that should, the porter will have to check for portability problems.
 *  These tests attempt to do things in order of complexity.
 *
 *  See Also:   regress2.cr
 */

#include "../grief.h"
#include "../debug.h"

#define __TEST(__x, __cond) \
            if (!(__cond)) failed(__x, #__cond); else passed(__x);
#define TEST(x, cond)   __TEST(x, cond)

#define TESTASYNC(__x, __cond) \
            {extern int x_lastnum; x_lastnum=0;} if (!(__cond)) failed(__x, #__cond); else passed(__x);

#if defined(__PROTOTYPES__)
static void             passed(int num);
static void             failed(int num, string statement);

static int              test_basic(void);
static int              test_1(~string, ~int, ~string, ~int);
static void             test_2(...);
static int              test_4(void);
static int              test_6(void);
static int              test_7(void);
static int              test_8(~declare v1, ~declare v2, ~declare v3, ~declare v4,
                            ~declare v5, ~declare v6, ~declare v7, ~declare v8, ~declare v9);
static int              test_9(~string, ~int, ~string, ~int);
static int              test_10(int value);

static void             test_returns(void);
static int              returns_1(void);
static string           returns_2(void);
static string           returns_3(void);
static int              returns_4(void);
static string           returns_5(void);
static void             test_operators(void);
static void             test_arith(void);
static void             test_typeof(void);
static void             test_for(void);
static void             test_while(void);
static void             test_do(void);
static void             test_switch(void);
static void             test_cexpr(void);
static void             test_math(void);
static void             test_cast(void);
static string           switchi(int ival);
static int              switchs(string sval);
static void             test_escapes(void);
static void             test_string(void);
static void             test_substr(void);
static void             test_regexp(void);
static void             test_sub(void);
static void             test_compress(void);
static void             test_list(void);
static void             test_list2(void);
static void             test_list3(void);
static void             test_nth(void);
static void             test_splice(void);
static void             splice_1(void);
static void             splice_2(void);
static void             splice_3(void);
static void             splice_4(void);
static void             test_sort_list(void);
static void             test_sprintf(void);
static void             test_replacement(void);
static void             test_isa(void);
static void             test_basedir(void);
static void             test_env(void);
static void             test_register(void);
static void             test_buffer(void);
static void             test_key(void);
static void             test_debug(void);
static void             test_defaults(void);
static void             test_reference(void);
static void             test_strtol(void);
static void             test_strtof(void);
static void             test_strtod(void);
static void             test_pushpop(void);
static void             test_globals(void);
static void             test_search(void);
static void             test_called(void);
static string           caller(void);
static void             test_history(void);

static void             test_scope(void);
static int              scope_1(void);
static int              scope_2(void);
static int              scope_3(void);

static void             test_dict(void);
static void             test_macro(void);
static void             test_undef(void);

static int              test_leaks(void);
static int              leak_1(int count);

static int              test_if(declare val);
static int              test_ifbug(void);
static int              test_elsif1(void);
static int              test_elsif2(void);

string                  xcompletion(string file);
string                  xcompl_file(string file);

extern void             hello(void);
extern void             regress2(void);
#endif  /*__PROTOTYPES__*/

static list             splice_l1 = {
    { "aaa",  "bbb"  },
    { "ccc",  "ddd",
        { "eee", "fff", { 100, 1000 }, 1, 2, 3 } },
    { "ggg",  "hhh"  }
    };

static string           gs1, gs2, gs3, gi, gj, gk;

static list             gl1 = {
    "One",
    "Two",
    { "Three",
        { "2.1.0", "2.1.1" },
    },
    { "x", "y" },
    "fred",
    { "Off", "5.1", "On" },
    };

static list             gl2 = {
    2, 1, 1,
    };

extern int              x_extern_dontexist;
static int              x_static = 1;
int                     x_global;

#pragma message("regress.cr: regression test module")

void
main(void)
{
    module("regress");
    require("demos/regress2");
}


void
regress(~string test)
{
    int num_passed = 0, num_failed = 0, x_lastnum = 0;
    list failed_list;
    int opause;
    int i;

    UNUSED(x_lastnum);

    refresh();
    opause = pause_on_error(1, FALSE);
    if (is_string(test) && strlen(test)) {
        test = "regress::test_" + test;
        if (inq_macro(test, 0x01)) {
            execute_macro(test);
        } else {
            error("No matching test case ..");
        }
    } else {
        test_basic();
        test_returns();                         // 30/04/10
        test_operators();
        test_arith();
        test_typeof();
        test_for();
        test_while();
        test_do();
        test_switch();
        test_cexpr();
        test_math();
        test_cast();
        test_escapes();
        test_string();
        test_regexp();
        test_sub();
        test_substr();
        test_compress();
        test_list();
        test_list2();
        test_list3();
        test_nth();
        test_splice();
        test_sort_list();
        test_sprintf();
        test_replacement();                     // 23/04/10
        test_isa();
        test_basedir();
        test_env();
        test_register();
        test_buffer();
        test_key();
        test_debug();
        test_strtol();
        test_strtof();
        test_strtod();
        test_defaults();
        test_reference();
        test_pushpop();
        test_globals();
        test_search();
        test_called();
        test_macro();
        test_undef();
        test_history();
        test_scope();
        test_dict();
        test_leaks();
        regress2();
    }

    set_echo_format();
    pause_on_error(opause, FALSE);              /* turn off buffer change flag */
    set_buffer_flags(NULL, NULL, ~BF_CHANGED);

    message("Tests passed: %d, failed: %d .. ", num_passed, num_failed);
    if (0 == num_failed) {
        return;                                 /* success */
    }

    if (edit_file("Regression-Test") > 0) {     /* export results */
        int maj, min, edit;
        int h, m, s, dy, yr;
        string mn, infobuf;

        set_buffer_type(NULL, BFTYP_UTF8);
        date(yr, NULL, dy, mn);
        time(h, m, s);
        version(maj, min, edit);
        sprintf(infobuf, "\n%d %s %d %02d:%02d v%d.%d%c (%s)\n",
            dy, mn, yr, h, m, maj, min, edit, (grief_version(1) ? "release" : "debug"));

        end_of_buffer();
        insert(infobuf);
        for (i = 0; i < length_of_list(failed_list);) {
            insert(failed_list[i++]);
        }
        set_bottom_of_window();
    }
}


static void
passed(int num)
{
    extern list failed_list;
    extern int num_passed, x_lastnum;

    if (x_lastnum > 0 && (num - 1) != x_lastnum) {
        failed_list += "warning: test seq " + num + " from " + x_lastnum + "\n";
    }
    x_lastnum = num;
    ++num_passed;
}


static void
failed(int num, string statement)
{
    extern list failed_list;
    extern int num_failed, x_lastnum;

    if (x_lastnum > 0 && (num - 1) != x_lastnum) {
        failed_list += "warning: test seq" + num + "from " + x_lastnum + "\n";
    }
    failed_list += "Test #" + num + ": (" + statement + ") failed.\n";
    x_lastnum = num;
    ++num_failed;
}


static int
test_basic(void)
{
    list l1;
    string s1, s2, s3;
    declare d1;
    int i, j, k;

    i = j = k = 0;
    s1 = "String one";
    s2 = "String two";
    s3 = "String three";

    TEST(1, i == 0);

    s1 = s2;
    TEST(2, s1 == "String two");
    TEST(3, s1 == "String two" + "");

    s1 = s2 + s3;
    TEST(4, s1 == "String twoString three");

    s2 = "HELLO";
    s2 = s2;
    TEST(5, s2 == "HELLO");

    s2 = "S2";
    s1 = s2 + "-second-" + s2;
    TEST(6, s1 == "S2-second-S2");

    s1 = "variable";
    k = 99;
    TEST(7, test_1("literal-string", 23, s1, k));

    test_2(i, j, k, s1, s2, s3);
    TEST(8, i == 25);
    TEST(9, j == 26);
    TEST(10, k == 27);
    TEST(11, s1 == "literal");
    TEST(12, s2 == "variable");
    TEST(13, s3 == "5");

    k = 1 ? 2 : 3;
    TEST(14, k == 2);

    s1 = 1 ? "abc" : "def";
    TEST(15, s1 == "abc");

    s1 = 0 ? "abc" : "def";
    TEST(16, s1 == "def");

    s2 = "variable";
    k = 99;
    sprintf(s1, "%s,%d,%s,%d", "literal", 1, s2, k);
    TEST(17, s1 == "literal,1,variable,99");

    sprintf(s1, "--%s--", 1 ? "abc" : "def");
    TEST(18, s1 == "--abc--");

    TEST(19, test_4() == 0);

    d1 = test_7();
    TEST(20, is_integer(d1));
    TEST(21, d1 == 1);

    TEST(22, test_if(0) == FALSE);              // int
    TEST(23, test_if(1) == TRUE);
    TEST(24, test_if(0.0) == FALSE);            // float
    TEST(25, test_if(1.1) == TRUE);
    TEST(26, test_if("") == FALSE);             // string
    TEST(27, test_if("hello") == TRUE);
    TEST(28, test_if(l1) == FALSE);             // list
    l1 += "hello"; TEST(29, test_if(l1) == TRUE);
    TEST(30, test_if(NULL) == FALSE);           // NULL

    TEST(31, test_ifbug() == 1);
    TEST(32, test_elsif1() == 999);
    TEST(33, test_elsif2() == 999);

    TEST(34, test_8());
    TEST(35, test_9("str",1));
}


static int
test_1(~string, ~int, ~string, ~int)
{
    string s1, s2;
    int i1, i2;

    get_parm(0, s1);
    get_parm(1, i1);
    get_parm(2, s2);
    get_parm(3, i2);
    return s1 == "literal-string" && i1 == 23 && s2 == "variable" && i2 == 99;
}


static void
test_2(...)
{
    string s1 = "variable";

    put_parm(0, 25);
    put_parm(1, 26);
    put_parm(2, 27);
    put_parm(3, "literal");
    put_parm(4, s1);
    put_parm(5, "5");
    put_parm(10, 999);
}


static int
test_4(void)
{
    int dir, re;
    string prompt;

    dir = 0;
    re = 1;
    sprintf(prompt, "%c Pattern%s: ", dir ? 25 : 24, re ? "" : "(RE off)");
    return prompt != "\030 Pattern: ";
}


static int
test_6(void)
{
    int i;

    while (i < 10) {
        i = 20;
        switch(113) {
        case 1:
        case 2:
        case 113:
            return 99;
        default:
            i = 30;
        break;
        }
    }
    return i;
}


static int
test_7(void)
{
    float f1;
    int i;
    declare d1;

    d1 = execute_macro(" + 123 0.0");
    f1 = cvt_to_object("1.23");
    i  = f1;
    return i;
}


static int
test_8(~declare v1, ~declare v2, ~declare v3, ~declare v4,
    ~declare v5, ~declare v6, ~declare v7, ~declare v8, ~declare v9)
{
    /*
     *  By default polymorphic variables are given the type
     *  integer and the value of 0.
     */
    return is_integer(v1) && is_integer(v2) && is_integer(v3) &&
        is_integer(v4) && is_integer(v5) && is_integer(v6) && is_integer(v7) &&
        is_integer(v8) && is_integer(v9) &&
        v1 == 0 && v2 == 0 && v3 == 0 && v4 == 0 && v5 == 0 &&
        v6 == 0 && v7 == 0 && v8 == 0 && v9 == 0;
}


static int
test_9(~string, ~int, ~string, ~int)
{
    string s1, s2;
    int i1, i2;

    return (get_parm(0, s1) > 0 && get_parm(1, i1) > 0 &&
                get_parm(2, s2) <= 0 && get_parm(3, i2) <= 0);
}


static int
test_10(int value)
{
    return value;
}


/*
 *  test_returns ---
 *      returns() functionality.
 */
static void
test_returns(void)
{
    int returns_here = 0;

    TEST(36, returns_1() == 42);
    TEST(37, returns_2() == "returns_2");
    TEST(38, returns_3() == "returns_3");

    TEST(39, 99 == returns_4());
    TEST(40, 4 == returns_here);

    TEST(41, "hello world" == returns_5());
    TEST(42, 5 == returns_here);
}


static int
returns_1(void)
{
    returns(42);
}


static string
returns_2(void)
{
    returns("returns_2");
}


static string
returns_3(void)
{
    string s1 = "returns_3";
    returns(1 ? s1 : "def");
}


static int
returns_4(void)
{
    extern int returns_here;

    returns(99);
    returns_here = 4;
}


static string
returns_5(void)
{
    extern int returns_here;

    returns("hello world");
    returns_here = 5;
}


/*
 *  test_operators ---
 *      basic operators.
 */
static void
test_operators(void)
{
    /*
     *  basic
     */
    TEST(43, 100 > 1);
    TEST(44, (1 > 100) == 0);

    TEST(45, 1 < 100);
    TEST(46, (100 < 1) == 0);

    TEST(47, 1 <= 100);
    TEST(48, 100 <= 100);
    TEST(49, (100 <= 1) == 0);

    TEST(50, 100 >= 1);
    TEST(51, 100 >= 100);
    TEST(52, (1 >= 100) == 0);

    TEST(53, (1 && 1) == 1);
    TEST(54, (1 && 0) == 0);
    TEST(55, (0 && 1) == 0);

    TEST(56, (1 || 1) == 1);
    TEST(57, (1 || 0) == 1);
    TEST(58, (0 || 1) == 1);

    /*
     *  alternatives functors
     */
    TEST(59, above(1, 1) == 0);
    TEST(60, above(3, 2) == 1);

    TEST(61, above_eq(9,   10) == 0);
    TEST(62, above_eq(100, 100) == 1);
    TEST(63, above_eq(1001, 1000) == 1);

    TEST(64, below(1, 1) == 0);
    TEST(65, below(2, 2) == 0);

    TEST(66, below_eq(10, 9) == 0);
    TEST(67, below_eq(100, 100) == 1);
    TEST(68, below_eq(1000, 1001) == 1);

    TEST(69, compare("a", "a") == 0);
    TEST(70, compare(1, 1) == 0);
}


/*
 *  test_arith ---
 *      basic arthmetric.
 */
static void
test_arith(void)
{
    float f;
    int i;

    /*
     *  operators
     *      contant expressions
     */
    TEST(71, 1 + 2      == 3);                  // ADD
    TEST(72, 1.5 + 2.5  == 4);

    TEST(73, 2 - 1      == 1);                  // MIN
    TEST(74, 2.5 - 1.0  == 1.5);

    TEST(75, 8 / 2      == 4);                  // DIV
    TEST(76, 10.0 / 4.0 == 2.5);

    TEST(77, 5 * 5      == 25);                 // MUL
    TEST(78, 5.5 * 1.6  == 8.8);

    TEST(79, 12 % 10    == 2);                  // MOD

    TEST(80, 1 << 10 == 1024);                  // LSH
    TEST(81, 8 >> 3  == 1);                     // RSH
    TEST(82, (10 ^ 2) == 8);                    // XOR

    TEST(83, (0xff54 & 0xff) == 0x54);          // AND
    TEST(84, (0xff54 | 0xff) == 0xffff);        // OR

    TEST(85, (~0 & 0xffff) == 0xffff);          // COM
    TEST(86, !0 == 1);                          // NOT

    TEST(87, -(+1) == -1);                      // NEG
    TEST(88, +(-1) == -1);                      // PLUS
    TEST(89, ((-(-2)) == +2));
    TEST(90, (3 - (-2) == 5));


    /*
     *  operators
     *      non-contant expressions
     */
    i = 1;      TEST(91, i + 2      == 3);
    f = 1.5;    TEST(92, f + 2      == 3.5);

    i = 2;      TEST(93, i - 1      == 1);
    f = 2.5;    TEST(94, f - 1      == 1.5);

    i = 8;      TEST(95, i / 2      == 4);
    f = 9;      TEST(96, f / 2      == 4.5);

    i = 5;      TEST(97, i * 5      == 25);
    f = 5;      TEST(98, f * 5.1    == 25.5);

    i = 12;     TEST(99, i % 10     == 2);
    i = 0;      TEST(100, (~i & 0xffff) == 0xffff);
    i = 0;      TEST(101, (! i)      == 1);

    i = 1;      TEST(102, i << 10    == 1024);
    i = 8;      TEST(103, i >> 3     == 1);
    i = 10;     TEST(104, (i ^ 2)    == 8);
    i = 0xff54; TEST(105, (i & 0xff) == 0x54);
    i = 0xff54; TEST(106, (i | 0xff) == 0xffff);

    /*
     *  operators
     *      non-contant assignment expressions
     */
    i = 1;      TEST(107, i + 2      == 3);
    i = 2;      TEST(108, i - 1      == 1);
    i = 8;      TEST(109, i / 2      == 4);
    i = 5;      TEST(110, i * 5      == 25);
    i = 12;     TEST(111, i % 10     == 2);

    i = 1;      TEST(112, i << 10    == 1024);
    i = 8;      TEST(113, i >> 3     == 1);
    i = 10;     TEST(114, (i ^ 2)    == 8);
    i = 0xff54; TEST(115, (i & 0xff) == 0x54);
    i = 0xff54; TEST(116, (i | 0xff) == 0xffff);

    i = 69;     TEST(117, (i += 10)  == 79   && i == 79);
    i = 42;     TEST(118, (i -= 40)  == 2    && i == 2);
    i = 99;     TEST(119, (i /= 3)   == 33   && i == 33);
    i = 41;     TEST(120, (i *= 3)   == 123  && i == 123);
    i = 102;    TEST(121, (i %= 10)  == 2    && i == 2);

    i = 42;     TEST(122, (i <<= 3)  == 336  && i == 336);
    i = 8;      TEST(123, (i >>= 3)  == 1    && i == 1);
    i = 10;     TEST(124, (i ^= 2)   == 8    && i == 8);
    i = 0xf7;   TEST(125, (i &= 0xf) == 0x7  && i == 0x7);
    i = 0xf7;   TEST(126, (i |= 0xf) == 0xff && i == 0xff);

    /*
     *  type conversion
     */
    f = 2.34;
    i = f;
    TEST(127, i == 2);
    TEST(128, is_integer(i));
}


/*
 *  test_typeof ---
 *      Tyep functionality
 */
static void
test_typeof(void)
{
    declare d1;

    d1 = "hello";
    TEST(129, typeof(d1) == "string");

    d1 = 1;
    TEST(130, typeof(d1) == "integer");

    d1 = 1.2334;
    TEST(131, typeof(d1) == "float");

    d1 = NULL;
    TEST(132, typeof(d1) == "NULL");

    d1 = quote_list("one", "two");
    TEST(133, typeof(d1) == "list");

    /*
     *  FIXME/XXX - unsure. review bug or feature
     *      assigning NULL to list zeros the list yet the type is still "list".
     */
    d1 = NULL;
    TEST(134, typeof(d1) == "list");
    TEST(135, is_null(d1));
}


/*
 *  test_for ---
 *     for primitives
 */
static void
test_for(void)
{
    int x, y;

    for (x = 0; x < 2; ++x) { }
    TEST(136, 2 == x);

    for (x = 0, y = 1234; x < 2; ++x) {
        break; y = 0;
    }
    TEST(137, 1234 == y);
    TEST(138, 0 == x);

    for (x = 0; x < 2; ++x) {
        if (0 == x) continue;
        TEST(139, 1 == x);
    }
    TEST(140, 2 == x);
}


/*
 *  test_while ---
 *     while primitives
 */
static void
test_while(void)
{
    int x, y;

    x = 0;
    while (x < 2) { ++x; }
    TEST(141, 2 == x);

    x = 0; y = 1234;
    while (x < 2) {
        break; y = 0; ++x;
    }
    TEST(142, 1234 == y);
    TEST(143, 0 == x);

    x = 0;
    while (x < 2) {
        if (0 == x) {
            x++;
            continue;
        }
        TEST(144, 1 == x);
        ++x;
    }
    TEST(145, 2 == x);
}


/*
 *  test_do ---
 *      do() primitives
 */
static void
test_do(void)
{
    int x, y;

    x = 0;
    do { ++x; } while (x < 2);
    TEST(146, 2 == x);

    x = 0; y = 1234;
    do {
        break; y = 0; ++x;
    } while (x < 2);
    TEST(147, 1234 == y);
    TEST(148, 0 == x);

    x = 0;
    do {
        if (0 == x) {
            x++;
            continue;
        }
        TEST(149, 1 == x);
        ++x;
    } while (x < 2);
    TEST(150, 2 == x);
}


/*
 *  test_switch ---
 *     switch/case primitives
 */
static void
test_switch(void)
{
    string s1, s2;
    int i;

#pragma warning off
    for (i = 1; i <= 8; i *= 2) {
        switch (i) {
        case 1:
            TEST(151, 1 == i);
            break;
        case 2:
        case 3:
            TEST(152, 2 == i);
        case 5:
        case 4:
            TEST(153, 4 == i);
        default:
            TEST(154, 8 == i);
            break;
        }
    }
#pragma warning on

    switch(3) {
    case 1: i = 101; break;
    case 2: i = 102; break;
    case 3: i = 103; break;
    }
    TEST(155, i == 103);

    switch("hello") {
    case "hello, everybod": s1 = "first"; break;
    case "hello": s1 = "second";  break;
    default: s1 = "default";
    }
    TEST(156, s1 == "second");

    s1 = "hello, everybod";
    s2 = "hello";
#pragma warning off
    switch("hello") {
    case s1: s1 = "first"; break;
    case s2: s1 = "second"; break;
    default: s1 = "default"; break;
    }
#pragma warning on
    TEST(157, s1 == "second");

    TEST(158, "none"  == switchi(0));
    TEST(159, "one"   == switchi(1));
    TEST(160, "two"   == switchi(2));
    TEST(161, "three" == switchi(3));

    TEST(162, 0 == switchs(""));
    TEST(163, 1 == switchs("1"));
    TEST(164, 2 == switchs("22"));
    TEST(165, 3 == switchs("333"));
}


static string
switchi(int ival)
{
#pragma warning off
    switch (ival) {         // mixed usage
    case "":    return "none";
    case "1":   return "one";
    case "12":  return "two";
    default:
        return "three";
    }
#pragma warning on
}


static int
switchs(string sval)
{
#pragma warning off
    switch (sval) {         // mixed usage
    case 0:     return 0;
    case 1:     return 1;
    case 2:     return 2;
    default:
        return 3;
    }
#pragma warning on
}


/*
 *  test_cexpr ---
 *      const expressions
 */
static void
test_cexpr(void)
{
    int i = -1;

    while (++i <= 3)
        if (0 == i)
            1;
        else if (1 == i)
            1.1;
        else if (2 == i)
            "";

    do 1;   while (++i <= 4);

    do 1.1; while (++i <= 5);

    do "";  while (++i <= 6);

    TEST(166, 7 == i);
}


/*
 *  test_math ---
 *      Maths functionality
 */
static void
test_math(void)
{
#define PI (3.141592653589793)

    float expmantissa;
    int expret;

    float modintegral, modfractional;

    TEST(167, abs(1)        == 1);
    TEST(168, abs(-2)       == 2);

    TEST(169, 0 == isclose(1.233, 1.4566));
    TEST(170, 1 == isclose(1.233, 1.233));
    TEST(171, 1 == isclose(1.233, 1.233000001));

    TEST(172, acos(0.55)    == 0.9884320889261531);
    TEST(173, acos(-0.55)   == 2.15316056466364);

    TEST(174, asin(0.55)    == 0.5823642378687435);
    TEST(175, asin(-0.55)   == -0.5823642378687435);

    TEST(176, atan(67.0)    == 1.5558720618048116);
    TEST(177, atan(-21.0)   == -1.5232132235179132);

    TEST(178, atan2(8.0, 5.0) == 1.0121970114513341);
    TEST(179, atan2(20.0, 10.0) == 1.1071487177940904);

    TEST(180, ceil(9.2)     == 10.0);
    TEST(181, ceil(-9.2)    == -9.0);

    // TODO: comb() [ Python ]
    // TODO: copysign() [ Python ]

    TEST(182, cos(0.0)      == 1.0);
    TEST(183, cos(3.14159265359) == -1.0);

    TEST(184, cosh(1.0)     == 1.5430806348152437);
    TEST(185, cosh(0.0)     == 1.0);

    // TODO: degrees() [ Python ]
    // TODO: dist() [ Python ]
    // TODO: erf() [ Python ]
    // TODO: erfc() [ Python ]
    // TODO: div()

    TEST(186, isclose(exp(23.0), 9744803446.248903));
    TEST(187, exp(-1.234)   == 0.2911257425960852);

    TEST(188, fabs(2.0)     == 2.0);
    TEST(189, fabs(-4.0)    == 4.0);

    // TODO: fdiv()
    // TODO: factorial() [ Python ]

    TEST(190, floor(9.2)    == 9.0);
    TEST(191, floor(-9.2)   == -10.0);

    TEST(192, fmod(67.0, 7.0) == 4.0);
    TEST(193, fmod(17.0, 4.0) == 1.0);

    expmantissa = frexp(4.0, expret);
    TEST(194, expmantissa == 0.5 && expret == 3);
    expmantissa = frexp(7.9, expret);
    TEST(195, expmantissa == 0.9875 && expret == 3);

    // TODO: fsum [ Python ]
    // TODO: gamma [ Python ]

    TEST(196, isfinite(-45.34));
    TEST(197, isfinite(+45.34));
        // FIXME: +45.34 grunch should handle leading '+'
    TEST(198, ! isfinite(NAN));
    TEST(199, ! isfinite(INFINITY));
    TEST(200, ! isfinite(-INFINITY));

    TEST(201, isinf(INFINITY));
    TEST(202, isinf(-INFINITY));
    TEST(203, ! isinf(56.00));
    TEST(204, ! isinf(NAN));

    TEST(205, isnan(NAN));
    TEST(206, ! isnan(56.00));
    TEST(207, ! isnan(INFINITY));
    TEST(208, ! isnan(-INFINITY));

    // TODO: isqrt [ Python ]
    // TODO: ldexp
    // TODO: lgamma [ Python ]

    TEST(209, fabs(log(2.0) - 0.693147) < 0.000001);
    TEST(210, isclose(log(2.0), 0.693147, 1e-5));
    TEST(211, fabs(log(4.0) - 1.386294) < 0.000001);
    TEST(212, isclose(log(4.0), 1.386294, 1e-5));

    TEST(213, log10(10.0)   == 1.0);
    TEST(214, log10(100.0)  == 2.0);

    modfractional = modf(52.42, modintegral);
    TEST(215, modintegral == 52 && isclose(modfractional, 0.42));
    modfractional = modf(NAN, modintegral);
    TEST(216, isnan(modfractional));

    TEST(217, pow(9.0, 3.0) == 729.0);

    TEST(218, sin(0.0)      == 0.0);
    TEST(219, sin(10.0)     == -0.5440211108893698);
    TEST(220, sin(PI / 2)   == 1.0);

    TEST(221, sinh(0.0)     == 0.0);
    TEST(222, isclose(sinh(-23.45), -7641446994.979367));
    TEST(223, isclose(sinh(23.0), 4872401723.124452));
    TEST(224, isclose(sinh(PI), 11.548739357257748));

    TEST(225, sqrt(4.0)     == 2.0);
    TEST(226, isclose(sqrt(10.0), 3.162278, 0.000001));

    TEST(227, tan(90.0)     == -1.995200412208242);
    TEST(228, tan(-90.0)    == 1.995200412208242);
    TEST(229, tan(45.0)     == 1.6197751905438615);
    TEST(230, tan(60.0)     == 0.320040389379563);

    TEST(231, isclose(tanh(8.0), 0.9999997749296758, 1e14));
    TEST(232, isclose(tanh(1.0), 0.7615941559557649, 1e14));
    TEST(233, tanh(-6.2)    == -0.9999917628565104);

    // TODO: trunc
}


/*
 *  test_cast ---
 *      integer and float cast.
 */
static void
test_cast(void)
{
    declare x;

    TEST(234, (int)1.1 == 1);
    x = ((int)1.1); TEST(235, is_integer(x));

    TEST(236, (float)3 == 3.0);
    x = ((float)1); TEST(237, is_float(x));
}


/*
 *  test_escapes ---
 *      escape sequences.
 */
static void
test_escapes(void)
{
    TEST(238, '\cA' == 0x00 && '\0' == 0x00 && '\000' == 0x00);
                                                // NUL
    TEST(239, '\c[' == 0x1b && '\e' == 0x1b);   // ESC
#pragma warning(push, off)
    TEST(240, L'\u2b3c'     == 0x2b3c);         // Unicode
    TEST(241, L'\U00012b3c' == 0x12b3c);        // Unicode (76604 dec)
    TEST(242, L'\x{1a2b}'   == 0x1a2b);         // Extended hexidecimal contants.
    TEST(243, L'\o{1234}'   == 01234);          // Extended octal contants.
#pragma warning(pop)
}


/*
 *  test_string ---
 *      string primitives.
 */
static void
test_string(void)
{
    string s1, s2, empty, nonempty = "x";
    float f;
    int i;

    // operations on string scalars
    TEST(244, !empty);                          // undocumented feature, scalar reference.
    TEST(245, nonempty);                        //  only function as 'single' expression NOT within compound

    // string addition
    s1 = "xyz"; s1 += "abc";
    TEST(246, s1 == "xyzabc");

    s1 = "xyz"; s2 = "abc"; s1 += s2;
    TEST(247, s1 == "xyzabc");

    s1 = "xyz"; s2 = s1; s1 += s2;
    TEST(248, s1 == "xyzxyz");

    s1 = "xyz";
    TEST(249, (s1 += "abc") == "xyzabc");

    s1 = "xyz";
    TEST(250, (s1 += s1) == "xyzxyz");

    s1 = "xyz";
    TEST(251, (s1 = s1) == "xyz");

    // numeric conversion
    i = 99;
    s1 = "abc";
    s1 = s1 + i;
    TEST(252, s1 == "abc99");

    i = 99;
    s1 = "abc";
    s1 = i + s1;
    TEST(253, s1 == "99abc");

    s1 = "abc";
    s1 += 0;
    TEST(254, s1 == "abc0");

    f = 1.234;
    s1 = "abc";
    s1 = f + s1;
    TEST(255, s1 == "1.234abc");

    // string multipler
    //  trailing or leading
    //  integer or float multipler
    //
    i = 0;
    s1 = "xyz" * i;
    TEST(256, s1 == "");
    s1 = i * "xyz";
    TEST(257, s1 == "");

    i = 1;
    s1 = "xyz" * i;
    TEST(258, s1 == "xyz");
    s1 = i * "xyz";
    TEST(259, s1 == "xyz");

    i = 2;
    s1 = "xyz" * i;
    TEST(260, s1 == "xyzxyz");
    s1 = i * "xyz";
    TEST(261, s1 == "xyzxyz");

    f = 1.1;
    s1 = "xyz" * f;
    f = 2.2;
    s1 = "xyz" * f;
    TEST(262, s1 == "xyzxyz");

    // string accumulator tests
    TEST(263, "abc" + "def" == "abcdef");
    TEST(264, 1 + "def" == "1def");
    TEST(265, "abc" + 1 == "abc1");
    TEST(266, 1.2 + "def" == "1.2def");
    TEST(267, "abc" + 1.2 == "abc1.2");

    // string primitives
    TEST(268, trim("  harry  ") == "harry");
    TEST(269, rtrim("harry  ") == "harry");
    TEST(270, ltrim("  harry") == "harry");

    s1 = "  harry  ";
    TEST(271, rtrim(s1) == "  harry");
    TEST(272, ltrim(s1) == "harry  ");

    TEST(273, atoi("98") == 98);
    TEST(274, strlen("abcd") == 4);
    s1 = "abcd" + " ";
    TEST(275, strlen(s1) == 5);

    TEST(276, strlen("nothing") == 7);

    s1 = "1234554321";
    TEST(277, index(s1, "4") == 4);
    TEST(278, index(s1, "") == 11);
    TEST(279, rindex(s1, "5") == 6);

    s1 = "";
    s1 = substr(s1, index(s1, ";") + 1);
    TEST(280, s1 == "");

    gs1 = "";
    get_parm(2, gs1);
    gs1 = substr(gs1, index(gs1, ";") + 1);
    TEST(281, gs1 == "");

    TEST(282, upper("aBc") == "ABC");
    TEST(283, lower("AbC") == "abc");
    TEST(284, 5 == string_count("axba bax", "abz"));

    TEST(285, 0 == strcasecmp("AaA", "aAa"));
    TEST(286, strerror(0) == "Success");

    s1 = "abAB";
    TEST(287, "a"  == strpop(s1));
    TEST(288, "b"  == strpop(s1));
    TEST(289, "AB" == strpop(s1, 2));
    TEST(290, ""   == strpop(s1));

    s1 = "abcdefg";
    TEST(291, 3 == strpbrk(s1, "dc"));

    // strstr, strrstr and strcasestr
    s1 = "abcmandefg";
    TEST(292, 4 == strstr(s1, "man"));
    TEST(293, 4 == strrstr(s1, "man"));
    TEST(294, 4 == strcasestr(s1, "MAN"));

    s1 = "abcmanmandefg";
    TEST(295, 4 == strstr(s1, "man"));
    TEST(296, 7 == strrstr(s1, "man"));

    TEST(297, 0 == strstr(s1, "ban"));
    TEST(298, 0 == strrstr(s1, "ban"));

    TEST(299, 1 == strstr(s1, ""));
    TEST(300, 0 == strrstr(s1, ""));
}


/*
 *  test_regexp ---
 *      re_search and re_translate
 */
static void
test_regexp(void)
{
    string s1, s2;
    int i;

    s1 = "Hello World";

    //  search primitives,
    //      test offset and length returns.
    //
    i = -1;
    TEST(301, 6 == search_string(" [worl]+", s1, i, -2 /*MAXIMUM*/, FALSE));
    TEST(302, 5 == i);

    i = -1;                                     /* FIXME/XXX - Brief minimal logic needs reviewing */
    TEST(303, 6 == re_search(SF_BRIEF|SF_IGNORE_CASE, " [worl]+", s1, NULL, i));
    TEST(304, 2 == i);

    i = -1;
    TEST(305, 6 == re_search(SF_BRIEF|SF_MAXIMAL|SF_IGNORE_CASE, " [worl]+", s1, NULL, i));
    TEST(306, 5 == i);

    i = -1;
    TEST(307, 6 == re_search(SF_UNIX|SF_IGNORE_CASE, " [worl]+", s1, NULL, i));
    TEST(308, 5 == i);

    i = -1;
    TEST(309, 6 == re_search(SF_EXTENDED|SF_IGNORE_CASE, " [worl]+", s1, NULL, i));
    TEST(310, 5 == i);

    i = -1;
    TEST(311, 6 == re_search(SF_PERL|SF_IGNORE_CASE, " [worl]+", s1, NULL, i));
    TEST(312, 5 == i);

    i = -1;
    TEST(313, 6 == re_search(SF_TRE|SF_IGNORE_CASE, " [worl]+", s1, NULL, i));
    TEST(314, 5 == i);

    TEST(315, 6 == re_search(SF_PERL|SF_IGNORE_CASE, " [worl]+", s1, NULL, i));
    TEST(316, 5 == i);

    s1 = "Hello World";
    TEST(317, 6 == re_search(SF_PERL|SF_IGNORE_CASE, " ([worl]+)", s1, NULL, i));
    TEST(318, 5 == i);

    //  captures
    //
    s1 = "y aabbccddee z";
    s2 = "y XaabbccddeeX z";

    TEST(319, re_search(SF_BRIEF,       " {[abcde]+} ", s1, NULL, i) == 2);

    TEST(320, re_search(SF_UNIX,        " \\([abcde]+\\) ", s1, NULL, i) == 2);

    TEST(321, re_search(SF_EXTENDED,    " ([abcde]+) ", s1, NULL, i) == 2);

    TEST(322, re_search(SF_PERL,        " ([abcde]+) ", s1, NULL, i) == 2);

    TEST(323, re_translate(SF_BRIEF|SF_MAXIMAL, "{[abcde]+}", "X\\0X", s1) == s2);

    TEST(324, re_translate(SF_UNIX,     "\\([abcde]+\\)", "X\\0X", s1) == s2);

    TEST(325, re_translate(SF_EXTENDED, "([abcde]+)", "X\\0X", s1) == s2);

    TEST(326, re_translate(SF_PERL,     "([abcde]+)", "X$1X", s1) == s2);

    TEST(327, re_translate(SF_PERL,     "([abcde]+)", "$`", s1) == "y y  z");

    TEST(328, re_translate(SF_PERL,     "([abcde]+)", "$'", s1) == "y  z z");

    TEST(329, re_translate(SF_TRE,      "([abcde]+)", "X$1X", s1) == s2);
}


/*
 *  test_sub ---
 *      sub and gsub macros
 */
static void
test_sub(void)
{
    TEST(330, gsub("ana", "anda", "banana") == "bandana");
    TEST(331, gsub("a",   "-&-",  "banana") == "b-a-n-a-n-a-");
    TEST(332, gsub("a+",  "-&-",  "banana") == "b-a-n-a-n-a-");
    TEST(333, gsub("a+",  "-a-",  "baaaanaaaaanaaaa") == "b-a-n-a-n-a-");
    TEST(334, sub("ana",  "anda", "banana") == "bandana");
    TEST(335, sub("a",    "-&-",  "banana") == "b-a-nana");
    TEST(336, sub("a+",   "-&-",  "banana") == "b-a-nana");
    TEST(337, sub("a+",   "-a-",  "baaaanaaaaanaaaa") == "b-a-naaaaanaaaa");
    TEST(338, sub("na$",  "na.",  "banana") == "banana.");
    TEST(339, gsub("^a",  "A",    "anana")  == "Anana");
    TEST(340, gsub("n.n", "[&]",  "banana") == "ba[nan]a");
    TEST(341, gsub("a|n", "z",    "anna")   == "zzzz");

    TEST(342, sub("f\\(.*\\)t",   "F\\1T", "first:second") == "FirsT:second");
}



/*
 *  test_substr ---
 *      sub-string
 */
static void
test_substr(void)
{
    string s1;

    s1 = substr("ABC", 0, 3);
    TEST(343, s1 == "ABC");

    s1 = substr("ABC", -1000, 1000);
    TEST(344, s1 == "ABC");

    s1 = substr("ABC", 1000, 1000);
    TEST(345, s1 == "");

    s1 = substr("ABC", 1, 0);
    TEST(346, s1 == "");

    s1 = substr("ABC", 1, 1);
    TEST(347, s1 == "A");

    s1 = substr("ABC", 1, 2);
    TEST(348, s1 == "AB");

    s1 = substr("ABC", 1, 3);
    TEST(349, s1 == "ABC");

    s1 = substr("ABC", 1, 100);
    TEST(350, s1 == "ABC");

    s1 = substr("ABC", 3, 0);
    TEST(351, s1 == "");

    s1 = substr("ABC", 3, 1);
    TEST(352, s1 == "C");

    s1 = substr("ABC", 3, 100);
    TEST(353, s1 == "C");
}


/*
 *  test_compress ---
 *      compress
 */
static void
test_compress(void)
{
    // basic
    TEST(354, compress("  harry  ", 0) == " harry ");
    TEST(355, compress("  harry  ", 1) == "harry");
    TEST(356, compress("  h  a  r   r  y  ", 0) == " h a r r y ");
    TEST(357, compress(" h  a  r   r  y ", 0) == " h a r r y ");
    TEST(358, compress("  h  a  r   r  y  ", 1) == "h a r r y");

    // extended features
    TEST(359, compress("  harry  ", 0, " r") == " ha y ");
    TEST(360, compress("  harry  ", 0, " r", 'x') == "xhaxyx");
}


/*
 *  test_list ---
 *      basic list operations and primitives
 */
static void
test_list(void)
{
    list    l1, l2, l3;
    string  s1;
    declare a57, b57;
    declare d1;

    // 1.
    l1 = quote_list(123, 1.23, "xyz", hello()); /* int, float, string and symbol */
    TEST(361, length_of_list(l1) == 4);

    l2 = l1;
    TEST(362, length_of_list(l2) == 4);
    TEST(363, l1[0] == l2[0]);
    TEST(364, l1[1] == l2[1]);
    TEST(365, l1[2] == l2[2]);

    d1 = l1[0];
    TEST(366, is_integer(d1));                  /* 123 */
    TEST(367, is_type(d1, "integer"));

    d1 = l1[1];
    TEST(368, is_float(d1));                    /* 1.23 */
    TEST(369, is_type(d1, "float"));

    d1 = l1[2];
    TEST(370, is_string(d1));                   /* "xyz" */
    TEST(371, is_type(d1, "string"));

    d1 = l1[3];
    TEST(372, is_list(d1));                     /* hello() */
    TEST(373, is_type(d1, "list"));

    pause_on_error(0, FALSE);
    d1 = l1[4];                                 /* range error */
    pause_on_error(1, FALSE);
    TEST(374, is_null(d1));
    TEST(375, is_type(d1, "null"));

    // 2.
    l1 = quote_list(1);
    l1[0] = 2;
    TEST(376, l1[0] == 2);

    l1 = quote_list(1, "abc");
    l1[0] = 2;
    TEST(377, l1[0] == 2);

    l1 = quote_list("abc");
    l1[0] = 2;
    TEST(378, l1[0] == 2);

    l1 = quote_list("abc", 1);
    l1[1] = 2;
    TEST(379, l1[1] == 2);

    l1 = quote_list(1, "abc", 3);
    l1[1] = 2;
    TEST(380, l1[1] == 2);

    l1 = quote_list(1, 2, 3);
    l1[1] = "abc";
    TEST(381, l1[1] == "abc");

    l1 = quote_list(1, 2, 3);
    l2 = l1;
    l1[1] = l2;
    TEST(382, length_of_list(l1) == 5);

    l1 = quote_list(1, 2, 3);
    l1[1] = quote_list(1, 2, 3);
    TEST(383, length_of_list(l1) == 5);

    TEST(384, 1. == 1);

    l1 = quote_list(1, 2, 3);
    l1[1] = make_list(quote_list(1, 2, 3));
    l1[3] = "end";
    TEST(385, l1[3] == "end");
    TEST(386, length_of_list(l1) == 4);

    l3[0] = 0;
    l3[1] = 1;
    l3[2] = 2;
    TEST(387, l3[0] == 0);
    TEST(388, l3[1] == 1);
    TEST(389, l3[2] == 2);

    l1 = NULL;
    TEST(390, length_of_list(l1) == 0);

    l1[0] = "hello";
    TEST(391, l1[0] == "hello");

    s1 = "abc";
    l1[0] = s1;
    TEST(392, l1[0] == "abc");

    s1 = "abc";
    l1[0] = s1;
    s1 = "123456789";
    TEST(393, l1[0] == "abc");

    b57 = "hello";
    a57 = b57;
    TEST(394, a57 == "hello");

    TEST(395, test_6() == 99);

    l1 = quote_list("one", "", "three");
    s1 = "TWO";
    l1[1] = s1;
    l1[1] = s1;
    TEST(396, l1[1] == "TWO");

    l1 = quote_list(1, 2, 3);
    l2 = make_list(l1);
    l1[1] = l2;
    TEST(397, length_of_list(l1) == 3);

    l1 = quote_list("hello", "list", 1, NULL, 2.3);
    l2 = make_list(l1);
    l1[1] = l2;
    TEST(398, length_of_list(l1) == 5);

    l1 = quote_list(1, 2, 3);
    l1[1] = make_list(quote_list(1, 2, 3));
    TEST(399, length_of_list(l1) == 3);

    l1 = quote_list(1, 2, 3);
    l1[2] = make_list(quote_list(1, 2, 3));
    TEST(400, length_of_list(l1) == 3);

    l1 = quote_list(1, 2, 3);
    l1[0] = make_list(quote_list(1, 2, 3));
    TEST(401, length_of_list(l1) == 3);

    l1 = make_list(quote_list(1, 2, 3));
    l1 += "abc";
    l1 += "def";
    l1[1] = 1;
    TEST(402, length_of_list(l1) == 3);

    l1 = make_list(quote_list(1, 2, 3), quote_list(1, 2, 3));
    TEST(403, car(car(l1)) == 1);
    TEST(404, length_of_list(l1) == 2);

    l1 = NULL;
    TEST(405, length_of_list(l1) == 0);
    pause_on_error(0, FALSE);
    nth(l1, 99);                                /* subscript out of range */
    pause_on_error(1, FALSE);

    // list functions
    l1 = command_list();
    TEST(406, is_list(l1));

    l1 = macro_list();
    TEST(407, is_list(l1));

    l1 = get_term_keyboard();
    TEST(408, is_list(l1));

    l1 = key_list();
    TEST(409, is_list(l1));

    l1 = bookmark_list();
    TEST(410, is_list(l1));

    l1 = macro_list();
    TEST(411, strlen(l1) != 0);
    TEST(412, length_of_list(cdr(macro_list())) > 0);
    TEST(413, search_list(NULL, "def", quote_list("abc", "def", "ghi")) == 1);
    l1 = quote_list("abc");
    l1 += "def";
    l1 += "ghi";
    TEST(414, search_list(NULL, "def", l1) == 1);

    // features
    l1 = inq_feature();
    TEST(415, is_list(l1));

    // list concat
    l1 = quote_list(1, "2", 3.0);
    l1 = l1 + l1;
    TEST(416, length_of_list(l1) == 6);
    l2 = l1;

    // element insert
    l1[3] = quote_list("a", "b", "c");
    TEST(417, length_of_list(l1) == 8);

    // element replace
    l1[0] = 99;
    TEST(418, l1[0] != l2[0]);

    // re_search
    l1 = quote_list("abc");
    l1 += "def";
    l1 += "ghi";
    TEST(419, re_search(NULL, "def", l1) == 1);
}


/*
 *  test_list2 ---
 *      extended list primitives (basic splice interface)
 */
static void
test_list2(void)
{
    list l1 = {"one", "two", "three", "four"};

    TEST(420, shift(l1) == "one");
    TEST(421, 3 == length_of_list(l1));         // two, three, four

    TEST(422, 5 == unshift(l1, "first", "second"));
    TEST(423, 5 == length_of_list(l1));         // first, second, two, three, four

    TEST(424, pop(l1) == "four");               // first, second, two, three
    TEST(425, 4 == length_of_list(l1));

    TEST(426, shift(l1) == "first" && shift(l1) == "second");
    TEST(427, 2 == length_of_list(l1));         // two, three

    TEST(428, pop(l1) == "three");              // two
    TEST(429, 1 == length_of_list(l1));

    TEST(430, shift(l1) == "two");              // null
    TEST(431, 0 ==  length_of_list(l1));
    TEST(432, is_null(l1));

    push(l1, "five", "six");
    TEST(433, 2 ==  length_of_list(l1));
}


/*
 *  test_list3 ---
 *      list foreach() primitive.
 */
static void
test_list3(void)
{
    list l1 = {1, "2", 3.3, 4, "5", 6.6};
    declare value;
    int idx, count;

    while ((idx = list_each(l1, value)) >= 0) {
        switch(idx) {
        case 0:
            TEST(434, 1 == value);
            break;
        case 1:
            TEST(435, "2" == value);
            break;
        case 2:
            TEST(436, 3.3 == value);
            break;
        case 3:
            TEST(437, 4 == value);
            break;
        case 4:
            TEST(438, "5" == value);
            break;
        case 5:
            TEST(439, 6.6 == value);
            break;
        }
        ++count;
    }
    TEST(440, length_of_list(l1) == count);
}


/*
 *  test_nth ---
 *      nth (multiple dim support)
 */
static void
test_nth(void)
{
    declare d1;

    gl1[1] = "TWO";
    TEST(441, gl1[1] == "TWO");
    TEST(442, gl1[3][0] == "x");
    TEST(443, gl1[3][1] == "y");
    TEST(444, gl1[2][1][1] == "2.1.1");
    TEST(445, gl1[gl2[0]][gl2[1]][gl2[2]] == "2.1.1");
    TEST(446, gl1[test_10(2)][test_10(1)][test_10(1)] == "2.1.1");
    pause_on_error(0, FALSE);
    d1 = gl1[2][1][4];                          /* range error */
    pause_on_error(1, FALSE);
    TEST(447, is_null(d1));
}


/*
 *  test_splice ---
 *      list splice functionaliy
 */
static void
test_splice(void)
{
    splice_1();
    splice_2();
    splice_3();
    splice_4();
}


static void
splice_1(void)
{
    list l = { "a", "b" };
    list x = quote_list("x", "x");

    /* [a, b, a, b] */
    l += l;
    TEST(448, length_of_list(l) == 4 && l[0] == "a" && l[1] == "b" && l[2] == "a" && l[3] == "b");

    /* [x, x, b, a, b] */
    l[0] = x;
    TEST(449, length_of_list(l) == 5 && l[0] == "x" && l[1] == "x");

    /* [x, x, b, a, b, Nul, Nul, Nul, Nul, x, x] */
    l[9] = x;
    TEST(450, length_of_list(l) == 11 && l[9] == "x");

    /* Join */
    l = splice_l1 + splice_l1;
    TEST(451, length_of_list(l) == 6);

    /* Append */
    l += splice_l1;
    TEST(452, length_of_list(l) == 9);
}


/*
 *  splice_2 ---
 *      slice tests
 *
 *      Delete tests:
 *          0.  EOL, shouldn't delete anything
 *          1.  Past EOL, shouldn't delete anything
 *          2.  length+1 EOL, shouldn't delete anything
 *          3.  Last
 *          4.  All
 *          5.  Empty
 */
static void
splice_2(void)
{
    list l = quote_list("red", "green", "blue");

    splice(l, -1);
    TEST(453, length_of_list(l) == 3 && l[2] == "blue");

    splice(l, 999);
    TEST(454, length_of_list(l) == 3 && l[2] == "blue");

    splice(l, 3);
    TEST(455, length_of_list(l) == 3 && l[2] == "blue");

    splice(l, 2);
    TEST(456, length_of_list(l) == 2 && l[1] == "green");

    splice(l, 0);
    TEST(457, length_of_list(l) == 0);

    splice(l, 0);
    TEST(458, length_of_list(l) == 0);
}


static void
splice_3(void)
{
    list l1 = quote_list("red", "green", "blue");
    list l2 = quote_list("red", "green", "blue");

    splice(l1, 1, 2, "yellow", "orange", "pink");
    TEST(459, length_of_list(l1) == 4 && l1[0] == "red" && l1[1] == "yellow" && l1[2] == "orange" && l1[3] == "pink");

    splice(l2, 1, 2, "yellow");
    TEST(460, length_of_list(l2) == 2 && l2[0] == "red" && l2[1] == "yellow");

    splice(l2, -1, 0, "orange");
    TEST(461, length_of_list(l2) == 3 && l2[0] == "red" && l2[1] == "yellow" && l2[2] == "orange");
}


static void
splice_4(void)
{
    list l1 = quote_list("1a", "1b", "1c");
    list l2 = quote_list("2a", "2b", "2c");
    list l3 = quote_list("3a", "3b", "3c");
    list l4 = quote_list("4a", "4b", "4c");
    list l0 = quote_list(l1, l2, l3);

    splice(l0, 0, 3, l1, l2, l3, l4);
    TEST(462, length_of_list(l0) == 4);

    splice(l0, 0, 0, l1, l2, l3, l4, l1, l2, l3, l4, l1, l2, l3, l4, l1, l2, l3, l4);
    TEST(463, length_of_list(l0) == 20);

    splice(l0, -1, 0, l0);                      /* self reference */
    TEST(464, length_of_list(l0) == 21);
}


/*
 *  test_sort_list ---
 *      sort_list and <=> operator.
 */
static int
sort_forward(/*const*/ string a, /*const*/ string b)
{
    return (a <=> b);                           /* test comparsion operator */
}


static int
sort_backward(/*const*/ string b, /*const*/ string a)
{
    return (a <=> b);                            /* test comparsion operator */
}


static void
test_sort_list(void)
{
    list list_0;
    list list_1 = {"a"};
    list list_2 = {"b", "a"};
    list list_3 = {"b", "a", "c"};
    list list_7 = {"c", "d", "a", "z", "b", "e", "f"};
    list r;

    r = sort_list(NULL);                        // error
    TEST(465, is_null(r));
    r = sort_list(list_0);
    TEST(466, is_null(r));
    r = sort_list(list_1);
    TEST(467, length_of_list(r) == 1 && r[0] == "a");
    r = sort_list(list_2);
    TEST(468, length_of_list(r) == 2 && r[0] == "a");
    r = sort_list(list_3);
    TEST(469, length_of_list(r) == 3 && r[0] == "a");
    r = sort_list(list_7);
    TEST(470, length_of_list(r) == 7 && r[0] == "a");

    r = sort_list(list_2, 0);                   // forward
    TEST(471, length_of_list(r) == 2 && r[0] == "a");
    r = sort_list(list_7, 0);
    TEST(472, length_of_list(r) == 7 && r[0] == "a");

    r = sort_list(list_2, 1);                   // backwards
    TEST(473, length_of_list(r) == 2 && r[0] == "b");
    r = sort_list(list_7, 1);
    TEST(474, length_of_list(r) == 7 && r[0] == "z");

    r = sort_list(list_2, "::sort_forward");
    TEST(475, length_of_list(r) == 2 && r[0] == "a");
    r = sort_list(list_7, "::sort_forward");
    TEST(476, length_of_list(r) == 7 && r[0] == "a");

    r = sort_list(list_2, "::sort_backward");
    TEST(477, length_of_list(r) == 2 && r[0] == "b");
    r = sort_list(list_7, "::sort_backward");
    TEST(478, length_of_list(r) == 7 && r[0] == "z");

    // qsort
    r = sort_list(list_7, NULL, 1);
    TEST(479, length_of_list(r) == 7 && r[0] == "a");

    // mergesort
    r = sort_list(list_7, NULL, 2);
    TEST(480, length_of_list(r) == 7 && r[0] == "a");

    // heapsort
    r = sort_list(list_7, NULL, 3);
    TEST(481, length_of_list(r) == 7 && r[0] == "a");
}


/*
*  test_sprintf ---
 *      Enhanced sprintf functionality including error cases.
 */
static void
test_sprintf(void)
{
    string s1;
    int i;

    // enhanced features
    TEST(482, sprintf(s1, "Hello world") == 11);
    TEST(483, sprintf(s1, "%*s", 20, "") == 20);
    TEST(484, sprintf(s1, "%b", 0xf3) == 8 && s1 == "11110011");
    TEST(485, sprintf(s1, "val=%B", 3, "\10\2BITTWO\1BITONE") == 20 &&
                s1 == "val=3<BITTWO,BITONE>");
    TEST(486, sprintf(s1, "12345%n6", "i") == 6 && i == 5);

    // error cases
    TEST(487, sprintf(s1, "%s", NULL)   && s1 == "<NULL>");
    TEST(488, sprintf(s1, "%s")         && s1 == "<bad-string>");
    TEST(489, sprintf(s1, "%d", NULL)   && s1 == "0");
    TEST(490, sprintf(s1, "%c", NULL)   && s1 == " ");
    TEST(491, sprintf(s1, "%.2f", NULL) && s1 == "0.00");
    TEST(492, sprintf(s1, "%y")         && s1 == "y");

    // format
    TEST(493, format("Hello World") == "Hello World");
    TEST(494, format("%b", 0xf3) == "11110011");

    // large argument list
    TEST(495, format("%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s",
                        "1.", "2.", "3.", "4.", "5.", "6.", "7.", "8.", "9.", "10.", "11.", "12.", "13.", "14.", "15.", "16.")
                  == "1.2.3.4.5.6.7.8.9.10.11.12.13.14.15.16.");

    TEST(496, format("%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s",
                        "1.", "2.", "3.", "4.", "5.", "6.", "7.", "8.", "9.", "10.", "11.", "12.", "13.", "14.", "15.", "16.",
                        "1.", "2.", "3.", "4.", "5.", "6.", "7.", "8.", "9.", "10.", "11.", "12.", "13.", "14.", "15.", "16.")
                  == "1.2.3.4.5.6.7.8.9.10.11.12.13.14.15.16.1.2.3.4.5.6.7.8.9.10.11.12.13.14.15.16.");

    TEST(497, format("%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s",
                        "1.", "2.", "3.", "4.", "5.", "6.", "7.", "8.", "9.", "10.", "11.", "12.", "13.", "14.", "15.", "16.",
                        "1.", "2.", "3.", "4.", "5.", "6.", "7.", "8.", "9.", "10.", "11.", "12.", "13.", "14.", "15.", "16.",
                        "1.", "2.", "3.", "4.", "5.", "6.", "7.", "8.", "9.", "10.", "11.", "12.", "13.", "14.", "15.", "16.")
                  == "1.2.3.4.5.6.7.8.9.10.11.12.13.14.15.16.1.2.3.4.5.6.7.8.9.10.11.12.13.14.15.16.1.2.3.4.5.6.7.8.9.10.11.12.13.14.15.16.");

    TEST(498, format("%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s",
			"1.", "2.", "3.", "4.", "5.", "6.", "7.", "8.", "9.", "10.", "11.", "12.", "13.", "14.", "15.", "16.",
			"1.", "2.", "3.", "4.", "5.", "6.", "7.", "8.", "9.", "10.", "11.", "12.", "13.", "14.", "15.", "16.",
			"1.", "2.", "3.", "4.", "5.", "6.", "7.", "8.", "9.", "10.", "11.", "12.", "13.", "14.", "15.", "16.",
			"1.", "2.", "3.", "4.", "5.", "6.", "7.", "8.", "9.", "10.", "11.", "12.", "13.", "14.", "15.", "16.")
                  == "1.2.3.4.5.6.7.8.9.10.11.12.13.14.15.16.1.2.3.4.5.6.7.8.9.10.11.12.13.14.15.16.1.2.3.4.5.6.7.8.9.10.11.12.13.14.15.16.1.2.3.4.5.6.7.8.9.10.11.12.13.14.15.16.");
}


/*
 *  test_replace ---
 *      replacement functionality test
 */
replacement int
__regress_replacement(int x,
        int a, string b, list c, declare d, ~int e, ~string f, ~list g, ~declare h)
{
    declare ret;

    UNUSED(a, b, c, d, e, f, g, h);

    //  call original, arg1 states the argument to be returned
    //
    switch(x) {
    case 1:
        //  return original 2th parameter
        //
        ret = __regress_replacement(2, a, b, c, d);
        TEST(499, ret == a);
        break;

    case 2:
        //  return original 2th parameter
        //
        ret = __regress_replacement(2, NULL, b, c, d);
        TEST(500, ret == a);
        break;

    case 3:
        //  return override 2th parameter
        //
        ret = __regress_replacement(2, 999, b, c, d);
        TEST(501, ret == 999);
        break;

    case 4:
        //  return original 6th parameter
        //
        ret = __regress_replacement(6, a, b, c, d);
        TEST(502, ret == e);
        break;

    case 5:
        //  return original 6th parameter
        //
        ret = __regress_replacement(6, a, b, c, d, NULL);
        TEST(503, ret == e);
    }
    return 42;
}


static void
test_replacement(void)
{
    list l3 = {3};
    list l7 = {7};
    int ret;

    ret = __regress_replacement(1, 1, "2", l3, 4, 5, "6", l7, 8);
    ret = __regress_replacement(2, 1, "2", l3, 4, 5, "6", l7, 8);
    ret = __regress_replacement(3, 1, "2", l3, 4, 5, "6", l7, 8);
    ret = __regress_replacement(4, 1, "2", l3, 4, 5, "6", l7, 8);
    ret = __regress_replacement(5, 1, "2", l3, 4, 5, "6", l7, 8);
    TEST(504, ret == 42);
}


/*
 *  test_isa ---
 *      isaxxx character class functionality.
 */
static void
test_isa(void)
{
    TEST(505, isalpha('A'));
    TEST(506, isascii('A'));
    TEST(507, iscntrl(0x01));
    TEST(508, iscsym('_'));
    TEST(509, isgraph('A'));
    TEST(510, isprint('A'));
    TEST(511, ispunct('!'));

    TEST(512, isdigit('1')   && isdigit(0x30)     && isdigit("a1a", 2));
    TEST(513, isdigit("1a1") && isdigit("a1a", 2) && !isdigit("a1a", -1) && !isdigit("", 1));
    TEST(514, islower('a')   && !islower('A'));
    TEST(515, isupper('A')   && !isupper('a'));
    TEST(516, isalnum('Z')   && !isalnum('*'));
    TEST(517, isxdigit('f')  && isxdigit('F')     && isxdigit('1'));
    TEST(518, isspace(' ')   && isspace('\t')     && isspace('\n')       && !isspace('a'));
}


/*
 *  test_basedir ---
 *      basename and dirname
 */
static void
test_basedir(void)
{
    TEST(519, basename("dir/file") == "file");
    TEST(520, basename("dir/file.c", ".c") == "file");
    TEST(521, basename("", "") == "");
    TEST(522, basename("//", "") == "/");
    TEST(523, basename("dir/file/") == "file");
    TEST(524, basename("dir\\file/") == "file");
    TEST(525, basename("/x/") == "x");
    TEST(526, basename("x/") == "x");
    TEST(527, basename("/x") == "x");

    TEST(528, dirname(".") == ".");
    TEST(529, dirname("/") == "/");
    TEST(530, dirname("//") == "/");
    TEST(531, dirname("/xx") == "/");
    TEST(532, dirname("//xx") == "/");
    TEST(533, dirname("aaa/bbb") == "aaa");
    TEST(534, dirname("aaaa//bbb/cccc///") == "aaaa//bbb" );
}


/*
 *  test_ebv ---
 *      putenv, setenv and expandpath
 */
static void
test_env(void)
{
    putenv("GRREGRESS", "home");
    TEST(535, getenv("GRREGRESS") == "home");
    TEST(536, expandpath("$$/filename", 0x3) == "$/filename");
    TEST(537, expandpath("$GRREGRESS", 0x3) == "home");
    TEST(538, expandpath("$GRREGRESS/", 0x3) == "home/");
    TEST(539, expandpath("${GRREGRESS}xxx/", 0x3) == "homexxx/");
    TEST(540, expandpath("$(GRREGRESS)xxx/", 0x3) == "homexxx/");

    /*invalid*/
    TEST(541, expandpath("${GRREGRESSxxx/", 0x3) == "${GRREGRESSxxx/");
    TEST(542, expandpath("$(GRREGRESSxxx/", 0x3) == "$(GRREGRESSxxx/");
    TEST(543, expandpath("${GRREGRESSxxx", 0x3) == "${GRREGRESSxxx");
    TEST(544, expandpath("$(GRREGRESSxxx", 0x3) == "$(GRREGRESSxxx");
    TEST(545, expandpath("$UNKNOWN/", 0x3) == "/");
    TEST(546, expandpath("$(UNKNOWN)xxx/", 0x3) == "xxx/");
    TEST(547, expandpath("${UNKNOWN}xxx/", 0x3) == "xxx/");
}


/*
 *  test_register ---
 *      register functionaliy.
 */
static void
test_register(void)
{
    int event = 0;

    register_macro(REG_REGRESS, "__regress_event");
    call_registered_macro(REG_REGRESS);
    TEST(548, 1 == event);

    event = 0;
    register_macro(REG_REGRESS, "__regress_event");
    reregister_macro(REG_REGRESS, "__regress_event");
    call_registered_macro(REG_REGRESS);
    TEST(549, 2 == event);

    event = 0;
    unregister_macro(REG_REGRESS, "__regress_event");
    call_registered_macro(REG_REGRESS);
    TEST(550, 1 == event);

    event = 0;
    unregister_macro(REG_REGRESS, "__regress_event");
    TEST(551, 0 == event);
}


void
__regress_event(void)
{
    extern int event;

    ++event;
}


static void
test_buffer(void)
{
    int curbuf = inq_buffer(), curwin = inq_window();
    int buf;

    save_position();
    if ((buf = create_buffer("-regress-buffer-", NULL, TRUE)) == -1) {
        return;
    }
    TEST(552, curbuf == inq_buffer());
    TEST(553, curbuf == set_buffer(buf));
    TEST(554, buf == inq_buffer());
    TEST(555, 0 != inq_system());
    TEST(556, (inq_buffer_flags(buf) & BF_SYSBUF) == BF_SYSBUF);
    TEST(557, 0 != inq_buffer_flags(NULL, "sysbuf"));
    TEST(558, 0 == inq_modified());
    //TODO
    //  set_attribute()
    //  inq_attribute()
    //  insert_buffer()
    //  inq_next_buffer()
    //  inq_prev_buffer()
    //
    {   const string sval = "1234567890abcdefghijklmnopqrstuvwxyz"+
                "1234567890abcdefghijklmnopqrstuvwxyz\n";
        const int slen = strlen(sval);
        int status, count;
        string line;

        top_of_buffer();
        count = insert(sval);
        TEST(559, slen == count);
        TEST(560, slen == inq_line_length());

        top_of_buffer();
        line = read(NULL, status);
        TEST(561, status == 1);                 /* EOF */
        TEST(562, line == sval);

        line = read(slen, status);
        TEST(563, status == 1);                 /* EOF */
        TEST(564, line == sval);

        line = read(1, status);
        TEST(565, status == 0);                 /* partial */
        TEST(566, line == "1");

        line = read(9, status);
        TEST(567, status == 0);                 /* partial */
        TEST(568, line == "123456789");

        top_of_buffer();
        count = insertf("%s\n", "abcdefg1234567890");
        TEST(569, 18 == count);
        TEST(570, slen == inq_line_length());

        top_of_buffer();
        line = read(NULL, status);
        TEST(571, status == 1);                 /* EOF */
        TEST(572, line == "abcdefg1234567890\n");
    }

    restore_position(2);
    TEST(573, curbuf == inq_buffer());
    TEST(574, curwin == inq_window());
    delete_buffer(buf);
}


static void
test_key(void)
{
    TEST(575, int_to_key(key_to_int("<Up>")) == "<Up>");
}


static void
test_debug(void)
{
    list l1;

    l1 = debug_support(DBG_INQ_VARS, 0);
    TEST(576, is_list(l1));
    l1 = debug_support(DBG_STACK_TRACE, NULL, "");
    TEST(577, is_list(l1));
    l1 = debug_support(DBG_INQ_VAR_INFO, 0, "l1");
    TEST(578, is_list(l1));
}


static void
test_strtol(void)
{
    int ret, endp;

    ret = strtol("xxx");                        /* basic */
    TEST(579, ret == 0);

    ret = strtol("1");                          /* dec */
    TEST(580, ret == 1);

    ret = strtol("01");                         /* oct */
    TEST(581, ret == 1);

    ret = strtol("0x1");                        /* hex */
    TEST(582, ret == 1);

    ret = strtol("g", NULL, 36);                /* base 36 (0123456789abc...) */
    TEST(583, ret == 16);

    ret = strtol("xxx", endp);                  /* invalid */
    TEST(584, ret == 0);
    TEST(585, endp == 0);

    ret = strtol("12", endp);
    TEST(586, ret == 12);
    TEST(587, endp == 3);
}


static void
test_strtof(void)
{
    float ret;
    int endp;

    ret = strtof("0.0");                        /* dec */
    TEST(588, ret == 0.0);

    ret = strtof("1.0");                        /* dec */
    TEST(589, ret == 1.0);

    ret = strtof("xxx", endp);                  /* invalid */
    TEST(590, ret == 0);
    TEST(591, endp == 0);

    ret = strtof("0.0");                        /* dec */
    TEST(592, ret == 0.0);

    ret = strtof("1.0");                        /* dec */
    TEST(593, ret == 1.0);

    ret = strtof("xxx", endp);                  /* invalid */
    TEST(594, ret == 0);
    TEST(595, endp == 0);
}


static void
test_strtod(void)
{
    float ret;
    int endp;

    ret = strtod("0.0");                        /* dec */
    TEST(596, ret == 0.0);

    ret = strtod("1.0");                        /* dec */
    TEST(597, ret == 1.0);

    ret = strtod("xxx", endp);                  /* invalid */
    TEST(598, ret == 0);
    TEST(599, endp == 0);

    ret = strtod("0.0");                        /* dec */
    TEST(600, ret == 0.0);

    ret = strtod("1.0");                        /* dec */
    TEST(601, ret == 1.0);

    ret = strtod("xxx", endp);                  /* invalid */
    TEST(602, ret == 0);
    TEST(603, endp == 0);
}


static void
test_defaults1(int value = 666)
{
    TEST(604, value == 666);
}


static void
test_defaults2(string value = "666")
{
    TEST(605, value == "666");
}


static void
test_defaults3(string value = 1.1)
{
    TEST(606, value == 1.1);
}


static void
test_defaults(void)
{
    test_defaults1();
    test_defaults2();
    test_defaults3();
}


static void
test_reference1(int &value)
{
    value = 2;
}


static void
test_reference2(string &value)
{
    value = "two";
}


static void
test_reference3(float &value)
{
    value = 2;
}


static void
test_reference(void)
{
    int ivalue = 1;
    string svalue = "one";
    float fvalue = 1;

    test_reference1(ivalue);
    TEST(607, ivalue == 2);
    test_reference2(svalue);
    TEST(608, svalue == "two");
    test_reference3(fvalue);
    TEST(609, fvalue == 2);
}


/*
 *  test_pushpop ---
 *      push/pop.
 */
static void
test_pushpop(void)
{
    list l;
    string msg;
    int i, success = TRUE;
    declare r;

    for (i = 0; i < 1000; ++i) {
        sprintf(msg, "work%04d", i);
        push(l, msg);
    }
    TEST(610, length_of_list(l) == 1000);

    for (i = i-1; i >= 0; --i) {
        sprintf(msg, "work%04d", i);
        if (msg != pop(l)) {
            success = FALSE;
            break;
        }
    }
    TEST(611, success);

    TEST(612, length_of_list(l) == 0);
    pause_on_error(0, FALSE);
    r = pop(l);
    pause_on_error(1, FALSE);
    TEST(613, is_null(r));                      /* FIXME/XXX - is_null(pop(l)) broken/limitation of LVAL's */
}


static void
test_globals(void)
{
    int line, col;

    TEST(614, inq_window() == current_window);
    TEST(615, inq_buffer() == current_buffer);
    inq_position(line, col);
    TEST(616, line == current_line);
    inq_position(line, col);
    TEST(617, col == current_col);
}


/*
 *  test_searh ---
 *      basic search primitives.
 */
static void
test_search(void)
{
    TEST(618, quote_regexp("<>") == "\\<\\>");
//  search string
//  search buffer
}


//  XXX - breaks compiler
//
//      static int test_macro(void) ;
//          <= function not defined
//      {
//      }
//


static void
test_called(void)
{
    TEST(619, "test_called" == caller());
    set_calling_name("");
    TEST(620, "" == caller());
    set_calling_name("test_called");
    TEST(621, "test_called" == caller());
    set_calling_name("hello_world");
    TEST(622, "hello_world" == caller());
    set_calling_name(inq_called());
    TEST(623, "regress" == caller());
    set_calling_name(NULL);                     /* extension, reset/clear */
    TEST(624, "test_called" == caller());
}


static string
caller(void)
{
    return inq_called();
}


/*
 *  test_macro ---
 *      inq_macro() tests.
 */
static void
test_macro(void)
{
    int ret;

    ret = inq_macro("regress");                 /* defined */
    TEST(625, ret == 1);

    ret = inq_macro("list_of_dictionaries");    /* builtin */
    TEST(626, ret == 0);

    ret = inq_macro("cut");                     /* replacement */
    TEST(627, ret == 2);

    ret = inq_macro("this_should_not_be_undefined");
    TEST(628, ret == -1);                       /* undefined */
}


static void
test_undef(void)
{
    extern int undefined_ival;
    extern float undefined_fval;
    extern string undefined_sval;
    int opause;

    opause = pause_on_error(0, FALSE);
//  //  control how 'undefines' are handled??
//  try {
        if (1) {
            TEST(629, inq_symbol("undefined_ival") == 0);
            TEST(630, inq_symbol("undefined_fval") == 0);
            TEST(631, inq_symbol("undefined_sval") == 0);
            TEST(632, undefined_ival == 0);     /* wont be executed! */
            TEST(633, undefined_fval == 0);     /* wont be executed! */
            TEST(634, undefined_sval == "");    /* wont be executed! */
        }
//  } catch {
//  } finally {
//  }
    pause_on_error(opause, FALSE);
}


/*
 *  test_history ---
 *      macro history.
 */
static void
test_history(void)
{
    const string top = inq_macro_history(0);

                                                /* <Alt-10> or from features */
    TEST(635, top == "execute_macro" || top == "sel_list");
    TEST(636, inq_command() == inq_macro_history());
    set_macro_history(0, "function1");
    set_macro_history(1, "function2");
    TEST(637, "function1" == inq_macro_history());
    TEST(638, "function2" == inq_macro_history(1));
}


/*
 *  test_scope ---
 *      basic scope tests.
 */
static void
test_scope(void)
{
    if (first_time()) {                         /* can only be run once */
        TEST(639, 1 == scope_1());
        TEST(640, 2 == scope_1());
        TEST(641, 3 == scope_1());

        TEST(642, 1 == scope_2());
        TEST(643, 2 == scope_2());
        TEST(644, 3 == scope_2());

        TEST(645, 1 == scope_3());
        TEST(646, 2 == scope_3());
        TEST(647, 3 == scope_3());
    } else {                                    /* emulate */
        extern int num_passed;
        num_passed += 9;
    }

    TESTASYNC(648, 0 == inq_symbol("x_extern_dontexist"));
    TEST(649, 0 != inq_symbol("x_static"));
}


static int
scope_1(void)
{
    return x_static++;
}


static int
scope_2(void)
{
    static int x_local = 1;
    return x_local++;
}


static int
scope_3(void)
{
    static int x_local = 1;
    return x_local++;
}


/*
 *  Tests to make sure we are not losing memory.
 */
#if (XXX)
    inq_module()
        inq_module() == ""                      /* not a module */

    inq_macro()
        inq_macro("home")                       /* known replacement */

    inq_symbol()
    find_macro()
    first_time()
#endif


/*
 *  test_dict ---
 *      dictionary support.
 */
static void
test_dict(void)
{
    declare var;
    int dict, dict2, ret;
    list l1;

    /* create dictionaries */
    dict = create_dictionary();
    TEST(650, dict);

    dict2 = create_dictionary("namedict");
    TEST(651, dict2);

    /* basic set/get primitives */
    set_property(dict, "property", 1234);
    var = get_property(dict, "property");
    TEST(652, 1234 == var);

    set_property(dict, "property", "hello world");
    var = get_property(dict, "property");
    TEST(653, "hello world" == var);

    /* indirection operators */
    dict.value = "value1";                      /* FIXME/XXX - compiler issues yet resolved */
    TEST(654, "value1" == dict.value);

    dict2.value = "value2";
    dict.indirect = dict2;                      /* FIXME/XXX - reference count issues */
    TEST(655, "value2" == dict.indirect.value);
    ret = dict_exists(dict, "property");
    TEST(656, ret);

    l1 = dict_list(dict);
    TEST(657, is_list(l1));
    TEST(658, 3 == length_of_list(l1));

    set_property(dict, "property2", 1234);
    set_property(dict, "property3", 5678);
    l1 = dict_list(dict);
    TEST(659, 5 == length_of_list(l1));

    // FIXME/XXX --
    //  compiler bug, allows declare when string expected.
    //  language issue, can not pass a declare when a string is expected.
    //
    string key;                                 /* new language/compiler feature */
    declare value;
    int idx, count;

    while ((idx = dict_each(dict, key, value)) >= 0) {
        switch(key) {
        case "property":
            TESTASYNC(660, "hello world" == value);
            ++count;
            break;
        case "property2":
            TESTASYNC(661, 1234 == value);
            ++count;
            break;
        case "property3":
            TESTASYNC(662, 5678 == value);
            ++count;
            break;
        }
    }
    TESTASYNC(663, 3 == count);

    ret = dict_delete(dict, "property");
    TEST(664, 0 == ret);

    l1 = dict_list(dict);
    TEST(665, is_list(l1));
    TEST(666, 4 == length_of_list(l1));

    l1 = list_of_dictionaries();
    TEST(667, is_list(l1));
    TEST(668, length_of_list(l1) >= 2);         /* all dictionaries */

    ret = delete_dictionary(dict);
    TEST(669, 0 == ret);
    ret = delete_dictionary(dict2);
    TEST(670, 0 == ret);
}



/*
 *  test_leaks ---
 *      Tests to make sure we are not losing memory.
 */
static int
test_leaks(void)
{
    TEST(671, leak_1(1000) == 1000);            /* list are limited 2^16 atoms */
}


static int
leak_1(int count)
{
    list l1;
    int i;

    l1 = NULL;
    for (i = 0; i < count; i++) {
        l1 += 0;
    }
    return length_of_list(l1);
}


static int
test_if(declare var)
{
    if (var) return TRUE;
    return FALSE;
}


/*
 *  test_ifbuf --
 *      Test an obscure bug caused by the if-stmt code.
 */
static int
test_ifbug(void)
{
    string line;

    line = xcompletion("Edit file:");
    return 1;
}


string
xcompletion(string file)
{
    if (1) {
        return xcompl_file(file);
    }
}


string
xcompl_file(string file)
{
    return file;
}


static int
test_elsif1(void)
{
    string token = "done";

    if (token == "one")
        ;
    else if (token == "two")
        ;
    else if (token == "done")
        return 999;
    return 0;
}


static int
test_elsif2(void)
{
    string token = "done";

    if (token != "one")
        if (token == "two")
            ;
        else if (token == "done")
            return 999;
    return 0;
}


/*
 *  regress_renumber ---
 *      renumber the regression tests ... use with care.
 */
void
regress_renumber(void)
{
    string file, line;
    int base = 0, test = 0;

    inq_names(NULL, NULL, file, NULL);
    if ("regress.cr" == file || "regress2.cr" == file) {
        if ("regress2.cr" == file) {
            base = test = 999;
        }
        save_position();
        top_of_buffer();
        while (re_search(0, "TES[A-Z]+([0-9]+,") > 0) {
            /*
             *  examples:
             *      TEST(672,
             *      TESTASYNC(673,
             */
            line = read(18);
            delete_char(index(line, ","));
            insertf("%.*s", index(line, "("), line);
            sprintf(line, "%d,", ++test);
            insert(line);
            beginning_of_line();
            down();
        }
        restore_position();
        message("regress_renumber: <%s> total tests: %d", file, test - base);

    } else {
        error("regress_renumber: current source file must be 'regress[2].cr'");
    }
}

/*end*/
