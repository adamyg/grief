#ifndef GR_M_FILEIO_H_INCLUDED
#define GR_M_FILEIO_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_m_fileio_h,"$Id: m_fileio.h,v 1.8 2014/10/22 02:33:03 ayoung Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: m_fileio.h,v 1.8 2014/10/22 02:33:03 ayoung Exp $
 * File i/o primitives.
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

extern void                 do_fclose(void);
extern void                 do_feof(void);
extern void                 do_ferror(void);
extern void                 do_fflush(void);
extern void                 do_fioctl(void);
extern void                 do_flock(void);
extern void                 do_fmktemp(void);
extern void                 do_fopen(void);
extern void                 do_fread(void);
extern void                 do_fseek(void);
extern void                 do_fstat(void);
extern void                 do_ftell(void);
extern void                 do_ftruncate(void);
extern void                 do_fwrite(void);

__CEND_DECLS

#endif /*GR_M_FILEIO_H_INCLUDED*/
