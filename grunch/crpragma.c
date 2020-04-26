#include <edidentifier.h>
__CIDENT_RCSID(gr_crpragma_c,"$Id: crpragma.c,v 1.14 2020/04/23 12:35:50 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: crpragma.c,v 1.14 2020/04/23 12:35:50 cvsuser Exp $
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

static int              prlexer(lexer_t *lexer);
static int              prget(lexer_t *lexer);

static void             pragma_message(lexer_t *lexer);
static void             pragma_control_message(lexer_t *lexer, int enable);
static void             pragma_scoping(lexer_t *lexer, int state);
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

    lexer.get      = prget;
    lexer.l_flags  = LEX_FNOSALLOC;
    lexer.l_cursor = chk_salloc(cp);

    if (O_SYMBOL == prlexer(&lexer)) {
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

        } else if (0 == strcmp(word, "lexical_scope")) {
            pragma_scoping(&lexer, 1);

        } else if (0 == strcmp(word, "dynamic_scope")) {
            pragma_scoping(&lexer, 0);

        } else if (0 == strcmp(word, "scope")) {
            pragma_scoping(&lexer, -1);

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


/*  Function:           prlexer
 *      Pragma lexer, basic tokeniser without symbol lookup.
 *
 *  Returns:
 *      Token identifier.
 */
static int
prlexer(lexer_t *lexer)
{
    return yylexer(lexer, FALSE /*noexpand*/);
}


/*  Function:           prget
 *      Pragma lexer stream operator.
 *
 *  Parameters:
 *      lexer -             Lexer stream.
 *
 *  Returns:
 *      nothing
 */
static int
prget(lexer_t *lexer)
{
    const int ch = (int)*lexer->l_cursor;

    if (ch) ++lexer->l_cursor;
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
    if (O_INTEGER_CONST == prlexer(lexer)) {
        const int msgno = yylval.ival;

        switch (msgno) {
        case RC_SYNTAX_ERROR:
            break;
        default:
            crmessage(msgno, enable);
            break;
        }
        if (prlexer(lexer)) {
            yyerror("expected end-of-line");
        }
    }
}


/*  Function;
 *      Scoping directive.
 *
 *  Syntax:
 *      #pragma lexical_scope | dynamic_scope
 *
 *      #pragma scope( push [,lexical|dynamic] )
 *      #pragma scope( pop )
 *
 *  Parameters:
 *      lexer -             Lexer stream.
 *      state -             0=enable, 1=disable
 *
 *  Returns:
 *      nothing
 */
static void
pragma_scoping(lexer_t *lexer, int state)
{
    static int scopelvl = 0, scopestk[16] = {0};
    int push = 0, pop = 0;

    // parse
    if (-1 == state) {
        if (O_OROUND == prlexer(lexer)) {
            if (O_SYMBOL == prlexer(lexer)) {
                const char *specifier = yylval.sval;
                int token = 0;

                // scope( push [,lexical|dynamic] )
                if (0 == strcmp(specifier, "push")) {
                    if (O_COMMA == (token = prlexer(lexer))) {
                        if (O_SYMBOL == prlexer(lexer)) {
                            const char *specifier2 = yylval.sval;

                            if (0 == strcmp(specifier2, "lexical")) {
                                state = 1;

                            } else if (0 == strcmp(specifier2, "dynamic")) {
                                state = 0;

                            } else {
                                yyerrorf("unexpected token '%s'", specifier2);
                                return;
                            }
                            token = 0;

                        } else {
                            yyerror("expected a token");
                            return;
                        }
                    }
                    push = 1;

                // scope( pop )
                } else if (0 == strcmp(specifier, "pop")) {
                    pop = 1;

                // scope( lexical )
                } else if (0 == strcmp(specifier, "lexical")) {
                    state = 1;

                // scope( dynamic )
                } else if (0 == strcmp(specifier, "dynamic")) {
                    state = 0;

                } else {
                    yyerrorf("unexpected token '%s'", specifier);
                    return;
                }

                if (0 == token) token = prlexer(lexer);
                if (O_CROUND != token) {
                    yyerror("expected a ')'");
                    return;
                }

            } else {
                yyerror("expected a token");
                return;
            }

        } else {
            yyerror("expected a '('");
            return;
        }
    }

    if (prlexer(lexer)) {                       /* termination */
        yyerror("expected end-of-line");
        return;
    }

    // apply
    if (x_funcname && x_funcname[0]) {
        yywarningf("lexical scoping set inside function scope; ignored");
    } else {
        if (push) {
            if (scopelvl < sizeof(scopestk)) {
                scopestk[scopelvl++] = xf_lexical_scope;
            } else {
                yyerror("scope stack exceeded");
            }
        }

        if (state >= 0) xf_lexical_scope = state;

        if (pop) {
            if (scopelvl > 0) {
                xf_lexical_scope = scopestk[--scopelvl];
            } else {
                yyerror("scope stack empty");
            }
        }
    }
}


/*  Function:           pragma_message
 *      Message directive, outputs the specified strings unconditionally to stdout.
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
    if (O_OROUND != prlexer(lexer)) {
        yyerror("expected a '('");
    } else {
        int token, count = 0;

        for (;;) {
            if ((token = prlexer(lexer)) != O_STRING_CONST) {
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
 *      #pragma warning off|on|push|pop
 *
 *      #pragma warning( <specifier> : <number> [,...] [,<specifier ...] )
 *      #pragma warning( level : <1,2,3,4> )
 *
 *      #pragma warning( push[ , n ] )
 *      #pragma warning( pop )
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

    if ((token = prlexer(lexer)) != O_OROUND) {
        /*
         *  #pragma warning off|on|push|pop
         */
        if (O_SYMBOL == token) {
            const char *specifier = yylval.sval;

            // warning off
            if (0 == strcmp(specifier, "off")) {
                xf_warnings = 0;

            // warning on
            } else if (0 == strcmp(specifier, "on")) {
                xf_warnings = 1;

            // warning push
            } else if (0 == strcmp(specifier, "push")) {
                if (warninglvl < sizeof(warningstk)) {
                    warningstk[warninglvl++] = xf_warnings;
                } else {
                    yyerror("warning stack exceeded");
                }

            // warning pop
            } else if (0 == strcmp(specifier, "pop")) {
                if (warninglvl > 0) {
                    xf_warnings = warningstk[--warninglvl];
                } else {
                    yyerror("warning stack empty");
                }

            } else {
                yywarningf("unknown warning directive '%s'", specifier);
            }

            if (prlexer(lexer)) {
                yyerror("expected end-of-line");
            }
        } else {
            yyerror("expected a '('");
        }

    } else {
        /*
         *  #pragma warning (once|default|disable|enable|error : <list> ...)
         *  #pragma warning (level : <level>)
         *  #pragma warning (push|pop)
         */
        if (O_SYMBOL == (token = prlexer(lexer))) {
            const char *specifier = yylval.sval;

            token = 0;                      // clear current token

            // warning( push [, n|off|on ] )
            if (0 == strcmp(specifier, "push")) {
                if (warninglvl < sizeof(warningstk)) {
                    warningstk[warninglvl++] = xf_warnings;
                } else {
                    yyerror("warning stack exceeded");
                }

                if (O_COMMA == (token = prlexer(lexer))) {
                    if (O_INTEGER_CONST != (token = prlexer(lexer)) ||
                            yylval.ival < 1 || yylval.ival > 4) {
                        if (O_SYMBOL == token && 0 == strcmp(yylval.sval, "off")) {
                            xf_warnings = 0;
                        } else if (O_SYMBOL == token && 0 == strcmp(yylval.sval, "on")) {
                            xf_warnings = 1;
                        } else {
                            yyerror("expected a numeric value 1 .. 4");
                            return;
                        }
                    } else {
                        xf_warnings = yylval.ival;
                    }
                    token = 0;
                }

            // warning( pop )
            } else if (0 == strcmp(specifier, "pop")) {
                if (warninglvl > 0) {
                    xf_warnings = warningstk[--warninglvl];
                } else {
                    yyerror("warning stack empty");
                }

            // warning( off )
            } else if (0 == strcmp(specifier, "off")) {
                xf_warnings = 0;

            // warning( on )
            } else if (0 == strcmp(specifier, "on")) {
                xf_warnings = 1;

            // warning( specifier : values .. [, ...])
            } else {
                do {
                    specifier = chk_salloc(yylval.sval);
                    token = prlexer(lexer);     // next token

                    if (O_SEMICOLON != token) {
                        yyerrorf("expected a colon, following '%s'", specifier);
                        return;
                    }

//                  if (0 == strcmp(specifier, "once")) {
//                  } else if (0 == strcmp(specifier, "default")) {
//                  } else if (0 == strcmp(specifier, "disable")) {
//                  } else if (0 == strcmp(specifier, "enable")) {
//                  } else if (0 == strcmp(specifier, "error")) {
//                  } else {
                        if (! token) {
                            yyerror("expected a token");
                        } else {
                            yywarningf("unknown warning directive '%s'", specifier);
                        }
//                  }

                    chk_free((void *)specifier);

                } while (O_COMMA == token &&
                    O_SYMBOL == (token = prlexer(lexer)));
            }
        }

        if (0 == token) token = prlexer(lexer);
        if (O_CROUND != token) {
            yyerror("expected a ')'");
        } else if (token) {
            if (prlexer(lexer)) {               /* termination */
                yyerror("expected end-of-line");
            }
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
    if (O_OROUND != prlexer(lexer)) {
        yyerror("expected a '('");
    } else {
        int token;

        for (;;) {                          /* "<name>" [,] ... */
            if ((token = prlexer(lexer)) == O_STRING_CONST) {
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
