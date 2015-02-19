#include <edidentifier.h>
__CIDENT_RCSID(gr_crpragma_c,"$Id: crpragma.c,v 1.12 2014/10/22 02:33:28 ayoung Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: crpragma.c,v 1.12 2014/10/22 02:33:28 ayoung Exp $
 * Pragma support.
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

#include "grunch.h"                             /* local definitions */

static int              pragma_get(lexer_t *lexer);
static void             pragma_message(lexer_t *lexer);
static void             pragma_control_message(lexer_t *lexer, int enable);
static void             pragma_warning(lexer_t *lexer);
static void             pragma_autoload(lexer_t *lexer);


/*  Function:           pragma_process
 *      Process pragma directives.
 *
 *  Parameters:
 *      cp -                Character stream.
 *
 *  Returns:
 *      nothing
 */
void
pragma_process(const char *cp)
{
    lexer_t lexer = {0};

    lexer.get      = pragma_get;
    lexer.l_flags  = LEX_FNOSALLOC;
    lexer.l_cursor = chk_salloc(cp);

    if (O_SYMBOL == yylexer(&lexer)) {
        const char *word = yylval.sval;

        if (0 == strcmp(word, "message")) {
            pragma_message(&lexer);

        } else if (0 == strcmp(word, "enable_message")) {
            pragma_control_message(&lexer, TRUE);

        } else if (0 == strcmp(word, "disable_message")) {
            pragma_control_message(&lexer, FALSE);

        } else if (0 == strcmp(word, "warning")) {
            pragma_warning(&lexer);

        } else if (0 == strcmp(word, "autoload")) {
            pragma_autoload(&lexer);

        } else if (0 == strcmp(word, "once") ||
                     0 == strcmp(word, "align") ||
                     0 == strcmp(word, "ident") ||      /* ident <string *> */
                     0 == strcmp(word, "options") ||    /* options <option>[=<value>] ... */
                     0 == strcmp(word, "pack")) {       /* pack (nopack|1..4|pop) */
            yywarningf("ignoring pragma '%s'", word);

        } else {
            yywarningf("unknown pragma directive '%s'", word);
        }

    } else {
        yyerror("invalid pragma directive");
    }
}


/*  Function:           pragma_get
 *      Pragma lexer stream operator.
 *
 *  Parameters:
 *      lexer -             Lexer stream.
 *
 *  Returns:
 *      nothing
 */
static int
pragma_get(lexer_t *lexer)
{
    const int ch = (int)*lexer->l_cursor;

    if (ch) {
        ++lexer->l_cursor;
    }
    return ch;
}


/*  Function:           pragma_control_message
 *      [dis]enable display of selected message number.
 *
 *  Syntax:
 *      #pragma enable_message messageNo
 *      #pragma disable_message messageNo
 *
 *  Parameters:
 *      lexer -             Lexer stream.
 *      enable -            *TRUE* is enabling otherwise disable.
 *
 *  Returns:
 *      nothing
 */
static void
pragma_control_message(lexer_t *lexer, int enable)
{
    if (O_INTEGER_CONST == yylexer(lexer)) {
        const int msgno = yylval.ival;

        switch (msgno) {
        case RC_SYNTAX_ERROR:
            break;
        default:
            crmessage(msgno, enable);
            break;
        }
        if (yylexer(lexer)) {
            yyerror("expected end-of-line");
        }
    }
}


/*  Function:           pragma_message
 *      Message directive, outputs the specified strings unconditionally
 *      to stdout.
 *
 *  Syntax:
 *      #pragma message ("one or more " "long message " "strings")
 *
 *  Parameters:
 *      lexer -             Lexer stream.
 *
 *  Returns:
 *      nothing
 */
static void
pragma_message(lexer_t *lexer)
{
    if (O_OROUND != yylexer(lexer)) {
        yyerror("expected a '('");
    } else {
        int token, count = 0;

        for (;;) {
            if ((token = yylexer(lexer)) != O_STRING_CONST) {
                break;
            }
            printf("%s", yylval.sval);
            ++count;
        }
        if (count) printf("\n");
        if (token != O_CROUND) {
            yyerror("expected a ')'");
        }
    }
}


/*  Function:           pragma_warning
 *      warning directive.
 *
 *  Syntax:
 *      #pragma warning( <specifier> : <number> [,...] [,<specifier ...] )
 *      #pragma warning( level : <1,2,3,4> )
 *      #pragma warning( push[ , n ] )
 *      #pragma warning( pop )
 *
 *      #pragma warning off
 *
 *  Description;
 *      Allows selective modification of the behavior of compiler warning messages.
 *
 *      The specifier can be one of the following.
 *
 *          Specifier       Meaning
 *          once            Display the specified message(s) only once.
 *          default         Apply the default compiler behavior to the specified message(s).
 *          disable         Do not issue the specified warning message(s).
 *          error           Report the specified warnings as errors.
 *          level           Apply the given warning level to the specified warning message(s).
 *
 *      The warning pragma also supports the following syntax:
 *
 *          #pragma warning( push )
 *          #pragma warning( pop )
 *
 *      The pragma warning( push ) stores the current warning state for all warnings.
 *
 *      The pragma warning( pop ) pops the last warning state pushed onto the stack. Any changes
 *      made to the warning state between push and pop are undone.
 */
static void
pragma_warning(lexer_t *lexer)
{
    static int warninglvl = 0, warningstk[16] = {0};
    int token;

    if ((token = yylexer(lexer)) != O_OROUND) {
        /*
         *  #pragma warning off|on|push|pop
         */
        if (O_SYMBOL == token) {
            const char *specifier = yylval.sval;

            if (0 == strcmp(specifier, "off")) {
                xf_warnings = 0;

            } else if (0 == strcmp(specifier, "on")) {
                xf_warnings = 1;

            } else if (0 == strcmp(specifier, "push")) {
                if (warninglvl < sizeof(warningstk)) {
                    warningstk[warninglvl++] = xf_warnings;
                }

            } else if (0 == strcmp(specifier, "pop")) {
                if (warninglvl > 0) {
                    xf_warnings = warningstk[--warninglvl];
                }

            } else {
                yywarningf("unknown warning directive '%s'", specifier);
            }

            if (yylexer(lexer)) {
                yyerror("expected end-of-line");
            }
        } else {
            yyerror("expected a '('");
        }

    } else {
        /*
         *  #pragma warning (once|default|disable|enable|error : <list>)
         *  #pragma warning (level : <level>)
         *  #pragma warning (push|pop)
         */
        do {
            if (O_SYMBOL == (token = yylexer(lexer))) {
                const char *specifier = chk_salloc(yylval.sval);

                token = yylexer(lexer);
                if (0 == strcmp(specifier, "once")) {
                    if (O_SEMICOLON == token) {
                    }

                } else if (0 == strcmp(specifier, "default")) {
                    if (O_SEMICOLON == token) {
                    }

                } else if (0 == strcmp(specifier, "disable")) {
                    if (O_SEMICOLON == token) {
                    }

                } else if (0 == strcmp(specifier, "enable")) {
                    if (O_SEMICOLON == token) {
                    }

                } else if (0 == strcmp(specifier, "error")) {
                    if (O_SEMICOLON == token) {
                    }

                } else if (0 == strcmp(specifier, "level")) {
                    if (O_SEMICOLON == token) {
                    }

                } else if (0 == strcmp(specifier, "off")) {
                    xf_warnings = 0;

                } else if (0 == strcmp(specifier, "on")) {
                    xf_warnings = 1;

                } else if (0 == strcmp(specifier, "push")) {
                    if (warninglvl < sizeof(warningstk)) {
                        warningstk[warninglvl++] = xf_warnings;
                    }

                } else if (0 == strcmp(specifier, "pop")) {
                    if (warninglvl > 0) {
                        xf_warnings = warningstk[--warninglvl];
                    }

                } else {
                    yywarningf("unknown warning directive '%s'", specifier);
                }

                chk_free((void *)specifier);
            }

            /* consume until ',' or ')' */
            while (token && O_COMMA != token && O_CROUND != token) {
                token = yylexer(lexer);
            }
        } while (O_COMMA == token);

        if (O_CROUND != token) {
            yyerror("expected a ')'");
        }
    }
}


/*  Function:           pragma_autoload
 *      autoload directive.
 *
 *  Syntax:
 *      #pragma autoload ("function1" ...)
 *
 *  Parameters:
 *      lexer -             Lexer stream.
 *
 *  Returns:
 *      nothing
 */
static void
pragma_autoload(lexer_t *lexer)
{
    if (O_OROUND != yylexer(lexer)) {
        yyerror("expected a '('");
    } else {
        int token;

        for (;;) {                          /* "<name>" [,] ... */
            if ((token = yylexer(lexer)) == O_STRING_CONST) {
                autoload_push(yylval.sval);

            } else if (O_SYMBOL == token && /* __FUNCTION__ */
                            0 == strcmp("__FUNCTION__", yylval.sval)) {
                if (x_funcname && x_funcname[0]) {
                    autoload_push(x_funcname);
                } else {
                    yyerrorf("__FUNCTION__ referenced outside function scope");
                }
            } else {
                if (O_COMMA == token) {
                    continue;
                }
                break;
            }
        }
        if (O_CROUND != token) {
            yyerror("expected a ')'");
        }
    }
}
/*end*/
