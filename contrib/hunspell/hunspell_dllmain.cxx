/* -*- mode: c; indent-width: 4; -*- */
/* ==nonotice== 
 * hunspelldll main
 * This file is part of the GRIEF Editor and is placed in the public domain.
 *
 */

#include "hunspell.hxx"
#include <windows.h>

#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <cstdlib>
#include <cstdio>

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
