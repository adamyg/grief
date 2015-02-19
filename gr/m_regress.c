#include <edidentifier.h>
__CIDENT_RCSID(gr_m_regress_c,"$Id: m_regress.c,v 1.14 2014/11/16 17:28:41 ayoung Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: m_regress.c,v 1.14 2014/11/16 17:28:41 ayoung Exp $
 * Regression test support macros.
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
#include <eddebug.h>
#include <edstacktrace.h>

#include "m_regress.h"

#include "accum.h"                              /* acc_...() */
#include "eval.h"                               /* get_...() */
#include "lisp.h"
#include "system.h"


/*  Function:       do_regress_replacement
 *      __regress_replacement primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: __regress_replacement - Replacment regression testing.

        declare
        __regress_replacement(...)

    Macro Description:
        The '__regress_replacement()' primitive is an *internal*
        function utilised by the regress macro to assert replacement
        macro features.

    Macro Returns:
        n/a

    Warning!:
        Direct use of __regress_replacement() is not advised and is
        only documented for completeness.

        Functionality may change or be removed without notice.

    Macro Portability:
        Grief specific, functionality may change without notice.

    Macro See Also:
        regress, __regress_op
*/
void
do_regress_replacement(void)
{
    const int argi = get_xinteger(1, -1);

    if (argi >= 1) {
        /*
         *  return the specified argument
         */
        const char *svalue = NULL;
        const LIST *lvalue = NULL;
        accfloat_t fvalue = 0;

        if (NULL != (svalue = get_xstr(argi))) {
            acc_assign_str(svalue, get_strlen(argi));

        } else if (NULL != (lvalue = get_xlist(argi))) {
            acc_assign_list(lvalue, get_listlen(argi));

        } else if (0.0 != (fvalue = get_xaccfloat(argi, 0.0))) {
            acc_assign_float(fvalue);

        } else {
            acc_assign_int(get_xaccint(argi, -1));
        }

    } else {
        acc_assign_null();
    }
}


/*  Function:       do_regress_op
 *      __regress_op primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: __regress_op - Regression operations.

        int 
        __regress_op(...)

    Macro Description:
        The '__regress_op()' primitive is an *INTERNAL* function utilised
        using system regression testing, providing access to
        miscellanous system library functions.

    Macro Returns:
        n/a

    Warning!:
        Direct use of __regress_op() is not advised and is only
        documented for completeness.

        Functionality may change or be removed without notice.

    Macro Portability:
        Grief specific, functionality may change without notice.

    Macro See Also:
        regress, __regress_op
*/
void
do_regress_op(void)
{
    const int type = get_xinteger(1, -1);
    double fvalue = 0;
    int i = 0;

    switch (type) {
    case 9999: {    //back-trace [output-name]
            const char *stream;
            FILE *out;

            if (NULL != (stream = get_xstr(2))) {
                out = fopen(stream, "w+");
            } else if (NULL == (out = trace_stream())) {
                out = stdout;
            }
            if (out) {
                edbt_stackdump(out, 1);
                if (stream) {
                    fclose(out);
                }
            }
        }
        break;

    case 9998:     //core [corename]
        sys_core(NULL, get_xstr(2), NULL);
        break;

    case 9997:     //div-by-zero
        fvalue /= fvalue;
        break;

    case 9996:     //div-by-zero
        i /= i;
        break;

    case 9995: {    //null write
            int *iptr = NULL;
            *iptr = 0;
        }
        break;

    case 9994: {    //null read
            int *iptr = NULL;
            i = *iptr;
        }
        break;
    default:
        break;
    }
}
/*end*/
