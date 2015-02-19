#ifndef GR_M_STRING_H_INCLUDED
#define GR_M_STRING_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_m_string_h,"$Id: m_string.h,v 1.13 2014/10/22 02:33:08 ayoung Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: m_string.h,v 1.13 2014/10/22 02:33:08 ayoung Exp $
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

#include <edsym.h>

__CBEGIN_DECLS

extern void                 do_atoi(void);
extern void                 do_characterat(void);
extern void                 do_compress(void);
extern void                 do_firstof(void);
extern void                 do_index(void);
extern void                 do_isalnum(void);
extern void                 do_isalpha(void);
extern void                 do_isascii(void);
extern void                 do_isblank(void);
extern void                 do_iscntrl(void);
extern void                 do_iscsym(void);
extern void                 do_isdigit(void);
extern void                 do_isgraph(void);
extern void                 do_islower(void);
extern void                 do_isprint(void);
extern void                 do_ispunct(void);
extern void                 do_isspace(void);
extern void                 do_isupper(void);
extern void                 do_isword(void);
extern void                 do_isxdigit(void);
extern void                 do_itoa(void);
extern void                 do_lastof(void);
extern void                 do_lower(void);
extern void                 do_ltrim(void);
extern void                 do_rindex(void);
extern void                 do_rtrim(void);
extern void                 do_strcasecmp(void);
extern void                 do_strcasestr(void);
extern void                 do_strcmp(void);
extern void                 do_string_count(void);
extern void                 do_strlen(void);
extern void                 do_strnlen(void);
extern void                 do_strpbrk(void);
extern void                 do_strpop(void);
extern void                 do_strrstr(void);
extern void                 do_strstr(void);
extern void                 do_strtod(void);
extern void                 do_strtof(void);
extern void                 do_strtol(void);
extern void                 do_strverscmp(void);
extern void                 do_substr(void);
extern void                 do_trim(void);
extern void                 do_upper(void);

extern void                 string_mul(const char *str, int len, int multipler);

__CEND_DECLS

#endif /*GR_M_STRING_H_INCLUDED*/
