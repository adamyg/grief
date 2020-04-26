/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: perf.cr,v 1.9 2020/04/20 23:14:33 cvsuser Exp $
 * Engine performance monitoring.
 *
 *
 */

#include "../grief.h"

#define UNITS       1000000
#define BASE        10

static string       perf_time(int test, string mac);
static string       benchmark_time(int test, string mac);
static string       display_time(int s, int e);
static void         pf_assert(int cond);
static void         pf_loop(void);
static void         pf_loop_reg(void);
static void         pf_macro_list(void);
static void         pf_command_list(void);
static void         pf_strcat(void);
static void         pf_string_assign(void);
static void         pf_get_nth(void);
static void         pf_put_nth(void);
static void         pf_if(void);
static void         pf_trim(void);
static void         pf_compress(void);
static void         pf_loop_float(void);
static void         pf_loop_float_reg(void);
static void         pf_sieve(void);

static list         basic = {
   "loop",
   "loop_reg",
   "macro_list",
   "command_list",
   "strcat",
   "string_assign",
   "get_nth",
   "put_nth",
   "if",
   "trim",
   "compress",
   "loop_float",
   "loop_float_reg",
   "primes"
   };

static list         benchmarks = {
   "add_int",
   "add_float",
   "add_string",
   "assign_add",
   "assign_addto",
   "assign_boolean",
   "assign_const_int",
   "assign_literal",
   };


/*
 * Perform a set of performance tests.
 */
void
perf(declare arg = 0)
{
    const int basic_count = length_of_list(basic),
        benchmark_count = length_of_list(benchmarks);
    int h, m, s;
    int d, y;
    int i;
    int maj, min, edit;
    string mon;
    string buf;
    int stime;

    date(y, NULL, d, mon);
    time(h, m, s);
    edit_file("PERF.log");
    end_of_buffer();
    version(maj, min, edit);
    sprintf(buf, "\n%d %s %d %02d:%02d v%d.%d%c (%s)\n",
        d, mon, y, h, m, maj, min, edit, (grief_version(1) ? "release" : "debug"));
    insert(buf);
    set_bottom_of_window();
    refresh();

    stime = inq_clock();

    if (is_integer(arg) && arg >= 1) {
        /*
         *  indexed
         */
        if (arg >= 1 && arg <= basic_count) {
            perf_time(arg, basic[arg - 1]);

        } else if (arg > basic_count && arg <= (basic_count + benchmark_count)) {
            benchmark_time(arg, benchmarks[arg - basic_count]);
        }

    } else if (is_string(arg) && strlen(arg)) {
        /*
         *  named
         */
        int test = 0;

        for (i = 0; i < length_of_list(basic); ++i, ++test) {
            if (arg == basic[i]) {
                perf_time(++test, basic[i]);
                test = -1;                      /* done */
                break;
            }
        }

        if (test != -1) {                       /* not found */
            for (i = 0; i < length_of_list(benchmarks); ++i, ++test) {
                if (arg == benchmarks[i]) {
                    benchmark_time(++test, benchmarks[i]);
                    test = -1;
                    break;
                }
            }
        }

    } else {
        /*
         *  test collection
         */
        int test = 0;

        for (i = 0; i < length_of_list(basic); ++i) {
            perf_time(++test, basic[i]);
        }
        for (i = 0; i < length_of_list(benchmarks); ++i) {
            benchmark_time(++test, benchmarks[i]);
        }
    }

    stime = inq_clock() - stime;
    sprintf(buf, "                     Total: %d.%06d\n", stime / UNITS, stime % UNITS);
    insert(buf);
}


static string
perf_time(int test, string mac)
{
    int s, e;
    string str;

    sprintf(str, "  %d) %s", test, mac);
    insert(str);
    move_abs(0, 40);
    refresh();
    s = inq_clock();
    execute_macro("pf_" + mac);
    e = inq_clock();
    str = display_time(s, e);
    insert(str);
    insert("\n");
    refresh();
}


static string
benchmark_time(int test, string mac)
{
    int s, e;
    string str;

    sprintf(str, "  %d) %s", test, mac);
    insert(str);
    move_abs(0, 40);
    refresh();
    load_macro("benchmarks/" + mac);
    message(mac);
    s = inq_clock();
    execute_macro("benchmark_" + mac);
    e = inq_clock();
    str = display_time(s, e);
    insert(str);
    insert("\n");
    refresh();
}


static string
display_time(int s, int e)
{
    int sec = (e - s) / UNITS;
    int usec = (e - s) % UNITS;
    string buf;

//  sprintf(buf, "Time: %d.%02d", sec, usec / 10000);
    sprintf(buf, "Time: %d.%03d", sec, usec / 1000);
    return buf;
}


static void
pf_assert(int cond)
{
    if (cond == 0) {
        error("Assertion failed.");
    }
}


static void
pf_loop(void)
{
    int i;

    for (i = 0; i < 30000*BASE; i++) {
        ;
    }
}


static void
pf_loop_reg(void)
{
    register int i;

    for (i = 0; i < 30000*BASE; i++) {
        ;
    }
}



static void
pf_macro_list(void)
{
    int i;

    for (i = 0; i < 1000*BASE; i++) {
        macro_list();
    }
}


static void
pf_command_list(void)
{
    int i;

    for (i = 0; i < 1000*BASE; i++) {
        command_list();
    }
}


static void
pf_strcat(void)
{
    string s;
    int i, j;

    for (i = 0; i < 1000*BASE; i++) {
        s = "";
        for (j = 0; j < 10; j++)
            s += "A";
    }
}


static void
pf_string_assign(void)
{
    int i;
    string s;

    for (i = 0; i < 10000*BASE; i++) {
        s = "abc" + "def";
    }
}


static void
pf_get_nth(void)
{
    declare x, y, z;
    list l;
    int i;

    for (i = 0; i < 2500*BASE; i++) {
        l = quote_list("ABC", "DEF", "GHI");
        x = l[0];
        y = l[1];
        z = l[2];
    }
}


static void
pf_put_nth(void)
{
    list l;
    int i;

    for (i = 0; i < 5000*BASE; i++) {
        l = NULL;
        l[0] = 123;
        l[1] = "abc" + "def";
        l[2] = quote_list(1, 2, 3);
    }
}


static void
pf_if(void)
{
    string s;
    int i;

    for (i = 0; i < 10000*BASE; i++) {
        if ((s = "abc") == "def") {
            break;
        }
    }
}


static void
pf_trim(void)
{
    string s;
    int i;

    for (i = 0; i < 1000*BASE; i++) {
        s = trim("abc \n");
        pf_assert(s == "abc");
        s = ltrim("xxabc", "xx");
        pf_assert(s == "abc");
        s = trim("\n");
        pf_assert(s == "");
    }
}


static void
pf_compress(void)
{
    int i;
    string s;

    for (i = 0; i < 5000*BASE; i++) {
        s = compress(ltrim(trim("\n")));
        pf_assert(s == "");
    }
}


static void
pf_loop_float(void)
{
    float i;

    for (i = 0; i < 30000*BASE; i++) {
        ;
    }
}


static void
pf_loop_float_reg(void)
{
    register float i;

    for (i = 0; i < 30000*BASE; i++) {
        ;
    }
}


static void
pf_primes(void)
{
    extern list calc_primes(int);

    calc_primes(5000);
}

/*end*/
