#ifndef GR_M_BACKUP_H_INCLUDED
#define GR_M_BACKUP_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_m_backup_h,"$Id: m_backup.h,v 1.8 2014/10/22 02:33:00 ayoung Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: m_backup.h,v 1.8 2014/10/22 02:33:00 ayoung Exp $
 * File backup option/configuration primitives.
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

extern const char *         bkcfg_dir(BUFFER_t *bp);
extern int                  bkcfg_versions(BUFFER_t *bp);
extern int                  bkcfg_oneext(void);
extern const char *         bkcfg_prefix(BUFFER_t *bp);
extern const char *         bkcfg_suffix(BUFFER_t *bp);
extern char *               bkcfg_askmsg(char *buf, size_t length, size_t size);
extern int                  bkcfg_ask(const char *fname);

extern void                 do_set_backup(void);
extern void                 do_set_backup_option(void);
extern void                 inq_backup(void);
extern void                 inq_backup_option(void);

__CEND_DECLS

#endif /*GR_M_BACKUP_H_INCLUDED*/
