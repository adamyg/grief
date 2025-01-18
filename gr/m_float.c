#include <edidentifier.h>
__CIDENT_RCSID(gr_m_float_c,"$Id: m_float.c,v 1.30 2025/01/18 16:25:05 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: m_float.c,v 1.30 2025/01/18 16:25:05 cvsuser Exp $
 * Floating point primitives.
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
#include <math.h>
#include <float.h>

#if defined(_MSC_VER) && defined(_WIN32)
//#define  WINDOWS_MEAN_AND_LEAN
//#include <windows.h>
#endif

#include "m_float.h"                            /* public interface */

#include "accum.h"                              /* acc_...() */
#include "builtin.h"                            /* margv */
#include "echo.h"                               /* ewprintf() */
#include "eval.h"                               /* get_symbol() */
#include "symbol.h"                             /* sym_...() */


#if defined(NO_FLOAT_MATH)
/*
 *  disable float functionality/
 *      can be used to attempt smaller builds on machines FPP hardware.
 */
#define isnan(x)            0
#define isfinite(x)         0
#define isinf(x)            0
#define acos(x)             0.0
#define asin(x)             0.0
#define atan(x)             0.0
#define atan2(x, y)         0.0
#define ceil(x)             0.0
#define cos(x)              0.0
#define cosh(x)             0.0
#define exp(x)              0.0
#define fabs(x)             0.0
#define floor(x)            0.0
#define fmod(x, y)          0.0
#define frexp(x, y)         0.0
#define ldexp(x, y)         0.0
#define log(x)              0.0
#define log10(x)            0.0
#define modf(x, y)          0.0
#define pow(x, y)           0.0
#define sin(x)              0.0
#define sinh(x)             0.0
#define sqrt(x)             0.0
#define tan(x)              0.0
#define tanh(x)             0.0

#else
#if defined(HAVE_FENV_H)
#if !defined(_WIN32) && !defined(WIN32) && !defined(_GNU_SOURCE)
#define _GNU_SOURCE         /*extensions*/
#define HAVE_FECLEAREXCEPT  /*c99*/
#endif
#include <fenv.h>
#include <errno.h>
#endif  /*HAVE_FENV_H*/

#if defined(HAVE_FENV_H) && defined(HAVE_FECLEAREXCEPT)
#define ___XEXEC(__y)       __y
#define __FEXEC(__x) \
        accfloat_t __r; \
        int ret; \
        errno = 0; \
        feclearexcept(FE_ALL_EXCEPT); \
        __r = (accfloat_t) ___XEXEC(__x); \
        if (0 == (ret = errno)) { \
            system_errno(errno); \
        } else if (fetestexcept(FE_INVALID | FE_DIVBYZERO | FE_OVERFLOW | FE_UNDERFLOW)) { \
            ret = ERANGE; \
        } \
        acc_assign_float(ret ? 0 : __r); \
        system_errno(ret);
#define __IEXEC(__x) \
        accint_t __r; \
        int ret; \
        errno = 0; \
        feclearexcept(FE_ALL_EXCEPT); \
        __r = ___XEXEC(__x); \
        if (0 == (ret = errno)) { \
            system_errno(errno); \
        } else if (fetestexcept(FE_INVALID | FE_DIVBYZERO | FE_OVERFLOW | FE_UNDERFLOW)) { \
            ret = ERANGE; \
        } \
        acc_assign_int(ret ? 0 : __r); \
        system_errno(ret);
#endif
#if defined(_MSC_VER)
#define ___XEXEC(__y)       __y
#define __FEXEC(__x) \
        __try { \
            double __r = ___XEXEC(__x); \
            acc_assign_float(__r); \
        } __except (EXCEPTION_EXECUTE_HANDLER) { \
            system_errno(errno); \
            acc_assign_int(0); \
        }
#define __IEXEC(__x) \
        __try { \
            int __r = ___XEXEC(__x); \
            acc_assign_int(__r); \
        } __except (EXCEPTION_EXECUTE_HANDLER) { \
            system_errno(errno); \
            acc_assign_int(0); \
        }
#endif
#endif  /*NO_FLOAT_MATH*/

#if !defined(__FEXEC)
#define ___XEXEC(__y)       __y
#define __FEXEC(__x) \
        acc_assign_float((accfloat_t)___XEXEC(__x))
#define __IEXEC(__x) \
        acc_assign_int((accint_t)___XEXEC(__x))
#endif

#if defined(__WATCOMC__)
unsigned _WCNEAR _chipbug = 0;                  /* suppress Pentuin div checking */
#endif

#define arg_float1          margv[1].l_float
#define arg_float2          margv[2].l_float
#define arg_float3          margv[3].l_float
#define arg_float4          margv[4].l_float


/*
 *  Both types of NaNs are represented by the largest biased exponent
 *  allowed by the format (single- or double-precision) and a
 *  mantissa that is non-zero.
 *
 *   o The bit pattern of the mantissa for a signaling NaN has the
 *     most significant digit set to zero and at least one of the
 *     remaining digits set to one.
 *
 *   o The bit pattern of the mantissa for a quiet NaN has the most
 *     significant digit set to one.
 *
 *  For single-precision values:
 *
 *   o Positive infinity is represented by the bit pattern 7F800000.
 *   o Negative infinity is represented by the bit pattern FF800000.
 *   o A signaling NaN (NANS) is represented by any bit pattern
 *      between 7F800001 and 7FBFFFFF or between FF800001 and FFBFFFFF.
 *   o A quiet NaN (NANQ) is represented by any bit pattern
 *      between 7FC00000 and 7FFFFFFF or between FFC00000 and FFFFFFFF.
 *
 *  For double-precision values:
 *
 *   o Positive infinity is represented by the bit pattern 7FF0000000000000.
 *   o Negative infinity is represented by the bit pattern FFF0000000000000.
 *   o A signaling NaN is represented by any bit pattern
 *      between 7FF0000000000001 and 7FF7FFFFFFFFFFFF or
 *      between FFF0000000000001 and FFF7FFFFFFFFFFFF.
 *   o A quiet NaN is represented by any bit pattern
 *      between 7FF8000000000000 and 7FFFFFFFFFFFFFFF or
 *      between FFF8000000000000 and FFFFFFFFFFFFFFFF.
 */

#if defined(HAVE_MATHERR)
int                                             /* SYSV math error handling */
matherr(struct exception *exc)
{
    ewprintf("matherr %s exception in %s(%f [,%f]) = %f",
            (exc->type == DOMAIN)    ?    "DOMAIN" :
            (exc->type == OVERFLOW)  ?  "OVERFLOW" :
            (exc->type == UNDERFLOW) ? "UNDERFLOW" :
            (exc->type == SING)      ?      "SING" :
            (exc->type == TLOSS)     ?     "TLOSS" :
            (exc->type == PLOSS)     ?     "PLOSS" : "???",
                exc->name, exc->arg1, exc->arg2, exc->retval);
    return 1;
}
#endif  /*HAVE_MATHERR*/


/*  Function:               do_isnan
 *      isnan primitive
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: isnan - Test for a NaN

        int
        isnan(float val)

    Macro Description:
        The 'isnan()' primitive tests the specified floating-point
        value 'val', returning a nonzero value if 'val' is a not a
        number (NaN).

        A *NaN* is generated when the result of a floating-point
        operation cannot be represented in Institute of Electrical
        and Electronics Engineers (IEEE) format.

        On systems not supporting NaN values, isnan() always
        returns 0.

    Macro Returns:
        Non-zero if *NaN*, otherwise 0.

    Macro Portability:
        A Grief extension.
*/
void
do_isnan(void)                  /* int (float val) */
{
#if defined(HAVE_ISNAN) || defined(isnan)
    acc_assign_int(isnan(arg_float1));

#elif defined(HAVE__ISNAN)
    acc_assign_int(_isnan(arg_float1));

#else
    acc_assign_int(0);
#endif
}


/*  Function:               do_isinf
 *      isinf primitive
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: isinf - Test for infinity

        int
        isinf(float val)

    Macro Description:
        The 'isinf()' primitive shall determine whether its argument
        has a finite value (zero, subnormal, or normal, and not
        infinite or NaN).

    Macro Returns:
        The 'isinf()' primitive shall return a non-zero value if and
        only if its argument has an infinite value.

    Macro Portability:
        A Grief extension.

*/
void
do_isinf(void)                  /* int (float val) */
{
#if defined(isinf) || defined(HAVE_ISINF)
    acc_assign_int(isinf(arg_float1));

#elif defined(HAVE__FINITE) && defined(HAVE__ISNAN) /* MSC_VER */
    acc_assign_int(!_finite(arg_float1) && !_isnan(arg_float1));

#elif defined(HAVE_ISNAN) || defined(isnan)
    acc_assign_int(!isnan(arg_float1));

#elif defined(HAVE__ISNAN)
    acc_assign_int(!_isnan(arg_float1));

#else
    acc_assign_int(0);

#endif
}


/*  Function:               do_isclose
 *      isclose primitive.
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: isclose - Test for floating point equality

        int
        isclose(float v1, float v2, float ~rel_tot, float ~abs_tol)

    Macro Description:
        The 'isclose()' primitive determines whether two floating point numbers are close in value.

    Macro Parameters:
        v1 - First string.

        v2 - Second value to compare against.

        rel_tol - Optional non-negative value, used for relative tolerance.
                  Default value is 1e-09.

        abs_tol - Optional non-negative value, used for the minimum absolute tolerance,
                  for values near 0. Default value is 0.0.

    Macro Returns:
        The 'isclose()' primitive returns a non-zero value if v1 is close in value to v2,
        otherwise zero or -1 on error.

    Macro Portability:
        A Grief extension.

        Result on error are system dependent; where supported 'errno' shall be set to a
        non-zero manifest constant describing the error.
*/

static int
isclose(const double v1, const double v2, double rel_tol, double abs_tol)
{
    double delta = fabs(v2 - v1);
    return (((delta <= fabs(rel_tol * v2)) || (delta <= fabs(rel_tol * v1))) || (delta <= abs_tol));
}

void
do_isclose(void)                /* int (float v1, float v2, float rel_tol = 1e09, float abs_tol = 0.0) */
{
    double v1 = arg_float1, v2 = arg_float2;
    double rel_tol = 1e-09, abs_tol = 0.0;

    if (isa_float(3)) rel_tol = arg_float3;
    if (isa_float(4)) abs_tol = arg_float4;

    if (rel_tol < 0.0 || abs_tol < 0.0) {       /* check parameters */
        ewprintf("isclose: tolerances must be non-negative.");
        system_errno(EINVAL);
        acc_assign_int(-1);

    } else if (v1 == v2) {                      /* simple case */
        acc_assign_int(1);

#if defined(isinf) || defined(HAVE_ISINF)
    } else if (isinf(v1) || isinf(v2)) {        /* infinite never match */
        acc_assign_int(0);
#elif defined(HAVE__ISINF)
    } else if (_isinf(v1) || _isinf(v2)) {      /* infinite never match */
        acc_assign_int(0);
#endif

    } else {
        /*
         *  return (fabs(v1 - v2) < rel_tol)    // alternative method.
         */
        __IEXEC(isclose(v1, v2, rel_tol, abs_tol));
    }
}


/*  Function:               do_isfinite
 *      isfinite primitive.
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: isfinite - Test for finite value.

        int
        isfinite(float val)

    Macro Description:
        The 'isfinite()' primitive shall determine whether its argument has a finite
        value (zero, subnormal, or normal, and not infinite or NaN).

    Macro Returns:
        The 'isfinite()' primitive shall return a non-zero value if and
        only if its argument has a finite value.

    Macro Portability:
        A Grief extension.
*/
void
do_isfinite(void)               /* int (float val) */
{
#if defined(HAVE_ISFINITE) || defined(isfinite)
    acc_assign_int(isfinite(arg_float1));

#elif defined(HAVE__ISFINITE)
    acc_assign_int(_isfinite(arg_float1))

#elif defined(HAVE__FINITE)                     /* MSC_VER */
    acc_assign_int(_finite(arg_float1));

#else
    acc_assign_int(0);
#endif
}


/*  Function:               do_acos
 *      acos primitive
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: acos - Arc cosine

        float
        acos(float x)

    Macro Description:
        The 'acos()' primitive shall compute the principal value of the arc cosine of the argument 'x'.
        The value of 'x' should be in the range [-1,1].

    Macro Returns:
        The 'acos()' primitive on successful completion shall return
        the arc cosine of 'xl', in the range [0,pi] radians.

    Macro Portability:
        Result on error are system dependent.

    Macro See Also:
        cos, isnan
*/
void
do_acos(void)                   /* float (float val) */
{
#if defined(DO_LONG_DOUBLE)
    __FEXEC(acosl(arg_float1));
#else
    __FEXEC(acos(arg_float1));
#endif
}


/*  Function:               do_asin
 *      asin primitive
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: asin - Arc sine

        float
        asin(float x)

    Macro Description:
        The 'asin()' primitive shall compute the principal value of
        the arc sine of the argument 'x'. The value of 'x' should
        be in the range [-1,1].

    Macro Returns:
        Upon successful completion shall return the arc sine of 'x',
        in the range [-pi/2,pi/2] radians.

    Macro Portability:
        Result on error are system dependent; where supported 'errno'
        shall be set to a non-zero manifest constant describing the
        error.

    Macro See Also:
        isnan, sin
*/
void
do_asin(void)                   /* float (float x) */
{
#if defined(DO_LONG_DOUBLE)
    __FEXEC(asinl(arg_float1));
#else
    __FEXEC(asin(arg_float1));
#endif
}


/*  Function:               do_atan
 *      atan primitive
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: atan - Arctangent

        float
        atan(float x)

    Macro Description:
        The 'atan()' primitive calculates the arctangent of 'x'.

    Macro Returns:
        Returns a value in the range -pi/2 to pi/2 radians.

    Macro Portability:
        Result on error are system dependent; where supported
        'errno' shall be set to a non-zero manifest constant
        describing the error.

    Macro See Also:
        atan2
*/
void
do_atan(void)                   /* float (float val) */
{
#if defined(DO_LONG_DOUBLE)
    __FEXEC(atanl(arg_float1));
#else
    __FEXEC(atan(arg_float1));
#endif
}


/*  Function:               do_atan2
 *      atan2 primitive
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: atan2 - Arctangent division

        float
        atan2(float y, float x)

    Macro Description:
        The 'atan2()' primitive calculate the arctangent (inverse
        tangent) of the operand division 'y/x'.

    Macro Returns:
        The 'atan2()' primitive returns a value in the range -pi to pi
        radians. If both arguments of atan2() are zero, the
        function sets errno to EDOM, and returns 0. If the correct
        value would cause underflow, zero is returned and the value
        ERANGE is stored in errno.

    Macro Portability:
        Result on error are system dependent; where supported 'errno'
        shall be set to a non-zero manifest constant describing the
        error.

    Macro See Also:
        atan
*/
void
do_atan2(void)                  /* float (float y, float x) */
{
#if defined(DO_LONG_DOUBLE)
    __FEXEC(atan2l(arg_float1, arg_float2));
#else
    __FEXEC(atan2(arg_float1, arg_float2));
#endif
}


/*  Function:               do_ceil
 *      ceil primitive
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: ceil - Round up to integral value.

        float
        ceil(float x)

    Macro Description:
        The 'ceil()' primitive computes the smallest integer that is
        greater than or equal to 'x'.

    Macro Returns:
        Returns the calculated value as a double, float, or long double
        value.

        If there is an overflow, the function sets errno to ERANGE and
        returns HUGE_VAL.

    Macro Portability:
        Result on error are system dependent; where supported 'errno'
        shall be set to a non-zero manifest constant describing the
        error.

    Macro See Also:
        floor, fabs
*/
void
do_ceil(void)                   /* float (float x) */
{
#if defined(DO_LONG_DOUBLE)
    __FEXEC(ceill(arg_float1));
#else
    __FEXEC(ceil(arg_float1));
#endif
}


/*  Function:               do_cos
 *      cos primitive
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: cos - Cosine

        float
        cos(float x)

    Macro Description:
        The 'cos()' primitive calculates the cosine of 'x'. The value
        'x' is expressed in radians.

    Macro Returns:
        Returns the calculated value.

        If 'x' is outside prescribed limits, the value is not
        calculated. Instead, the function returns 0 and sets the errno
        to *ERANGE*.

        If the correct value would cause an underflow, zero is returned
        and the value *ERANGE* is stored in 'errno'.

    Macro Portability:
        Result on error are system dependent; where supported 'errno'
        shall be set to a non-zero manifest constant describing the
        error.

    Macro See Also:
        sin, atan, atan2
*/
void
do_cos(void)                    /* float (float val) */
{
#if defined(DO_LONG_DOUBLE)
    __FEXEC(cosl(arg_float1));
#else
    __FEXEC(cos(arg_float1));
#endif
}


/*  Function:               do_cosh
 *      cosh primitive
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: cosh - Hyperbolic cosine.

        float
        cosh(float x)

    Macro Description:
        The 'cosh()' primitive calculates the hyperbolic cosine of
        'x'. The value 'x' is expressed in radians.

    Macro Returns:
        Returns the calculated value.

        If the result overflows, the function returns *+HUGE_VAL* and
        sets 'errno' to *ERANGE*.

    Macro Portability:
        Result on error are system dependent; where supported 'errno'
        shall be set to a non-zero manifest constant describing the
        error.

    Macro See Also:
        cos, sin
*/
void
do_cosh(void)                   /* float (float val) */
{
#if defined(DO_LONG_DOUBLE)
    __FEXEC(coshl(arg_float1));
#else
    __FEXEC(cosh(arg_float1));
#endif
}


/*  Function:               do_exp
 *      exp primitive
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: exp - Exponential function

        float
        exp(float x)

    Macro Description:
        The 'exp()' primitive calculates the exponent of 'x', defined
        as 'e**x', where 'e' equals '2.17128128....'

    Macro Returns:
        If successful, the function returns the calculated value.

        If an overflow occurs, the function returns *HUGE_VAL*. If an
        underflow occurs, it returns '0'. Both overflow and underflow
        set 'errno' to *ERANGE*.

    Macro Portability:
        Result on error are system dependent; where supported 'errno'
        shall be set to a non-zero manifest constant describing the
        error.

    Macro See Also:
        frexp, log, log10
*/
void
do_exp(void)                    /* float (float val) */
{
#if defined(DO_LONG_DOUBLE)
    __FEXEC(expl(arg_float1));
#else
    __FEXEC(exp(arg_float1));
#endif
}


/*  Function:               do_fabs
 *      fabs primitive
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: fabs - Floating-point absolute value.

        float
        fabs(float x)

    Macro Description:
        The 'fabs()' primitive calculates the absolute value of a
        floating-point argument 'x'.

    Macro Returns:
        Returns the absolute value of the float input.

    Macro Portability:
        n/a

    Macro See Also:
        ceil, floor, abs
*/
void
do_fabs(void)                   /* float (float val) */
{
#if defined(DO_LONG_DOUBLE)
    __FEXEC(fabsl(arg_float1));
#else
    __FEXEC(fabs(arg_float1));
#endif
}


/*  Function:               do_floor
 *      floor primitive
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: floor - Round down to integral value.

        float
        floor(float x)

    Macro Description:
        The 'floor()' primitive calculates the largest integer that
        is less than or equal to 'x'.

    Macro Returns:
        Returns the calculated integral value expressed as a double,
        float, or long double value. The result cannot have a range
        error.

    Macro Portability:
        Result on error are system dependent; where supported 'errno'
        shall be set to a non-zero manifest constant describing the
        error.

    Macro See Also:
        ceil, fmod, modf, fabs

*/
void
do_floor(void)                  /* float (float val) */
{
#if defined(DO_LONG_DOUBLE)
    __FEXEC(floorl(arg_float1));
#else
    __FEXEC(floor(arg_float1));
#endif
}


/*  Function:               do_fmod
 *      fmod primitive
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: fmod - Floating-point remainder.

        float
        fmod(float x, float y)

    Macro Description:
        The 'fmod()' primitive Calculates the floating-point
        remainder of 'x/y'. The absolute value of the result is
        always less than the absolute value of 'y'. The result will
        have the same sign as 'x'.

    Macro Returns:
        If successful, the function returns the floating-point remainder of x/y.

    Macro Portability:
        Result on error are system dependent; where supported 'errno'
        shall be set to a non-zero manifest constant describing the
        error.

        Dependent on the implementation if 'y' is 0 one of two may occur,

            o fmod returns 0;
            o the function sets errno to *EDOM* and returns NaN.

        No other errors will occur.

    Macro See Also:
        ceil, floor, modf
*/
void
do_fmod(void)                   /* float (float x, float y) */
{
#if defined(DO_LONG_DOUBLE)
    __FEXEC(fmodl(arg_float1, arg_float2));
#else
    __FEXEC(fmod(arg_float1, arg_float2));
#endif
}


/*  Function:               do_frexp
 *      frexp primitive
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: frexp - Extract mantissa and exponent

        float
        frexp(float num, int &exp)

    Macro Description:
        The 'frexp()' primitive breaks a floating-point number 'num'
        into a normalised fraction and an integral power of 2. It
        stores the integer exponent in the int object 'exp'.

        An application wishing to check for error situations should
        set errno to 0 before calling frexp(). If errno is non-zero
        on return, or the return value is NaN, an error has occurred.

    Macro Returns:
        The 'frexp()' primitive returns the value 'x', such that 'x' is
        a double with magnitude in the interval [0.5, 1) or 0, and
        num equals 'x' times 2 raised to the power 'exp'.

        If 'num' is 0, both parts of the result are 0.

    Macro Portability:
        Result on error are system dependent; where supported 'errno'
        shall be set to a non-zero manifest constant describing the
        error.

    Macro See Also:
        isnan, ldexp, modf
*/
void
do_frexp(void)                  /* float (float val, int &exp) */
{
    int exp_val = 0;

#if defined(DO_LONG_DOUBLE)
    __FEXEC(frexpl(arg_float1, &exp_val));
#else
    __FEXEC(frexp(arg_float1, &exp_val));
    sym_assign_int(get_symbol(2), (accint_t) exp_val);
#endif
}


/*  Function:               do_ldexp
 *      ldexp primitive
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: ldexp - Multiply by a power of two.

        float
        ldexp(float val, int exp)

    Macro Description:
        The 'ldexp()' primitive calculates the value of 'x*(2^exp)'.

        This is 'x' multiplied by 2 to the power of 'exp'.

    Macro Returns:
        Returns the calculated value.

        Otherwise, if the correct calculated value is outside the
        range of representable values, *-/+HUGE_VAL* is returned,
        according to the sign of the value. The value *ERANGE* is
        stored in 'errno' to indicate that the result was out of range.

    Macro Portability:
        Result on error are system dependent; where supported 'errno'
        shall be set to a non-zero manifest constant describing the
        error.

    Macro See Also:
        modf
*/
void
do_ldexp(void)                  /* float (float val, int exp) */
{
#if defined(DO_LONG_DOUBLE)
    __FEXEC(ldexpl(arg_float1, get_xinteger(2, 1)));
#else
    __FEXEC(ldexp(arg_float1, get_xinteger(2, 1)));
#endif
}


/*  Function:               do_log
 *      log primitive
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: log - Natural logarithm.

        float
        log(float x)

    Macro Description:
        The 'log()' primitive calculates the natural logarithm (base e)
        of 'x', for 'x' greater than 0.

    Macro Returns:
        Returns the computed value.

        If 'x' is negative, the function sets errno to *EDOM* and
        returns *-HUGE_VAL*. If 'x' is 0.0, the function returns
        *-HUGE_VAL* and sets 'errno' to *ERANGE*.

        If the correct value would cause an underflow, 0 is returned
        and the value *ERANGE* is stored in 'errno'.

    Macro Portability:
        Result on error are system dependent; where supported 'errno'
        shall be set to a non-zero manifest constant describing the
        error.

    Macro See Also:
        log10, exp
*/
void
do_log(void)                    /* float (float val) */
{
#if defined(DO_LONG_DOUBLE)
    __FEXEC(logl(arg_float1));
#else
    __FEXEC(log(arg_float1));
#endif
}


/*  Function:               do_log10
 *      log10 primitive
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: log10 - Base 10 logarithm function.

        float
        log10(float val)

    Macro Description:
        The 'log10()' primitive calculates the base 10 logarithm of
        the positive value of 'x'.

    Macro Returns:
        Returns the computed value.

        If 'x' is negative, the function sets errno to *EDOM* and
        returns *-HUGE_VAL*. If 'x' is 0.0, the function returns
        *-HUGE_VAL* and sets 'errno' to *ERANGE*.

        If the correct value would cause an underflow, 0 is returned
        and the value *ERANGE* is stored in 'errno'.

    Macro Portability:
        Result on error are system dependent; where supported 'errno'
        shall be set to a non-zero manifest constant describing the
        error.

    Macro See Also:
        log, pow
*/
void
do_log10(void)                  /* float (float val) */
{
#if defined(DO_LONG_DOUBLE)
    __FEXEC(log10l(arg_float1));
#else
    __FEXEC(log10(arg_float1));
#endif
}


/*  Function:               do_modf
 *      modf primitive
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: modf - Decompose a floating-point number.

        float
        modf(float num, float &mod)

    Macro Description:
        The 'modf()' primitive breaks the argument 'num' into integral
        and fractional parts, each of which has the same sign as the
        argument. It stores the integral part as a double in the
        object 'mod'.

        An application wishing to check for error situations should
        set errno to 0 before calling modf(). If errno is non-zero on
        return, or the return value is NaN, an error has occurred.

    Macro Returns:
        Upon successful completion, modf() returns the signed
        fractional part of 'num'.

        If 'num' is NaN, NaN is returned, errno may be set to *EDOM*.

        If the correct value would cause underflow, 0 is returned and
        errno may be set to *ERANGE*.

    Macro Portability:
        Result on error are system dependent; where supported 'errno'
        shall be set to a non-zero manifest constant describing the
        error.

    Macro See Also:
        frexp, isnan, ldexp
*/
void
do_modf(void)                   /* float (float val, float &mod) */
{
    double iptr = 0.0;

#if defined(DO_LONG_DOUBLE)
    __FEXEC(modfl(arg_float1, &iptr));
#else
    __FEXEC(modf(arg_float1, &iptr));
#endif
    sym_assign_float(get_symbol(2), (accfloat_t)iptr);
}


/*  Function:               do_pow
 *      pow primitive
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: pow - Raise to power.

        float
        pow(float x, float y)

    Macro Description:
        The 'pow()' primitive returns 'x' raised to the power of 'y'.

    Macro Returns:
        Returns the computed value.

        If 'y' is 0, the function returns '1'.

        If 'x' is negative and y is non-integral, the function sets
        'errno' to *EDOM* and returns *-HUGE_VAL*. If the correct
        value is outside the range of representable values,
        *+HUGE_VAL* is returned according to the sign of the value,
        and the value of *ERANGE* is stored in 'errno'.

    Macro Portability:
        Result on error are system dependent; where supported 'errno'
        shall be set to a non-zero manifest constant describing the
        error.

    Macro See Also:
        exp, log
*/
void
do_pow(void)                    /* float (float x, float y) */
{
#if defined(DO_LONG_DOUBLE)
    __FEXEC(pow(arg_float1, arg_float2));
#else
    __FEXEC(pow(arg_float1, arg_float2));
#endif
}


/*  Function:               do_sin
 *      sin primitive
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: sin - Sine function.

        float
        sin(float val)

    Macro Description:
        The 'sin()' primitive computes the sine of the argument 'val,
        measured in radians.

    Macro Returns:
        Upon successful completion, shall return the sine of x.

    Macro Portability:
        Result on error are system dependent; where supported 'errno'
        shall be set to a non-zero manifest constant describing the
        error.

    Macro See Also:
        asin, isnan
*/
void
do_sin(void)                    /* float (float val) */
{
#if defined(DO_LONG_DOUBLE)
    __FEXEC(sinl(arg_float1));
#else
    __FEXEC(sin(arg_float1));
#endif
}


/*  Function:               do_sinh
 *      sinh primitive
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: sinh - Hyperbolic sine function.

        float
        sinh(float val)

    Macro Description:
        The 'sinh()' primitive computes the hyperbolic sine of the
        argument 'val'.

    Macro Returns:
        Upon successful completion, shall return the hyperbolic sine
        of 'x'.

    Macro Portability:
        Result on error are system dependent; where supported 'errno'
        shall be set to a non-zero manifest constant describing the
        error.

    Macro See Also:
        sin
*/
void
do_sinh(void)                   /* float (float val) */
{
#if defined(DO_LONG_DOUBLE)
    __FEXEC(sinhl(arg_float1));
#else
    __FEXEC(sinh(arg_float1));
#endif
}


/*  Function:               do_sqrt
 *      sqrt primitive
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: sqrt - Square root function.

        float
        sqrt(float val)

    Macro Description:
        The 'sqrt()' primitive computes the square root of the
        argument 'val'.

    Macro Returns:
        Upon successful completion, shall return the square root of
        'val'.

    Macro Portability:
        Result on error are system dependent; where supported 'errno'
        shall be set to a non-zero manifest constant describing the
        error.

    Macro See Also:
        isnan
*/
void
do_sqrt(void)                   /* float (float val) */
{
#if defined(DO_LONG_DOUBLE)
    __FEXEC(sqrtl(arg_float1));
#else
    __FEXEC(sqrt(arg_float1));
#endif
}


/*  Function:               do_tan
 *      tan primitive
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: tan - Tangent function

        float
        tan(float val)

    Macro Description:
        The 'tan()' primitive computes the tangent of the argument
        'val', measured in radians.

    Macro Returns:
        Upon successful completion, shall return the tangent of 'val'.

    Macro Portability:
        Result on error are system dependent; where supported 'errno'
        shall be set to a non-zero manifest constant describing the
        error.

    Macro See Also:
        atan, isnan
*/
void
do_tan(void)                    /* float (float val) */
{
#if defined(DO_LONG_DOUBLE)
    __FEXEC(tanl(arg_float1));
#else
    __FEXEC(tan(arg_float1));
#endif
}


/*  Function:               do_tanh
 *      tanh primitive
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: tanh - Hyperbolic tangent function.

        float
        tanh(float val)

    Macro Description:
        The 'tanh()' primitive computes the hyperbolic tangent of the
        argument 'val'.

    Macro Returns:
        Upon successful completion, shall return the hyperbolic
        tangent of 'x'.

    Macro Portability:
        Result on error are system dependent; where supported 'errno'
        shall be set to a non-zero manifest constant describing the
        error.

    Macro See Also:
        tan
*/
void
do_tanh(void)                   /* float (float val) */
{
#if defined(DO_LONG_DOUBLE)
    __FEXEC(tanhl(arg_float1));
#else
    __FEXEC(tanh(arg_float1));
#endif
}

/*end*/
