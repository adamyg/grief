/* -*- mode: c; indent-width: 4; -*-
 * $Id: edpack1.h,v 1.5 2019/03/15 23:03:09 cvsuser Exp $
 * Structure packing.
 * ==noguard==
 *
 * Copyright (c) 1998 - 2019, Adam Young.
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
 *      #include <edpack1.h>
 *      struct __CPACKED_PRE__ mypackedstruct {
 *              :
 *      } __CPACKED_POST__;
 *      #include <edpack0.h>
 */

#ifndef __CPACKED_PRE__
#   if (defined(lint) || defined(_lint))
#       define __CPACKED_PRE__      /**/
#       define __CPACKED_POST__     /**/
#   elif ((defined(_MSC_VER) && _MSC_VER >= 800) || \
            defined(__BORLANDC__) || defined(__PARADIGM__))
#       define __CPACKED_PRE__      /**/
#       define __CPACKED_POST__     /**/
#   elif defined(__WATCOMC__)
#       define __CPACKED_PRE__      /**/
#       define __CPACKED_POST__     /**/
#   elif defined(__GNUC__)
#       define __CPACKED_PRE__      /**/
#       define __CPACKED_POST__     __attribute__((packed, aligned(1)))
#   elif defined(__SUNPRO_C) || defined(__SUNPRO_CC)
#       define __CPACKED_PRE__      /**/
#       define __CPACKED_POST__     /**/
#   elif defined(_MCC68K)
#       define __CPACKED_PRE__      packed
#       define __CPACKED_POST__     /**/
#   else
#error structure packing method unknown for this compiler ...
#   endif
#endif  /*__CPACKED_PRE__*/

#if !(defined(lint) || defined(_lint))
#   if defined(_MSC_VER) && (_MSC_VER >= 800)
#       if !defined(EDPACK1_H_INCLUDED_ONESHOT)
#           pragma warning(disable:4103)
#       endif
#       pragma pack(1)
#   elif defined(__WATCOMC__) || defined(__BORLANDC__) || defined(__PARADIGM__)
#       pragma pack(1)
#   endif
#   ifndef EDPACK1_H_INCLUDED_ONESHOT
#   define EDPACK1_H_INCLUDED_ONESHOT
#   endif
#endif

#if defined(EDPACK1_H_INCLUDED)
#error <edpack0.h> required ...
#endif
#define EDPACK1_H_INCLUDED

/*end*/
