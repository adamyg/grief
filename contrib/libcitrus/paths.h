#ifndef _CITRUS_PATHS_H_INCLUDED_
#define _CITRUS_PATHS_H_INCLUDED_

/* $Id: paths.h,v 1.4 2015/03/01 02:56:37 cvsuser Exp $
 *
 * libcitrus <paths.h>
 *
 * Copyright (c) 2012-2015 Adam Young.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 * ==end==
 */

#include <sys/cdefs.h>
#include "namespace.h"

#define __LEADING_UNDERSCORE

#ifdef  __LEADING_UNDERSCORE
#define _C_LABEL(x)             __CONCAT(_,x)
#define _C_LABEL_STRING(x)      "_"x
#else
#define _C_LABEL(x)             x
#define _C_LABEL_STRING(x)      x
#endif

#define _DEF_PATH_ICONV         "/usr/share/i18n/iconv"
#define _DEF_PATH_ESDB          "/usr/share/i18n/esdb"
#define _DEF_PATH_CSMAPPER      "/usr/share/i18n/csmapper"
#define _DEF_PATH_I18NMODULE    "/usr/lib/i18n/module"

#define _PATH_ICONV             __citrus_PATH_ICONV()
#define _PATH_ESDB              __citrus_PATH_ESDB(NULL)
#define _PATH_CSMAPPER          __citrus_PATH_CSMAPPER(NULL)
#define _PATH_I18NMODULE        __citrus_PATH_I18NMODULE()

__BEGIN_DECLS
const char *                    __citrus_PATH_ICONV(void);
const char *                    __citrus_PATH_ESDB(const char *subdir);
const char *                    __citrus_PATH_CSMAPPER(const char *subdir);
const char *                    __citrus_PATH_I18NMODULE(void);
__END_DECLS

#endif /*_CITRUS_PATHS_H_INCLUDED_*/


