/* -*- mode: c; indent-width: 4; -*- */
/* $Id: dllmain.c,v 1.1 2014/07/08 22:50:18 ayoung Exp $
 *
 * libtre - dllmain
 *
 * Copyright (c) 2013-2014 Adam Young.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 * ==end==
 */

#include <config.h>
#include <windows.h>

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

BOOL APIENTRY
DllMain(HINSTANCE hInst, DWORD reason, LPVOID reserved)
{
    switch (reason) {
    case DLL_PROCESS_ATTACH:
        break;
    case DLL_PROCESS_DETACH:
        break;
    case DLL_THREAD_ATTACH:
        break;
    case DLL_THREAD_DETACH:
        break;
    }
    return TRUE;
}


#if !defined(isblank)
#if !defined(HAVE_ISBLANK)
/*
NAME
    isblank - test for a blank character.

SYNOPSIS
    #include <ctype.h>
    int isblank(int c);

DESCRIPTION
    The functionality described on this reference page is aligned with the ISO C
    standard. Any conflict between the requirements described here and the ISO C
    standard is unintentional.

    This volume of IEEE Std 1003.1-2001 defers to the ISO C standard.

    The isblank() function shall test whether c is a character of class blank in the
    program's current locale; see the Base Definitions volume of IEEE Std 1003.1-2001,
    Chapter 7, Locale.

    The c argument is a type int, the value of which the application shall ensure is a
    character representable as an unsigned char or equal to the value of the macro EOF.
    If the argument has any other value, the behavior is undefined.

RETURN VALUE
    The isblank() function shall return non-zero if c is a <blank>; otherwise, it shall
    return 0.

ERRORS
    No errors are defined.
*/
int
isblank(int ch)
{
    return (' ' == ch || '\t' == ch);
}
#endif
#endif
/*end*/
