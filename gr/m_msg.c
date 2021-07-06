#include <edidentifier.h>
__CIDENT_RCSID(gr_m_msg_c,"$Id: m_msg.c,v 1.31 2021/07/05 15:01:27 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: m_msg.c,v 1.31 2021/07/05 15:01:27 cvsuser Exp $
 * Message and formatting primitives.
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
#include <stdarg.h>

#include "m_msg.h"                              /* public interface */

#include "accum.h"                              /* acc_...() */
#include "debug.h"                              /* trace_...() */
#include "echo.h"                               /* x_pause_on_error */
#include "eval.h"
#include "main.h"
#include "prntf.h"                              /* print_formatted() */
#include "symbol.h"


/*  Function:           do_message
 *      message primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: message - Display a message on the command line.

        int
        message(string format, ...)

    Macro Description:
        The 'message()' primitive is used to display a message on the
        prompt line at the bottom of the screen. 'format' is a string
        and may contain printf-like % formatting characters. The
        following arguments 'format' and so forth are integer or
        string expressions used to satisfy the % formatting options.

        This primitive may be used to display informational messages
        on the bottom of the screen; if error messages are to be
        displayed then the <error> primitive macro should be used
        instead.

    Macro Parameters:
        format - String that contains the text to be written. It can
            optionally contain an embedded <Format Specification>
            that are replaced by the values specified in subsequent
            additional arguments and formatted as requested.

        ... - Optional format specific arguments, depending on the
            format string, the primitive may expect a sequence of
            additional arguments, each containing a value to be used
            to replace a format specifier in the format string.

            There should be at least as many of these arguments as
            the number of values specified in the format specifiers.
            Additional arguments are ignored by the primitive.

    Format Specification:
        A format specification, which consists of optional and
        required fields, has the following form:

>           %[FLAGS] [WIDTH] [.PRECISION] [PREFIXES] type

        Each field of the format specification is a single character
        or a number signifying a particular format option. The
        simplest format specification contains only the percent sign
        and a type character (for example, %s). If a percent sign is
        followed by a character that has no meaning as a format field,
        the character is simply copied. For example, to print a
        percent-sign character, use %%.

        The optional fields, which appear before the type character,
        control other aspects of the formatting, as follows:

        type - Required character that determines whether the
            associated argument is interpreted as a character, a
            string, or a number.

        flags - Optional character or characters that control
            justification of output and printing of signs, blanks,
            decimal points, and octal and hexadecimal prefixes. More
            than one flag can appear in a format specification.

        width - Optional number that specifies the minimum number of
            characters output.

        precision - Optional number that specifies the maximum number
            of characters printed for all or part of the output field,
            or the minimum number of digits printed for integer values.

        The following format types are supported;

(start table,format=nd)

    [Type       [Description                                            ]

 !  %d          Print an integer expression. Options such as %03d can
                be used to perform zero insertion and supply a field
                width.

 !  %i          Print an integer expression, same as %d.

 !  %u          Print an unsigned expression.

 !  %x          Print integer expression in hex.

 !  %t          Print using the symbol natural type (ie 'd' = integers
                and 's' = strings).

 !  %o          Print integer expression in octal.

 !  %s          Print a string. Field width and alignments are allowed.

 !  %c          Print a character.

 !  %p          If its less than a second since the last call to
                message (or error), then don't display the message.
                This is used by macros which want to print progress
                messages.

 !  %e          Double signed value having the form [-]d.dddd
                e[sign]ddd where d is a single decimal digit, dddd is
                one or more decimal digits, ddd is exactly three
                decimal digits, and sign is '+' or.

 !  %E          Double identical to the 'e' format except that 'E'
                rather than 'e' introduces the exponent.

 !  %f          Double signed value having the form [ ]dddd.dddd,
                where dddd is one or more decimal digits. The number
                of digits before the decimal point depends on the
                magnitude of the number, and the number of digits
                after the decimal point depends on the requested
                precision.

 !  %g          Double signed value printed in 'f' or 'e' format,
                whichever is more compact for the given value and
                precision. The e format is used only when the
                exponent of the value is less than 4 or greater than
                or equal to the precision argument. Trailing zeros
                are truncated, and the decimal point appears only if
                one or more digits follow it.

 !  %G          Double identical to the 'g' format, except that 'E',
                rather than 'e', introduces the exponent (where
                appropriate).

 !  %n          Number of characters successfully written so far to
                the stream or buffer; this value is stored in the
                integer variable given as the argument.

 !  %b          Print integer expression in binary.

 !  %B
                Print integer expression as a decoded binary, using
                the bit values as described within the secondary
                string parameter of the form;

                (code)
                  <base><arg>
                (end code)

                Where <base> is the output base expressed as a control
                character, e.g. \10 gives octal; \20 gives hex. Each
                arg is a sequence of characters, the first of which
                gives the bit number to be inspected (origin 1), and
                the next characters (up to a control character, i.e. a
                character <= 32), give the name of the register. Thus:
(end table)

    Flags::

        The following flags are supported.

(start table,format=nd)

    [Flag   [Description                            [Default            ]

 !  -
            Left align the result within the
            given field width.

                                                    Right align.

 !  +
            Prefix the output value with a
            sign (+ or -) if the output value
            is of a signed type.

                                                    Sign appears only
                                                    for negative signed
                                                    values ().

 !  0
            If width is prefixed with '0',
            zeros are added until the minimum
            width is reached. If '0' and
            appear, the '0' is ignored. If
            '0' is specified with an integer
            format ('i', 'u', 'x', 'X', 'o',
            'd') the 0 is ignored.

                                                    No padding.

 !  <space>
            Prefix the output value with a
            blank if the output value is
            signed and positive; the blank is
            ignored if both the blank and +
            flags appear.

                                                    No blank appears.

 !  #
            When used with the 'o', 'x', or
            'X' format, the '#' flag prefixes
            any nonzero output value with 0,
            '0x', or '0X', respectively.

                                                    No blank appears.

            When used with the 'e', 'E', or
            'f' format, the '#' flag forces
            the output value to contain a
            decimal point in all cases.

                                                    Decimal point appears
                                                    only if digits follow
                                                    it.

            When used with the 'g' or 'G'
            format, the '#' flag forces the
            output value to contain a decimal
            point in all cases and prevents
            the truncation of trailing zeros.

                                                    Decimal point appears
                                                    only if digits follow
                                                    it. Trailing zeros
                                                    are truncated.

            Ignored when used with 'c', 'd',
            'i', 'u', or 's'.
(end table)

    Prefixes::

        The following prefixes are supported.

(start table,format=nd)
    [Prefix     [Description                                            ]

 !  l           User long format for printing, e.g. floats are printed
                with full double-precision.

(end table)

    Width::

        The second optional field of the format specification is the
        width specification. The width argument is a nonnegative
        decimal integer controlling the minimum number of characters
        printed. If the number of characters in the output value is
        less than the specified width, blanks are added to the left
        or the right of the values depending on whether the flag (for
        left alignment) is specified until the minimum width is
        reached. If width is prefixed with 0, zeros are added until
        the minimum width is reached (not useful for left-aligned
        numbers).

        width specification never causes a value to be truncated. If
        the number of characters in the output value is greater than
        the specified width, or if width is not given, all characters
        of the value are printed (subject to the precision
        specification).

        If the width specification is an asterisk (*), an int
        argument from the argument list supplies the value. The width
        argument must precede the value being formatted in the
        argument list. A nonexistent or small field width does not
        cause the truncation of a field; if the result of a
        conversion is wider than the field width, the field expands
        to contain the conversion result.

    Precision::

        The third optional field of the format specification is the
        precision specification. It specifies a nonnegative decimal
        integer, preceded by a period (.), which specifies the number
        of characters to be printed, the number of decimal places, or
        the number of significant digits. Unlike the width
        specification, the precision specification can cause either
        truncation of the output value or rounding of a
        floating-point value. If precision is specified as 0 and the
        value to be converted is 0, the result is no characters
        output, as shown below:

>           "%.0d", 0   // characters output

        If the precision specification is an asterisk (*), an int
        argument from the argument list supplies the value. The
        precision argument must precede the value being formatted in
        the argument list.

        The type determines the interpretation of precision and the
        default when precision is omitted, as shown in the following
        table.

(start table,format=nd)

    [Type           [Description                    [Default                ]

!   c,C             Character is printed.

!   d,i,u,o,x,X
                    Minimum number of digits to be
                    printed. If the number of
                    digits in the argument is less
                    than precision, the output
                    value is padded on the left
                    with zeros. The value is not
                    truncated when the number of
                    digits exceeds precision.

                                                    Default precision is 1.

!   e,E
                    Number of digits to be printed
                    after the decimal point. The
                    last printed digit is rounded.

                                                    Default precision is 6;
                                                    if precision is 0 or
                                                    the period (.) appears
                                                    without a number
                                                    following it, no
                                                    decimal point is printed.

!   f
                    Number of digits after the
                    decimal point. If a decimal
                    point appears, at least one
                    digit appears before it. The
                    value is rounded to the
                    appropriate number of digits.

                                                    Default precision is 6;
                                                    if precision is 0, or
                                                    if the period (.)
                                                    appears without a
                                                    number following it, no
                                                    decimal point is printed.

!   g,G
                    Maximum number of significant
                    digits printed.

                                                    Six significant digits
                                                    are printed, with any
                                                    trailing zeros truncated.

!   s
                    Maximum number of characters to
                    be printed.

                                                    Characters in excess of
                                                    precision are not
                                                    printed. Characters are
                                                    printed until EOS is
                                                    encountered.
(end table)


    Macro Examples:

      String output::

>           message("See %s %s.", "spot", "run");

>           message("%pMacro completed: %d%%", (val*100)/total);

      Binary output::

>           message("val=%B", 3, "\\10\\2BITTWO\\1BITONE\\n")

        shall produce the output.

>           val=3<BITTWO,BITONE>


    Macro Return:
        The 'message()' primitive returns the number of characters
        printed.

    Macro Portablility:
        Many of the format options are Grief extensions, including '%b',
        '%n', '%t', '%B' and '*' support.

        The original implementation has a void declaration and
        returns nothing.

        The standard interface only permits upto 6 arguments, whereas
        Grief is unbound.

        CRisPEdit(tm) allows the second argument to be a list
        expression, which emulates a vsprintf() style of operation.
        In this case, the first element of the list is assumed to be
        a string format descriptor, and subsequent elements of the
        list are the arguments to print; this feature is not
        currently supported.

    Macro See Also:
        error, printf, sprintf
 */
void
do_message(void)                /* (string format, ...) */
{
    int len = -1;
    const char *cp = print_formatted(0, &len, NULL);

    if (cp && len) {
        infos(cp);
    }
    acc_assign_int(len);
}


/*  Function:           do_error
 *      error primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: error - Issue an error message on the command line.

        int
        error(string format, ...)

    Macro Description:
        The 'error()' primitive is used to display a message on the
        prompt line at the bottom of the screen. 'format' is a string
        and may contain printf-like % formatting characters. The
        following arguments 'format' and so forth are integer or
        string expressions used to satisfy the % formatting options.

        This primitive may be used to error messages on the bottom of
        the screen; if informational messages are required to be
        displayed then the <message> primitive macro should be used
        instead.

        Error messages are printed in the error color (see color) and
        truncated if they are too long.

        In addition, upon enabling <pause_on_error> then the error
        message is displayed suffixed with a '..' and Grief waits for
        the user to type any key to continue; useful during debugging.

        This function is one of a set of formatted output primitives
        each supporting a common feature set, see <message> for a
        complete discussion on the supported format specification
        syntax.

    Macro Parameters:
        format - String that contains the text to be written. It can
            optionally contain an embedded <Format Specification>
            that are replaced by the values specified in subsequent
            additional arguments and formatted as requested.

        ... - Optional format specific arguments, depending on the
            format string, the primitive may expect a sequence of
            additional arguments, each containing a value to be used
            to replace a format specifier in the format string.

            There should be at least as many of these arguments as
            the number of values specified in the format specifiers.
            Additional arguments are ignored by the primitive.

    Macro Returns:
        The 'message()' primitive returns the number of characters
        printed.

    Example:

>           message("File: %s", current_name);

    Macro Portability:
        Many of the format options are Grief extensions, including
        '%b', '%n', '%t', '%B' and '*' support.

        The original implementation has a void declaration and
        returns nothing.

        The standard interface only permits upto 6 arguments, whereas
        Grief is unbound.

    Macro See Also:
        message, pause_on_error
 */
void
do_error(void)                  /* int (string format, ...) */
{
    int len = -1;
    const char *cp = print_formatted(0, &len, NULL);

    if (cp && len) {
        errorf("%s", cp);
    }
    acc_assign_int(len);
}


/*  Function:           do_print
 *      print primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: print - Print formatted string to stdout.

        int
        print()

    Macro Description:
        The 'print()' primitive sends the contents of the currently
        marked area to the printer.

    Macro Parameters:
        none

    Macro Returns:
        0 for success, -1 for printer busy, less than -1 for other
        printer errors or no marked block.

    Macro Portability:
        n/a

    Macro See Also:
        error, message, dprintf
 */
void
do_print(void)                  /* int () */
{
    /*replacement, see macro implementation*/
    acc_assign_int(-1);
}


/*  Function:           do_printf
 *      printf primitive, print debugging messages on stdout.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: printf - Print formatted string to stdout.

        void
        printf(string format, ...)

    Macro Description:
        The 'printf()' primitive can be used for debugging macros,
        although its use is questionable. It exists for compatibility
        reasons with BRIEF. It causes the formatted string to be sent
        directly to stdout, bypassing all of Griefs internal screen
        manipulations.

        The format primitive is similar to the C printf() primitive,
        exporting the formatted result to stdout. Use of this
        interface is debatable as output is written to standard out,
        which is also the primary interface to the terminal/display.

        The 'printf()' primitive produces formatted output according
        to the format specification 'format' into the diagnostics
        log. The trailing arguments '...' are the integer, string or
        list expressions used to satisfy the % formatting options;
        refer to (see message) for details on the supported format
        specifications.

        This function is one of a set of formatted output primitives
        each supporting a common feature set, see <message> for a
        complete discussion on the supported format specification
        syntax.

    Macro Parameters:
        format - String containing the <Format Specification>.

        ... - Optional list of arguments which should correspond to
            the values required to satisfy the format specification.

    Macro Returns:
        nothing

    Macro Portability:
        The standard function has a void declaration and returns
        nothing.

    Macro See Also:
        error, message, dprintf
 */
void
do_printf(void)                 /* (string format, ...) */
{
    const char *cp = print_formatted(0, NULL, NULL);

    if (NULL == cp) {
        acc_assign_int(-1);
    } else {
        acc_assign_int((accint_t) printf("%s", cp));
        fflush(stdout);
    }
}


/*  Function:           do_dprintf
 *      dprintf primitive, print debugging messages on the log.
 *
 *      format_string is a C printf() string, containing the following set of
 *      %-sequences. [value1] ... are the optional parameters used to satisfy
 *      the %-escapes.
 *
 *      All parameters are evaluated, even if they are not needed.
 *
 *          %c          Print parameter as an ASCII character.
 *          %d          Print parameter in decimal.
 *          %x          Print parameter in hex.
 *          %s          Print argument as a string.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: dprintf - Formatted diagnostics output.

        int
        dprintf(string format, ...)

    Macro Description:
        This primitive is used to write a diagnostic messages to the
        trace log when debug is enabled.

        The 'dprintf()' primitive produces formatted output according
        to the format specification 'format' into the diagnostics
        log. The trailing arguments '...' are the integer, string or
        list expressions used to satisfy the % formatting options;
        refer to (see message) for details on the supported format
        specifications.

        The format primitive is similar to the C fprintf() primitive,
        exporting the formatted result to an external stream.

        This function is one of a set of formatted output primitives
        each supporting a common feature set, see <message> for a
        complete discussion on the supported format specification
        syntax.

    Macro Parameters:
        format - String that contains the text to be written. It can
            optionally contain an embedded <Format Specification>
            that are replaced by the values specified in subsequent
            additional arguments and formatted as requested.

        ... - Optional format specific arguments, depending on the
            format string, the primitive may expect a sequence of
            additional arguments, each containing a value to be used
            to replace a format specifier in the format string.

            There should be at least as many of these arguments as
            the number of values specified in the format specifiers.
            Additional arguments are ignored by the primitive.

    Macro Returns:
        The 'dprintf()' primitive returns the number of charaters stored
        within the result string 'buffer'.

    Macro Portability:
        This is a Grief extension.

    Macro See Also:
        debug, error, message
 */
void
do_dprintf(void)                /* (string format, ...) */
{
    const char *cp = print_formatted(0, NULL, NULL);

    if (NULL == cp) {
        acc_assign_int(-1);
    } else {
        acc_assign_int(trace_log("%s", cp));
    }
}


/*  Function:           do_sprintf
 *      sprintf primitive,
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: sprintf - Formatted printing to a string.

        int
        sprintf(string &buffer, string format, ...)

    Macro Description:
        The 'sprintf()' primitive produces formatted output according
        to the format specification 'format' into the given string
        'buffer'. The trailing arguments '...' are the integer,
        string or list expressions used to satisfy the % formatting
        options; refer to (see message) for details on the supported
        format specifications.

        The format primitive is similar to the C sprintf() primitive,
        exporting the formatted result into the destination buffer.

        This function is one of a set of formatted output primitives
        each supporting a common feature set, see <message> for a
        complete discussion on the supported format specification
        syntax.

    Macro Parameters:
        buffer - String which shall be populated with the result.

        format - String that contains the text to be written. It can
            optionally contain an embedded <Format Specification>
            that are replaced by the values specified in subsequent
            additional arguments and formatted as requested.

        ... - Optional format specific arguments, depending on the
            format string, the primitive may expect a sequence of
            additional arguments, each containing a value to be used
            to replace a format specifier in the format string.

            There should be at least as many of these arguments as
            the number of values specified in the format specifiers.
            Additional arguments are ignored by the primitive.

    Macro Returns:
        The 'sprintf()' primitive returns the number of charaters stored
        within the result string 'buffer'.

    Macro Portability:
        See <message>

    Macro See Also:
        format, error, message
 */
void
do_sprintf(void)                /* int (string buffer, string format, ...) */
{
    SYMBOL *sp = get_symbol(1);
    const char *cp;
    int len = 0, width = 0;

    cp = print_formatted(1, &len, &width);
    if (cp) {
        sym_assign_str(sp, cp);
    }
    acc_assign_int(width);
}


/*  Function:           do_format
 *      format primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: format - Formatted printing.

        string
        format(string format, ...)

    Macro Description:
        The 'format()' primitive produces formatted output according
        to the format specification 'format'. The trailing arguments
        '...' are the integer, string or list expressions used to
        satisfy the % formatting options; refer to (see message) for
        details on the supported format specifications.

        The format primitive is similar to the sprintf() primitive with
        the exception the formatted result is returned directly as a
        string.

        This function is one of a set of formatted output primitives
        each supporting a common feature set, see <message> for a
        complete discussion on the supported format specification syntax.

    Macro Parameters:
        format - String that contains the text to be written. It can
            optionally contain an embedded <Format Specification>
            that are replaced by the values specified in subsequent
            additional arguments and formatted as requested.

        ... - Optional format specific arguments, depending on the
            format string, the primitive may expect a sequence of
            additional arguments, each containing a value to be used
            to replace a format specifier in the format string.

            There should be at least as many of these arguments as
            the number of values specified in the format specifiers.
            Additional arguments are ignored by the primitive.

    Macro Returns:
        The 'format()' primitive returns a string containing the
        formatted results, as defined by the format specification and
        the values contained within the associated arguments.

    Macro Portability:
        See <message>

    Macro See Also:
        sprintf
 */
void
do_format(void)                 /* (string format, ...) */
{
    const char *cp;
    int len = 0;

    cp = print_formatted(0, &len, NULL);
    acc_assign_str(cp, len);
}


/*  Function:           do_pause_on_error
 *      pause_on_error primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: pause_on_error - Set the error display mode.

        int
        pause_on_error([int pause = NULL], [int echo = TRUE])

    Macro Description:
        The 'pause_on_error()' primitive controls the automatic pausing
        upon a macro error; by default the 'pause status' is 'off'.

        On the execution of the 'error' primitive plus the internal
        equivalent, the 'pause status' is checked and when enabled
        error messages shall be displayed with a '..' suffix and
        Grief shall await for the user to acknowledge the
        condition via a key-press before execution continues.

        If 'pause' is specified as a non-negative (>= 0) then the
        current setting is set to the pause value of the integer
        expression, whereas a negative value (-1) behaves like a
        inquiry primitive reporting the current value without change.
        Otherwise if omitted the current status is toggled.

        Unless 'echo' is stated as *FALSE*, upon modification of the
        pause state, the new status shall be echoed on the command as
        follows

>           Pausing errors on.

        or

>           Pausing errors off.


    Macro Parameters:
        pause - Optional int flag, when a non-negative value (>=0) it
            states the new pause status, a negative value (-1)
            reports the current value without change, otherwise if
            omitted the current pause state is toggled.

        echo - Optional int flag, when stated as *FALSE* the
            function is silent, otherwise the change in the pause
            state is echoed on the command prompt.

    Macro Returns:
        The 'pause_on_error' function returns the previous pause state.

    Macro Portability:
        'echo' is a Grief extension.

    Macro Example:
        The following examples enables error acknowledgement during the
        execution of the macro 'subshell()' and restores the previous
        status on completion.

>           int state = pause_on_error(TRUE);
>           subshell();
>           pause_on_error(status, FALSE);

    Macro See Also:
        error, message
 */
void
do_pause_on_error(void)         /* int ([int pause = NULL], [int echo = TRUE]) */
{
    const int old = x_pause_on_error;
    const int echo = get_xinteger(2, TRUE);
    const int toggled = !x_pause_on_error;
    int pause = (isa_undef(1) ? toggled : get_xinteger(1, toggled));

    if (pause >= 0) {
        x_pause_on_error = (pause ? 1 : 0);
        if (echo) {
            if (old != x_pause_on_error) {
                ewprintf("Pausing errors %s.", x_pause_on_error ? "on" : "off");
            }
        }
    }
    acc_assign_int(old);
}


/*  Function:           do_pause_on_message
 *      pause_on_message primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: pause_on_message - Set the message display mode.

        int
        pause_on_message([int pause = NULL], [int echo = TRUE])

    Macro Description:
        The 'pause_on_message()' primitive controls the automatic pausing
        upon a macro message; by default the 'pause status' is 'off'.

        On the execution of the 'message' primitive plus the internal
        equivalent, the 'pause status' is checked and when enabled
        error messages shall be displayed with a '..' suffix and Grief
        shall await for the user to acknowledge the condition via a
        key-press before execution continues.

        If 'pause' is specified as a non-negative (>= 0) then the
        current setting is set to the pause value of the integer
        expression, whereas a negative value (-1) behaves like a
        inquiry primitive reporting the current value without change.
        Otherwise if omitted the current status is toggled.

        Unless 'echo' is stated as *FALSE*, upon modification of the
        pause state, the new status shall be echoed on the command as
        follows

>           Pausing all messages on.

        or

>           Pausing all messages off.


    Macro Parameters:
        pause - Optional int flag, when a non-negative value (>=0) it
            states the new pause status, a negative value (-1)
            reports the current value without change, otherwise if
            omitted the current pause state is toggled.

        echo - Optional int flag, when stated as *FALSE* the
            function is silent, otherwise the change in the pause
            state is echoed on the command prompt.

    Macro Returns:
        The 'pause_on_message' function returns the previous pause state.

    Macro Portability:
        'echo' is a Grief extension.

    Macro Example:
        The following examples enables error acknowledgement during
        the execution of the macro 'subshell()' and restores the
        previous status on completion.

>           int state = pause_on_message(TRUE);
>           subshell();
>           pause_on_message(status, FALSE);

    Macro See Also:
        error, message
 */
void
do_pause_on_message(void)       /* int ([int pause = NULL], [int echo = TRUE]) */
{
    const int old = x_pause_on_message;
    const int echo = get_xinteger(2, TRUE);
    const int toggled = !x_pause_on_message;
    int pause = (isa_undef(1) ? toggled : get_xinteger(1, toggled));

    if (pause >= 0) {
        x_pause_on_message = (pause ? 1 : 0);
        if (echo) {
            if (old != x_pause_on_message) {
                ewprintf("Pausing all messages %s.", x_pause_on_message ? "on" : "off");
            }
        }
    }
    acc_assign_int(old);
}
/*end*/
