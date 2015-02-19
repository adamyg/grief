/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: regress.cr,v 1.33 2014/11/27 15:54:15 ayoung Exp $
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
            if (!(__cond)) failed(__x); else passed(__x);
#define TEST(x, cond)   __TEST(x, cond)

#define TESTASYNC(x, cond) \
            {extern int x_lastnum; x_lastnum=0;} if (!(cond)) failed(x); else passed(x);

#if defined(__PROTOTYPES__)
static void             passed(int num);
static void             failed(int num);

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
static void             test_math(void);
static void             test_switch(void);
static void             test_cexpr(void);
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
        test_math();
        test_switch();
        test_cexpr();
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

    /*
     *  Turn off buffer changed flag, because it's not worth saving
     *  the output from this macro
     */
    set_echo_format();
    pause_on_error(opause, FALSE);
    set_buffer_flags(NULL, NULL, ~BF_CHANGED);

    message("Tests passed: %d, failed: %d .. ", num_passed, num_failed);
    if (0 == num_failed) {
        return;
    }

    edit_file("Regression-Test");
    for (i = 0; i < length_of_list(failed_list);) {
        insert(failed_list[i++]);
    }
}


static void
passed(int num)
{
    extern list failed_list;
    extern int num_passed, x_lastnum;

    if (x_lastnum > 0 && (num - 1) != x_lastnum) {
        failed_list += ".. test seq " + num + " from " + x_lastnum + "\n";
    }
    x_lastnum = num;
    ++num_passed;
}


static void
failed(int num)
{
    extern list failed_list;
    extern int num_failed, x_lastnum;

    if (x_lastnum > 0 && (num - 1) != x_lastnum) {
        failed_list += ".. test seq" + num + "from " + x_lastnum + "\n";
    }
    x_lastnum = num;
    failed_list += "Test " + num + ": Failed.\n";
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
    float f1;
    int i;

    /*
     *  operators
     *      contant expressions
     */
    TEST(71, 1 + 2   == 3);                     // ADD
    TEST(72, 2 - 1   == 1);                     // MIN
    TEST(73, 8 / 2   == 4);                     // DIV
    TEST(74, 5 * 5   == 25);                    // MUL
    TEST(75, 12 % 10 == 2);                     // MOD

    TEST(76, 1 << 10 == 1024);                  // LSH
    TEST(77, 8 >> 3  == 1);                     // RSH
    TEST(78, (10 ^ 2) == 8);                    // XOR
    TEST(79, (0xff54 & 0xff) == 0x54);          // AND
    TEST(80, (0xff54 | 0xff) == 0xffff);        // OR
    TEST(81, (~0 & 0xffff) == 0xffff);          // COM

    /*
     *  operators
     *      non-contant expressions
     */
    i = 1;      TEST(82, i + 2      == 3);
    i = 2;      TEST(83, i - 1      == 1);
    i = 8;      TEST(84, i / 2      == 4);
    i = 5;      TEST(85, i * 5      == 25);
    i = 12;     TEST(86, i % 10     == 2);
    i = 0;      TEST(87, (~i & 0xffff) == 0xffff);
    i = 0;      TEST(88, (! i)      == 1);

    i = 1;      TEST(89, i << 10    == 1024);
    i = 8;      TEST(90, i >> 3     == 1);
    i = 10;     TEST(91, (i ^ 2)    == 8);
    i = 0xff54; TEST(92, (i & 0xff) == 0x54);
    i = 0xff54; TEST(93, (i | 0xff) == 0xffff);

    /*
     *  operators
     *      non-contant assignment expressions
     */
    i = 1;      TEST(94, i + 2      == 3);
    i = 2;      TEST(95, i - 1      == 1);
    i = 8;      TEST(96, i / 2      == 4);
    i = 5;      TEST(97, i * 5      == 25);
    i = 12;     TEST(98, i % 10     == 2);

    i = 1;      TEST(99, i << 10    == 1024);
    i = 8;      TEST(100, i >> 3     == 1);
    i = 10;     TEST(101, (i ^ 2)    == 8);
    i = 0xff54; TEST(102, (i & 0xff) == 0x54);
    i = 0xff54; TEST(103, (i | 0xff) == 0xffff);

    i = 69;     TEST(104, (i += 10)  == 79   && i == 79);
    i = 42;     TEST(105, (i -= 40)  == 2    && i == 2);
    i = 99;     TEST(106, (i /= 3)   == 33   && i == 33);
    i = 41;     TEST(107, (i *= 3)   == 123  && i == 123);
    i = 102;    TEST(108, (i %= 10)  == 2    && i == 2);

    i = 42;     TEST(109, (i <<= 3)  == 336  && i == 336);
    i = 8;      TEST(110, (i >>= 3)  == 1    && i == 1);
    i = 10;     TEST(111, (i ^= 2)   == 8    && i == 8);
    i = 0xf7;   TEST(112, (i &= 0xf) == 0x7  && i == 0x7);
    i = 0xf7;   TEST(113, (i |= 0xf) == 0xff && i == 0xff);

    /*
     *  type conversion
     */
    f1 = 2.34;
    i = f1;
    TEST(114, i == 2);
    TEST(115, is_integer(i));
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
    TEST(116, typeof(d1) == "string");

    d1 = 1;
    TEST(117, typeof(d1) == "integer");

    d1 = 1.2334;
    TEST(118, typeof(d1) == "float");

    d1 = NULL;
    TEST(119, typeof(d1) == "NULL");

    d1 = quote_list("one", "two");
    TEST(120, typeof(d1) == "list");

    /*
     *  XXX - unsure. maybe a bug/feature
     *      assigning NULL to list zeros the list yet the type is still "list".
     */
    d1 = NULL;
    TEST(121, typeof(d1) == "list");
    TEST(122, is_null(d1));
}


/*
 *  test_math ---
 *      Maths functionality
 */
