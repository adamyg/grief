#include <edidentifier.h>
__CIDENT_RCSID(gr_syntaxdfa_c,"$Id: syntaxdfa.c,v 1.30 2015/02/21 22:47:27 ayoung Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: syntaxdfa.c,v 1.30 2015/02/21 22:47:27 ayoung Exp $
 * Deterministic Finite Automata (DFA) based syntax highlighting.
 *
 *
 * Copyright (c) 1998 - 2015, Adam Young.
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

#include <editor.h>
#include <edconfig.h>
#include <time.h>
#include <tailqueue.h>
#include <libstr.h>                             /* str_...()/sxprintf() */

#include "color.h"                              /* ATTR_... */
#include "debug.h"
#include "echo.h"                               /* ewprintf() */
#include "eval.h"                               /* get_str() */
#include "regdfa.h"                             /* regular expression DFA engine */
#include "syntax.h"                             /* syntax basic functionality */

#define DFARULES_MAX            128             /* rules */
#define DFAGROUP_MAX            32              /* sub-groups */
#define DFASTATES_MAX           32              /* states per limit */

struct dfastate {
    const char *            name;
    unsigned                regno;
    struct regdfa *         regex;
    struct dfarule const *  regst[DFASTATES_MAX];
};

struct dfarule {
    const char *            name;
    const char *            group;
    const char *            pattern;
    const char *            contains;
    unsigned                flags;
    int                     stateno;
    int                     colour;
    int                     writex;
    struct dfastate *       state;
};

typedef struct {
    struct SyntaxDriver     dfa_driver;
    struct SyntaxTable *    dfa_owner;
    time_t                  dfa_built;
    struct dfastate *       dfa_base;
    unsigned                dfa_groups;

    unsigned                dfa_grpcnt;
    const char *            dfa_grpnames[DFAGROUP_MAX];
    struct dfastate *       dfa_grpst[DFAGROUP_MAX];
    struct dfastate *       dfa_comment;
    struct dfastate *       dfa_string;

    unsigned                dfa_count;
    struct dfarule          dfa_rules[DFARULES_MAX];
} SyntaxDFA_t;

static const struct hloption *  hloption_lookup(const char *name, int length);

static int                      syndfa_select(SyntaxTable_t *st, void *object);
static void                     syndfa_destroy(SyntaxTable_t *st, void *object);
static const LINECHAR *         syndfa_regex(SyntaxTable_t *st, const SyntaxDFA_t *dfa, const struct dfastate *state,
                                        int normal, unsigned offset, int *rvalue, const LINECHAR *cursor, const LINECHAR *end);
static int                      syndfa_write(SyntaxTable_t *st, void *object, const LINECHAR *cursor, unsigned offset, const LINECHAR *end);
static SyntaxDFA_t *            syndfa_resolve(int argi, int create, SyntaxTable_t **st);
static void                     syndfa_rule(SyntaxDFA_t *syntax, const char *pattern, const char *name, int namelen,
                                        const char *group, int grouplen, const char *contains, int containslen, unsigned flags, int colour);
static struct dfarule *         syndfa_find(SyntaxDFA_t *syntax, const char *name, int namelen);
static void                     syndfa_make(SyntaxDFA_t *syntax);
static struct dfastate *        syndfa_group(SyntaxDFA_t *syntax, const char *group);

static void                     syndfa_save(SyntaxDFA_t *syntax, const char *name, int timestamp);
static int                      syndfa_load(SyntaxDFA_t *syntax, const char *name, int timestamp);

static struct hloption {
    const char *            f_name;
    unsigned                f_length;
    uint32_t                f_flag;

#define FLAG_ERROR              0x00000001
#define FLAG_DIRECTIVE          0x00000002
#define FLAG_KEYWORD            0x00000004
#define FLAG_WORD               0x00000008

#define FLAG_SPELL              0x00000100
#define FLAG_TODO               0x00000200
#define FLAG_MARKUP             0x00000400
#define FLAG_TAGS               0x00000800

#define FLAG_QUICK              0x00001000
#define FLAG_CONTAINED          0x00002000
#define FLAG_PREPROC            0x00004000
#define FLAG_SILENT             0x00008000

#define FLAG_NAME               0x00010000      /* name=<name> */
#define FLAG_GROUP              0x00020000      /* group=<group> */
#define FLAG_CONTAINS           0x00040000      /* contains=<name> */
#define FLAG_COLOR              0x00080000      /* color=<name> */

#define FLAG_BAD                0x10000000

} hiliteoptions[] = {
#define NFIELD(__x)     __x, (sizeof(__x) - 1)
    { NFIELD("keyword"),        FLAG_KEYWORD    },
    { NFIELD("word"),           FLAG_WORD       },
    { NFIELD("tags"),           FLAG_TAGS       },
    { NFIELD("directive"),      FLAG_DIRECTIVE  },
    { NFIELD("spell"),          FLAG_SPELL      },
    { NFIELD("todo"),           FLAG_TODO       },
    { NFIELD("markup"),         FLAG_MARKUP     },
    { NFIELD("preproc"),        FLAG_PREPROC    },
      { NFIELD("pp"),           FLAG_PREPROC    },
    { NFIELD("quick"),          FLAG_QUICK      },
    { NFIELD("name"),           FLAG_NAME       },
    { NFIELD("group"),          FLAG_GROUP      },
    { NFIELD("contains"),       FLAG_CONTAINS   },
    { NFIELD("contained"),      FLAG_CONTAINED  },
    { NFIELD("silent"),         FLAG_SILENT     }
#undef  NFIELD
    };

static char START_DFA[] =       "[START-DFA]";
static char TIMESTAMP[] =           "TIMESTAMP=";
static char START_PATTERN[] =       "[START-PATTERN]";
static char END_PATTERN[] =         "[END-PATTERN]";
static char START_CACHE[] =         "[START-CACHE]";
static char START_GROUP[] =             "[START-GROUP]";
static char END_GROUP[] =               "[END-GROUP]";
static char END_CACHE[] =           "[END-CACHE]";
static char END_DFA[] =         "[END-DFA]";


/*  Function:           do_syntax_rule
 *      syntax_rule primitive.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 *
 *<<GRIEF>>
    Macro: syntax_rule - Define a syntax hilite rule.

        void
        syntax_rule(string pattern,
            string attribute, [int|string syntable])

    Macro Description:
        The 'syntax_rule()' primitive pushes a Deterministic Finite
        Automata (DFA) rule into the enhanced highlighting definition
        for the syntax table specified by 'syntable'.

        The rule is described by the regular expression contained
        within the string 'rule', and the associated 'attribute' is
        then applied against any matched constructs.

        These rules works alongside the basic syntax elements
        declared by <syntax_token> against the same syntax table.

        For example, the rules to highlight floating point numbers
        could be encoding as;

>       syntax_rule("[0-9]+\\.[0-9]*([Ee][-+]?[0-9]*)?[fFlL]?[iIjJ]?", "float");
>       syntax_rule("[0-9]+[Ee][-+]?[0-9]*[fFlL]?[iIjJ]?", "float");

        Note!:
        Consult the numerous bundled language mode definitions for
        working examples.

    Macro Parameters:
        pattern - Rule regular expression.

        attribute - Attribute specification.

        syntable - Optional syntax-table name or identifier, if
            omitted the current syntax table shall be referenced.

    Attribute Specification:

        The attribute specification takes the following form. None or
        comma separated options plus the associated colour attribute
        (see set_color).

>           [<option>[="....."] [, <option> ...] :] <attribute>

        Options::

            o word -                possible word.

            o keyword -             possible keyword.

            o tags -                possible symbol within tagdb.

            o directive -           possible preprocessor directive.

            o preproc/pp -          enter preprocessor mode.

            o quick -               quick expression evaluation.
                                    marks the regular expression for
                                    minimal closure, by default
                                    evaluation matches against the
                                    longest possible rule, quick
                                    short-circuits expression
                                    execution upon being matched,
                                    reducing the greedy nature of DFA
                                    regular expressions.

            o spell -               apply spell checks.

            o todo -                apply TODO checks.

            o markup -              apply markup checks.

            o silent -              silent regarding issues, for
                                    example non-existent children.

            o name=<name> -         rule name.

            o group=<grpname> -     group name, implied sub-rule.

            o color=<spec> -        color specification, implies the
                                    creation of the attribute if it
                                    does not exist.

            o contains=<rule> -     contains one or more rules.

            o contained -           is contained within another rule.

        Examples::

            Comment block, with both spelling and TODO token
            processing enabled.

>               "spell,todo:comment"

            Normal text, yet token may be either a keyword or
            directive.

>               "keyword,directive:normal"

    Rule Regular Expression:

        A regular expression is a pattern that the regular expression
        engine attempts to match in input text. A pattern consists of
        one or more character literals, operators, or constructs. For
        a brief introduction, see .NET Framework Regular Expressions.

        Each section in this quick reference lists a particular
        category of characters, operators, and constructs that you
        can use to define regular expressions:

     Character Escapes::

        The backslash character (\) in a regular expression indicates
        that the character that follows it either is a special
        character (as shown in the following table), or should be
        interpreted literally.

(start table,format=nd)
        [Anchor     [Description                                        ]

      ! \t          Tab.

      ! \n          Newline.

      ! \r          Return.

      ! \f          Formfeed.

      ! \a          Alarm (bell, beep, etc).

      ! \e          Escape (\027).

      ! \\          This escapes the meaning of a special.
(end)

     Character Classes::

        A character class matches any one of a set of characters.
        Character classes include the language elements listed in the
        following table.

(start table,format=nd)
        [Anchor     [Description                                        ]

      ! [...]       Matches any one character contained within the
                    character sequence.

      !  .          Match any single character except newline.

      ! \d          Same as [0-9].

      ! \x          Same as [a-fA-f0-9].

      ! \s          Same as [ \\t\\f].

      ! \w          Same as [a-zA-Z_0-9].
(end)

     Character Sequences::

        The conversion specification includes all subsequent characters
        in the format string up to and including the matching right
        square bracket (]).

        The characters between the square brackets (the scanlist)
        comprise the scanset, unless the character after the left
        square bracket is a circumflex (^), in which case the scanset
        contains all characters that do not appear in the scanlist
        between the circumflex and the right square bracket.

        If the conversion specification begins with "[]" or "[^]",
        the right square bracket is included in the scanlist and the
        next right square bracket is the matching right square
        bracket that ends the conversion specification; otherwise the
        first right square bracket is the one that ends the
        conversion specification.

        If a hyphen character (-) is in the scanlist and is not the
        first character, nor the second where the first character is
        a circumflex (^), nor the last character, it indicates a
        range of characters to be matched. To include a hyphen, make
        it the last character before the final close bracket. For
        instance, `[^]0-9-]' means the set `everything except close
        bracket, zero through nine, and hyphen'.

        Within a bracket expression, the name of a character class
        enclosed in [: and :] stands for the list of all characters
        (not all collating elements!) belonging to that class.

            alnum  - An alphanumeric (letter or digit).
            alpha  - A letter.
            blank  - A space, tab or form-feed character.
            cntrl  - A control character.
            digit  - A decimal digit.
            graph  - A character with a visible representation.
            lower  - A lower-case letter.
            print  - An alphanumeric (same as alnum).
            punct  - A punctuation character.
            space  - A character producing white space in displayed text.
            upper  - An upper-case letter.
            word   - "word" character (alphanumeric plus "_").
            xdigit - A hexadecimal digit.

     Anchors::

        Anchors, or atomic zero-width assertions, cause a match to
        succeed or fail depending on the current position in the
        string, but they do not cause the engine to advance through
        the string or consume characters. The metacharacters listed
        in the following table are anchors.

(start table,format=nd)
        [Anchor     [Description                                        ]

      ! ^           If this is the first character of the regular
                    expression, it matches the beginning of the line.

      ! $           If this is the last character of the regular
                    expression, it matches the end of the line.

      ! \c          Anchor start of the matched text to the proceeding
                    token.
(end)

     Quantifiers::

        A quantifier specifies how many instances of the previous
        element must be present in the input string for a match to
        occur.

(start table,format=nd)
        [Anchor     [Description                                        ]

      ! *           Match the preceding character or range of characters
                    0 or more times.

      ! +           Match the preceding character or range of characters
                    1 or more times.

      ! ?           Match the preceding character or range of characters
                    0 or 1 times.
(end)

     Specials::

        Grouping constructs delineate subexpressions of a regular
        expression and typically capture substrings of an input string.

(start table,format=nd)
        [Anchor     [Description                                        ]

      ! |           This symbol is used to indicate where to separate
                    two sub regular expressions for a logical OR
                    operation.

      ! (..)        Group boundaries.

      ! \\Q..\\E    A section enclosed in these symbols it taken
                    literally. In side these sections, meta characters
                    and special symbols have no meaning. If a \\E needs
                    to appear in one of these sections, the \\ must be
                    escaped with \\.
(end)

    Macro Returns:
        nothing

    Macro Portability:
        A Grief extension.

    Macro See Also:
        create_syntax, attach_syntax, detach_syntax, inq_syntax,
            syntax_rule, syntax_build
 */
void
do_syntax_rule(void)            /* void (string pattern, string attribute, [int|string syntable]) */
{
    const char *pattern = get_xstr(1);
    const char *attribute = get_str(2);
    SyntaxDFA_t *syntax;
    unsigned flags = 0;
    int colour;

    if (NULL != (syntax = syndfa_resolve(3, 1, NULL)) && !syntax->dfa_built) {
        const char *end, *cursor = attribute;
        const char *name = NULL, *group = NULL, *contains = NULL;
        int namelen = 0, grouplen = 0, containslen = 0;

        /*
         *  determine end of options, allowing embedded strings.
         *
         *      [<option>[="....."] [, <option> ...] :] <attribute>
         */
        ED_TRACE2(("RULE=<%s>\n", cursor))
        if (NULL != (end = strchr(cursor, ':'))) {
            const char *quote, *endquote;
                                                /* skip quoted text */
            while (end && NULL != (quote = strchr(end + 1, '"')) && quote < end) {

                if (NULL != (endquote = strchr(quote + 1, '"'))) {
                    end = strchr(endquote + 1, ':');

                } else {
                    errorf("syntaxdfa: unbalanced quote '%s'", quote);
                    break;
                }
            }
        }

        /*
         *  process options (if any)
         */
        if (end) {                              /* options */
            while (cursor < end) {
                const char *delim = strchr(cursor, ','),
                        *argument = strchr(cursor, '=');
                int length, arglength = 0;

                if (NULL == delim) delim = end;
                if (argument > delim) argument = NULL;
                length = delim - cursor;

                cursor = str_trim(cursor, &length);
                if (argument) {
                    length = argument++ - cursor;
                    arglength = delim - argument;

                    if ('"' == *argument) {     /* strip quoted arguments */
                        --arglength, ++argument;
                        if (arglength > 0 && '"' == argument[arglength - 1]) {
                            --arglength;
                        }
                        argument = str_trim(argument, &arglength);
                    }
                }

                if (length > 0) {
                    const struct hloption *hloption;

                    if (NULL != (hloption = hloption_lookup(cursor, length))) {
                        const uint32_t flag = hloption->f_flag;

                        ED_TRACE2(("\t+<%.*s>=<%.*s>\n", length, cursor, arglength, argument))
                        if (flag >= FLAG_NAME) {
                            if (argument) {
                                if (FLAG_NAME == flag) {
                                    namelen = arglength;
                                    name = argument;

                                } else if (FLAG_GROUP == flag) {
                                    grouplen = arglength;
                                    group = argument;

                                } else if (FLAG_CONTAINS == flag) {
                                    containslen = arglength;
                                    contains = argument;

                            //  } else if (FLAG_COLOR == flag) {
                            //      colorlen = arglength;
                            //      color = len;
                                }

                            } else {
                                errorf("syntaxdfa: missing option argument '%.*s'", length, cursor);
                                flags |= FLAG_ERROR;
                            }
                        }
                        flags |= flag;

                    } else {
                        errorf("syntaxdfa: unknown attribute option '%.*s'", length, cursor);
                        flags |= FLAG_ERROR;
                    }
                }
                cursor = delim + 1;             /* next */
            }
        }
                                                /* decode colour and push rule */
        ED_TRACE2(("\t<%s>\n", end + 1))
        if ((colour = attribute_value(cursor)) >= 0) {
//TODO          || (colorspec && (colour = attribute_new(cursor, colorspec))) >= 0)

            if (0 == (flags & FLAG_ERROR)) {
                syndfa_rule(syntax, pattern, name, namelen, group, grouplen, contains, containslen, flags, colour);
            }
        } else /*if (!colorspec)*/ {
            if (flags) {
                errorf("syntaxdfa: missing attribute '%s', expected [<option>[,...]:]<attribute>", cursor);

            } else {
                errorf("syntaxdfa: unknown attribute '%s'", cursor);
            }
        }
    }
}


static const struct hloption *
hloption_lookup(const char *name, int length)
{
    if (NULL != (name = str_trim(name, &length)) && length > 0) {
        unsigned i;

        for (i = 0; i < (unsigned)(sizeof(hiliteoptions)/sizeof(hiliteoptions[0])); ++i)
            if (length == (int)hiliteoptions[i].f_length &&
                    0 == str_nicmp(hiliteoptions[i].f_name, name, length)) {
                return hiliteoptions + i;
            }
    }
    return NULL;
}


/*  Function:           syntax_build
 *      syntaxt_build primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: syntax_build - Build a syntax hiliting engine.

        void
        syntax_build([int timestamp],
                [string cache], [int|string syntable])

    Macro Description:
        The 'syntax_build()' primitive constructs the underlying
        Deterministic Finite Automata (DFA) from the current set of
        defined rule via the <syntax_rule> primitive.

    Macro Parameters:
        timestamp - Optional numeric time reference, utilised to
            time-stamp the cache (see time); should be modified upon
            each change to the DFA scheme.

        cache - Optional name of the cache file image.

        syntable - Optional syntax-table name or identifier, if
            omitted the current syntax table shall be referenced.

    Macro Returns:
        nothing

    Macro Portability:
        A Grief extension.

    Macro See Also:
        create_syntax, attach_syntax, detach_syntax, inq_syntax,
            syntax_rule, syntax_build
 */
void
do_syntax_build(void)           /* ([int timestamp], [string cache], [int|string syntable]) */
{
    const int timestamp = get_xinteger(1, -1);
    const char *cache = get_xstr(2);
    SyntaxTable_t *st = NULL;
    SyntaxDFA_t *syntax;

    if (NULL != (syntax = syndfa_resolve(3, 0, &st))) {
        int load = TRUE;

        if (NULL == cache) {
            cache = st->st_name;
        }

        if (timestamp > 0) {
            if (! syntax->dfa_built) {
                syndfa_load(syntax, cache, timestamp);
                if (syntax->dfa_built) {
                    load = FALSE;
                }
            }
        }

        if (!syntax->dfa_built) {
            ewprintf("Creating '%s' syntax table ...", st->st_name);
            syndfa_make(syntax);
            eclear();
        }

        if (timestamp > 0) {
            if (syntax->dfa_built) {
                if (load) {
                    syndfa_save(syntax, cache, timestamp);
                }
            }
        }
    }
}


static int
syndfa_select(SyntaxTable_t *st, void *object)
{
    const SyntaxDFA_t *dfa = (const SyntaxDFA_t *)object;

    __CUNUSED(st)
    if (dfa->dfa_built) {
        return 1;
    }
    return 0;
}


static void
syndfa_destroy(SyntaxTable_t *st, void *object)
{
    SyntaxDFA_t *dfa = (SyntaxDFA_t *)object;

    __CUNUSED(st)
    if (dfa) {
        if (dfa->dfa_built) {
            struct dfastate *state;
            unsigned g = dfa->dfa_grpcnt;

            while (g > 0) {
                if (NULL != (state = dfa->dfa_grpst[--g])) {
                    regdfa_destroy(state->regex);
                }
            }
            state = dfa->dfa_base;
            regdfa_destroy(state->regex);
            dfa->dfa_built = 0;
        }
    }
}


static const LINECHAR *
syndfa_writex(SyntaxTable_t *st, const struct dfarule *rule,
        const LINECHAR *start, const LINECHAR *end, int colour)
{
    if (FLAG_MARKUP & rule->flags) {
        /*
         *  If *not* contained within MARKUP list, flag as a spelling error
         *  otherwise export under the rule attribute.
         */
        if (syntax_keywordx(st, start, end - start, SYNK_MARKUP, SYNK_MARKUP, 1) < 0) {
            return syntax_writex(st, start, end, ATTR_SPELL, 0);
        }
    }
    return syntax_writex(st, start, end, colour, rule->writex);
}


static const LINECHAR *
syndfa_regex(SyntaxTable_t *st, const SyntaxDFA_t *dfa, const struct dfastate *state,
        int normal, unsigned offset, int *preproc, const LINECHAR *cursor, const LINECHAR *end)
{
    const LINECHAR *ocursor = cursor, *nomatch = NULL;
    int ispreproc = *preproc;                   /* TODO -- promote to 2 */
    int sol = (offset ? FALSE : TRUE);

    ED_TRACE(("dfa_parse: off:%u, normal:%3d, preproc:%d, state:%s/%d, text:<%.*s>\n", \
        offset, normal, ispreproc, state->name, state->regno, end - cursor, cursor))

    while (cursor < end) {
        const char *t_start = (const char *)cursor, *t_end = NULL;
        int astate;

        if ((astate = regdfa_match(             /* MCHAR??? */
                state->regex, (const char *)cursor, (const char *)end, sol, &t_start, &t_end)) >= 0) {

            const LINECHAR *tokenend = (const LINECHAR *)(t_end + 1);
            const struct dfarule *rule = state->regst[ astate ];
            const unsigned flags = rule->flags;
            int colour, t_colour = -1;

            assert((unsigned)astate < state->regno);

            /* anchored pattern, flush */
            if (t_start > (const char *)cursor) {
                ED_TRACE(("\twrite[0]: non-leading\n"))
                if (!nomatch) nomatch = cursor;
                cursor = (const LINECHAR *)t_start;
            }

            /* select colour */
            if ((FLAG_DIRECTIVE & flags) && ispreproc &&
                    (t_colour = syntax_preprocessor(st, cursor, tokenend - cursor)) >= 0) {
                colour = t_colour;              /* processor directive */

            } else if ((FLAG_KEYWORD & flags) &&
                    (t_colour = syntax_keyword(st, cursor, tokenend - cursor)) >= 0) {
                if (ispreproc /*>= 2*/) {       /* keyword */
                    /*
                     *  TODO - allow selection of r-value color,
                     *              either preprocessor (ispreproc) or native code colours (ispreproc >= 2)
                     *  issue: must mirror selection/logic within line-style flags.
                     */
                    colour = ATTR_PREPROCESSOR_KEYWORD;
                } else {
                    colour = t_colour;
                }

            } else {                            /* not keyword/directive, word */
                colour = rule->colour;
                if (ispreproc) {
                    if ((FLAG_KEYWORD|FLAG_WORD) & flags) {
                        colour = ATTR_PREPROCESSOR_WORD;
                    }
                } else {
                    if ((FLAG_KEYWORD|FLAG_WORD) & flags) {
                        colour = ATTR_WORD;
                    }
                }
            }

            /* print (or cache if we are concat) */
            if (colour == normal && 0 == (FLAG_PREPROC & flags)) {
                if (NULL == nomatch) {          /* concat onto existing no-match/normal area */
                    nomatch = cursor;
                }
                cursor = tokenend;

            } else {
                if (nomatch) {                  /* flush no-match/normal */
                    ED_TRACE(("\twrite[1]: color:%3d, text:<%.*s>\n", normal, cursor - nomatch, nomatch))
                    syntax_write(st, nomatch, cursor, normal);
                    nomatch = NULL;
                }

                if (rule->state) {              /* sub-expressions */
                    cursor = syndfa_regex(st, dfa, rule->state, colour,
                                    offset + (cursor - ocursor), &ispreproc, cursor, tokenend);

                } else {
                    ED_TRACE(("\twrite[2]: color:%3d, text:<%.*s>\n", colour, tokenend - cursor, cursor))
                    cursor = syndfa_writex(st, rule, cursor, tokenend, colour);
                }
            }

            /* set-up right colour */
            if (FLAG_PREPROC & flags) {         /* right side of the preprocessor statement? */
                /*
                 *  0 = SYNT_PREPROCESSOR not defined.
                 *  1 = SYNT_PREPROCESSOR defined.
                 *  2 = SYNT_PREPROCESSOR and PREPROC.
                 */
                normal = rule->colour;          /* generally ATTR_PROCESSOR */
                if (ispreproc < 2) ++ispreproc;
            }

        } else {
            if (NULL == nomatch) {
                nomatch = cursor;
            }
            ++cursor;
        }

        sol = FALSE;
    }

    if (nomatch) {                              /* flush no-match/normal */
        ED_TRACE(("\twrite[3]: color:%3d, text:<%.*s>\n", normal, end - nomatch, nomatch))
        syntax_write(st, nomatch, end, normal);
    }

    *preproc = ispreproc;
    return end;
}


/*
 *  syndfa_write ---
 *      Line write.
 */
static int
syndfa_write(
    SyntaxTable_t *st, void *object, const LINECHAR *cursor, unsigned offset, const LINECHAR *end)
{
    const SyntaxDFA_t *dfa = (const SyntaxDFA_t *)object;
    int normal = ATTR_CURRENT, preproc = 0;

    if (L_IN_PREPROC == st->st_lsyntax) {
        normal = ATTR_PREPROCESSOR;
        preproc = 1;
    }

    syndfa_regex(st, dfa, dfa->dfa_base, normal, offset, &preproc, cursor, end);

    if (preproc && L_IN_CONTINUE == st->st_lsyntax) {
        st->st_lsyntax = L_IN_PREPROC;          /* promote */
    }
    return 0;
}


/*
 *  syndfa_cont ---
 *      Section continuation, execute 'comment' and 'string' subrules.
 */
static int
syndfa_cont(
    SyntaxTable_t *st, void *object, int attr, const LINECHAR *cursor, unsigned offset, const LINECHAR *end)
{
    const SyntaxDFA_t *dfa = (const SyntaxDFA_t *)object;
    int preproc = 0;

    switch (attr) {
    case ATTR_COMMENT: {
            const struct dfastate *comment;

            if (NULL != (comment = dfa->dfa_comment)) {
                syndfa_regex(st, dfa, comment, attr, offset, &preproc, cursor, end);
                return 0;
            }
        }
        break;
    case ATTR_STRING: {
            const struct dfastate *string;

            if (NULL != (string = dfa->dfa_string)) {
                syndfa_regex(st, dfa, string, attr, offset, &preproc, cursor, end);
                return 0;
            }
        }
        break;
    }
    syntax_write(st, cursor, end, attr);
    return 0;
}


static SyntaxDFA_t *
syndfa_resolve(int argi, int create, SyntaxTable_t **stp)
{
    SyntaxTable_t *st;

    if (NULL == (st = syntax_argument(argi, 0))) {
        return NULL;
    }

    if (NULL == st->st_drivers[SYNI_REGDFA]) {
        if (create) {
            SyntaxDFA_t *dfa;

            if (NULL == (dfa = chk_calloc(sizeof(SyntaxDFA_t),1))) {
                return NULL;
            }
            dfa->dfa_driver.sd_instance = dfa;
            dfa->dfa_driver.sd_select   = syndfa_select;
            dfa->dfa_driver.sd_write    = syndfa_write;
            dfa->dfa_driver.sd_cont     = syndfa_cont;
            dfa->dfa_driver.sd_destroy  = syndfa_destroy;
            dfa->dfa_owner = st;

            st->st_drivers[SYNI_REGDFA] = &dfa->dfa_driver;
        }
    }

    if (stp) {
        *stp = st;
    }
    return (SyntaxDFA_t *)st->st_drivers[SYNI_REGDFA]->sd_instance;
}


static void
syndfa_rule(SyntaxDFA_t *syntax, const char *pattern, const char *name, int namelen,
        const char *group, int grouplen, const char *contains, int containslen, unsigned flags, int colour)
{
    const int idx = syntax->dfa_count;
    struct dfarule *rule = syntax->dfa_rules + idx;
    int writex = 0;

    if (idx >= DFARULES_MAX) {
        errorf("syntaxdfa: rule count exceeds (%d)", DFARULES_MAX);
        return;                                 /* local storage */
    }

    if (FLAG_CONTAINED & flags) {
        if (NULL == name) {
            errorf("syntaxdfa: contains must be named.");
            return;
        } else if (NULL != contains) {
            errorf("syntaxdfa: child '%.*s' cannot also be a parent.", namelen, name);
            return;
        }
    }

    if (name) {
        if (syndfa_find(syntax, name, namelen)) {
            errorf("syntaxdfa: rule '%.*s' must be unique", namelen, name);
            return;
        }
    }

    if (group) {
        unsigned g = 0;

        // TODO:
        //  group=<grpname>[;<grpname>[;...]>
        //      allow multiple groups.
        //
        for (g = 0; g < syntax->dfa_grpcnt; ++g) {
            const char *grpname = syntax->dfa_grpnames[g];

            if ((int)strlen(grpname) == grouplen &&
                    0 == strncmp(grpname, group, grouplen)) {
                group = grpname;
                break;
            }
        }

        if (g >= syntax->dfa_grpcnt) {
            if (syntax->dfa_grpcnt >= DFAGROUP_MAX) {
                errorf("syntaxdfa: group count exceeds (%d)", DFAGROUP_MAX);
                return;
            }
            syntax->dfa_grpnames[ syntax->dfa_grpcnt++ ] =
                    group = chk_snalloc(group, grouplen);
        }
    }

    if (pattern) {
        if (0 == *pattern || regdfa_check(pattern) != 0) {
            return;                             /* regular expression error */
        }

        if (FLAG_QUICK & flags) {
            const size_t len = strlen(pattern);
            char *buf = chk_alloc(len + 1 + 1);

            buf[0] = 0x01;                      /* priority flag */
            strcpy(buf + 1, pattern);
            pattern = buf;
        } else {
            pattern = chk_salloc(pattern);
        }
    }

    if (FLAG_SPELL & flags) writex |= SYNW_SPELL;
    if (FLAG_TODO  & flags) writex |= SYNW_TODO;
    if (FLAG_TAGS  & flags) writex |= SYNW_TAGS;

    if (ATTR_COMMENT == colour) {               /* inform display driver */
        if (FLAG_SPELL & flags) syntax->dfa_driver.sd_writex |= SYNW_COMMENT_SPELL;
        if (FLAG_TODO  & flags) syntax->dfa_driver.sd_writex |= SYNW_COMMENT_TODO;
    }

    rule->pattern   = pattern;
    rule->name      = (name ? chk_snalloc(name, namelen) : NULL);
    rule->contains  = (contains ? chk_snalloc(contains, containslen) : NULL);
    rule->group     = group;
    rule->flags     = flags;
    rule->colour    = colour;
    rule->writex    = writex;

    ++syntax->dfa_count;
}


static struct dfarule *
syndfa_find(SyntaxDFA_t *syntax, const char *name, int namelen)
{
    const unsigned count = syntax->dfa_count;
    struct dfarule *rule = syntax->dfa_rules;
    unsigned i;

    for (i = 0; i < count; ++i, ++rule) {
        const char *rulename;

        if (NULL != (rulename = rule->name) &&
                (int)strlen(rulename) == namelen && 0 == strncmp(rulename, name, namelen)) {
            return rule;
        }
    }
    return NULL;
}


static void
syndfa_make(SyntaxDFA_t *syntax)
{
    if (syntax->dfa_built) {                    /* created? */
        return;
    }

    syntax->dfa_base =                          /* primary expression */
        syndfa_group(syntax, NULL);
    assert(syntax->dfa_base);

    if (syntax->dfa_grpcnt) {                   /* build groups */
        unsigned g = 0;

        for (g = 0; g < syntax->dfa_grpcnt; ++g) {
            const char *grpname = syntax->dfa_grpnames[g];
            struct dfastate *state;

            if (NULL != (state = syndfa_group(syntax, grpname))) {
                int grpcolour;
                                                /* assign group */
                if ((grpcolour = attribute_value(grpname)) >= 0) {
                    struct dfarule *rule = syntax->dfa_rules, *end = rule + syntax->dfa_count;

                    while (rule < end) {
                        if (NULL == rule->state) {
                            if (NULL == rule->group) {
                                if (rule->colour == grpcolour) {
                                    rule->state = state;
                                }
                            }
                        }
                        ++rule;
                    }
                }

                if (0 == strcmp("string", grpname)) {
                    syntax->dfa_string  = state;
                } else if (0 == strcmp("comment", grpname)) {
                    syntax->dfa_comment = state;
                }

                syntax->dfa_grpst[g] = state;
            }
        }
    }

    syntax->dfa_built = time(NULL);
    assert(syntax->dfa_base);
}


static struct dfastate *
syndfa_group(SyntaxDFA_t *syntax, const char *group)
{
    struct dfastate *state = NULL;
    struct dfarule *rule = syntax->dfa_rules, *ruleend;
    const uint32_t flags = syntax->dfa_owner->st_flags;

    const char *patterns[DFASTATES_MAX] = {0};
    struct dfarule *states[DFASTATES_MAX] = {0};
    unsigned stickyflags = 0, cnt = 0;

    for (ruleend = rule + syntax->dfa_count; rule < ruleend && cnt < DFASTATES_MAX; ++rule) {

#define STICKY_FLAGS    (FLAG_SPELL|FLAG_TODO|FLAG_TAGS)

        if (FLAG_CONTAINED & rule->flags) {
            continue;
        }

        if (group != rule->group) {
            continue;
        }

        if (rule->pattern) {
            states[cnt] = rule;
            patterns[cnt] = rule->pattern;
            stickyflags |= (rule->flags & (STICKY_FLAGS));
            ++cnt;
        }

        if (rule->contains) {
            const char *cursor = rule->contains,
                    *end = cursor + strlen(cursor);

            stickyflags |= (rule->flags & (STICKY_FLAGS));

            while (cursor < end) {              /* <child> [; <child> ....] */
                const char *delim = strchr(cursor, ';');
                int length;

                if (NULL == delim) delim = end;
                length = delim - cursor;

                cursor = str_trim(cursor, &length);

                if (length > 0) {
                    struct dfarule *child;

                    if (NULL == (child = syndfa_find(syntax, cursor, length))) {
                        errorf("syntaxdfa: referencing non-existent rule '%.*s'", length, cursor);

                    } else if (0 == (FLAG_CONTAINED & child->flags)) {
                        errorf("syntaxdfa: referencing non-contained rule '%s'", child->name);

                    } else if (cnt >= DFASTATES_MAX) {
                        errorf("syntaxdfa: state count exceeded %d", DFASTATES_MAX);

                    } else {
                        states[cnt] = child;
                        patterns[cnt] = child->pattern;
                        ++cnt;
                    }
                }
                cursor = delim + 1;
            }
        }
    }

    if (cnt && NULL != (state = chk_calloc(sizeof(struct dfastate),1))) {
        unsigned stateno, groupno = ++syntax->dfa_groups;

        state->name  = (group ? group : "base");
        state->regno = cnt;
        state->regex = regdfa_create(patterns, cnt,
                         ((SYNF_CASEINSENSITIVE & flags) ? REGDFA_ICASE : 0));

        trace_log("Group:%s, regno:%d\n", state->name, cnt);

        for (stateno = 0; stateno < cnt; ++stateno) {
            rule = states[stateno];
            rule->stateno = stateno;
            if (0 == (FLAG_CONTAINED & rule->flags)) {
                rule->flags |= stickyflags;
            }
            state->regst[stateno] = rule;
            trace_ilog("%2d.%2d] colour:%3d, pattern:%s", groupno, stateno, rule->colour, rule->pattern);
            if (rule->name) {
                trace_log(", name:%s\n", rule->name);
            }
            trace_log("\n");
        }
    }

    return state;
}


/*  Function:           syndfa_save
 *      Export the specified DFA.
 *
 *  Parameters:
 *      syntax -            Syntax definition.
 *      filename -          Cache filename.
 *      timestamp -         Timestamp of specification.
 *
 *  Returns:
 *      nothing.
 */
static void
syndfa_save(
    SyntaxDFA_t *syntax, const char *filename, int timestamp)
{
    char filepath[MAX_PATH];
    FILE *fd;

    if (NULL == filename || 0 == filename[0]) {
        return;
    }

    sxprintf(filepath, sizeof(filepath), GRDFA_PATTERN, filename);
    if (NULL == (fd  = fopen(filepath, "wb"))) {
        return;
    }

    trace_ilog("syndfa_save(%s)\n", filepath);
    fprintf(fd, "%s\n", START_DFA);
    fprintf(fd, "VERSION=1\n");                 /* format version */
    fprintf(fd, "%s%d\n", TIMESTAMP, timestamp);
    fprintf(fd, "%s\n", START_PATTERN);

    if (syntax->dfa_count) {
        struct dfarule *rule = syntax->dfa_rules, *end = rule + syntax->dfa_count;
        unsigned stateno = 0;

        while (rule < end) {
            fprintf(fd, "%3u; %3u; %3u; 0x%04x; pattern:%s, name:%s, group:%s, contains:%s\n",
                stateno, rule->stateno, rule->colour, rule->flags,
                (rule->pattern  ? (0x01 == *rule->pattern ? rule->pattern + 1 : rule->pattern) : ""),
                (rule->name     ? rule->name     : ""),
                (rule->group    ? rule->group    : ""),
                (rule->contains ? rule->contains : ""));
            ++stateno;
            ++rule;
        }
    }

    fprintf(fd, "%s\n", END_PATTERN);
    fprintf(fd, "%s\n", START_CACHE);
    if (syntax->dfa_base) {
        unsigned grp = syntax->dfa_grpcnt;

        regdfa_export(syntax->dfa_base->regex, fd);

        while (grp-- > 0) {
            struct dfastate *state;

            if (NULL != (state = syntax->dfa_grpst[grp])) {
                fprintf(fd, "%s\n%s: ", START_GROUP, syntax->dfa_grpnames[grp]);
                regdfa_export(state->regex, fd);
                fprintf(fd, "%s\n", END_GROUP);
            }
        }
    }
    fprintf(fd, "%s\n", END_CACHE);
    fprintf(fd, "%s\n", END_DFA);

    fclose(fd);
}


/*  Function:           syndfa_load
 *      Import the specified DFA.
 *
 *  Parameters:
 *      syntax -            Syntax definition.
 *      filenname -         Cache filename.
 *      timestamp -         Timestamp of specification.
 *
 *  Returns:
 *      int
 */
static int
syndfa_load(
    SyntaxDFA_t *syntax, const char *filename, int timestamp)
{
    char line[1024], filepath[MAX_PATH];
    int state = -1;
    FILE *fd;

    if (NULL == filename || 0 == filename[0]) {
        return -1;
    }

    sxprintf(filepath, sizeof(filepath), GRDFA_PATTERN, filename);
    if (NULL == (fd = fopen(filepath, "rb"))) {
        return -1;
    }

    trace_ilog("syndfa_load(%s)\n", filepath);

    while (fgets(line, sizeof(line), fd)) {
        size_t length = strlen(line);

        if (length > 0 && '\n' == line[length-1]) {
            line[--length] = 0;                 /* remove newline */
        }

        if (0 == strcmp(line, START_DFA)) {
            state = 1;

        } else if (1 == state && length > 10 && 0 == strncmp(line, TIMESTAMP, 10)) {
            int t_timestamp = atoi(line + 10);

            trace_ilog("\ttimestamp(%d,%d)\n", t_timestamp, timestamp);
            if (t_timestamp != timestamp) {
                state = -1;
                break;
            }
            state = 2;

        } else if (2 == state && 0 == strcmp(line, START_CACHE)) {
//TODO      syntax->dfa_base = regdfa_import(fd);
            state = 3;

        } else if (3 == state && 0 == strcmp(line, END_CACHE)) {
            state = 4;

        } else if (4 == state && 0 == strcmp(line, END_DFA)) {
            if (syntax->dfa_built) {
                state = 0;
            } else {
                state = -1;
            }
            break;
        }
    }

    trace_ilog("==> %d\n", state);
    fclose(fd);
    return state;
}
/*end*/
