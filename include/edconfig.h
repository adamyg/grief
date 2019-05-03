#ifndef GR_EDCONFIG_H_INCLUDED
#define GR_EDCONFIG_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_edconfig_h,"$Id: edconfig.h,v 1.12 2019/03/15 23:03:05 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: edconfig.h,v 1.12 2019/03/15 23:03:05 cvsuser Exp $
 * Configuration.
 *
 *  GRINIT_FILE
 *      User level configuration.
 *
 *  GRRESTORE_FILE
 *      User level restore.
 *
 *  GRSTATE_FILE
 *      Directory specific state.
 *
 *  GRSTATE_DB
 *      State database, references GRSTATE_FILE images by directory.
 *
 *  GRLOG_FILE
 *      Diagnostics log file.
 *
 *  GRINIT_MACRO
 *      Initialisation macro function.
 *
 *  GRINIT_OBJECT
 *      Initialisation macro object, containing GRINIT_MACRO.
 *
 * Copyright (c) 1998 - 2019, Adam Young.
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

#include <edtypes.h>
#include <edcm.h>

#if defined(DOSISH) || defined(__OS2__)
#define GRINIT_FILE     "_grinit"
#define GRRESTORE_FILE  "_grief"
#define GRSTATE_FILE    "_grstate"
#define GRLOG_FILE      "grief.log"
#else
#define GRINIT_FILE     ".grinit"
#define GRRESTORE_FILE  ".grief"
#define GRSTATE_FILE    ".grstate"
#define GRLOG_FILE      ".grief.log"
#endif

#define GRDFA_PATTERN	"/tmp/%s.grdfa" 	/* syntax dfa cache/dump */

#define GRDUMP_MKSTEMP	"/tmp/griefscreen.XXX"	/* screen_dump() */
#define GRDUMP_DEFAULT  "/tmp/grief.scr"

#define GRPROFILE       ".grief"                /* profile subdirectory */

#define GRSTATE_DB      "grstatedb"             /* restore status database */

#define GRINIT_MACRO    "grief"                 /* startup macro */
#define GRINIT_OBJECT   __CSTRCAT("grief", CM_EXTENSION) /* and associated object */

#endif /*GR_EDCONFIG_H_INCLUDED*/