static void
test_math(void)
{
    TEST(123, abs(1) == 1);
    TEST(124, abs(-2) == 2);

    //TODO
    //  acos(0);
    //  asin(0);
    //  atan(0);
    //  atan2(0)
    //  ceil(0);
    //  cos(0);
    //  cosh
    //  exp(1);
    //  fabs(0);
    //  floor(0);
    //  fmod(0);
    //  frexp(0);
    //  ldexp(0);
    //  log(0);
    //  log10(0);
    //  modf(0);
    //  pow(10, 2) == 100.0
    //  sin(0);
    //  sinh(0);
    //  sqrt(100, 2) == 10.0
    //  tan(0);
    //  tanh(0);

    //TODO
    //  [f]div
    //  mod
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
            TEST(125, 1 == i);
            break;
        case 2:
        case 3:
            TEST(126, 2 == i);
        case 5:
        case 4:
            TEST(127, 4 == i);
        default:
            TEST(128, 8 == i);
            break;
        }
    }
#pragma warning on

    switch(3) {
    case 1: i = 101; break;
    case 2: i = 102; break;
    case 3: i = 103; break;
    }
    TEST(129, i == 103);

    switch("hello") {
    case "hello, everybod": s1 = "first"; break;
    case "hello": s1 = "second";  break;
    default: s1 = "default";
    }
    TEST(130, s1 == "second");

    s1 = "hello, everybod";
    s2 = "hello";
#pragma warning off
    switch("hello") {
    case s1: s1 = "first"; break;
    case s2: s1 = "second"; break;
    default: s1 = "default"; break;
    }
#pragma warning on
    TEST(131, s1 == "second");

    TEST(132, "none"  == switchi(0));
    TEST(133, "one"   == switchi(1));
    TEST(134, "two"   == switchi(2));
    TEST(135, "three" == switchi(3));

    TEST(136, 0 == switchs(""));
    TEST(137, 1 == switchs("1"));
    TEST(138, 2 == switchs("22"));
    TEST(139, 3 == switchs("333"));
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

    TEST(140, 7 == i);
}


/*
 *  test_escapes ---
 *      escape sequences.
 */
