#include <edidentifier.h>
__CIDENT_RCSID(gr_rc_test_c,"$Id: rc_test.c,v 1.4 2024/12/05 19:18:15 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: rc_test.c,v 1.4 2024/12/05 19:18:15 cvsuser Exp $
 *
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>

#include "argrc.h"

static void         dump(const struct argrc *rc);
static void         tests(void);

static unsigned     verbose = 1;

int
main(int argc, char **argv)
{
    struct argrc rc = {0};

    if (argc < 2 || argc > 2) {
        printf("rc_test: usage <input>\n");
        exit(3);
    }

    if (0 == strcmp(argv[1], "--tests")) {
        verbose = 0;
        tests();

    } else if (0 == strcmp(argv[1], "--vtests")) {
        tests();

    } else {
        if (argrc_load(argv[1], &rc) >= 1) {
            dump(&rc);
            argrc_free(&rc);
        }
    }
    return 0;
}


static void
dump(const struct argrc *rc)
{
    unsigned cnt = 0;
    if (verbose) {
        while (cnt < rc->argc && rc->argv[cnt]) {
            printf("%u: %s\n", cnt, rc->argv[cnt]);
            ++cnt;
        }
    }
}


static void
check(unsigned a, unsigned b, int ret)
{
    printf("%u.%2u: %s\n", a, b, (ret ? "SUCCESS" : "FAILURE"));
}


// blank and/or comments; empty
static void
test1(void)
{
    struct argrc rc = {0};
    unsigned b = 0;

    check(1, ++b, argrc_parse("", &rc) == 0);
    check(1, ++b, argrc_parse(" ", &rc) == 0);
    check(1, ++b, argrc_parse("\t", &rc) == 0);
    check(1, ++b, argrc_parse("\t\n", &rc) == 0);
    check(1, ++b, argrc_parse("\f\v\n\r", &rc) == 0);

    check(1, ++b, argrc_parse("# comment", &rc) == 0);
    check(1, ++b, argrc_parse("# comment\n", &rc) == 0);
    check(1, ++b, argrc_parse("# comment\n\r", &rc) == 0);

    check(1, ++b, argrc_parse("   # comment", &rc) == 0);
    check(1, ++b, argrc_parse("\t# comment", &rc) == 0);
    check(1, ++b, argrc_parse("# comment\n   # comment\n", &rc) == 0);
    check(1, ++b, argrc_parse("  \n", &rc) == 0);
    check(1, ++b, argrc_parse("  \n  \n", &rc) == 0);
    check(1, ++b, argrc_parse("  \n # comment\n\t\t\n\t#comment\n", &rc) == 0);
}


// text optional comments; non-empty results
static void
test2(void)
{
    struct argrc rc = {0};
    unsigned b = 0;

    /// 2.1
    check(2, ++b, argrc_parse("xxx", &rc) == 1 &&
                        rc.argc == 1 &&
                        strcmp(rc.argv[0], "xxx") == 0);
    dump(&rc);
    argrc_free(&rc);

    /// 2.2
    check(2, ++b, argrc_parse("xxx\n", &rc) == 1 &&
                        rc.argc == 1 &&
                        strcmp(rc.argv[0], "xxx") == 0);
    dump(&rc);
    argrc_free(&rc);

    /// 2.3
    check(2, ++b, argrc_parse("aaa bbb", &rc) == 2 &&
                        rc.argc == 2 &&
                        strcmp(rc.argv[0], "aaa") == 0 &&
                        strcmp(rc.argv[1], "bbb") == 0 &&
                        rc.argv[2] == NULL);
    dump(&rc);
    argrc_free(&rc);

    /// 2.4
    check(2, ++b, argrc_parse("aa\tbb", &rc) == 2 &&
                        rc.argc == 2 &&
                        strcmp(rc.argv[0], "aa") == 0 &&
                        strcmp(rc.argv[1], "bb") == 0 &&
                        rc.argv[2] == NULL);
    dump(&rc);
    argrc_free(&rc);

    /// 2.5
    check(2, ++b, argrc_parse("a\nb", &rc) == 2 &&
                        rc.argc == 2 &&
                        strcmp(rc.argv[0], "a") == 0 &&
                        strcmp(rc.argv[1], "b") == 0 &&
                        rc.argv[2] == NULL);
    dump(&rc);
    argrc_free(&rc);

    /// 2.6
    check(2, ++b, argrc_parse("a\n\rb\n", &rc) == 2 &&
                        rc.argc == 2 &&
                        strcmp(rc.argv[0], "a") == 0 &&
                        strcmp(rc.argv[1], "b") == 0 &&
                        rc.argv[2] == NULL);
    dump(&rc);
    argrc_free(&rc);

    /// 2.7
    check(2, ++b, argrc_parse("\na\n\nb\n", &rc) == 2 &&
                        rc.argc == 2 &&
                        strcmp(rc.argv[0], "a") == 0 &&
                        strcmp(rc.argv[1], "b") == 0 &&
                        rc.argv[2] == NULL);
    dump(&rc);
    argrc_free(&rc);

    /// 2.8
    check(2, ++b, argrc_parse("yyy\n\nzzz", &rc) == 2 &&
                        rc.argc == 2 &&
                        strcmp(rc.argv[0], "yyy") == 0 &&
                        strcmp(rc.argv[1], "zzz") == 0 &&
                        rc.argv[2] == NULL);
    dump(&rc);
    argrc_free(&rc);

    /// 2.9
    check(2, ++b, argrc_parse("yyy\n  \nzzz", &rc) == 2 &&
                        rc.argc == 2 &&
                        strcmp(rc.argv[0], "yyy") == 0 &&
                        strcmp(rc.argv[1], "zzz") == 0 &&
                        rc.argv[2] == NULL);
    dump(&rc);
    argrc_free(&rc);

    /// 2.10
    check(2, ++b, argrc_parse("yyy\n#comment\nzzz", &rc) == 2 &&
                        rc.argc == 2 &&
                        strcmp(rc.argv[0], "yyy") == 0 &&
                        strcmp(rc.argv[1], "zzz") == 0 &&
                        rc.argv[2] == NULL);
    dump(&rc);
    argrc_free(&rc);

    /// 2.11
    check(2, ++b, argrc_parse("#comment\nyyy\n#comment", &rc) == 1 &&
                        rc.argc == 1 &&
                        strcmp(rc.argv[0], "yyy") == 0 &&
                        rc.argv[1] == NULL);
    dump(&rc);
    argrc_free(&rc);

    /// 2.12
    check(2, ++b, argrc_parse("#comment\nxxx\n#comment\n\nyyy\n#comment\nzzz", &rc) == 3 &&
                        rc.argc == 3 &&
                        strcmp(rc.argv[0], "xxx") == 0 &&
                        strcmp(rc.argv[1], "yyy") == 0 &&
                        strcmp(rc.argv[2], "zzz") == 0 &&
                        rc.argv[3] == NULL);
    dump(&rc);
    argrc_free(&rc);
}

// quoted text
static void
test3(void)
{
    struct argrc rc = {0};
    unsigned b = 0;

    /// 3.1
    check(3, ++b, argrc_parse("\"hello\"", &rc) == 1 &&
                        strcmp(rc.argv[0], "hello") == 0 &&
                        rc.argv[1] == NULL);
    dump(&rc);
    argrc_free(&rc);

    /// 3.2
    check(3, ++b, argrc_parse("\"hello world\"", &rc) == 1 &&
                        strcmp(rc.argv[0], "hello world") == 0 &&
                        rc.argv[1] == NULL);
    dump(&rc);
    argrc_free(&rc);

    /// 3.3
    check(3, ++b, argrc_parse("\"hello \\\" world\"", &rc) == 1 &&
                        strcmp(rc.argv[0], "hello \" world") == 0 &&
                        rc.argv[1] == NULL);
    dump(&rc);
    argrc_free(&rc);

    /// 3.4
    check(3, ++b, argrc_parse("\"hello \\' world\"", &rc) == 1 &&
                        strcmp(rc.argv[0], "hello ' world") == 0 &&
                        rc.argv[1] == NULL);
    dump(&rc);
    argrc_free(&rc);

    /// 3.5
    check(3, ++b, argrc_parse("\"hello \\\\ world\"", &rc) == 1 &&
                        strcmp(rc.argv[0], "hello \\ world") == 0 &&
                        rc.argv[1] == NULL);
    dump(&rc);
    argrc_free(&rc);

    /// 3.6
    check(3, ++b, argrc_parse("'hello world'", &rc) == 1 &&
                        strcmp(rc.argv[0], "hello world") == 0 &&
                        rc.argv[1] == NULL);
    dump(&rc);
    argrc_free(&rc);

    /// 3.7
    check(3, ++b, argrc_parse("'hello \\' world'", &rc) == 1 &&
                        strcmp(rc.argv[0], "hello ' world") == 0 &&
                        rc.argv[1] == NULL);
    dump(&rc);
    argrc_free(&rc);

    /// 3.8
    check(3, ++b, argrc_parse(" \"hello world\" \n", &rc) == 1 &&
                        strcmp(rc.argv[0], "hello world") == 0 &&
                        rc.argv[1] == NULL);
    dump(&rc);
    argrc_free(&rc);

    /// 3.9
    check(3, ++b, argrc_parse("\"hello\"\n\"world\"\n", &rc) == 2 &&
                        strcmp(rc.argv[0], "hello") == 0 &&
                        strcmp(rc.argv[1], "world") == 0 &&
                        rc.argv[2] == NULL);
    dump(&rc);
    argrc_free(&rc);

    /// 3.10
    check(3, ++b, argrc_parse("\"hello \n\"world\"\n", &rc) == 2 && // missing trailing; terminated end-of-line
                        strcmp(rc.argv[0], "hello ") == 0 &&
                        strcmp(rc.argv[1], "world") == 0 &&
                        rc.argv[2] == NULL);
    dump(&rc);
    argrc_free(&rc);
}


// argv0
static void
test4(void)
{
    struct argrc rc = {0};
    unsigned b = 0;

    /// 4.1
    check(4, ++b, argrc_parse2("appname", "#empty", &rc) == 0 &&
                        rc.argc == 0 && rc.argv == NULL);
    dump(&rc);
    argrc_free(&rc);

    /// 4.2
    check(4, ++b, argrc_parse2("appname", "\"hello\"", &rc) == 1 &&
                        rc.argc == 2 &&
                        strcmp(rc.argv[0], "appname") == 0 &&
                        strcmp(rc.argv[1], "hello") == 0 &&
                        rc.argv[2] == NULL);
    dump(&rc);
    argrc_free(&rc);
}


static void
tests(void)
{
    test1();
    test2();
    test3();
    test4();
}


///////////////////////////////////////////////////////////////////////////////

void *
chk_alloc(size_t size)
{
    assert(size);
    return (void *)malloc(size);
}


void
chk_free(void *p)
{
    if (p) free(p);
}

//end
