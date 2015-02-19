/* -*- mode: c; indent-width: 4; -*-
 * $Id: edpack0.h,v 1.3 2015/02/19 00:16:56 ayoung Exp $
 * Structure packing.
 * ==noguard==
 *
 * Copyright (c) 1998 - 2015, Adam Young.
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
 *
 */

#if !defined(EDPACK1_H_INCLUDED)
#error <edpack1.h> required ...
#endif
#undef EDPACK1_H_INCLUDED

#if !(defined(lint) || defined(_lint))
#   if defined(_MSC_VER) && (_MSC_VER >= 800)
#       pragma pack()
#   elif defined(__WATCOMC__) || defined(__BORLANDC__) || defined(__PARADIGM__)
#       pragma pack()
#   endif
#endif

/*end*/