static void
test_escapes(void)
{
    TEST(141, '\cA' == 0x00 && '\0' == 0x00 && '\000' == 0x00);
                                                // NUL
    TEST(142, '\c[' == 0x1b && '\e' == 0x1b);   // ESC
#pragma warning(push, off)
    TEST(143, L'\u2b3c'     == 0x2b3c);         // Unicode
    TEST(144, L'\U00012b3c' == 0x12b3c);        // Unicode (76604 dec)
    TEST(145, L'\x{1a2b}'   == 0x1a2b);         // Extended hexidecimal contants.
    TEST(146, L'\o{1234}'   == 01234);          // Extended octal contants.
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
    TEST(147, !empty);                          // undocumented feature, scalar reference.
    TEST(148, nonempty);                        //  only function as 'single' expression NOT within compound

    // string addition
    s1 = "xyz"; s1 += "abc";
    TEST(149, s1 == "xyzabc");

    s1 = "xyz"; s2 = "abc"; s1 += s2;
    TEST(150, s1 == "xyzabc");

    s1 = "xyz"; s2 = s1; s1 += s2;
    TEST(151, s1 == "xyzxyz");

    s1 = "xyz";
    TEST(152, (s1 += "abc") == "xyzabc");

    s1 = "xyz";
    TEST(153, (s1 += s1) == "xyzxyz");

    s1 = "xyz";
    TEST(154, (s1 = s1) == "xyz");

    // numeric conversion
    i = 99;
    s1 = "abc";
    s1 = s1 + i;
    TEST(155, s1 == "abc99");

    i = 99;
    s1 = "abc";
    s1 = i + s1;
    TEST(156, s1 == "99abc");

    s1 = "abc";
    s1 += 0;
    TEST(157, s1 == "abc0");

    f = 1.234;
    s1 = "abc";
    s1 = f + s1;
    TEST(158, s1 == "1.234abc");

    // string multipler
    //  trailing or leading
    //  integer or float multipler
    //
    i = 0;
    s1 = "xyz" * i;
    TEST(159, s1 == "");
    s1 = i * "xyz";
    TEST(160, s1 == "");

    i = 1;
    s1 = "xyz" * i;
    TEST(161, s1 == "xyz");
    s1 = i * "xyz";
    TEST(162, s1 == "xyz");

    i = 2;
    s1 = "xyz" * i;
    TEST(163, s1 == "xyzxyz");
    s1 = i * "xyz";
    TEST(164, s1 == "xyzxyz");

    f = 1.1;
    s1 = "xyz" * f;
    f = 2.2;
    s1 = "xyz" * f;
    TEST(165, s1 == "xyzxyz");

    // string accumulator tests
    TEST(166, "abc" + "def" == "abcdef");
    TEST(167, 1 + "def" == "1def");
    TEST(168, "abc" + 1 == "abc1");
    TEST(169, 1.2 + "def" == "1.2def");
    TEST(170, "abc" + 1.2 == "abc1.2");

    // string primitives
    TEST(171, trim("  harry  ") == "harry");
    TEST(172, rtrim("harry  ") == "harry");
    TEST(173, ltrim("  harry") == "harry");

    s1 = "  harry  ";
    TEST(174, rtrim(s1) == "  harry");
    TEST(175, ltrim(s1) == "harry  ");

    TEST(176, atoi("98") == 98);
    TEST(177, strlen("abcd") == 4);
    s1 = "abcd" + " ";
    TEST(178, strlen(s1) == 5);

    TEST(179, strlen("nothing") == 7);

    s1 = "1234554321";
    TEST(180, index(s1, "4") == 4);
    TEST(181, index(s1, "") == 11);
    TEST(182, rindex(s1, "5") == 6);

    s1 = "";
    s1 = substr(s1, index(s1, ";") + 1);
    TEST(183, s1 == "");

    gs1 = "";
    get_parm(2, gs1);
    gs1 = substr(gs1, index(gs1, ";") + 1);
    TEST(184, gs1 == "");

    TEST(185, upper("aBc") == "ABC");
    TEST(186, lower("AbC") == "abc");
    TEST(187, 5 == string_count("axba bax", "abz"));

    TEST(188, 0 == strcasecmp("AaA", "aAa"));
    TEST(189, strerror(0) == "Success");

    s1 = "abAB";
    TEST(190, "a"  == strpop(s1));
    TEST(191, "b"  == strpop(s1));
    TEST(192, "AB" == strpop(s1, 2));
    TEST(193, ""   == strpop(s1));

    s1 = "abcdefg";
    TEST(194, 3 == strpbrk(s1, "dc"));

    // strstr, strrstr and strcasestr
    s1 = "abcmandefg";
    TEST(195, 4 == strstr(s1, "man"));
    TEST(196, 4 == strrstr(s1, "man"));
    TEST(197, 4 == strcasestr(s1, "MAN"));

    s1 = "abcmanmandefg";
    TEST(198, 4 == strstr(s1, "man"));
    TEST(199, 7 == strrstr(s1, "man"));
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
    TEST(200, 6 == search_string(" [worl]+", s1, i, -2 /*MAXIMUM*/, FALSE));
    TEST(201, 5 == i);

    i = -1;                                     /* XXX - Brief minimal logic needs reviewing */
    TEST(202, 6 == re_search(SF_BRIEF|SF_IGNORE_CASE, " [worl]+", s1, NULL, i));
    TEST(203, 2 == i);

    i = -1;
    TEST(204, 6 == re_search(SF_BRIEF|SF_MAXIMAL|SF_IGNORE_CASE, " [worl]+", s1, NULL, i));
    TEST(205, 5 == i);

    i = -1;
    TEST(206, 6 == re_search(SF_UNIX|SF_IGNORE_CASE, " [worl]+", s1, NULL, i));
    TEST(207, 5 == i);

    i = -1;
    TEST(208, 6 == re_search(SF_EXTENDED|SF_IGNORE_CASE, " [worl]+", s1, NULL, i));
    TEST(209, 5 == i);

    i = -1;
    TEST(210, 6 == re_search(SF_PERL|SF_IGNORE_CASE, " [worl]+", s1, NULL, i));
    TEST(211, 5 == i);

    i = -1;
    TEST(212, 6 == re_search(SF_TRE|SF_IGNORE_CASE, " [worl]+", s1, NULL, i));
    TEST(213, 5 == i);

    TEST(214, 6 == re_search(SF_PERL|SF_IGNORE_CASE, " [worl]+", s1, NULL, i));
    TEST(215, 5 == i);

    s1 = "Hello World";
    TEST(216, 6 == re_search(SF_PERL|SF_IGNORE_CASE, " ([worl]+)", s1, NULL, i));
    TEST(217, 5 == i);

    //  captures
    //
    s1 = "y aabbccddee z";
    s2 = "y XaabbccddeeX z";

    TEST(218, re_search(SF_BRIEF,       " {[abcde]+} ", s1, NULL, i) == 2);

    TEST(219, re_search(SF_UNIX,        " \\([abcde]+\\) ", s1, NULL, i) == 2);

    TEST(220, re_search(SF_EXTENDED,    " ([abcde]+) ", s1, NULL, i) == 2);

    TEST(221, re_search(SF_PERL,        " ([abcde]+) ", s1, NULL, i) == 2);

    TEST(222, re_translate(SF_BRIEF|SF_MAXIMAL, "{[abcde]+}", "X\\0X", s1) == s2);

    TEST(223, re_translate(SF_UNIX,     "\\([abcde]+\\)", "X\\0X", s1) == s2);

    TEST(224, re_translate(SF_EXTENDED, "([abcde]+)", "X\\0X", s1) == s2);

    TEST(225, re_translate(SF_PERL,     "([abcde]+)", "X$1X", s1) == s2);

    TEST(226, re_translate(SF_PERL,     "([abcde]+)", "$`", s1) == "y y  z");

    TEST(227, re_translate(SF_PERL,     "([abcde]+)", "$'", s1) == "y  z z");

    TEST(228, re_translate(SF_TRE,      "([abcde]+)", "X$1X", s1) == s2);
}


/*
 *  test_sub ---
 *      sub and gsub macros
 */
static void
test_sub(void)
{
    TEST(229, gsub("ana", "anda", "banana") == "bandana");
    TEST(230, gsub("a",   "-&-",  "banana") == "b-a-n-a-n-a-");
    TEST(231, gsub("a+",  "-&-",  "banana") == "b-a-n-a-n-a-");
    TEST(232, gsub("a+",  "-a-",  "baaaanaaaaanaaaa") == "b-a-n-a-n-a-");
    TEST(233, sub("ana",  "anda", "banana") == "bandana");
    TEST(234, sub("a",    "-&-",  "banana") == "b-a-nana");
    TEST(235, sub("a+",   "-&-",  "banana") == "b-a-nana");
    TEST(236, sub("a+",   "-a-",  "baaaanaaaaanaaaa") == "b-a-naaaaanaaaa");
    TEST(237, sub("na$",  "na.",  "banana") == "banana.");
    TEST(238, gsub("^a",  "A",    "anana")  == "Anana");
    TEST(239, gsub("n.n", "[&]",  "banana") == "ba[nan]a");
    TEST(240, gsub("a|n", "z",    "anna")   == "zzzz");

    TEST(241, sub("f\\(.*\\)t",   "F\\1T", "first:second") == "FirsT:second");
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
    TEST(242, s1 == "ABC");

    s1 = substr("ABC", -1000, 1000);
    TEST(243, s1 == "ABC");

    s1 = substr("ABC", 1000, 1000);
    TEST(244, s1 == "");

    s1 = substr("ABC", 1, 0);
    TEST(245, s1 == "");

    s1 = substr("ABC", 1, 1);
    TEST(246, s1 == "A");

    s1 = substr("ABC", 1, 2);
    TEST(247, s1 == "AB");

    s1 = substr("ABC", 1, 3);
    TEST(248, s1 == "ABC");

    s1 = substr("ABC", 1, 100);
    TEST(249, s1 == "ABC");

    s1 = substr("ABC", 3, 0);
    TEST(250, s1 == "");

    s1 = substr("ABC", 3, 1);
    TEST(251, s1 == "C");

    s1 = substr("ABC", 3, 100);
    TEST(252, s1 == "C");
}


