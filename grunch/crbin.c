#include <edidentifier.h>
__CIDENT_RCSID(gr_crbin_c,"$Id: crbin.c,v 1.24 2021/04/05 09:09:48 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: crbin.c,v 1.24 2021/04/05 09:09:48 cvsuser Exp $
 * Binary backend code generator.
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
#include <edcm.h>

#define OBJECT_SIZE     4096                    /* Initial size of object code buffer */
#define OBJECT_INCR     2048                    /* Code buffer increment */

#ifndef LIST_MAXLEN
#define LIST_MAXLEN     0x7fff                  /* 2^15 */
#endif


/*
 *  Header for the .cm file
 */
static CM_t             cm_header;

static Head_p           hd_list_stack;          /* Stack of FLIST entries */
static unsigned char *  byte_ptr;               /* Ptr to next byte to be added */
static unsigned char *  object_code;            /* Ptr to object code buffer */
static uint32_t         object_allocated;       /* Size of object code buffer */


/*
 *  splay tree of allocated strings
 */
typedef struct string_t {
    uint32_t            str_index;              /* Index into the string table */
    const char *        str_str;
} string_t;

static SPTREE *         str_tree;
static uint32_t         str_index;


/*
 *  Following is an array of pointers to the strings in the splay tree
 */
static ref_t *          str_table;


/*
 *  List of indices to where the macros start. Needed for the
 *  header so we can locate the start of each macro easily
 */
static ref_t *          macro_offsets;
static int              list_level;

static void             push_str(const char *str, int type);
static void             push_int(accint_t);
static void             push_float(accfloat_t);
static void             push_byte(int);

static void             free_binary(void);


void
init_binary(void)
{
    assert(sizeof(cm_header) == CM_STRUCT_SIZE);

    free_binary();
    str_tree = spinit();
    str_index = 0;
    hd_list_stack = ll_init();
    macro_offsets = r_init(F_STR, (void *) NULL, 32 * sizeof(uint32_t));
    str_table = r_init(F_STR, (void *) NULL, 256 * sizeof(char *));
    if (NULL == object_code) {
        object_code = (unsigned char *) chk_alloc(OBJECT_SIZE);
    }
    byte_ptr = object_code;
    memset(&cm_header, 0, sizeof(cm_header));
    list_level = 0;
}


static void
free_binary(void)
{
    list_free(&hd_list_stack);
    r_dec(macro_offsets);
    r_dec(str_table);
    if (str_tree) {
        string_t *strp;
        SPBLK **array = spflatten(str_tree);
        unsigned i;

        for (i = 0; array[i]; ++i) {
            strp = (string_t *) (array[i]->data);
            chk_free((void *)strp->str_str);
            chk_free(array[i]->data);
            spfreeblk(array[i]);
        }
        chk_free(array);
        spfree(str_tree);
    }
}


void
genb_list(void)
{
    uint32_t offset;

    if (0 == list_level++) {
        return;
    }
    push_byte(F_LIST);
    offset = (uint32_t)(byte_ptr - object_code);
    ll_push(hd_list_stack, (void *)((size_t)offset));
    push_byte(0);                                /* LISTSIZE, see end_list() */
    push_byte(0);
}


void
genb_end_list(void)
{
    if (0 == --list_level) {
        push_byte(F_HALT);

    } else {
        List_p lp = ll_first(hd_list_stack);
        uint32_t length, offset;

        assert(lp);
        if (NULL == lp) {
            crerror(RC_ERROR, "internal list stack empty");
            return;
        }        
        offset = (uint32_t)ll_elem(lp);
        push_byte(F_HALT);
        ll_pop(hd_list_stack);                  /* LISTSIZE */
        length = ((byte_ptr - object_code) - offset + 1);
        if (length > LIST_MAXLEN) {
            crerror(RC_ERROR, "internal list object length exceeded");
        }
        LPUT16((LIST *) (object_code + offset - 1), (uint16_t)length);
            /* backfill list length, in bytes */
    }
}


/* integer constant */
void
genb_int(accint_t ival)
{
    push_byte(F_INT);
    push_int(ival);
}


/* numeric/float constant */
void
genb_float(accfloat_t fval)
{
    push_byte(F_FLOAT);
    push_float(fval);
}


/* string constant */
void
genb_string(const char *str)
{
    push_str(str, F_LIT);
}


/* builtin */
void
genb_token(int val)
{
    const char *token = yymap(val);
    push_str(token, F_ID);
}


/* identifer, possible builtin */
void
genb_id(const char *str)
{
    push_str(str, F_ID);
}


/* symbol */
void
genb_sym(const char *str)
{
    push_str(str, F_SYM);
}


/* 03/20, register variable */
void
genb_reg(const char *name, int index)
{
    assert(index >= 0 && index < 64);           /* XXX: limit to 64 */
    if (index >= 64) {
       push_str(name, F_SYM);                   /* convert to a standard symbol reference */
    } else {
       push_str(name, F_REG);                   /* F_REG <symbol> <index> */
       push_byte(index);
    }
}


/* push string element */
static void
push_str(const char *str, int type)
{
    char *alloc_str, *cp, ch;
    const char *in;
    string_t *strp;
    SPBLK *sp;

    if (F_ID == type) {                         /* 10/11/10, only map builtin's */
        const int id = builtin_index(str);

        if (id >= 0) { /* F_ID <builtin> */
            push_byte(F_ID);
            push_byte(0);
            push_byte(0);
            LPUT16((LIST *)(byte_ptr - 3), (uint16_t)id);
            return;
        }

        type = F_SYM;                           /* non-builtin, remap */
    }

    /*
     *  Convert string losing the backslash sequences
     *      note these *should* match crlexer.c
     */
    cp = alloc_str = chk_alloc(strlen(str) + 1);

    for (in = str; 0 != (ch = *in);) {
        ++in;

        if ('\\' == ch) {
            ch = *in++;
            switch(ch) {            // fixed control
            case 'a':  ch = '\a'; break;
            case 'b':  ch = '\b'; break;
            case 'f':  ch = '\f'; break;
            case 'n':  ch = '\n'; break;
            case 'r':  ch = '\r'; break;
            case 't':  ch = '\t'; break;
            case 'v':  ch = '\v'; break;
            case 'e':  ch = 0x1b; break;
            case '"':
            case '?':
            case '\'':
            case '\\':
                break;
                                    // octal
            case '0': case '1': case '2': case '3':
            case '4': case '5': case '6': case '7': {
                    unsigned n = 0, len = 0;

                    while (len < 3) {
                        ch = *in;
                        if (isdigit(ch)) {
                            const int value = (ch - '0');
                            if (value >= 8) {
                                break;          // not 0..7
                            }
                            n <<= 3;
                            n += value;
                        } else {
                            break;
                        }
                        ++in, ++len;
                    }
                    if (0 == len) {
                        printf("crbin: invalid octal escape '\\0%c'\n", ch);
                        n = in[-1];
                    } else if (n > 0xff) {
                        printf("crbin: large octal escape (%d)", n);
                    }
                    ch = (char) n;
                }
                break;

            case 'x': case 'X': {   // hexidecimal
                    unsigned limit = ('X' == ch ? 4 : 2);
                    unsigned n = 0, len = 0;

                    if ('{' == *in) {           // extended hexidecimal
                        limit = 17;             // 64bit
                        ++in;
                    }
                    while (len < limit) {
                        ch = *in;
                        if (isdigit(ch)) {
                            n <<= 4;
                            n += (ch - '0');
                        } else if (isupper(ch)) {
                            n <<= 4;
                            n += (ch - 'A') + 10;
                        } else if (islower(ch)) {
                            n <<= 4;
                            n += (ch - 'a') + 10;
                        } else {
                            if ('}' == ch && 17 == limit) {
                                ++in;           // terminator
                            }
                            break;
                        }
                        ++in, ++len;
                    }

                    if (0 == len) {
                        printf("crbin: invalid hexidecimal escape '\\x%c'\n", ch);
                        n = 'x';
                    } else if (16 == limit) {
                        if ('}' != ch) {
                            printf("crbin: invalid extended hex escape, missing trailing '}'");
                            n = 'x';
                        }
                    } else if (n > 0xff) {
                        printf("crbin: large hex escape (%d)", n);
                    }
                    ch = (char) n;
                }
                break;

            case 0:                 //nul
                printf("crbin: missing escape\n");
                ch = '\\';
                --in;
                break;

            case 'o':               //octal extended
                if ('{' == *in) {
                    unsigned limit = 16;
                    unsigned n = 0, len = 0;

                    ++in;
                    while (len < limit) {
                        ch = *in;
                        if (isdigit(ch)) {
                            const int value = (ch - '0');
                            if (value >= 8) {
                                break;          // not 0..7
                            }
                            n <<= 3;
                            n += value;
                        } else {
                            if ('}' == ch) {
                                ++in;           // terminator
                            }
                            break;
                        }
                        ++in, ++len;
                    }
                    if (0 == len) {
                        printf("crbin: invalid octal escape '\\0%c'\n", ch);
                        n = 'o';
                    } else if ('}' != ch) {
                        printf("crbin: invalid extended octal escape, missing trailing '}'");
                        n = 'o';
                    } else if (n > 0xff) {
                        printf("crbin: large octal escape (%d)", n);
                    }
                    ch = (char) n;
                } else {
                    ch = 'o';
                }
                break;

            case 'u':               //unicode
            case 'U':
            default:
                printf("crbin: unknown escape '\\%c'\n", ch);
                break;
            }
        }
        *cp++ = ch;
    }
    *cp = 0;

    if (NULL != (sp = splookup(alloc_str, str_tree))) {
        strp = (string_t *) sp->data;
        chk_free(alloc_str);

    } else {
        sp = spblk(sizeof(string_t));
        strp = (string_t *) sp->data;
        sp->key = alloc_str;
        r_append(str_table, (char *) &alloc_str, sizeof(alloc_str), 128 * sizeof(char *));
        strp->str_index = str_index++;
        strp->str_str = alloc_str;
        ++cm_header.cm_num_strings;
        spenq(sp, str_tree);
    }

    assert(F_SYM == type || F_REG == type || F_LIT == type);
    assert(strp->str_index <= 0x0001ffff);      /* 128k limit */

    push_byte(type);
    push_int(strp->str_index);
}


void
genb_null(void)
{
    push_byte(F_NULL);
}


void
genb_macro(void)
{
    uint32_t offset = (uint32_t)(byte_ptr - object_code);

    ++cm_header.cm_num_macros;
    r_append(macro_offsets, (void *) &offset, sizeof(offset), 64 * sizeof(offset));
}


/* push an integer element */
static void
push_int(accint_t ival)
{
    LIST t_buf[sizeof(accint_t)+1];
    unsigned i;

    assert(SIZEOF_LONG == sizeof(accint_t));    /* verify env */
    LPUT_INT(t_buf, ival);
    for (i = 1; i < sizeof(t_buf); ++i) {
        push_byte(t_buf[i]);
    }
}


/* push a numeric/float element */
static void
push_float(accfloat_t fval)
{
    LIST t_buf[sizeof(accfloat_t)+1];
    unsigned i;

    assert(8 == sizeof(accfloat_t));            /* verify env */
    LPUT_FLOAT(t_buf, fval);
    for (i = 1; i < sizeof(t_buf); ++i) {
        push_byte(t_buf[i]);
    }
}


/* push a byte */
static void
push_byte(int b)
{
    uint32_t offset;

    if (byte_ptr > (object_code + object_allocated - 10)) {
        offset = (uint32_t)(byte_ptr - object_code);
        object_allocated += OBJECT_INCR;
        object_code =
            (unsigned char *) chk_realloc((void *) object_code, (int) object_allocated);
        byte_ptr = object_code + offset;
    }
    *byte_ptr++ = (unsigned char) b;
}


/*
 *  Now that compilation is finished, we can output the compiled code
 *
 *  TODO, enforce limits
 *
 *      CM_ATOMS_MAX            Max. number of atoms in a macro definition.
 *      CM_GLOBALS_MAX          Max. no. of global statements in program.
 *      CM_STRINGS_MAXS         Max. strings in macro file.
 */
void
genb_finish(void)
{
    uint32_t code_size, offset, j;
    uint32_t *maclp, *macend;
    const char **strp;

  //push_byte(F_END); /*1/4/2020*/

    /*
     *  Header
     */
    code_size = (uint32_t)(byte_ptr - object_code);
    if (3 & code_size) {                        /* round */
        code_size = (code_size | 3) + 1;
    }

    if (str_index > CM_STRINGS_MAX) {
        crerror(RC_ERROR, "internal string-table limit exceeded");
    }

    cm_header.cm_magic = CM_MAGIC;
    cm_header.cm_version = (uint16_t)cm_version;
    cm_header.cm_builtin = builtin_count;
    cm_header.cm_signature = builtin_signature;
    cm_header.cm_num_atoms = (uint32_t)(byte_ptr - object_code);
    cm_header.cm_globals = 0;                   /* not-used */
    cm_header.cm_num_globals = 0;               /* not-used */
    cm_xdr_export(&cm_header);

    fwrite(&cm_header, 1, sizeof(cm_header), x_bfp);

    maclp = (uint32_t *) r_ptr(macro_offsets);
    macend = maclp + (r_used(macro_offsets)/sizeof(uint32_t));
    while (maclp < macend) {
        uint32_t idx = *maclp++;

        idx = WPUT32(idx);                      /* 64/32BIT */
        fwrite(&idx, sizeof(idx), 1, x_bfp);
    }

    offset = WPUT32(code_size);
    fwrite(&offset, sizeof(offset), 1, x_bfp);  /* 64/32BIT */

    offset = cm_header.cm_num_strings;
    fwrite(&offset, sizeof(offset), 1, x_bfp);  /* 64/32BIT */

    /*
     *  Code
     */
    fwrite(object_code, code_size, 1, x_bfp);

    /*
     *  Strings/
     *      o offset table
     *      o string data/content
     */
    strp = (const char **) r_ptr(str_table);
    for (offset = 0, j = 0; j++ < str_index;) {
        const char *cp = *strp++;
        uint32_t o = WPUT32(offset);            /* 64/32BIT */

        fwrite(&o, sizeof(o), 1, x_bfp);
        offset += strlen(cp) + 1;
    }

    strp = (const char **) r_ptr(str_table);
    for (j = 0; j++ < str_index;) {
        const char *cp = *strp++;

        fwrite(cp, strlen(cp) + 1, 1, x_bfp);
    }
}

/*end*/
