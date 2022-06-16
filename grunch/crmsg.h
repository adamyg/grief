#ifndef GR_CRMSG_H_INCLUDED
#define GR_CRMSG_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_crmsg_h,"$Id: crmsg.h,v 1.9 2022/05/27 03:13:33 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: crmsg.h,v 1.9 2022/05/27 03:13:33 cvsuser Exp $
 * Compiler messages.
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

#include <edtypes.h>

__CBEGIN_DECLS

enum {
    RC_ERROR = 1,

    RC_SYNTAX_ERROR,
    RC_SYNTAX_TOKEN,

    RC_NUL_CHARACTER,

    RC_CHARACTER_EMPTY,                 // "empty character constant"
    RC_CHARACTER_UNTERMINATED,          // "unterminated character constant"
    RC_CHARACTER_NEWLINE,               // "newline in character constant"
    RC_CHARACTER_ESCAPE,                // "unknown escape sequence '<character-code>/<character-value>'"
    RC_CHARACTER_CONTROL,               // "invalid control character"
    RC_CONTROL_NEWLINE,                 // "newline within control sequence constant"
    RC_CHARACTER_OCTAL,
    RC_OCTAL_NEWLINE,
    RC_CHARACTER_HEX,                   // "invalid hex constant"
    RC_CHARACTER_WIDE,                  // "character constant too long"
    RC_HEX_NEWLINE,                     // "newline within hexidecimal escape constant"
    RC_CHARACTER_MULTI,                 // "multi-character character constant"
    RC_UNICODE_SHORT,                   // "short unicode constant"
    RC_UNICODE_INVALID,                 // "invalid unicode constant"

    RC_STRING_UNTERMINATED,             // "unterminated string constant"
    RC_STRING_NEWLINE,                  // "newline within string constant"
    RC_STRING_LENGTH,                   // "string length exceeds 32k"

    RC_TYPE_STORAGEDUP,                 // "more then one storage class specified"
    RC_TYPE_QUALDUP,                    // "type qualifier specified more then once"
    RC_TYPE_SIGNDUP,                    // "duplicate 'sign'"
    RC_TYPE_SIGNMIXED,                  // "mix of both signed and unsigned specified"
    RC_TYPE_DUPLICATE,                  // "data type 'type' specified more then once"
    RC_TYPE_MULTIPLE,                   // "two data types in declaration"
    RC_TYPE_MODDUP,                     // "more then one function modifier specified"
    RC_TYPE_VOID,                       // "invalid use of void type"
    RC_TYPE_INVALID,                    // "'type' cannot be used against 'type'"

    RC_MAIN_VOID,                       // "'main' expected to be a 'void' function."
    RC_INIT_RESERVED,                   // "'_init' is a reserved function name."

    RC_REPLACING_BUILTIN,               // "implicit replacement of the builtin function '<function>'
    RC_REPLACING_MACRO,                 // "replacing non-builtin macro <function>"
    
    RC_VARARG_MULTIPLE,                 // "multiple '...' operators for function <function>"

    RC_UNSUPPORTED_POINTER,
    RC_UNSUPPORTED_REFGLOBOL,
    RC_UNSUPPORTED_REFVAR,

    RC_UNSUPPORTED_REFFUNC,
    RC_UNSUPPORTED_REFDEFAULT,

    RC_UNUSED,

    RC_MAX = 999
};

typedef enum {
    ONCEONLY_TAGPARAMETERSCOPE,
    ONCEONLY_NUL_CHARACTER
} cronceonly_t;

void                yymsginit(void);

void                yyerror(const char *str);
void                yyerrorf(const char *str, ...) __ATTRIBUTE_FORMAT__((printf, 1, 2));
void                yywarning(const char *str);
void                yywarningf(const char *str, ...) __ATTRIBUTE_FORMAT__((printf, 1, 2));

unsigned            yyerrors(void);
unsigned            yywarnings(void);

void                xprintf(const char *, ...) __ATTRIBUTE_FORMAT__((printf, 1, 2));

int                 cronceonly(cronceonly_t);

void                crmessage(int msgno, int enable);

void                crerrorv(int, const char *, va_list ap) __ATTRIBUTE_FORMAT__((printf, 2, 0));
void                crerrorv_line(int, int, const char *, va_list ap) __ATTRIBUTE_FORMAT__((printf, 3, 0));
void                crerrorx(int, const char *, ...) __ATTRIBUTE_FORMAT__((printf, 2, 3));
void                crerrorx_line(int, int, const char *, ...) __ATTRIBUTE_FORMAT__((printf, 3, 4));
void                crerror(int, const char *);
void                crerror_line(int, int, const char *);

void                crwarnv(int, const char *, va_list ap) __ATTRIBUTE_FORMAT__((printf, 2, 0));
void                crwarnv_line(int, int, const char *, va_list ap) __ATTRIBUTE_FORMAT__((printf, 3, 0));
void                crwarnx(int, const char *, ...) __ATTRIBUTE_FORMAT__((printf, 2, 3));
void                crwarnx_line(int, int, const char *, ...) __ATTRIBUTE_FORMAT__((printf, 3, 4));
void                crwarn(int, const char *);
void                crwarn_line(int, int, const char *);

__CEND_DECLS

#endif /*GR_CRMSG_H_INCLUDED*/