/*
 *  test_compress ---
 *      compress
 */
static void
test_compress(void)
{
    // basic
    TEST(253, compress("  harry  ", 0) == " harry ");
    TEST(254, compress("  harry  ", 1) == "harry");
    TEST(255, compress("  h  a  r   r  y  ", 0) == " h a r r y ");
    TEST(256, compress(" h  a  r   r  y ", 0) == " h a r r y ");
    TEST(257, compress("  h  a  r   r  y  ", 1) == "h a r r y");

    // extended features
    TEST(258, compress("  harry  ", 0, " r") == " ha y ");
    TEST(259, compress("  harry  ", 0, " r", 'x') == "xhaxyx");
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
    TEST(260, length_of_list(l1) == 4);

    l2 = l1;
    TEST(261, length_of_list(l2) == 4);
    TEST(262, l1[0] == l2[0]);
    TEST(263, l1[1] == l2[1]);
    TEST(264, l1[2] == l2[2]);

    d1 = l1[0];
    TEST(265, is_integer(d1));                  /* 123 */
    TEST(266, is_type(d1, "integer"));

    d1 = l1[1];
    TEST(267, is_float(d1));                    /* 1.23 */
    TEST(268, is_type(d1, "float"));

    d1 = l1[2];
    TEST(269, is_string(d1));                   /* "xyz" */
    TEST(270, is_type(d1, "string"));

    d1 = l1[3];
    TEST(271, is_list(d1));                     /* hello() */
    TEST(272, is_type(d1, "list"));

    pause_on_error(0, FALSE);
    d1 = l1[4];                                 /* range error */
    pause_on_error(1, FALSE);
    TEST(273, is_null(d1));
    TEST(274, is_type(d1, "null"));

    // 2.
    l1 = quote_list(1);
    l1[0] = 2;
    TEST(275, l1[0] == 2);

    l1 = quote_list(1, "abc");
    l1[0] = 2;
    TEST(276, l1[0] == 2);

    l1 = quote_list("abc");
    l1[0] = 2;
    TEST(277, l1[0] == 2);

    l1 = quote_list("abc", 1);
    l1[1] = 2;
    TEST(278, l1[1] == 2);

    l1 = quote_list(1, "abc", 3);
    l1[1] = 2;
    TEST(279, l1[1] == 2);

    l1 = quote_list(1, 2, 3);
    l1[1] = "abc";
    TEST(280, l1[1] == "abc");

    l1 = quote_list(1, 2, 3);
    l2 = l1;
    l1[1] = l2;
    TEST(281, length_of_list(l1) == 5);

    l1 = quote_list(1, 2, 3);
    l1[1] = quote_list(1, 2, 3);
    TEST(282, length_of_list(l1) == 5);

    TEST(283, 1. == 1);

    l1 = quote_list(1, 2, 3);
    l1[1] = make_list(quote_list(1, 2, 3));
    l1[3] = "end";
    TEST(284, l1[3] == "end");
    TEST(285, length_of_list(l1) == 4);

    l3[0] = 0;
    l3[1] = 1;
    l3[2] = 2;
    TEST(286, l3[0] == 0);
    TEST(287, l3[1] == 1);
    TEST(288, l3[2] == 2);

    l1 = NULL;
    TEST(289, length_of_list(l1) == 0);

    l1[0] = "hello";
    TEST(290, l1[0] == "hello");

    s1 = "abc";
    l1[0] = s1;
    TEST(291, l1[0] == "abc");

    s1 = "abc";
    l1[0] = s1;
    s1 = "123456789";
    TEST(292, l1[0] == "abc");

    b57 = "hello";
    a57 = b57;
    TEST(293, a57 == "hello");

    TEST(294, test_6() == 99);

    l1 = quote_list("one", "", "three");
    s1 = "TWO";
    l1[1] = s1;
    l1[1] = s1;
    TEST(295, l1[1] == "TWO");

    l1 = quote_list(1, 2, 3);
    l2 = make_list(l1);
    l1[1] = l2;
    TEST(296, length_of_list(l1) == 3);

    l1 = quote_list("hello", "list", 1, NULL, 2.3);
    l2 = make_list(l1);
    l1[1] = l2;
    TEST(297, length_of_list(l1) == 5);

    l1 = quote_list(1, 2, 3);
    l1[1] = make_list(quote_list(1, 2, 3));
    TEST(298, length_of_list(l1) == 3);

    l1 = quote_list(1, 2, 3);
    l1[2] = make_list(quote_list(1, 2, 3));
    TEST(299, length_of_list(l1) == 3);

    l1 = quote_list(1, 2, 3);
    l1[0] = make_list(quote_list(1, 2, 3));
    TEST(300, length_of_list(l1) == 3);

    l1 = make_list(quote_list(1, 2, 3));
    l1 += "abc";
    l1 += "def";
    l1[1] = 1;
    TEST(301, length_of_list(l1) == 3);

    l1 = make_list(quote_list(1, 2, 3), quote_list(1, 2, 3));
    TEST(302, car(car(l1)) == 1);
    TEST(303, length_of_list(l1) == 2);

    l1 = NULL;
    TEST(304, length_of_list(l1) == 0);
    pause_on_error(0, FALSE);
    nth(l1, 99);                                /* subscript out of range */
    pause_on_error(1, FALSE);

    // list functions
    l1 = command_list();
    TEST(305, is_list(l1));

    l1 = macro_list();
    TEST(306, is_list(l1));

    l1 = get_term_keyboard();
    TEST(307, is_list(l1));

    l1 = key_list();
    TEST(308, is_list(l1));

    l1 = bookmark_list();
    TEST(309, is_list(l1));

    l1 = macro_list();
    TEST(310, strlen(l1) != 0);
    TEST(311, length_of_list(cdr(macro_list())) > 0);
    TEST(312, search_list(NULL, "def", quote_list("abc", "def", "ghi")) == 1);
    l1 = quote_list("abc");
    l1 += "def";
    l1 += "ghi";
    TEST(313, search_list(NULL, "def", l1) == 1);

    l1 = inq_feature();
    TEST(314, is_list(l1));

    // list concat
    l1 = quote_list(1, "2", 3.0);
    l1 = l1 + l1;
    TEST(315, length_of_list(l1) == 6);
    l2 = l1;

    // element insert
    l1[3] = quote_list("a", "b", "c");
    TEST(316, length_of_list(l1) == 8);

    // element replace
    l1[0] = 99;
    TEST(317, l1[0] != l2[0]);

    // re_search
    l1 = quote_list("abc");
    l1 += "def";
    l1 += "ghi";
    TEST(318, re_search(NULL, "def", l1) == 1);
}


