#ifndef GR_EDFILEIO_H_INCLUDED
#define GR_EDFILEIO_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_edfileio_h,"$Id: edfileio.h,v 1.14 2018/10/04 01:27:59 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: edfileio.h,v 1.14 2018/10/04 01:27:59 cvsuser Exp $
 * File input/output functionality system api names.
 * Required by non-posix environments (ie. WIN32).
 *
 *
 *
 * Copyright (c) 1998 - 2018, Adam Young.
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

__CBEGIN_DECLS

#if defined(__CYGWIN__)
#define FILEIO_EXEEXT               ".exe"
#define FILEIO_DIRDELIM             ':'
#define FILEIO_PATHSEP              '/'
#define FILEIO_ISSEP(c)             ('\\' == (c) || '/' == (c))

#elif defined(DOSISH)
#define FILEIO_EXEEXT               ".exe"
#define FILEIO_DIRDELIM             ';'
#define FILEIO_PATHSEP              '\\'
#define FILEIO_ISSEP(c)             ('\\' == (c) || '/' == (c))

#else
#define FILEIO_DIRDELIM             ':'
#define FILEIO_PATHSEP              '/'
#define FILEIO_ISSEP(c)             ('/' == (c))
#endif

#if defined(WIN32)
/*
 *  MSVC and WATCOMC
 */
#define fileio_open(_fn, _of, _om)  _open(_fn, _of, _om)
#define fileio_close(_fd)           _close(_fd)
#define fileio_read(_fd, _ib, _is)  _read(_fd, _ib, _is)
#define fileio_write(_fd, _ob, _is) _write(_fd, _ob, _is)
#define fileio_lseek(_fd, _o, _w)   _lseek(_fd, _o, _w)

#define fileio_access(_fn, _m)      _access(_fn, _m)
#define fileio_chmod(_fn, _m)       _chmod(_fn, _m)
#define fileio_unlink(_fn)          _unlink(_fn)
#define fileio_mkdir(_dn, _m)       w32_mkdir(_dn, _m)

#define fileio_fdopen(_fd, _om)     _fdopen(_fd, _om)
#define fileio_fileno(_fs)          _fileno(_fs)
#define fileio_umask(_mk)           _umask(_mk)

#else
#define fileio_open(_fn, _of, _om)  open(_fn, _of, _om)
#define fileio_close(_fd)           close(_fd)
#define fileio_read(_fd, _ib, _is)  read(_fd, _ib, _is)
#define fileio_write(_fd, _ob, _is) write(_fd, _ob, _is)
#define fileio_lseek(_fd, _o, _w)   lseek(_fd, _o, _w)

#define fileio_access(_fn, _m)      access(_fn, _m)
#define fileio_chmod(_fn, _m)       chmod(_fn, _m)
#define fileio_unlink(_fn)          unlink(_fn)
#define fileio_mkdir(_dn, _m)       mkdir(_dn, _m)

#define fileio_fdopen(_fd, _om)     fdopen(_fd, _om)
#define fileio_fileno(_fs)          fileno(_fs)
#define fileio_umask(_mk)           umask(_mk)
#endif

__CEND_DECLS

#endif /*GR_EDFILEIO_H_INCLUDED*/
/*end*/
