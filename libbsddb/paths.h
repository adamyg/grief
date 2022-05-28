#ifndef LIBDBHASH_PATHS_H_INCLUDED
#define LIBDBHASH_PATHS_H_INCLUDED
/* $Id: paths.h,v 1.4 2022/05/26 13:36:07 cvsuser Exp $
 *
 * libbsddb <paths.h>
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

#include <edtypes.h>

#if defined(HAVE_PATHS_H)
#include <paths.h>
#endif

#if !defined(_PATH_TMP)
#define NEED_libbsddb_PATH_TMP
#define _PATH_TMP               libbsddb_PATH_TMP()

__CBEGIN_DECLS
const char *                    libbsddb_PATH_TMP(void);
__CEND_DECLS
#endif /*_PATH_TMP*/

#endif /*LIBDBHASH_PATHS_H_INCLUDED*/
