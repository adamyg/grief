#include <edidentifier.h>
__CIDENT_RCSID(gr_language_c,"$Id: language.c,v 1.42 2014/10/27 23:27:53 ayoung Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: language.c,v 1.42 2014/10/27 23:27:53 ayoung Exp $
 * Module loader and inline compiler for lisp source.
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

#include <editor.h>
#include <edfileio.h>
#include <edenv.h>                              /* gputenvv(), ggetenv() */
#include <edcm.h>
#include <libstr.h>                             /* str_...()/sxprintf() */

#include "builtin.h"
#include "debug.h"
#include "echo.h"
#include "file.h"
#include "keywd.h"
#include "language.h"
#include "system.h"
#include "word.h"

#define MAX_FILES       10
#define FBUFSIZ         1024
#define MAX_CM_SIZE     65530
#define NATOMS          2048
#define NMACROS         256
#define YYMAX           128

struct fp {
    const char *        name;
    const char *        buf;
    const char *        bufp;
    const char *        bufend;
    int                 fd;
    FSIZE_t             size;
    int                 line_no;
};

enum _xxt {
    _XXDIGIT            =0x01,
    _XXSYMBOL           =0x02,
    _XXWS               =0x04,
    _XXISPRINT          =0x08,
    _XXTABESC           =0x10 
};

enum _ltoks {
    TOKEN_EOF           =0,
    OPEN_PAREN          =1,
    CLOSE_PAREN         =2,
    TOKEN_ID            =3,
    TOKEN_INT           =4,
    TOKEN_STR           =5,
    TOKEN_FLOAT         =6
};

typedef struct cmnode {
    FSIZE_t             m_size;
    TAILQ_ENTRY(cmnode) m_node;
} CMNODE_t;

static TAILQ_HEAD(cmnodelist, cmnode)           /* loaded macros */
                        cmnodeq;

static struct fp *      fp_hdr = NULL;          /* current open file */
static struct fp        fps[MAX_FILES];         /* file stack */

static DEFINE *         def_head;
static DEFINE *         def_ptr;

static int              cmrunning = FALSE;

static Head_p           hd_syms;
static LIST *           first_atom;

static int              pendingnumber;
static int              pendingmacros[NMACROS]; /* List of macros to be inserted */

#define newline()       {++fp_hdr->line_no;}
#define fpgetch()       (yytchar = (fp_hdr->bufp < fp_hdr->bufend ? *fp_hdr->bufp++ : fpunderflow(TRUE)))
#define fpungetch()     {--fp_hdr->bufp;}
#define fppeek()        (fp_hdr->bufp < fp_hdr->bufend ? *fp_hdr->bufp : fpunderflow(FALSE))

static int              fpopen(const char *filename);
static void             fppush(const char *filename, int line_no, const char *buf, unsigned length);
static int              fpclose(void);
static int              fpunderflow(int consume);

static void             free_defines(DEFINE *ptr);
static int              gr_parse1(int base_atom);
static List_p           decl_lookup(const char *sym);
static void             decl_enter(const char *sym, OPCODE type);
static OPCODE           decl_gettype(const char *sym);
static int              gr_lexer(void);
static void             gr_character(char *str);
static int              gr_comment(int C_comment);
static int              gr_string(int quote);
static void             gr_getuntil(register char *str, register int mask);
static void             gr_cpp(void);
static void             gr_include(const char *cp);
static void             gr_define(char *cp);
static int              gr_number(int ch);
static int              gr_symbol(int ch);

static int              gr_loadobject(struct fp *fp, void (*execute)(const LIST *lp, int size));

static const unsigned char _chars_[256] = {
    /* | nul| soh | stx | etx | eot | enq | ack | bel | */
        0,    0,    0,    0,    0,    0,    0,    0,        /* 0x00-0x07 */
    /* | bs | ht  | nl  | vt  | np  | cr  | so  | si  | */
        0x10, 0x14, 0,    0,    4,    0,    0,    0,        /* 0x08-0x0f */
    /* | dle| dc1 | dc2 | dc3 | dc4 | nak | syn | etb | */
        0,    0,    0,    0,    0,    0,    0,    0,        /* 0x10-0x17 */
    /* | can| em  | sub | esc | fs  | gs  | rs  | us  | */
        0,    0,    0,    0x10, 0,    0,    0,    0,        /* 0x18-0x0f */
    /* | sp |  !  |  "  |  #  |  $  |  %  |  &  |  '  | */
        0x0c, 0x0a, 0x08, 0x08, 0x0a, 0x0a, 0x0a, 0x08,
    /* |  ( |  )  |  *  |  +  |  ,  |  -  |  .  |  /  | */
        0x08, 0x08, 0x0a, 0x0a, 0x08, 0x0a, 0x0a, 0x0a,
    /* |  0 |  1  |  2  |  3  |  4  |  5  |  6  |  7  | */
        0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b,
    /* |  8 |  9  |  :  |  ;  |  <  |  =  |  >  |  ?  | */
        0x0b, 0x0b, 0x08, 0x08, 0x0a, 0x0a, 0x0a, 0x08,
    /* |  @ |  A  |  B  |  C  |  D  |  E  |  F  |  G  | */
        0x08, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0a,
    /* |  H |  I  |  J  |  K  |  L  |  M  |  N  |  O  | */
        0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a,
    /* |  P |  Q  |  R  |  S  |  T  |  U  |  V  |  W  | */
        0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a,
    /* |  X |  Y  |  Z  |  [  |  \  |  ]  |  ^  |  _  | */
        0x0b, 0x0a, 0x0a, 0x08, 0x08, 0x08, 0x0a, 0x0a,
    /* |  ` |  a  |  b  |  c  |  d  |  e  |  f  |  g  | */
        0x08, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0a,
    /* |  h |  i  |  j  |  k  |  l  |  m  |  n  |  o  | */
        0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a,
    /* |  p |  q  |  r  |  s  |  t  |  u  |  v  |  w  | */
        0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a,
    /* |  x |  y  |  z  |  {  |  |  |  }  |  ~  | del | */
        0x0b, 0x0a, 0x0a, 0x08, 0x0a, 0x08, 0x0a, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,     /* 0x80-0x87 */
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,     /* 0x88-0x8f */
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,     /* 0x90-0x97 */
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,     /* 0x98-0x9f */
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,     /* 0xa0-0xa7 */
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,     /* 0xa8-0xaf */
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,     /* 0xb0-0xb7 */
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,     /* 0xb8-0xbf */
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,     /* 0xc0-0xc7 */
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,     /* 0xc8-0xcf */
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,     /* 0xd0-0xd7 */
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,     /* 0xd8-0xdf */
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,     /* 0xe0-0xe7 */
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,     /* 0xe8-0xef */
        0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,     /* 0xf0-0xf7 */
        0x08, 0x08, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00,     /* 0xf8-0xff */
        };


/*  Function:           cm_init
 *      Runtime initialise the cm storage
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 */
void
cm_init(int cm)
{
    unsigned i;

    TAILQ_INIT(&cmnodeq);
    for (i = 0; i < MAX_FILES; ++i) {
        fps[i].fd = -1;
    }
    hd_syms = ll_init();
    cmrunning = cm;
    fp_hdr = NULL;
}


/*  Function:           cm_shutdown
 *      Release storage of loaded macro objects.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 */
void
cm_shutdown(void)
{
    CMNODE_t *n;

    while (NULL != (n = TAILQ_FIRST(&cmnodeq))) {
        TAILQ_REMOVE(&cmnodeq, n, m_node);
        chk_free(n);
    }
    ll_free(hd_syms);
    hd_syms = NULL;
}


/*  Function:           gr_push
 *      initialise the data structures used to start compiling a '.m' file.
 *
 *  Parameters:
 *      fname -             File name.
 *
 *  Returns:
 *      -1 if file not available, zero if file successfully opened.
 */
int
cm_push(const char *filename)
{
    assert(NULL == fp_hdr);
    return fpopen(filename);
}


void
cm_pop(void)
{
    while (fp_hdr) {
        fpclose();
    }
}


int
cm_push2(const char *filename, const char *buf, unsigned length)
{
    assert(NULL == fp_hdr);
    fppush(filename, 1, buf, length);
    return 0;
}


static int
fpopen(const char *filename)
{
    struct stat stat_buf;
    char *buf;

    if (NULL == fp_hdr) {
        fp_hdr = fps;
    } else {
        if (++fp_hdr >= (fps + MAX_FILES)) {
            errorf("include files nested too deeply");
            --fp_hdr;
            return 0;
        }
    }

    assert(fp_hdr->fd == -1);
    if ((fp_hdr->fd = fileio_open(filename, OPEN_R_BINARY | O_RDONLY, 0)) < 0) {
        fp_hdr->fd = -1;
        if (--fp_hdr < fps) {
            fp_hdr = NULL;
        }
        return -1;
    }

#define FP_UNGET    2

    if (fstat(fp_hdr->fd, &stat_buf) < 0 ||
            (stat_buf.st_mode & S_IFMT) != S_IFREG ||
            NULL == (buf = (char *)chk_calloc(FBUFSIZ + (FP_UNGET * 2), 1))) {
        fpclose();
        return -1;
    }

    fp_hdr->name    = chk_salloc(filename);
    fp_hdr->size    = stat_buf.st_size;
    fp_hdr->line_no = 1;
    fp_hdr->buf     = buf + FP_UNGET;           /* +2 for unget buffer */
    fp_hdr->bufp    = fp_hdr->bufend = fp_hdr->buf;

    buf[0] = buf[1] = 0;

    if (fp_hdr->size) {
        if (0 == fpunderflow(FALSE)) {          /* load first buffer */
            fp_hdr->bufend = NULL;
            fp_hdr->size = 0;
        }

    } else {
        fp_hdr->bufend = NULL;
    }
    return 0;
}


static void
fppush(const char *filename, int line_no, const char *buf, unsigned len)
{
    if (++fp_hdr >= (fps + MAX_FILES)) {
        errorf("File structure to complex, include files nested too deeply -- ignored");
        --fp_hdr;
        return;
    }
    assert(fp_hdr->fd == -1);
    fp_hdr->name    = filename;
    fp_hdr->line_no = line_no;
    fp_hdr->bufp    = fp_hdr->buf = buf;
    fp_hdr->bufend  = buf + len;
    fp_hdr->fd      = -1;
}


static int
fpclose(void)
{
    assert(fp_hdr);
    if (fp_hdr) {
        if (fp_hdr->fd >= 0) {
            fileio_close(fp_hdr->fd);
            fp_hdr->fd = -1;
            chk_free((void *)fp_hdr->name);
            fp_hdr->name = NULL;
            if (fp_hdr->buf) {
                chk_free((char *)(fp_hdr->buf - FP_UNGET));
            }
            fp_hdr->buf = fp_hdr->bufp = fp_hdr->bufend = NULL;
        }
        if (--fp_hdr >= fps) {
            return FALSE;
        }
        fp_hdr = NULL;
    }
    return TRUE;                                /* EOF */
}


/*  Function:       fpunderflow
 *      Interface called when the stream buffer is empty
 *
 *  Parameters:
 *      consume -       Whether the first character should be consumed.
 *
 *  Returns:
 *      Next character value, otherwise 0 on an EOF stream condition
 */
static int
fpunderflow(int consume)
{
    register struct fp *fp;
    int n;

again:;
    fp = fp_hdr;
    if (fp->bufp >= fp->bufend) {
        if (fp->bufend && fp->fd >= 0 &&
                (n = sys_read(fp->fd, (char *)fp->buf, FBUFSIZ)) > 0) {

            fp->bufp = fp->buf;
            fp->bufend = fp->buf + n;

        } else {
            if (fpclose()) {
                return 0;
            }
            goto again;
        }
    }

    if (consume) {
        return *fp->bufp++;
    }
    return *fp->bufp;
}


static char             yytext[YYMAX];
static accint_t         yyint;
static accfloat_t       yyfloat;
static int              yyllevel = 0;
static const char **    yyincludes;
static int              yytoken;
static int              yymallocsz = NATOMS;
static int              yyerror;

int
cm_parse(void (*execute)(const LIST *lp, int size), const char **includes)
{
    int atom;
    int i;

    /* object load */
    if (fp_hdr->size >= (int)sizeof(CM_t)) {
        const CM_t *cm = (const CM_t *) fp_hdr->bufp;

        if (CM_MAGIC == cm->cm_magic) {
            if (! cmrunning) {
                i = gr_loadobject(fp_hdr, execute);

            } else {
                errorf("already a compiled object.");
                errno = EINVAL;
                i = -1;
            }
            fpclose();
            return i;
        }
    }

    /* parse image */
    yyerror = 0;

    def_head = def_ptr = NULL;
    pendingnumber = 0;

    if (NULL == (first_atom = chk_calloc(yymallocsz, 1))) {
        cm_error("Cannot allocate room for macro");
        return -1;
    }

    atom = 0;
    yyincludes = includes;
    while (1) {
        pendingmacros[pendingnumber] = atom;

        yytoken = gr_lexer();
        if (yytoken == TOKEN_EOF)
            break;

        if (yytoken != OPEN_PAREN) {
            cm_error("Macro does not start with a '('");
            break;
        }

        atom = gr_parse1(atom);
        if (yytoken < 0)
            break;

        if (yytoken != CLOSE_PAREN) {
            cm_error("Macro does not end with a ')'");
            break;
        }

        first_atom[atom++] = F_END;             /* defunct */
        ++pendingnumber;
    }

    free_defines(def_head);
    ll_clear(hd_syms);
    yyincludes = NULL;

    /* cleanup */
    if (yyerror) {
        while (fp_hdr) {
            fpclose();
        }
        if (first_atom) {
            chk_free(first_atom);
        }
        return yyerror;
    }

    /* execute initialisation interface */
    if (atom && execute) {
        first_atom = chk_realloc((char *) first_atom, atom + 1);

        for (i = 0; i < pendingnumber; ++i) {
            int sizeof_macro = pendingmacros[i + 1] - pendingmacros[i];

            (*execute)(first_atom + pendingmacros[i], sizeof_macro);
        }
    }
    return 0;
}


static void
free_defines(DEFINE * ptr)
{
    if (ptr) {
        free_defines(ptr->next);
        chk_free((void *)ptr->name);
        chk_free((void *)((char *)ptr->value - 1));
        chk_free(ptr);
    }
}


static int
gr_parse1(int base_atom)
{
    LIST *ap;
    int atom = base_atom;
    int new_atom;
    int first_token = TRUE;
    int decl = 0;
    int len;

    while (1) {
        if (atom > yymallocsz - 10) {
            yymallocsz += NATOMS;
            if ((first_atom = chk_realloc(first_atom, yymallocsz)) == NULL) {
                cm_error("Cannot allocate room for macro");
                return -1;
            }
        }

        yytoken = gr_lexer();

        if (yytoken == OPEN_PAREN) {
            if (decl) {
                cm_error("Cannot nest declarations.");
                return 0;
            }

            first_atom[atom] = F_LIST;
            if ((new_atom = gr_parse1(atom + sizeof_atoms[F_LIST])) == 0) {
                return 0;
            }

            if (yytoken == TOKEN_EOF) {
                cm_error("Missing close parenthesis.");
                return 0;
            }

            len = (new_atom - atom);
            assert(len < 0xffff);
            LPUT_LEN(first_atom + atom, (uint16_t)len);
            atom = new_atom;
            continue;
        }

        ap = &first_atom[atom];
        if (yytoken == CLOSE_PAREN) {
            *ap = F_HALT;
            return ++atom;
        }

        if (decl || yytoken == TOKEN_ID) {
            const BUILTIN *bp;

            if ((bp = builtin_lookup(yytext)) != 0) {
                if (first_token) {
                    if (0 == strcmp(yytext, "int")) {
                        decl = F_INT;
                    } else if (0 == strcmp(yytext, "string")) {
                        decl = F_STR;
                    } else if (0 == strcmp(yytext, "list")) {
                        decl = F_LIST;
#if defined(F_ARRAY)
                    } else if (0 == strcmp(yytext, "array")) {
                        decl = F_ARRAY;
#endif
                    } else if (0 == strcmp(yytext, "float")) {
                        decl = F_FLOAT;
                    } else if (0 == strcmp(yytext, "global")) {
                        decl = -1;
                    }
                    first_token = FALSE;
                }
                *ap = F_ID;
                LPUT16(ap, (uint16_t)(bp - builtin));   /* builtin identifier/offset */
                atom += sizeof_atoms[F_ID];
                continue;
            }

            if (decl == -1) {
                OPCODE type;

                type = decl_gettype(yytext);
                if ((int) type == 0) {
                    cm_error("Undefined symbol %s", yytext);
                    return 0;
                }

                if (type == F_ERROR) {
                    cm_error("Trying to globalise symbol declared with different types: %s", yytext);
                    return 0;
                }

                *ap = F_INT;
                LPUT_INT(ap, type);
                atom += sizeof_atoms[F_INT];
                ap = &first_atom[atom];

            } else if (decl) {
                decl_enter(yytext, decl);
            }

            if (yytext[0] == 'N' && strcmp(yytext, "NULL") == 0) {
                *ap = F_NULL;
            } else if (yytext[0] != '"') {
                *ap = F_STR;
                LPUT_PTR(ap, chk_salloc(yytext));
            } else {
                *ap = F_LIT;
                LPUT_PTR(ap, chk_salloc(yytext + 1));
            }
            atom += sizeof_atoms[*ap];
            first_token = FALSE;
            continue;
        }

        first_token = FALSE;
        if (yytoken == TOKEN_INT) {
            *ap = F_INT;
            LPUT_INT(ap, yyint);
            atom += sizeof_atoms[F_INT];
            continue;
        }

        if (yytoken == TOKEN_FLOAT) {
            *ap = F_FLOAT;
            LPUT_FLOAT(ap, yyfloat);
            atom += sizeof_atoms[F_FLOAT];
            continue;
        }

        if (yytoken == TOKEN_EOF) {
            return atom;
        }

        yytoken = -1;
        cm_error("Invalid token");
        return 0;
    }
}


static List_p
decl_lookup(const char *sym)
{
    List_p lp;
    SYMBOL *sp;

    for (lp = ll_first(hd_syms); lp; lp = ll_next(lp)) {
        sp = (SYMBOL *) ll_elem(lp);
        if (strcmp(sp->s_name, sym) == 0) {
            return lp;
        }
    }
    return NULL;
}


static void
decl_enter(const char *sym, OPCODE type)
{
    List_p lp = decl_lookup(sym);
    SYMBOL *sp;

    if (lp) {
        sp = (SYMBOL *) ll_elem(lp);
        if (sp->s_type != type)
            sp->s_type = F_ERROR;
    } else {
        sp = memset(chk_alloc(sizeof(SYMBOL)), 0, sizeof(SYMBOL));
        strcpy(sp->s_name, yytext);
        sp->s_type = type;
        ll_append(hd_syms, (char *) sp);
    }
}


static OPCODE
decl_gettype(const char * sym)
{
    List_p lp = decl_lookup(sym);
    SYMBOL *sp;

    if (lp == NULL) {
        return (OPCODE) 0;
    }
    sp = (SYMBOL *) ll_elem(lp);
    return sp->s_type;
}


static int
gr_lexer(void)
{
    register int yytchar;

again:
    while (_chars_[fpgetch()] & _XXWS)
        ;

    switch (yytchar) {
    case '(':
        return OPEN_PAREN;

    case ')':
        return CLOSE_PAREN;

    case '-':
    case '+':
    case '.':
        if ((_chars_[fppeek()] & _XXDIGIT) == 0) {
            goto alpha;
        }

    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
        return gr_number(yytchar);

    case 'A': case 'a':
    case 'B': case 'b':
    case 'C': case 'c':
    case 'D': case 'd':
    case 'E': case 'e':
    case 'F': case 'f':
    case 'G': case 'g':
    case 'H': case 'h':
    case 'I': case 'i':
    case 'J': case 'j':
    case 'K': case 'k':
    case 'L': case 'l':
    case 'M': case 'm':
    case 'N': case 'n':
    case 'O': case 'o':
    case 'P': case 'p':
    case 'Q': case 'q':
    case 'R': case 'r':
    case 'S': case 's':
    case 'T': case 't':
    case 'U': case 'u':
    case 'V': case 'v':
    case 'W': case 'w':
    case 'X': case 'x':
    case 'Y': case 'y':
    case 'Z': case 'z':
    case '_':
    case '$':
        if (gr_symbol(yytchar) == TRUE)
            goto again;
        return yytoken = TOKEN_ID;

    case '/':
        yytext[0] = (char) yytchar;
        if (*fp_hdr->bufp == '*') {
            if (gr_comment(TRUE) == 0)
                return TOKEN_EOF;
            goto again;
        }

        if (*fp_hdr->bufp == '\0') {
            if (fpgetch() == '*') {
                if (gr_comment(TRUE) == 0)
                    return TOKEN_EOF;
                goto again;
            }
            fpungetch();
        }

        gr_getuntil(yytext + 1, _XXSYMBOL);
        if (yytext[1] != '/' || yytext[2] != '\0')
            return TOKEN_ID;
        /* FALLTHRU - (allows // as a comment) */

    case ';':
        if (gr_comment(FALSE) == 0)
            return TOKEN_EOF;
        goto again;

    case '*':
    case '<': case '>':
    case '=': case '!':
    case '|': case '&':
    case '^': case '~':
    case '%':
alpha:; yytext[0] = (char) yytchar;
        gr_getuntil(yytext + 1, _XXSYMBOL);
        return TOKEN_ID;

    case '\'':
        if (gr_string('\'') == FALSE) {
            cm_error("Character constant too long or unterminated.");
            return -1;
        }
        yyint = yytext[0];
        return yytoken = TOKEN_INT;

    case '"':
        if (gr_string('"') == FALSE) {
            cm_error("String literal not terminated.");
            return -1;
        }
        return yytoken;

    case '#':
        gr_cpp();
        goto again;

    case '\r':
        goto again;

    case '\n':
        newline();
        goto again;

    case 0x04:                    /* CTRL-D */
    case 0x1a:                    /* CTRL-Z */
    case 0:
        return TOKEN_EOF;

    default:
        cm_error("illegal character: 0x%02x (%c)", (yytchar & 0xff), yytchar);
        return -1;
    }
}


static void
gr_character(char *str)
{
    char *charp = str++;
    char ch = *str++;
    int byte;

    switch (ch) {
    case 't':
        byte = '\t';
        break;

    case 'n':
        byte = '\n';
        break;

    case 'f':
        byte = '\f';
        break;

    case 'r':
        byte = '\r';
        break;

    case 'x':
        byte = *str++;
        if (isdigit(byte)) {
            byte -= '0';
        } else if (byte >= 'A' && byte <= 'F') {
            byte = byte - 'A' + 10;
        } else if (byte >= 'a' && byte <= 'f') {
            byte = byte - 'a' + 10;
        } else {
            --str;
            break;
        }

        /* Second digit */
        ch = *str++;
        if (isdigit(ch)) {
            byte = (byte << 4) + ch - '0';
        } else if (ch >= 'A' && ch <= 'F') {
            byte = (byte << 4) + ch - 'A' + 10;
        } else if (ch >= 'a' && ch <= 'f') {
            byte = (byte << 4) + ch - 'a' + 10;
        } else {
            --str;
        }
        break;

    default:
        byte = ch;
        break;
    }
    *charp++ = (char)byte;
    strcpy(charp, str);
}


static int
gr_comment(int C_comment)
{
    const int end_ch = (C_comment ? '*' : '\n');
    register int yytchar;
    int lineno = fp_hdr->line_no;

    while (1) {
        if (fpgetch() <= 0) {
            cm_error("Unterminated comment at line %d", lineno);
            return 0;
        }

second_char:;
        if (yytchar == '\n') {
            newline();
        }

        if (yytchar == end_ch) {
            if (!C_comment)
                return 1;
            if (fpgetch() <= 0)
                return 0;
            if (yytchar == '/')
                return 1;
            goto second_char;
        }
    }
}


static int
gr_string(int quote)
{
    register unsigned char *cp = (unsigned char *) yytext;
    register int yytchar;

    if (quote == '"')
        *cp++ = '"';

    while (1) {
        if (fpgetch() <= 0) {
            *cp = '\0';
            return FALSE;
        }

        if (yytchar == quote) {
            *cp = '\0';
            break;
        }

        if (yytchar == '\n') {
            *cp = '\0';
            return FALSE;
        }

        if (yytchar != '\\') {
            *cp++ = (char) yytchar;
            continue;
        }
        *cp++ = '\\';
        *cp++ = (unsigned char)fpgetch();
    }

    for (cp = (unsigned char *)yytext; *cp; ++cp) {
        if (*cp != '\\') {
            continue;
        }
        gr_character((char *)cp);
    }

    yytoken = TOKEN_ID;
    return TRUE;
}


static void
gr_getuntil(register char * str, register int mask)
{
    register int yytchar;

    while (1) {
        if (fpgetch() <= 0) {
            *str = '\0';
            return;
        }

        if ((_chars_[yytchar] & mask) == 0) {
            fpungetch();
            *str = '\0';
            return;
        }
        *str++ = (char) yytchar;
    }
}


void
cm_error(const char *fmt, ...)
{
    char buf[MAX_PATH + 20];
    va_list ap;
    int len = 0;

    yyerror = -1;
    if (fp_hdr >= fps) {
        len += sprintf(buf, "%s(%d): ", fp_hdr->name, fp_hdr->line_no);
    }
    va_start(ap, fmt);
    vsxprintf(buf + len, sizeof(buf) - len, fmt, ap);
    va_end(ap);
    ewprintf("%s", buf);
    yyllevel = 0;
    yytoken = -1;
}


static void
gr_cpp(void)
{
    register char *cp;
    register int yytchar;

    yytext[0] = '#';
    for (cp = yytext + 1;; *cp++ = (char) yytchar)
        if (fpgetch() <= 0 || yytchar == '\n' || yytchar == ';') {
            if (yytchar > 0) {
                fpungetch();
            }
            *cp = '\0';
            break;
        }

    for (cp = yytext + 1; *cp == ' ' || *cp == '\t';) {
        ++cp;
    }

    if (0 == strncmp(cp, "define", 6)) {
        gr_define(cp + 6);
        return;
    }

    if (0 == strncmp(cp, "include", 7)) {
        gr_include(cp + 7);
        return;
    }

    cm_error("pre-processor command not recognized");
}


static void
gr_include(const char *cp)
{
    const char delimiter[2] = {FILEIO_DIRDELIM, 0},
            separator[2] = {FILEIO_PATHSEP, 0};
    char incfile[MAX_PATH], buf[MAX_PATH];
    const char *bpath, *s1, *s2;
    char *inc = incfile;
    int delim;

    while (isspace(*cp)) ++cp;

    delim = *cp++;
    if ('<' != delim && '"' != delim) {
        cm_error("#include missing file-name");
        return;
    }

    while (*cp && '>' != *cp && '"' != *cp) {
        *inc++ = *cp++;
    }
    *inc = '\0';

    if (inc == incfile ||
            ('<' == delim && '>' != *cp) ||
            ('"' == delim && '"' != *cp)) {
        cm_error("#include invalid file-name");
        return;
    }

    if ('"' == delim && fpopen(incfile) >= 0) {
        return;                                 /* "filename" */
    }

    s1 = strrchr(fp_hdr->name, '/');
    s2 = strrchr(fp_hdr->name, '\\');
    if (s1 || s2) {
        const int pathlen =                     /* length of leading path */
            ((s1 > s2 ? s1 : s2) - fp_hdr->name) + 1;

        strxcpy(buf, (const char *)fp_hdr->name, sizeof(buf));
        strxcpy(buf + pathlen, (const char *)incfile, sizeof(buf) - pathlen);
        if (fpopen(buf) >= 0) {
            return;
        }
    }

    if (yyincludes) {
        unsigned idx;

        for (idx = 0; yyincludes[idx]; ++idx) {
            strxcpy(buf, yyincludes[idx], sizeof(buf));
            strxcat(buf, separator, sizeof(buf));
            strxcat(buf, incfile, sizeof(buf));
            if (fpopen(buf) >= 0) {
                 return;
            }
        }
    }

    if (NULL != (bpath = ggetenv("GRPATH"))) {  /* <filename> */
        char *t_bpath = chk_salloc(bpath);

        for (s1 = strtok(t_bpath, delimiter);
                    s1 != NULL; s1 = strtok(NULL, delimiter)) {
            strxcpy(buf, s1, sizeof(buf));
            strxcat(buf, separator, sizeof(buf));
            strxcat(buf, incfile, sizeof(buf));
            if (fpopen(buf) >= 0) {
                chk_free((void *)t_bpath);
                return;
            }
        }
        chk_free((void *)t_bpath);
    }

    ewprintf("Cannot read %s", incfile);
}


static void
gr_define(char *cp)
{
    const char *symbol, *value;
    DEFINE *dp = def_head;
    int l;

    while (isspace(*cp)) ++cp;
    if (! *cp) {                                /* 21/10/07 */
        cm_error("#define missing r-value");
        return;
    }

    symbol = strtok(cp, " \t");
    if (NULL == (cp = strtok((char *) NULL, "\n"))) {
        cp = (char *)"";
    }
    while (*cp && isspace(*cp)) {
        ++cp;
    }

    if (*cp == '"') {
        value = cp++;
        for (; *cp && *cp != '"'; ++cp) {
            if (*cp == '\\') {
                ++cp;
            }
        }
        if (*cp == '"') {
            *++cp = '\0';
        }
    } else if (*cp) {                           /* 21/10/07 */
        value = strtok(cp, " \t\n");

    } else {
        value = cp;
    }

    l = (int)strlen(value);
    for (; dp; dp = dp->next) {
        if (0 == strcmp(dp->name, symbol)) {
            break;
        }
    }

    if (NULL == dp) {
        if (NULL == def_ptr) {
            def_head = def_ptr = chk_calloc(sizeof(DEFINE), 1);
        } else {
            def_ptr->next = chk_calloc(sizeof(DEFINE), 1);
            def_ptr = def_ptr->next;
        }
        def_ptr->name  = chk_salloc(symbol);
        def_ptr->value = ((char *)chk_alloc(l + 4)) + 1;
        def_ptr->next  = NULL;
        dp = def_ptr;

    } else if ((int) strlen(dp->value + 1) > l) {
        chk_free((char *)(dp->value - 1));
        dp->value = ((char *) chk_alloc(l + 4)) + 1;
    }

    dp->length = l;
    memcpy((char *)(dp->value + 1), value, l + 1);
}


/*
 *  get next character.
 */
static int
parse_get(void *p)
{
    register int yytchar;

    (void)p;
    return fpgetch();
}


/*
 *  unget a character.
 */
static int
parse_unget(void *p, int ch)
{
    __CUNUSED(p)
    fpungetch();
    return ch;
}


/*
 *  parse a number.
 */
static int
gr_number(int ch)
{
    int ret;

    parse_unget(NULL, ch);
    if ((ret = str_numparsex(parse_get, parse_unget, NULL, &yyfloat, &yyint, NULL)) > 0) {
        switch (ret) {
        case NUMPARSE_INTEGER:
            return yytoken = TOKEN_INT;
        case NUMPARSE_FLOAT:
            return yytoken = TOKEN_FLOAT;
        }
    } else {
        cm_error("%s", str_numerror(ret));
    }
    return yytoken = TOKEN_INT;
}


static int
gr_symbol(int ch)
{
    register DEFINE *dp = def_head;

    yytext[0] = (char) ch;
    gr_getuntil(yytext + 1, _XXSYMBOL);

    for (; dp; dp = dp->next)
        if (dp->name[0] == yytext[0] && strcmp(dp->name, yytext) == 0) {
            fppush(fp_hdr->name, fp_hdr->line_no, dp->value+1, dp->length);
            return TRUE;
        }
    return FALSE;
}


/*  Function:       gr_loadobject
 *      Load a 'cm' object into memory.
 *
 *  Parameters:
 *      fp -            Stream
 *
 *      execute -       Callback on success macro load.
 *
 *  Returns;
 *      Zero(0) on success, otherwise -1 on error.
 *
 */
static int
gr_loadobject(struct fp *fp, void (*execute)(const LIST *lp, int size))
{
    register LIST *lp;
    register LIST *lpend;
    LIST *base_list;
    uint32_t *vm_offsets;
    uint32_t *globals;
    uint32_t *soffsets;
    char *str_table;
    CMNODE_t *node;
    CM_t *cm;
    uint32_t u;

    /*
     *  Load the object image
     */
    if (fp->size > MAX_CM_SIZE ||
            NULL == (node = chk_alloc(sizeof(CMNODE_t) + (unsigned)fp->size))) {
        errorf("Macro file too big to read");
        return -1;
    }

    node->m_size = fp->size;                    /* raw size */

    cm = (CM_t *)(node + 1);                    /* object storage */

    if (fp->fd >= 0) {
        /*
         *  Read object
         */
        if (fileio_lseek(fp->fd, 0, SEEK_SET) != 0 ||
                sys_read(fp->fd, (char *)cm, (int)fp->size) != (int) fp->size) {
            errorf("read error on %s file", CM_EXTENSION);
            chk_free(node);
            return -1;
        }
    } else {
        /*
         *  Import object buffer
         */
        assert(fp->buf == fp->bufp);
        memcpy((char *)cm, (const char *)fp->buf, fp->size);
    }

    cm_xdr_import(cm);

    if (cm->cm_version != cm_version) {
        errorf("object version %u.%01u not supported",
            (unsigned)(cm->cm_version/10), (unsigned)(cm->cm_version % 10));
        chk_free(node);
        return -1;
    }

    if (cm->cm_builtin != builtin_count) {
        errorf("builtin macro count disagrees, rebuild required");
        chk_free(node);
        return -1;
    }

    if (cm->cm_signature != builtin_signature) {
        errorf("builtin macro signature disagrees, rebuild required");
        chk_free(node);
        return -1;
    }


    /*
     *  Fixup function, string and global tables.
     */
    vm_offsets = (uint32_t *) (cm + 1);
    WGET32_block(vm_offsets, cm->cm_num_macros + 2);
    base_list = (LIST *) (vm_offsets + cm->cm_num_macros + 2);

    soffsets = (uint32_t *) (((char *) base_list) + vm_offsets[cm->cm_num_macros]);
    str_table = (char *) (soffsets + cm->cm_num_strings);
    WGET32_block(soffsets, cm->cm_num_strings);

    globals = (uint32_t *) (((char *) cm) + cm->cm_globals);
    WGET32_block(globals, cm->cm_num_globals);

    if (cm->cm_globals & 1) {
        errorf("Global decls not on even boundary.");
        chk_free(node);
        return -1;
    }

    TAILQ_INSERT_TAIL(&cmnodeq, node, m_node);  /* queue */

    /*
     *  Walk atoms list and fixup string references.
     */
    lpend = base_list + cm->cm_num_atoms;
    for (lp = base_list; lp < lpend; lp += sizeof_atoms[*lp]) {
        if (F_STR == *lp || F_LIT == *lp) {
            uint32_t offset = LGET32(lp);      /* 32/64 */

            assert(offset < cm->cm_num_strings);
            LPUT_PTR(lp, (str_table + soffsets[offset]));
        }
    }

    /*
     *  Execute "global" variable initialisation code.
     */
#if !defined(__NOFUNCTIONS__)
    for (u = 0; u < cm->cm_num_globals; ++u) {
        lp = (LIST *) (base_list + *globals++);
        trace_ilist(lp);
        execute_xmacro(lp, lp + sizeof_atoms[*lp]);
    }
#endif  /*CM*/

    /*
     *  Execute "macro" list definitions.
     */
    if (execute) {
        for (u = 0; u < cm->cm_num_macros; ++u) {
            (*execute)(base_list + vm_offsets[u], -1);
        }
    }

    return 0;
}
/*end*/

