/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: regress2.cr,v 1.16 2015/02/17 23:26:17 ayoung Exp $
 * Regression tests ... part2.
 *
 *
 */

#include "../grief.h"
#include "../debug.h"

#define TEST(x, cond) \
            if (!(cond)) failed2(x); else passed2(x);

#define TESTASYNC(x, cond) \
            {extern int x_lastnum; x_lastnum=0;} if (!(cond)) failed2(x); else passed2(x);

#if defined(__PROTOTYPES__)
static void             passed2(int num);
static void             failed2(int num);

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
static void             test_getopt(void);
static void             test_getsubopt(void);
static void             test_parsename(void);
static void             test_command_list(void);
static void             test_file(void);
static void             test_misc(void);
static void             test_module(void);
static void             test_try(void);
static void             test_display(void);
static void             test_ruler(void);
static void             test_datetime(void);
static void             test_sysinfo(void);
static void             test_ini(void);
#endif


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
    test_misc();
    test_module();
 // test_try();
 // test_exception();
    test_display();
    test_ruler();
    test_datetime();
    test_sysinfo();
    test_ini();
}


static void
passed2(int num)
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
failed2(int num)
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


static void
test_cvt(void)
{
    declare d;

    d = cvt_to_object("1234");                  // integer decimal
    TEST(1000, is_integer(d));

    d = cvt_to_object("01234");                 // integer oct
    TEST(1001, is_integer(d));

    d = cvt_to_object("0x1234");                // integer hex
    TEST(1002, is_integer(d));

    d = cvt_to_object("0b010101");              // integer binary
    TEST(1003, is_integer(d));

    d = cvt_to_object(".123");
    TEST(1004, is_float(d));

    d = cvt_to_object("1.234");
    TEST(1005, is_float(d));

    d = cvt_to_object("1.234e12");
    TEST(1006, is_float(d));

    d = cvt_to_object("NaN");
    TEST(1007, is_float(d) && isnan(d));

    d = cvt_to_object("Inf");
    TEST(1008, is_float(d) && !isfinite(d));

    d = cvt_to_object("\"string\"");
    TEST(1009, is_string(d));

    d = cvt_to_object("'string'");
    TEST(1010, is_string(d));

    d = cvt_to_object("a");
    TEST(1011, is_null(d));
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
    TEST(1012, fmt == fmt2);
    TEST(1013, E_FORMAT & eflags);

    set_echo_format(NULL);                      // clear
    eflags = inq_echo_line();
    fmt2 = inq_echo_format();
    TEST(1014, "" == fmt2);
    TEST(1015, 0 == (E_FORMAT & eflags));

    set_echo_format(ofmt);                      // restore
}


static void
test_split(void)
{
    list l;

    /* basic */
    l = split("a|b|c", "|");
    TEST(1016, length_of_list(l) == 3);
    TEST(1017, l[0] == "a" && l[1] == "b" && l[2] == "c");

    l = split("a|b|", "|");
    TEST(1018, length_of_list(l) == 2);

    l = split("a||b", "|");
    TEST(1019, length_of_list(l) == 2);

    /* numeric */
    l = split("a|16|b|32", "|", 1);
    TEST(1020, l[1] == 16 && l[3] == 32);

    /* numeric, enhanced */
    l = split("a|0x10|b|020|c|0b10000", "|", 2);
    TEST(1021, l[1] == 16 && l[3] == 16 && l[5] == 16);

    /* empties */
    l = split("a||b", "|", NULL, NULL, TRUE);
    TEST(1022, length_of_list(l) == 3);

    l = split("a|b|", "|", NULL, NULL, TRUE);
    TEST(1023, length_of_list(l) == 3);

    /* character mode */
    l = split("a|b|c", '|');
    TEST(1024, length_of_list(l) == 3);

    /* quoted */
    l = split("a,\" b \",c", ",\"");
    TEST(1025, length_of_list(l) == 3 && l[1] == " b ");

    l = split("a,\" b' \",' c\" '", ",\"'");
    TEST(1026, length_of_list(l) == 3 && l[1] == " b' " && l[2] == " c\" ");
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
    TEST(1027, 1 == length_of_list(l));

    l = split_arguments("aaa bbb ccc ddd eee");
    TEST(1028, 5 == length_of_list(l));

    l = split_arguments("\"aaa bbb\" \"ccc ddd\" eee");
    TEST(1029, 3 == length_of_list(l));

    l = split_arguments("");
    TEST(1030, 0 == length_of_list(l));
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
    TEST(1031, length_of_list(l) == 0);

    l = tokenize("hello world", " ");
    TEST(1032, length_of_list(l) == 2 && "hello" == l[0] && "world" == l[1]);

    l = tokenize("hello world", ' ');
    TEST(1033, length_of_list(l) == 2 && "hello" == l[0] && "world" == l[1]);

    l = tokenize("' hello ' ' world '", ' ', TOK_SINGLE_QUOTES);
    TEST(1034, length_of_list(l) == 2 && " hello " == l[0] && " world " == l[1]);

    l = tokenize("\" hello \" \" world \"", ' ', TOK_DOUBLE_QUOTES);
    TEST(1035, length_of_list(l) == 2 && " hello " == l[0] && " world " == l[1]);
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
    TEST(1036, 0 == ret && 0 == result);

    ret = firstof("abcedf", "1234", result);
    TEST(1037, 0 == ret && 0 == result);

    ret = firstof("abcedf", "12ab", result);
    TEST(1038, 1 == ret && 'a' == result);

    ret = firstof("abcedf", "1f2", result);
    TEST(1039, 6 == ret && 'f' == result);

    //lastof
    ret = lastof("abcedf", "", result);
    TEST(1040, 0 == ret && 0 == result);

    ret = lastof("abcedf", "1234", result);
    TEST(1041, 0 == ret && 0 == result);

    ret = lastof("abcedf", "1abf", result);
    TEST(1042, 6 == ret && 'f' == result);

    ret = lastof("abcedf", "1a2", result);
    TEST(1043, 1 == ret && 'a' == result);
}


