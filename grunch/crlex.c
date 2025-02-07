#include <edidentifier.h>
__CIDENT_RCSID(gr_crlex_c,"$Id: crlex.c,v 1.38 2025/02/07 03:03:22 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: crlex.c,v 1.38 2025/02/07 03:03:22 cvsuser Exp $
 * Lexical analyser for the GRUNCH language.
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

#include <float.h>                              /* FLT_MIN/FLT_MAX */
#include <math.h>
#include <limits.h>

#if defined(_MSC_VER)
#include <msvcversions.h>
#endif

#if (defined(_MSC_VER) && (_MSC_VER >= _MSC_VER_2015)) || \
           (__STDC_VERSION__ >= 201103L) || (__cplusplus >= 201103L)
#define HEX_FLOATS      1                       /* Experimental */
    /* strtod() 2015+ supports hexadecimal floats */
#endif
#define INC_YYTEXT      2048                    /* Size increment for yytext[] buffer */

/*
 *  Table used for fast character classification
 */
enum {
    XSYMBOL             = 0x01,         /* [A-Za-z_] */
    XSPACE              = 0x02,         /* White space */
    XDIGIT              = 0x04,         /* Decimal digit */
    XHEXDIGIT           = 0x08          /* Hex digit */
};

#define ISSYMBOL(ch)    (lextab[ch] & XSYMBOL)
#define ISSPACE(ch)     (lextab[ch] & XSPACE)
#define ISDIGIT(ch)     (lextab[ch] & XDIGIT)
#define ISXDIGIT(ch)    (lextab[ch] & XHEXDIGIT)

static const unsigned char lextab[256] = {
    /*0x00*/0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /*0x10*/0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /*0x20*/0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /*0x30*/0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, /* 0..9 etc */
            0x0c, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /*0x40*/0x00, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x01,
            0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    /*0x50*/0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
            0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01,
    /*0x60*/0x00, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x01,
            0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    /*0x70*/0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
            0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
    /*0x80*/0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /*0x90*/0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /*0xa0*/0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /*0xb0*/0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /*0xc0*/0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /*0xd0*/0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /*0xe0*/0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /*0xf0*/0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    };

static const struct map {
    int val;
    const char *symbol;
    int slength;
    const char *token;              /* Token in source language */
    const char *desc;               /* Description */
 /* const char *lang;               -* Token at code level */
} opcodes[] =
    {
#define T_SYMBOL(_x_)   _x_, #_x_, sizeof(#_x_)-1

        /*
         *  keywords
         */
        { T_SYMBOL(K_DEFAULT),          "default"       },
        { T_SYMBOL(K_FLOAT),            "float"         },
        { T_SYMBOL(K_DOUBLE),           "double"        },
        { T_SYMBOL(K_INT),              "int"           },
        { T_SYMBOL(K_IF),               "if"            },
        { T_SYMBOL(K_ELSE),             "else"          },
        { T_SYMBOL(K_FOR),              "for"           },
        { T_SYMBOL(K_BREAK),            "break"         },
        { T_SYMBOL(K_CONTINUE),         "continue"      },
        { T_SYMBOL(K_WHILE),            "while"         },
        { T_SYMBOL(K_SWITCH),           "switch"        },
        { T_SYMBOL(K_CASE),             "case"          },
        { T_SYMBOL(K_RETURN),           "return"        },
        { T_SYMBOL(K_STRUCT),           "struct"        },
        { T_SYMBOL(K_UNION),            "union"         },
        { T_SYMBOL(K_ENUM),             "enum"          },
        { T_SYMBOL(K_EXTERN),           "extern"        },
        { T_SYMBOL(K_LOCAL),            "local"         },
        { T_SYMBOL(K_STATIC),           "static"        },
        { T_SYMBOL(K_CONST),            "const"         },
        { T_SYMBOL(K_VOLATILE),         "volatile"      },
        { T_SYMBOL(K_REGISTER),         "register"      },
        { T_SYMBOL(K_RESTRICT),         "restrict"      },  /*c99*/
        { T_SYMBOL(K_INLINE),           "inline"        },  /*c99*/
        { T_SYMBOL(K_LONG),             "long"          },
        { T_SYMBOL(K_CHAR),             "char"          },
        { T_SYMBOL(K_SHORT),            "short"         },
        { T_SYMBOL(K_UNSIGNED),         "unsigned"      },
        { T_SYMBOL(K_VOID),             "void"          },
        { T_SYMBOL(K_SIGNED),           "signed"        },
        { T_SYMBOL(K_DO),               "do"            },
        { T_SYMBOL(K_SIZEOF),           "sizeof"        },
        { T_SYMBOL(K_TYPEDEF),          "typedef"       },
        { T_SYMBOL(K_BOOL),             "bool"          },

        { T_SYMBOL(K_LIST),             "list"          },
        { T_SYMBOL(K_ARRAY),            "array"         },
    //  { T_SYMBOL(K_HASH),             "hash"          },
        { T_SYMBOL(K_STRING),           "string"        },
        { T_SYMBOL(K_DECLARE),          "declare"       },
        { T_SYMBOL(K_GLOBAL),           "global"        },
        { T_SYMBOL(K_REPLACEMENT),      "replacement"   },
        { T_SYMBOL(K_TRY),              "try"           },
        { T_SYMBOL(K_CATCH),            "catch"         },
        { T_SYMBOL(K_FINALLY),          "finally"       },
        { T_SYMBOL(K_FOREACH),          "foreach"       },
        { T_SYMBOL(K_COMMAND),          "_command"      },
    //  { T_SYMBOL(K_FOREACH),          "for_each",     NULL,   "foreach" },
    //  { T_SYMBOL(K_BLESS),            "bless"         },

    /*  { T_SYMBOL(K_BOOL),             "_Bool",        },      */
    /*  { T_SYMBOL(K_COMPLEX),          "_Complex",     },      */
    /*  { T_SYMBOL(K_IMAGINARY),        "_Imaginary",   },      */

    /*  { T_SYMBOL(K_ALIGNAS),          "_Alignas"      },      */
    /*  { T_SYMBOL(K_ATOMIC),           "_Atomic"       },      */
    /*  { T_SYMBOL(K_NORETURN),         "_Noreturn"     },      */
    /*  { T_SYMBOL(K_THREAD_LOCAL),     "_Thread_local" },      */
    /*  { T_SYMBOL(K_ALIGNOF),          "_Alignof"      },      */
    /*  { T_SYMBOL(K_GENERIC),          "_Generic"      },      */
    /*  { T_SYMBOL(K_STATIC_ASSERT),    "_Static_assert" },     */

        /*
         *  language operators/tokens
         */
        { T_SYMBOL(O_STRING_CONST),     NULL,           "string constant" },
        { T_SYMBOL(O_INTEGER_CONST),    NULL,           "integer constant" },
        { T_SYMBOL(O_FLOAT_CONST),      NULL,           "floating point constant" },

        { T_SYMBOL(O_SYMBOL),           NULL,           "symbol" },
        { T_SYMBOL(O_TYPEDEF_NAME),     NULL,           "typedef" },

        { T_SYMBOL(O_OCURLY),           NULL,           "{"   },
        { T_SYMBOL(O_CCURLY),           NULL,           "}"   },
        { T_SYMBOL(O_OROUND),           NULL,           "("   },
        { T_SYMBOL(O_CROUND),           NULL,           ")"   },
        { T_SYMBOL(O_COMMA),            NULL,           ","   },
        { T_SYMBOL(O_SEMICOLON),        NULL,           ";"   },
        { T_SYMBOL(O_EQ),               NULL,           "="   },
        { T_SYMBOL(O_PLUS_EQ),          NULL,           "+="  },
        { T_SYMBOL(O_MINUS_EQ),         NULL,           "-="  },
        { T_SYMBOL(O_MUL_EQ),           NULL,           "*="  },
        { T_SYMBOL(O_DIV_EQ),           NULL,           "/="  },
        { T_SYMBOL(O_MOD_EQ),           NULL,           "%="  },
        { T_SYMBOL(O_AND_EQ),           NULL,           "&="  },
        { T_SYMBOL(O_OR_EQ),            NULL,           "|="  },
        { T_SYMBOL(O_XOR_EQ),           NULL,           "^="  },
        { T_SYMBOL(O_EQ_OP),            NULL,           "=="  },
        { T_SYMBOL(O_NE_OP),            NULL,           "!="  },
        { T_SYMBOL(O_GE_OP),            NULL,           ">="  },
        { T_SYMBOL(O_GT_OP),            NULL,           ">"   },
        { T_SYMBOL(O_CMP_OP),           NULL,           "<=>" },
        { T_SYMBOL(O_LE_OP),            NULL,           "<="  },
        { T_SYMBOL(O_LT_OP),            NULL,           "<"   },
        { T_SYMBOL(O_ARROW),            NULL,           "->"  },
        { T_SYMBOL(O_LSHIFT),           NULL,           "<<"  },
        { T_SYMBOL(O_RSHIFT),           NULL,           ">>"  },
        { T_SYMBOL(O_LSHIFT_EQ),        NULL,           "<<=" },
        { T_SYMBOL(O_RSHIFT_EQ),        NULL,           ">>=" },
        { T_SYMBOL(O_CAND),             NULL,           "&&"  },
        { T_SYMBOL(O_COR),              NULL,           "||"  },
        { T_SYMBOL(O_PLUS),             NULL,           "+"   },
        { T_SYMBOL(O_MINUS),            NULL,           "-"   },
        { T_SYMBOL(O_MUL),              NULL,           "*"   },
        { T_SYMBOL(O_DIV),              NULL,           "/"   },
        { T_SYMBOL(O_MOD),              NULL,           "%"   },
        { T_SYMBOL(O_OR),               NULL,           "|"   },
        { T_SYMBOL(O_AND),              NULL,           "&"   },
        { T_SYMBOL(O_XOR),              NULL,           "^"   },
        { T_SYMBOL(O_NOT),              NULL,           "!"   },
        { T_SYMBOL(O_COMPLEMENT),       NULL,           "~"   },
        { T_SYMBOL(O_PLUS_PLUS),        NULL,           "++"  },
        { T_SYMBOL(O_MINUS_MINUS),      NULL,           "--"  },
        { T_SYMBOL(O_COLON),            NULL,           ":"   },
        { T_SYMBOL(O_OSQUARE),          NULL,           "["   },
        { T_SYMBOL(O_CSQUARE),          NULL,           "]"   },
        { T_SYMBOL(O_DOT),              NULL,           "."   },
        { 0 }
#undef  T_SYMBOL
    };

typedef struct {
    int                 type;
    int                 errors;
    int                 overflow;
    accint_t            ival;
    accfloat_t          fval;
} numeric_t;

static int              streamget(lexer_t *lexer);
static int              lexget(lexer_t *lexer);
static int              lexunget(lexer_t *lexer, int ch);

static int              get_line(lexer_t *lexer);
static void             get_while(lexer_t *lexer, int chclass);
static void             get_string(lexer_t *lexer, int wchar);
static void             get_unquoted(lexer_t *lexer, const int end);
static const char *     utf8_decode(const void *src, const void *cpend, int *result);
static int              utf8_encode(int ch, void *buffer);
static int              get_character(lexer_t *lexer, int wchar);
static int              get_escape(lexer_t *lexer, unsigned wchar, unsigned limit, int *value);

static int              get_numeric(lexer_t *lexer, numeric_t *value, int ch);
static int              get_float(lexer_t *lexer, numeric_t *value, char *buffer, unsigned cursor, int base);
static int              convertnumeric(numeric_t *value, int base, const char *buffer, unsigned length);
static int              convertfloat(numeric_t *value, int type, const char *buffer, unsigned length);

static int              kwlookup(const char *str);
static void             newline(void);
static void             filename_free(void);

#define yybuffer(extra) \
    if ((yyleng + (extra)) >= yysize) { \
        yysize += INC_YYTEXT; \
        yytext = (char *) chk_realloc(yytext, yysize+1); \
    }

#define RETURN          return

static lexer_t          stdstream = {
        streamget
    };

FILE *                  x_lexfp = NULL;     /* File pointer for current input file */

int                     x_columnno = 0;     /* Start column of last token so we can print helpful error messages */
int                     x_lineno = 0;       /* Current line number */

char *                  yytext = NULL;      /* Working buffer (dynamicly resized) */
static unsigned         yysize;             /* Number of bytes allocated to yytext */
static unsigned         yyleng;             /* Number of bytes in string */

static Head_p           hd_filenames;       /* List of filenames included (needed so we can put
                                             * correct file name with each symbol defined
                                             * without duplicated filename for each symbol)
                                             */
/*
 *  init_lex ---
 *      Called to initialise lexer when a new file is processed.
 */
void
init_lex(void)
{
    yymsginit();

    x_lineno = 1;
    stdstream.l_unidx = 0;
    x_columnno = 0;

    filename_free();
    if (0 == yysize) {
        yysize = INC_YYTEXT;
        yytext = (char *) chk_alloc(yysize);
    }
}


int
yylex(void)
{
    return yylexer(&stdstream, TRUE /*expand_symbols*/);
}


int
yylexer(lexer_t *lexer, int expand_symbols)
{
    register int ch;

    for (;;) {
        ch = lexget(lexer);
        if (ISSPACE(ch)) {
            continue;
        }

        /* Check for a symbol and save in yytext keep expanding yytext if too big */
        if (ISSYMBOL(ch)) {
            symbol_t *sp;
            int k;

            /*
             *  Wide strings/characters
             */
            if ('L' == ch) {
                const int nch = lexget(lexer);

                if ('"' == nch) {               /* L"abc" */
                    get_string(lexer, TRUE);
                    yylval.xval = (lexer->l_flags & LEX_FNOSALLOC ? yytext : chk_salloc(yytext));
                    RETURN(O_STRING_CONST);

                } else if ('\'' == nch) {       /* L'a' */
                    yylval.ival = get_character(lexer, TRUE);
                    RETURN(O_INTEGER_CONST);

                }
                lexunget(lexer, nch);

            } else if ('R' == ch) {
                const int nch = lexget(lexer);

                if ('"' == nch) {               /* R"abc", C11 style raw strings */
                    get_unquoted(lexer, nch);
                    yylval.xval = (lexer->l_flags & LEX_FNOSALLOC ? yytext : chk_salloc(yytext));
                    RETURN(O_STRING_CONST);
                }
                lexunget(lexer, nch);
            }

            /*
             *  Symbols
             */
            lexunget(lexer, ch);
            get_while(lexer, XSYMBOL | XDIGIT);

            if (expand_symbols) {
                if ((k = kwlookup(yytext)) > 0) {
                    return k;                   /* keyword */
                }

                if (NULL != (sp = sym_lookup(yytext, SYMLK_FUNDEF))) {
                    if (SC_TYPEDEF == ((sp->s_type & SC_MASK) >> SC_SHIFT)) {
                        /*
                         *  Typedef
                         */
                        yylval.sym = sp;
                        yylval.sval = yytext;
                        return O_TYPEDEF_NAME;  /* typedef */
                    }
                }
            }

            yylval.xval = (lexer->l_flags & LEX_FNOSALLOC ? yytext : chk_salloc(yytext));
            RETURN(O_SYMBOL);
        }

        if ('"' == ch) {                        /* "abc" */
            get_string(lexer, FALSE);
            yylval.xval = (lexer->l_flags & LEX_FNOSALLOC ? yytext : chk_salloc(yytext));
            RETURN(O_STRING_CONST);

        } else if ('`' == ch && xf_grunch) {
            get_unquoted(lexer, ch);            /* `abc` */
            yylval.xval = (lexer->l_flags & LEX_FNOSALLOC ? yytext : chk_salloc(yytext));
            RETURN(O_STRING_CONST);
        }

        if (ISDIGIT(ch) || '.' == ch) {
            numeric_t numeric = {0};
            int type = get_numeric(lexer, &numeric, ch);

            switch (type) {
            case O_DOT:
            case K_DOTS:
                yylval.ival = type;
                return type;

            case O_INTEGER_CONST:
#if defined(O_LONG_INTEGER_CONST)
            case O_LONG_INTEGER_CONST:
#endif
                yylval.ival = numeric.ival;
                return type;

            case O_FLOAT_CONST:
            case O_DOUBLE_CONST:
            case O_LONG_DOUBLE_CONST:
                yylval.fval = numeric.fval;
                return type;

            default:
                yyerrorf("syntax error reading numeric constant (type:%d)", type);
                break;
            }
            yyerror("syntax error reading numeric constant");
            continue;
        }

        switch (ch) {
        case '\r':
            continue;
        case '\n':
            newline();
            continue;
        case '(':
            RETURN(O_OROUND);
        case ')':
            RETURN(O_CROUND);
        case '{':
            RETURN(O_OCURLY);
        case '}':
            RETURN(O_CCURLY);
        case ',':
            RETURN(O_COMMA);
        case ';':
            RETURN(O_SEMICOLON);
        case ':':
            RETURN(O_COLON);
        case '=':
            ch = lexget(lexer);
            if ('=' == ch)
                RETURN(O_EQ_OP);
            lexunget(lexer, ch);
            RETURN(O_EQ);
        case '!':
            ch = lexget(lexer);
            if ('=' == ch)
                RETURN(O_NE_OP);
            lexunget(lexer, ch);
            RETURN(O_NOT);
        case '>':
            ch = lexget(lexer);
            if ('=' == ch)
                RETURN(O_GE_OP);
            if ('>' == ch) {
                ch = lexget(lexer);
                if ('=' == ch)
                    RETURN(O_RSHIFT_EQ);
                lexunget(lexer, ch);
                RETURN(O_RSHIFT);
            }
            lexunget(lexer, ch);
            RETURN(O_GT_OP);
        case '<':
            ch = lexget(lexer);
            if ('=' == ch) {
                ch = lexget(lexer);
                if ('>' == ch)
                    RETURN(O_CMP_OP);
                lexunget(lexer, ch);
                RETURN(O_LE_OP);
            }
            if ('<' == ch) {
                ch = lexget(lexer);
                if ('=' == ch)
                    RETURN(O_LSHIFT_EQ);
                lexunget(lexer, ch);
                RETURN(O_LSHIFT);
            }
            lexunget(lexer, ch);
            RETURN(O_LT_OP);
        case '+':
            ch = lexget(lexer);
            if ('=' == ch)
                RETURN(O_PLUS_EQ);
            if ('+' == ch)
                RETURN(O_PLUS_PLUS);
            lexunget(lexer, ch);
            RETURN(O_PLUS);
        case '-':
            ch = lexget(lexer);
            if ('=' == ch)
                RETURN(O_MINUS_EQ);
            if ('-' == ch)
                RETURN(O_MINUS_MINUS);
            if ('>' == ch)
                RETURN(O_ARROW);
            lexunget(lexer, ch);
            RETURN(O_MINUS);
        case '*':
            ch = lexget(lexer);
            if ('=' == ch)
                RETURN(O_MUL_EQ);
            lexunget(lexer, ch);
            RETURN(O_MUL);
        case '/':
            ch = lexget(lexer);
            if ('=' == ch)
                RETURN(O_DIV_EQ);
            if ('*' == ch) {
                while (1) {
                    if (0 == (ch = lexget(lexer)))
                        break;                  /* eof */
                    if ('\n' == ch)
                        newline();
                    if (ch != '*')
                        continue;
                    ch = lexget(lexer);
                    if ('/' == ch)
                        break;
                    lexunget(lexer, ch);
                }
                continue;
            }
            if ('/' == ch) {                    /* allow // style comments */
                while (0 != (ch = lexget(lexer)) && ch != '\n') /* .. inline as preprocessor may not support */
                    /**/;
                newline();
                continue;
            }
            lexunget(lexer, ch);
            RETURN(O_DIV);
        case '%':
            ch = lexget(lexer);
            if ('=' == ch)
                RETURN(O_MOD_EQ);
            lexunget(lexer, ch);
            RETURN(O_MOD);
        case '^':
            ch = lexget(lexer);
            if ('=' == ch)
                RETURN(O_XOR_EQ);
            lexunget(lexer, ch);
            RETURN(O_XOR);
        case '&':
            ch = lexget(lexer);
            if ('=' == ch)
                RETURN(O_AND_EQ);
            if ('&' == ch)
                RETURN(O_CAND);
            lexunget(lexer, ch);
            RETURN(O_AND);
        case '|':
            ch = lexget(lexer);
            if ('=' == ch)
                RETURN(O_OR_EQ);
            if ('|' == ch)
                RETURN(O_COR);
            lexunget(lexer, ch);
            RETURN(O_OR);
        case '[':
            RETURN(O_OSQUARE);
        case ']':
            RETURN(O_CSQUARE);
        case '?':
            RETURN(O_QUESTION);
        case '~':
            RETURN(O_COMPLEMENT);
        case '#': {
                int columnno = x_columnno;
                const char *cp;
                const int newlines = get_line(lexer);

                for (cp = yytext; ' ' == *cp; ++cp)
                    ;                           /* consume leading white-space */

                if (0 == strncmp(cp, "pragma", 6)) {
                    /*
                     *  pragma's
                     */
                    const char *pragma = chk_salloc(cp + 6);
                    int lineno = x_lineno;

                    x_lineno -= newlines;
                    pragma_process(pragma);
                    chk_free((void *)pragma);
                    x_lineno = lineno;

                } else if (1 == columnno || 0 == strncmp(cp, "line", 4)) {
                    /*
                     *  line directive, for example:
                     *
                     *      # 644 "grief.h"
                     *      #line 644 "grief.h"
                     *      #line 700
                     */
                    int lineno;

                    while (*cp && !ISDIGIT((int) *cp)) {
                        ++cp;                   /* consume leading characters */
                    }

                    if (*cp && (lineno = atoi(cp)) > 0) {
                        static char filename[MAX_PATH+1];
                        const char *end = (filename + sizeof(filename)) - 1;
                        char *p = filename;

                        x_lineno = lineno;      /* line-number */

                        while (*cp && '"' != *cp && '<' != *cp) {
                            ++cp;
                        }

                        if (*cp) {              /* trailing optional filename */
                            const char term = ('"' == *cp ? '"' : '>');

                            x_filename = filename;
                            for (++cp; *cp && *cp != term; ++cp) {
                                if (p < end) {
                                    *p++ = *cp;
                                }
                            }
                            *p = '\0';
                        }
                    }
                }
            }
            continue;

        case '\'':
            yylval.ival = get_character(lexer, FALSE);
            RETURN(O_INTEGER_CONST);
        case 0:
            RETURN(0);
        }

        crerrorx(RC_SYNTAX_TOKEN, "syntax error, invalid token/character '%c'", ch);
    }
}


static int
lexget(lexer_t *lexer)
{
    if (lexer->l_unidx) {
        const int ch = lexer->l_unbuffer[--lexer->l_unidx];
        assert(ch);
        return ch;
    }
    return lexer->get(lexer);
}


static int
lexunget(lexer_t *lexer, int ch)
{
    assert(lexer->l_unidx < (sizeof(lexer->l_unbuffer)/sizeof(lexer->l_unbuffer[0])));
    if (ch) {
        lexer->l_unbuffer[lexer->l_unidx++] = (char)ch;
    }
    return 0;
}


static int
streamget(lexer_t *lexer)
{
    int ch;

    __UNUSED(lexer)

    ++x_columnno;
    ch = fgetc(x_lexfp);
    if (0 == ch) {
        if (cronceonly(ONCEONLY_NUL_CHARACTER)) {
            crerror(RC_NUL_CHARACTER, "NUL character(s) encountered within the input");
        }
        ch = ' ';
    }
    return (ch == EOF ? 0 : ch);
}


/*  Function:           get_line
 *      Process and return the result text until the end-of-line.
 *
 *  Parameters:
 *      lexer -             Lexer stream.
 *
 *  Returns:
 *      Newlines consumed.
 */
static int
get_line(lexer_t *lexer)
{
    register int ch;
    int ch2, lines = 0;

    yyleng = 0;
    while (1) {
        yybuffer(16);

        ch = lexget(lexer);

        if (0 == ch || '\r' == ch || '\n' == ch) {
            goto eol;

        } else if ('\\' == ch) {
            if ((ch = lexget(lexer)) == '\n') {
                newline();                      /* line continuation */
                ++lines;
                continue;
            }
            yytext[yyleng++] = '\\';

        } else if ('/' == ch) {
            if ((ch = lexget(lexer)) == '*') {  /* comment */
                for (ch = ch2 = 0;;) {
                    ch = lexget(lexer);

                    if (0 == ch || '\r' == ch || '\n' == ch) {
                        goto eol;

                    } else if ('*' == ch2 && '/' == ch) {
                        break;                  /* eos */
                    }
                    ch2 = ch;
                }
                continue;
            }

            yytext[yyleng++] = '/';
            if (0 == ch || '\r' == ch || '\n' == ch)
                goto eol;
        }

        yytext[yyleng++] = (char) ch;
    }
    return lines;

eol:;
    yytext[yyleng] = '\0';
    newline();
    return (lines + 1);
}


/*  Function:           get_while
 *      Process and return the result text until the character-class is no-longer
 *      available within the stream.
 *
 *  Parameters:
 *      lexer -             Lexer stream.
 *      chclass -           Character class map.
 *
 *  Returns:
 *      nothing
 */
static void
get_while(lexer_t *lexer, int chclass)
{
    register int ch;

    yyleng = 0;
    while (1) {
        yybuffer(0);

        ch = lexget(lexer);

        if ((lextab[ch] & chclass) == 0) {
            lexunget(lexer, ch);
            yytext[yyleng] = '\0';
            return;
        }

        yytext[yyleng++] = (char) ch;
    }
}


/*  Function:           get_string
 *      Process and return a string constant containing either normal or wide-characters.
 *
 *  Parameters:
 *      lexer -             Lexer stream.
 *      wchar -             *true* if a wide-character, otherwise standard.
 *
 *  Returns:
 *      nothing
 */
static void
get_string(lexer_t *lexer, int wchar)
{
    int once = 0, ch;

    yyleng = 0;
    while (1) {
        yybuffer(2);

        ch = lexget(lexer);

        if ('"' == ch) {                        /* end-of-string */
            if (yyleng > (32 * 1024)) {
                crwarn(RC_STRING_LENGTH, "string length exceeds 32k");
            }
            yytext[yyleng] = '\0';
            return;
        }

        if ('\\' == ch) {                       /* quote next */
            if ('\n' == (ch = lexget(lexer))) {
                newline();

            } else if (ch) {
                int status, eh = 0;

                lexunget(lexer, ch);

                if (1 == (status = get_escape(lexer, wchar, TRUE, &eh))) {
                    /*
                     *  conversion error
                     *      If a backslash precedes a character that does not appear in
                     *      the table, handle the undefined character as the character
                     *      itself. For example, \c is treated as an c.
                     */
                    yytext[yyleng++] = (char)eh;

                } else if (status >= 0) {
                    /*
                     *  encode results/
                     *      note, escape sequences *should* match crbin.c
                     */
                    switch (eh) {
                    case '\a': eh = 'a'; break; /* alert */
                    case '\b': eh = 'b'; break; /* backspace */
                    case 0x1b: eh = 'e'; break; /* ESC */
                    case '\f': eh = 'f'; break; /* formfeed */
                    case '\n': eh = 'n'; break; /* newline */
                    case '\r': eh = 'r'; break; /* return */
                    case '\t': eh = 't'; break; /* tab */
                    case '\v': eh = 'v'; break; /* vertical tab */
                    case '?':                   /* question */
                    case '\"':                  /* double quote */
                    case '\'':                  /* quote */
                    case '\\':                  /* backslash */
                        break;

                    case -1:                    /* unknown, unquote */
                        yytext[yyleng++] = (char)ch;
                        continue;

                    default:
                        if (eh < 32 || eh >= 0x7f) {
                            /*
                             *  recoding using a suitable format/
                             *      suitable being the closest format to the orginal.
                             */
                            yybuffer(16);       /* export result */

                            if (3 == status) {  /* UTF-8/UNICODE character */
                                yyleng += utf8_encode(eh, yytext);

                            } else {            /* hex or octal */
                                char *cp, buffer[16];

                                if (wchar && (eh & 0xFFFFFF00)) {
                                    if (! once) {
                                        crwarn(RC_CHARACTER_MULTI,
                                            "multi-character character constant");
                                        ++once;
                                    }
                                }
                                                /* extended octal or hex formats */
                                if (eh > 0xffff) {
                                    if ('0' == ch) {
                                        sprintf(buffer, "\\o{%o}", eh);
                                    } else {
                                        sprintf(buffer, "\\x{%x}", eh);
                                    }
                                                /* \\0ooo */
                                } else if (eh <= 0xff && '0' == ch)  {
                                    sprintf(buffer, "\\0%o", eh);

                                } else {        /* \xff or \Xffff */
                                    sprintf(buffer, "\\%c%.*x",
                                        ((eh <= 0xff) ? 'x' : 'X'),
                                        ((eh <= 0xff) ?  2  : 4), eh);
                                }

                                for (cp = buffer; *cp; ++cp) {
                                    yytext[yyleng++] = *cp;
                                }
                            }
                            continue;
                        }
                    }
                    yytext[yyleng++] = '\\';
                    yytext[yyleng++] = (char)eh;

                } else {
                    yytext[yyleng++] = '\\';
                    yytext[yyleng++] = '\\';
                }
                continue;
            }
        }

        if (0 == ch) {                          /* end-of-file */
            crerror(RC_STRING_UNTERMINATED, "unterminated string constant");
            yytext[yyleng] = '\0';
            return;
        }

        if ('\r' == ch || '\n' == ch) {         /* end-of-line */
            crerror(RC_STRING_NEWLINE, "newline within string constant");
            yytext[yyleng] = '\0';
            return;
        }

        yytext[yyleng++] = (char) ch;
    }
}


/*  Function:           get_unquoted
 *      Process and return a unquoted string constant containing either normal or wide-characters.
 *
 *  Parameters:
 *      lexer -             Lexer stream.
 *
 *  Returns:
 *      nothing
 */
static void
get_unquoted(lexer_t *lexer, const int end)
{
    int ch;

    yyleng = 0;
    while (1) {
        yybuffer(2);

        ch = lexget(lexer);

        if (end == ch) {                        /* end-of-string */
            if (yyleng > (32 *1024)) {
                crwarn(RC_STRING_LENGTH, "string length exceeds 32k");
            }
            yytext[yyleng] = '\0';
            return;
        }

        if (0 == ch) {                          /* end-of-file */
            crerror(RC_STRING_UNTERMINATED, "unterminated string constant");
            yytext[yyleng] = '\0';
            return;
        }

        if ('\r' == ch || '\n' == ch) {         /* end-of-line */
            crerror(RC_STRING_NEWLINE, "newline within string constant");
            yytext[yyleng] = '\0';
            return;
        }

        yytext[yyleng++] = (char) ch;
    }
}

/*
 *  00000000-01111111  00-7F  0-127     Single-byte encoding (compatible with US-ASCII).
 *  10000000-10111111  80-BF  128-191   Second, third, or fourth byte of a multi-byte sequence.
 *  11000000-11000001  C0-C1  192-193   Overlong encoding: start of 2-byte sequence, but would encode a code point 127.
 *  11000010-11011111  C2-DF  194-223   Start of 2-byte sequence.
 *  11100000-11101111  E0-EF  224-239   Start of 3-byte sequence.
 *  11110000-11110100  F0-F4  240-244   Start of 4-byte sequence.
 *  11110101-11110111  F5-F7  245-247   Restricted by RFC 3629: start of 4-byte sequence for codepoint above 10FFFF.
 *  11111000-11111011  F8-FB  248-251   Restricted by RFC 3629: start of 5-byte sequence.
 *  11111100-11111101  FC-FD  252-253   Restricted by RFC 3629: start of 6-byte sequence.
 *  11111110-11111111  FE-FF  254-255   Invalid: not defined by original UTF-8 specification.
 */
static const char *
utf8_decode(const void *src, const void *cpend, int *result)
{
    register const unsigned char *t_src = (const unsigned char *)src;
    unsigned char ch;
    int32_t ret = 0;
    int remain;

    /*
    //  Bits    Last code point     Byte 1      Byte 2      Byte 3      Byte 4      Byte 5      Byte 6
    //  7       U+007F              0xxxxxxx
    //  11      U+07FF              110xxxxx    10xxxxxx
    //  16      U+FFFF              1110xxxx    10xxxxxx    10xxxxxx
    //  21      U+1FFFFF            11110xxx    10xxxxxx    10xxxxxx    10xxxxxx
    //  26      U+3FFFFFF           111110xx    10xxxxxx    10xxxxxx    10xxxxxx    10xxxxxx
    //  31      U+7FFFFFFF          1111110x    10xxxxxx    10xxxxxx    10xxxxxx    10xxxxxx    10xxxxxx
    */
    assert(src < cpend);
    ch = *t_src++;

    if (ch & 0x80) {
                                                /* C0-C1  192-193  Overlong encoding: start of 2-byte sequence. */
        if ((ch & 0xE0) == 0xC0) {              /* C2-DF  194-223  Start of 2-byte sequence. */
            remain = 1;
            ret = ch & 0x1F;

        } else if ((ch & 0xF0) == 0xE0) {       /* E0-EF  224-239  Start of 3-byte sequence. */
            remain = 2;
            ret = ch & 0x0F;

        } else if ((ch & 0xF8) == 0xF0) {       /* F0-F4  240-244  Start of 4-byte sequence. */
            remain = 3;
            ret = ch & 0x07;

        } else if ((ch & 0xFC) == 0xF8) {       /* F8-FB  248-251  Start of 5-byte sequence. */
            remain = 4;
            ret = ch & 0x03;

        } else if ((ch & 0xFE) == 0xFC) {       /* FC-FD  252-253  Start of 6-byte sequence. */
            remain = 5;
            ret = ch & 0x01;

        } else {                                /* invalid continuation (0x80 - 0xbf). */
            ret = -ch;
            goto done;
        }

        while (remain--) {
            if (t_src >= (const unsigned char *)cpend) {
                ret = -ret;
                goto done;
            }
            ch = *t_src++;
            if (0x80 != (0xc0 & ch)) {          /* invalid secondary byte (0x80 - 0xbf). */
                --t_src;
                ret = -ret;
                goto done;
            }
            ret <<= 6;
            ret |= (ch & 0x3f);
        }
    } else {
        ret = ch;
    }

done:;
    *result = ret;
    return (const void *)t_src;
}


static int
utf8_encode(int ch, void *buffer)
{
    register unsigned char *t_buffer = (unsigned char *)buffer;
    register unsigned t_ch = (unsigned) ch;
    int count = 0;

    if (ch < 0) {
        count = 0;

    } else if (t_ch < 0x80) {
        t_buffer[0] = (unsigned char)t_ch;
        count = 1;

    } else if (t_ch < 0x800) {
        t_buffer[0] = (unsigned char)(0xC0 | ((t_ch >> 6)  & 0x1F));
        t_buffer[1] = (unsigned char)(0x80 |  (t_ch        & 0x3F));
        count = 2;

    } else if (t_ch < 0x10000) {
        t_buffer[0] = (unsigned char)(0xE0 | ((t_ch >> 12) & 0xF));
        t_buffer[1] = (unsigned char)(0x80 | ((t_ch >> 6)  & 0x3F));
        t_buffer[2] = (unsigned char)(0x80 |  (t_ch        & 0x3F));
        count = 3;

    } else if (t_ch < 0x200000) {
        t_buffer[0] = (unsigned char)(0xF0 | ((t_ch >> 18) & 0x7));
        t_buffer[1] = (unsigned char)(0x80 | ((t_ch >> 12) & 0x3F));
        t_buffer[2] = (unsigned char)(0x80 | ((t_ch >> 6)  & 0x3F));
        t_buffer[3] = (unsigned char)(0x80 |  (t_ch        & 0x3F));
        count = 4;

    } else if (t_ch < 0x4000000) {
        t_buffer[0] = (unsigned char)(0xF8 | ((t_ch >> 24) & 0x3));
        t_buffer[1] = (unsigned char)(0x80 | ((t_ch >> 18) & 0x3F));
        t_buffer[2] = (unsigned char)(0x80 | ((t_ch >> 12) & 0x3F));
        t_buffer[3] = (unsigned char)(0x80 | ((t_ch >> 6)  & 0x3F));
        t_buffer[4] = (unsigned char)(0x80 |  (t_ch        & 0x3F));
        count = 5;

    } else {
        t_buffer[0] = (unsigned char)(0xFC | ((t_ch >> 30) & 0x1));
        t_buffer[1] = (unsigned char)(0x80 | ((t_ch >> 24) & 0x3F));
        t_buffer[2] = (unsigned char)(0x80 | ((t_ch >> 18) & 0x3F));
        t_buffer[3] = (unsigned char)(0x80 | ((t_ch >> 12) & 0x3F));
        t_buffer[4] = (unsigned char)(0x80 | ((t_ch >> 6)  & 0x3F));
        t_buffer[5] = (unsigned char)(0x80 |  (t_ch        & 0x3F));
        count = 6;
    }
    return count;
}


/*  Function:           get_character
 *      Process and return a character constant containing either normal or wide-characters.
 *
 *  Parameters:
 *      lexer -             Lexer stream.
 *      wchar -             *true* if a wide-character, otherwise standard.
 *
 *  Returns:
 *      Character value.
 */
static int
get_character(lexer_t *lexer, int wchar)
{
    char buffer[6 + 1] = {0};                   /* UTF8 working buffer */
    int ch, error = 0;
    int value = 0;

    ch = lexget(lexer);
    if ('\'' == ch) {
        crerror(RC_CHARACTER_EMPTY, "empty character constant");
        ++error;

    } else {
        unsigned nchars = 0;

        for (;;) {
bad:;       if (0 == ch) {
                crerror(RC_CHARACTER_UNTERMINATED, "unterminated character constant");
                ++error;
                break;

            } else if ('\r' == ch || '\n' == ch) {
                crerror(RC_CHARACTER_NEWLINE, "newline in character constant");
                ++error;
                break;

            } else if ('\\' == ch) {
                if (get_escape(lexer, wchar, FALSE, &ch) < 0) {
                    goto bad;
                }
            }

            value = ch;
            if (nchars < (sizeof(buffer) - 1))
                buffer[nchars] = (char)ch;
            ++nchars;

            if ('\'' == (ch = lexget(lexer))) {
                break;                          /* end of constant */
            }
        }

        if (nchars > 1 && !error) {
            if (!wchar) {
                crerrorx(RC_CHARACTER_WIDE, "character constant too large");
                ++error;

            } else if (nchars > 6 ||
                    utf8_decode(buffer, buffer + nchars, &value) != (buffer + nchars)) {
                crerrorx(RC_CHARACTER_WIDE, "invalid multi-character character constant");
                ++error;
            }
        }
    }

#if (SIGNEDCHAR)
    if (signedchar && (value & 0xFFFFFF80) == 0x80) {
        value |= 0xFFFFFF00;                    /* if sign-bit is on, sign extend it */
    }
#endif

    if (!error) {
        if (wchar && (value & 0xFFFFFF00)) {
            crwarn(RC_CHARACTER_MULTI, "multi-character character constant");
        }
    }

    return value;
}


/*  Function:           get_escape
 *      Process and return a character escape-sequence containing either normal or
 *      wide-characters.
 *
 *  Supported Escapes:
 *
 *      o fixed
 *
 *          '\a'            Alert
 *          '\b'            Backspace
 *          '\f'            Formfeed
 *          '\e'            ESC
 *          '\n'            Newline
 *          '\r'            Return
 *          '\t'            Tab
 *          '\v'            Vertical tab
 *          '?';            Literal question mark
 *          '\"'            Double quote
 *          '\''            Signal quote
 *          '\\'            Backslash
 *
 *      o Sequences
 *
 *          '\cx'           Control character (character code point value).
 *
 *          '\x##'          Hexidecimal encoded character codes.
 *          '\X####'
 *          '\x{# ...}'
 *
 *          '\0##'          Octal encoded character codes.
 *          '\o{# ...}'
 *
 *          '\d###          Decimal encoded character code.
 *
 *          '\u####'        Unicode 16 character code, values must be complete;
 *                          disallowing nul's, surrogates and reserved constants.
 *
 *          '\U########     Unicode 32 character code.
 *
 *  Parameters:
 *      lexer -             Lexer stream.
 *      wchar -             *true* if a wide-character, otherwise standard.
 *      limit -             *true* if limit parsing to end-of-escape.
 *      value -             Returning value.
 *
 *  Returns:
 *      Return zero(0) on success, otherwise
 *
 *          -1      End-of-line condition.
 *          1       Conversion errors.
 *          2       Overflow.
 *          3       UNICODE.
 */
static int
get_escape(lexer_t *lexer, unsigned wchar, unsigned limit, int *value)
{
    int status = 0, ch;

    if (0 == (ch = lexget(lexer)) || '\n' == ch || '\r' == ch) {
        *value = ch;
        return -1;
    }

    if (ch >= '0' && ch <= '7') {
        /*
         *  Octal,  \0[00]
         */
        unsigned length = 1;
        int n = 0;

        limit = (limit ? 3 : (unsigned)-1);
        do {
            n = (n << 3) + (ch - '0');

            if (n > 0377 && !wchar) {
                if (! status) {
                    crerrorx(RC_CHARACTER_WIDE,
                        "octal character constant too long '\\%o'", n);
                }
                status = 1;
                n &= 0377;                      /* mask off high bits */
            }

            if (0 == (ch = lexget(lexer)) || '\n' == ch || '\r' == ch) {
                *value = ch;
                return -1;
            }

        } while (++length <= limit && (ch >= '0' && ch <= '7'));

        lexunget(lexer, ch);
        ch = n;

    } else if ('o' == ch) {
        /*
         *  Octal escape sequence (Perl style)
         *
         *      \o{....}        Unrestricted length octal
         */
        int n = 0;

        if (0 == (ch = lexget(lexer)) || '\n' == ch || '\r' == ch) {
            *value = ch;
            return -1;
        }

        if ('{' == ch) {
            while (1) {
                if (0 == (ch = lexget(lexer)) || '\n' == ch || '\r' == ch) {
                    crerror(RC_OCTAL_NEWLINE, "newline within octal constant");
                    status = 1;
                    break;
                }

                if (ch >= '0' && ch <= '7') {
                    n = (n << 3) + (ch - '0');
                } else if ('}' == ch) {
                    break;
                } else {
                    crerrorx(RC_CHARACTER_OCTAL,
                        "invalid octal character '\\%o'", ch);
                    status = 1;
                }
            }
        } else {
            status = 1;
            n = 'o';
        }

        if ((wchar && (n & 0xFF000000)) || (!wchar && (n & 0xFFFFFF00))) {
            if (! status) {
                crerrorx(RC_CHARACTER_WIDE,
                    "octal character constant too large \\%o", n);
            }
            status = 2;
            ch = n;

        } else if ((ch = n) > 0xff) {
            if (0 == n || n > 0x10ffff ||
                    0xffff == (n & 0xffff) || 0xfffe == (n & 0xffff) ||
                    (n >= 0xd800 && n < 0xe000)) {
                crerrorx(RC_UNICODE_INVALID, "invalid unicode constant");
                ch = 0xfffd;
            }
            status = 3;
        }

    } else if ('x' == ch || 'u' == ch || 'U' == ch) {
        /*
         *  Hexidecimal or Unicode escape sequence
         *
         *      \xHH            Hexadecimal char (encoded byte value).
         *      \x{....}        Unrestricted length hexidecimal (Perl/Java).
         *
         *      \u#### or       Unicode, values must be complete; disallowing nul's, surrogates
         *                      and reserved constants.
         *      \U########
         */
        int unicode = ('x' == ch ? 0 : 1);
        unsigned length = 0;
        int n = 0;

        if (unicode) {
            limit = ('u' == ch ? 4 : 8);        /* #### or ######## */
        } else {
            limit = (limit ? (wchar ? 4 : 2) : 0xff);
        }

        while (length < limit) {
            if (0 == (ch = lexget(lexer)) || '\n' == ch || '\r' == ch) {
                if (16 == limit) {
                    crerror(RC_HEX_NEWLINE, "newline within hexidecimal escape constant");
                    status = 1;
                }
                *value = ch;
                return -1;
            }

            if (! ISXDIGIT(ch)) {
                if ('{' == ch) {
                    if (16 != limit) {
                        limit = 17;             /* 64bit */
                        continue;
                    }
                } else if ('}' == ch && 17 == limit) {
                    break;
                }
                lexunget(lexer, ch);
                break;
            }

            if (ISDIGIT(ch)) {
                n = (n << 4) + (ch - '0');
            } else if (isupper(ch)) {
                n = (n << 4) + (ch - 'A' + 10);
            } else {
                n = (n << 4) + (ch - 'a' + 10);
            }

            ++length;
        }

        if (17 == limit && '}' != ch) {
            crerror(RC_CHARACTER_HEX, "invalid hexidecimal constant, missing trailing '}'");
            status = 1;
            ch = 'x';

        } else if (0 == length) {
            crerror(RC_CHARACTER_HEX, "empty hexidecimal constant");
            status = 1;
            ch = 'x';

        } else if (unicode && length < limit) {
            crerrorx(RC_UNICODE_SHORT, "incomplete unicode constant");
            ch = 0xfffd;

        } else if ((wchar && (n & 0xFF000000)) || (!wchar && (n & 0xFFFFFF00))) {
            if (! status) {
                crerrorx(RC_CHARACTER_WIDE,
                    "hexidecimal character constant too large \\x%x", n);
            }
            status = 2;
            ch = n;

        } else if ((ch = n) > 0xff && unicode) {
            if (0 == n || n > 0x10ffff ||
                    0xffff == (n & 0xffff) || 0xfffe == (n & 0xffff) ||
                (n >= 0xd800 && n < 0xe000)) {
                crerrorx(RC_UNICODE_INVALID, "invalid unicode constant \\x%x", n);
                ch = 0xfffd;
            }
            status = 3;
        }

    } else if ('c' == ch) {
        /*
         *  Control character (Perl/Java)
         *
         *      \cx         Control char (character code point value)
         *
         *          \cA or \ca          U+0001 START OF HEADING
         *          \cB or \cb          U+0002 START OF TEXT
         *          \cC or \cc          U+0003 END OF TEXT
         *          \cD or \cd          U+0004 END OF TRANSMISSION
         *          \cE or \ce          U+0005 ENQUIRY
         *          \cF or \cf          U+0006 ACKNOWLEDGE
         *          \cG or \cg          U+0007 BELL
         *          \cH or \ch          U+0008 BACKSPACE
         *          \cI or \ci          U+0009 CHARACTER TABULATION
         *          \cJ or \cj          U+000A LINE FEED (LF)
         *          \cK or \ck          U+000B LINE TABULATION
         *          \cL or \cl          U+000C FORM FEED (FF)
         *          \cM or \cm          U+000D CARRIAGE RETURN (CR)
         *          \cN or \cn          U+000E SHIFT OUT
         *          \cO or \co          U+000F SHIFT IN
         *          \cP or \cp          U+0010 DATA LINK ESCAPE
         *          \cQ or \cq          U+0011 DEVICE CONTROL ONE
         *          \cR or \cr          U+0012 DEVICE CONTROL TWO
         *          \cS or \cs          U+0013 DEVICE CONTROL THREE
         *          \cT or \ct          U+0014 DEVICE CONTROL FOUR
         *          \cU or \cu          U+0015 NEGATIVE ACKNOWLEDGE
         *          \cV or \cv          U+0016 SYNCHRONOUS IDLE
         *          \cW or \cw          U+0017 END OF TRANSMISSION BLOCK
         *          \cX or \cx          U+0018 CANCEL
         *          \cY or \cy          U+0019 END OF MEDIUM
         *          \cZ or \cz          U+001A SUBSTITUTE
         */
        if (0 == (ch = lexget(lexer)) || '\n' == ch || '\r' == ch) {
            crerror(RC_CONTROL_NEWLINE, "newline within control sequence constant");
            status = 1;
            *value = ch;
            return -1;
        }

        switch(ch) {
        case '[':       /* ESC */
            ch = 0x1b;
            break;
        case '?':       /* DEL */
            ch = 0x7f;
            break;
        default:        /* ^A -- ^Z, see above */
            if (isalpha(ch)) {
                ch = 'A' - toupper(ch);
            } else {
                crwarnx(RC_CHARACTER_CONTROL, "invalid control sequence '\\c-%c' (0x%x)", ch, ch);
                status = 1;
            }
            break;
        }

    } else {
        /*
         *  Fixed escape characters
         *
         *  Seq.    Dec     Hex     ASCII   Cntrl   Description.
         *  \a      7       07      BEL     \cG     alarm or bell
         *  \b      8       08      BS      \cH     backspace
         *  \e      27      1B      ESC     \c[     escape character
         *  \f      12      0C      FF      \cL     form feed
         *  \n      10      0A      LF      \cJ     line feed
         *  \r      13      0D      CR      \cM     carriage return
         *  \t      9       09      TAB     \cI     tab
         */
        switch (ch) {
        case 'a':  ch = '\a'; break;            /* alert */
        case 'b':  ch = '\b'; break;            /* backspace */
        case 'f':  ch = '\f'; break;            /* formfeed */
        case 'e':  ch = 0x1b; break;            /* ESC */
        case 'n':  ch = '\n'; break;            /* newline */
        case 'r':  ch = '\r'; break;            /* return */
        case 't':  ch = '\t'; break;            /* tab */
        case 'v':  ch = '\v'; break;            /* vertical tab */
        case '?':  ch = '?';  break;            /* literal question mark */
        case '\"': ch = '\"'; break;            /* double quote */
        case '\'': ch = '\''; break;            /* signal quote */
        case '\\': ch = '\\'; break;            /* backslash */
        default:
            crwarnx(RC_CHARACTER_ESCAPE, "unknown escape sequence '\\%c' (0x%x)", ch, ch);
            status = 1;
            break;
        }
    }

    *value = ch;
    return status;
}


static int
get_numeric(lexer_t *lexer, numeric_t *value, int ch)
{
    enum {
        SUFFIX_NONE = 0,
        SUFFIX_U    = (1 << 1),
        SUFFIX_I    = (1 << 2),
        SUFFIX_L    = (1 << 3),
        SUFFIX_LL   = (1 << 4),
        SUFFIX_F    = (1 << 5),
        SUFFIX_D    = (1 << 6),
        SUFFIX_UI   = SUFFIX_U | SUFFIX_I,
        SUFFIX_UL   = SUFFIX_U | SUFFIX_L,
        SUFFIX_ULL  = SUFFIX_U | SUFFIX_LL,
    } suffix = SUFFIX_NONE;

    char buffer[128+8];                         /* 128bit binary, plus prefix and suffix */
    unsigned cursor = 0;
    int error, base;

    /*
     *  consume digits
     */
    if ('.' == ch) {
        buffer[cursor++] = '.';
        ch = lexget(lexer);

        if ('.' == ch) {
            ch = lexget(lexer);
            if ('.' == ch) {
                return K_DOTS;                  /* ... */
            }
            yyerror("ellipsis has the syntax '...'");
            return O_DOT;
        }

        if (ISDIGIT(ch)) {                      /* .x */
            lexunget(lexer, ch);
            return get_float(lexer, value, buffer, cursor, 10);
        }

        lexunget(lexer, ch);
        return O_DOT;                           /* . */

    } else if ('0' == ch) {
        buffer[cursor++] = '0';
        ch = lexget(lexer);

        if ('x' == ch || 'X' == ch) {
            /*
             *  hex
             */
            int t_ch;

            buffer[cursor++] = (char)ch;
            t_ch = lexget(lexer);
            while (ISXDIGIT(t_ch)) {
                if (cursor < sizeof(buffer)-2) {
                    buffer[cursor++] = (char)t_ch;
                }
                t_ch = lexget(lexer);
            }

#if defined(HEX_FLOATS)
            if ('.' == t_ch || 'p' == t_ch || 'P' == t_ch) {
                buffer[cursor++] = (char)t_ch;
                return get_float(lexer, value, buffer, cursor, 16);
            }
#endif //HEX_FLOAT

            if (cursor <= 2) {
                lexunget(lexer, t_ch);
                base = 10;                      /* 0x only, treat as suffix (invalid) */
            } else {
                base = 16;
                ch = t_ch;
            }

        } else if ('b' == ch || 'B' == ch) {
            /*
             *   binary - extension
             */
            int t_ch;

            buffer[cursor++] = (char)ch;
            t_ch = lexget(lexer);
            while ('0' == t_ch || '1' == t_ch) {
                if (cursor < sizeof(buffer)-2) {
                    buffer[cursor++] = (char)t_ch;
                }
                t_ch = lexget(lexer);
            }

            if (cursor <= 2) {
                lexunget(lexer, t_ch);
                base = 10;                      /* 0b only, treat as suffix (invalid) */
            } else {
                base = 2;
                ch = t_ch;
            }

        } else {
            /*
             *  oct (maybe)
             */
            unsigned char digits = 0;

            while (ch >= '0' && ch <= '9') {
                digits |= (unsigned char) ch;
                if (cursor < sizeof(buffer)-2) {
                    buffer[cursor++] = (char)ch;
                }
                ch = lexget(lexer);
            }

            if ('.' == ch || 'e' == ch || 'E' == ch) {
                buffer[cursor++] = (char)ch;
                return get_float(lexer, value, buffer, cursor, 10);
            }

            base = (1 == cursor ? 10 : 8);
            if (digits & 0x08) {
                base = -8;                      /* 8 or 9 encountered */
            }
        }

    } else {
        /*
         *  decimal
         */
        base = 10;
        do {
            if (cursor < sizeof(buffer)-2) {
                buffer[cursor++] = (char)ch;
            }
            ch = lexget(lexer);
        } while (ch >= '0' && ch <= '9');

        if ('.' == ch || 'e' == ch || 'E' == ch) {
            buffer[cursor++] = (char)ch;
            return get_float(lexer, value, buffer, cursor, 10);
        }
    }

    /*
     *  convert
     */
    buffer[cursor] = 0;
    error = convertnumeric(value, base, buffer, cursor);

    /*
     *  intger-suffix:
     *      unsigned-suffix  long-suffixopt
     *      unsigned-suffix  long-long-suffix
     *      long-suffix      unsigned-suffix[opt]
     *      long-long-suffix unsigned-suffix[opt]
     *      float-suffix
     *      double-suffix
     *
     *  unsigned-suffix:        u|U
     *  long-suffix:            l|L
     *  long-long-suffix:       ll|LL
     *  float-suffix:           f
     *  double-suffix:          d
     */
    if (base) {
        unsigned t_cursor = cursor;

        if ('u' == ch || 'U' == ch) {
            buffer[cursor++] = (char)ch;
            ch = lexget(lexer);

            if ('l' == ch || 'L' == ch) {
                buffer[cursor++] = (char)ch;
                ch = lexget(lexer);

                if ('l' == ch || 'L' == ch) {
                    buffer[cursor++] = (char)ch;
                    suffix = SUFFIX_ULL;            /* unsigned long long */

                } else {
                    suffix = SUFFIX_UL;             /* unsigned long */
                    goto suffix_unknown;
                }

            } else if ('i' == ch || 'I' == ch) {
                buffer[cursor++] = (char)ch;
                suffix = SUFFIX_UI;                 /* unsigned int */

            } else {
                suffix = SUFFIX_U;                  /* unsigned */
                goto suffix_unknown;
            }

        } else if ('l' == ch || 'L' == ch) {
            buffer[cursor++] = (char)ch;
            ch = lexget(lexer);

            if ('u' == ch || 'U' == ch) {
                buffer[cursor++] = (char)ch;
                suffix = SUFFIX_UL;                 /* unsigned long */

            } else if ('l' == ch || 'L' == ch) {
                buffer[cursor++] = (char)ch;
                ch = lexget(lexer);

                if ('u' == ch || 'U' == ch) {
                    buffer[cursor++] = (char)ch;
                    suffix = SUFFIX_ULL;            /* unsigned long long */

                } else {
                    suffix = SUFFIX_LL;             /* long long */
                    goto suffix_unknown;
                }

            } else {
                suffix = SUFFIX_L;                  /* long */
                goto suffix_unknown;
            }

        } else if ('i' == ch || 'I' == ch) {        /* integer */
            buffer[cursor++] = (char)ch;
            suffix = SUFFIX_I;

        } else if ('f' == ch || 'F' == ch) {        /* float */
            buffer[cursor++] = (char)ch;
            suffix = SUFFIX_F;
            value->type = O_FLOAT_CONST;

        } else if ('d' == ch || 'D' == ch) {        /* C#, double */
            buffer[cursor++] = (char)ch;
            suffix = SUFFIX_D;
            value->type = O_DOUBLE_CONST;

        } else {
            goto suffix_unknown;

        }
        ch = lexget(lexer);

        /*
         *  process trailing character
         */
suffix_unknown:;
        if (0 == ch) {
            yyerror("unterminated integer constant");

        } else if (isalpha(ch)) {
            yyerrorf("invalid suffix '%.*s%c' on integer constant", cursor - t_cursor, buffer + t_cursor, ch);

        } else {
            if (SUFFIX_I == suffix || SUFFIX_UI == suffix) {
                /*
                 *  allow i64|32|16|8
                 */
                unsigned t_base = 0;

                while (ch >= '0' && ch <= '9') {
                    t_base = (t_base * 10) + (ch - '0');
                    buffer[cursor++] = (char)ch;
                    ch = lexget(lexer);
                }
                switch (t_base) {
                case 64:
                case 32:
                case 16:
                case 8:
                    break;
                default:
                    if (0 == t_base) {
                        yyerror("missing integer suffix");
                    } else {
                        yyerrorf("invalid integer 'i%d' suffix", t_base);
                    }
                    break;
                }
            }
            lexunget(lexer, ch);
        }
    }
    buffer[cursor] = 0;

#if (XXX_DIAG)
    printf("INTEGER(base:%d, suffix:%d, buffer:'%s'%c, overflow:%d) = %d\n", \
          base, suffix, buffer, (ch ? ch : ' '), value->overflow, value->ival);
#endif

    if (error) {
        yyerrorf("invalid integer constant '%s'", buffer);
    } else if (value->overflow) {
        if (value->overflow < 0) {
            yyerror("integral constant underflow on conversion");
        } else {
            yyerror("integral constant overflow on conversion");
        }
    }
    value->errors += error;
    return value->type;
}


static int
get_float(lexer_t *lexer, numeric_t *value, char *buffer, unsigned cursor, int base)
{
    int error = 0, type, ch;

    assert(cursor);
    assert(10 == base || 16 == base);

    /*
     *  Consume digits, decimal point and exponent
     *
     *      [digit]*[.[digit]+][e[-+]][digit]+[fl]
     *
     *  or
     *
     *      0x[hexdigit]*[.[digit]+][e[-+]][digit]+[fl]
     */
    ch = buffer[--cursor];

    if ('.' == ch) {
        buffer[cursor++] = '.';
        while (0 != (ch = lexget(lexer)) && ISDIGIT(ch)) {
            buffer[cursor++] = (char)ch;
        }
    }

    if (16 != base && ('e' == ch || 'E' == ch)) {
        /* Decimal float
        // Components:
        //  o (optional) plus or minus sign
        //  o nonempty sequence of decimal digits optionally containing decimal-point character (as determined by the current C locale) (defines significand)
        //  o (optional) e or E followed with optional minus or plus sign and nonempty sequence of decimal digits (defines exponent to base 10)
        */
        if ('.' == buffer[cursor-1]) {
            ++error;                            /* .. dont allow '.e' (nonempty sequence) */
        }
        buffer[cursor++] = (char)ch;
        ch = lexget(lexer);
        if ('+' == ch || '-' == ch) {           /* optional sign */
            buffer[cursor++] = (char)ch;
            ch = lexget(lexer);
        }

        if (! ISDIGIT(ch)) {                    /* E[+-][digit]+ */
            if (0 == ch) {
                yyerror("unterminated float constant");
            } else if ('\r' == ch || '\n' == ch) {
                yyerror("newline within float constant");
            }
            ++value->errors;
        }

        while (ISDIGIT(ch)) {
            buffer[cursor++] = (char)ch;
            ch = lexget(lexer);
        }

#if defined(HEX_FLOATS)
    } else if (16 == base && ('p' == ch || 'P' == ch)) {
        /* Hexadecimal float
        // Components:
        //  o (optional) plus or minus sign
        //  o 0x or 0X
        //  o nonempty sequence of hexadecimal digits optionally containing a decimal-point character (as determined by the current C locale) (defines significand)
        //  o (optional) p or P followed with optional minus or plus sign and nonempty sequence of decimal digits (defines exponent to base 2)
        */
        if ('.' == buffer[cursor-1]) {
            ++error;                            /* .. dont allow '.p' (nonempty sequence) */
        }
        buffer[cursor++] = (char)ch;
        ch = lexget(lexer);
        if ('+' == ch || '-' == ch) {           /* optional sign */
            buffer[cursor++] = (char)ch;
            ch = lexget(lexer);
        }

        if (! ISDIGIT(ch)) {                    /* P[+-][digit]+ */
            if (0 == ch) {
                yyerror("unterminated hexidecimal float constant");
            } else if ('\r' == ch || '\n' == ch) {
                yyerror("newline within hexidecimal float constant");
            }
            ++value->errors;
        }

        while (ISDIGIT(ch)) {
            buffer[cursor++] = (char)ch;
            ch = lexget(lexer);
        }
#endif //HEX_FLOATS
    }


    /*
     *  numeric-suffix:
     *      float-suffix:           f|F
     *      long-suffix:            l|L
     *      double-suffix:          d|D
     */
    if ('f' == ch || 'F' == ch) {               /* float */
        type = O_FLOAT_CONST;

    } else if ('d' == ch || 'D' == ch) {        /* C#, double (extension) */
        type = O_DOUBLE_CONST;

    } else if ('l' == ch || 'L' == ch) {
#if defined(O_LONG_DOUBLE_CONST) && defined(HAVE_STRTOLD)
        type = O_LONG_DOUBLE_CONST;
#else
        yyerror("long doubles are not supported");
        type = O_DOUBLE_CONST;
#endif
    } else {                                    /* default (double) */
        lexunget(lexer, ch);
        type = O_DOUBLE_CONST;
    }

    /*
     *  convert
     */
    buffer[cursor] = '\0';
    if (0 == error) {
        error = convertfloat(value, type, buffer, cursor);
    }

#if (XXX_DIAG)
    printf("FLOAT(type:%d, buffer:'%s'%c, overflow:%d) = %g\n", \
        type, buffer, (ch ? ch : ' '), value->overflow, value->fval);
#endif

    /*
     *  report
     */
    if (error) {
        yyerror("invalid floating point constant");
    } else if (value->overflow) {
        if (value->overflow < 0) {
            if (O_FLOAT_CONST == type) {
                yywarning("float constant underflow on conversion");
            } else if (-1 == value->overflow) {
                yywarning("double constant underflow on conversion");
            }
        } else {
            if (O_FLOAT_CONST == type) {
                yywarning("float constant overflow on conversion");
            } else if (-1 == value->overflow) {
                yywarning("double constant underflow on conversion");
            }
        }
    }
    value->errors += error;
    return value->type;
}


static int
convertnumeric(numeric_t *value, int base, const char *buffer, unsigned __CUNUSEDARGUMENT(length))
{
    int error = 0;
    char *endp = NULL;
    long ret;

    value->type = O_INTEGER_CONST;
    if (2 == base || 16 == base)
        buffer += 2;                            /* 0x or 0b */
    errno = 0;
    ret = strtol(buffer, &endp, base);
    if (ERANGE == errno) {
        value->overflow = (ret == LONG_MIN ? -1 : 1);
    } else if (errno || *endp) {
        ++error;
    }
    value->ival = (accint_t)ret;
    return error;
}


static int
convertfloat(numeric_t *value, int type, const char *buffer, unsigned __CUNUSEDARGUMENT(length))
{
    double ret;
    char *end;
    int error = 0;

#if !defined(HUGE_VALF) && defined(_HUGE_VALF)
#defined HUGE_VALF      _HUGE_VALF
#endif

#if defined(O_LONG_DOUBLE_CONST) && defined(HAVE_STRTOLD)
#if !defined(HUGE_VALL) && defined(_HUGE_VALL)
#defined HUGE_VALL      _HUGE_VALL
#endif

    value->overflow = 0;

    if (O_FLOAT_CONST == type) {
        errno = 0;
        ret = strtod(buffer, &end);
        if (ERANGE == errno) {                  /* double range */
            value->overflow = (ret == -HUGE_VAL ? -1 : 1);
                // If the correct value would cause overflow, signed plus or minus HUGE_VAL is returned, and ERANGE is stored in errno.
                // If the correct value would cause underflow, zero is returned and ERANGE is stored in errno.

#if defined(FLT_MIN) && defined(FLT_MAX)        /* float range */
        } else if (0 == errno && ret) {
            if (ret < FLT_MIN) {
                value->overflow = -2;
            } else if (ret > FLT_MAX) {
                value->overflow = 2;
            }
#endif
        } else if (errno || *end) {
            ++error;
        }

    } else if (O_LONG_DOUBLE_CONST == type) {
        long double lret;

        errno = 0;
        lret = strtold(buffer, &end);
        if (ERANGE == errno) {
#if defined(HUGE_VALL)
            value->overflow = (lret == -HUGE_VALL ? -1 : 1);
#else
            value->overflow = 1;
#endif
        } else if (errno || *end) {
            ++error;
        }
        ret = (double)lret;                     /* XXX - needs work */

    } else /*DOUBLE*/
#endif  /*O_LONG_DOUBLE_CONST*/
    {
        errno = 0;
        ret = strtod(buffer, &end);
        if (ERANGE == errno) {
            value->overflow = (ret == -HUGE_VAL ? -1 : 1);
        } else if (errno || *end) {
            ++error;
        }
    }

    value->type = type;
    value->fval = ret;
    return error;
}


static int
kwlookup(const char *str)
{
    register const struct map *mp;

    for (mp = opcodes; mp->token; ++mp) {
        if (*mp->token == *str && 0 == strcmp(str, mp->token)) {
#if (XXX_NOTEXTENSIONS)
            const int token = mp->val;

            if (! xf_c99extensions) {
                switch (token) {
                case K_INLINE:
                case K_RESTRICT:
                case K_COMPLEX:
                case K_IMAGINARY:
                case K_BOOL:
                    return -1;
                }

            if (0 == xf_grunch) {
                switch (token) {
                case K_LIST:
                case K_ARRAY:
                case K_STRING:
                case K_DECLARE:
                case K_GLOBAL:
                case K_REPLACEMENT:
                    return -1;
                }
            } else if (2 == xf_grunch) {
                switch (token) {
                case K_TRY:
                case K_CATCH:
                case K_FINALLY:
                    return -1;
                }
            }
#endif  /*XXX_NOTEXTENSIONS*/
            return mp->val;
        }
    }
    return -1;
}


static void
newline(void)
{
    x_columnno = 0;
    ++x_lineno;
}


const char *
yysymbol(const char *str, int length)
{
    register const struct map *mp;

    for (mp = opcodes; mp->symbol; ++mp) {
        if (mp->slength == length &&
                *str == *mp->symbol && 0 == memcmp(str, mp->symbol, length)) {
            if (mp->desc) {
                return mp->desc;
            }
            return mp->token;
        }
    }
    return NULL;
}


const char *
yymap(int word)
{
    register const struct map *mp;
    static char buf[32];

    for (mp = opcodes; mp->symbol; ++mp) {
        if (mp->val == word)  {
            if (mp->token) {
/*              if (mp->lang) {
 *                  return mp->lang;
 *              }
 */
                return mp->token;
            }
            return mp->desc;
        }
    }
    sprintf(buf, "<%x>", word);
    return buf;
}


const char *
filename_cache(const char *name)
{
    char *cp;

    if (NULL == hd_filenames) {
        hd_filenames = ll_init();
    } else {
        register List_p lp;

        for (lp = ll_first(hd_filenames); lp; lp = ll_next(lp)) {
            if (strcmp(ll_elem(lp), name) == 0) {
                return ll_elem(lp);
            }
        }
    }
    cp = chk_salloc(name);
    ll_push(hd_filenames, cp);
    return cp;
}


static void
filename_free(void)
{
    register List_p lp;

    if (hd_filenames) {
        while ((lp = ll_first(hd_filenames)) != NULL) {
            chk_free(ll_elem(lp));
            ll_delete(lp);
        }
        ll_free(hd_filenames);
        hd_filenames = NULL;
    }
}

/*end*/
