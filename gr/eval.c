#include <edidentifier.h>
__CIDENT_RCSID(gr_eval_c,"$Id: eval.c,v 1.38 2020/06/05 13:06:21 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: eval.c,v 1.38 2020/06/05 13:06:21 cvsuser Exp $
 * Evaluator.
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

#include <libstr.h>                             /* str_...()/sxprintf() */
#include "accum.h"
#include "builtin.h"
#include "debug.h"
#include "echo.h"
#include "eval.h"
#include "keywd.h"
#include "lisp.h"
#include "macros.h"
#include "main.h"
#include "symbol.h"
#include "tty.h"

#define  WORD_INLINE        /* inline LIST interface */
#include "word.h"

static char                 x_nullstr[16] = {0};

static __CINLINE int
nullscan(void)
{
    register size_t i;

    for (i = 0; i < sizeof(x_nullstr) && 0 == x_nullstr[i]; ++i) {
        /*cont*/;
    }
    return (i == sizeof(x_nullstr));
}


// Evaluation, direct result
int
eval(register const LIST *lp, register LISTV *lpv)
{
    const char *name;
    SYMBOL *sp;
    int type;

    assert(nullscan());
    switch (*lp) {
    case F_INT:                     /* integers */
        lpv->l_int = LGET_INT(lp);
        lpv->l_flags = F_INT;
        return F_INT;

    case F_FLOAT:                   /* floats */
        lpv->l_float = LGET_FLOAT(lp);
        lpv->l_flags = F_FLOAT;
        return F_FLOAT;

    case F_LIT:                     /* string-literals */
        lpv->l_str = LGET_PTR2(const char, lp);
        lpv->l_flags = F_LIT;
        return F_LIT;

    case F_REG:                     /* registers, 01/04/2020 */
        if (NULL != (sp = sym_rlookup(lp[SIZEOF_VOID_P + 1]))) {
#if !defined(NDEBUG) && defined(DO_REGISTER_CHECK)
            SYMBOL *sp2 = sym_elookup(LGET_PTR2(const char, lp));
            assert(sp == sp2);
#endif  //_DEBUG
            goto symbols;
        }
        /*FALLTHRU*/

    case F_SYM:                     /* symbols */
        name = LGET_PTR2(const char, lp);
        sp = sym_lookup(name);
        if (sp) {
symbols:;   trace_sym_ref(sp);
            switch (sp->s_type) {
            case F_INT:
                lpv->l_int = sp->s_int;
                lpv->l_flags = F_INT;
                return F_INT;
            case F_FLOAT:
                lpv->l_float = sp->s_float;
                lpv->l_flags = F_FLOAT;
                return F_FLOAT;
            case F_STR:
                lpv->l_ref = sp->s_obj;
                lpv->l_flags = F_RSTR;
                return F_RSTR;
            case F_LIST:
                lpv->l_ref = sp->s_obj;
                lpv->l_flags = F_RLIST;
                return F_RLIST;
            case F_NULL:
                lpv->l_ref = 0;
                lpv->l_flags = F_NULL;
                return F_NULL;
            default:
                panic("eval: Unexpected symtype (%d)?", sp->s_type);
                lpv->l_ref = 0;
                lpv->l_flags = F_NULL;
                return F_ERROR;
            }
        }

#if defined(XXX_INDIRECT_EXEC)
        /*
         *  On an error, force the current macro to be aborted
         */
        const MACRO *mptr = macro_lookup(name);
        if (NULL != mptr) {
               /*
                *   XXX - issues
                *       no way of execution within the same env, module etc
                *       alt solution return macro name $xxxxx:macro, require builtin enhancements.
                */
            execute_nmacro(mptr->m_list);
            goto list;
        }
#endif
        ewprintf("Undefined symbol: %s", name);
      //acc_assign_null();                      /* XXX: 1/4/2020, review return value on error. */
      //    or set_return_error();
        set_return();
        lpv->l_int = 0;
        lpv->l_flags = F_NULL;
        return F_ERROR;

    case F_RSTR:                    /* string-references, 01/07; see sort_list() */
        lpv->l_ref = LGET_PTR2(ref_t, lp);
        lpv->l_flags = F_RSTR;
        return F_RSTR;

    case F_LIST:                    /* expressions */
        execute_macro(lp + CM_ATOM_LIST_SZ);
/*list:*/
        type = acc_get_type();
        switch (type) {
        case F_INT:
            lpv->l_int   = acc_get_ival();
            break;
        case F_FLOAT:
            lpv->l_float = acc_get_fval();
            break;
        case F_LIT:
        case F_STR:
            lpv->l_str   = acc_get_sval();
            break;
        case F_RSTR:
            lpv->l_ref   = acc_get_ref();
            break;
        case F_LIST:
        case F_RLIST:
            lpv->l_ref   = acc_get_ref();
            type = F_RLIST;
            break;
        case F_NULL:
            break;
        default:
            panic("eval: Unexpected accumlator type (%d)?", type);
            break;
        }
        lpv->l_flags = type;
        return type;

    case F_NULL:                    /* NULL list */
        lpv->l_flags = F_NULL;
        return F_NULL;

//  case F_RLIST:                   /* expression references */
//  case F_HALT:
//  case F_ID:
    default:                        /* unknown */
        panic("eval: Unexpected type (0x%x/%d)", *lp, *lp);
        break;
    }

    lpv->l_int = 0;
    lpv->l_flags = F_NULL;
    return F_ERROR;
}


// Evaluation, indirect result published to accumulator (see: do_if())
int
eval2(register const LIST *lp)
{
    const char *name;
    SYMBOL *sp;

    assert(nullscan());
    switch (*lp) {
    case F_INT:                     /* integers */
        acc_assign_int(LGET_INT(lp));
        return F_INT;

    case F_FLOAT:                   /* floats */
        acc_assign_float(LGET_FLOAT(lp));
        return F_FLOAT;

    case F_LIT:                     /* string-literals */
        acc_assign_str(LGET_PTR2(const char, lp), -1);
        return F_LIT;

    case F_REG:                     /* registers, 01/04/2020 */
        if (NULL != (sp = sym_rlookup(lp[SIZEOF_VOID_P + 1]))) {
#if !defined(NDEBUG) && defined(DO_REGISTER_CHECK)
            SYMBOL *sp2 = sym_elookup(LGET_PTR2(const char, lp));
            assert(sp == sp2);
#endif  //_DEBUG
            goto symbols;
        }
        /*FALLTHRU*/

    case F_SYM:                     /* symbols */
        name = LGET_PTR2(const char, lp);
        sp = sym_lookup(name);
        if (sp) {
symbols:;   trace_sym_ref(sp);
            switch (sp->s_type) {
            case F_INT:
                acc_assign_int(sp->s_int);
                return F_INT;
            case F_FLOAT:
                acc_assign_float(sp->s_float);
                return F_FLOAT;
            case F_STR:
                acc_assign_ref(sp->s_obj);
                return F_RSTR;
            case F_LIST:
                acc_assign_ref(sp->s_obj);
                return F_RLIST;
            case F_NULL:
                acc_assign_null();
                return F_NULL;
            default:
                panic("eval2: Unexpected symtype (%d)?", sp->s_type);
                return F_ERROR;
            }
        }
        ewprintf("Undefined symbol: %s", name);
        set_return();
        return F_ERROR;

    case F_RSTR:                    /* string-references */
        acc_assign_ref(LGET_PTR2(ref_t, lp));
        return F_RSTR;

    case F_LIST:                    /* expressions */
        execute_macro(lp + CM_ATOM_LIST_SZ);
        return acc_get_type();

    case F_NULL:                    /* NULL list */
        acc_assign_null();
        return F_NULL;

//  case F_RLIST:                   /* expression references */
//  case F_HALT:
//  case F_ID:
    default:                        /* unknown */
        panic("eval2: Unexpected type (0x%x/%d)", *lp, *lp);
        break;
    }
    return F_ERROR;
}


/*__CBOOL*/ int
listv_null(const LISTV *lvp)
{
    if (lvp && F_NULL == lvp->l_flags) {
        return TRUE;
    }
    return FALSE;
}


/*__CBOOL*/ int
listv_int(const LISTV *lvp, accint_t *val)
{
    if (lvp && F_INT == lvp->l_flags) {
        if (val) {
            *val = lvp->l_int;
        }
        return TRUE;
    }
    return FALSE;
}


/*__CBOOL*/ int
listv_float(const LISTV *lvp, accfloat_t *val)
{
    if (lvp && F_FLOAT == lvp->l_flags) {
        if (val) {
            *val = lvp->l_float;
        }
        return TRUE;
    }
    return FALSE;
}


/*__CBOOL*/ int
listv_str(const LISTV *lvp, const char **val)
{
    if (lvp) {
        switch (lvp->l_flags) {
        case F_LIT:
        case F_STR:
            if (val) {
                *val = lvp->l_str;
            }
            return TRUE;
        case F_RSTR:
            if (val) {
                *val = r_ptr(lvp->l_ref);
            }
            return TRUE;
        default:
            break;
        }
    }
    return FALSE;
}


const char *
listv_xstr(const LISTV *lvp)
{
    if (lvp) {
        switch (lvp->l_flags) {
        case F_LIT:
        case F_STR:
            return lvp->l_str;
        case F_RSTR:
            return r_ptr(lvp->l_ref);
        default:
            break;
        }
    }
    return NULL;
}


/*__CBOOL*/ int
listv_list(const LISTV *lvp, const LIST **val)
{
    if (lvp) {
        switch (lvp->l_flags) {
        case F_LIST:
            if (val) {
                *val = lvp->l_list;
            }
            return TRUE;
        case F_RLIST:
            if (val) {
                *val = (LIST *) r_ptr(lvp->l_ref);
            }
            return TRUE;
        default:
            break;
        }
    }
    return FALSE;
}


/*   Function:          do_cvt_to_object
 *      cvt_to_object primitive
 *
 *  Parameters:
 *      none.
 *
 *   Returns:
 *      nothing.
 *
 *<<GRIEF>>
    Macro: cvt_to_object - Convert string to object.

        declare
        cvt_to_object(string value, [int &length])

    Macro Description:
        The 'cvt_to_object()' primitive converts the initial portion of the string
        specified string 'value' into an underlying <int>, <float> or <string> object.

     Firstly, the input string is divided into three parts::

            o An initial, possibly empty, sequence of white-space characters.

            o Either:

                a) A subject sequence interpreted as string of characters enclosed by
                    either single(') or double(") quotes.

                b) A subject sequence interpreted as a integer constant represented in
                    some radix determined by the value of base.

                c) A subject sequence interpreted as a floating-point constant or
                    representing 'infinity' or 'NaN'.

            o A final string of one or more unrecognised characters, including the
                terminating null byte of the input string.

        Then they shall attempt to convert the subject sequence to either a string, 
        integer-constant or float-point constant, and return the result.

    Integer Values:

        If the value of base is 0, the expected form of the subject sequence is that of
        a decimal constant, octal constant, or hexadecimal constant, any of which may
        be preceded by a '+' or '-' sign.

        A decimal constant begins with a non-zero digit, and consists of a sequence of
        decimal digits.

        An octal constant consists of the prefix '0' optionally followed by a sequence
        of the digits '0' to '7' only.

        A binary constant consists of the prefix '0b' or '0B' followed by a sequence of
        the decimal digits '0' or '1'.

        A hexadecimal constant consists of the prefix '0x' or '0X' followed by a
        sequence of the decimal digits and letters 'a' (or 'A' ) to 'f' (or 'F' ) with
        values 10 to 15 respectively.

    Floating Point Values:

        A floating-point number should have the form "SI.FESX" containing at least a
        fractional value or exponent, where

            o *S* is the sign; may be "+", "-", or omitted.

            o *I* is the integer part of the mantissa.

            o *F* is the fractional part of the mantissa prefixed with a dot.

            o *X* is the exponent.
        
        Either 'I' or 'F' may be omitted, or both. The decimal point isn't necessary
        unless 'F' is present.

        A character sequence 'INF' or 'INFINITY', ignoring case, shall be interpreted
        as an infinity; if representable.

        A character sequence 'NAN' shall be interpreted as a quiet NaN, ignoring case, 
        shall be interpreted as an Quiet Not-A-Number; if representable.

    Macro Example:

(start code)
        int
        parsetoken(string line)
        {
            declare x;
            int len;

            x = cvt_to_object(line, len);
            switch (typeof(x)) {
            case "integer":   // integer constant.
                break;
            case "float":     // floating point constant.
                break;
            case "string":    // string constant.
                break;
            default:
                // error condition
                break;
            }
            return len;
        }
(end)

    Macro Parameters:
        value - String value to be parsed.
        length - Optional integer variable reference, if specified
            shall be populated with the number of characters consumed
            within the source string, including all consumed leading
            characters, for example white-space. Upon a parse error
            the populated length shall be 0.

    Macro Returns:
        The 'cvt_to_object()' primitive returns the value of the
        parsed object converted to the most suitable type, either
        as an <int>, <float> or <string> type. 
        
        Otherwise if the string cannot be parsed NULL is returned.

    Macro Portability:
        n/a

    Macro See Also:
        atoi, strtod
 */
void
do_cvt_to_object(void)          /* (string value, [string length]) */
{
    static const char *terms = "\"'+-.0123456789NnIi";

    const char *cursor = get_str(1), 
        *cvt = cursor;
    char ch;

    while ((ch = *cursor) != '\0') {
        if (strchr(terms, ch)) {                /* consume leading characters */
            break;
        }
        ++cursor;                                  
    }

    if ('\0' == ch) {                           /* NUL */
        acc_assign_null();

    } else if ('"' == ch || '\'' == ch) {       /* "<string>" or '<string>' */
        const char *str = ++cursor,
            term = ch;

        while (*cursor && term != *cursor) {
            if ('\\' == *cursor && cursor[1]) {
                ++cursor;
            }   
            ++cursor;
        }
        acc_assign_str(str, cursor - str);
        if (term == *cursor) ++cursor;

    } else {                                    /* numeric */
        accfloat_t dval;
        accint_t ival;
        int len;

        switch (str_numparse(cursor, &dval, &ival, &len)) {
        case NUMPARSE_INTEGER:
            acc_assign_int(ival);
            cursor += len;
            break;
        case NUMPARSE_FLOAT:
            acc_assign_float(dval);
            cursor += len;
            break;
        default:
            acc_assign_null();
            cvt = NULL;
            break;
        }
    }

    if (! isa_undef(2)) {
        if (cvt) {
            sym_assign_int(get_symbol(2), (accint_t) ((cursor - cvt) + 1));
        } else {
            sym_assign_int(get_symbol(2), (accint_t) 0);
        }
    }
}


/*  Function:           isa_undef
 *      Determine whether the specified argument is undefined, being either an
 *      explicit NULL value or omitted.
 *
 *  Parameters:
 *      argi - Argument index.
 *
 *  Returns:
 *      TRUE if either NULL or omitted, otherwise FALSE.
 */
/*__CBOOL*/ int
isa_undef(int argi)
{
    assert(argi > 0);
    if (argi < margc) {
        register const LISTV *lp = margv + argi;

        if (F_NULL != lp->l_flags) {
            return FALSE;                       /*!NULL*/
        }
    }
    return TRUE;                                /*NULL or omitted/undefined*/
}


/*  Function:           isa_null
 *      Determine whether the specified argument is NULL.
 *
 *  Parameters:
 *      argi - Argument index.
 *
 *  Returns:
 *      TRUE if NULL, otherwise FALSE.
 */
/*__CBOOL*/ int
isa_null(int argi)
{
    assert(argi > 0);
    if (argi < margc) {
        register const LISTV *lp = margv + argi;

        if (F_NULL == lp->l_flags) {
            return TRUE;                        /*NULL*/
        } else if (F_SYM == lp->l_flags) {
            return (F_NULL == lp->l_sym->s_type);
        }
    }
    return FALSE;                               /*!NULL*/
}


/*  Function:           isa_integer
 *      Determine whether the specified argument is an integer.
 *
 *  Parameters:
 *      argi - Argument index.
 *
 *  Returns:
 *      TRUE if an integer, otherwise FALSE.
 */
/*__CBOOL*/ int
isa_integer(int argi)
{
    assert(argi > 0);
    if (argi < margc) {
        register const LISTV *lp = margv + argi;

        if (F_INT == lp->l_flags) {
            return TRUE;
        } else if (F_SYM == lp->l_flags) {
            return (F_INT == lp->l_sym->s_type);
        }
    }
    return FALSE;
}


/*  Function:           isa_float
 *      Determine whether the specified argument is a float.
 *
 *  Parameters:
 *      argi - Argument index.
 *
 *  Returns:
 *      TRUE if a float, otherwise FALSE.
 */
/*__CBOOL*/ int
isa_float(int argi)
{
    assert(argi > 0);
    if (argi < margc) {
        register const LISTV *lp = margv + argi;

        if (F_FLOAT == lp->l_flags) {
            return TRUE;
        } else if (F_SYM == lp->l_flags) {
            return (F_FLOAT == lp->l_sym->s_type);
        }
    }
    return FALSE;
}


/*  Function:           isa_string
 *      Determine whether the specified argument is a string.
 *
 *  Parameters:
 *      argi - Argument index.
 *
 *  Returns:
 *      TRUE if a string, otherwise FALSE.
 */
/*__CBOOL*/ int
isa_string(int argi)
{
    assert(argi > 0);
    if (argi < margc) {
        register const LISTV *lp = margv + argi;

        switch (lp->l_flags) {
        case F_LIT:
        case F_STR:
        case F_RSTR:
            return TRUE;
        case F_SYM:
            switch (lp->l_sym->s_type) {
            case F_LIT:
            case F_STR:
            case F_RSTR:
                return TRUE;
            default:
                break;
            }
            break;
        default:
            break;
        }
    }
    return FALSE;
}


/*  Function:           isa_list
 *      Determine whether the specified argument is a LIST.
 *
 *  Parameters:
 *      argi - Argument index.
 *
 *  Returns:
 *      TRUE if a LIST, otherwise FALSE.
 */
/*__CBOOL*/ int
isa_list(int argi)
{
    assert(argi > 0);
    if (argi < margc) {
        register const LISTV *lp = margv + argi;

        switch (lp->l_flags) {
        case F_LIST:
        case F_RLIST:
            return TRUE;
        case F_SYM:
            switch (lp->l_sym->s_type) {
            case F_LIST:
            case F_RLIST:
                return TRUE;
            default:
                break;
            }
            break;
        default:
            break;
        }
    }
    return FALSE;
}


/*  Function:           get_str
 *      Retrieve the specified STRING argument.
 *
 *  Parameters:
 *      argi - Argument index.
 *
 *  Returns:
 *      Arguments string value, which in the event of the argument not being available shall be
 *      the global null string.
 */
const char *
get_str(int argi)
{
    assert(nullscan());
    assert(argi > 0);
    if (argi < margc) {                         /* 30/07/09, address uninit reads */
        register const LISTV *lp = margv + argi;

        assert(F_SYM != lp->l_flags);           /* LVAL, use get symbol */
        switch (lp->l_flags) {
        case F_LIT:
        case F_STR:
            return lp->l_str;
        case F_RSTR:
            return r_ptr(lp->l_ref);
        case F_NULL:
        case F_HALT:
            break;
        case F_INT:
            trace_log("warning: get_str(arg:%d of argc:%d) INT returning NULL.\n", argi, margc);
            return x_nullstr;
        case F_FLOAT:
            trace_log("warning: get_str(arg:%d of argc:%d) FLOAT returning NULL.\n", argi, margc);
            return x_nullstr;
        default:
            panic("get_str what? [%d of %d]->%d", argi, margc, lp->l_flags);
            break;
        }
    }

    /*
     *  primitive requested a non-optional argument yet the parameter wasnt available, as
     *  such represents are internal implementation error where the get_xstr() interface should
     *  be used in place of the get_str() interface.
     */
    trace_log("warning: get_str(arg:%d of argc:%d) returning NULL.\n", argi, margc);
    return x_nullstr;
}


/*  Function:           get_strlen
 *      Retrieve the specified STRING argument length. This interface is a
 *      more optimate method of determining string arguments taking advantage
 *      of storage knowledge.
 *
 *  Parameters:
 *      argi - Argument index.
 *
 *   Returns:
 *      Arguments string length in bytes, otherwise 0.
 */
int
get_strlen(int argi)
{
    assert(nullscan());
    assert(argi > 0);
    if (argi < margc) {                         /* 30/07/09, address uninit reads */
        register const LISTV *lp = margv + argi;

        assert(F_SYM != lp->l_flags);           /* LVAL, use get symbol */
        switch (lp->l_flags) {
        case F_STR:
        case F_LIT:
            return (int)strlen(lp->l_str);
        case F_RSTR:
            return r_used(lp->l_ref);
        case F_INT:
        case F_NULL:
        case F_FLOAT:
        case F_HALT:
            break;
        default:
            panic("get_strlen what? [%d of %d]->%d", argi, margc, lp->l_flags);
            break;
        }
    }
    trace_log("warning: get_strlen(arg:%d of argc:%d) returning 0.\n", argi, margc);
    return 0;
}


/*  Function:           get_xstr
 *      Retrieve the specified optional STRING argument.
 *
 *  Parameters:
 *      argi - Argument index.
 *
 *   Returns:
 *      Arguments string value, otherwise NULL.
 */
const char *
get_xstr(int argi)
{
    assert(argi > 0);
    if (argi < margc) {                         /* 30/07/09, address uninit reads */
        register const LISTV *lp = margv + argi;

        assert(F_SYM != lp->l_flags);           /* LVAL, use get symbol */
        switch (lp->l_flags) {
        case F_STR:
        case F_LIT:
            return lp->l_str;
        case F_RSTR:
            return r_ptr(lp->l_ref);
        default:
            break;
        }
    }
    return NULL;
}


/*  Function:           get_xcharacter
 *      Get optional character/byte argument
 *
 *  Parameters:
 *      argi - Argument index.
 *
 *  Returns:
 *      Arguments character value, otherwise -1.
 */
int
get_xcharacter(int argi)
{
    int value = -1;

    assert(argi > 0);
    if (argi < margc) {                         /* 30/07/09, address uninit reads */
        register const LISTV *lp = margv + argi;
        const char *str;

        assert(F_SYM != lp->l_flags);           /* LVAL, use get symbol */
        if (F_INT == lp->l_flags) {
            value = lp->l_int;

        } else if (NULL != (str = get_xstr(argi)) && *str) {
            value = (unsigned char)(*str);      /* return first character */
        }
    }
    return value;
}


/*  Function:           get_xinteger
 *      Retrieve the specified optional INTEGER argument.
 *
 *  Parameters:
 *      argi - Argument index.
 *      undef - Value return in the event of the argument missing.
 *
 *  Returns:
 *      Arguments integer value.
 */
int
get_integer(int argi)
{
    int value = 0;

    assert(argi > 0);
    if (argi < margc) {                         /* 30/07/09, address uninit reads */
        register const LISTV *lp = margv + argi;

        assert(F_SYM != lp->l_flags);           /* LVAL, use get symbol */
        if (F_INT == lp->l_flags) {
            value = (int) lp->l_int;
#if (ACCINT_SIZEOF > SIZEOF_INT)
            if (lp->l_int != (accint_t) value)
                if (xf_warnings) {
                    ewprintf("get_integer: lost of precision");
                }
#endif
        } else {
            panic("get_integer what? [%d of %d]->%d", argi, margc, lp->l_flags);
        }
    }
    return value;
}


/*  Function:           get_xinteger
 *      Retrieve the specified optional INTEGER argument.
 *
 *  Parameters:
 *      argi - Argument index.
 *      undef - Value return in the event of the argument missing.
 *
 *  Returns:
 *      Arguments integer value, otherwise the 'undef' value.
 */
int
get_xinteger(int argi, int undef)
{
    int value = undef;

    assert(argi > 0);
    if (argi < margc) {                         /* 30/07/09, address uninit reads */
        register const LISTV *lp = margv + argi;

        assert(F_SYM != lp->l_flags);           /* LVAL, use get symbol */
        if (F_INT == lp->l_flags) {
            value = (int) lp->l_int;
#if (ACCINT_SIZEOF > SIZEOF_INT)
            if (lp->l_int != (accint_t) value)
                if (xf_warnings) {
                    ewprintf("get_integer; lost of precision");
                }
#endif
        }
    }
    return value;
}


/*  Function:           get_accint
 *      Retrieve the specified INTEGER argument as an accumulator sized element.
 *
 *  Parameters:
 *      argi - Argument index.
 *
 *  Returns:
 *      Arguments integer value, otherwise the 'undef' value.
 */
accint_t
get_accint(int argi)
{
    accint_t value = 0;

    assert(argi > 0);
    if (argi < margc) {                         /* 30/07/09, address uninit reads */
        register const LISTV *lp = margv + argi;

        assert(F_SYM != lp->l_flags);           /* LVAL, use get symbol */
        if (F_INT == lp->l_flags) {
            value = lp->l_int;
        } else {
            panic("get_accint what? [%d of %d]->%d", argi, margc, lp->l_flags);
        }
    }
    return value;
}


/*  Function:           get_xaccint
 *      Retrieve the specified optional INTEGER argument as an accumulator sized element.
 *
 *  Parameters:
 *      argi - Argument index.
 *      undef - Value return in the event of the argument missing.
 *
 *  Returns:
 *      Arguments integer value, otherwise the 'undef' value.
 */
accint_t
get_xaccint(int argi, accint_t undef)
{
    accint_t value = undef;

    assert(argi > 0);
    if (argi < margc) {                         /* 30/07/09, address uninit reads */
        register const LISTV *lp = margv + argi;

        assert(F_SYM != lp->l_flags);           /* LVAL, use get symbol */
        if (F_INT == lp->l_flags) {
            value = lp->l_int;
        }
    }
    return value;
}


/*  Function:           get_accfloat
 *      Retrieve the specified FLOAT argument.
 *
 *  Parameters:
 *      argi - Argument index.
 *
 *  Returns:
 *      Arguments float value.
 */
accfloat_t
get_accfloat(int argi)
{
    accfloat_t value = 0.0;

    assert(argi > 0);
    if (argi < margc) {                         /* 30/07/09, address uninit reads */
        register const LISTV *lp = margv + argi;

        assert(F_SYM != lp->l_flags);           /* LVAL, use get symbol */
        if (F_FLOAT == lp->l_flags) {
            value = lp->l_float;
        } else {
            panic("get_accfloat what? [%d of %d]->%d", argi, margc, lp->l_flags);
        }
    }
    return value;
}


/*  Function:           get_xaccfloat
 *      Retrieve the specified optional FLOAT argument.
 *
 *  Parameters:
 *      argi - Argument index.
 *      undef - Value return in the event of the argument missing.
 *
 *  Returns:
 *      Arguments float value, otherwise the 'undef' value.
 */
accfloat_t
get_xaccfloat(int argi, accfloat_t undef)
{
    accfloat_t value = undef;

    assert(argi > 0);
    if (argi < margc) {                         /* 30/07/09, address uninit reads */
        register const LISTV *lp = margv + argi;

        assert(F_SYM != lp->l_flags);           /* LVAL, use get symbol */
        if (F_FLOAT == lp->l_flags) {
            value = lp->l_float;
        }
    }
    return value;
}


/*  Function:           get_list
 *      Retrieve the specified LIST argument
 *
 *  Parameters:
 *      argi - Argument index.
 *
 *  Returns:
 *      LIST address, otherwise NULL.
 */
const LIST *
get_list(int argi)
{
    assert(argi > 0);
    if (argi < margc) {                         /* 30/07/09, address uninit reads */
        register const LISTV *lp = margv + argi;

        assert(F_SYM != lp->l_flags);           /* LVAL, use get symbol */
        switch (lp->l_flags) {
        case F_LIST:
           return lp->l_list;
        case F_RLIST:
           return (LIST *) r_ptr(lp->l_ref);
        case F_NULL:
        case F_HALT:
           return NULL;
        default:
           panic("get_list: what? [%d of %d]->%d", argi, margc, lp->l_flags);
        }
    }
    trace_log("warning: get_list(arg:%d of argc:%d) returning NULL.\n", argi, margc);
    return NULL;
}


/*  Function:           get_listlen
 *      Retrieve the specified LIST argument
 *
 *  Parameters:
 *      argi - Argument index.
 *
 *  Returns:
 *      LIST length, otherwise -1.
 */
int
get_listlen(int argi)
{
    assert(argi > 0);
    if (argi < margc) {                         /* 30/07/09, address uninit reads */
        register const LISTV *lp = margv + argi;

        assert(F_SYM != lp->l_flags);           /* LVAL, use get symbol */
        switch (lp->l_flags) {
        case F_LIST:
            return -1;
        case F_RLIST:
            return r_used(lp->l_ref);
        case F_NULL:
        case F_HALT:
            return 0;
        default:
            panic("get_listlen: what? [%d of %d]->%d", argi, margc, lp->l_flags);
            break;
        }
    }
    return -1;
}


/*  Function:           get_xlist
 *      Retrieve the specified optional LIST argument.
 *
 *  Parameters:
 *      argi - Argument index.
 *
 *  Returns:
 *      LIST address, otherwise NULL.
 */
const LIST *
get_xlist(int argi)
{
    assert(argi > 0);
    if (argi < margc) {                         /* 30/07/09, address uninit reads */
        register const LISTV *lp = margv + argi;

        assert(F_SYM != lp->l_flags);           /* LVAL, use get symbol */
        switch (lp->l_flags) {
        case F_LIST:
            return lp->l_list;
        case F_RLIST:
            return (LIST *) r_ptr(lp->l_ref);
        default:
            break;
        }
    }
    return NULL;
}


SYMBOL *
get_symbol(int argi)
{
    assert(argi > 0);
    if (argi < margc) {
        register const LISTV *lp = margv + argi;

        assert(F_SYM == lp->l_flags);           /* LVAL assumed */
        return lp->l_sym;
    }
    return NULL;
}


/*  Function:           get_iarg1
 *      Retrieve a INTEGER argument, prompting if non was specified.
 *
 *  Parameters:
 *      prompt - Command line prompt.
 *      ival - Address of variable populated with the resulting numeric value.
 *
 *  Returns:
 *      0 on success, otherwise -1 on an aborted prompt.
 */
int
get_iarg1(const char *prompt, accint_t *ival)
{
    int ret = 0;

    if (isa_integer(1)) {
        *ival = get_xinteger(1, 0);

    } else {
        char buf[MAX_CMDLINE] = {0};
        const LISTV *saved_argv;
        int saved_argc;

        saved_argc = margc;
        saved_argv = margv;
        if (ereply(prompt, buf, sizeof(buf)) != TRUE || !*buf) {
            ret = -1;
        } else {
            eclear();
            if (1 != sscanf(buf, "%" ACCINT_FMT, ival)) {
                *ival = 0;
                ret = -1;
            }
        }
        margv = saved_argv;
        margc = saved_argc;
    }
    return ret;
}


/*  Function:           get_arg1
 *      Retrieve a STRING argument, prompting if non was specified.
 *
 *  Parameters:
 *      prompt - Command line prompt.
 *      buf - Buffer used using prompt.
 *      bufsize - Size of the buffer, in bytes.
 *
 *  Returns:
 *      0 on success, otherwise -1 on an aborted prompt.
 */
const char *
get_arg1(const char *prompt, char *buf, int bufsiz)
{
    return get_xarg(1, prompt, buf, bufsiz);
}


const char *
get_xarg(int argi, const char *prompt, char *buf, int bufsiz)
{
    const char *ret;

    assert(argi >= 1);
    if (NULL == (ret = get_xstr(argi))) {
        int saved_argc;
        const LISTV *saved_argv;

        saved_argc = margc;
        saved_argv = margv;
        if (ereply(prompt, buf, bufsiz) == TRUE && *buf) {
            register char *cp;

            for (cp = buf; *cp; ++cp) {
                if (*cp != ' ') {               /* first not-blank */
                    break;
                }
            }
            ret = (*cp == '\0' ? NULL : buf);
        }
        margv = saved_argv;
        margc = saved_argc;
    }
    return ret;
}

/*end*/
