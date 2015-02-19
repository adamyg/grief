#include <edidentifier.h>
__CIDENT_RCSID(gr_mchar_util_c,"$Id: mchar_util.c,v 1.14 2014/11/16 17:28:43 ayoung Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: mchar_util.c,v 1.14 2014/11/16 17:28:43 ayoung Exp $
 * Locale/multibyte character utility functionality.
 *
 *
 * Copyright (c) 1998 - 2014, Adam Young.
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
#include <edenv.h>                              /* gputenvv(), ggetenv() */
#include <libstr.h>                             /* str_...()/sxprintf() */
#if defined(HAVE_LOCALE_H)
#include <locale.h>
#endif
#if defined(HAVE_LANGINFO_CODESET)
#include <langinfo.h>
#endif

#include <assert.h>
#include "mchar.h"                              /* mchar_...() */
#include "../libchartable/libchartable.h"


/*  Function:           mchar_ucs_width
 *      Retrieve the character width of the specified Unicode character.
 *
 *  Parameters:
 *      ch -                Character value.
 *
 *      bad -               Method of evaluating invalid character values.
 *
 *  Returns:
 *      Character width.
 */
int
mchar_ucs_width(int32_t ch, int bad)
{
    return charset_width_ucs(ch, bad);
}


int
mchar_ucs_encode(int32_t ch, char *buffer)
{
    return charset_utf8_encode(ch, buffer);
}


/*  Function:           mchar_locale_utf8
 *      Determine whether the specified locale supports utf-8.
 *
 *  Parameters:
 *      encoding -          Locale/codeset specification.
 *
 *  Returns:
 *      *true* or *false* as to whether the locale/encoding supports utf-8.
 */
int
mchar_locale_utf8(const char *encoding)
{
    if (encoding && *encoding) {
        const char *dot;

        if (0 == charset_compare("utf8", encoding, (int)strlen(encoding))) {
            return 1;                           /* <utf[-]8> */
        }

        while (NULL != (dot = strchr(encoding, '.'))) {
            encoding = dot + 1;                 /* <language[_territory]>.<utf[-]8>[@<modifier>] */
            if (0 == str_nicmp(dot, "utf8", 4) || 0 == str_nicmp(dot, "utf-8", 5)) {
                return 1;
            }
        }
    }
    return 0;
}


/*  Function:           sys_get_locale
 *      Return the locale in effect.
 *
 *  Parameters:
 *      isterminal -        Whether to check terminal otherwise text.
 *
 *  External References:
 *      GRTERM_LOCALE -     Terminal override.
 *
 *      XTERM_LOCALE -      xterm(1) exported locale.
 *
 *      GRTEXT_LANG -       Text locale override.
 *
 *      LC_ALL/LC_CTYPE -   setlocale() environmental settings.
 *
 *      LANG -              Language setting.
 *
 *  Returns:
 *      Returns the locale.
 */
const char *
sys_get_locale(int isterminal)
{
    const char *t_env, *env = NULL;

    if (isterminal) {
        if ((NULL != (env = ggetenv("GRTERM_LOCALE")) && *env) ||
                (NULL != (env = ggetenv("XTERM_LOCALE")) && *env)) {
            return env;
        }
    } else {
        if (NULL != (env = ggetenv("GRTEXT_LANG")) && *env) {
            return env;
        }
    }

#if defined(HAVE_LANGINFO_CODESET)
    if (NULL == env) {
        env = nl_langinfo(CODESET);
    }
#endif
#if defined(HAVE_LOCALE_H)
    if (NULL == env) {
        env = setlocale(LC_CTYPE, NULL);
    }
#endif

    if (NULL == env) {
        if ((NULL != (t_env = ggetenv("LC_ALL")) && *t_env) ||
                (NULL != (t_env = ggetenv("LC_CTYPE")) && *t_env) ||
                    (NULL != (t_env = ggetenv("LANG")) && *t_env)) {
            env = t_env;
        }
    }
    return env;
}


/*  Function:           sys_unicode_locale
 *      Check if we are running in a utf-8 locale.
 *
 *  Parameters:
 *      isterminal -        Whether to check terminal otherwise text.
 *
 *  Returns:
 *      *true* or *false* as to whether the locale supports utf-8.
 */
int
sys_unicode_locale(int isterminal)
{
    return mchar_locale_utf8(sys_get_locale(isterminal));
}
/*end*/





