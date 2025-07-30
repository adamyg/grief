#include <edidentifier.h>
__CIDENT_RCSID(gr_pty_win32_c,"$Id: pty_win32.c,v 1.24 2025/02/07 03:03:22 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: pty_win32.c,v 1.24 2025/02/07 03:03:22 cvsuser Exp $
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

#if defined(WIN32)                              /* WIN32 specific module */

#include <edfileio.h>
#include <edtermio.h>
#include <edalt.h>

#include <../libw32/win32_child.h>

#include "builtin.h"
#include "debug.h"
#include "echo.h"
#include "m_pty.h"
#include "procspawn.h"
#include "system.h"
#include "tty.h"

static int		dpcreate(DISPLAY_t *dp, const char *shell, const char *cwd);
static void		dpcleanup(DISPLAY_t *dp);


/*
 *  pty_send_signal ---
 *      Send signal to process group.
 */
int
pty_send_signal(int pid, int value)
{
    return sys_kill(-pid, value);               // not fully implemented
}


/*
 *  pty_send_term ---
 *      Send a terminate signal to the specified process.
 */
void
pty_send_term(int pid)
{
    sys_kill(pid, SIGTERM);
}


int
pty_died(BUFFER_t *bp)
{
    DISPLAY_t *dp = bp->b_display;

    proc_check();                               // handle any terminations
    if (dp) {
#if defined(_MSC_VER) && (_MSC_VER >= 1900)
#pragma warning(push)
#pragma warning(disable:4312) // 4312: 'type cast': conversion from 'pid_t' to 'HANDLE' of greater size
#endif
        HANDLE hProcess = (HANDLE)(bp->b_display->d_pid);
#if defined(_MSC_VER) && (_MSC_VER >= 1900)
#pragma warning(pop)
#endif

        if (WAIT_TIMEOUT == WaitForSingleObject(hProcess, 0)) {
            return FALSE;
        }
        pty_cleanup(bp);
    }
    return TRUE;
}


int
pty_connect(DISPLAY_t *dp, const char *shell, const char *cwd)
{
    if ((dp->d_pid = dpcreate(dp, shell, cwd)) < 0) {
        ewprintf("cannot create IPC.");
        return -1;                              // error
    }
    return 1;                                   // pty connected
}



/*
 *  pty_read ---
 *      Non-blocking PTY read ...
 */
int
pty_read(BUFFER_t *bp, char *buf, int count)
{
    HANDLE hPipe = bp->b_display->d_handle_in;
    DWORD cnt = 0;

    if (PeekNamedPipe(hPipe, NULL, 0, NULL, &cnt, NULL) && cnt <= 0) {
        return 0;                               // valid and no data
    }
    if (!ReadFile(hPipe, buf, count, &cnt, NULL)) {
        return -1;                              // read error
    }
    return (int)cnt;                            // .. count
}


static int
dpcreate(DISPLAY_t *dp, const char *shell, const char *cwd)
{
    HANDLE hInputRead = INVALID_HANDLE_VALUE,
        hInputWrite = INVALID_HANDLE_VALUE;
    SECURITY_ATTRIBUTES sa = {0};
    win32_spawn_t args = {0};
    const char *argv[3];
    int in, out, pid;

    __CUNUSED(cwd)

    //  Set up the security attributes struct.
    //
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.lpSecurityDescriptor = NULL;
    sa.bInheritHandle = TRUE;

    //  Create the child input pipe.
    //
    if (! CreatePipe(&hInputRead, &hInputWrite, &sa, 0)) {
        return -1;
    }

    //  Create the process
    //
    //  Note /k stop the CMD/COMMAND copyright notice.
    //
    (void)memset(&args, 0, sizeof(args));
    args.argv = argv;
    argv[0] = shell;
    argv[1] = (proc_shell_iscmd(shell) ? NULL : "/k");
    argv[2] = NULL;

    //  CREATE_NEW_CONSOLE
    //      The new process has a new console, instead of inheriting its parent's
    //      console (the default). For more information, see Creation of a Console.
    //
    //  DETACHED_PROCESS
    //      The new process is created without a console.
    //
    assert(sizeof(HANDLE) == sizeof(dp->d_handle_in));
    in = _open_osfhandle((intptr_t) hInputWrite, _O_NOINHERIT);
    if ((pid = w32_spawnA(&args, in, -1, &out)) <= 0) {
        CloseHandle(hInputRead);
        fileio_close(in);

    } else {
        dp->d_cleanup = dpcleanup;
        dp->d_pipe_out = out;                   // process stdin
        dp->d_handle_in = hInputRead;
    }
    return (pid <= 0 ? -1 : pid);
}


static void
dpcleanup(DISPLAY_t *dp)
{
    CloseHandle((HANDLE)dp->d_handle_in);
}

#endif  /*WIN32*/