/*
 *  test_list2 ---
 *      extended list primitives (basic splice interface)
 */
static void
test_list2(void)
{
    list l1 = {"one", "two", "three", "four"};

    TEST(319, shift(l1) == "one");
    TEST(320, 3 == length_of_list(l1));         // two, three, four

    TEST(321, 5 == unshift(l1, "first", "second"));
    TEST(322, 5 == length_of_list(l1));         // first, second, two, three, four

    TEST(323, pop(l1) == "four");               // first, second, two, three
    TEST(324, 4 == length_of_list(l1));

    TEST(325, shift(l1) == "first" && shift(l1) == "second");
    TEST(326, 2 == length_of_list(l1));         // two, three

    TEST(327, pop(l1) == "three");              // two
    TEST(328, 1 == length_of_list(l1));

    TEST(329, shift(l1) == "two");              // null
    TEST(330, 0 ==  length_of_list(l1));
    TEST(331, is_null(l1));

    push(l1, "five", "six");
    TEST(332, 2 ==  length_of_list(l1));
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
            TEST(333, 1 == value);
            break;
        case 1:
            TEST(334, "2" == value);
            break;
        case 2:
            TEST(335, 3.3 == value);
            break;
        case 3:
            TEST(336, 4 == value);
            break;
        case 4:
            TEST(337, "5" == value);
            break;
        case 5:
            TEST(338, 6.6 == value);
            break;
        }
        ++count;
    }
    TEST(339, length_of_list(l1) == count);
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
    TEST(340, gl1[1] == "TWO");
    TEST(341, gl1[3][0] == "x");
    TEST(342, gl1[3][1] == "y");
    TEST(343, gl1[2][1][1] == "2.1.1");
    TEST(344, gl1[gl2[0]][gl2[1]][gl2[2]] == "2.1.1");
    TEST(345, gl1[test_10(2)][test_10(1)][test_10(1)] == "2.1.1");
    pause_on_error(0, FALSE);
    d1 = gl1[2][1][4];                          /* range error */
    pause_on_error(1, FALSE);
    TEST(346, is_null(d1));
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
    TEST(347, length_of_list(l) == 4 && l[0] == "a" && l[1] == "b" && l[2] == "a" && l[3] == "b");

    /* [x, x, b, a, b] */
    l[0] = x;
    TEST(348, length_of_list(l) == 5 && l[0] == "x" && l[1] == "x");

    /* [x, x, b, a, b, Nul, Nul, Nul, Nul, x, x] */
    l[9] = x;
    TEST(349, length_of_list(l) == 11 && l[9] == "x");

    /* Join */
    l = splice_l1 + splice_l1;
    TEST(350, length_of_list(l) == 6);

    /* Append */
    l += splice_l1;
    TEST(351, length_of_list(l) == 9);
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
    TEST(352, length_of_list(l) == 3 && l[2] == "blue");

    splice(l, 999);
    TEST(353, length_of_list(l) == 3 && l[2] == "blue");

    splice(l, 3);
    TEST(354, length_of_list(l) == 3 && l[2] == "blue");

    splice(l, 2);
    TEST(355, length_of_list(l) == 2 && l[1] == "green");

    splice(l, 0);
    TEST(356, length_of_list(l) == 0);

    splice(l, 0);
    TEST(357, length_of_list(l) == 0);
}