/*
 *  test_regexp2 ---
 *      additional regexp primitives.
 */
static void
test_regexp2(void)
{
    string s;

    TEST(1044, 6 == re_search(SF_BRIEF|SF_MAXIMAL|SF_IGNORE_CASE|SF_CAPTURES, "{[0-9]+}", "Hello1234World"));
    TEST(1045, re_result(1, s) == 4 && s == "1234");
    TEST(1046, re_result(2, s) == -1);

    TEST(1047, re_search(SF_BRIEF|SF_MAXIMAL|SF_CAPTURES, "{[A-Za-z]+} {[A-Za-z]+}", "Hello World") > 0);
    TEST(1048, re_result(1, s) == 5 && s == "Hello");
    TEST(1049, re_result(2, s) == 5 && s == "World");
    TEST(1050, re_result(3, s) == -1);
}


/*
 *  test_scanf ---
 *      scanf primitive.
 */
static void
test_scanf(void)
{
    string s1, s2;
    int i1, i2, i3;

    /*strings       %s*/
    sscanf("hello1 world1", "%s %s", s1, s2);
    TEST(1051, s1 == "hello1" && s2 == "world1");

    sscanf("hello2world2", "%6s%6s", s1, s2);
    TEST(1052, s1 == "hello2" && s2 == "world2");

    sscanf("aaAhello", "%[a]", s1);
    TEST(1053, s1 == "aa");

    sscanf("hello world", "%[a-z]", s1);
    TEST(1054, s1 == "hello");

    sscanf("\t hello", "%[^a-z]", s1);
    TEST(1055, s1 == "\t ");

    sscanf("hello-world", "%[^-]", s1);
    TEST(1056, s1 == "hello");

    sscanf("hello]", "%[^]]", s1);
    TEST(1057, s1 == "hello");

    sscanf("aa-hello", "%[a-]", s1);
    TEST(1058, s1 == "aa-");

    sscanf("abchello", "%[a-bb-c]", s1);
    TEST(1059, s1 == "abc");

    /* character    %c */
    sscanf("hello", "%c%c", i1, i2);
    TEST(1060, i1 == 'h' && i2 == 'e');

    /* numeric      %d,%u,%i */
    sscanf("1234 -5678 +9010", "%d%d%d", i1, i2, i3);
    TEST(1061, 1234 == i1 && -5678 == i2 && 9010 == i3);
    sscanf("56781234", "%4d%4d", i1, i2);
    TEST(1062, 1234 == i2 && i1 == 5678);

    sscanf("5678 1234 ", "%u%u", i1, i2);
    TEST(1063, 5678 == i1 && 1234 == i2);

    sscanf("1234 -5678 +9010", "%i%i%i", i1, i2, i3);
    TEST(1064, 1234 == i1 && -5678 == i2 && 9010 == i3);

    sscanf("0b01010 0b101010", "%b%b", i1, i2);     /*binary*/
    TEST(1065, 0b01010 == i1 && 0b101010 == i2);

    sscanf("01234 0567", "%o%o", i1, i2);           /*oct*/
    TEST(1066, 01234 == i1 && 0567 == i2);

    sscanf("0xABCD 0xFEA1", "%x%x", i1, i2);        /*hex*/
    TEST(1067, 0xABCD == i1 && 0xFEA1 == i2);

    sscanf("FEDB ABCD", "%x%x", i1, i2);            /*hex*/
    TEST(1068, 0xFEDB == i1 && 0xABCD == i2);

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
    sscanf(" Aabcyz123zZ0 -=()_", " %[[:ascii:]]", s1);
    TEST(1069, s1 == "Aabcyz123zZ0 -=()_");

    sscanf(" Aabcyz123zZ0 -=()_", " %[[:alnum:]]", s1);
    TEST(1070, s1 == "Aabcyz123zZ0");

    sscanf(" Aabcyz123zZ0 -=()_", " %[[:alpha:]]", s1);
    TEST(1071, s1 == "Aabcyz");

    sscanf("\t \nAabcyz123zZ0 -=()_", "%[[:blank:]]", s1);
    TEST(1072, s1 == "\t ");

    sscanf(" \a\b\f\v\e", " %[[:cntrl:]]", s1);
    TEST(1073, s1 == "\a\b\f\v\e");

    sscanf(" Aabcyz123_zZ0 -=()_", " %[[:csym:]]", s1);
    TEST(1074, s1 == "Aabcyz123_zZ0");

    sscanf(" 9876abcyz ", " %[[:digit:]]", s1);
    TEST(1075, s1 == "9876");

    sscanf(" abcyzABCYZ987 ", " %[[:lower:]]", s1);
    TEST(1076, s1 == "abcyz");

    sscanf(" 9876abcyz ", " %[[:print:]]", s1);
    TEST(1077, s1 == "9876abcyz ");

    sscanf(" \t9876abcyz ", "%[[:space:]]", s1);
    TEST(1078, s1 == " \t");

    sscanf(" ABCYZabcyz9876abcyz ", " %[[:upper:]]", s1);
    TEST(1079, s1 == "ABCYZ")

    sscanf(" 9876_abcyzABCYZ ", " %[[:word:]]", s1);
    TEST(1080, s1 == "9876_abcyzABCYZ");

    sscanf(" 9876abcfABCYZ ", " %[[:xdigit:]]", s1);
    TEST(1081, s1 == "9876abcfABC");

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

    test_arg_list1(iarg, farg, sarg, larg, atoi("5"));
    test_arg_list2(iarg, farg, sarg, larg, atoi("5"));
}


