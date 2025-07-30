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

#if defined(_WIN32)

#if !defined(_WIN32_WINNT)
#define _WIN32_WINNT 0x501
#endif

#include <config.h>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

#include <stdlib.h>
#include <assert.h>

#include "argvwin.h"

static int ArgumentSplit (char* cmd, char** argv, int cnt);


/**
 *  ArgumentSplit ---
 *      Split the command line, handling quoting and escapes.
 *
 *  Parameters:
 *      cmd  - Command line buffer; shall be modified.
 *      argv - Argument vector to be populated, otherwise NULL; only count is returned.
 *      cnt  - Argument limit, -1 unlimited.
 *
 *  Returns:
 *      Argument count.
 */

static int
ArgumentSplit(char *cmd, char **argv, int cnt)
{
    char *start, *end;
    int argc;

    if (cmd == NULL) {
        return -1;
    }

    argc = 0;
    for (;;) {
        // Skip white-space
        while (*cmd == ' '|| *cmd == '\t' || *cmd == '\n') {
            ++cmd;
        }

        if (! *cmd)
            break;

        // element start
        if ('\"' == *cmd || '\'' == *cmd) { // quoted argument
            char quote = *cmd++;

            start = end = cmd;
            for (;;) {
                const char ch = *cmd;

                if (0 == ch)
                    break; // eos

                if ('\n' == ch || ch == quote) {
                    ++cmd; // delimiter
                    break;
                }

                if ('\\' == ch) { // quote
                    if (cmd[1] == '\"' || cmd[1] == '\'' || cmd[1] == '\\') {
                        ++cmd;
                    }
                }

                if (argv) *end++ = *cmd;
                ++cmd;
            }

        } else {
            start = end = cmd;
            for (;;) {
                const char ch = *cmd;

                if (0 == ch)
                    break; // eos

                if ('\n' == ch || ' ' == ch || '\t' == ch) {
                    ++cmd; // delimiter
                    break;
                }

                if ('\\' == ch) { // quote
                    if (cmd[1] == '\"' || cmd[1] == '\'' || cmd[1] == '\\') {
                        ++cmd;
                    }
                }

                if (argv) *end++ = *cmd;
                ++cmd;
            }
        }

        // element completion
        if (NULL == argv || cnt > 0) {
            if (argv) {
                argv[ argc ] = start;
                *end = '\0';
            }
            ++argc;
            --cnt;
        }
    }

    if (argv && cnt) {
        argv[ argc ] = NULL;
    }
    return argc;
}


/**
 *  win_GetUTF8Arguments ---
 *      Generate a UTF8-8 encoding argument vector from the wide-char command-line.
 *
 *  Parameters:
 *      pargv = Buffer populated with the argument count.
 *
 *  Returns:
 *      Argument vector, otherwise NULL.
 */

char **
win_GetUTF8Arguments(int *pargc)
{
    const wchar_t *wcmdline;

    if (NULL == (wcmdline = GetCommandLineW()) || wcmdline[0] == 0) {
        // import application name; fully qualified path; on empty command-line
        unsigned pathsz = GetModuleFileNameW(NULL, NULL, 0); // length, excluding terminating null character.

        if (pathsz) {
            if (NULL != (wcmdline = calloc(pathsz + 1 /*NUL*/, sizeof(wchar_t)))) {
                GetModuleFileNameW(NULL, (wchar_t *)wcmdline, pathsz + 1 /*NUL*/);
            }
        }
    }

    if (NULL != wcmdline) {
        // split into arguments
        const unsigned wcmdsz = wcslen(wcmdline),
            cmdsz = WideCharToMultiByte(CP_UTF8, 0, wcmdline, (int)wcmdsz, NULL, 0, NULL, NULL);
        char *cmd;

        if (NULL != (cmd = calloc(cmdsz + 1 /*NUL*/, sizeof(char)))) {
            char **argv = NULL;
            char *t_cmd;
            int argc;

            WideCharToMultiByte(CP_UTF8, 0, wcmdline, (int)wcmdsz, cmd, cmdsz + 1, NULL, NULL);

            if (NULL != (t_cmd = _strdup(cmd))) { // temporary working copy.
                if ((argc = ArgumentSplit(t_cmd, NULL, -1)) > 0) { // argument count.
                    if (NULL != (argv = calloc(argc + 1, sizeof(char *)))) {
                        ArgumentSplit(cmd, argv, argc + 1); // populate arguments.
                        if (pargc) *pargc = argc;
                        free(t_cmd);
                        return argv;
                    }
                }
                free(t_cmd);
            }
        }
        free(cmd);
    }

    if (pargc) *pargc = 0;
    return NULL;
}

#else  //_WIN32

extern void argvwin_linkage(void);

void
argvwin_linkage(void)
{
}

#endif //_WIN32

//end
