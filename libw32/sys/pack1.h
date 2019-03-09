/* -*- mode: c; indent-width: 4; -*-
 * $Id: pack1.h,v 1.7 2018/09/29 02:25:24 cvsuser Exp $
 * ==noguard==
 *
 * win32 declaration helpers
 *
 * Copyright (c) 1998 - 2018, Adam Young.
 * All rights reserved.
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
 *
 * Usage:
 *
 *      #include <sys/cdefs.h>
 *
 *      #include <sys/pack1.h>
 *      struct __packed_pre__ mypackedstruct {
 *              :
 *      } __packed_post__;
 *      #include <sys/pack0.h>
 *
 */

#include <sys/cdefs.h>                          /* __packed_pre__ and __packed_post__ */

#if !(defined(lint) || defined(_lint))
#   if defined(_MSC_VER) && (_MSC_VER >= 800)
#       ifndef __SYS_PACK1_H_INCLUDED__
#           pragma warning(disable:4103)
#       endif
#       pragma pack(1)
#   elif defined(__WATCOMC__) || defined(__BORLANDC__) || defined(__PARADIGM__)
#       pragma pack(1)
#   endif
#endif
#define __SYS_PACK1_H_INCLUDED__
