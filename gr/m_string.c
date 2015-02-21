#include <edidentifier.h>
__CIDENT_RCSID(gr_m_string_c,"$Id: m_string.c,v 1.37 2015/02/21 22:47:27 ayoung Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: m_string.c,v 1.37 2015/02/21 22:47:27 ayoung Exp $
 * String primitives.
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
#include <limits.h>
#include <stdlib.h>
#include <math.h>                               /* HUGE_VAL */
#include <libstr.h>                             /* str_...()/sxprintf() */

#include "m_string.h"                           /* public interface */

#include "accum.h"                              /* acc_...() */
#include "builtin.h"
#include "debug.h"                              /* trace_...() */
#include "eval.h"
#include "keywd.h"
#include "lisp.h"                               /* lst_...() */
#include "symbol.h"                             /* sym_...() */
#include "word.h"

static const char *     space_characters    = " \t\r\n\f\v";
static const char *     trim_characters     = " \t\r\n";

#if !defined(HAVE_STRCASESTR)
static const char *     x_strcasestr(const char *haystack, const char *needle);
#endif


/*  Function:           do_strxlen
 *      Work-horse strlen() and strnlen() primitives.
 *
 *  Parameters:
 *      step - List step increment.
 *
 *  Returns:
 *      String length.
 */
static accint_t
do_strxlen(int step)
{
    accint_t len, longest = 0;
    const LIST *lp;
    int s;

    if (isa_string(1)) {
        return get_strlen(1);
    }

    if (step <= 0) {
        step = 1;                               /* default */
    }

    for (lp = get_list(1); lp && F_HALT != *lp;) {
        switch (*lp) {
        case F_STR:
        case F_LIT:
            len = (int)strlen(LGET_PTR2(const char, lp));
            break;
        case F_ID: {
                const int id = LGET_ID(lp);
                const char *name = builtin[id].b_name;

                assert(id >= 0 || (unsigned)id < builtin_count);
                len = (int)strlen(name);
            }
            break;
        case F_RSTR:
            len = r_used(LGET_PTR2(const ref_t, lp));
            break;
        default:
            len = 0;
            break;
        }

        if (len > longest) {
            longest = len;
        }

        for (s = 0; s < step && lp; ++s) {
            lp = atom_next(lp);
        }
    }

    return longest;
}


/*  Function:           do_strlen
 *      strlen primitive - return length of string argument or length of
 *      longest string in a list.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: strlen - String length.

        int
        strlen(string|list arg, [int step = 1])

    Macro Description:
        The 'strlen()' primitive computes the length of 'arg'.

        For string arguments, computes the length of the string in
        character.

        In the case 'arg' is a list, computes the length of the
        longest string element contained within the list. Non-string
        elements shall be omitted. If specified and positive,
        iteration through the list shall only inspect each 'step'
        element, starting with the first element within the list.

    Macro Parameters:
        arg - String or list.
        step - Optional integer, stating the list iterator step value
                1 or greater.

    Macro Returns:
        The 'strlen()' primitive returns the number of characters
        contained within the string.

    Macro See Also:
        strnlen, length_of_list
 */
void
do_strlen(void)                 /* (string|list arg, [int step = 1]) */
{
    acc_assign_int(do_strxlen(get_xinteger(2, 1)));
}


/*  Function:           do_strnlen
 *      strnlen primitive - return length of string argument or length of
 *      longest string in a list, limited to maxlen.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: strnlen - String length limited to an explicit maximum.

        int
        strnlen(string|list arg, int maxlen, [int step = 1])

    Macro Description:
        The 'strnlen()' primitive computes the length of 'arg' limited
        to 'maxlen'.

        For string arguments, computes the length of the string in
        character.

        In the case 'arg' is a list, computes the length of the
        longest string element contained within the list in
        characters. Non-string elements shall be omitted. If
        specified and positive, iteration through the list shall only
        inspect each 'step' element, starting with the first element
        within the list.

    Macro Parameters:
        arg - String or list.
        maxlen - Upper limit to be applied to the length.
        step - Optional integer, stating the iterator step value.

    Macro Returns:
        The 'strnlen()' primitive returns either the same result as
        strlen() or maxlen, whichever is smaller.

    Macro Portability:
        A Grief extension.

    Macro See Also:
        strlen
 */
void
do_strnlen(void)                /* (string str, int maxlen, [int step = 1]) */
{
    const accint_t len = do_strxlen(get_xinteger(3, 1));
    const accint_t maxlen = get_xinteger(2, 1);

    acc_assign_int(len > maxlen ? maxlen : len);
}


/*  Function:           do_atoi
 *      atoi primitive - string to integer
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: atoi - Convert string to a decimal number.

        int
        atoi(string str, [int svalue = TRUE])

    Macro Description:
        The 'atoi()' primitive converts the initial portion of the
        string 'str' into its numeric value. This behaviour is
        equivalent to using <strtol> as follows.

>           val = strtol(str, NULL, 10);

        Optionally 'atoi' if 'svalue' is specified and zero then the
        ascii value of the first character in string is returned.
        This behaviour is equivalent to using <characterat> as follows.

>           val = characterat(str, 1);

    Macro Parameters:
        str - String object.
        svalue - Optional flag, when stated as FALSE only the value
            of the leading character is returned.

    Macro Returns:
        The 'atoi' function returns integer value of argument string
        treated as an ascii number or the ascii value of the first
        character in string if 'svalue' is zero.

    Macro Portability:
        A Grief extension.

    Macro See Also:
        strtol
 */
void
do_atoi(void)                   /* int (string str, [int svalue = TRUE]) */
{
    const char *cp = get_str(1);

    if (isa_undef(2) || get_xinteger(2, 0)) {
                                            /* string value */
        acc_assign_int((accint_t) accstrtoi(cp, NULL, 10));

    } else {                                /* character value */
        acc_assign_int((accint_t) *((uint8_t *)cp));
    }
}



/*  Function:           do_itoa
 *      itoa primitive - integer to string.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: itoa - Convert an integer into a string.

        string
        itoa(int value, [int base = 10])

    Macro Description:
        The 'itoa()' primitive converts an integer value to a string
        using the specified base and returns the result.

        If base is 10 and value is negative, the resulting string is
        preceded with a minus sign (-). With any other base, value is
        always considered unsigned.

    Macro Returns:
        Upon successful completion, 'itoa()' returns the converted
        value. If no conversion could be performed, 'itoa()' returns
        an empty string.

    Macro Portability:
        A Grief extension.

    Macro See Also:
        atoi, itoa, strtod, strtof, sscanf
 */
void
do_itoa(void)                   /* string (int value, int base = 10) */
{
    static const char hexdigits[] = "0123456789abcdefghijklmnopqrstuvwxyz";

    accint_t value = get_xinteger(1, 0);
    const accint_t base = get_xinteger(2, 10);
    char buffer[128], *cursor = buffer;

    if (base >= 2 && base <= 32) {
        unsigned char sign = 0;

        if (value < 0) {
            if (10 == base) sign = '-';
            value = -value;
        }

        do {
            *cursor++ = hexdigits[(unsigned char)(value % base)];
            value /= base;
        } while (value > 0);

        if (sign) *cursor++ = sign;
        *cursor = '\0';

        {   char *l, *r;
            for (l = buffer, r = (cursor - 1); l < r; ++l, --r) {
                char ch = *r; *r = *l; *l = ch;
            }
        }

        acc_assign_str(buffer, cursor - buffer);

    } else {
        acc_assign_str("", 0);
    }
}


/*  Function:           do_strtol
 *      strtol primitive - string to integer.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: strtol - Convert a string into its numeric value.

        int
        strtol(string str, [int &endoffset], [int base])

    Macro Description:
        The 'strtol()' primitive converts the initial portion of the
        string 'str' to a type integer representation; it is an
        interface to the standard library function of the same name.

        If the value of base is '0' (or if not supplied), the
        expected form of the subject sequence is that of a decimal
        constant, octal constant or hexadecimal constant, any of
        which may be preceded by a '+ 'or '- 'sign. A decimal
        constant begins with a non-zero digit, and consists of a
        sequence of decimal digits. An octal constant consists of the
        prefix 0 optionally followed by a sequence of the digits '0
        'to '7 'only. A hexadecimal constant consists of the prefix
        '0x' or '0X' followed by a sequence of the decimal digits and
        letters 'a' (or 'A') to 'f' (or 'F') with values '10' to '15'
        respectively.

        If the value of base is between '2' and '36', the expected
        form of the subject sequence is a sequence of letters and
        digits representing an integer with the radix specified by
        base, optionally preceded by a '+' or '-' sign. The letters
        from 'a' (or 'A') to 'z' (or 'Z') inclusive are ascribed the
        values '10' to '35'; only letters whose ascribed values are
        less than that of base are permitted. If the value of base is
        16, the characters 0x or 0X may optionally precede the
        sequence of letters and digits, following the sign if present.

        The subject sequence is defined as the longest initial
        subsequence of the input string, starting with the first
        non-white-space character, that is of the expected form. The
        subject sequence contains no characters if the input string
        is empty or consists entirely of white-space characters, or
        if the first non-white-space character is other than a sign
        or a permissible letter or digit.

        If the subject sequence has the expected form and the value
        of base is 0, the sequence of characters starting with the
        first digit is interpreted as an integer constant. If the
        subject sequence has the expected form and the value of base
        is between '2' and '36', it is used as the base for
        conversion, ascribing to each letter its value as given
        above. If the subject sequence begins with a minus sign, the
        value resulting from the conversion is negated.

    Macro Returns:
        Upon successful completion, 'strtol()' returns the converted
        value, if any, and (if supplied) an index (base of 1) to the
        first unprocessed character within the string is stored in
        the integer object 'endoffset'.

        If no conversion could be performed, 'strtol()' returns 0 and
        <errno> may be set to *EINVAL*. The subject sequence contains
        no characters and index of 0 is stored in 'endoffset'.

        If the correct value is outside the range of representable
        values, 'strtol()' returns *LONG_MAX* or *LONG_MIN* and
        <errno> is set to *ERANGE*.

    Macro Portability:
        A Grief extension.

    Macro See Also:
        atoi, itoa, strtod, strtof, sscanf
 */
void
do_strtol(void)                 /* int (string str, [int endoffset], [int base]) */
{
    const char *cp = get_str(1);
    int base = get_xinteger(3, 0);
    char *endp = NULL;
    accint_t ret;

    if (base < 0 || base > 36) {
        base = 0;                               /* strtol limits */
    }
    errno = 0;
    ret = accstrtoi(cp, &endp, base);
    if (!isa_undef(2)) {
        if (EINVAL == errno || endp == cp) {    /* conversion error */
            sym_assign_int(get_symbol(2), 0);
            ret = 0;

        } else {                                /* success */
            sym_assign_int(get_symbol(2), (endp - cp) + 1);
        }
    }
    acc_assign_int(ret);
    system_call(-1);
}


/*  Function:           do_strtod
 *      strtod primitive - string to double precision float.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: strtod - String to double.

        int
        strtod(string str, [int &endofoffset])

    Macro Description:
        The 'strtod()' primitive converts the initial portion of the
        string 'str' to a floating point double representation; it
        is an interface to the standard library function of the
        same name.

    Macro Returns:
        Upon successful completion, 'strtod()' returns the converted
        value, if any, and (if supplied) an index (base of 1) to the
        first unprocessed character within the string is stored in
        the integer object 'endoffset'.

        If no conversion could be performed, strtod() returns 0 and
        <errno> may be set to *EINVAL*. The subject sequence contains
        no characters and index of 0 is stored in 'endoffset'.

        If the correct value is outside the range of representable
        values, strtod() returns *HUGE_MAX* or *HUGE_MIN* and <errno>
        is set to *ERANGE*.

    Macro Portability:
        A Grief extension.

    Macro See Also:
        atoi, strtof, sscanf
 */
void
do_strtod(void)                 /* float (string str, [int endoffset]) */
{                                               /* 05/08/09 */
    const char *cp = get_str(1);
    char *endp = NULL;
    double ret;

    errno = 0;
    ret = strtod(cp, &endp);
    if (!isa_undef(2)) {
        if (EINVAL == errno || endp == cp) {    /* conversion error */
            sym_assign_int(get_symbol(2), 0);
            ret = 0;

#if defined(HUGE_VAL)
        } else if (ret < -HUGE_VAL || ret > HUGE_VAL) {
            sym_assign_int(get_symbol(2), 0);
            errno = ERANGE;
            ret = 0;
#endif

        } else {                                /* success */
            sym_assign_int(get_symbol(2), (endp - cp) + 1);
        }
    }

    acc_assign_float((accfloat_t) ret);
    system_call(-1);
}


/*  Function:           do_strtof
 *      strtof primitive - string to float
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: strtof - String to float.

        int
        strtof(string str, [int &endofoffset])

    Macro Description:
        The 'strtof()' primitive converts the initial portion of the
        string 'str' to a floating point representation; it is an
        interface to the standard library function of the same name.

    Macro Returns:
        Upon successful completion, 'strtof()' returns the converted
        value, if any, and (if supplied) an index (base of 1) to the
        first unprocessed character within the string is stored in
        the integer object 'endoffset'.

        If no conversion could be performed, 'strtof()' returns 0 and
        <errno> may be set to *EINVAL*. The subject sequence contains
        no characters and index of 0 is stored in 'endoffset'.

        If the correct value is outside the range of representable
        values, 'strtod()' returns *FLT_MAX* or *FLT_MIN* and <errno>
        is set to ERANGE.

    Macro Portability:
        A Grief extension

    Macro See Also:
        atoi, strtod, sscanf

 */
void
do_strtof(void)                 /* float (string str, [int endoffset]) */
{                                               /* 05/08/09 */
    const char *cp = get_str(1);
    char *endp = NULL;
    double ret;

    errno = 0;
    ret = strtod(cp, &endp);
    if (!isa_undef(2)) {
        if (EINVAL == errno || endp == cp) {    /* conversion error */
            sym_assign_int(get_symbol(2), 0);
            errno = EINVAL;
            ret = 0;

#if defined(FLT_MIN) && defined(FLT_MAX)
        } else if (ret < FLT_MIN || ret > FLT_MAX) {
            sym_assign_int(get_symbol(2), 0);
            errno = ERANGE;
            ret = 0;
#endif

        } else {                                /* success */
            sym_assign_int(get_symbol(2), (endp - cp) + 1);
        }
    }

    acc_assign_float(ret);
    system_call(-1);
}


/*  Function:           do_string_count
 *      string_count primitive - count of matching substrs.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: string_count - Count occurrences of characters in a string.

        int
        string_count(string haystack, int needle|string needles)

    Macro Description:
        The 'string_count()' primitive computes the number of
        occurrences of the character(s) within 'needles' in the
        string 'haystack'.

    Macro Parameters:
        haystack - String to be searched.
        needle - Elements to the counted, each character shall be accumulated.

    Macro Returns:
        Returns the number of times the characters in 'needle' occur
        in the string parameter.

    Macro Portability:
        The integer needle form is a Grief extension.

    Macro See Also:
        substr, rindex, index
 */
void
do_string_count(void)           /* int (string str, int character|string substrcop) */
{
    register const char *str1 = get_str(1);
    int val = 0;

    if (isa_integer(2)) {                       /* extension */
        const int ch = get_xinteger(2, 0);

        if (ch > 0) {
            while (*str1) {
                if (NULL == (str1 = strchr(str1, ch))) {
                    break;
                }
                ++str1;
                ++val;
            }
        }
    } else {
        register const char *str2 = get_str(2);

        while (*str1) {
            if (strchr(str2, *str1++)) {
                ++val;
            }
        }
    }

    acc_assign_int(val);
}


/*  Function:           do_index
 *      index primitive - index of substr or character.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: index - Search string for a leftmost sub-string or character.

        int
        index(string str, int ch|string s)

    Macro Description:
        The 'index()' primitive returns the offset to the first
        occurrence of the character 'ch' or the string 's' in the
        string 'str'.

    Macro Parameters:
        str - String object to be searched.
        ch|s - Object to be matched against.

    Macro Returns:
        The 'index()' primitive returns the starting offsetr or 0 if
        the character or string was not found.

    Macro Portability:
        The character needle form is a Grief extension.

    Macro See Also:
        rindex
 */
void
do_index(void)                  /* int (string str, int ch|string s) */
{
    const char *str = get_str(1);
    const char *cp;
    int val = 0;

    if (isa_integer(2)) {                       /* extension */
        const int ch = get_xinteger(2, 0);

        if (ch > 0) {
            if (NULL != (cp = strchr(str, ch))) {
                val = (cp - str) + 1;
            }
        }

    } else {
        const char *cp2 = get_xstr(2);

        if (NULL == cp2 || 0 == *cp2) {
            val = get_strlen(1) + 1;            /* EOS */

        } else if (NULL != (cp = strstr(str, cp2))) {
            val = (cp - str) + 1;               /* character position */
        }
    }

    acc_assign_int(val);
}


/*  Function:           do_firstof
 *      firstof primitive - first index of character.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: firstof - Leftmost character search.

        int
        firstof(string str, string chars, [int &result])

    Macro Description:
        The 'firstof()' primitive returns the offset to the first
        occurrence of any of the characters contained within 'chars'
        in the string 'str'.

        If supplied, on success 'result' shall be populated with the
        matching character otherwise is set if zero.

        firstof() is similar to <index> yet allows multiple
        characters to be matched.

    Macro Parameters:
        str - String object to be searched.
        chars - Character set to match against.
        result - Optional result, populated with the matching character value.

    Macro Returns:
        The 'firstof()' primitive returns the starting offset or 0 if
        non of the characters are found.

    Macro Portability:
        A Grief extension.

    Macro See Also:
        lastof, index, rindex
 */
void
do_firstof(void)                /* int (string str, string chars [,int &result]) */
{
    const char *start = get_str(1), *str = start,
            *end = str + get_strlen(1);
    const char *chars = get_xstr(2);
    const char *cp;
    int val = 0;

    while (str < end)  {
        if (NULL != (cp = strchr(chars, *str++))) {
            val = str - start;                  /* starting from offset 1 */
            break;
        }
    }

    sym_assign_int(get_symbol(3), (val > 0 ? str[-1] : 0));
    acc_assign_int(val);
}


/*  Function:           do_rindex
 *      rindex primitive - reverse index of substr or character.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: rindex - Search string for a rightmost sub-string or character.

        int
        rindex(string str, int ch|string s)

    Macro Description:
        The 'rindex()' primitive returns the offset to the last (in
        otherwords the right) occurrence of the character 'ch' or the
        string 's' in the string 'str'.

    Macro Parameters:
        str - String object to be searched.
        ch|s - Object to be matched against.

    Macro Returns:
        The 'rindex()' primitives returns the starting offset or 0 if
        the character or string was not found.

    Macro Portability:
        The character needle form is a Grief extension.

    Macro See Also:
        index
 */
void
do_rindex(void)                 /* int (string str, int ch|string s) */
{
    const char *str = get_str(1);
    const char *cp;
    int val = 0;

    if (isa_integer(2)) {                       /* extension 21/04/06 */
        int ch = get_xinteger(2, 0);

        if (ch > 0) {
            if ((cp = strrchr(str, ch)) != NULL) {
                val = (cp - str) + 1;
            }
        }

    } else {
        const char *str2 = get_str(2);
        int len = get_strlen(2);

        for (cp = str + get_strlen(1) - 1; cp >= str; --cp)
            if (0 == strncmp(cp, str2, len)) {
                val = (cp - str) + 1;
                break;
            }
    }

    acc_assign_int(val);
}


/*  Function:           do_lastof
 *      lastof primitive - last index of character.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: lastof - Rightmost character search.

        int
        lastof(string str, string chars, [int &result])

    Macro Description:
        The 'lastof()' primitive returns the offset to the last
        occurrence of any of the characters contained within 'chars'
        in the string 'str'.

        If supplied, on success 'result' shall be populated with the
        matching character otherwise is set if zero.

        lastof() is similar to <rindex> yet allows multiple
        characters to be matched.

    Macro Parameters:
        str - String object to be searched.
        chars - Character set to match against.
        result - Optional result, populated with the matching character value.

    Macro Returns:
        The 'lastof()' primitive returns the starting offset or 0 if
        non of the characters are found.

    Macro Portability:
        A Grief extension.

    Macro See Also:
        lastof, index, rindex
 */
void
do_lastof(void)                 /* int (string str, string chars [,int &result]) */
{
    const char *start = get_str(1),
            *end = start + get_strlen(1), *str = end;
    const char *chars = get_xstr(2);
    const char *cp;
    int val = 0;

    while (str > start)  {
        if (NULL != (cp = strchr(chars, *--str))) {
            val = (str - start) + 1;        /* starting from offset 1 */
            break;
        }
    }

    sym_assign_int(get_symbol(3), (val > 0 ? *str : 0));
    acc_assign_int(val);
}


/*  Function:           do_substr
 *      substr primitive - retrieve a substr.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: substr - Extract a sub-string.

        string
        substr(string str, [int offset], [int length])

    Macro Description:
        The 'substr()' primitive extracts parts of a string, beginning
        at the character at the specified position, and returns the
        specified number of characters.

        The 'substr()' primitive does not change the original string.

    Macro Parameters:
        offset - The position where to start the extraction. First
                        character is at index '1'.

        length - Optional, the number of characters to extract.

    Macro Returns:
        Returns the sub-string of string which starts at start, and
        goes on for length characters, or the end of the string if
        length is omitted.

    Macro See Also:
        index, rindex
 */
void
do_substr(void)                 /* string (string str, [int offset], [int length]) */
{
    const char *str = get_str(1);
    int slen = get_strlen(1);
    int offset = get_xinteger(2, 1);            /* non-optional?? */
    const char *cp;
    int length;

    if (--offset < 0) {                         /* index-1 */
        //
        //  TODO: JavaScript style, to extract characters from the end of
        //  the string, use a negative start number.
        //
        offset = 0;
    } else if (offset > slen) {
        offset = slen;
    }
    cp = str + offset;
    slen -= offset;

    if (isa_undef(3)) {
        length = slen;
    } else if ((length = get_xinteger(3, 0)) < 0) {
        length = 0;
    } else if (length > slen) {
        length = slen;
    }
    acc_assign_str(cp, length);
}


/*  Function:           acc_ltrim
 *      Left trim the accumerlator.
 *
 *  Parameters:
 *      trimchars -         Trim buffer.
 *
 *  Returns:
 *      nothing
 */
static void
acc_ltrim(const char *trimchars)
{
    char *acc = acc_get_sbuf();                 /* working image */
    register const char *cursor;

    if (NULL == trimchars) {
        trimchars = trim_characters;
    }
    cursor = acc;
    while (*cursor && strchr(trimchars, *cursor)) {
       ++cursor;
    }
    if (cursor > acc) {
        while (*cursor) {
            *acc++ = *cursor++;
        }
        *acc = 0;
    }
}


/*  Function:           acc_rtrim
 *      Right trim the accumulator.
 *
 *  Parameters:
 *      trimchars -         Trim buffer.
 *      len -               Buffer length.
 *
 *  Returns:
 *      nothing
 */
static void
acc_rtrim(const char *trimchars, int len)
{
    char *acc = acc_get_sbuf();                 /* working image */

    if (len > 0) {
        register char *cursor = acc + len;

        assert(0 == *cursor);
        if (NULL == trimchars) {
            trimchars = trim_characters;
        }
        while (--cursor >= acc && strchr(trimchars, *cursor)) {
            *cursor = 0;
        }
    }
}


/*  Function:           do_compress
 *      compress primitive - compress multiple instances of white-space.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: compress - Compress repeated instances of white-space characters.

        string
        compress(string str, [int trim = FALSE],
                    [string chars = " \t\r\n"], [int replacement = ' '])

    Macro Description:
        The 'compress()' primitive takes a string and removes all
        multiple white space characters, by converting consecutive
        white-space characters into a single white-space character.

        The default is to compress all tabs, spaces and newline
        characters. If 'trimchars' is specified, then all characters
        within the 'trimchar' string are compressed.

        If 'trim' is specified then 'compress()' acts the same as

>           trim(compress(str,trimchars),trimchars)

    Macro Parameters:
        str - String object to be compressed.

        trim - Optional flag, when *TRUE* invokes trim on the
            compressed result.

        chars - Optional string defining the set to characters be to
            compressed and optionally trimmed.

        replacement - Optional replacement character, if given as a
            non-positive (<= 0) value when the first character with
            the consecutive set shall be used.

    Macro Returns:
        Returns a copy of string with all spaces, tabs and newlines
        mapped to single spaces.

    Macro Portability:
        The 'chars' and 'replacement' options are a Grief extensions.

        By default BRIEF preserved the first character in every group of
        compressed characters, whereas Grief replaces them with a single
        space; unless replacement is stated as a non-positive number
        (e.g. -1).

    Macro See Also:
        trim, rtrim, ltrim
 */
void
do_compress(void)               /* (string str, [int trim = FALSE],
                                        [string chars = NULL], [int replacement = ' ']) */
{
    const char *str = get_xstr(1);
    const int   len = (str ? get_strlen(1) : 0);
    const int   dotrim = get_xinteger(2, FALSE);
    const char *chars = get_xstr(3);
    const int   replacement = get_xinteger(4, ' ');
    char *cursor, *acc = acc_expand(len + 1);

    if (NULL == chars) chars = space_characters;
    cursor = acc;
    if (str) {
        while (*str) {
            if (strchr(chars, *str)) {
                const int first = *str;

                while (*++str && strchr(chars, *str))
                    /*continue*/;
                *cursor++ = (char)(replacement > 0 ? replacement : first);
                if (!*str) {
                    break;                      /* EOS */
                }
            }
            *cursor++ = *str++;
        }
    }

    *cursor = 0;
    acc_assign_strlen(cursor - acc);

    if (dotrim && *acc) {
        acc_rtrim(chars, cursor - acc);
        acc_ltrim(chars);
    }
}


/*  Function:           do_trim
 *      trim primitive - string left/right trim.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: trim - Chomp characters from a string.

        string
        trim(string str, [string chars = " \t\r\n"])

    Macro Description:
        The 'trim()' primitive removes leading and trailing characters
        from the specified 'string'.

        The default is to remove all tabs, spaces and newline
        characters. If 'chars' is specified, then all characters
        within the trimchar string are removed from the beginning and
        end of the string.

    Macro Parameters:
        str - String object to be trimmed.
        chars - Optional string defining the set to characters to be
                    removed.

    Macro Returns:
        Returns a copy of string with all leading and trailing white
        space characters removed. (spaces, tabs and newlines by
        default).

    Macro Portability:
        The 'chars' and removal of trailing characters in addition to
        leading is a Grief extension (see rtrim).

    Macro See Also:
        compress, ltrim, rtrim
 */
void
do_trim(void)                   /* (string str, [string chars = NULL]) */
{
    const int len = get_strlen(1);
    const char *chars = get_xstr(2);

    acc_assign_str(get_str(1), len);
    acc_rtrim(chars, len);
    acc_ltrim(chars);
}


/*  Function:           do_rtrim
 *      rtrim primitive - string right/trailing trim.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: rtrim - Chomp characters from the end of a string.

        string
        rtrim(string str, string chars = " \t\r\n")

    Macro Description:
        The 'rtrim()' primitive removes trailing (or right) characters
        from the specified 'string'.

        The default is to remove all tabs, spaces and newline
        characters. If 'chars' is specified, then all characters
        within the 'trimchar' string are removed from the end of the
        string.

    Macro Returns:
        Returns a copy of string with all trailing white space
        characters removed. (spaces, tabs and newlines by default).

    Macro Portability:
        A Grief extension; this is a replacement of the original
        trim() function.

    Macro See Also:
        compress, trim, ltrim
 */
void
do_rtrim(void)                  /* (string str, [string chars = NULL]) */
{
    const int len = get_strlen(1);

    acc_assign_str(get_str(1), len);
    acc_rtrim(get_xstr(2), len);
}


/*  Function:           do_ltrim
 *      ltrim primitive - string left/leading trim.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: ltrim - Chomp characters from the front of a string.

        string
        ltrim(string str, [string chars = NULL])

    Macro Description:
        The 'ltrim()' primitive removes leading (or left) characters
        from the specified 'string'.

        The default is to remove all tabs, spaces and newline
        characters. If 'chars' is specified, then all characters
        within the 'trimchar' string are removed from the beginning
        of the string.

    Macro Returns:
        Returns a copy of string with all leading white space
        characters removed. (spaces, tabs and newlines by default).

    Macro Portability:
        n/a

    Macro See Also:
        compress, trim, rtrim
 */
void
do_ltrim(void)                  /* (string str, [string chars = NULL]) */
{
    acc_assign_str(get_str(1), get_strlen(1));
    acc_ltrim(get_xstr(2));
}


/*  Function:           do_upper
 *      upper primitive - convert specified character or string to uppercase.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: upper - Convert string or character to uppercase.

        string|int
        upper(string str|int character)

    Macro Description:
        The 'upper()' primitive converts all alphabetic characters
        within the string object 'str' or just the specified
        character 'ch' to uppercase.

    Macro Returns:
        Returns the specified object with all alphabetic characters
        converted to uppercase.

        If the argument is a string then a copy of the string is
        returned, otherwise the integer value of the converted
        character.

    Macro Portability:
        Character support is a Grief extension.

    Macro See Also:
        lower
 */
void
do_upper(void)                  /* (string str|int character, [TODO int capitalize) */
{
    if (isa_integer(1)) {                       /* 24/04/06 */
        int ch = get_xinteger(1, 0);

        if (ch >= 'a' && ch <= 'z') {           /* ASCII/MCHAR??? */
            ch = toupper(ch);
        }
        acc_assign_int(ch);

    } else {
        register unsigned char *cp;

        acc_assign_str(get_str(1), get_strlen(1));
        cp = (unsigned char *)acc_get_sbuf();
        for (; *cp; ++cp) {
            const int ch = *cp;

            if (ch >= 'a' && ch <= 'z') {       /* ASCII/MCHAR??? */
                *cp = (unsigned char)toupper(ch);
            }
        }
    }
}


/*  Function:           do_lower
 *      lower primitive - convert specified character or string to lower-case.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: lower - Convert string or character to lowercase.

        string|int
        lower(string str|int character)

    Macro Description:
        The 'lower()' primitive converts all alphabetic characters
        within the string object 'str' or just the specified
        character 'ch' to lowercase.

    Macro Returns:
        Returns the specified object with all alphabetic characters
        converted to lowercase.

        If the argument is a string then a copy of the string is
        returned, otherwise the integer value of the converted
        character.

    Macro Portability:
        Character support is a Grief extension.

    Macro See Also:
        lower
 */
void
do_lower(void)                  /* string|int (string str|int character) */
{
    if (isa_integer(1)) {                       /* 24/04/06 */
        int ch = get_xinteger(1, 0);

        if (ch >= 'A' && ch <= 'Z') {           /* ASCII/MCHAR??? */
            ch = tolower(ch);
        }
        acc_assign_int(ch);

    } else {
        register unsigned char *cp;

        acc_assign_str(get_str(1), get_strlen(1));
        cp = (unsigned char *)acc_get_sbuf();

        for (; *cp; ++cp) {
            const int ch = *cp;

            if (ch >= 'A' && ch <= 'Z') {       /* ASCII/MCHAR??? */
                *cp = (unsigned char)tolower(ch);
            }
        }
    }
}


/*  Function:           isa
 *      Workhorse for isxxx primitive set.
 *
 *  Description:
 *      The isxxxx set of primitives are used to test whether the specified
 *      argument matches a specific character class.
 *
 *      The specified argument can be an integer (whose ASCII code represents a single
 *      character), or a string, in which case the first character of the string is
 *      tested. The optional index allows an alternative character within the string to
 *      be tested, starting at offset one being the first character.
 *
 *  Parameters:
 *      ch | str -          Character or string.
 *
 *      [index] -           If a string, an optional character index within the string,
 *                          starting at offset one being the first character. if the index
 *                          denotes a character out-of-bounds, the function return 0.
 *
 *<<GRIEF>> [string]
    Topic: ctype

        Character classes

    Description:
        The Grief ctype (character class) functionality is used to
        designated character-coded integer values into one or more
        character types.

    Standard character classes are;

        o  alnum    - An alphanumeric (letter or digit).
        o  alpha    - A letter.
        o  ascii    - An ASCII character (ie 0 > x < 127).
        o  blank    - A space or tab character.
        o  cntrl    - A control character.
        o  csym     - A symbol.
        o  digit    - A decimal digit.
        o  graph    - A character with a visible representation.
        o  lower    - A lower-case letter.
        o  print    - An alphanumeric (same as alnum).
        o  punct    - A punctuation character.
        o  space    - A character producing white space in displayed text.
        o  upper    - An uppercase letter.
        o  xdigit   - A hexadecimal digit.

    Macro Support:

        The following Grief interfaces have direct support for
        character-classes:

    *Bracket expressions*

        Within <sscanf> and search <regexp> expressions, the name of
        a character class enclosed in [: and :] stands for the list
        of all characters (not all collating elements!) belonging to
        that class.

    *Macros*

        The set of the isaxxxx(object, [index]) primitive exist
        allowing tests within macros. Each of these subroutines
        returns a nonzero value is the specified value is contained
        within the given class, otherwise zero.

        The specified argument 'object' can be an integer (whose
        ASCII code represents a single character), or a string, in
        which case the first character of the string is tested. The
        optional index allows an alternative character within the
        string to be tested, starting at offset one being the first
        character.

    Note!:
        The 'isaxxx()' primitives should only be used on character data
        that can be represented by a single byte value (0 through
        255). Attempting to use the ctype subroutines on multi-byte
        locale data may give inconsistent results.

    Macro See Also:
        sscanf, regexp, isalnum, isalpha, isascii, iscntrl, iscsym,
        isdigit, isgraph, islower, isprint, ispunct, isspace, isupper,
        isxdigit

 */
static void
isa(int (*isacmp)(int))         /* (string str|int character) */
{
    int ret = 0;

    if (isa_integer(1)) {                       /* 27/04/06 */
        const int ch = get_xinteger(1, 0);

        if (ch > 0 && ch < 255) {
            ret = isacmp(get_xinteger(1, 0));
        }

    } else {
        const unsigned char *cp = (unsigned char *)get_str(1);
        int length = get_strlen(1);

        if (cp && *cp) {
            if (isa_integer(2)) {               /* index */
                const accint_t position = get_xaccint(2, 0);

                if (position >= 1 && position <= length) {
                    ret = isacmp((int) cp[position - 1]);
                }
            } else {                            /* 1st charactr */
                ret = isacmp((int) *cp);
            }
        }
    }
    acc_assign_int(ret ? 1 : 0);
}


#if !defined(HAVE_ISASCII)
static int
isascii(int c)
{
    return (c >= 0 && c <= 0x7f);
}
#endif  /*HAVE_ISASCII*/


#if !defined(HAVE_ISBLANK)
static int
isblank(int c)
{
    return (' ' == c || '\t' == c);
}
#endif  /*HAVE_ISBLANK*/


static int
isword(int c)
{
    return ('_' == c || '-' == c || isalnum(c));
}


#if !defined(HAVE_ISCSYM) && \
        !defined(_MSC_VER) && !defined(__WATCOMC__) && !defined(__MINGW32__)
static int
iscsym(int c)
{
    return ('_' == c || isalnum(c));
}
#endif


/*  Function:           do_isalnum
 *      isalnum primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: isalnum - Alphanumeric character predicate.

        int
        isalnum(string |int object, [int index])

    Macro Description:
        The 'isalnum()' primitive determines whether the specified
        object 'object' belongs to the 'alpha' or 'numeric'
        character-classes.

        This is ['0' through '9'], ['A' through 'Z'] and ['a' through
        'z'] in the program's current locale; which is equivalent to
        (isalpha() || isdigit()).

        The specified object can be an integer (whose ASCII code
        represents a single character), or a string, in which case
        the first character of the string is tested.

        The optional 'index' allows an alternative character within
        the string to be tested, with the first character represented
        by offset 'one'.

    Macro Parameters:
        object - Character or string object.

        [index] - If a string, an optional character index within the
            string, starting at offset one being the first character. if
            the index denotes a character out-of-bounds, the function
            returns 0.

    Macro Returns:
        The 'isalnum()' primitive returns non-zero if 'object' is an
        alphanumeric character; otherwise it returns 0.

    Macro Portability:
        The 'index' option is a Grief extension.

    Macro Example:

>       if (isalnum(read(1)))
>           message("Next character is alnum.");

    Macro See Also:
        ctype
 */
void
do_isalnum(void)                /* int (string str|int character) */
{
    isa(isalnum);
}


/*  Function:           do_isalpha
 *       isalpha primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: isalpha - Alpha character predicate.

        int
        isalpha(string |int object, [int index])

    Macro Description:
        The 'isalph()' primitive determines whether the specified
        object 'object' belongs to the 'alpha' character-class.

        This is ['A' through 'Z'] and ['a' through 'z'] in the
        program's current locale; which is equivalent to (isupper()
        || islower()).

        The specified object can be an integer (whose ASCII code
        represents a single character), or a string, in which case
        the first character of the string is tested.

        The optional 'index' allows an alternative character within
        the string to be tested, with the first character represented
        by offset 'one'.

    Macro Parameters:
        object - Character or string object.

        [index] - If a string, an optional character index within the
            string, starting at offset one being the first character.
            if the index denotes a character out-of-bounds, the
            function returns 0.

    Macro Returns:
        The 'isalpha()' primitive returns non-zero if 'object' is an
        alphanumeric character; otherwise it returns 0.

    Macro Example:

>       if (isalpha(read(1)))
>           message("Next character is a alpha character.");

    Macro Portability:
        The 'index' option is a Grief extension.

    Macro See Also:
        ctype
 */
void
do_isalpha(void)                /* int (string str|int character) */
{
    isa(isalpha);
}


/*  Function:           do_isascii
 *      isascii primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: isascii - ASCII character predicate.

        int
        isascii(string |int object, [int index])

    Macro Description:
        The 'isascii()' primitive determines whether the specified
        object 'object' belongs to the 'ascii' character-class.

        This is any character value in the range 0 through 0177 ('0'
        through '0x7F'), inclusive.

        The specified object can be an integer (whose ASCII code
        represents a single character), or a string, in which case
        the first character of the string is tested.

        The optional 'index' allows an alternative character within
        the string to be tested, with the first character represented
        by offset 'one'.

    Macro Parameters:
        object - Character or string object.

        [index] - If a string, an optional character index within the
            string, starting at offset one being the first character.
            if the index denotes a character out-of-bounds, the
            function returns 0.

    Macro Returns:
        The 'isascii()' primitive returns non-zero if 'object' is an
        alphanumeric character; otherwise it returns 0.

    Macro Example:

>       if (isascii(read(1)))
>           message("Next character is an ascii character.");

    Macro Portability:
        The 'index' option is a Grief extension.

    Macro See Also:
        ctype
 */
void
do_isascii(void)                /* int (string str|int character) */
{
    isa(isascii);
}


/*  Function:           do_isblank
 *      isblank primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: isblank - Blank character predicate.

        int
        isblank(string |int object, [int index])

    Macro Description:
        The 'isblank()' primitive determines whether the specified
        object 'object' belongs to the 'blank' character-class.

        This is a space ( ) or tab (\t) character.

        The specified object can be an integer (whose ASCII code
        represents a single character), or a string, in which case
        the first character of the string is tested.

        The optional 'index' allows an alternative character within
        the string to be tested, with the first character represented
        by offset 'one'.

    Macro Parameters:
        object - Character or string object.

        [index] - If a string, an optional character index within the
            string, starting at offset one being the first character.
            if the index denotes a character out-of-bounds, the
            function returns 0.

    Macro Returns:
        The 'isblank()' primitive returns non-zero if 'object' is an
        alphanumeric character; otherwise it returns 0.

    Macro Example:

>       if (isblank(read(1)))
>           message("Next character is blank.");

    Macro Portability:
        The 'index' option is a Grief extension.

    Macro See Also:
        ctype
 */
void
do_isblank(void)                /* int (string str|int character) */
{
    isa(isblank);
}


/*  Function:           do_isword
 *      isword primitives.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: isword - Word character predicate.

        int
        isword(string |int object, [int index])

    Macro Description:
        The 'isword()' primitive determines whether the specified
        object 'object' belongs to the 'word' character-class.

        This is [A through Z], [a through z], [o through 9] and
        underscore (_) or dash (-) in the program's current locale;
        which is equivalent to (isalpha() || isdigit() || '_' || '-').

        The specified object can be an integer (whose ASCII code
        represents a single character), or a string, in which case
        the first character of the string is tested.

        The optional 'index' allows an alternative character within
        the string to be tested, with the first character represented
        by offset 'one'.

    Macro Parameters:
        object - Character or string object.

        [index] - If a string, an optional character index within the
            string, starting at offset one being the first character.
            if the index denotes a character out-of-bounds, the
            function returns 0.

    Macro Returns:
        The 'isword()' primitive returns non-zero if 'object' is an word
        character; otherwise it returns 0.

    Macro Example:

>       if (isword(read(1)))
>           message("Next character is a word character.");

    Macro Portability:
        The 'index' option is a Grief extension.

    Macro See Also:
        ctype
 */
void
do_isword(void)                 /* int (string str|int character) */
{
    isa(isword);
}


/*  Function:           do_iscsym
 *      iscsym primitives.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: iscsym - A symbol character predicate.

        int
        iscsym(string |int object, [int index])

    Macro Description:
        The 'iscsym()' primitive determines whether the specified
        object 'object' belongs to the 'c-symbol' classes, which
        represent a symbol in the C/C++, Grief macro and similar
        languages.

        This is ['0' through '9'], ['A' through 'Z'], ['a' through
        'z'] and underscore (_) in the program's current locale.
        which is equivalent to (isalpha() || isdigit() || '_').

        The specified object can be an integer (whose ASCII code
        represents a single character), or a string, in which case
        the first character of the string is tested.

        The optional 'index' allows an alternative character within
        the string to be tested, with the first character represented
        by offset 'one'.

    Macro Parameters:
        object - Character or string object.

        [index] - If a string, an optional character index within the
            string, starting at offset one being the first character.
            if the index denotes a character out-of-bounds, the
            function returns 0.

    Macro Returns:
        The 'iscsym()' primitive returns non-zero if 'object' is a
        symbol character; otherwise it returns 0.

    Macro Example:

>       if (iscsym(read(1)))
>           message("Next character is symbol character.");

    Macro Portability:
        The 'index' option is a Grief extension.

    Macro See Also:
        ctype
 */
void
do_iscsym(void)                 /* int (string str|int character) */
{
    isa(iscsym);
}


/*  Function:           do_iscntrl
 *      iscntrl primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: iscntrl - Control character predicate.

        int
        iscntrl(string |int object, [int index])

    Macro Description:
        The 'iscntrl()' primitive determines whether the specified
        object 'object' belongs to the 'control' character-class.

        The control-class is any character for which the isprint()
        subroutine returns a value of False (0) and any character
        that is designated a control character in the current locale.
        For the C locale, control characters are the ASCII delete
        character (0177 or 0x7F), or an ordinary control character

        The specified object can be an integer (whose ASCII code
        represents a single character), or a string, in which case
        the first character of the string is tested.

        The optional 'index' allows an alternative character within
        the string to be tested, with the first character represented
        by offset 'one'.

    Macro Parameters:
        object - Character or string object.

        [index] - If a string, an optional character index within the
            string, starting at offset one being the first character.
            if the index denotes a character out-of-bounds, the
            function returns 0.

    Macro Returns:
        The 'iscntrl()' primitive returns non-zero if 'object' is an
        control character; otherwise it returns 0.

     Macro Example:

>       if (iscntrl(read(1)))
>           message("Next character is a control character.");

    Macro Portability:
        The 'index' option is a Grief extension.

    Macro See Also:
        ctype
 */
void
do_iscntrl(void)                /* int (string str|int character) */
{
    isa(iscntrl);
}


/*  Function:           do_isdigit
 *      isdigit primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: isdigit - Numeric character predicate.

        int
        isdigit(string |int object, [int index])

    Macro Description:
        The 'isdigit()' primitive determines whether the specified
        object 'object' belongs to the 'numeric' character-class.

        This is ['0' through '9'] in the program's current locale.

        The specified object can be an integer (whose ASCII code
        represents a single character), or a string, in which case
        the first character of the string is tested.

        The optional 'index' allows an alternative character within
        the string to be tested, with the first character represented
        by offset 'one'.

    Macro Parameters:
        object - Character or string object.

        [index] - If a string, an optional character index within the
            string, starting at offset one being the first character.
            if the index denotes a character out-of-bounds, the
            function returns 0.

    Macro Returns:
        The 'isdigit()' primitive returns non-zero if 'object' is an
        numeric character; otherwise it returns 0.

    Macro Example:

>       if (isdigit(read(1)))
>           message("Next character is a numeric character.");

    Macro Portability:
        The 'index' option is a Grief extension.

    Macro See Also:
        ctype
 */
void
do_isdigit(void)                /* int (string str|int character) */
{
    isa(isdigit);
}


/*  Function:           do_isgraph
 *      isgraph primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: isgraph - Graphic character predicate.

        int
        isgraph(string |int object, [int index])

    Macro Description:
        The 'isgraph()' primitive determines whether the specified
        object 'object' belongs to the 'graphic' character-classes.

        The a graphic character is equivalent to

>           (isprint() && not-space)

        The specified object can be an integer (whose ASCII code
        represents a single character), or a string, in which case the
        first character of the string is tested.

        The optional 'index' allows an alternative character within the
        string to be tested, with the first character represented by
        offset 'one'.

    Macro Parameters:
        object - Character or string object.

        [index] - If a string, an optional character index within the
            string, starting at offset one being the first character.
            if the index denotes a character out-of-bounds, the
            function returns 0.

    Macro Returns:
        The 'isgraph()' primitive returns non-zero if 'object' is an
        graphic character; otherwise it returns 0.

    Macro Example:

>       if (isgraph(read(1)))
>           message("Next character is a graphic character.");

    Macro Portability:
        The 'index' option is a Grief extension.

    Macro See Also:
        ctype
 */
void                            /* int (string str|int character) */
do_isgraph(void)
{
    isa(isgraph);
}


/*  Function:           do_isprint
 *      isprint primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: isprint - A printable character predicate.

        int
        isprint(string |int object, [int index])

    Macro Description:
        The 'isprint()' primitive determines whether the specified
        object 'object' belongs to the 'printable' character-class.

        The specified object can be an integer (whose ASCII code
        represents a single character), or a string, in which case
        the first character of the string is tested.

        The optional 'index' allows an alternative character within
        the string to be tested, with the first character represented
        by offset 'one'.

    Macro Parameters:
        object - Character or string object.

        [index] - If a string, an optional character index within the
            string, starting at offset one being the first character.
            if the index denotes a character out-of-bounds, the
            function returns 0.

    Macro Returns:
        The 'isprint()' primitive returns non-zero if 'object' is an
        printable character; otherwise it returns 0.

    Macro Example:

>       if (isprint(read(1)))
>           message("Next character is printable.");

    Macro Portability:
        The 'index' option is a Grief extension.

    Macro See Also:
        ctype
 */
void
do_isprint(void)                /* int (string str|int character) */
{
    isa(isprint);
}


/*  Function:           do_ispunct
 *      ispunct primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: ispunct - Punctuation character predicate.

        int
        ispunct(string |int object, [int index])

    Macro Description:
        The 'ispunct()' primitive determines whether the specified
        object 'object' belongs to the 'punctuation' character-class.

        Punctuation characters are ones that are printable (see
        isprint) character but neither alphanumeric (see isalnum) nor
        a space (see isspace).

        The specified object can be an integer (whose ASCII code
        represents a single character), or a string, in which case
        the first character of the string is tested.

        The optional 'index' allows an alternative character within
        the string to be tested, with the first character represented
        by offset 'one'.

    Macro Parameters:
        object - Character or string object.

        [index] - If a string, an optional character index within the
            string, starting at offset one being the first character.
            if the index denotes a character out-of-bounds, the
            function returns 0.

    Macro Returns:
        The 'ispunct()' primitive returns non-zero if 'object' is an
        alphanumeric character; otherwise it returns 0.

    Macro Example:

>       if (ispunct(read(1)))
>           message("Next character is alnum.");

    Macro Portability:
        The 'index' option is a Grief extension.

    Macro See Also:
        ctype
 */
void
do_ispunct(void)                /* int (string str|int character) */
{
    isa(ispunct);
}


/*  Function:           do_islower
 *      islower primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: islower - Lowercase character predicate.

        int
        islower(string |int object, [int index])

    Macro Description:
        The 'islower()' primitive determines whether the specified
        object 'object' belongs to the 'lower-case' character-classes.

        This is [a through z] in the program's current locale.

        The specified object can be an integer (whose ASCII code
        represents a single character), or a string, in which case
        the first character of the string is tested.

        The optional 'index' allows an alternative character within
        the string to be tested, with the first character represented
        by offset 'one'.

    Macro Parameters:
        object - Character or string object.

        [index] - If a string, an optional character index within the
            string, starting at offset one being the first character.
            if the index denotes a character out-of-bounds, the
            function returns 0.

    Macro Returns:
        The 'islower()' primitive returns non-zero if 'object' is an
        lower-case character; otherwise it returns 0.

    Macro Example:

>       if (islower(read(1)))
>           message("Next character is a lower-case character.");

    Macro Portability:
        The 'index' option is a Grief extension.

    Macro See Also:
        ctype
 */
void
do_islower(void)                /* int (string str|int character) */
{
    isa(islower);
}


/*  Function:           do_isupper
 *    isupper primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: isupper - Uppercase character predicate.

        int
        isupper(string |int object, [int index])

    Macro Description:
        The 'isupper()' primitive determines whether the specified
        object 'object' belongs to the 'uppercase' character-class.

        This is [A through Z] in the program's current locale.

        The specified object can be an integer (whose ASCII code
        represents a single character), or a string, in which case
        the first character of the string is tested.

        The optional 'index' allows an alternative character within
        the string to be tested, with the first character represented
        by offset 'one'.

    Macro Parameters:
        object - Character or string object.

        [index] - If a string, an optional character index within the
            string, starting at offset one being the first character.
            if the index denotes a character out-of-bounds, the
            function returns 0.

    Macro Returns:
        The 'isupper()' primitive returns non-zero if 'object' is an
        uppercase character; otherwise it returns 0.

    Macro Example:

>       if (isupper(read(1)))
>           message("Next character is an uppercase character.");

    Macro Portability:
        The 'index' option is a Grief extension.

    Macro See Also:
        ctype
 */
void
do_isupper(void)                /* int (string str|int character) */
{
    isa(isupper);
}


/*  Function:           do_isxdigit
 *      isxdigit primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: isxdigit - Hexadecimal character predicate.

        int
        isxdigit(string |int object, [int index])

    Macro Description:
        The 'isxdigit()' primitive determines whether the specified
        object 'object' belongs to the 'hexadecimal' character-class.

        This is [0 through 9], [A through f] and [a through f] in the
        program's current locale.

        The specified object can be an integer (whose ASCII code
        represents a single character), or a string, in which case
        the first character of the string is tested.

        The optional 'index' allows an alternative character within
        the string to be tested, with the first character represented
        by offset 'one'.

    Macro Parameters:
        object - Character or string object.

        [index] - If a string, an optional character index within the
            string, starting at offset one being the first character.
            if the index denotes a character out-of-bounds, the
            function returns 0.

    Macro Returns:
        The 'isxdigit()' primitive returns non-zero if 'object' is a
        hexadecimal character; otherwise it returns 0.

    Macro Example:

>       if (isxdigit(read(1)))
>           message("Next character is a hexadecimal character.");

    Macro Portability:
        The 'index' option is a Grief extension.

    Macro See Also:
        ctype
 */
void
do_isxdigit(void)               /* int (string str|int character) */
{
    isa(isxdigit);
}


/*  Function:           do_isspace
 *      isspace primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: isspace - Space character predicate.

        int
        isspace(string |int object, [int index])

    Macro Description:
        The 'isspace()' primitive determines whether the specified
        object 'object' belongs to the 'space' character-class.

        White-space characters are (space, form feed (\f), new-line
        (\n), carriage-return (\r), horizontal tab (\t) or vertical
        tab (\v).

        The specified object can be an integer (whose ASCII code
        represents a single character), or a string, in which case
        the first character of the string is tested.

        The optional 'index' allows an alternative character within
        the string to be tested, with the first character represented
        by offset 'one'.

    Macro Parameters:
        object - Character or string object.

        [index] - If a string, an optional character index within the
            string, starting at offset one being the first character.
            if the index denotes a character out-of-bounds, the
            function returns 0.

    Macro Returns:
        The 'issspace()' primitive returns non-zero if 'object' is an
        whitespacec character; otherwise it returns 0.

    Macro Example:

>       if (isspace(read(1)))
>           message("Next character is a space character.");

    Macro Portability:
        The 'index' option is a Grief extension.

    Macro See Also:
        ctype
 */
void
do_isspace(void)                /* (string str|int character) */
{
    isa(isspace);
}


/*  Function:           do_strcmp
 *      strcmp primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: strcmp - String compare.

        int
        strcmp(string s1, string s2, [ int length])

    Macro Description:
        The 'strcmp()' primitive shall compare the string 's1' to the
        string 's2' not ignoring case.

    Macro Parameters:
        s1 - First string.

        s2 - Second string to compare against.

        length - Optional, when specified only the first 'length'
            characters of both string shall be compared.

    Macro Return:
        strcmp() shall return an integer greater than, equal to, or
        less than 0, if the string pointed to by 's1' is greater than,
        equal to, or less than the string pointed to by 's2',
        respectively.

    Macro Portability:
        A Grief extension.

    Macro See Also:
        strcasecmp, <=>
 */
void
do_strcmp(void)                 /* int (string s1, string s2 [, int length]) */
{
    const char *s1 = get_str(1);
    const char *s2 = get_str(2);
    int len = get_xinteger(3, -1);

    if (len > 0) {
        acc_assign_int(strncmp(s1, s2, len));

    } else {
        acc_assign_int(strcmp(s1, s2));
    }
}


/*  Function:           do_strcasecmp
 *      strcasecmp primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: strcasecmp - String case insensitive compare.

        int
        strcasecmp(string s1, string s2, [int length])

    Macro Description:
        The 'strcmp()' primitive shall compare the string 's1' to the
        string 's2', ignoring case.

    Macro Parameters:
        s1 - First string.
        s2 - Second string to compare against.
        length - Optional, when specified only the first 'length'
            characters of both string shall be compared.

    Macro Return:
        strcasecmp() shall return an integer greater than, equal to,
        or less than 0, if the string pointed to by 's1' is greater
        than, equal to, or less than the string pointed to by 's2',
        respectively; when ignoring case.

    Macro Portability:
        A Grief extension.

    Macro See Also:
        strcmp, ==, <=>
 */
void
do_strcasecmp(void)             /* (string s1, string s2 [, int length]) */
{
    const char *s1 = get_str(1);
    const char *s2 = get_str(2);
    int len = get_xinteger(3, -1);

    if (len > 0) {
        acc_assign_int(str_nicmp(s1, s2, len));

    } else {
        acc_assign_int(str_icmp(s1, s2));
    }
}


/*  Function:           do_strverscmp
 *      strverscmp primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: strverscmp - Version string compare.

        int
        strverscmp(string s1, string s2)

    Macro Description:
        strverscmp(3)/versionsort(3) style version comparison function.

        Often one has files jan1, jan2, ..., jan9, jan10, ... and it
        feels wrong when ls orders them jan1, jan10, ..., jan2, ...,
        jan9. In order to rectify this, GNU introduced the -v option
        to ls(1), which is implemented using versionsort(3), which
        again uses strverscmp.

        Thus, the task of strverscmp is to compare two strings and
        find the "right" order, while strcmp only finds the
        lexicographic order.

    Macro Return:
        The 'strverscmp()' primitive returns an integer less than,
        equal to, or greater than zero if s1 is found, respectively,
        to be earlier than, equal to, or later than s2.

    Macro Portability:
        A Grief extension.

    Macro See Also:
        strcmp, strcasecmp, ==
 */
void
do_strverscmp(void)             /* (string s1, string s2) */
{
    const char *s1 = get_str(1);
    const char *s2 = get_str(2);

    acc_assign_int(str_verscmp(s1, s2));
}


/*  Function:           do_strpop
 *      strpop primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: strpop - Pop the leading character(s).

        string
        strpop(string str, [int length = 1])

    Macro Description:
        The 'strpop()' primitive is equivalent to substr(sym, 1,
        length) with the additional functionality that the returned
        character is removed from the specified string 'str'.

    Macro Portability:
        'length' is a Grief extension.

    Macro Return:
        String value containing the first character(s) of the
        original value of sym.

    Macro See Also:
        substr, characterat
 */
void
do_strpop(void)                 /* (string str, [int length = 1], [int encoding]) */
{
    SYMBOL *sp = get_symbol(1);
    int length = (int) get_xinteger(2, 1);
/*- int encoding = (int) get_xinteger(3, -1); -*/
    int val = FALSE;

    if (F_STR == sp->s_type && length > 0) {
        ref_t *rp = sp->s_obj;
        int used;

        if (rp && (used = r_used(rp)) > 0) {
            /*
             *  decode character value
             */
            if (length >= used) {
                length = used;                  /* trim to symbol length */
            }
            acc_assign_str((const char *)r_ptr(rp), length);
            val = TRUE;
            if (length > 0) {                   /* remove */
                if (1 == r_refs(rp)) {
                    r_pop(rp, length);
                } else {                        /* multiple references, copy */
                    sp->s_obj = r_nstring(((const char *)r_ptr(rp)) + length, r_used(rp) - length);
                    r_dec(rp);
                }
            }
        }
    }

    if (! val) {
        acc_assign_str("", 0);
    }
}


/*  Function:           do_strpstr
 *      strpstr primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: strpbrk - Search a string for any of a set of characters.

        int
        strpbrk(string str, string characters)

    Macro Description:
        The 'strpbrk()' primitive returns the index of the first
        character in 'str' which matches any character from the
        string 'characters'.

        This function is like 'index()' and 'rindex()' yet these only
        matches a single character string rather than a complete
        sub-string.

    Macro Return:
        Index of first matching character starting at the index one,
        otherwise zero if no match.

    Macro Portability:
        A Grief extension.

    Macro See Also:
        firstof, lastof
 */
void
do_strpbrk(void)                /* int (string str, string characters) */
{
    const char *pbrk, *str = get_str(1);
    const char *characters = get_str(2);
    accint_t position = 0;

    if (NULL != (pbrk = strpbrk(str, characters))) {
        position = (accint_t)((pbrk - str) + 1);
    }
    acc_assign_int(position);
}



/*  Function:           do_strstr
 *      strstr primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: strstr - Locate first occurrence of a sub-string.

        int
        strstr(string haystack, string needle)

    Macro Description:
        The 'strstr()' primitive finds the first occurrence of the
        sub-string 'needle' in the string 'haystack'.

    Macro Parameters:
        haystack - String object to be searched.
        needle - String to be matched.

    Macro Return:
        Index of first matching character starting at the index one,
        otherwise zero if no match.

    Macro Portability:
        A Grief extension.

    Macro See Also:
        strcasestr, strrstr, index, rindex
 */
void
do_strstr(void)                 /* int (string haystack, string needle) */
{
    const char *haystack = get_xstr(1);
    const char *needle = get_xstr(2);
    accint_t position = 0;

    if (haystack && needle) {
        const char *str;

        if (NULL != (str = strstr(haystack, needle))) {
            position = (accint_t)((str - haystack) + 1);
        }
    }
    acc_assign_int(position);
}



/*  Function:           do_strrstr
 *      strrstr primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: strrstr - Locate last occurrence of a sub-string.

        int
        strrstr(string haystack, string needle)

    Macro Description:
        The 'strrstr()' primitive finds the last occurrence of the
        sub-string 'needle' in the string 'haystack'.

    Macro Parameters:
        haystack - String object to be searched.
        needle - String to be matched.

    Macro Return:
        Index of last matching character starting at the index one,
        otherwise zero if no match.

    Macro Portability:
        A Grief extension.

    Macro See Also:
        strstr, index, rindex
 */
void
do_strrstr(void)                /* int (string haystack, string needle) */
{
    const char *haystack = get_xstr(1);
    const char *needle = get_xstr(2);
    accint_t position = 0;

    if (haystack && needle) {
        const char *str;

        if (NULL != (str = strstr(haystack, needle))) {
            position = (accint_t)((str - haystack) + 1);

                                /* FIXME - hack implementation */
            while (NULL != (str = strstr(str + 1, needle))) {
                position = (accint_t)((str - haystack) + 1);
            }
        }
    }
    acc_assign_int(position);
}


/*  Function:           do_strcasestr
 *      strcasestr primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: strcasestr - Locate first occurrence of a case insensitive
                            sub-string.

        int
        strcasestr(string haystack, string needle)

    Macro Description:
        The 'strcasestr()' primitive finds the first occurrence of the
        case insensitive sub-string 'needle' in the string 'haystack'.

    Macro Parameters:
        haystack - String object to be searched.
        needle - String to be matched.

    Macro Return:
        Index of first matching character starting at the index one,
        otherwise zero if no match.

    Macro Portability:
        A Grief extension.

    Macro See Also:
        strstr, strrstr, index, rindex
 */
void
do_strcasestr(void)             /* int (string haystack, string needle) */
{
    const char *haystack = get_xstr(1);
    const char *needle = get_xstr(2);
    accint_t position = 0;

    if (haystack && needle) {
        const char *str;

#if defined(HAVE_STRCASESTR)
        if (NULL != (str = strcasestr(haystack, needle))) {
#else
        if (NULL != (str = x_strcasestr(haystack, needle))) {
#endif
            position = (accint_t)((str - haystack) + 1);
        }
    }
    acc_assign_int(position);
}


#if !defined(HAVE_STRCASESTR)
static const char *
x_strcasestr(const char *haystack, const char *needle)
{
    const char *p, *startn = 0, *np = 0;

    for (p = haystack; *p; ++p) {
        if (np) {
            if (toupper(*p) == toupper(*np)) {
                if (!*++np) {
                    return startn;
                }
            } else {
                np = 0;
            }
        } else if (toupper(*p) == toupper(*needle)) {
            np = needle + 1;
            startn = p;
        }
    }
    return 0;
}
#endif  /*HAVE_STRCASESTR*/


/*  Function:           do_characterat
 *      characterat primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: characterat - Retrieve the character value within a string.

        int
        characterat(string str, int index)

    Macro Description:
        The 'characterat' function returns the character value within
        the string 'str' located at the specified position 'index'
        starting at offset one.

    Macro Parameters:
        str - String object to be searched.
        index - Character index, starting from on offset of 1 being
                    the first character.

    Macro Return:
        Character value as an integer, otherwise -1.

    Macro Portability:
        A Grief extension.
 */
void
do_characterat(void)            /* int (string str, int index) */
{
    const char *str = get_str(1);
    int length = get_strlen(1);
    accint_t position = get_xinteger(2, -1);
    accint_t val = -1;

    if (position > 0 && position <= length) {
        val = str[position - 1];                /* TODO/MCHAR??? */
    }
    acc_assign_int(val);
}


/*  Function:           string_mul
 *      Function to replicate a string 'n' times.
 *
 *  Returns:
 *      nothing
 */
void
string_mul(const char *str, int len, int multiple)
{
    char *nstr;
    int i;

    if (len <= 0 || multiple <= 0 ||
            NULL == (nstr = acc_expand((multiple * len) + 1))) {
        acc_assign_str("", 1);
        return;
    }

    for (i = 0; multiple-- > 0; i += len) {
        memcpy(nstr + i, str, len);
    }
    nstr[i] = '\0';

    acc_assign_strlen(i);
}
/*end*/