static void
test_arg_list1()        // (int iarg, float farg, string sarg, list larg, declare xarg)
{
    list l;
    string str;

    l = arg_list(0);                            // not eval'ed
    TEST(1082, 5 == length_of_list(l));
    TEST(1083, l[0] == "iarg" && l[3] == "larg");
    sprintf(str, "%s", l);

    l = arg_list(0, 1, 3);                      // not eval'ed, limited selection
    TEST(1084, 3 == length_of_list(l));
    TEST(1085, l[0] == "farg" && l[2] == "larg");

    sprintf(str, "%s", l);                      // sprintf list support
    TEST(1086, strlen(str) >= 5*2);
}


static void
test_arg_list2()        // (int iarg, float farg, string sarg, list larg, declare xarg)
{
    list l;

    l = arg_list(1);                            // eval'ed
    TEST(1087, 5 == length_of_list(l));
    TEST(1088, l[0] == 1 && l[2] == "3");

    l = arg_list(1, 1, 3);                      // eval'ed, limited selection
    TEST(1089, 3 == length_of_list(l));
    TEST(1090, l[0] == 2 && l[1] == "3");
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

    int match, ch;
    string value;

    if ((ch = getopt(value, "h", longoptions, arguments, "regress")) >= 0) {
        do {
            ++match;
            switch(ch) {
            case 'h':           // -h or --help option
                if (1 == match) {
                    TEST(1091, 1 == match & value == "");
                } else {
                    TEST(1092, 2 == match && value == "");
                }
                break;

            case 2:             // --option2
                TEST(1093, 3 == match && value == "");
                break;

            case 3:             // --option3
                TEST(1094, 4 == match && value == "");
                break;

            case 'i':           // --integer=<value>
                TEST(1095, 5 == match && value == "1234");
                break;

            case 's':           // --string=<value>
                TEST(1096, 6 == match && value == "56 78");
                break;

            case 'b':           // --bool=<value>
                if (match <= 11) {
                    TEST(1097, value == "1");
                } else {
                    TEST(1098, value == "0");
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

    TEST(1099, match == 16);
    TEST(1100, value == "910 11 \"22 22\"");
    TEST(1101, -1 == getopt(value));
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
                TEST(1102, 1 == match && value == "");
                break;

            case 2:             // 'option2'
                TEST(1103, 2 == match && value == "");
                break;

            case 3:             // 'option3'
                TEST(1104, 3 == match && value == "");
                break;

            case 'i':           // 'integer=<value>'
                TEST(1105, 4 == match && value == "1234");
                break;

            case 's':           // 'string=<value>'
                TEST(1106, 5 == match && value == "56 78");
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

    TEST(1107, 5 == match);
    TEST(1108, -1 == getsubopt(value));
    TEST(1109, "" == value);
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
    TEST(1110, 0 == strcmp(drive, "c:"));
    TEST(1111, 0 == strcmp(path,  "//subdir\\subdir\\"));
    TEST(1112, 0 == strcmp(fname, "fname"));
    TEST(1113, 0 == strcmp(ext,   "ext"));

    parse_filename("z:fname.ext1.ext2", drive, path, fname, ext);
    TEST(1114, 0 == strcmp(drive, "z:"));
    TEST(1115, 0 == strcmp(path,  ""));
    TEST(1116, 0 == strcmp(fname, "fname.ext1"));
    TEST(1117, 0 == strcmp(ext,   "ext2"));     // last extension
}


/*
 *  test_command_list
 *      command/macro_list
 */
static void
test_command_list(void)
{
    list cmd = command_list(FALSE, "*float");
    TEST(1118, 2 == length_of_list(cmd));       // float and is_float

    list macros = macro_list("regress");
    TEST(1119, 1 == length_of_list(macros));    // regress()
}


/*
 *  test_file ---
 *      file functionality.
 */
static void
test_file(void)
{
    //  mkdir/rmdir
    TEST(1120, 0 != access("./grregress"));
    TEST(1121, 0 == mkdir("./grregress", 0666));
    TEST(1122, 0 == access("./grregress"));
    TEST(1123, 0 == rmdir("./grregress"));
    TEST(1124, 0 != access("./grregress"));

    //  mktemp
    //  rename
    //  remove
    //  stat
    int now = time();
    string base = format("%s/gr%dXXXXXX", inq_tmpdir(), getpid());
    string temp = mktemp(base),
       temp2 = temp + "2";
    int size = -1, mtime, ctime, atime, mode;

    TEST(1125, 0 == access(temp));
    TEST(1126, 0 == stat(temp, size, mtime, ctime, atime, mode) && 0 == size && \
            mtime >= now && ctime >= now && atime >= now && S_IFREG == (S_IFREG & mode));
    TEST(1127, 0 == rename(temp, temp2));
    TEST(1128, 0 == access(temp2));
    TEST(1129, 0 == remove(temp2));
    TEST(1130, 0 != access(temp2));
    TEST(1131, 0 != stat(temp2));

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
 *  test_module ---
 *      module functionality.
 */
static void
test_module(void)
{
    TEST(1132, inq_module() == "regress");      // current association
    TEST(1133, inq_macro("test_module", 2) == inq_macro("test_misc", 2));
    TEST(1134, inq_macro("test_module", 2) != inq_macro("crisp", 2));
    TEST(1135,  2 == module("regress"));        // pre-existing association
    TEST(1136, -1 == module("newmodule"));      // re-assignment, error
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

    TEST(1137, version(vmajor, vminor, vedit, NULL, vmachtype, vcompiled) >= 302 && \
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
    TEST(1138, "" == vmachtype);

    //  srand()/rand()
    const int seed = 1234;

    srand(seed); int rand1 = rand(); srand(seed);
    TEST(1139, rand1 == rand());

    //  strerror - need EINVAL etc
    TEST(1140, "Success" == strerror(0));
    TEST(1141, "Unknown error" == strerror(-1));
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
    TEST(1142, inq_display_mode("scroll_cols")  == display_mode(NULL, NULL, -1));
    TEST(1143, inq_display_mode("scroll_rows")  == display_mode(NULL, NULL, NULL, -1));
    TEST(1144, inq_display_mode("visible_cols") == display_mode(NULL, NULL, NULL, NULL, -1));
    TEST(1145, inq_display_mode("visible_rows") == display_mode(NULL, NULL, NULL, NULL, NULL, -1));
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
    TEST(1146, inq_tabs() == "8 17");

    tabs("9 17");
    TEST(1147, inq_tabs() == "9 17");
    TEST(1148, 8 == inq_tab());                  /* default */

    tabs("");
    TEST(1149, inq_tabs() == "");

    execute_macro("tabs " + saved_tabs);
    TEST(1150, inq_tabs() == saved_tabs);

    /* indent */
    set_indent(5);
    TEST(1151, inq_indent() == 5);               /* indent builds simple ruler */

    set_indent(0);
    TEST(1152, inq_indent() == 0);

    set_indent(saved_indent);
    TEST(1153, inq_indent() == saved_indent);

    /* ruler */
    set_ruler(NULL, ruler);
    TEST(1154, inq_ruler(NULL, 5) == "5 9 20 25 30");
    r = inq_ruler(NULL, 10, TRUE);
    TEST(1155, r[0] == 5 && r[1] == 9 && r[4] == 30);
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
    int time1 = time();
    sleep(1);
    TEST(1156, time() > time1);
    sleep(1);
    TEST(1157, time() > ++time1);
    sleep(1);
    TEST(1158, time() > ++time1);

    //  inq_idle_time/sleep
    TEST(1159, inq_idle_time() >= 1);
    sleep(1);
    TEST(1160, inq_idle_time()  > 1);

    //  inq_clock/sleep(ms)
    int clock1 = inq_clock();
    for (time1 = time(); time1 == time();)
        { }
    TEST(1161, inq_clock() > clock1);

    //  localtime/date
    int year1, mon1, mday1, hour1, min1, sec1;
      string monname1, dayname1;
    int year2, mon2, mday2, hour2, min2, sec2;
      string monname2, dayname2;
    int year3, mon3, mday3;
      string monname3, dayname3;

    localtime(time1, year1, mon1, mday1, monname1, dayname1, hour1, min1, sec1);
    gmtime(time1, year2, mon2, mday2, monname2, dayname2, hour2, min2, sec2);
    date(year3, mon3, mday3, monname3, dayname3);
    TEST(1162, year1 >= 2015 && mon1 >= 1 && mon1 <= 12 && mday1 >= 1 && mday1 <= 31);
    TEST(1163, year2 >= 2015 && mon2 >= 1 && mon2 <= 12 && mday2 >= 1 && mday2 <= 31);
    TEST(1164, year3 >= 2015 && mon3 >= 1 && mon3 <= 12 && mday3 >= 1 && mday3 <= 31);
    TEST(1165, year1 == year2 && year2 == year3 &&
                 mon1 == mon2 && mon2 == mon3 &&
                  mday1 == mday2 && mday2 == mday3);

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
		
    TEST(1166, 0 == access(homedir));
    TEST(1167, strlen(profiledir));
    TEST(1168, 0 == access(tmpdir));

    string username = inq_username(),
        hostname = inq_hostname();

    TEST(1169, username);
    TEST(1170, hostname);

    string usysname, unodename, uversion, urelease, umachine;

    TEST(1171, 0 == uname(usysname, unodename, uversion, urelease, umachine) && \
            strlen(usysname) && strlen(unodename) && strlen(uversion) && strlen(urelease) && strlen(umachine));
}


/*
 *  test_ini ---
 *      INI parser.
 */
static void
test_ini(void)
{
    const string section = "Section X";
    string sect, key, value;
    int fd, ret, cnt;

    fd = iniopen();
    TEST(1172, fd >= 0);

    inipush(fd, section, "zzz", "9999", "comment 1");
    inipush(fd, section, "zzz", "8888", "comment 2");
    inipush(fd, section, "yyy", "7777", "comment 3", TRUE);
    inipush(fd, section, "yyy", "6666", "comment 4", TRUE);
    inipush(fd, section, "xxx", "5555");
    iniremove(fd, section, "yyy");

    for (cnt = 0, ret = inifirst(fd, sect, key, value); 1 == ret;
            ret = ininext(fd, sect, key, value)) {
        switch (++cnt) {
        case 1: TEST(1173, key == "zzz" && value == "8888"); break;
        case 2: TEST(1174, key == "xxx" && value == "5555"); break;
        }
    }
    TEST(1175, 2 == cnt);

    iniexport(fd, "iniout.cfg");
    iniclose(fd);
}
/*end*/








