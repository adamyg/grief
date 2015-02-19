#include <edidentifier.h>
__CIDENT_RCSID(gr_math_c,"$Id: math.c,v 1.30 2014/11/27 18:56:53 ayoung Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: math.c,v 1.30 2014/11/27 18:56:53 ayoung Exp $
 * Math operators/primitives.
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

#include "accum.h"
#include "builtin.h"
#include "debug.h"
#include "echo.h"
#include "eval.h"
#include "keywd.h"
#include "lisp.h"
#include "m_string.h"
#include "main.h"
#include "math.h"
#include "symbol.h"
#include "word.h"

#ifndef NO_FLOAT_MATH
#include <math.h>
#endif

static int              lst_append(SYMBOL *sp, const LISTV *lvp);


/*<<GRIEF>>
    Operator: = - Assignment operator.

        var = expr1

    Macro Description:
        The '=' operator applies simple assignment, in which the value of
        the second operand 'expr1' is stored in the object specified by
        the first operand 'var'.

    Macro Parameters:
        var - Any scalar type.
        expr1 - An expression resulting in value of the same type as
            the scalar type 'var'.

    Macro Returns:
        The accumulator shall be set to the result of the assignment.

    Macro See Also:
        Operators, declare
 */


/*<<GRIEF>>
    Operator: *= - Multiplication assignment operator.

        var *= expr1

    Macro Description:
        The '*=' operator multiplies the value of the first operand 'var'
        by the value of the second operand 'expr1'; store the result in
        the object specified by the first operand 'var'.

    Macro Parameters:
        var - Any scalar type.
        expr1 - A numeric expression.

    Macro Returns:
        The accumulator shall be set to the result of the assignment.

    Macro See Also:
        Operators, declare
 */


/*<<GRIEF>>
    Operator: /= - Division assignment operator.

        var /= expr1

    Macro Description:
        The '/=' operator divides the value of the first operand 'var' by
        the value of the second operand 'expr1', storing the result in
        the object specified by the first operand 'var'.

    Macro Parameters:
        var - A numeric scalar type.
        expr1 - A numeric expression.

    Macro Returns:
        The accumulator shall be set to the result of the assignment.

    Macro See Also:
        Operators, declare
 */


/*<<GRIEF>>
    Operator: %= - Modulus assignment operator.

        var %= expr1

    Macro Description:
        The '%=' operator takes the modulus of the first operand 'var'
        specified by the value of the second operand 'expr1', storing the
        result in the object specified by the first operand 'var'.

    Macro Parameters:
        var - A numeric scalar type.
        expr1 - A numeric expression.

    Macro Returns:
        The accumulator shall be set to the result of the assignment.

    Macro See Also:
        Operators, declare
 */


/*<<GRIEF>>
    Operator: += - Addition assignment operator.

        var += expr1

    Macro Description:
        The '+=' operator adds the value of the second operand to the
        value of the first operand 'var', storing the result in the
        object specified by the first operand 'var'.

    Macro Parameters:
        var - Any scalar type.
        expr1 - An expression resulting in value of the same type as
            the scalar type 'var'.

    Macro Returns:
        The accumulator shall be set to the result of the assignment.

    Macro See Also:
        Operators, declare
 */


/*<<GRIEF>>
    Operator: -= - Subtraction

        var -= expr1

    Macro Description:
        The '-=' operator subtracts the value of the second operand
        'expr1' from the value of the first operand 'var', storing the
        result in the object specified by the first operand 'var'.

    Macro Parameters:
        var - A numeric scalar type.
        expr1 - A numeric expression.

    Macro Returns:
        The accumulator shall be set to the result of the assignment.

    Macro See Also:
        Operators, declare
 */


/*<<GRIEF>>
    Operator: <<= - Left-shift assignment operator.

        var <<= expr1

    Macro Description:
        The '<<=' operator shift the value of the first operand 'var'
        left the number of bits specified by the value of the second
        operand 'expr1', storing the result in the object specified by
        the first operand 'var'.

    Macro Parameters:
        var - A numeric scalar type.
        expr1 - A numeric expression.

    Macro Returns:
        The accumulator shall be set to the result of the assignment.

    Macro See Also:
        Operators, declare
 */


/*<<GRIEF>>
    Operator: >>= - Right-shift assignment operator.

        var >>= expr1

    Macro Description:
        The '>>=' operator shift the value of the first operand 'var'
        right the number of bits specified by the value of the second
        operand 'expr1', storing the result in the object specified by
        the first operand 'var'.

    Macro Parameters:
        var - A numeric scalar type.
        expr1 - A numeric expression.

    Macro Returns:
        The accumulator shall be set to the result of the assignment.

    Macro See Also:
        Operators, declare
 */


/*<<GRIEF>>
    Operator: &= - Bitwise AND assignment operator.

        var &= expr1

    Macro Description:
        The '&=' operator obtains the bitwise AND of the first operand
        'var' and second operand 'expr1', storing the result in the
        object specified by the first operand 'var'.

    Macro Parameters:
        var - A numeric scalar type.
        expr1 - A numeric expression.

    Macro Returns:
        The accumulator shall be set to the result of the assignment.

    Macro See Also:
        Operators, declare
 */


/*<<GRIEF>>
    Operator: ^= - Bitwise exclusive OR assignment operator.

        var ^= expr1

    Macro Description:
        The '^=' operator obtains the bitwise exclusive OR of the
        first operand 'var' and second operand 'expr1', storing the
        result in the object specified by the first operand 'var'.

    Macro Parameters:
        var - A numeric scalar type.
        expr1 - A numeric expression.

    Macro Returns:
        The accumulator shall be set to the result of the assignment.

    Macro See Also:
        Operators, declare
 */


/*<<GRIEF>>
    Operator: |= - Bitwise OR assignment operator.

        var |= expr1

    Macro Description:
        The '|=' operator obtains the bitwise inclusive OR of the
        first operand 'var' and second operand 'var', storing the
        result in the object specified by the first operand 'var'.

    Macro Parameters:
        var - A numeric scalar type.
        expr1 - A numeric expression.

    Macro Returns:
        The accumulator shall be set to the result of the assignment.

    Macro See Also:
        Operators, declare
 */


/*  Function:           com_equ
 *      Assignment Operators.
 *
 *  Macro Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 */
void
do_com_equ(int op)
{
    (void) com_equ(op, get_symbol(1), margv + 2);
}


int
com_equ(int op, register SYMBOL *sp, const LISTV *lvp)
{
#define lvp_int         lvp->l_int
#define lvp_float       lvp->l_float
#define lvp_str         lvp->l_str
#define lvp_list        lvp->l_list
#define lvp_ref         lvp->l_ref

    LISTV nvalue = *lvp;                        /* local working copy, used during conversions */
    OPCODE type = lvp->l_flags;
    accint_t rvalue = 0;
    char buf[32];

    lvp = &nvalue;                              /* re-associate to localised copy */

    if (F_LIST != sp->s_type && F_NULL == type && 0 == (sp->s_flags & SF_POLY)) {
        ewprintf("Missing assignment value.");
        return -1;
    }

    if (sp->s_type != type) {
        /*
         *  type conversion
         */
        if (MOP_NOOP == op && (sp->s_flags & SF_POLY)) {
            switch (sp->s_type) {
            case F_STR:
                r_dec(sp->s_obj);
                break;
            case F_LIST:
                r_dec(sp->s_obj);
                if (type == F_NULL) {
                    sp->s_obj = r_inc(x_halt_list);
                    acc_assign_null();
                    return 0;
                }
                break;
            default:
                break;
            }
            sp->s_obj = NULL;
            sp->s_type =
                type == F_RLIST ? F_LIST :
                type == F_RSTR ? F_STR :
                type == F_LIT ? F_STR :
                    type;
            goto ok_check;
        }

        if (F_STR == sp->s_type) {
            switch (type) {
            case F_RSTR:
            case F_LIT:
                break;
            case F_INT:
                sprintf(buf, "%" ACCINT_FMT, lvp_int);
                nvalue.l_str = buf;
                nvalue.l_flags = F_STR;
                type = F_STR;
                break;
            case F_FLOAT:
                sprintf(buf, "%" ACCFLOAT_FMT, lvp_float);
                nvalue.l_str = buf;
                nvalue.l_flags = F_STR;
                type = F_STR;
                break;
            default:
                goto bad_asgn;
            }
            goto ok_check;
        }

        if (F_NULL == type) {
            ;

        } else if (sp->s_type == F_INT && type == F_FLOAT) {
            /*
             *  Cast float value to an int
             */
            rvalue = (accint_t) lvp_float;
            nvalue.l_int = rvalue;
            type = F_INT;

        } else if (sp->s_type == F_FLOAT && type == F_INT) {
            /*
             *  Cast int value to a float
             */
            nvalue.l_float = (accfloat_t) lvp_int;
            type = F_FLOAT;

        } else if (sp->s_type == F_LIST  && op == MOP_PLUS) {
            /*
             *  Append the object
             */
            return lst_append(sp, lvp);

        } else if (sp->s_type == F_LIST  && type == F_RLIST) {
            /*
             *  Nothing to do here
             */

        } else {
bad_asgn:   ewprintf("Mixed types in assignment: %s", sp->s_name);
            return -1;
        }
    }

ok_check:
    if (F_INT == type) {
        rvalue = lvp_int;
    }

    switch (op) {
    case MOP_NOOP:          /* = */
        switch (type) {
        case F_INT:
            sp->s_int = rvalue;
            break;
        case F_FLOAT:
            sym_assign_float(sp, lvp_float);
            acc_assign_float(lvp_float);
            return 0;
        case F_LIT:
        case F_STR:
            sym_assign_str(sp, lvp_str);
            acc_assign_ref(sp->s_obj);
            return 0;
        case F_RSTR:
            assert(sp->s_type == F_STR);
            sym_assign_ref(sp, lvp_ref);
            acc_assign_ref(lvp_ref);
            return 0;
        case F_RLIST:
            assert(sp->s_type == F_LIST);
            sym_assign_ref(sp, lvp_ref);
            acc_assign_ref(lvp_ref);
            return 0;
        case F_LIST: {
                ref_t *rp;

                if (NULL != (rp = rlst_build(lvp_list, -1))) {
                    sym_donate_ref(sp, rp);
                    acc_assign_ref(rp);
                }
            }
            return 0;
        case F_NULL:
            assert(sp->s_type == F_STR || sp->s_type == F_LIST || sp->s_type == F_NULL);
            if (sp->s_obj) {
                r_dec(sp->s_obj);
                sp->s_obj = r_inc(x_halt_list);
            }
            acc_assign_null();
            return 0;
        default:
            panic("com_equ: what do i do? (%d)", type);
            break;
        }
        break;

    case MOP_PLUS:          /* += */
        switch (type) {
        case F_LIT:
        case F_STR:
            sp->s_obj = r_cat(sp->s_obj, lvp_str);
            trace_sym(sp);
            acc_assign_ref(sp->s_obj);
            return 0;
        case F_RSTR:
            sp->s_obj = r_cat(sp->s_obj, (const char *)r_ptr(lvp_ref));
            trace_sym(sp);
            acc_assign_ref(sp->s_obj);
            return 0;
        case F_FLOAT:
            sym_assign_float(sp, sp->s_float + lvp_float);
            acc_assign_float(sp->s_float);
            return 0;
        case F_LIST:
            return lst_append(sp, lvp);
        default:
            sp->s_int += rvalue;
            break;
        }
        break;

    case MOP_MINUS:         /* -= */
        if (F_FLOAT == type) {
            sym_assign_float(sp, sp->s_float - lvp_float);
            acc_assign_float(sp->s_float);
            return 0;
        }
        sp->s_int -= rvalue;
        break;

    case MOP_MULTIPLY:      /* *= */
        if (F_FLOAT == type) {
            sym_assign_float(sp, sp->s_float * lvp_float);
            acc_assign_float(sp->s_float);
            return 0;
        }
        sp->s_int *= rvalue;
        break;

    case MOP_DIVIDE:        /* /= */
        if (F_FLOAT == type) {
            if (lvp_float != 0.0) {
                sym_assign_float(sp, sp->s_float / lvp_float);
                acc_assign_float(sp->s_float);
            }
            return 0;
        }
        if (0 == rvalue) {
            ewprintf("Divide by zero.");
            rvalue = 1;
        }
        sp->s_int /= rvalue;
        break;

    case MOP_MODULO:        /* %= */
        if (0 == rvalue) {
            ewprintf("Modula by zero.");
            rvalue = 1;
        }
        sp->s_int %= rvalue;
        break;

    case MOP_BAND:          /* &= */
        sp->s_int &= rvalue;
        break;

    case MOP_BOR:           /* |= */
        sp->s_int |= rvalue;
        break;

    case MOP_BXOR:          /* ^= */
        sp->s_int ^= rvalue;
        break;

    case MOP_LSHIFT:        /* <<= */
        sp->s_int <<= rvalue;
        break;

    case MOP_RSHIFT:        /* >>= */
        sp->s_int >>= rvalue;
        break;

    default:                /* unknown */
        panic("com_equ: bad op (%d)", op);
        break;
    }
    acc_assign_int(sp->s_int);
    trace_sym(sp);
    return 0;

#undef  lvp_int
#undef  lvp_float
#undef  lvp_str
#undef  lvp_list
#undef  lvp_ref
}


/*  Function:           lst_append
 *      Append to the end of the specified list.
 *
 *      Note list handling flattens list at level 0. Hence when used in the following
 *      context the result is;
 *
 *          list lst = make_list("a", "b");
 *
 *          lst += lst;
 *          // result = "a", "b", "a", "b"
 *
 *      The alternative are the <splice> or <push> functions
 *
 *          splice(lst, length_of_list(lst), 0, lst);
 *
 *      which shall place a list reference within the lst.
 */
static int
lst_append(SYMBOL *sp, const LISTV *lvp)
{
    ref_t *rp, *nrp;

    rp = sp->s_obj;
    if (NULL != (nrp = rlst_splice(rp, -1, 0, lvp, 1, TRUE))) {
        if (sp->s_obj != nrp) {
            sym_donate_ref(sp, nrp);
        }
        rp = nrp;
    }
    acc_assign_ref(rp);
    return 0;
}


/*<<GRIEF>>
    Operator: == - Equality operator.

        expr1 == expr2

   Macro Description:
        The '==' operator compares for equality, yielding the value '1'
        if the relation is true, and '0' if the relation is false. The
        result type is 'int'.

    Macro Parameters:
        expr1 - Left expression.
        expr2 - Right expression.

    Macro Returns:
        The accumulator shall be set to the result of the comparison.

    Limitations:
        Lists can not be compared.

    Macro See Also:
        Operators, declare
 */


/*<<GRIEF>>
    Operator: != - Non-equality operator.

        expr1 != expr2

    Macro Description:
        The '!=' operator compares for inequality, yielding the value
        '1' if the relation is true, and '0' if the relation is false.
        The result type is 'int'.

    Macro Parameters:
        expr1 - Left expression.
        expr2 - Right expression.

    Macro Returns:
        The accumulator shall be set to the result of the comparison.

    Limitations:
        Lists can not be compared.

    Macro See Also:
        Operators, declare
 */


/*<<GRIEF>>
    Operator: > - Greater than comparison.

        expr1 > expr2

    Macro Description:
        The '>' operator performs greater-then comparisons between the
        operands 'expr1' and 'expr2'.

    Macro Parameters:
        expr1 - Left expression.
        expr2 - Right expression.

    Macro Returns:
        The accumulator shall be set to the result of the comparison.

    Macro See Also:
        Operators, declare
 */


/*<<GRIEF>>
    Operator: >= - Greater than or equal comparison.

        expr1 >= expr2

    Macro Description:
        The '>' operator performs greater-then-or-equal comparisons
        between the operands 'expr1' and 'expr2'.

    Macro Parameters:
        expr1 - Left expression.
        expr2 - Right expression.

    Macro Returns:
        The accumulator shall be set to the result of the comparison.

    Macro See Also:
        Operators, declare
 */


/*<<GRIEF>>
    Operator: < - Less than comparison.

        expr1 < expr2

    Macro Description:
        The '<' operator performs less-then comparisons between the
        operands 'expr1' and 'expr2'.

    Macro Parameters:
        expr1 - Left expression.
        expr2 - Right expression.

    Macro Returns:
        The accumulator shall be set to the result of the comparison.

    Macro See Also:
        Operators, declare
 */


/*<<GRIEF>>
    Operator: <= - Less than or equal comparison.

        expr1 <= expr2

    Macro Description:
        The '<=' operator performs less-then comparisons between the
        operands 'expr1' and 'expr2'.

    Macro Parameters:
        expr1 - Left expression.
        expr2 - Right expression.

    Macro Returns:
        The accumulator shall be set to the result of the comparison.

    Macro See Also:
        Operators, declare
 */


/*<<GRIEF>>
    Operator: ! - Not operator.

        !expr

    Macro Description:
        The '!' operator yields the logical not of the expression 'expr'.

        If the operand has the value zero, then the result value is 1.

        If the operand has some other value, then the result is 0.

    Macro Parameters:
        expr - Expression.

    Macro Returns:
        Returns the logical not of the expression expr. expr must
        evaluate to an integer expression.

    Macro See Also:
        Operators, declare
 */


/*<<GRIEF>>
    Operator: * - Multiplication operator.

        exp1 * expr2

    Macro Description:
        The '*' operator yields the product of its operands 'expr1' and
        'expr2'. The operands must have numeric types.

    Macro Parameters:
        expr1 - Left numeric expression.
        expr2 - Right numeric expression.

    Macro Returns:
        The accumulator shall be set to the result of the computation.

    Notes:
        If either number is NaN, the result is NaN. Multiplication of
        Infinity by zero gives a result of NaN, while multiplying
        Infinity by any non-zero number gives a result of Infinity.

    Macro See Also:
        Operators, declare, isnan, isfinite
 */


/*<<GRIEF>>
    Operator: / - Division operator.

        expr1 / expr2

    Macro Description:
        The '/' operator yields the quotient from the division of the
        first operand by the second operand. The operands must have
        numeric types.

    Macro Parameters:
        expr1 - Left numeric expression.
        expr2 - Right numeric expression.

    Macro Returns:
        The accumulator shall be set to the result of the computation.

    Notes:
        If number1 is a finite, non-zero number, and number2 is zero,
        the result of the division is Infinity if the number1 is
        positive, and -Infinity if negative. If both number1 and
        number2 are zero, the result is NaN.

    Macro See Also:
        Operators, declare, isnan, isfinite
 */


/*<<GRIEF>>
    Operator: % - Modulus operator.

        expr1 % expr2

    Macro Description:
        The '%' operator yields the remainder from the division of
        the first operand by the second operand. The operands of must
        have numeric types.

    Macro Parameters:
        expr1 - Left expression.
        expr2 - Right expression.

    Macro Returns:
        The accumulator shall be set to the result of the computation.

    Macro See Also:
        Operators, declare
 */

/*<<GRIEF>>
    Operator: + - Addition operator.

        expr1 + expr2

    Macro Description:
        The '+' operator yields the sum of its operands resulting from
        the addition of the first operand with the second.

        If 'expr1 or 'expr2' is a list, then a new list is returned which
        is the concatenation of expr1 and expr2.

        If 'expr1' or 'expr2' is a string, then the other operand is
        converted to a string, and a string is returned.

        If either operand is a floating point number then the other the
        result is the sum of the two expressions.

        Otherwise the integer value of the sum of the two expressions is
        returned.

    Macro Parameters:
        expr1 - Left expression.
        expr2 - Right expression.

    Macro Returns:
        The accumulator shall be set to the result of the computation.

        A list value if either operand is a list; a string if either
        operand is a string; a float if either operand is a float;
        otherwise an integer value.

    Macro See Also:
        Operators, declare
 */


/*<<GRIEF>>
    Operator: - - Subtraction operator.

        expr1 - expr2

    Macro Description:
        The '-' operator yields the difference resulting from the
        subtraction of the second operand from the first.

    Macro Parameters:
        expr1 - Left expression.
        expr2 - Right expression.

    Macro Returns:
        The accumulator shall be set to the result of the computation.

    Macro See Also:
        Operators, declare
 */


/*<<GRIEF>>
 *  Operator: << - Left-shift operator.
 *
 *      expr1 << expr2
 *
 *  Macro Description:
        The '<<' operator shift the value of the first operand 'expr1'
        left the number of bits specified by the value of the second
        operand 'expr1'.

        Both operands must have an integral type, and the integral
        promotions are performed on them. The type of the result is the
        type of the promoted left operand.

    Macro Parameters:
        expr1 - Left expression.
        expr2 - Right expression.

    Macro Returns:
        The accumulator shall be set to the result of the computation.

    Macro See Also:
            Operators, declare
 */


/*<<GRIEF>>
    Operator: >> - Right-shift operator.

        expr1 >> expr2

    Macro Description:
        The '>>' operator shift the value of the first operand 'expr1'
        right the number of bits specified by the value of the second
        operand 'expr1'.

        Both operands must have an integral type, and the integral
        promotions are performed on them. The type of the result is the
        type of the promoted left operand.

    Macro Parameters:
        expr1 - Left expression.
        expr2 - Right expression.

    Macro Returns:
        The accumulator shall be set to the result of the computation.

    Macro See Also:
        Operators, declare
 */


/*<<GRIEF>>
 *  Operator: & - Bitwise AND operator.
 *
 *      expr1 & expr2
 *
 *  Macro Description:
        The '&' operator yields the result is the *bitwise AND* of the
        two operands 'expr1' and 'expr2'. That is, the bit in the result
        is set if and only if each of the corresponding bits in the
        operands are set.

    Macro Parameters:
        expr1 - Left expression.
        expr2 - Right expression.

    Macro Returns:
        The accumulator shall be set to the result of the computation.

    Macro See Also:
        Operators, declare
 */


/*<<GRIEF>>
    Operator: ~ - Bitwise complement.

        ~expr

    Macro Description:
        The '^' operator yields the bitwise complement 1's complement or
        bitwise not operator of 'expr'.

        The type of the operand must be an integral type, and integral
        promotion is performed on the operand. The type of the result is
        the type of the promoted operand.

        Each bit of the result is the complement of the corresponding bit
        in the operand, effectively turning 0 bits to 1, and 1 bits to 0.
        The ! symbol is the logical not operator. Its operand must be a
        scalar type (not a structure, union or array). The result type is
        int.

        If the operand has the value zero, then the result value is 1.

        If the operand has some other value, then the result is 0.

    Macro Parameters:
        expr - Expression.

    Macro Returns:
        The accumulator shall be set to the result of the computation.

    Macro See Also:
        Operators, declare
 */


/*<<GRIEF>>
    Operator: ^ - Bitwise exclusive OR operator.

        expr1 ^ expr2

    Macro Description:
        The '^' operator yields the result is the bitwise exclusive OR of
        the two operands.

        That is, the bit in the result is set if and only if exactly one
        of the corresponding bits in the operands are set.

    Macro Parameters:
        expr1 - Left expression.
        expr2 - Right expression.

    Macro Returns:
        The accumulator shall be set to the result of the computation.

    Macro See Also:
        Operators, declare
 */


/*<<GRIEF>>
 *  Operator: | - Bitwise OR operator.
 *
 *      expr1 | expr2
 *
 *  Macro Description:
        The '|' operator yields the result is the bitwise *inclusive OR*
        of the two operands 'expr1' and 'expr2'. That is, the bit in the
        result is set if at least one of the corresponding bits in the
        operands is set.

    Macro Parameters:
        expr1 - Left expression.
        expr2 - Right expression.

    Macro Returns:
        The accumulator shall be set to the result of the computation.

    Macro See Also:
        Operators, declare
 */


/*<<GRIEF>>
    Operator: <=> - Comparison operator.

        expr1 <=> expr2

    Macro Description:
        The '<=>' operator yields the value -1 if the first expression is
        less then the second, 0 if the equals, and 1 the greater than;
        which are arithmetic and lexicographically comparisons
        respectively.

    Macro Parameters:
        expr1 - Left expression.
        expr2 - Right expression.

    Macro Returns:
        The accumulator shall be set to the result of the comparison.

    Limitations:
        Lists can not be compared.

    Macro See Also:
        Operators, declare
 */


/*<<GRIEF>>
   Macro: above - Greater than comparison.

        int
        above(declare expr1, declare expr2)

   Macro Description:
        The 'above()' primitive is the functional form of the '>' operator.

        The function yields the comparison of the two arguments
        'expr1' and 'expr2' which both must be of equivalent types,
        either numeric or string; which are arithmetic and
        lexicographically comparisons respectively.

    Macro Parameters:
        expr1 - Left expression, numeric or string.
        expr2 - Right expression, numeric or string.

    Macro Returns:
        The 'above()' primitive yields the value 1 if 'expr1' is greater
        then 'expr2', and 0 if the relation is false.

    Limitations:
        Lists can not be compared.

    Macro See Also:
        Operators, >
 */


/*<<GRIEF>>
    Macro: above_eq - Greater than or equal comparison.

        int
        above_eq(declare expr1, declare expr2)

    Macro Description:
        The 'above_eq()' primitive is the functional form of the '>='
        operator.

        The function yields the comparison of the two arguments
        'expr1' and 'expr2' which both must be of equivalent types,
        either numeric or string; which are arithmetic and
        lexicographically comparisons respectively.

    Macro Parameters:
        expr1 - Left expression, numeric or string.
        expr2 - Right expression, numeric or string.

    Macro Returns:
        The 'above_eq()' primitive yields the value 1 if 'expr1' is greater
        then or equals 'expr2', and 0 if the relation is false.

    Macro See Also:
        Operators, >=
 */


/*<<GRIEF>>
    Macro: below - Less than comparison.

        int
        below(declare expr1, declare expr2)

    Macro Description:
        The 'below()' primitive is the functional form of the '<' operator.

        The function yields the comparison of the two arguments
        'expr1' and 'expr2' which both must be of equivalent types,
        either numeric or string; which are arithmetic and
        lexicographically comparisons respectively.

    Macro Parameters:
        expr1 - Left expression, numeric or string.
        expr2 - Right expression, numeric or string.

    Macro Returns:
        The 'below()' primitive yields the value 1 if 'expr1' is less than
        'expr2', and 0 if the relation is false.

    Limitations:
        Lists can not be compared.

    Macro See Also:
        Operators, <
 */


/*<<GRIEF>>
    Macro: below_eq - Less than or equal comparison.

        int
        below_eq(declare expr1, declare expr2)

    Macro Description:
        The 'below_eq()' primitive is the functional form of the '<='
        operator.

        The function yields the comparison of the two arguments
        'expr1' and 'expr2' which both must be of equivalent types,
        either numeric or string; which are arithmetic and
        lexicographically comparisons respectively.

    Macro Parameters:
        expr1 - Left expression, numeric or string.
        expr2 - Right expression, numeric or string.

    Macro Returns:
        The 'below_eq()' primitive yields the value 1 if 'expr1' is less
        then or equals 'expr2', and 0 if the relation is false.

    Limitations:
         Lists can not be compared.

    Macro See Also:
        Operators, >=
 */

/*<<GRIEF>>
    Macro: compare - Comparison.

        int
        compare(expr1, expr2)

    Macro Description:
        The 'compare()' primitive is the functional form of the '<=>'
        operator.

        The function yields the comparison of the two arguments
        'expr1' and 'expr2' which both must be of equivalent types,
        either numeric or string; which are arithmetic and
        lexicographically comparisons respectively.

    Macro Parameters:
        expr1 - Left expression.
        expr2 - Right expression.

    Macro Returns:
        The 'compare()' primitive yields the value -1 if the first
        expression is less then the second, 0 if the equals, and 1 the
        greater than; which are arithmetic and lexicographically
        comparisons respectively.

    Limitations:
        Lists can not be compared.

    Macro See Also:
        Operators, declare
 */

/*  Function:           com_equ
 *      Compulation Operators.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 */
void
do_com_op(int op)
{
#define arg_flags1      margv[1].l_flags
#define arg_int1        margv[1].l_int
#define arg_float1      margv[1].l_float

#define arg_flags2      margv[2].l_flags
#define arg_int2        margv[2].l_int
#define arg_float2      margv[2].l_float

    enum {B_INTEGER, B_FLOAT, B_STRING, B_LIST, B_NONE} type = B_NONE;
    accint_t int1 = 0, int2 = 0;
    accfloat_t float1 = 0, float2 = 0;
    const char *str1 = NULL, *str2 = NULL;
    const LIST *lp = NULL;
    const LISTV *lvp = NULL;
    int str1len = 0, str2len = 0;
    int llen = 0;
    accint_t val = 0;
    char buf[64];

    /*
     *  Basic type co-oercion
     */
    if (MOP_BNOT == op) {                       /* Bitwise NOT (complement) is special */
        if (F_INT == arg_flags1) {
            int1 = arg_int1;
        } else if (F_FLOAT == arg_flags1) {
            int1 = (accint_t) arg_float1;
    // experimental
    //  } else if (F_LIT == arg_flags1 || F_STR == arg_flags1 || F_RSTR == arg_flags1) {
    //      int1 = (accint_t) get_strlen(1);    /* 3.3.0 */
        } else {
            goto com_op_mixed;
        }
        acc_assign_int(~int1);
        return;
    }

    switch (arg_flags1) {
    case F_INT:
        switch (arg_flags2) {
        case F_INT:         /* int, int */
            int1 = arg_int1;
            int2 = arg_int2;
            type = B_INTEGER;                   /* integer */
            break;
        case F_FLOAT:       /* int, float */
            float1 = (accfloat_t) arg_int1;
            float2 = arg_float2;
            type = B_FLOAT;                     /* convert to float */
            break;
        case F_LIT:         /* int, str */
        case F_STR:
        case F_RSTR:
            str2 = get_str(2);
            str2len = get_strlen(2);
            if (MOP_MULTIPLY == op) {           /* string multipler */
                string_mul(str2, str2len, arg_int1);
                return;
            }
            str1 = buf;
            str1len = sprintf(buf, "%" ACCINT_FMT, arg_int1);
            type = B_STRING;                    /* convert string */
            break;
        case F_RLIST:       /* int, list */
        case F_LIST:
            lp = get_list(2);
            llen = get_listlen(2);
            lvp = margv + 1;
            type = B_LIST;                      /* list + type */
            break;
        default:
            goto com_op_mixed;
        }
        break;

    case F_FLOAT:
        switch (arg_flags2) {
        case F_INT:         /* float, int */
            float1 = arg_float1;
            float2 = (accfloat_t) arg_int2;
            type = B_FLOAT;
            break;
        case F_FLOAT:       /* float, float */
            float1 = arg_float1;
            float2 = arg_float2;
            type = B_FLOAT;
            break;
        case F_LIT:         /* float, str */
        case F_STR:
        case F_RSTR:
            str2 = get_str(2);
            str2len = get_strlen(2);
            if (MOP_MULTIPLY == op) {
                string_mul(str2, str2len, (int)arg_float1);
                return;
            }
            str1 = buf;
            str1len = sprintf(buf, "%" ACCFLOAT_FMT, arg_float1);
            type = B_STRING;
            break;
        case F_RLIST:       /* float, list */
        case F_LIST:
            lp = get_list(2);
            llen = get_listlen(2);
            lvp = margv + 1;
            type = B_LIST;                      /* list + float */
            break;
        default:
            goto com_op_mixed;
        }
        break;

    case F_RLIST:
    case F_LIST:
        if (MOP_PLUS != op) /* list, int|float|string|list */
            goto com_op_mixed;
        lp = get_list(1);
        llen = get_listlen(1);
        lvp = margv + 2;
        type = B_LIST;
        break;

    case F_LIT:
    case F_STR:
    case F_RSTR:
        switch (arg_flags2) {
        case F_INT:         /* str, int */
            str1 = get_str(1);
            str1len = get_strlen(1);
            if (MOP_MULTIPLY == op) {
                string_mul(str1, str1len, arg_int2);
                return;
            }
            str2 = buf;
            str2len = sprintf(buf, "%" ACCINT_FMT, arg_int2);
            type = B_STRING;
            break;
        case F_FLOAT:       /* str, float */
            str1 = get_str(1);
            str1len = get_strlen(1);
            if (MOP_MULTIPLY == op) {
                string_mul(str1, str1len, (int)arg_float2);
                return;
            }
            str2 = buf;
            str2len = sprintf(buf, "%" ACCFLOAT_FMT, arg_float2);
            type = B_STRING;
            break;
        case F_LIT:         /* str, str */
        case F_STR:
        case F_RSTR:
            if (MOP_MULTIPLY == op) {
                ewprintf("*: attempt to multiply two strings.");
                return;
            }
            str1 = get_str(1);
            str1len = get_strlen(1);
            str2 = get_str(2);
            str2len = get_strlen(2);
            type = B_STRING;
            break;
        case F_RLIST:       /* str, list */
        case F_LIST:
            lp = get_list(2);
            llen = get_listlen(2);
            lvp = margv + 1;
            type = B_LIST;
            break;
        default:
            goto com_op_mixed;
        }
        break;

    default:
        goto com_op_mixed;
    }

    /*
     *  Execute operation
     */
    switch (op) {
    case MOP_PLUS:          /* (+)  addition */
        switch (type) {
        case B_INTEGER:
            val = int1 + int2;
            break;
        case B_FLOAT:
            acc_assign_float(float1 + float2);
            return;
        case B_LIST: {
                LIST *nlp;
                int newlen;

                nlp = lst_join(lp, llen, lvp, &newlen);
                acc_donate_list(nlp, newlen);
            }
            return;
        case B_STRING:
            acc_assign_str2(str1, str1len, str2, str2len);
            return;
        default:
            goto com_op_mixed;
        }
        break;

    case MOP_MINUS:         /* (-)  subtraction */
        switch (type) {
        case B_INTEGER:
            val = (int1 - int2);
            break;
        case B_FLOAT:
            acc_assign_float(float1 - float2);
            return;
        default:
            goto com_op_mixed;
        }
        break;

    case MOP_MULTIPLY:      /* (*)  multiplication */
        switch (type) {
        case B_INTEGER:
            val = (int1 * int2);
            break;
        case B_FLOAT:
            acc_assign_float(float1 * float2);
            return;
        default:
            goto com_op_mixed;
        }
        break;

    case MOP_DIVIDE:        /* (/)  division */
        switch (type) {
        case B_INTEGER:
            if (0 == int2) {
                ewprintf("Divide by zero.");
            } else {
                val = int1 / int2;
            }
            break;
        case B_FLOAT:
            if (0 == float2) {
                ewprintf("Divide by zero.");
                acc_assign_float(0);
            } else {
                acc_assign_float(float1 / float2);
            }
            return;
        default:
            goto com_op_mixed;
        }
        break;

    case MOP_MODULO:        /* (%)  modulus */
        switch (type) {
        case B_INTEGER:
            if (0 == int2) {
                ewprintf("Modula by zero.");
            } else {
                val = (int1 % int2);
            }
            break;
        default:
            goto com_op_mixed;
        }
        break;

    case MOP_EQ:            /* (==) equals */
        switch (type) {
        case B_INTEGER:
            val = (int1 == int2);
            break;
        case B_FLOAT:
            val = (float1 == float2);
            break;
        case B_STRING:
            val = (strcmp(str1, str2) == 0);
            break;
        default:
            goto com_op_mixed;
        }
        break;

    case MOP_NE:            /* (!=) not-equals */
        switch (type) {
        case B_INTEGER:
            val = (int1 != int2);
            break;
        case B_FLOAT:
            val = (float1 != float2);
            break;
        case B_STRING:
            val = (strcmp(str1, str2) != 0);
            break;
        default:
            goto com_op_mixed;
        }
        break;

    case MOP_LT:            /* (<)  less-than */
        switch (type) {
        case B_INTEGER:
            val = (int1 < int2);
            break;
        case B_FLOAT:
            val = (float1 < float2);
            break;
        case B_STRING:
            val = strcmp(str1, str2) < 0;
            break;
        default:
            goto com_op_mixed;
        }
        break;

    case MOP_LE:            /* (<=) less than or equal */
        switch (type) {
        case B_INTEGER:
            val = (int1 <= int2);
            break;
        case B_FLOAT:
            val = (float1 <= float2);
            break;
        case B_STRING:
            val = (strcmp(str1, str2) <= 0);
            break;
        default:
            goto com_op_mixed;
        }
        break;

    case MOP_GT:            /* (>)  greater than */
        switch (type) {
        case B_INTEGER:
            val = (int1 > int2);
            break;
        case B_FLOAT:
            val = (float1 > float2);
            break;
        case B_STRING:
            val = (strcmp(str1, str2) > 0);
            break;
        default:
            goto com_op_mixed;
        }
        break;

    case MOP_GE:
        switch (type) {     /* (>=) greater than or equal */
        case B_INTEGER:
            val = (int1 >= int2);
            break;
        case B_FLOAT:
            val = (float1 >= float2);
            break;
        case B_STRING:
            val = (strcmp(str1, str2) >= 0);
            break;
        default:
            goto com_op_mixed;
        }
        break;

    case MOP_ABOVE:         /* above(a.b) */
        switch (type) {
        case B_INTEGER:
            val = (int1 > int2);
            break;
        case B_FLOAT:
            val = (float1 > float2);
            break;
        case B_STRING:
            val = (strcmp(str1, str2) > 0);
            break;
        default:
            goto com_op_mixed;
        }
        break;

    case MOP_ABOVE_EQ:      /* above_eq(a,b) */
        switch (type) {
        case B_INTEGER:
            val = (int1 >= int2);
            break;
        case B_FLOAT:
            val = (float1 >= float2);
            break;
        case B_STRING:
            val = (strcmp(str1, str2) >= 0);
            break;
        default:
            goto com_op_mixed;
        }
        break;

    case MOP_BELOW:         /* below(a,b) */
        switch (type) {
        case B_INTEGER:
            val = (int1 < int2);
            break;
        case B_FLOAT:
            val = (float1 < float2);
            break;
        case B_STRING:
            val = (strcmp(str1, str2) < 0);
            break;
        default:
            goto com_op_mixed;
        }
        break;

    case MOP_BELOW_EQ:      /* below_eq(a,b) */
        switch (type) {
        case B_INTEGER:
            val = (int1 <= int2);
            break;
        case B_FLOAT:
            val = (float1 <= float2);
            break;
        case B_STRING:
            val = (strcmp(str1, str2) <= 0);
            break;
        default:
            goto com_op_mixed;
        }
        break;

    case MOP_BAND:          /* (&)  bit-wise and */
        switch (type) {
        case B_INTEGER:
            val = (int1 & int2);
            break;
        default:
            goto com_op_mixed;
        }
        break;

    case MOP_BOR:           /* (|)  bit-wise or */
        switch (type) {
        case B_INTEGER:
            val = (int1 | int2);
            break;
        default:
            goto com_op_mixed;
        }
        break;

    case MOP_BXOR:          /* (^)  bit-wise xor */
        switch (type) {
        case B_INTEGER:
            val = (int1 ^ int2);
            break;
        default:
            goto com_op_mixed;
        }
        break;

    case MOP_LSHIFT:        /* (<<) left shift */
        switch (type) {
        case B_INTEGER:
            val = (int1 << int2);
            break;
        default:
            goto com_op_mixed;
        }
        break;

    case MOP_RSHIFT:        /* (>>) right shift */
        switch (type) {
        case B_INTEGER:
            val = (int1 >> int2);
            break;
        default:
            goto com_op_mixed;
        }
        break;

    case MOP_CMP:           /* (<=>) comparison operator, returns -1, 0 or 1 */
        switch (type) {
        case B_INTEGER:
            if (int1 == int2) {
                val = 0;
            } else if (int1 < int2) {
                val = -1;
            } else {
                val = 1;
            }
            break;
        case B_FLOAT:
            if (float1 == float2) {
                val = 0;
            } else if (float1 < float2) {
                val = -1;
            } else {
                val = 1;
            }
            break;
        case B_STRING:
            val = strcmp(str1, str2);
            break;
        default:
            goto com_op_mixed;
        }
        break;

    default:
        panic("com_op: bad op (%d)", op);
        break;
    }
    acc_assign_int(val);
    return;

com_op_mixed:
    ewprintf("%s: invalid parameters.", x_command_name);
    acc_assign_int(0);

#undef  arg_flags1
#undef  arg_int1
#undef  arg_float1
#undef  arg_flags2
#undef  arg_int2
#undef  arg_float2
}


/*  Function:           do_minusminus
 *      --expr operator
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Operator: -- - Prefix Decrement.

        --expr

    Macro Description:
        The '--' operator decreases the operand 'expr' by 1, with the
        result of the operation returned.

    Macro Parameters:
        expr - An expression.

    Macro Returns:
        The accumulator shall be set to the result of the assignment.

    Macro See Also:
        Operators, declare
 */
void
do_minusminus(void)
{
    SYMBOL *sym = get_symbol(1);

    if (F_INT == sym->s_type) {
        acc_assign_int(--sym->s_int);
    } else if (F_FLOAT == sym->s_type) {
        acc_assign_float(--sym->s_float);
    } else {
        ewprintf("--: non numeric.");
    }
}


/*  Function:           do_plusplus
 *      ++expr operator
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Operator: ++ - Prefix Increment.

        ++expr

    Macro Description:
        The '++' operator increases the operand 'expr' by 1, with the
        result of the operation returned.

    Macro Parameters:
        expr - An expression.

    Macro Returns:
        The accumulator shall be set to the result of the assignment.

    Macro See Also:
        Operators, declare
 */
void
do_plusplus(void)
{
    SYMBOL *sym = get_symbol(1);

    if (F_INT == sym->s_type) {
        acc_assign_int(++sym->s_int);
    } else if (F_FLOAT == sym->s_type) {
        acc_assign_float(++sym->s_float);
    } else {
        ewprintf("++: non numeric.");
    }
}


/*  Function:           do_post_minusminus
 *      expr-- operator
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Operator: post-- - Postfix Decrement.

        expr--

    Macro Description:
        The 'post--' operator decrements the operand 'expr' by 1,
        with the original value prior to the operation returned.

        In other words, the original value of the operand is used in
        the expression, and then it is decremented.

    Macro Parameters:
        expr - An expression.

    Macro Returns:
        The accumulator shall be set to orginal value of 'expr' prior
        to being decremented.

    Macro See Also:
        Operators, declare
 */
void
do_post_minusminus(void)
{
    SYMBOL *sym = get_symbol(1);

    if (F_INT == sym->s_type) {
        acc_assign_int(sym->s_int--);
    } else if (F_FLOAT == sym->s_type) {
        acc_assign_float(sym->s_float--);
    } else {
        ewprintf("post++: non numeric.");
    }
}


/*  Function:           post_plusplus
 *      ++expr operator
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Operator: post++ - Postfix Increment.

        expr++

    Macro Description:
        The 'post++' operator increments the operand 'expr' by 1,
        with the original value prior to the operation returned.

        In other words, the original value of the operand is used in
        the expression, and then it is incremented.

    Macro Parameters:
        expr - An expression.

    Macro Returns:
        The accumulator shall be set to orginal value of 'expr' prior
        to being incremented.

    Macro See Also:
        Operators, declare
 */
void
do_post_plusplus(void)
{
    SYMBOL *sym = get_symbol(1);

    if (F_INT == sym->s_type) {
        acc_assign_int(sym->s_int++);
    } else if (F_FLOAT == sym->s_type) {
        acc_assign_float(sym->s_float++);
    } else {
        ewprintf("post--: non numeric.");
    }
}


/*  Function:           do_lnot
 *      Logical NOT (! expr) operator.
 *
 *  Returns:
 *      nothing
 */
void
do_lnot(void)
{
    if (get_xstr(1)) {                          /* 3.2.0 */
        acc_assign_int(!get_strlen(1));
    } else {
        acc_assign_int(!get_integer(1));
    }
}


/*<<GRIEF>>
    Operator: && - Logical AND operator.

        expr1 && expr2

    Macro Description:
        The '&&' operator performs a logic AND between the two
        expressions 'expr1' and 'expr2'.

        Each of the operands must have scalar type. If both of the
        operands are not equal to zero, then the result is 1.
        Otherwise, the result is zero. The result type is int.

    Short Circuit Evaluation:

        Logical operators are executed using 'short-circuit'
        semantics whereby the second argument is only executed or
        evaluated if the first argument does not suffice to determine
        the value of the expression:

        o Logical ADD - If the first operand is zero, then the second
            operand is not evaluated. Any side effects that would
            have happened if the second operand had been executed do
            not happen. Any function calls encountered in the second
            operand do not take place.

        o Logical OR - If the first operand is not zero, then the second
            operand is not evaluated. Any side effects that would
            have happened if the second operand had been executed do
            not happen. Any function calls encountered in the second
            operand shall not take place.

    Macro See Also:
        Logical Operators
 */
void
do_andand(void)
{
    if (get_integer(1)) {                       /* left */
        LISTV result;

        if (F_INT == eval(get_list(2), &result)) {
            acc_assign_int(result.l_int);       /* right */
            return;
        }
    }
    acc_assign_int(0);
}


/*<<GRIEF>>
    Operator: || - Logical OR operator.

        expr1 || expr2

    Macro Description:
        The '||' operator performs a logic OR between the two
        expressions 'expr1' and 'expr2'.

        Each of the operands must have scalar type. If one or both of
        the operands is not equal to zero, then the result is 1.
        Otherwise, the result is zero (both operands are zero). The
        result type is int.

    Short Circuit Evaluation:

        Logical operators are executed using 'short-circuit'
        semantics whereby the second argument is only executed or
        evaluated if the first argument does not suffice to determine
        the value of the expression:

        o Logical ADD - If the first operand is zero, then the second
            operand is not evaluated. Any side effects that would
            have happened if the second operand had been executed do
            not happen. Any function calls encountered in the second
            operand do not take place.

        o Logical OR - If the first operand is not zero, then the second
            operand is not evaluated. Any side effects that would
            have happened if the second operand had been executed do
            not happen. Any function calls encountered in the second
            operand shall not take place.

    Macro See Also:
        Logical Operators
 */
void
do_oror(void)
{
    if (0 == get_integer(1)) {                  /* left */
        LISTV result;

        if (F_INT == eval(get_list(2), &result)) {
            acc_assign_int(result.l_int);       /* right */
        }
        return;
    }
    acc_assign_int(1);
}


/*<<GRIEF>>
    Macro: abs - Absolute value.

        int
        abs(int val)

    Macro Description:
        The 'abs()' primitive computes the absolute value of an integer.

    Macro Returns:
        Returns the absolute value.

    Macro See Also:
        fabs
 */
void
do_abs(void)
{
    if (isa_integer(1)) {
        acc_assign_int(abs(get_integer(1)));

    } else {
#ifdef NO_FLOAT_MATH
        acc_assign_float(0.0);
#else
        acc_assign_float(fabs(get_accfloat(1)));
#endif
    }
}
/*end*/