static void
splice_3(void)
{
    list l1 = quote_list("red", "green", "blue");
    list l2 = quote_list("red", "green", "blue");

    splice(l1, 1, 2, "yellow", "orange", "pink");
    TEST(358, length_of_list(l1) == 4 && l1[0] == "red" && l1[1] == "yellow" && l1[2] == "orange" && l1[3] == "pink");

    splice(l2, 1, 2, "yellow");
    TEST(359, length_of_list(l2) == 2 && l2[0] == "red" && l2[1] == "yellow");

    splice(l2, -1, 0, "orange");
    TEST(360, length_of_list(l2) == 3 && l2[0] == "red" && l2[1] == "yellow" && l2[2] == "orange");
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
    TEST(361, length_of_list(l0) == 4);

    splice(l0, 0, 0, l1, l2, l3, l4, l1, l2, l3, l4, l1, l2, l3, l4, l1, l2, l3, l4);
    TEST(362, length_of_list(l0) == 20);

    splice(l0, -1, 0, l0);                      /* self reference */
    TEST(363, length_of_list(l0) == 21);
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
    TEST(364, is_null(r));
    r = sort_list(list_0);
    TEST(365, is_null(r));
    r = sort_list(list_1);
    TEST(366, length_of_list(r) == 1 && r[0] == "a");
    r = sort_list(list_2);
    TEST(367, length_of_list(r) == 2 && r[0] == "a");
    r = sort_list(list_3);
    TEST(368, length_of_list(r) == 3 && r[0] == "a");
    r = sort_list(list_7);
    TEST(369, length_of_list(r) == 7 && r[0] == "a");

    r = sort_list(list_2, 0);                   // forward
    TEST(370, length_of_list(r) == 2 && r[0] == "a");
    r = sort_list(list_7, 0);
    TEST(371, length_of_list(r) == 7 && r[0] == "a");

    r = sort_list(list_2, 1);                   // backwards
    TEST(372, length_of_list(r) == 2 && r[0] == "b");
    r = sort_list(list_7, 1);
    TEST(373, length_of_list(r) == 7 && r[0] == "z");

    r = sort_list(list_2, "::sort_forward");
    TEST(374, length_of_list(r) == 2 && r[0] == "a");
    r = sort_list(list_7, "::sort_forward");
    TEST(375, length_of_list(r) == 7 && r[0] == "a");

    r = sort_list(list_2, "::sort_backward");
    TEST(376, length_of_list(r) == 2 && r[0] == "b");
    r = sort_list(list_7, "::sort_backward");
    TEST(377, length_of_list(r) == 7 && r[0] == "z");

    // qsort
    r = sort_list(list_7, NULL, 1);
    TEST(378, length_of_list(r) == 7 && r[0] == "a");

    // mergesort
    r = sort_list(list_7, NULL, 2);
    TEST(379, length_of_list(r) == 7 && r[0] == "a");

    // heapsort
    r = sort_list(list_7, NULL, 3);
    TEST(380, length_of_list(r) == 7 && r[0] == "a");
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
    TEST(381, sprintf(s1, "Hello world") == 11);
    TEST(382, sprintf(s1, "%*s", 20, "") == 20);
    TEST(383, sprintf(s1, "%b", 0xf3) == 8 && s1 == "11110011");
    TEST(384, sprintf(s1, "val=%B", 3, "\10\2BITTWO\1BITONE") == 20 &&
                s1 == "val=3<BITTWO,BITONE>");
    TEST(385, sprintf(s1, "12345%n6", "i") == 6 && i == 5);

    // error cases
    TEST(386, sprintf(s1, "%s", NULL)   && s1 == "<NULL>");
    TEST(387, sprintf(s1, "%s")         && s1 == "<bad-string>");
    TEST(388, sprintf(s1, "%d", NULL)   && s1 == "0");
    TEST(389, sprintf(s1, "%c", NULL)   && s1 == " ");
    TEST(390, sprintf(s1, "%.2f", NULL) && s1 == "0.00");
    TEST(391, sprintf(s1, "%y")         && s1 == "y");

    // format
    TEST(392, format("Hello World") == "Hello World");
    TEST(393, format("%b", 0xf3) == "11110011");
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
        TEST(394, ret == a);
        break;

    case 2:
        //  return original 2th parameter
        //
        ret = __regress_replacement(2, NULL, b, c, d);
        TEST(395, ret == a);
        break;

    case 3:
        //  return override 2th parameter
        //
        ret = __regress_replacement(2, 999, b, c, d);
        TEST(396, ret == 999);
        break;

    case 4:
        //  return original 6th parameter
        //
        ret = __regress_replacement(6, a, b, c, d);
        TEST(397, ret == e);
        break;

    case 5:
        //  return original 6th parameter
        //
        ret = __regress_replacement(6, a, b, c, d, NULL);
        TEST(398, ret == e);
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
    TEST(399, ret == 42);
}


/*
 *  test_isa ---
 *      isaxxx character class functionality.
 */
static void
test_isa(void)
{
    TEST(400, isalpha('A'));
    TEST(401, isascii('A'));
    TEST(402, iscntrl(0x01));
    TEST(403, iscsym('_'));
    TEST(404, isgraph('A'));
    TEST(405, isprint('A'));
    TEST(406, ispunct('!'));

    TEST(407, isdigit('1')   && isdigit(0x30)     && isdigit("a1a", 2));
    TEST(408, isdigit("1a1") && isdigit("a1a", 2) && !isdigit("a1a", -1) && !isdigit("", 1));
    TEST(409, islower('a')   && !islower('A'));
    TEST(410, isupper('A')   && !isupper('a'));
    TEST(411, isalnum('Z')   && !isalnum('*'));
    TEST(412, isxdigit('f')  && isxdigit('F')     && isxdigit('1'));
    TEST(413, isspace(' ')   && isspace('\t')     && isspace('\n')       && !isspace('a'));
}


/*
 *  test_basedir ---
 *      basename and dirname
 */
static void
test_basedir(void)
{
    TEST(414, basename("dir/file") == "file");
    TEST(415, basename("dir/file.c", ".c") == "file");
    TEST(416, basename("", "") == "");
    TEST(417, basename("//", "") == "/");
    TEST(418, basename("dir/file/") == "file");
    TEST(419, basename("dir\\file/") == "file");
    TEST(420, basename("/x/") == "x");
    TEST(421, basename("x/") == "x");
    TEST(422, basename("/x") == "x");

    TEST(423, dirname(".") == ".");
    TEST(424, dirname("/") == "/");
    TEST(425, dirname("//") == "/");
    TEST(426, dirname("/xx") == "/");
    TEST(427, dirname("//xx") == "/");
    TEST(428, dirname("aaa/bbb") == "aaa");
    TEST(429, dirname("aaaa//bbb/cccc///") == "aaaa//bbb" );
}


/*
 *  test_ebv ---
 *      putenv, setenv and expandpath
 */
