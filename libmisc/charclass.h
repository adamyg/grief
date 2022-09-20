#ifndef GR_CHARCLASS_H_INCLUDED
#define GR_CHARCLASS_H_INCLUDED
/*
 * Public domain, 2008, Todd C. Miller <Todd.Miller@courtesan.com>
 *
 * Copyright (c) 1998 - 2022, Adam Young.
 * All rights reserved.
 *
 * This file is part of the GRIEF Editor.
 *
 * The GRIEF Editor is free software: you can redistribute it
 * and/or modify it under the terms of the GRIEF Editor License.
 *
 * Redistributions of source code must retain the above copyright
 * notice, and must be distributed with the license document above.
 *
 * Redistributions in binary form must reproduce the above copyright
 * notice, and must include the license document above in
 * the documentation and/or other materials provided with the
 * distribution.
 *
 * The GRIEF Editor is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * License for more details.
 * ==end==
 */

/*
 *  POSIX character class support for fnmatch() and glob().
 */

static int is_alnum(int c)      { return isalnum((unsigned char)c); }
static int is_alpha(int c)      { return isalpha((unsigned char)c); }
static int is_blank(int c)
{
#if defined(HAVE___ISBLANK)
    return __isblank((unsigned char)c);
#elif defined(HAVE_ISBLANK)
    return isblank((unsigned char)c);
#else
    return (' ' == c || '\t' == c);
#endif
}
static int is_cntrl(int c)      { return iscntrl((unsigned char)c); }
static int is_digit(int c)      { return isdigit((unsigned char)c); }
static int is_graph(int c)      { return isgraph((unsigned char)c); }
static int is_lower(int c)      { return islower((unsigned char)c); }
static int is_print(int c)      { return isprint((unsigned char)c); }
static int is_punct(int c)      { return ispunct((unsigned char)c); }
static int is_space(int c)      { return isspace((unsigned char)c); }
static int is_upper(int c)      { return isupper((unsigned char)c); }
static int is_word(int c)       { return ('_' == c || isalnum((unsigned char)c)); }
static int is_xdigit(int c)     { return isxdigit((unsigned char)c); }

static struct cclass {
        const char *name;
        int (*isctype)(int);
} cclasses[] = {
        { "alnum",      is_alnum },
        { "alpha",      is_alpha },
        { "blank",      is_blank },
        { "cntrl",      is_cntrl },
        { "digit",      is_digit },
        { "graph",      is_graph },
        { "lower",      is_lower },
        { "print",      is_print },
        { "punct",      is_punct },
        { "space",      is_space },
        { "upper",      is_upper },
        { "word",       is_word },
        { "xdigit",     is_xdigit },
        { NULL,         NULL }
};

#define NCCLASSES       (sizeof(cclasses) / sizeof(cclasses[0]) - 1)

#endif  /*GR_CHARCLASS_H_INCLUDED*/

/*end*/
