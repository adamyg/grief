/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: perf.cr,v 1.7 2014/10/22 02:34:29 ayoung Exp $
 * Engine performance monitoring.
 *
 *
 */

#include "../grief.h"

#define UNITS       1000000
#define BASE        10

static string       perf_time(int test, string mac);
static string       display_time(int s, int e);
static void         pf_assert(int cond);
static void         pf_loop(void);
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
static void         pf_sieve(void);

static list         tests = {
   "loop",
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
   "sieve"
   };


void
main(void)
{
}


/* Perform a set of performance tests. These are used for
 * performance comparisons across processor ranges and for some
 * regression testing.
 */
void
perf(int arg)
{
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
    sprintf(buf, "\n%d %s %d %02d:%02d v%d.%d%c\n", d, mon, y, h, m, maj, min, edit);
    insert(buf);
    set_bottom_of_window();
    refresh();

    stime = inq_clock();

    if (arg >= 1 && arg <= length_of_list(tests)) {
        perf_time(arg, tests[arg - 1]);
    } else {					/* start of tests.. */
        for (i = 0; i < length_of_list(tests); i++) {
            perf_time(i + 1, tests[i]);
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
    move_abs(0, 24);
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
display_time(int s, int e)
{
    int sec = (e - s) / UNITS;
    int usec = (e - s) % UNITS;
    string buf;

    sprintf(buf, "Time: %d.%02d", sec, usec / 10000);
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
pf_sieve(void)
{
    extern list calc_primes(int);

    calc_primes(1000*BASE);
}