static void
test_env(void)
{
    putenv("GRREGRESS", "home");
    TEST(430, getenv("GRREGRESS") == "home");
    TEST(431, expandpath("$$/filename", 0x3) == "$/filename");
    TEST(432, expandpath("$GRREGRESS", 0x3) == "home");
    TEST(433, expandpath("$GRREGRESS/", 0x3) == "home/");
    TEST(434, expandpath("${GRREGRESS}xxx/", 0x3) == "homexxx/");
    TEST(435, expandpath("$(GRREGRESS)xxx/", 0x3) == "homexxx/");

    /*invalid*/
    TEST(436, expandpath("${GRREGRESSxxx/", 0x3) == "${GRREGRESSxxx/");
    TEST(437, expandpath("$(GRREGRESSxxx/", 0x3) == "$(GRREGRESSxxx/");
    TEST(438, expandpath("${GRREGRESSxxx", 0x3) == "${GRREGRESSxxx");
    TEST(439, expandpath("$(GRREGRESSxxx", 0x3) == "$(GRREGRESSxxx");
    TEST(440, expandpath("$UNKNOWN/", 0x3) == "/");
    TEST(441, expandpath("$(UNKNOWN)xxx/", 0x3) == "xxx/");
    TEST(442, expandpath("${UNKNOWN}xxx/", 0x3) == "xxx/");
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
    TEST(443, 1 == event);

    event = 0;
    register_macro(REG_REGRESS, "__regress_event");
    reregister_macro(REG_REGRESS, "__regress_event");
    call_registered_macro(REG_REGRESS);
    TEST(444, 2 == event);

    event = 0;
    unregister_macro(REG_REGRESS, "__regress_event");
    call_registered_macro(REG_REGRESS);
    TEST(445, 1 == event);

    event = 0;
    unregister_macro(REG_REGRESS, "__regress_event");
    TEST(446, 0 == event);
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
    TEST(447, curbuf == inq_buffer());
    TEST(448, curbuf == set_buffer(buf));
    TEST(449, buf == inq_buffer());
    TEST(450, 0 != inq_system());
    TEST(451, (inq_buffer_flags(buf) & BF_SYSBUF) == BF_SYSBUF);
    TEST(452, 0 != inq_buffer_flags(NULL, "sysbuf"));
    TEST(453, 0 == inq_modified());
    //TODO
    //  set_attribute()
    //  inq_attribute()
    //  inq_line_length
    //  insert()
    //  insertf()
    //  insert_buffer()
    //  read()
    //  set_buffer()
    //  inq_next_buffer()
    //  inq_prev_buffer()
    //
    restore_position(2);
    TEST(454, curbuf == inq_buffer());
    TEST(455, curwin == inq_window());
    delete_buffer(buf);
}


static void
test_key(void)
{
    TEST(456, int_to_key(key_to_int("<Up>")) == "<Up>");
}


static void
test_debug(void)
{
    list l1;

    l1 = debug_support(DBG_INQ_VARS, 0);
    TEST(457, is_list(l1));
    l1 = debug_support(DBG_STACK_TRACE, NULL, "");
    TEST(458, is_list(l1));
    l1 = debug_support(DBG_INQ_VAR_INFO, 0, "l1");
    TEST(459, is_list(l1));
}


static void
test_strtol(void)
{
    int ret, endp;

    ret = strtol("xxx");                        /* basic */
    TEST(460, ret == 0);

    ret = strtol("1");                          /* dec */
    TEST(461, ret == 1);

    ret = strtol("01");                         /* oct */
    TEST(462, ret == 1);

    ret = strtol("0x1");                        /* hex */
    TEST(463, ret == 1);

    ret = strtol("g", NULL, 36);                /* base 36 (0123456789abc...) */
    TEST(464, ret == 16);

    ret = strtol("xxx", endp);                  /* invalid */
    TEST(465, ret == 0);
    TEST(466, endp == 0);

    ret = strtol("12", endp);
    TEST(467, ret == 12);
    TEST(468, endp == 3);
}


static void
test_strtof(void)
{
    float ret;
    int endp;

    ret = strtof("0.0");                        /* dec */
    TEST(469, ret == 0.0);

    ret = strtof("1.0");                        /* dec */
    TEST(470, ret == 1.0);

    ret = strtof("xxx", endp);                  /* invalid */
    TEST(471, ret == 0);
    TEST(472, endp == 0);

    ret = strtof("0.0");                        /* dec */
    TEST(473, ret == 0.0);

    ret = strtof("1.0");                        /* dec */
    TEST(474, ret == 1.0);

    ret = strtof("xxx", endp);                  /* invalid */
    TEST(475, ret == 0);
    TEST(476, endp == 0);
}


static void
test_strtod(void)
{
    float ret;
    int endp;

    ret = strtod("0.0");                        /* dec */
    TEST(477, ret == 0.0);

    ret = strtod("1.0");                        /* dec */
    TEST(478, ret == 1.0);

    ret = strtod("xxx", endp);                  /* invalid */
    TEST(479, ret == 0);
    TEST(480, endp == 0);

    ret = strtod("0.0");                        /* dec */
    TEST(481, ret == 0.0);

    ret = strtod("1.0");                        /* dec */
    TEST(482, ret == 1.0);

    ret = strtod("xxx", endp);                  /* invalid */
    TEST(483, ret == 0);
    TEST(484, endp == 0);
}


static void
test_defaults1(int value = 666)
{
    TEST(485, value == 666);
}


static void
test_defaults2(string value = "666")
{
    TEST(486, value == "666");
}


static void
test_defaults3(string value = 1.1)
{
    TEST(487, value == 1.1);
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
    TEST(488, ivalue == 2);
    test_reference2(svalue);
    TEST(489, svalue == "two");
    test_reference3(fvalue);
    TEST(490, fvalue == 2);
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
    TEST(491, length_of_list(l) == 1000);

    for (i = i-1; i >= 0; --i) {
        sprintf(msg, "work%04d", i);
        if (msg != pop(l)) {
            success = FALSE;
            break;
        }
    }
    TEST(492, success);

    TEST(493, length_of_list(l) == 0);
    pause_on_error(0, FALSE);
    r = pop(l);
    pause_on_error(1, FALSE);
    TEST(494, is_null(r));                      /* XXX - is_null(pop(l)) broken/limitation of LVAL's */
}


