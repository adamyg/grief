/* -*- mode: cr; indent-width: 4; -*- */
/* charset=utf8
 * $Id: regress2.cr,v 1.31 2025/06/29 12:54:25 cvsuser Exp $
 * Regression tests ... part2.
 *
 *
 */

#include "../grief.h"
#include "../debug.h"

#define TEST(__x, __cond) \
            if (!(__cond)) failed2(__x, #__cond); else passed2(__x);

#define TESTASYNC(__x, __cond) \
            {extern int x_lastnum; x_lastnum=0;} if (!(__cond)) failed2(__x, #__cond); else passed2(__x);

#if defined(__REGISTERS__)
#define REGISTER register                       /* 1/4/2020 */
#else
#pragma message("WARNING: registers not available ...")
#define REGISTER
#endif


#if defined(__PROTOTYPES__)
static void             passed2(int num);
static void             failed2(int num, string statement);

void                    regress2(void);
static void             test_cvt(void);
static void             test_line(void);
static void             test_anchor(void);
static void             test_echoline(void);
static void             test_split(void);
static void             test_split_arguments(void);
static void             test_tokenize(void);
static void             test_string2(void);
static void             test_regexp2(void);
static void             test_scanf(void);
static void             test_arg_list(void);
static void             test_arg_list1();
static void             test_arg_list2();
static string           test_arg_fn();
static void             test_getopt(void);
static void             test_getsubopt(void);
static void             test_parsename(void);
static void             test_command_list(void);
static void             test_file(void);
static void             test_splitpath(void);
static void             test_module(void);
static void             test_misc(void);
static void             test_try(void);
static void             test_display(void);
static void             test_ruler(void);
static void             test_datetime(void);
static void             test_sysinfo(void);
static void             test_ini(void);
static void		test_block(void);
static void             test_register_int(void);

static void             test_unicode_version(void);
static void             test_wcwidth(void);
static void             test_wstrlen(void);
static void             test_wstrnlen(void);
static void             test_wcharacterat(void);
static void             test_wstrstr(void);
static void             test_wstrrstr(void);
static void             test_wsubstr(void);
static void             test_wfirstof(void);
static void             test_wstrpbrk(void);
static void             test_wlastof(void);
static void             test_wstrcmp(void);
static void             test_wstrcasecmp(void);
static void             test_wlower(void);
static void             test_wupper(void);
static void             test_wread(void);
static void             test_wsprintf(void);
#endif //__PROTOTYPES__


void
main(void)
{
    module("regress");
}


void
__regress_divbyzero(void)
{
    __regress_op(9998);
}


void
__regress_nullwrite(void)
{
    __regress_op(9997);
}


#pragma warning(push)
#pragma warning(off)

void
__regress_arg_missing(void)
{
    __regress_op(/*missing*/);
}


void
__regress_arg_toomany(void)
{
   __regress_op(1,2,3);
}

#pragma warning(pop)



void
regress2(void)
{
    extern int x_lastnum;

    x_lastnum = 999;
    test_cvt();
    test_line();
    test_anchor();
    test_echoline();
    test_split();
    test_split_arguments();
    test_tokenize();
    test_string2();
    test_regexp2();
    test_scanf();
    test_arg_list();                            // 02/04/10
    test_getopt();                              // 02/04/10
    test_getsubopt();                           // 19/04/10
    test_parsename();
    test_command_list();
    test_file();
    test_splitpath();
    test_module();
    test_misc();
 // test_try();
 // test_exception();
    test_display();
    test_ruler();
    test_datetime();
    test_sysinfo();
    test_ini();
    test_register_int();

    // 06/2020
    test_unicode_version();
    test_wcwidth();
    test_wstrlen();
    test_wstrnlen();
    test_wcharacterat();
    test_wstrstr();
    test_wstrrstr();
    test_wsubstr();
    test_wfirstof();
    test_wstrpbrk();
    test_wlastof();
    test_wstrcmp();
    test_wstrcasecmp();
    test_wlower();
    test_wupper();
    test_wread();
    test_wsprintf();
}


static void
passed2(int num)
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
failed2(int num, string statement)
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


static void
test_cvt(void)
{
    declare d;

    d = cvt_to_object("0");                     // integer decimal
    TEST(1000, is_integer(d) && d == 0);

    d = cvt_to_object("+0");                    // integer decimal
    TEST(1001, is_integer(d) && d == 0);

    d = cvt_to_object("1");                     // integer decimal
    TEST(1002, is_integer(d) && d == 1);

    d = cvt_to_object("+1");                    // integer decimal
    TEST(1003, is_integer(d) && d == 1);

    d = cvt_to_object("-1");                    // integer decimal
    TEST(1004, is_integer(d) && d == -1);

    d = cvt_to_object("1234");                  // integer decimal
    TEST(1005, is_integer(d) && d == 1234);

    d = cvt_to_object("01234");                 // integer oct
    TEST(1006, is_integer(d) && d == 01234);

    d = cvt_to_object("0x1234");                // integer hex
    TEST(1007, is_integer(d) && d == 0x1234);

    d = cvt_to_object("0b010101");              // integer binary
    TEST(1008, is_integer(d));

    d = cvt_to_object(".123");                  // float
    TEST(1009, is_float(d) && d == .123);

    d = cvt_to_object("1.234");                 // float
    TEST(1010, is_float(d) && d == 1.234);

    d = cvt_to_object("1.234e12");              // float
    TEST(1011, is_float(d) && d == 1.234e12);

    d = cvt_to_object("1.7976931348623157e+308");  // Largest representable number without losing precision.
    TEST(1012, is_float(d) && d == 1.7976931348623157e+308);

    d = cvt_to_object("2.2250738585072014e-308");  // Smallest number without losing precision.
    TEST(1013, is_float(d) && d == 2.2250738585072014e-308);

    d = cvt_to_object("NaN");                   // float
    TEST(1014, is_float(d) && isnan(d));

    d = cvt_to_object("Inf");                   // float
    TEST(1015, is_float(d) && !isfinite(d));

    d = cvt_to_object("\"string\"");            // float
    TEST(1016, is_string(d));

    d = cvt_to_object("'string'");              // float
    TEST(1017, is_string(d));

    d = cvt_to_object("a");                     // float
    TEST(1018, is_null(d));
}


static void
test_line(void)
{
    //TODO
    //  hilite_create
    //  hilite_destroy
    //
    //  mark_line
    //  find_marker
    //  inq_line_flags
    //  set_line_flags
    //  find_line_flags
}


static void
test_anchor(void)
{
    //TODO
    //  inq_marked
    //  drop_anchor
    //  swap_anchor
    //  end_anchor
    //  raise_anchor
}


static void
test_echoline(void)
{
    string ofmt = inq_echo_format();
    string fmt2, fmt = "regress, cr %v: %b (%m%n) (%l %c) %t";
    int eflags;

    set_echo_format(fmt);                       // set
    redraw();
    sleep(2);

    eflags = inq_echo_line();
    fmt2 = inq_echo_format();
    TEST(1019, fmt == fmt2);
    TEST(1020, E_FORMAT & eflags);

    set_echo_format(NULL);                      // clear
    eflags = inq_echo_line();
    fmt2 = inq_echo_format();
    TEST(1021, "" == fmt2);
    TEST(1022, 0 == (E_FORMAT & eflags));

    set_echo_format(ofmt);                      // restore
}


static void
test_split(void)
{
    list l;

    /* basic */
    l = split("a|b|c", "|");
    TEST(1023, length_of_list(l) == 3);
    TEST(1024, l[0] == "a" && l[1] == "b" && l[2] == "c");

    l = split("a|b|", "|");
    TEST(1025, length_of_list(l) == 2);

    l = split("a||b", "|");
    TEST(1026, length_of_list(l) == 2);

    /* limit */
    l = split("a|b", "|", NULL, NULL, NULL, 1);
    TEST(1027, l[0] == "a|b");

    l = split("a|b", "|", NULL, NULL, NULL, 2);
    TEST(1028, l[0] == "a" && l[1] == "b");

    l = split("a|b|c", "|", NULL, NULL, NULL, 3);
    TEST(1029, l[0] == "a" && l[1] == "b" && l[2] == "c");

    l = split("a|b|c", "|", NULL, NULL, NULL, 2);
    TEST(1030, l[0] == "a" && l[1] == "b|c");

    /* numeric */
    l = split("a|16|b|32", "|", 1);
    TEST(1031, l[1] == 16 && l[3] == 32);

    /* numeric, enhanced */
    l = split("a|0x10|b|020|c|0b10000", "|", 2);
    TEST(1032, l[1] == 16 && l[3] == 16 && l[5] == 16);

    /* empties */
    l = split("a||b", "|", NULL, NULL, TRUE);
    TEST(1033, length_of_list(l) == 3);

    l = split("a|b|", "|", NULL, NULL, TRUE);
    TEST(1034, length_of_list(l) == 3);

    /* character mode */
    l = split("a|b|c", '|');
    TEST(1035, length_of_list(l) == 3);

    /* quoted */
    l = split("a,\" b \",c", ",\"");
    TEST(1036, length_of_list(l) == 3 && l[1] == " b ");

    l = split("a,\" b' \",' c\" '", ",\"'");
    TEST(1037, length_of_list(l) == 3 && l[1] == " b' " && l[2] == " c\" ");
}


/*
 *  test_split_arguments -
 *      split_arguments primitive test.
 */
static void
test_split_arguments(void)
{
    list l;

    l = split_arguments("aaa");
    TEST(1038, 1 == length_of_list(l));

    l = split_arguments("aaa bbb ccc ddd eee");
    TEST(1039, 5 == length_of_list(l));

    l = split_arguments("\"aaa bbb\" \"ccc ddd\" eee");
    TEST(1040, 3 == length_of_list(l));

    l = split_arguments("");
    TEST(1041, 0 == length_of_list(l));
}


/*
 *  test_tokenize ---
 *      basic tokenize functionality.
 */
static void
test_tokenize(void)
{
    list l;

    l = tokenize("hello world", "");
    TEST(1042, length_of_list(l) == 0);

    l = tokenize("hello world", " ");
    TEST(1043, length_of_list(l) == 2 && "hello" == l[0] && "world" == l[1]);

    l = tokenize("hello world", ' ');
    TEST(1044, length_of_list(l) == 2 && "hello" == l[0] && "world" == l[1]);

    l = tokenize("' hello ' ' world '", ' ', TOK_SINGLE_QUOTES);
    TEST(1045, length_of_list(l) == 2 && " hello " == l[0] && " world " == l[1]);

    l = tokenize("\" hello \" \" world \"", ' ', TOK_DOUBLE_QUOTES);
    TEST(1046, length_of_list(l) == 2 && " hello " == l[0] && " world " == l[1]);
}


/*
 *  test_string2 ---
 *      additional string primitives.
 */
static void
test_string2(void)
{
    int result, ret;

    //firstof
    ret = firstof("abcedf", "", result);
    TEST(1047, 0 == ret && 0 == result);

    ret = firstof("abcedf", "1234", result);
    TEST(1048, 0 == ret && 0 == result);

    ret = firstof("abcedf", "12ab", result);
    TEST(1049, 1 == ret && 'a' == result);

    ret = firstof("abcedf", "1f2", result);
    TEST(1050, 6 == ret && 'f' == result);

    //lastof
    ret = lastof("abcedf", "", result);
    TEST(1051, 0 == ret && 0 == result);

    ret = lastof("abcedf", "1234", result);
    TEST(1052, 0 == ret && 0 == result);

    ret = lastof("abcedf", "1abf", result);
    TEST(1053, 6 == ret && 'f' == result);

    ret = lastof("abcedf", "1a2", result);
    TEST(1054, 1 == ret && 'a' == result);
}


/*
 *  test_regexp2 ---
 *      additional regexp primitives.
 */
static void
test_regexp2(void)
{
    string s;

    TEST(1055, 6 == re_search(SF_BRIEF|SF_MAXIMAL|SF_IGNORE_CASE|SF_CAPTURES, "{[0-9]+}", "Hello1234World"));
    TEST(1056, re_result(1, s) == 4 && s == "1234");
    TEST(1057, re_result(2, s) == -1);

    TEST(1058, re_search(SF_BRIEF|SF_MAXIMAL|SF_CAPTURES, "{[A-Za-z]+} {[A-Za-z]+}", "Hello World") > 0);
    TEST(1059, re_result(1, s) == 5 && s == "Hello");
    TEST(1060, re_result(2, s) == 5 && s == "World");
    TEST(1061, re_result(3, s) == -1);
}


/*
 *  test_scanf ---
 *      scanf primitive.
 */
static void
test_scanf(void)
{
    string s1, s2;
    int r, i1, i2, i3;

    /*strings: %s*/
    r = sscanf("hello1 world1", "%s %s", s1, s2);
    TEST(1062, r == 2 && s1 == "hello1" && s2 == "world1");

    r = sscanf("hello2world2", "%6s%6s", s1, s2);
    TEST(1063, r == 2 && s1 == "hello2" && s2 == "world2");

    r = sscanf("aaAhello", "%[a]", s1);
    TEST(1064, r == 1 && s1 == "aa");

    r = sscanf("hello world", "%[a-z]", s1);
    TEST(1065, r == 1 && s1 == "hello");

    r = sscanf("\t hello", "%[^a-z]", s1);
    TEST(1066, r == 1 && s1 == "\t ");

    r = sscanf("hello-world", "%[^-]", s1);
    TEST(1067, r == 1 && s1 == "hello");

    r = sscanf("hello]", "%[^]]", s1);
    TEST(1068, r == 1 && s1 == "hello");

    r = sscanf("aa-hello", "%[a-]", s1);
    TEST(1069, r == 1 && s1 == "aa-");

    r = sscanf("abchello", "%[a-bb-c]", s1);
    TEST(1070, r == 1 && s1 == "abc");

    /* character: %c */
    r = sscanf("hello", "%c%c", i1, i2);
    TEST(1071, r == 2 && i1 == 'h' && i2 == 'e');

    /* numeric: %d,%u,%i,%x */
    r = sscanf("1234 -5678 +9010", "%d%d%d", i1, i2, i3);
    TEST(1072, r == 3 && 1234 == i1 && -5678 == i2 && 9010 == i3);

    r = sscanf("56781234", "%4d%4d", i1, i2);
    TEST(1073, r == 2 && 1234 == i2 && i1 == 5678);

    r = sscanf("5678 1234 ", "%u%u", i1, i2);
    TEST(1074, r == 2 && 5678 == i1 && 1234 == i2);

    r = sscanf("1234 -5678 +9010", "%i%i%i", i1, i2, i3);
    TEST(1075, r == 3 && 1234 == i1 && -5678 == i2 && 9010 == i3);

    r = sscanf("1010 101010", "%b%b", i1, i2);     /*binary*/
    TEST(1076, r == 2 && 0b1010 == i1 && 0b101010 == i2);

    r = sscanf("01010 0101010", "%b%b", i1, i2);   /*binary; leading zeros*/
    TEST(1077, r == 2 && 0b01010 == i1 && 0b101010 == i2);

    r = sscanf("0b01010 0b101010", "%b%b", i1, i2); /*binary; prefix*/
    TEST(1078, r == 2 && 0b01010 == i1 && 0b101010 == i2);

    r = sscanf("01234 0567", "%o%o", i1, i2);      /*oct*/
    TEST(1079, r == 2 && 01234 == i1 && 0567 == i2);

    r = sscanf("01 009", "%d%d", i1, i2);          /*dec: leading zeros*/
    TEST(1080, r == 2 && 1 == i1 && 9 == i2);

    r = sscanf("0xABCD 0xFEA1", "%x%x", i1, i2);   /*hex: prefix*/
    TEST(1081, r == 2 && 0xABCD == i1 && 0xFEA1 == i2);

    r = sscanf("FEDB ABCD", "%x%x", i1, i2);       /*hex: upper*/
    TEST(1082, r == 2 && 0xFEDB == i1 && 0xABCD == i2);

    r = sscanf("fedb abcd", "%x%x", i1, i2);       /*hex; lower*/
    TEST(1083, r == 2 && 0xfedb == i1 && 0xabcd == i2);

    r = sscanf("0D 0BAD", "%x%x", i1, i2);         /*hex: leading zeros*/
    TEST(1084, r == 2 && 0xD == i1 && 0xBAD == i2);

    /*  classes
     *
     *      "ascii"     ASCII character.
     *      "alnum"     An alphanumeric (letter or digit).
     *      "alpha"     A letter.
     *      "blank"     A space or tab character.
     *      "cntrl"     A control character.
     *      "csym"      A langiage symbol.
     *      "digit"     A decimal digit.
     *      "graph"     A character with a visible representation.
     *      "lower"     A lower-case letter.
     *      "print"     An alphanumeric (same as alnum).
     *      "punct"     A punctuation character.
     *      "space"     A character producing white space in displayed text.
     *      "upper"     An upper-case letter.
     *      "word"      A "word" character (alphanumeric plus "_").
     *      "xdigit"    A hexadecimal digit.
     */
    r = sscanf(" Aabcyz123zZ0 -=()_", " %[[:ascii:]]", s1);
    TEST(1085, r == 1 && s1 == "Aabcyz123zZ0 -=()_");

    r = sscanf(" Aabcyz123zZ0 -=()_", " %[[:alnum:]]", s1);
    TEST(1086, r == 1 && s1 == "Aabcyz123zZ0");

    r = sscanf(" Aabcyz123zZ0 -=()_", " %[[:alpha:]]", s1);
    TEST(1087, r == 1 && s1 == "Aabcyz");

    r = sscanf("\t \nAabcyz123zZ0 -=()_", "%[[:blank:]]", s1);
    TEST(1088, r == 1 && s1 == "\t ");

    r = sscanf(" \a\b\f\v\e", " %[[:cntrl:]]", s1);
    TEST(1089, r == 1 && s1 == "\a\b\f\v\e");

    r = sscanf(" Aabcyz123_zZ0 -=()_", " %[[:csym:]]", s1);
    TEST(1090, r == 1 && s1 == "Aabcyz123_zZ0");

    r = sscanf(" 9876abcyz ", " %[[:digit:]]", s1);
    TEST(1091, r == 1 && s1 == "9876");

    r = sscanf(" abcyzABCYZ987 ", " %[[:lower:]]", s1);
    TEST(1092, r == 1 && s1 == "abcyz");

    r = sscanf(" 9876abcyz ", " %[[:print:]]", s1);
    TEST(1093, r == 1 && s1 == "9876abcyz ");

    r = sscanf(" \t9876abcyz ", "%[[:space:]]", s1);
    TEST(1094, r == 1 && s1 == " \t");

    r = sscanf(" ABCYZabcyz9876abcyz ", " %[[:upper:]]", s1);
    TEST(1095, r == 1 && s1 == "ABCYZ")

    r = sscanf(" 9876_abcyzABCYZ ", " %[[:word:]]", s1);
    TEST(1096, r == 1 && s1 == "9876_abcyzABCYZ");

    r = sscanf(" 9876abcfABCYZ ", " %[[:xdigit:]]", s1);
    TEST(1097, r == 1 && s1 == "9876abcfABC");

    /*%v*/

    /*%V*/

    /*%Q*/

    /*bad*/
//TODO
//  sscanf("hello world", "%[A-A]", s1);
//  sscanf("hello world", "%[[:digi:]]", s1);
//  sscanf("hello world", "%[[:digi:]", s1);
//  sscanf("hello world", "%[[:digi]]", s1);
//  sscanf("hello world", "%[");
//  sscanf("hello world", "%[a-");
//  sscanf("hello world", "%[b-a]");
//  sscanf("hello world", "%[abc-d-e]");
}


/*
 *  test_arg_list --
 *      arg_list() primitive test.
 */
static void
test_arg_list(void)
{
    int iarg = 1;
    float farg = 2;
    string sarg = "3";
    list larg = {4, 4.1};
    declare darg = 5;
    REGISTER int rarg = 6;

    test_arg_list1(iarg, farg, sarg, larg, darg, atoi("5"), rarg, test_arg_fn(1,"2",larg,rarg));
    test_arg_list2(iarg, farg, sarg, larg, darg, atoi("5"), rarg, test_arg_fn(1,"2",larg,rarg));
}


static void
test_arg_list1()        // (int iarg, float farg, string sarg, list larg, declare xarg1, declare xarg2, string fn1)
{
    list l;
    string str;

    l = arg_list(0);                            // not eval'ed
    TEST(1098, 8 == length_of_list(l));
    TEST(1099, l[0] == "iarg" && l[3] == "larg");
    TEST(1100, l[5][0] == "atoi");              // (atoi "5")
    TEST(1101, l[6] == "rarg");                 // register
    TEST(1102, l[7][0] == "test_arg_fn");       // (test_arg_fn 1 "2" larg rarg)
    TEST(1103, l[7][1] == 1);
    TEST(1104, l[7][2] == "2");
    TEST(1105, l[7][3] == "larg");
    TEST(1106, l[7][4] == "rarg");

    l = arg_list(0, 1, 3);                      // not eval'ed, limited selection
    TEST(1107, 3 == length_of_list(l));
    TEST(1108, l[0] == "farg" && l[2] == "larg");

    sprintf(str, "%s", l);                      // sprintf list support
    TEST(1109, strlen(str) >= 5*2);
}


static void
test_arg_list2()        // (int iarg, float farg, string sarg, list larg, declare xarg1, declare xarg2, string fn1)
{
    list l;

    l = arg_list(1);                            // eval'ed
    TEST(1110, 8 == length_of_list(l));
    TEST(1111, l[0] == 1 && l[2] == "3");
    TEST(1112, l[5] == "5");                    // atoi()
    TEST(1113, l[6] == 6);                      // register
    TEST(1114, l[7] == "7");                    // test_arg_fn()

    l = arg_list(1, 1, 3);                      // eval'ed, limited selection
    TEST(1115, 3 == length_of_list(l));
    TEST(1116, l[0] == 2 && l[1] == "3");
}


static string
test_arg_fn()
{
    return "7";
}


/*
 *  test_getopt ---
 *      getopt() primitive list.
 */
static void
test_getopt(void)
{
    string arguments =
        "-h --help "+
            " --option2 --option3" +
            " --integer=1234 " +
            " --string \"56 78\" " +
            " --bool=yes --bool=y --bool=true --bool=on --bool=1" +
                " --bool=no --bool=n --bool=false --bool=off --bool=0" +
            " 910 11 \"22 22\"";

    list longoptions = {
            "help,h",                           // note, dupicates result in first match
            "option2,#2",
            "option3",                          // index follows previous, hence 3
            "integer,i:integer",
            "string,s:string",
            "boolean,b:b"
            };

    int booltest = 0;
    int match, ch;
    string value;

    if ((ch = getopt(value, "h", longoptions, arguments, "regress")) >= 0) {
        do {
            ++match;
            switch(ch) {
            case 'h':           // -h or --help option
                if (1 == match) {
                    TEST(1117, 1 == match & value == "");
                } else {
                    TEST(1118, 2 == match && value == "");
                }
                break;

            case 2:             // --option2
                TEST(1119, 3 == match && value == "");
                break;

            case 3:             // --option3
                TEST(1120, 4 == match && value == "");
                break;

            case 'i':           // --integer=<value>
                TEST(1121, 5 == match && value == "1234");
                break;

            case 's':           // --string=<value>
                TEST(1122, 6 == match && value == "56 78");
                break;

            case 'b':           // --bool=<value>
                switch (++booltest) {
                case 1:  TEST(1123, value == "1"); break;
                case 2:  TEST(1124, value == "1"); break;
                case 3:  TEST(1125, value == "1"); break;
                case 4:  TEST(1126, value == "1"); break;
                case 5:  TEST(1127, value == "1"); break;

                case 6:  TEST(1128, value == "0"); break;
                case 7:  TEST(1129, value == "0"); break;
                case 8:  TEST(1130, value == "0"); break;
                case 9:  TEST(1131, value == "0"); break;
                case 10: TEST(1132, value == "0"); break;
                }
                break;

            case '?':           // error or unknown option
            case ':':           // missing parameter
            default:
                if (strlen(value)) {
                    error("%s", value);
                }
                match = 99;
                break;
            }
        } while ((ch = getopt(value)) >= 0);
    }

    TEST(1133, match == 16);
    TEST(1134, value == "910 11 \"22 22\"");
    TEST(1135, -1 == getopt(value));
}


/*
 *  test_getsubopt ---
 *      getsubopt() primitive list.
 */
static void
test_getsubopt(void)
{
    const list options = {
                "help,h",
                "option2,#2",
                "option3",
                "integer,i:integer",
                "string,s:string"
                };
    string arguments = "help,option2,option3,integer=1234,string=\"56 78\"";
    int match;

    string value;
    int ch;

    if ((ch = getsubopt(value, options, arguments)) > 0) {
        do {
            ++match;
            switch (ch) {
            case 'h':           // 'help' option
                TEST(1136, 1 == match && value == "");
                break;

            case 2:             // 'option2'
                TEST(1137, 2 == match && value == "");
                break;

            case 3:             // 'option3'
                TEST(1138, 3 == match && value == "");
                break;

            case 'i':           // 'integer=<value>'
                TEST(1139, 4 == match && value == "1234");
                break;

            case 's':           // 'string=<value>'
                TEST(1140, 5 == match && value == "56 78");
                break;

            default:
               error("unexpected return: %d (%s)", ch, value);
               match = 99;
               break;
            }
        } while ((ch = getsubopt(value)) > 0);

        if (ch < -1 && 99 != match) {
            if (strlen(value)) {
               error("myfunction: %s", value);
               match = 99;
            }
        }
    }

    TEST(1141, 5 == match);
    TEST(1142, -1 == getsubopt(value));
    TEST(1143, "" == value);
}


/*
 *  test_parsename
 *      parse_name functionality.
 *
 *  Usage:
 *      int
 *      parse_filename(string fullname,
 *          [string &drive], [string &path], [string &filename], [string &ext])
 */
static void
test_parsename(void)
{
    string drive, path, fname, ext;

    parse_filename("c://subdir\\subdir\\fname.ext", drive, path, fname, ext);
    TEST(1144, 0 == strcmp(drive, "c:"));
    TEST(1145, 0 == strcmp(path,  "//subdir\\subdir\\"));
    TEST(1146, 0 == strcmp(fname, "fname"));
    TEST(1147, 0 == strcmp(ext,   "ext"));

    parse_filename("z:fname.ext1.ext2", drive, path, fname, ext);
    TEST(1148, 0 == strcmp(drive, "z:"));
    TEST(1149, 0 == strcmp(path,  ""));
    TEST(1150, 0 == strcmp(fname, "fname.ext1"));
    TEST(1151, 0 == strcmp(ext,   "ext2"));     // last extension
}


/*
 *  test_command_list
 *      command/macro_list
 */
static void
test_command_list(void)
{
    list cmd = command_list(TRUE /*builtins-only*/, "*float");
    TEST(1152, 3 == length_of_list(cmd));       // float, is_float and cast_float

    list macros = macro_list("regress");
    TEST(1153, 1 == length_of_list(macros));    // regress()
}


/*
 *  test_file ---
 *      file functionality.
 */
static void
test_file(void)
{
    //  mkdir/rmdir
    TEST(1154, 0 != access("./grregress"));
    TEST(1155, 0 == mkdir("./grregress", 0666));
    TEST(1156, 0 == access("./grregress"));
    TEST(1157, 0 == rmdir("./grregress"));
    TEST(1158, 0 != access("./grregress"));

    //  mktemp
    //  rename
    //  remove
    //  stat
    int now = time()-1;
    string base = format("%s/gr%d-%d-B-XXXXXX", inq_tmpdir(), getpid(), now);
    string temp = mktemp(base),
       temp2 = temp + "2";
    int size = -1, mtime, ctime, atime, mode;

    TEST(1159, 0 == access(temp));

    TEST(1160, 0 == stat(temp, size, mtime, ctime, atime, mode));
      TEST(1161, 0 == size);
      TEST(1162, mtime >= now && ctime >= now && atime >= now);
      TEST(1163, (S_IFMT   & mode) == S_IFREG);
      TEST(1164, (S_IFMT   & mode) != S_IFCHR);
      TEST(1165, (S_IFMT   & mode) != S_IFDIR);
      TEST(1166, (S_IFMT   & mode) != S_IFIFO);
      TEST(1167, (S_IFMT   & mode) != S_IFLNK);
      TEST(1168, (S_IFMT   & mode) != S_IFSOCK);
      TEST(1169, (S_IFMT   & mode) != S_ISVTX || S_ISVTX == 0);
      TEST(1170, (S_IRUSR  & mode) == S_IRUSR);
      TEST(1171, (S_IWUSR  & mode) == S_IWUSR);
      TEST(1172, (S_IXGRP  & mode) == 0);
      TEST(1173, (S_IXOTH  & mode) == 0);
      TEST(1174, (S_IXUSR  & mode) == 0);

    TEST(1175, 0 == rename(temp, temp2));
    TEST(1176, 0 == access(temp2));
    TEST(1177, 0 == remove(temp2));
    TEST(1178, 0 != access(temp2));
    TEST(1179, 0 != stat(temp2));

//TODO
    //  lstat
    //  ftest

    //  mode_string
    //  inq_tmpdir
    //  realpath
    //  symlink
    //  umask

    //  fopen
    //  fread
    //  fwrite
    //  ftruncate
    //  ftell
    //  fseek
    //      SEEK_SET, SEEK_CUR, SET_END
    //  fclose
}


/*
 *  test_splitpath ---
 *      splitpath functionality.
 */
static void
test_splitpath(void)
{
    string dir, name, ext, drive;

    splitpath("x/y", dir, name, ext, drive);
    TEST(1180, dir == "x/" && name == "y" && ext == "" && drive == "");

    splitpath("x/", dir, name, ext, drive);
    TEST(1181, dir == "x/" && name == "" && ext == "" && drive == "");

    splitpath("/x", dir, name, ext, drive);
    TEST(1182, dir == "/" && name == "x" && ext == "" && drive == "");

    splitpath("x", dir, name, ext, drive);
    TEST(1183, dir == "" && name == "x" && ext == "" && drive == "");

    splitpath("", dir, name, ext, drive);
    TEST(1184, dir == "" && name == "" && ext == "" && drive == "");

    splitpath(".x", dir, name, ext, drive);
    TEST(1185, dir == "" && name == "" && ext == ".x" && drive == "");

    splitpath("a.b.c", dir, name, ext, drive);
    TEST(1186, dir == "" && name == "a.b" && ext == ".c" && drive == "");

    splitpath(":x", dir, name, ext, drive);
    TEST(1187, dir == "" && name == ":x" && ext == "" && drive == "");

    splitpath("a:x", dir, name, ext, drive);
    TEST(1188, dir == "" && name == "x" && ext == "" && drive == "a:");

    splitpath("a.b:x", dir, name, ext, drive);
    TEST(1189, dir == "" && name == "a" && ext == ".b:x" && drive == "");

    splitpath("C:/dos/command.com", dir, name, ext, drive);
    TEST(1190, dir == "/dos/" && name  == "command" && ext == ".com" && drive == "C:");
}


/*
 *  test_module ---
 *      module functionality.
 */
static void
test_module(void)
{
    TEST(1191, inq_module() == "regress");      // current association
    TEST(1192, inq_macro("test_module", 2) == inq_macro("test_misc", 2));
    TEST(1193, inq_macro("test_module", 2) != inq_macro("crisp", 2));
    TEST(1194,  2 == module("regress"));        // pre-existing association
    TEST(1195, -1 == module("newmodule"));      // re-assignment, error
}


/*
 *  test_misc ---
 *      miscellanous functionality.
 */
static void
test_misc(void)
{
    //  version
    int vmajor, vminor, vedit;
    string vmachtype, vcompiled;

    TEST(1196, version(vmajor, vminor, vedit, NULL, vmachtype, vcompiled) >= 302 && \
            vmajor >= 3 && vminor >= 0 && vedit >= 0);
    switch (vmachtype) {
    case "VMS":
    case "OS/2":
    case "Mingw32":
    case "Win32":
    case "DOS":
    case "MACOSX":
    case "UNIX":
        vmachtype = "";
        break;
    }
    TEST(1197, "" == vmachtype);                // defined

    //  srand()/rand()
    const int seed = 1234;

    srand(seed); int rand1 = rand(); srand(seed);
    TEST(1198, rand1 == rand());

    //  strerror - need EINVAL etc
    TEST(1199, "Success" == strerror(0));
    TEST(1200, "Unknown error" == strerror(-1));
}


static void
test_try(void)
{
#if (TODO)
    string ret;

    ret = "try1";
    try {
        int buffer;
        buffer = inq_buffer();
    } catch(err) {
        ret = err;
    }
    TXEST(1, "try1" == ret);

    ret = "try2";
    try {
        throw("throw2");
    } catch(err) {
        ret = err;
    } finally {
        ret = "finally2";
    }
    TXEST(1, "throw2" == ret);

    ret = "try3";
    try {
    } catch(err) {
        ret = err;
    } finally {
        ret = "finally3";
    }
    TXEST(1, "throw3" == ret);
#endif
}


/*
 *  test_display ---
 *      basic display interface.
 */
static void
test_display(void)
{
    TEST(1201, inq_display_mode("scroll_cols")  == display_mode(NULL, NULL, -1));
    TEST(1202, inq_display_mode("scroll_rows")  == display_mode(NULL, NULL, NULL, -1));
    TEST(1203, inq_display_mode("visible_cols") == display_mode(NULL, NULL, NULL, NULL, -1));
    TEST(1204, inq_display_mode("visible_rows") == display_mode(NULL, NULL, NULL, NULL, NULL, -1));
}


/*
 *  test_ruler ---
 *      ruler, ident and tabs tests.
 */
static void
test_ruler(void)
{
    string saved_tabs = inq_tabs();
    int saved_indent = inq_indent();
    list ruler = { 5, 9, 20, 25 };
    list r;

    /* tabs */
    tabs(8, 17);
    TEST(1205, inq_tabs() == "8 17");

    tabs("9 17");
    TEST(1206, inq_tabs() == "9 17");
    TEST(1207, 8 == inq_tab());                  /* default */

    tabs("");
    TEST(1208, inq_tabs() == "");

    execute_macro("tabs " + saved_tabs);
    TEST(1209, inq_tabs() == saved_tabs);

    /* indent */
    set_indent(5);
    TEST(1210, inq_indent() == 5);               /* indent builds simple ruler */

    set_indent(0);
    TEST(1211, inq_indent() == 0);

    set_indent(saved_indent);
    TEST(1212, inq_indent() == saved_indent);

    /* ruler */
    set_ruler(NULL, ruler);
    TEST(1213, inq_ruler(NULL, 5) == "5 9 20 25 30");
    r = inq_ruler(NULL, 10, TRUE);
    TEST(1214, r[0] == 5 && r[1] == 9 && r[4] == 30);
    set_ruler(NULL, NULL);                      /* clear */
}


/*
 *  test_datetime ---
 *      date and time functionality.
 */
static void
test_datetime(void)
{
    //  time/sleep
    int curtime = time();

    sleep(1);
    TEST(1215, time() > curtime);
    sleep(1, 10);
    TEST(1216, time() > ++curtime);
    sleep(1, 10);
    TEST(1217, time() > ++curtime);

    //  inq_idle_time/sleep
    TEST(1218, inq_idle_time() >= 1);
    sleep(1);
    TEST(1219, inq_idle_time()  > 1);

    //  inq_clock/sleep(ms)
    int clock1 = inq_clock();
    for (curtime = time(); curtime == time();)
        { }
    TEST(1220, inq_clock() > clock1);

    //  localtime/date
    int year1, mon1, mday1, hour1, min1, sec1;
      string monname1, dayname1;
    int year2, mon2, mday2, hour2, min2, sec2;
      string monname2, dayname2;
    int year3, mon3, mday3;
      string monname3, dayname3;

    localtime(curtime, year1, mon1, mday1, monname1, dayname1, hour1, min1, sec1);
    gmtime(curtime, year2, mon2, mday2, monname2, dayname2, hour2, min2, sec2);
    date(year3, mon3, mday3, monname3, dayname3);

    TEST(1221, year1 >= 2020 && mon1 >= 1 && mon1 <= 12 && mday1 >= 1 && mday1 <= 31);
    TEST(1222, year2 >= 2020 && mon2 >= 1 && mon2 <= 12 && mday2 >= 1 && mday2 <= 31);
    TEST(1223, year3 >= 2020 && mon3 >= 1 && mon3 <= 12 && mday3 >= 1 && mday3 <= 31);
    TEST(1224, year1 == year2 && year2 == year3);
    TEST(1225, mon1  == mon2  && mon2  == mon3);
  //TEST(XXXX, mday1 == mday2 && mday2 == mday3);

    //TODO
    //  cftime
}


/*
 *  test_sysinfo ---
 *     System information.
 */
static void
test_sysinfo(void)
{
    string homedir = inq_home(),
        profiledir = inq_profile(),
        tmpdir = inq_tmpdir();

    TEST(1226, 0 == access(homedir));
    TEST(1227, strlen(profiledir));
    TEST(1228, 0 == access(tmpdir));

    string username = inq_username(),
        hostname = inq_hostname();

    TEST(1229, username);
    TEST(1230, hostname);

    string usysname, unodename, uversion, urelease, umachine;

    TEST(1231, 0 == uname(usysname, unodename, uversion, urelease, umachine) && \
            strlen(usysname) && strlen(unodename) && strlen(uversion) && strlen(urelease) && strlen(umachine));
}


/*
 *  test_ini ---
 *      INI parser.
 */
static void
test_ini(void)
{
    const string section = "Section X", inifile = "regress_testini.cfg";
    string sect, key, value;
    int fd, ret, cnt;

    //////////////////////////////////////////////////////////////
    //  export
    //
    fd = iniopen();
    TEST(1232, fd >= 0);

    inipush(fd, section, "zzz", "9999", "comment 1");
    inipush(fd, section, "zzz", "8888", "comment 2");
    inipush(fd, section, "yyy", "7777", "comment 3", TRUE);
    inipush(fd, section, "yyy", "6666", "comment 4", TRUE);
    inipush(fd, section, "xxx", "5555");
    iniremove(fd, section, "yyy");

    for (cnt = 0, ret = inifirst(fd, sect, key, value); 1 == ret;
            ret = ininext(fd, sect, key, value)) {
        switch (++cnt) {
        case 1:
            TEST(1233, sect == section);
            TEST(1234, key == "zzz" && value == "8888");
            break;
        case 2:
            TEST(1235, sect == section);
            TEST(1236, key == "xxx" && value == "5555");
            break;
        }
    }
    TEST(1237, 2 == cnt);

    iniexport(fd, inifile);
    iniclose(fd);

    //////////////////////////////////////////////////////////////
    //  import
    //
    int fd2 = iniopen(inifile, IFILE_STANDARD|IFILE_STANDARDEOL);

    TEST(1238, fd2 >= 0);
    TEST(1239, fd2 != fd);

    for (cnt = 0, ret = inifirst(fd2, sect, key, value); 1 == ret;
            ret = ininext(fd2, sect, key, value)) {
        switch (++cnt) {
        case 1:
            TEST(1240, sect == section);
            TEST(1241, key == "zzz" && value == "8888");
            break;
        case 2:
            TEST(1242, sect == section);
            TEST(1243, key == "xxx" && value == "5555");
            break;
        }
    }
    TEST(1244, 2 == cnt);

    iniclose(fd2);
    remove(inifile);
}


/*
 *  test_register_int ---
 *      Register variables.
 */

#if defined(__REGISTERS__)
#pragma warning(push, off)                      /* hidden variable warnings */
#pragma scope(push, lexical)                    /* enable block scoping */

#else
#pragma message("WARNING: registers not available ...")
#define REGISTER
#endif

static void
test_block0(~ int)
{
    extern int x;
    int y;

    get_parm(1, y);
    TEST(1245, x == 42);
    TEST(1246, y == 42);
    {
        int x = 1;
        TEST(1247, x == 1);
        {
            int x = 2;
            TEST(1248, x == 2);
            {
                int x = 3;
                TEST(1249, x == 3);
                {
                    int x = 4;
                    TEST(1250, x == 4);
                }
                TEST(1251, x == 3);
            }
            TEST(1252, x == 2);
       }
       TEST(1253, x == 1);
    }
    TEST(1254, x == 42);
}


static void
test_block(void)
{
    int x = 42;
    scope_block0(x);
}


static int
test_register_int2(int level = 0)
{
    register int i1 = 7890;
    if (level < 10) {
        i1 = test_register_int2(level + 1);
    }
    return i1;
}


static void
test_register_int(void)
{
    REGISTER int i1 = 1234;

    TEST(1255, 1234 == i1);

    i1 += 10;
    TEST(1256, 1244 == i1);

    i1 -= 10;
    TEST(1257, 1234 == i1);

    i1 += i1;
    TEST(1258, 2468 == i1);

    i1  = i1 - 1234;
    TEST(1259, 1234 == i1);

    { /*nesting*/
        REGISTER int i2 = 4567;                 /* additional */

        TEST(1260, 1234 == i1);
        TEST(1261, 4567 == i2);

        { /*nesting*/
            REGISTER int i1 = 8901;             /* hidden */

            TEST(1262, 8901 == i1);
            TEST(1263, 4567 == i2);

            {
                REGISTER int i1 = 101234;       /* hidden */
                TEST(1264, 101234 == i1);
            }

            TEST(1265, 8901 == i1);
            TEST(1266, 4567 == i2);
        }

        TEST(1267, 1234 == i1);
    }
    TEST(1268, 1234 == i1);

    TEST(1269, 7890 == test_register_int2());   /* nested calls */
    TEST(1270, 1234 == i1);
}

#if defined(__REGISTERS__)
#pragma scope(pop)
#pragma warning(pop)
#endif

static void
test_unicode_version(void)
{
    string uv = inq_unicode_version();
    TEST(1271, set_unicode_version("11.0.0") == 110000); //match
    TEST(1272, set_unicode_version("13.0.0") == 130000); //match
    TEST(1273, set_unicode_version("19.0.0") == 150100); //closest (15.1.0, current upper)
    set_unicode_version(uv);
}


static void
test_wcwidth(void)
{
    TEST(1274, wcwidth("") == 0);
    TEST(1275, wcwidth("a") == 1);
    TEST(1276, wcwidth("The quick brown fox jumps over the lazy dog") == 43);
    TEST(1277, wcwidth("ξεσκεπάζω την ψυχοφθόρα βδελυγμία") == 33);
}


static void
test_wstrlen(void)
{
    TEST(1278, wstrlen("") == 0);
    TEST(1279, wstrlen("a") == 1);
    TEST(1280, wstrlen("The quick brown fox jumps over the lazy dog") == 43);
    TEST(1281, wstrlen("ξεσκεπάζω την ψυχοφθόρα βδελυγμία") == 33);
}


static void
test_wstrnlen(void)
{
    TEST(1282, wstrnlen("", 0) == 0);
    TEST(1283, wstrnlen("a", 1) == 1);
    TEST(1284, wstrnlen("The quick brown fox jumps over the lazy dog", 30) == 30);
    TEST(1285, wstrnlen("ξεσκεπάζω την ψυχοφθόρα βδελυγμία", 34) == 33);
}


static void
test_wcharacterat(void)
{
    TEST(1286, wcharacterat("АБВГДЕЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯ", 1)  == L'А')
    TEST(1287, wcharacterat("АБВГДЕЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯ", 7)  == L'Ж')
    TEST(1288, wcharacterat("АБВГДЕЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯ", 33) == -1)
}


static void
test_wstrstr(void)
{
    TEST(1289, 4 == wstrstr("abcmandefg", "man"));
    TEST(1290, 4 == wstrstr("abcmanmandefg", "man"));
    TEST(1291, 0 == wstrstr("abcmanmandefg", "ban"));
    TEST(1292, 1 == wstrstr("abcmanmandefg", ""));

    TEST(1293, wstrstr("АБВГДЕЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯАБВГДЕЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯ", "БВ") == 2)
    TEST(1294, wstrstr("АБВГДЕЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯАБВГДЕЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯ", "A")  == 0)
    TEST(1295, wstrstr("АБВГДЕЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯАБВГДЕЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯ", "")   == 1)
}


static void
test_wstrrstr(void)
{
    TEST(1296, 4 == wstrrstr("abcmandefg", "man"));
    TEST(1297, 7 == wstrrstr("abcmanmandefg", "man"));
    TEST(1298, 0 == wstrrstr("abcmanmandefg", "ban"));
    TEST(1299, 0 == wstrrstr("abcmanmandefg", ""));

    TEST(1300, wstrrstr("АБВГДЕЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯАБВГДЕЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯ", "БВ") == 34)
    TEST(1301, wstrrstr("АБВГДЕЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯАБВГДЕЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯ", "A")  == 0)
    TEST(1302, wstrrstr("АБВГДЕЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯАБВГДЕЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯ", "")   == 0)
}


static void
test_wsubstr(void)
{
    TEST(1303, wsubstr("ABC", 0, 3) == "ABC");
    TEST(1304, wsubstr("ABC", -1000, 1000) == "ABC");
    TEST(1305, wsubstr("ABC", 1000, 1000) == "");
    TEST(1306, wsubstr("ABC", 1, 0) == "");
    TEST(1307, wsubstr("ABC", 1, 1) == "A");
    TEST(1308, wsubstr("ABC", 1, 2) == "AB");
    TEST(1309, wsubstr("ABC", 1, 3) == "ABC");
    TEST(1310, wsubstr("ABC", 1, 100) == "ABC");
    TEST(1311, wsubstr("ABC", 3, 0) == "");
    TEST(1312, wsubstr("ABC", 3, 1) == "C");
    TEST(1313, wsubstr("ABC", 3, 100) == "C");

    TEST(1314, wsubstr("ξεσκεπάζω την ψυχοφθόρα βδελυγμία", 33, 0) == "");
    TEST(1315, wsubstr("ξεσκεπάζω την ψυχοφθόρα βδελυγμία", 1, 9)  == "ξεσκεπάζω");
    TEST(1316, wsubstr("ξεσκεπάζω την ψυχοφθόρα βδελυγμία", 11, 3) == "την");
}


static void
test_wfirstof(void)
{
    int wch;
    TEST(1317, wfirstof("АБВГДЕЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯАБВГДЕЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯ", "БВ", wch)  == 2 && wch == L'Б')
    TEST(1318, wfirstof("АБВГДЕЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯАБВГДЕЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯ", "CФВ", wch) == 3 && wch == L'В')
    TEST(1319, wfirstof("АБВГДЕЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯАБВГДЕЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯ", "XBC", wch) == 0 && wch == 0)
}


static void
test_wstrpbrk(void)
{
    TEST(1320,  3 == wstrpbrk("abcdefg", "dc"));
    TEST(1321, wfirstof("АБВГДЕЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯАБВГДЕЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯ", "БВ")  == 2)
    TEST(1322, wfirstof("АБВГДЕЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯАБВГДЕЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯ", "CФВ") == 3)
    TEST(1323, wfirstof("АБВГДЕЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯАБВГДЕЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯ", "XBC") == 0)
}


static void
test_wlastof(void)
{
    int wch;
    TEST(1324, wlastof("АБВГДЕЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯАБВГДЕЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯ", "БВ", wch)  == 35 && wch == L'В')
    TEST(1325, wlastof("АБВГДЕЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯАБВГДЕЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯ", "CФВ", wch) == 53 && wch == L'Ф')
    TEST(1326, wlastof("АБВГДЕЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯАБВГДЕЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯ", "XBC", wch) == 0  && wch == 0)
}


static void
test_wstrcmp(void)
{
    TEST(1327, 0 == wstrcmp("aaa", "aaa"));
    TEST(1328, 0 == wstrcmp("aaa", "aaa"));
    TEST(1329, 0 == wstrcmp("АБВГДЕЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯ", "АБВГДЕЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯ"));
    TEST(1330, 0 == wstrcmp("АБВГДЕЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯYYYY", "АБВГДЕЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯXXXX", 32));
}


static void
test_wstrcasecmp(void)
{
    TEST(1331, 0 == wstrcasecmp("AaA", "aAa"));
    TEST(1332, 0 == wstrcasecmp("AaAXXX", "aAaYYY", 3));
}


static void
test_wlower(void)
{
    TEST(1333, wlower("AbC") == "abc");
    TEST(1334, wlower("ÓÓ") == "óó");
    TEST(1335, wlower(L'Ó') == L'ó');
}


static void
test_wupper(void)
{
    TEST(1336, wupper("aBc") == "ABC");
    TEST(1337, wupper("óó") == "ÓÓ");
    TEST(1338, wupper(L'ó') == L'Ó');
}


static void
test_wread(void)
{
    const string sval = "1234567890abcdefghijklmnopqrstuvwxyz1234567890abcdefghijklmnopqrstuvwxyz\n";
    const string wval = "АБВГДЕЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯАБВГДЕЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯ\n";
    const int wlen = wstrlen(wval);
    int buf, status;
    string line;

    save_position();
    if ((buf = create_buffer("-regress-wbuffer-", NULL, TRUE, 0, "utf8")) == -1) {
        return;
    }
    set_buffer(buf);

    top_of_buffer();
    insert(sval);
    top_of_buffer();
    line = read(NULL, status);
    TEST(1339, status == 1);                     /* EOF */
    TEST(1340, line == sval);

    top_of_buffer();
    insert(wval);
    top_of_buffer();
    line = read(NULL, status);
    TEST(1341, status == 1);                     /* EOF */
    TEST(1342, line == wval);

    line = read(wlen, status);
    TEST(1343, status == 1);                     /* EOF */
    TEST(1344, line == wval);

    line = read(10, status);
    TEST(1345, status == 0);                     /* partial */
    TEST(1346, line == wsubstr(line, 1, 10));

    restore_position(2);
    delete_buffer(buf);
}


static void
test_wsprintf(void)
{
    const string sval = "1234567890abcdefghijklmnopqrstuvwxyz1234567890abcdefghijklmnopqrstuvwxyz";
    const string wval = "АБВГДЕЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯАБВГДЕЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯ";
    const string dval = "他們所有的設備和儀器彷彿都是有生命的";
    string buffer;
    int wc;

    wc = sprintf(buffer, "%s", sval);           // string, by length.
    TEST(1347, wc == strlen(sval));

    wc = sprintf(buffer, "%S", wval);           // wide-string, by length.
    TEST(1348, wc == wstrlen(wval));

    wc = sprintf(buffer, "%W", dval);           // by width
    TEST(1349, wc == wcwidth(dval));
}

/*end*/
