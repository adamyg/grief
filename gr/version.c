#include <edidentifier.h>
__CIDENT_RCSID(gr_version_c,"$Id: version.c,v 1.34 2025/07/03 04:53:32 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: version.c,v 1.34 2025/07/03 04:53:32 cvsuser Exp $
 * Version strings.
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

#include <edbuildinfo.h>

const int               x_major_version = GR_VERSION_1;
const int               x_minor_version = GR_VERSION_2;
const int               x_edit_version  = GR_VERSION_3;
const int               x_build_version = GR_VERSION_4;

const char *            x_version       = GR_VERSION;
const char *            x_buildnumber   = GR_BUILD_NUMBER;
const char *            x_compiled      = __DATE__ " " __TIME__;
const char *            x_copyright     = "(C) 1998 - " GR_BUILD_YEAR " A. Young, 1991 P. Fox";

/*end*/
