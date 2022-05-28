#include <edidentifier.h>
__CIDENT_RCSID(gr_sh_win32_c,"$Id: sh_win32.c,v 1.27 2022/05/26 16:39:50 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: sh_win32.c,v 1.27 2022/05/26 16:39:50 cvsuser Exp $
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

#if defined(WIN32)

#include <edfileio.h>
#include <edenv.h>                              /* gputenvv(), ggetenv() */

#include <../libw32/win32_child.h>

#include "accum.h"
#include "builtin.h"
#include "debug.h"
#include "display.h"
#include "echo.h"
#include "eval.h"
#include "getkey.h"
#include "main.h"
#include "mouse.h"
#include "procspawn.h"
#include "system.h"
#include "tty.h"

#define SLASHCHAR       '\\'
#define XSLASHCHAR      '/'
#define SLASH           "\\"
#define DELIMITER       ";"

#define ISSLASH(c)      (((c) == SLASHCHAR) || ((c) == XSLASHCHAR))

struct procdata {
    int                 type;
    DWORD               dwProcessId;
    HANDLE              hInput;
    HANDLE              hOutput;
    HANDLE              hError;
};

extern int              w32_errno_set(void);

static const char *     OutDirect(const char *path, int *append);
static int              Dup(HANDLE old, HANDLE *dup, BOOL inherit);
static void             ShellCleanup(void *p);
static void             Close(HANDLE handle);
static BOOL             SendCloseMessage(HANDLE hProc);


/*
 *  sys_shell ---
 *      System specfic shell interface.
 */
int
sys_shell(const char *cmd, const char *macro,
            const char *fstdin, const char *fstdout, const char *fstderr, int mode)
{
    static const char * sharg[] = {             // shell arguments
            "/C",       // command
            "/k"        // interactive
            };
    char *slash, *shname = chk_salloc(proc_shell_get());
    int xstdout = FALSE, xstderr = FALSE;       // mode (TRUE == append)
    struct procdata pd =
            {0, 0, INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE};
    SECURITY_ATTRIBUTES sa = {0};
    win32_spawn_t args = {0};
    const char *argv[4] = {0};
    int interactive = 0;
    int status = 0;
    HANDLE hProc;

     __CUNUSED(mode)

    // sync or async
    sa.nLength = sizeof(sa);
    sa.lpSecurityDescriptor = NULL;
    sa.bInheritHandle = TRUE;                   // inherited

    // redirection
    if (fstdin) {                               // O_RDONLY
        pd.hInput = CreateFileA(fstdin, GENERIC_READ,
                            0, &sa, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    }

    if (NULL != (fstdout = OutDirect(fstdout, &xstdout))) {
        if (! xstdout)  {                       // O_RDWR|O_CREAT|O_TRUNC
            pd.hOutput = CreateFileA(fstdout, GENERIC_READ | GENERIC_WRITE,
                                0, &sa, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

        } else {                                // O_RDWR|O_CREAT|O_APPEND
            pd.hOutput = CreateFileA(fstdout, GENERIC_READ | GENERIC_WRITE | FILE_APPEND_DATA,
                                0, &sa, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        }
    }

    if (NULL != (fstderr = OutDirect(fstderr, &xstderr))) {
        if (! xstderr)  {                       // O_RDWR|O_CREAT|O_TRUNC
            pd.hError = CreateFileA(fstderr, GENERIC_READ | GENERIC_WRITE,
                                0, &sa, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

        } else {                                // O_RDWR|O_CREAT|O_APPEND
            pd.hError = CreateFileA(fstderr, GENERIC_READ | GENERIC_WRITE | FILE_APPEND_DATA,
                                0, &sa, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        }

    } else if (fstdout) {
        if (! xstdout)  {                       // O_RDWR|O_CREAT|O_TRUNC
            pd.hError = CreateFileA(fstdout, GENERIC_READ | GENERIC_WRITE,
                                0, &sa, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

        } else {                                // O_RDWR|O_CREAT|O_APPEND
            pd.hError = CreateFileA(fstdout, GENERIC_READ | GENERIC_WRITE | FILE_APPEND_DATA,
                                0, &sa, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        }
    }

    if (pd.hInput == INVALID_HANDLE_VALUE) {    // stdin
        if (! Dup(GetStdHandle(STD_INPUT_HANDLE), &pd.hInput, TRUE)) {
            goto error;
        }
    }

    if (pd.hOutput == INVALID_HANDLE_VALUE) {   // stdout
        if (! Dup(GetStdHandle(STD_OUTPUT_HANDLE), &pd.hOutput, TRUE)) {
            goto error;
        }
    }

    if (pd.hError == INVALID_HANDLE_VALUE) {    // stderr
        if (! Dup(GetStdHandle(STD_ERROR_HANDLE), &pd.hError, TRUE)) {
            goto error;
        }
    }

    // command or interactive
    if (proc_shell_iscmd(shname)) {
        slash = shname - 1;
        while ((slash = strchr(slash + 1, XSLASHCHAR)) != NULL) {
            *slash = SLASHCHAR;                 // convert slashes
        }
    }

    if (NULL == cmd || !*cmd) {
        ++interactive;                          // interactive if no cmd
    }

    // create child process
    (void)memset(&args, 0, sizeof(args));
    argv[0] = shname;
    argv[1] = sharg[ interactive ];
    argv[2] = cmd;
    argv[3] = NULL;

    args.argv = argv;                           // create the process
    if ((hProc = w32_child_execA(&args, pd.hInput, pd.hOutput, pd.hError)) == 0) {
        ShellCleanup((void *)&pd);
        status = -1;

    } else {
        pd.dwProcessId = args._dwProcessId;     // See SendCloseMessage()
        proc_add((int)hProc, macro, ShellCleanup, (const void *)&pd, sizeof(pd));
        if (NULL == macro) {
            status = proc_wait((int)hProc);     // blocking
        }
    }

    chk_free(shname);
    return status;

error:
    w32_errno_set();
    ShellCleanup((void *)&pd);
    return -1;
}


/*
 *  remove output redirection operators
 */
static const char *
OutDirect(const char *path, int *append)
{
    *append = FALSE;
    if (path) {
        if ('>' == *path) {                     // ">name"
            ++path;
            if ('>' == *path) {                 // ">>name"
                *append = TRUE;
                ++path;
            }
        }
    }
    return path;
}


/*
 *  duplicate a file handle.
 */
static int
Dup(HANDLE old, HANDLE *dup, BOOL inherit)
{
    HANDLE self = GetCurrentProcess();

    if (dup == NULL || old == INVALID_HANDLE_VALUE ||
            !DuplicateHandle(
                self, old, self, dup, 0, inherit, DUPLICATE_SAME_ACCESS) ) {
        if (dup) *dup = INVALID_HANDLE_VALUE;
        return FALSE;
    }
    return TRUE;
}


/*
 *  sub-process cleanup callback
 */
static void
ShellCleanup(void *p)
{
    struct procdata *pd = (struct procdata *)p;

    Close(pd->hInput);
    Close(pd->hOutput);
    Close(pd->hError);
}



/*
 *  guarded handle close
 */
static void
Close(HANDLE handle)
{
    if (handle != INVALID_HANDLE_VALUE) {
        CloseHandle(handle);
    }
}


/*
 *  sys_waitpid ---
 *      The function waitpid() suspends the calling-process until the specified
 *      child-process terminates.
 *
 *      If the specifed child-process terminated prior to the call to wait(),
 *      return is immediate.
 */
struct _enum_proc_info {
    int         pid;
    int *       status;
};


static int
enum_proc_callback(int pid, void *u)
{
    struct _enum_proc_info *info = (struct _enum_proc_info *)u;

    /*
     *  Callback to proc_enum ...
     *
     *  Return Value:
     *      To continue enumeration, the callback function must
     *      return TRUE; to stop enumeration, it must return FALSE.
     */
    info->pid = pid;
    return w32_child_wait((HANDLE)pid, info->status, TRUE);
}


int
sys_waitpid(int pid, int *status, int options)
{
    int ret = -1;

    if (pid == -1) {
        /*
         *  wait for any child process ...
         */
        struct _enum_proc_info info;

        info.status = status;                   // walk process list
        if (proc_enum(enum_proc_callback, &info)) {
            ret = info.pid;
        }

    } else if (pid > 0) {
        /*
         *  wait for the child whose process ID is equal to the value of pid.
         */
        return w32_waitpid(pid, status, options);
    }
    return ret;
}


/*
 *  sys_kill ---
 *      The kill system call can be used to send any signal to any process group or
 *      process.
 *
 *      If pid is positive, then signal sig is sent to pid.
 *
 *      If pid equals 0, then sig is sent to every process in the process group of the
 *      current process.
 *
 *      If pid equals -1, then sig is sent to every process except for process 1 (init),
 *      but see below.
 *
 *      If pid is less than -1, then sig is sent to every process in the process group-pid.
 *
 *      If sig is 0, then no signal is sent, but error checking is still performed.
 *
 *  Returns:
 *      On success, zero is returned.  On error, -1 is returned, and
 *      errno is set appropriately.
 *
 *      EINVAL      An invalid signal was specified.
 *
 *      ESRCH       The pid or process group does not exist.  Note that
 *                  an existing process might be a zombie, a process which
 *                  already committed termination, but has not yet been
 *                  wait()ed for.
 *
 *      EPERM       The process does not have permission to send the signal
 *                  to any of the receiving processes.
 */
int
sys_kill(int pid, int value)
{
    if (pid > 0) {
        HANDLE hProc = (HANDLE)pid;

        if (WaitForSingleObject(hProc, 0) != WAIT_TIMEOUT) {
            errno = ESRCH;                      // not running
            return -1;
        }

        switch (value) {
        case 0:
            return 0;

        case SIGINT:
            /*
             *  CTRL_C_EVENT
             *      Generates a CTRL+C signal. This signal cannot be generated
             *      for process groups. If dwProcessGroupId is nonzero, this
             *      function will succeed, but the CTRL+C signal will not be
             *      received by processes within the specified process group.
             */
            GenerateConsoleCtrlEvent(CTRL_C_EVENT, /*LOCAL*/0);
            return 0;

        case SIGTERM:
            SendCloseMessage(hProc);
            return 0;
        }
    }
    errno = EINVAL;
    return -1;
}


/*
 *  SendCloseMessage ---
 *      Try to kill a process politely by sending a WM_CLOSE message.
 */
struct _enum_win_info {
    HANDLE          hProcess;
    DWORD           dwProcessId;
};


static BOOL CALLBACK
EnumWindowsProc(HWND hwnd, LPARAM lParam)
{
    /*
     *  Callback to EnumWindows..
     *
     *  Parameters
     *      hwnd    [in] Handle to a top-level window.
     *      lParam  [in] Specifies the application-defined value given in
     *              EnumWindows or EnumDesktopWindows.
     *
     *  Return Value:
     *      To continue enumeration, the callback function must
     *      return TRUE; to stop enumeration, it must return FALSE.
     */
    struct _enum_win_info *info = (struct _enum_win_info *)lParam;
    DWORD pid, status;

    (void) GetWindowThreadProcessId(hwnd, &pid);
    return pid != info->dwProcessId ||
                (GetExitCodeProcess(info->hProcess, &status) &&
                 status == STILL_ACTIVE &&       // value = 259
                 PostMessage(hwnd, WM_CLOSE, 0, 0));
}


static BOOL
SendCloseMessage(HANDLE hProc)
{
    struct _enum_win_info info = {0};
    struct procdata *pd = NULL;

    if (proc_find((int)hProc, (void **)&pd) == sizeof(struct procdata)) {
        if (pd) {                               // locate procdata
            info.hProcess = hProc;
            info.dwProcessId = pd->dwProcessId;
            return EnumWindows(EnumWindowsProc, (LPARAM)&info);
        }
    }
    return FALSE;
}


/*
 *  popen ---
 *      Pipe open implementation for WIN32.
 */
FILE *
popen(const char *cmd, const char *mode)
{
    return w32_popen(cmd, mode);
}


/*
 *  pclose ---
 *      Pipe close implementation for WIN32.
 */
int
pclose(FILE *file)
{
    return w32_pclose(file);
}

#endif /*WIN32*/
