#ifndef GR_ARGVWIN_H_INCLUDED
#define GR_ARGVWIN_H_INCLUDED
#include <edidentifier.h>
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 argument support.
 *
 * Copyright (c) 2024 - 2025 Adam Young.
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
 */
 
#include <edsym.h>

__CBEGIN_DECLS

extern char **win_GetUTF8Arguments (int *pargc);

__CEND_DECLS

#endif /*GR_ARGVWIN_H_INCLUDED*/
