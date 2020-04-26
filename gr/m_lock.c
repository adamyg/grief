#include <edidentifier.h>
__CIDENT_RCSID(gr_m_lock_c,"$Id: m_lock.c,v 1.8 2020/04/21 00:01:56 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: m_lock.c,v 1.8 2020/04/21 00:01:56 cvsuser Exp $
 * lock primitives -- not implemented
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

#include <editor.h>

#include "accum.h"

//  LOCK_NONE           - 0 Disable locking.
//  LOCK_GRIEF/EMAC     - 1 Grisp/emacs style locks.
//  LOCK_OS             - 2 Keep files open and open files in shared-reading mode.
//
void
do_lock_info(void)                  /* void (int type], [int lock_type], [string lock_dir]) */
{
    /*TODO*/
    acc_assign_int(-1);
}

/*end*/