static void
test_globals(void)
{
    int line, col;

    TEST(495, inq_window() == current_window);
    TEST(496, inq_buffer() == current_buffer);
    inq_position(line, col);
    TEST(497, line == current_line);
    inq_position(line, col);
    TEST(498, col == current_col);
}


/*
 *  test_searh ---
 *      basic search primitives.
 */
static void
test_search(void)
{
    TEST(499, quote_regexp("<>") == "\\<\\>");
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
    TEST(500, "test_called" == caller());
    set_calling_name("");
    TEST(501, "" == caller());
    set_calling_name("test_called");
    TEST(502, "test_called" == caller());
    set_calling_name("hello_world");
    TEST(503, "hello_world" == caller());
    set_calling_name(inq_called());
    TEST(504, "regress" == caller());
    set_calling_name(NULL);                     /* extension, reset/clear */
    TEST(505, "test_called" == caller());
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
    TEST(506, ret == 1);

    ret = inq_macro("list_of_dictionaries");    /* builtin */
    TEST(507, ret == 0);

    ret = inq_macro("cut");                     /* replacement */
    TEST(508, ret == 2);

    ret = inq_macro("this_should_not_be_undefined");
    TEST(509, ret == -1);                       /* undefined */
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
            TEST(510, inq_symbol("undefined_ival") == 0);
            TEST(511, inq_symbol("undefined_fval") == 0);
            TEST(512, inq_symbol("undefined_sval") == 0);
            TEST(513, undefined_ival == 0);
            TEST(514, undefined_fval == 0);
            TEST(515, undefined_sval == "");
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
    TEST(516, top == "execute_macro" || top == "sel_list");
    TEST(517, inq_command() == inq_macro_history());
    set_macro_history(0, "function1");
    set_macro_history(1, "function2");
    TEST(518, "function1" == inq_macro_history());
    TEST(519, "function2" == inq_macro_history(1));
}


/*
 *  test_scope ---
 *      basic scope tests.
 */
static void
test_scope(void)
{
    if (first_time()) {                         /* can only be run once */
        TEST(520, 1 == scope_1());
        TEST(521, 2 == scope_1());
        TEST(522, 3 == scope_1());

        TEST(523, 1 == scope_2());
        TEST(524, 2 == scope_2());
        TEST(525, 3 == scope_2());

        TEST(526, 1 == scope_3());
        TEST(527, 2 == scope_3());
        TEST(528, 3 == scope_3());
    } else {                                    /* emulate */
        extern int num_passed;
        num_passed += 9;
    }

    TESTASYNC(529, 0 == inq_symbol("x_extern_dontexist"));
    TEST(530, 0 != inq_symbol("x_static"));
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
    TEST(531, dict);

    dict2 = create_dictionary("namedict");
    TEST(532, dict2);

    /* basic set/get primitives */
    set_property(dict, "property", 1234);
    var = get_property(dict, "property");
    TEST(533, 1234 == var);

    set_property(dict, "property", "hello world");
    var = get_property(dict, "property");
    TEST(534, "hello world" == var);

    /* indirection operators */
    dict.value = "value1";                      /* XXX - compiler issues yet resolved */
    TEST(535, "value1" == dict.value);

    dict2.value = "value2";
    dict.indirect = dict2;                      /* XXX - reference count issues */
    TEST(536, "value2" == dict.indirect.value);
    ret = dict_exists(dict, "property");
    TEST(537, ret);

    l1 = dict_list(dict);
    TEST(538, is_list(l1));
    TEST(539, 3 == length_of_list(l1));

    set_property(dict, "property2", 1234);
    set_property(dict, "property3", 5678);
    l1 = dict_list(dict);
    TEST(540, 5 == length_of_list(l1));

    // XXX --
    //  compiler bug, allows declare when string expected.
    //  language issue, can not pass a declare when a string is expected.
    //
    string key;                                 /* new language/compiler feature */
    declare value;
    int idx, count;

    while ((idx = dict_each(dict, key, value)) >= 0) {
        switch(key) {
        case "property":
            TESTASYNC(541, "hello world" == value);
            ++count;
            break;
        case "property2":
            TESTASYNC(542, 1234 == value);
            ++count;
            break;
        case "property3":
            TESTASYNC(543, 5678 == value);
            ++count;
            break;
        }
    }
    TESTASYNC(544, 3 == count);

    ret = dict_delete(dict, "property");
    TEST(545, 0 == ret);

    l1 = dict_list(dict);
    TEST(546, is_list(l1));
    TEST(547, 4 == length_of_list(l1));

    l1 = list_of_dictionaries();
    TEST(548, is_list(l1));
    TEST(549, length_of_list(l1) >= 2);         /* all dictionaries */

    ret = delete_dictionary(dict);
    TEST(550, 0 == ret);
    ret = delete_dictionary(dict2);
    TEST(551, 0 == ret);
}



/*
 *  test_leaks ---
 *      Tests to make sure we are not losing memory.
 */
static int
test_leaks(void)
{
    TEST(552, leak_1(1000) == 1000);            /* list are limited 2^16 atoms */
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
    int test = 0;

    inq_names(NULL, NULL, file, NULL);
    if ("regress.cr" == file || "regress2.cr" == file) {
        if ("regress2.cr" == file) {
            test = 999;
        }
        save_position();
        top_of_buffer();
        while (re_search(0, "TES[A-Z]+([0-9]+,") > 0) {
            /*
             *  examples:
             *      TEST(553,
             *      TESTASYNC(554,
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
        message("regress_renumber: total tests: %d", test);
    } else {
        error("regress_renumber: current source file must be 'regress.cr'");
    }
}
/*end*/
