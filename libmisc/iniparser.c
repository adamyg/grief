#include <edidentifier.h>
__CIDENT_RCSID(gr_iniparser_c,"$Id: iniparser.c,v 1.20 2025/02/07 03:03:22 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: iniparser.c,v 1.20 2025/02/07 03:03:22 cvsuser Exp $
 * INI parser.
 *
 *
 * Copyright (c) 2012 - 2025, Adam Young.
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

#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif
//#define ED_LEVEL 1
#include <editor.h>
#include <eddebug.h>

#include <stdio.h>
#include <assert.h>
#include <libstr.h>                             /* str_...()/sxprintf() */

#define  __IFILE_INTERNAL__
#include "iniparser.h"

#define E_SECTION_MISSING_DELIMITER \
            "Invalid section, missing delimiter"

#define E_SECTION_EMPTY_NAME \
            "Invalid section, empty name"

#define E_SECTION_DUPLICATE \
            "Invalid section, duplicate name"

#define E_PROPERTY_MISSING_DELIMITER \
            "Invalid property, missing delimiter"

#define E_PROPERTY_UNCLOSED_QUOTE \
            "Invalid property, unclosed quote"

#define ISWHITE(__c)    (' ' == (__c) || '\t' == (__c))

typedef TAILQ_HEAD(_iniPropertyList, _iniProperty)
                        IPropertiesList_t;      // properties list.

typedef TAILQ_HEAD(_iniCommentList, _iniComment)
                        ICommentsList_t;        // comment list.

typedef struct _iniProperty {
    MAGIC_t             n_magic;                // structure magic.
#define IPROP_MAGIC         MKMAGIC('I','n','P','r')
    TAILQ_ENTRY(_iniProperty)
                        n_link;                 // property list.
    unsigned            n_line;                 // source line number.
    char                n_delimiter;            // key/value delimiter ('=' or ':').
    const char         *n_key;                  // key address.
    const char         *n_data;                 // optional data address.
    const char         *n_comment;              // optional comment text address.
} IniProperty_t;


typedef struct _iniComment {
    MAGIC_t             c_magic;                // structure magic.
#define ICOMMENT_MAGIC      MKMAGIC('I','n','C','t')
    TAILQ_ENTRY(_iniComment)
                        c_link;                 // property list.
    unsigned            c_line;                 // source line number.
    const char         *c_text;                 // comment text address.
} IniComment_t;


typedef struct _iniSection {
    MAGIC_t             s_magic;                // structure magic.
#define ISECT_MAGIC         MKMAGIC('I','n','S','t')

    TAILQ_ENTRY(_iniSection)
                        s_link;                 // section list.
    RB_ENTRY(_iniSection)
                        s_node;                 // section node.
    unsigned            s_line;                 // source line number.
    unsigned            s_count;                // property count.
    IPropertiesList_t   s_properties;           // properties.
    ICommentsList_t     s_comments;             // comments.
    unsigned            s_nlen;                 // section name length, in bytes.
    const char         *s_name;                 // section name.
} IniSection_t;


static IniSection_t *   iniSectionNew(IFILE *ifile, const char *name, unsigned nlen, unsigned line);
static IniSection_t *   iniSectionFnd(IFILE *ifile, const char *name, unsigned nlen);
static void             iniSectionClr(IniSection_t *sect, int comments);
static void             iniSectionDel(IFILE *ifile, IniSection_t *sect, int root);

static IniProperty_t *  iniPropertyNew(IniSection_t *sect, char delimiter,
                            const char *key, unsigned klen, const char *dat, unsigned dlen, unsigned line);
static IniProperty_t *  iniPropertyNewx(IniSection_t *sect, char delimiter,
                            const char *key, unsigned klen, const char *dat, unsigned dlen, const char *comment, int clen, unsigned line);
static IniComment_t *   iniCommentNew(ICommentsList_t *comments, const char *text, unsigned tlen, unsigned line);

static int              iniGetx(IFILE *ifile, IniSection_t **iSect);
static int              iniGetl(IFILE *ifile, const char **special, const char **comment);
static int              iniLoad(IFILE *ifile);


static int
iniSectionCmp(const IniSection_t *a, const IniSection_t *b)
{
    unsigned alen, blen;

    if ((alen = a->s_nlen) == (blen = b->s_nlen)) {
        return str_nicmp(a->s_name, b->s_name, alen);
    }
    return (alen < blen ? -1 : 1);
}


RB_PROTOTYPE_STATIC(_iniSectionTree, _iniSection, s_node, iniSectionCmp);
RB_GENERATE_STATIC(_iniSectionTree, _iniSection, s_node, iniSectionCmp);


/*  Function: iniSectionNew
 *      Section object creation.
 *
 *  Parameters:
 *      ifile - INI file object.
 *      name - Optional Section name.
 *      nlen - Name length, in bytes.
 *      line - Source line number, otherwise 0.
 *
 *  Returns:
 *      Section object, otherwise NULL.
 */
static IniSection_t *
iniSectionNew(IFILE *ifile, const char *name, unsigned nlen, unsigned line)
{
    ISectionList_t *sections = &ifile->i_sections;
    IniSection_t *sect;

    assert((name && nlen > 0) || (NULL == name && 0 == nlen));

    if (NULL != (sect =
            malloc(sizeof(IniSection_t) + nlen + 1))) {
        /*
         *  < Section Object >
         *      [ Name Buffer \0 ]
         */
        char *s_name = (char *)(sect + 1);

        sect->s_magic = ISECT_MAGIC;
        TAILQ_INIT(&sect->s_properties);
        TAILQ_INIT(&sect->s_comments);
        sect->s_name  = (nlen ? s_name : NULL);
        sect->s_nlen  = nlen;
        sect->s_line  = line;
        sect->s_count = 0;

        TAILQ_INSERT_TAIL(sections, sect, s_link);

        if (name) {
            memcpy(s_name, (const char *)name, nlen);
            s_name[nlen] = 0;

            RB_INSERT(_iniSectionTree, &ifile->i_lookup, sect);
        }
    }
    return sect;
}


/*  Function: iniSectionFnd
 *      Section find
 *
 *  Parameters:
 *      ifile - INI file object.
 *      name - Section name.
 *      nlen - Name length, in bytes.
 *
 *  Returns:
 *      Section object, otherwise NULL.
 */
static IniSection_t *
iniSectionFnd(IFILE *ifile, const char *name, unsigned nlen)
{
    IniSection_t t_sect;

    assert(name);
    if (0 == *name) {
        assert(ifile->i_root);
        return ifile->i_root;
    }
    t_sect.s_nlen = nlen;
    t_sect.s_name = name;
    return RB_FIND(_iniSectionTree, &ifile->i_lookup, &t_sect);
}


/*  Function: iniSectionClr
 *      Section clear.
 *
 *  Parameters:
 *      sect - Section object.
 *      docomments - *TRUE* clear line comments.
 *
 *  Returns:
 *      nothing.
 */
static void
iniSectionClr(IniSection_t *sect, int docomments)
{
    if (sect) {
        IPropertiesList_t *properties = &sect->s_properties;
        IniProperty_t *prop;

        assert(ISECT_MAGIC == sect->s_magic);
        while (NULL != (prop = TAILQ_FIRST(properties))) {
            assert(IPROP_MAGIC == prop->n_magic);
            TAILQ_REMOVE(properties, prop, n_link);
            free(prop);
        }

        if (docomments) {
            ICommentsList_t *comments = &sect->s_comments;
            IniComment_t *comment;

            while (NULL != (comment = TAILQ_FIRST(comments))) {
                assert(ICOMMENT_MAGIC == comment->c_magic);
                TAILQ_REMOVE(comments, comment, c_link);
                free(comment);
            }
        }
    }
}


/*  Function: iniSectionDel
 *      Section delete.
 *
 *  Parameters:
 *      ifile - INI file object.
 *      sect - Section object.
 *      root - *TRUE* permits deletion of the root section.
 *
 *  Returns:
 *      nothing.
 */
static void
iniSectionDel(IFILE *ifile, IniSection_t *sect, int root)
{
    if (sect) {
        iniSectionClr(sect, TRUE);

        if (sect == ifile->i_root) {
            if (root) {
                ifile->i_root = NULL;
            } else {
                sect = NULL;
            }
        }

        if (sect) {
            ISectionList_t *sections = &ifile->i_sections;

            if (sect->s_name) RB_REMOVE(_iniSectionTree, &ifile->i_lookup, sect);
            TAILQ_REMOVE(sections, sect, s_link);
            free(sect);
        }
    }
}


/*  Function: iniPropertyNew
 *      Property object creation.
 *
 *  Parameters:
 *      ifile - INI file object.
 *      key - Key buffer.
 *      klen - Length of key, in bytes.
 *      dat - Optional data buffer.
 *      dlen - Length of data, in bytes.
 *      line - Source line number, otherwise 0.
 *      comment - Optional eol-of-line comment buffer.
 *      clen - Comment line length, in bytes.
 *
 *  Returns:
 *      Property object, otherwise NULL.
 */
static IniProperty_t *
iniPropertyNewx(IniSection_t *sect, char delimiter,
        const char *key, unsigned klen, const char *data, unsigned dlen, const char *comment, int clen, unsigned line)
{
    IniProperty_t *prop;

    assert(key && klen > 0);
    assert((data && dlen > 0) || (0 == dlen));
    assert((comment && clen > 0) || (NULL == comment || 0 == clen));

    if (NULL != (prop =
            malloc(sizeof(IniProperty_t) + klen + dlen + clen + 3))) {
        /*
         *  < Property Object >
         *      [ Key Buffer  \0 ]
         *      [ Data Buffer \0 ]
         *      [ Comment Buffer \0 ]
         */
        char *n_key = (char *)(prop + 1);
        char *n_data = (char *)(n_key + klen + 1);
        char *n_comment = (char *)(n_data + dlen + 1);

        prop->n_magic   = IPROP_MAGIC;
        prop->n_delimiter = delimiter;
        prop->n_line    = line;
        prop->n_key     = (klen ? n_key : NULL);
        prop->n_data    = (dlen ? n_data : NULL);
        prop->n_comment = (clen ? n_comment : NULL);

        memcpy(n_key, (const char *)key, klen);
        n_key[klen] = 0;

        if (data) {
            memcpy(n_data, (const char *)data, dlen);
            n_data[dlen] = 0;
        }

        if (comment) {
            memcpy(n_comment, (const char *)comment, clen);
            n_comment[clen] = 0;
        }

        TAILQ_INSERT_TAIL(&sect->s_properties, prop, n_link);
        ++sect->s_count;
    }

    return prop;
}


static IniProperty_t *
iniPropertyNew(IniSection_t *sect, char delimiter,
        const char *key, unsigned klen, const char *data, unsigned dlen, unsigned line)
{
    return iniPropertyNewx(sect, delimiter, key, klen, data, dlen, NULL, 0, line);
}


/*  Function: iniCommentNew
 *      Single line comment object creation.
 *
 *  Parameters:
 *      ifile - INI file object.
 *      text - Comment contain.
 *      clen - Name length, in bytes.
 *      line - Source line number, otherwise 0.
 *
 *  Returns:
 *      Comment object, otherwise NULL.
 */
static IniComment_t *
iniCommentNew(ICommentsList_t *comments, const char *text, unsigned tlen, unsigned line)
{
    IniComment_t *comment;

    if (NULL != (comment =
            malloc(sizeof(IniComment_t) + tlen + 1))) {
        /*
         *  < Section Object >
         *      [ Comment Buffer \0 ]
         */
        char *c_text = (char *)(comment + 1);

        comment->c_magic = ICOMMENT_MAGIC;
        comment->c_text = c_text;
        comment->c_line = line;

        memcpy(c_text, (const char *)text, tlen);
        c_text[tlen] = 0;

        TAILQ_INSERT_TAIL(comments, comment, c_link);
    }
    return comment;
}


/*  Function: initGetc
 *      Stream character get.
 *
 *  Parameters:
 *      ifile - INI file object.
 *
 *  Returns:
 *      Character value otherwise -1 on error/end-of-file.
 */
static __CINLINE int
iniGetc(register IFILE *ifile)
{
    FILE *fp = ifile->i_file;
    int ch;

    if ((ch = ifile->i_unget) > 0) {
        ifile->i_unget = -1;
    } else {
        ch = fgetc(fp);
    }

    if ('\r' == ch) {                           /* CR[LF] */
        if ('\n' != (ch = fgetc(fp))) {
            if (ch > 0) {
                ifile->i_unget = ch;
            }
            return '\r';                        /* CR */
        }
        return '\n';                            /* CRLF */
    }
    return ch;
}


/*  Function: initUngetc
 *      Stream character unget.
 *
 *  Parameters:
 *      ifile - INI file object.
 *      ch - Character value.
 *
 *  Returns:
 *      nothing
 */
static __CINLINE void
iniUngetc(IFILE *ifile, int ch)
{
    assert(-1 == ifile->i_unget);
    assert(ch >= 0);
    ifile->i_unget = ch;
}


/*  Function: iniTrimr
 *      Trim trailing (right) whitespace.
 *
 *  Parameters:
 *      buffer - Buffer start.
 *      cursor - Cursor within buffer.
 *
 *  Returns:
 *      Resulting cursor address.
 */
static char *
iniTrimr(char *buffer, char *cursor)
{
    assert(cursor >= buffer);

    while (cursor > buffer) {                   /* trailing white-space */
        if (ISWHITE(cursor[-1])) {
            --cursor;
            continue;
        }
        break;
    }
    return cursor;
}


static void
iniError(IFILE *ifile, const char *msg)
{
    if (ifile->i_report) {
        (*ifile->i_report)(ifile, ifile->i_udata, ifile->i_line, msg);
    }
    ++ifile->i_errors;
}


/*  Function: iniGetx
 *      Configuration section retrieval.
 *
 *      Keys may (but need not) be grouped into arbitrarily named sections. The section
 *      name appears on a line by itself, in square brackets ([ and ]). All keys after
 *      the section declaration are associated with that section. There is no explicit
 *      "end of section" delimiter; sections end at the next section declaration, or
 *      the end of the file. Sections may not be nested.
 *
 *>         [section]
 *>         a=a
 *>         b=b
 *
 *  Parameters:
 *      ifile - INI file object.
 *
 *  Returns:
 *      Length of the retrieved line, with leading/trailing white-space removed,
 *      otherwise -1 on error/end-of-file.
 */
static int
iniGetx(IFILE *ifile, IniSection_t **iSect)
{
    char *buffer = ifile->i_buffer,             /* working buffer */
            *cursor = buffer, *end = buffer + (IFILE_LINEMAX - 1);

    int done = 0;                               /* processing done (1 = success, -1 = EOF) */
    int section = 0;                            /* ']' encountered */
    int c;

#define BUFPUSH(__c)        (void)(cursor < end ? *cursor++ = (char)(__c) : (char)(-1))

    do {
        c = iniGetc(ifile);
        switch(c) {
        case EOF:           /* eof */
            done = -1;
            break;

        case '\n':          /* eol */
            done = 1;
            break;

        case '\"':          /* double-quote, quotes next N characters */
            if (0 == section) {
                BUFPUSH('\"');
                do {                            /* special quote processing loop */
                    if ((c = iniGetc(ifile)) < 0) {
                        done = -1;
                    } else if ('\"' == c) {
                        break;
                    } else {
                        if ('\\' == c)          /* backslash */
                            if ((c = iniGetc(ifile)) < 0) {
                                done = -1;
                                break;
                            }
                        BUFPUSH(c);
                    }
                } while (!done);
                BUFPUSH('\"');
            }
            break;

        case ']':           /* equal */
            ++section;
            break;

        default:
            if (0 == section) {
                if (cursor == buffer) {         /* leading white-space */
                    if (! ISWHITE(c)) {
                        BUFPUSH(c);
                    }
                } else {
                    BUFPUSH(c);
                }
            }
            break;
        }
    } while (!done);

    cursor  = iniTrimr(buffer, cursor);
    *cursor = '\0';

    if (cursor > buffer) {
        const unsigned buflen = (unsigned)(cursor - buffer);
        IniSection_t *sect = iniSectionFnd(ifile, buffer, buflen);

        if (NULL == (*iSect = sect)) {
            *iSect = iniSectionNew(ifile, buffer, buflen, ifile->i_line);

        } else if (0 == (IFILE_DUPLICATES & ifile->i_flags)) {
            iniError(ifile, E_SECTION_DUPLICATE);
        }

        if (1 != section) {
            iniError(ifile, E_SECTION_MISSING_DELIMITER);
        }
        return buflen;
    }

    if (0 == c) {
        iniError(ifile, (0 == section ? E_SECTION_MISSING_DELIMITER : E_SECTION_EMPTY_NAME));
        return 0;
    }
    return -1;

#undef  BUFPUSH
}


/*  Function: iniGetl
 *      Configuration property retrieval.
 *
 *    Elements::
 *
 *      The basic element contained in an INI file is the key or
 *      property. Every key has a name and a value, delimited by an
 *      equals sign (=). The name appears to the left of the equals
 *      sign.
 *
 *>         name=value
 *
 *    Comments::
 *
 *      Semicolons (;) at the beginning of the line indicate a
 *      comment. Comment lines are ignored, unless running
 *      IFILE_COMMENTS mode.
 *
 *>         ; comment text
 *
 *      In extended mode both ';' and hash (#) at the beginning of the
 *      line indicate a comment. In additional, extended mode end of
 *      comments are introduced by a leading double hash (##).
 *
 *>         name=value      ## comment text
 *
 *      Note!:
 *      Absolute comment positions are not retained but are associated
 *      with the section. Upon export these comments shall be echoed
 *      prior to any properties within the section.
 *
 *    Quoted values::
 *
 *      Values can be quoted, using double quotes, this allows for
 *      explicit declaration of white-space, and/or for quoting of
 *      special characters (equals, semicolon, etc.). Resulting line
 *      shall be returned with the quotation marks removed.
 *
 *    Whitespace::
 *
 *      Leading and trailing white-space around the property 'name'
 *      and 'value' shall be removed.
 *
 *    Escape characters::
 *
 *      Under extended mode, several escape character operations are
 *      supported, denoted by a leading backslash (\).
 *
 *      All escape sequences consist of two, the first of which is the
 *      backslash, the following character determine the
 *      interpretation of the escape sequence. The following sequences
 *      escapes are recognized; unknown sequences are ignored.
 *
 *(start table,format=nd)
 *        [Sequence     [Description                                 ]
 *      ! \\            \ (backslash, escaping the escape character)
 *      ! \'            Single quote.
 *      ! \"            Double quote.
 *      ! \a            Bell or audible alert.
 *      ! \b            Backspace.
 *      ! \t            Tab character.
 *      ! \r            Carriage return.
 *      ! \n            Line feed.
 *      ! \e            Escape (\027).
 *      ! \[            Open square bracket.
 *      ! \;            Semicolon.
 *      ! \#            Number sign.
 *      ! \=            Equals sign.
 *(end)
 *
 *      A backslash (\) may also introduce a "line continuation",
 *      where a backslash followed immediately by EOL (end-of-line)
 *      causes the line break to be ignored, and the "logical line" to
 *      be continued on the next actual line from the configuration
 *      file.
 *
 *  Parameters:
 *      ifile - INI file object.
 *      special - Address of special character within the buffer,
 *          being either comment (; or #) or equals (=) within line.
 *
 *  Returns:
 *      Length of the retrieved line, with leading/trailing
 *      white-space removed, otherwise -1 on error/end-of-file.
 */
static int
iniGetl(IFILE *ifile, const char **special, const char **comment)
{
    const unsigned flags = ifile->i_flags;
    char *buffer = ifile->i_buffer,             /* working buffer */
            *cursor, *end;                      /* and associated cursors */

    int done = 0;                               /* processing done (1 = success, -1 = EOF) */
    int parts = 0;                              /* '=' or ':' encountered */
    int ch;                                     /* character */

#define BUFRESET()          cursor = buffer, end = buffer + (IFILE_LINEMAX - 1)
#define BUFPUSH(__c)        (void)(cursor < end ? *cursor++ = (char)(__c) : (char)(-1))

    *special = NULL, *comment = NULL;
    BUFRESET();

    do {
        ch = iniGetc(ifile);
next:;  switch(ch) {
        case EOF:           /* eof */
            done = -1;
            break;

        case '\n':          /* eol */
            done = 1;
            break;

        case '\"':          /* double-quote, quotes next N characters */
            if ((IFILE_QUOTED|IFILE_QUOTES) & flags) {

                if (IFILE_QUOTES & flags) {
                    BUFPUSH('\"');
                }

                do {                            /* special quote processing loop */
                    if ((ch = iniGetc(ifile)) < 0) {
                        iniError(ifile, E_PROPERTY_UNCLOSED_QUOTE);
                        done = -1;
                    } else if ('\"' == ch) {
                        break;
                    } else {
                        if ('\\' == ch) {       /* backslash */
                            if ((ch = iniGetc(ifile)) < 0) {
                                done = -1;
                                break;
                            }
                        }
                        BUFPUSH(ch);
                    }
                } while (!done);

                if (IFILE_QUOTES & flags) {
                    BUFPUSH('\"');
                }

            } else {
                BUFPUSH('\"');
            }
            break;

        case ';':           /* comments */
            if (IFILE_STANDARD & flags) {
                if (cursor == buffer) {

                    *special = cursor;
                    BUFPUSH(';');

                    do {                        /* line comment */
                        if ((ch = iniGetc(ifile)) < 0) {
                            done = -1;
                        } else if ('\n' == ch) {
                            done = 1;
                        } else {
                            BUFPUSH(ch);
                        }
                    } while (!done);

                } else if ((IFILE_STANDARDEOL & flags) && *special) {

                    *comment = cursor =
                        iniTrimr(buffer, cursor);

                    do {                        /* eol line comment, consume */
                        if ((ch = iniGetc(ifile)) < 0) {
                            done = -1;
                        } else if ('\n' == ch) {
                            done = 1;
                        } else {
                            BUFPUSH(ch);
                        }
                    } while (!done);

                } else {
                    BUFPUSH(ch);
                }

            } else {
                BUFPUSH(ch);
            }
            break;

        case '\\':          /* backslash, quotes next character */
            if (IFILE_BACKSLASH & flags) {
                if ((ch = iniGetc(ifile)) < 0) {
                    done = -1;
                } else if ('\n' == ch) {
                    if (parts) {
                        ++ifile->i_line;        /* newline continuation */
                    } else {
                        done = 1;
                    }
                } else {
                    switch (ch) {
                    case '\\': ch = '\\'; break;
                    case '\'': ch = '\''; break;
                    case '\"': ch = '\"'; break;
                    case 'a':  ch = '\a'; break;
                    case 'b':  ch = '\b'; break;
                    case 't':  ch = '\t'; break;
                    case 'r':  ch = '\r'; break;
                    case 'n':  ch = '\n'; break;
                    case 'e':  ch = 0x1b; break;
                    case '[':  ch = '[';  break;
                    case ';':  ch = ';';  break;
                    case '#':  ch = '#';  break;
                    case '=':  ch = '=';  break;
                    case ':':  ch = ':';  break;
                    default:
                        /*XXX: unicode and hex*/
                        ch = 0;
                        break;
                    }
                    if (ch > 0) BUFPUSH(ch);
                }
            } else {
                BUFPUSH(ch);
            }
            break;

        case '#':           /* extended comments */
            if (IFILE_EXTENDED & flags) {
                if (cursor == buffer) {

                    *special = cursor;
                    BUFPUSH('#');

                    do {                        /* line comment */
                        if ((ch = iniGetc(ifile)) < 0) {
                            done = -1;
                        } else if ('\n' == ch) {
                            done = 1;
                        } else {
                            BUFPUSH(ch);
                        }
                    } while (!done);

                } else if (IFILE_EXTENDEDEOL & flags) {

                    if ((ch = iniGetc(ifile)) < 0) {
                        BUFPUSH('#');
                        done = -1;

                    } else if ('#' != ch) {
                        BUFPUSH('#');
                        BUFPUSH(ch);

                    } else {

                        *comment = cursor =
                            iniTrimr(buffer, cursor);

                        do {                    /* eol line comment, consume */
                            if ((ch = iniGetc(ifile)) < 0) {
                                done = -1;
                            } else if ('\n' == ch) {
                                done = 1;
                            } else {
                                BUFPUSH(ch);
                            }
                        } while (!done);
                    }

                } else {
                    BUFPUSH(ch);
                }

            } else {
                BUFPUSH(ch);
            }
            break;

        case '=':           /* delimiter(s) */
        case ':':

            if (('=' == ch && (IFILE_COLON != (IFILE_EQUALCOLON & flags))) ||
                (':' == ch && (IFILE_COLON & flags)) ) {

                if (!*special && 1 == ++parts) {

                    while (cursor > buffer && ISWHITE(cursor[-1])) {
                        --cursor;               /* trailing white-space */
                    }
                    *special = cursor;
                    BUFPUSH(ch);

                    do {                        /* leading white-space */
                        if ((ch = iniGetc(ifile)) < 0) {
                            done = -1;
                        } else if ('\n' == ch) {
                            done = 1;
                        }
                    } while (!done && ISWHITE(ch));
                    if (! done) {
                        goto next;              /* reparse closing character */
                    }
                    break;
                }
            }
            /*FALLTHRU*/

        default:
            if (cursor == buffer) {             /* leading white-space */
                if (! ISWHITE(ch)) {
                    BUFPUSH(ch);
                }
            } else {
                BUFPUSH(ch);
            }
            break;
        }
    } while (!done);

    cursor  = iniTrimr(buffer, cursor);
    *cursor = '\0';

    return ((cursor == buffer && ch < 0) ? -1 : (int)(cursor - buffer));

#undef  BUFPUSH
}


static int
iniLoad(IFILE *ifile)
{
    const unsigned flags = ifile->i_flags;
    const char delimiter =                      /* key/value delimiter */
        (IFILE_COLON == ((IFILE_COLON|IFILE_EQUALCOLON) & flags) ? ':' : '=');

    IniSection_t *sect = ifile->i_root;         /* active section */
    int ret;

ED_TRACE(("LOD: %s:\n", ifile->i_fname))
    for (;;) {
        if ((ret = iniGetc(ifile)) >= 0) {

            ++ifile->i_line;

            if ('[' == ret) {                   /* sections */
                ret = iniGetx(ifile, &sect);

ED_TRACE(("SEC: [%s]\n", ifile->i_buffer))

            } else {                            /* properties/comments */
                const char *special, *comment;

                iniUngetc(ifile, ret);

                if ((ret = iniGetl(ifile, &special, &comment)) > 0) {
                    const char *buffer = ifile->i_buffer;

                    if (special) {              /* key=value */
                        if ('=' == *special || ':' == *special) {
                            const int
                                klen = (int)(special - buffer),
                                dlen = (int)(comment ? (comment - special) - 1 : (ret - klen) - 1);

                            if (comment && (IFILE_COMMENTS & flags)) {
                                const int clen = (int)((buffer + ret) - comment);

                                iniPropertyNewx(sect, *special,
                                    buffer, klen, special + 1, dlen, comment, clen, ifile->i_line);

ED_TRACE(("PRO: %.*s%c%.*s\t## %s\n", \
            klen, buffer, *special, dlen, special + 1, comment))

                            } else {
                                iniPropertyNew(sect, *special,
                                    buffer, klen, special + 1, dlen, ifile->i_line);

ED_TRACE(("PRO: %.*s%c%.*s\n", \
            klen, buffer, *special, dlen, special + 1))

                            }
                                                /* line comments */
                        } else if (IFILE_COMMENTS & flags) {
                            iniCommentNew(&sect->s_comments, buffer, ret, ifile->i_line);

ED_TRACE(("CMT: %.*s\n", ret, buffer))
                        }

                    } else {                    /* key="" */
                        iniPropertyNew(sect, delimiter, buffer, ret, NULL, 0, ifile->i_line);

ED_TRACE(("KEY: %.*s\n", ret, buffer))
                    }
                }
            }
        }

        if (ret < 0) {
            if (feof(ifile->i_file)) {
                ifile->i_flags |= IFILE_EOF;
            }
            break;
        }
    }

ED_TRACE(("END: ----\n"))
    return 0;
}


/*  Function: IniOpen
 *      INI file open.
 *
 *  Parameters:
 *      ifile - INI file object.
 *      filename - Filename content to be loaded.
 *      flags - Load flags.
 *      reporter - Error message reporter.
 *      udata - User data.
 *
 *  Returns:
 *      0 on success, otherwise -1 on error.
 */
IFILE *
IniOpen(const char *filename, unsigned flags)
{
    return IniOpenx(filename, flags, NULL, NULL);
}


IFILE *
IniOpenx(const char *filename, unsigned flags, IniReporter_t reporter, void *udata)
{
    const size_t filenamelen = (filename ? strlen(filename) : 0);
    IFILE *ifile;

    if (NULL != (ifile =
            calloc(sizeof(IFILE) + filenamelen + IFILE_LINEMAX + 2, 1))) {
        /*
         *  IFILE
         *    <filename \0>
         *    <buffer \0>
         */
        ifile->i_magic  = IFILE_MAGIC;
        ifile->i_flags  = flags;
        ifile->i_fname  = (char *)(ifile + 1);
        if (filename) {
            memcpy((char *)(ifile + 1), filename, filenamelen);
        }
        ifile->i_file   = NULL;
        ifile->i_line   = 0;

        TAILQ_INIT(&ifile->i_sections);
        RB_INIT(&ifile->i_lookup);

        ifile->i_report = reporter;
        ifile->i_udata  = udata;

        ifile->i_errors = 0;
        ifile->i_modifications = 0;

        ifile->i_buffer = (char *)(ifile->i_fname + filenamelen + 2);
        ifile->i_getc   = NULL;
        ifile->i_unget  = -1;

        if (NULL != (ifile->i_root = iniSectionNew(ifile, NULL, 0, 1))) {

            if ((filename && NULL != (ifile->i_file = fopen(filename, "r"))) ||
                    ((IFILE_CREATE & flags) && (NULL == filename || ENOENT == errno))) {

                if (ifile->i_file) {
                    iniLoad(ifile);
                    fclose(ifile->i_file);
                    ifile->i_file = NULL;
                }

                ifile->i_report = NULL;
                ifile->i_udata = NULL;
                return ifile;
            }
        }

        IniClose(ifile);
    }
    return (IFILE *)NULL;
}


const char *
IniFilename(IFILE *ifile)
{
    if (ifile) {
        assert(IFILE_MAGIC == ifile->i_magic);
        return ifile->i_fname;
    }
    return NULL;
}


int
IniErrors(IFILE *ifile)
{
    if (ifile) {
        assert(IFILE_MAGIC == ifile->i_magic);
        return ifile->i_errors;
    }
    return -1;
}


int
IniModified(IFILE *ifile)
{
    if (ifile) {
        assert(IFILE_MAGIC == ifile->i_magic);
        return ifile->i_modifications;
    }
    return -1;
}


/*  Function: IniExport
 *      Export the loaded INI content to the specified file.
 *
 *  Parameters:
 *      ifile - INI file object.
 *      filename - Filename to where the content is be written.
 *
 *  Returns:
 *      0 on success, otherwise -1 on error.
 */
int
IniExport(IFILE *ifile, const char *filename)
{
    if (ifile && filename) {
        const unsigned flags = ifile->i_flags;
        const char *eolseperator = ((IFILE_EXTENDED & flags) ? "##" : ";");

        ISectionList_t *sections = &ifile->i_sections;
        IniSection_t *sect;
        int dobreak = 0;
        FILE *fd;

        assert(IFILE_MAGIC == ifile->i_magic);
        if (NULL != (fd = fopen(filename, "w"))) {

ED_TRACE(("EXP: %s:\n", filename))

            TAILQ_FOREACH(sect, sections, s_link) {
                const ICommentsList_t *comments = &sect->s_comments;
                const IPropertiesList_t *properties = &sect->s_properties;

                if (dobreak) {
                    fprintf(fd, "\n");
                    dobreak = 0;
                }

                assert(ISECT_MAGIC == sect->s_magic);
                if (sect->s_name) {
                    fprintf(fd, "[%s]\n", sect->s_name);

ED_TRACE(("SEC: %s:\n", sect->s_name))
                    dobreak = 1;
                }

                if (TAILQ_FIRST(comments)) {
                    const char *leading = ((IFILE_EXTENDED & flags) ? "#" : ";");
                    const IniComment_t *comment;

                    TAILQ_FOREACH(comment, comments, c_link) {
                        const char *text = comment->c_text;
                        int ch0 = *text;

                        assert(ICOMMENT_MAGIC == comment->c_magic);
                        fprintf(fd, "%s%s\n",
                            (';' != ch0 && '#' != ch0 ? leading : ""), text);

ED_TRACE(("CMT: %s\n", text))
                    }
                    dobreak = 1;
                }

                if (TAILQ_FIRST(properties)) {
                    const IniProperty_t *prop;

                    TAILQ_FOREACH(prop, properties, n_link) {
                        const int len =         /* property */
                            fprintf(fd, (((IFILE_QUOTED|IFILE_QUOTES) & flags) ? "%s%c%s\"%s\"" : "%s%c%s%s"),
                                prop->n_key, prop->n_delimiter, (':' == prop->n_delimiter ? " " : ""),
                                    (prop->n_data ? prop->n_data : ""));

                        if (prop->n_comment) {  /* eol-of-line comment */
                            fprintf(fd, " %*s%s%s%s", (len > 44 ? 4 : len - 48), "",
                                eolseperator, (ISWHITE(*prop->n_comment) ? "" : " "), prop->n_comment);

ED_TRACE(("PRO: %s%c%s\t## %s\n", \
            prop->n_key, prop->n_delimiter, (prop->n_data ? prop->n_data : ""), prop->n_comment))
                        } else {

ED_TRACE(("PRO: %s%c%s\n", \
            prop->n_key, prop->n_delimiter, (prop->n_data ? prop->n_data : "")))
                        }

                        assert(IPROP_MAGIC == prop->n_magic);
                        fprintf(fd, "\n");
                    }
                    dobreak = 1;
                }
            }

ED_TRACE(("END: ----\n"))

            ifile->i_modifications = 0;
            fclose(fd);
            return 0;
        }
    }
    return -1;
}


/*  Function: IniClose
 *      Close the loaded INI object.
 *
 *  Parameters:
 *      ifile - INI file object.
 *
 *  Returns:
 *      nothing
 */
void
IniClose(IFILE *ifile)
{
    if (ifile) {
        ISectionList_t *sections = &ifile->i_sections;
        IniSection_t *sect;

        assert(IFILE_MAGIC == ifile->i_magic);
        while (NULL != (sect = TAILQ_FIRST(sections))) {
            iniSectionDel(ifile, sect, TRUE);
        }

        ifile->i_magic = 0;
        free((void *)ifile);
    }
}


/*  Function: IniFirst
 *      Retrieve the first property.
 *
 *  Parameters:
 *      ifile - INI file object.
 *      section - Optional section, "" matches the root section.
 *
 *  Returns:
 *      Cursor address, otherwise NULL.
 */
IFILECursor *
IniFirst(IFILE *ifile, const char *section)
{
    if (ifile) {
        ISectionList_t *sections = &ifile->i_sections;
        IniSection_t *sect;

        assert(IFILE_MAGIC == ifile->i_magic);
        if (NULL != (sect =
                (section ? iniSectionFnd(ifile, section, (unsigned)strlen(section)) : TAILQ_FIRST(sections)))) {

            IPropertiesList_t *properties = &sect->s_properties;
            IniProperty_t *prop = TAILQ_FIRST(properties);

            assert(ISECT_MAGIC == sect->s_magic);
            if (NULL == prop) {                 /* iterate sections */
                while (NULL != (sect = TAILQ_NEXT(sect, s_link))) {
                    properties = &sect->s_properties;

                    assert(ISECT_MAGIC == sect->s_magic);
                    if (NULL != (prop = TAILQ_FIRST(properties))) {
                        break;
                    }
                }
            }

            if (sect && prop) {
                IFILECursor *icursor = &ifile->i_cursor;

                assert(IPROP_MAGIC == prop->n_magic);
                icursor->magic = IFILECURSOR_MAGIC;
                icursor->__w[IFILE_IFILE] = ifile;
                icursor->__w[IFILE_ISECT] = (NULL == section ? sect : NULL);
                icursor->__w[IFILE_IPROP] = prop;
                icursor->sect = sect->s_name;
                icursor->key  = prop->n_key;
                icursor->data = prop->n_data;
                return icursor;
            }
        }
    }
    return NULL;
}


/*  Function: IniNext
 *      Retrieve the next property.
 *
 *  Parameters:
 *      icursor - INI file cursor.
 *
 *  Returns:
 *      Cursor address, otherwise NULL.
 */
IFILECursor *
IniNext(IFILECursor *icursor)
{
    if (icursor) {
        IFILE *ifile = icursor->__w[IFILE_IFILE];

        assert(IFILECURSOR_MAGIC == icursor->magic);
        if (ifile) {
            IniSection_t *sect = icursor->__w[IFILE_ISECT];
            IniProperty_t *prop = icursor->__w[IFILE_IPROP];

            assert(IFILE_MAGIC == ifile->i_magic);
            prop = TAILQ_NEXT(prop, n_link);    /* iterate properties */

            if (NULL == prop && sect) {         /* iterate sections */
                assert(ISECT_MAGIC == sect->s_magic);
                while (NULL != (sect = TAILQ_NEXT(sect, s_link))) {
                    IPropertiesList_t *properties = &sect->s_properties;

                    if (NULL != (prop = TAILQ_FIRST(properties))) {
                        break;
                    }
                }
            }

            if (prop) {
                assert(IPROP_MAGIC == prop->n_magic);
                icursor->__w[IFILE_ISECT] = sect;
                icursor->__w[IFILE_IPROP] = prop;
                if (sect) {                     /* update section */
                    icursor->sect = sect->s_name;
                }
                icursor->key  = prop->n_key;
                icursor->data = prop->n_data;
                return icursor;
            }
        }
    }
    return NULL;
}


/*  Function: IniQuery
 *      Query a property value.
 *
 *  Parameters:
 *      icursor - INI file cursor.
 *      setion - Section name.
 *      key - Key value.
 *      prev - Previous value address, otherwise NULL.
 *
 *  Returns:
 *      Value address, otherwise NULL.
 */
static __CINLINE int
sectionMatch(const IniSection_t *sect, const char *section)
{
    if (sect->s_name) {
        if (section && 0 == str_icmp(sect->s_name, section)) {
            return 1;                           // matching section.
        }
    } else {
        if (NULL == section || !*section) {
            return 1;                           // NULL or empty string.
        }
    }
    return 0;
}


const char *
IniQuery(IFILE *ifile, const char *section, const char *key, const void *prev)
{
    assert(section);
    assert(key && *key);

    if (ifile) {
        ISectionList_t *sections = &ifile->i_sections;
        IniSection_t *sect = TAILQ_FIRST(sections);

        assert(IFILE_MAGIC == ifile->i_magic);
        TAILQ_FOREACH(sect, sections, s_link) {
            assert(ISECT_MAGIC == sect->s_magic);

            if (sectionMatch(sect, section)) {
                IPropertiesList_t *properties = &sect->s_properties;
                IniProperty_t *prop;

                TAILQ_FOREACH(prop, properties, n_link) {
                    assert(IPROP_MAGIC == prop->n_magic);

                    if (prop->n_data &&
                            prop->n_data != prev && 0 == str_icmp(prop->n_key, key)) {
                        return prop->n_data;
                    }
                }
            }
        }
    }
    return NULL;
}


/*  Function: IniPush
 *      Add/update a property.
 *
 *  Parameters:
 *      icursor - INI file cursor.
 *      section - Section name.
 *      key - Optional key buffer, if omitted assumed to be comment push.
 *      value - Value buffer, not optional when 'key' is given.
 *      comment - Optional comment, if -1 the current comment (if any) is retained.
 *      unique - TRUE if unique otherwise non-unique.
 *
 *  Returns:
 *      0 on success, otherwise -1 on error.
 */
int
IniPush(IFILE *ifile, const char *section, const char *key,
            const char *value, const char *comment, int unique)
{
    assert(section);
    assert((key && *key) || comment);
    assert((key && value) || comment);

    if (ifile) {
        const unsigned sectionlen = (unsigned)strlen(section);
        IniSection_t *sect;

        assert(IFILE_MAGIC == ifile->i_magic);
        if (NULL != (sect = iniSectionFnd(ifile, section, sectionlen)) ||
                NULL != (sect = iniSectionNew(ifile, section, sectionlen, 0))) {

            const char delimiter =              /* key/value delimiter */
                (IFILE_COLON == ((IFILE_COLON|IFILE_EQUALCOLON) & ifile->i_flags) ? ':' : '=');

            assert(ISECT_MAGIC == sect->s_magic);

            if (key) {                          /* property */
                IPropertiesList_t *properties = &sect->s_properties;
                IniProperty_t *prop = NULL, *nprop;

                if (value) {
                    if (unique) {
                        TAILQ_FOREACH(prop, properties, n_link) {
                            assert(IPROP_MAGIC == prop->n_magic);
                            if (0 == str_icmp(prop->n_key, key)) {
                                break;
                            }
                        }
                    }

                    if (((const char *)-1) == comment && prop && prop->n_comment) {
ED_TRACE(("PRO+ %s%c%s\t##%s\n", key, delimiter, value, comment))
                        nprop = iniPropertyNewx(sect, delimiter, key, (unsigned)strlen(key),
                                    value, (unsigned)strlen(value), prop->n_comment, (unsigned)strlen(prop->n_comment), 0);

                    } else if (comment && *comment) {
ED_TRACE(("PRO= %s%c%s\t##%s\n", key, delimiter, value, comment))
                        nprop = iniPropertyNewx(sect, delimiter, key, (unsigned)strlen(key),
                                    value, (unsigned)strlen(value), comment, (unsigned)strlen(comment), 0);

                    } else {
ED_TRACE(("PRO= %s%c%s\n", key, delimiter, value))
                        nprop = iniPropertyNew(sect, delimiter, key, (unsigned)strlen(key),
                                    value, (unsigned)strlen(value), 0);
                    }

                    if (nprop) {
                        if (prop) {
                            TAILQ_REMOVE(properties, prop, n_link);
                            free(prop);
                        }
                        ++ifile->i_modifications;
                        return 0;
                    }
                }

            } else {
                if (comment) {                  /* comment */
ED_TRACE(("CMT+ %s\n", comment))
                    iniCommentNew(&sect->s_comments, comment, (unsigned)strlen(comment), 0);
                    ++ifile->i_modifications;
                    return 0;
                }
            }
            return -1;
        }
    }
    return -1;
}


/*  Function: IniRemove
 *      Remove a property/section.
 *
 *  Parameters:
 *      icursor - INI file cursor.
 *      section - Section name.
 *      key - Optional key value, if omitted the section properties are removed.
 *      keep - Section keep flag; if *TRUE* keep the section definitions,
 *          removing only the associated properties.
 *
 *  Returns:
 *      The number of properties removed, otherwise -1 on error.
 */
int
IniRemove(IFILE *ifile, const char *section, const char *key, int keep)
{
    assert(section);

    if (ifile) {
        const unsigned sectionlen = (unsigned)strlen(section);
        IniSection_t *sect;

        assert(IFILE_MAGIC == ifile->i_magic);
        if (NULL != (sect = iniSectionFnd(ifile, section, sectionlen))) {
            IPropertiesList_t *properties = &sect->s_properties;
            IniProperty_t *prop;
            int count = 0;

            assert(ISECT_MAGIC == sect->s_magic);
            if (key) {                          /* matching keys */
                TAILQ_FOREACH(prop, properties, n_link) {
                    assert(IPROP_MAGIC == prop->n_magic);

                    if (0 == str_icmp(prop->n_key, key)) {
                        TAILQ_REMOVE(properties, prop, n_link);
                        ++count;
                    }
                }

            } else {                            /* section */
                if (keep) {
                    iniSectionClr(sect, FALSE);
                } else {
                    iniSectionDel(ifile, sect, FALSE);
                }
            }

            ifile->i_modifications += count;
            return count;
        }
    }
    return -1;
}


#if defined(LOCAL_MAIN)
static void
IniDump(IFILE *ifile)
{
    ISectionList_t *sections = &ifile->i_sections;
    IniSection_t *sect;

    assert(IFILE_MAGIC == ifile->i_magic);
    printf("%s:\n", ifile->i_fname);
    TAILQ_FOREACH(sect, sections, s_link) {
        IPropertiesList_t *properties = &sect->s_properties;
        IniProperty_t *prop;

        assert(ISECT_MAGIC == sect->s_magic);
        if (sect->s_name) {
            printf("%3d: [%.*s]\n",
                sect->s_line, sect->s_nlen, sect->s_name);
        }

        TAILQ_FOREACH(prop, &sect->s_properties, n_link) {
            assert(IPROP_MAGIC == prop->n_magic);
            printf("%3d: <%s><%s>\n",
                prop->n_line, prop->n_key, prop->n_data);
        }
    }
    printf("\n");
}


static void
IniIterate(IFILE *ifile)
{
    IFILECursor *icursor;
    unsigned item = 0;

    printf("%s:\n", ifile->i_fname);
    if (NULL != (icursor = IniFirst(ifile, NULL))) {
        do {
            printf("%3u: [%s]-><%s><%s>\n", ++item,
                (icursor->sect ? icursor->sect : ""), icursor->key, (icursor->data ? icursor->data : ""));
        } while (NULL != (icursor = IniNext(icursor)));
    }
    printf("\n");
}


static void
iniReporter(IFILE *ifile, void *udata, unsigned line, const char *msg)
{
    printf("INI: %s\n", msg);
}


void
main(void)
{
    const char *newSection = "Section X";

    {   IFILE *ifile;

        ifile = IniOpen("ini.cfg", IFILE_STANDARD);
        IniDump(ifile);
        IniIterate(ifile);
        IniClose(ifile);
    }

    {   IFILE *ifile;

        ifile = IniOpenx("ini.cfgx",
            IFILE_STANDARD|IFILE_EXTENDED|IFILE_COMMENTSEOL|IFILE_DUPLICATES|IFILE_QUOTED|IFILE_COMMENTS, iniReporter, NULL);
        IniDump(ifile);
        IniIterate(ifile);
        IniPush(ifile, newSection, "zzz", "9999", "comment 1", 0);
        IniPush(ifile, newSection, "zzz", "8888", "comment 2", 0);
        IniPush(ifile, newSection, "yyy", "7777", "comment 3", 1);
        IniPush(ifile, newSection, "yyy", "6666", "comment 4", 1);
        IniRemove(ifile, newSection, "yyy", 0);
        IniIterate(ifile);
        IniExport(ifile, "iniout.cfg");
        IniClose(ifile);
    }
}
#endif /*LOCAL_MAIN*/

/*end*/
