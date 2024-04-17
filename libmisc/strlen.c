#include <edidentifier.h>
__CIDENT_RCSID(gr_strlen_c,"$Id: strlen.c,v 1.9 2024/04/17 15:57:14 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: strlen.c,v 1.9 2024/04/17 15:57:14 cvsuser Exp $
 * libstr - String length.
 *
 *
 * Copyright (c) 1998 - 2024, Adam Young.
 * All rights reserved.
 *
 * This file is part of the GRIEF Editor.
 *
 * The GRIEF Editor is free software: you can redistribute it
 * and/or modify it under the terms of the GRIEF Editor License.
 *
 * Redistributions of source code must retain the above copyright
 * notice, and must be distributed with the license document above.
 *
 * Redistributions in binary form must reproduce the above copyright
 * notice, and must include the license document above in
 * the documentation and/or other materials provided with the
 * distribution.
 *
 * The GRIEF Editor is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * License for more details.
 * ==end==
 */

#include <libstr.h>


size_t
strxlen(const char *str, size_t maxlen)
{
    const char *cp;

    for (cp = str; maxlen && *cp != '\0'; ++cp, --maxlen)
        ;
    return (size_t)(cp - str);
}
/*end*/
