#include <edidentifier.h>
__CIDENT_RCSID(gr_cmain_c,"$Id: cmain.c,v 1.29 2015/02/17 23:26:17 ayoung Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: cmain.c,v 1.29 2015/02/17 23:26:17 ayoung Exp $
 * Main body, startup and command-line processing.
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
#if defined(HAVE_LOCALE_H)
#include <locale.h>
#endif
#include <edconfig.h>
#include <edstacktrace.h>
#include <edfileio.h>
#include <edpaths.h>
#include <edenv.h>                              /* gputenvv(), ggetenv() */
#include <libstr.h>                             /* str_...()/sxprintf() */

#include "../libvfs/vfs.h"                      /* vfs_init() */

#include "accum.h"                              /* acc_...() */
#include "anchor.h"
#include "arg.h"
#include "basic.h"
#include "bookmark.h"
#include "buffer.h"                             /* buf_...() */
#include "builtin.h"
#include "cmap.h"                               /* cmap_init() */
#include "color.h"
#include "debug.h"
#include "dialog.h"                             /* dialog_shutdown() */
#include "display.h"
#include "echo.h"
#include "file.h"
#include "getkey.h"
#include "keyboard.h"
#include "keywd.h"
#include "kill.h"
#include "language.h"
#include "line.h"                               /* line_init() */
#include "lock.h"                               /* flock_init() */
#include "macros.h"
#include "main.h"
#include "map.h"
#include "mchar.h"                              /* mchar_..() */
#include "mouse.h"
#include "playback.h"
#include "position.h"                           /* position_..() */
#include "procspawn.h"
#include "register.h"
#include "ruler.h"                              /* tabchar_set() */
#include "search.h"
#include "signals.h"
#include "spell.h"                              /* spell_init() */
#include "symbol.h"
#include "syntax.h"
#include "sysinfo.h"
#include "system.h"
#include "tags.h"                               /* tags_shutdown() */
#include "tty.h"
#include "undo.h"
#include "wild.h"
#include "window.h"

#include "m_feature.h"                          /* x_features */

#define MAX_M       32                          /* Number of -m switches, including -u switch. */

static struct argoption options[] = {
    { "add",            arg_none,           NULL,       'a',    "Add file to current profile" },

    { "nobackup",       arg_none,           NULL,       'b',    "Disable backup creation" },

    { "noautosave",     arg_none,           NULL,       300,    "Disable autosave" },

    { "compat",         arg_none,           NULL,       'c',    "Compatability mode (dont enable nodelay)" },

    { "display",        arg_none,           NULL,       'E',    "Enable display on startup" },

    { "define",         arg_required,       NULL,       'D',    "Define an environment variable",
                            "var[=value]" },

    { "echoflags",      arg_required,       NULL,       'e',    "Echo line state information",
                            "charval,virt,col,percent,time,remember,cursor" },

    { "echofmt",        arg_required,       NULL,       301,    "Echo line format definition",
                            "<format-spec>" },

    { "timer",          arg_required,       NULL,       'i',    "Set interval timer to N seconds",
                            "N" },

    { "vm",             arg_required,       NULL,       'M',    "Set virtual memory size to # lines",
                            "#" },

    { "mouse",          arg_none,           NULL,       1,      "Enable mouse" },

    { "nomouse",        arg_none,           NULL,       1,      "Disable mouse support" },

    { "scroll",         arg_none,           NULL,       2,      "Enable xterm scrolling" },

    { "noscroll",       arg_none,           NULL,       2,      "Disable xterm scrolling" },

    { "color",          arg_optional,       NULL,       3,      "Force color mode",
                            "=<depth>" },

    { "nocolor",        arg_none,           NULL,       3,      "Force black and white display" },

    { "visbell",        arg_none,           NULL,       302,    "Enable visable bell" },

    { "light",          arg_none,           NULL,       4,      "Light color scheme" },

    { "dark",           arg_none,           NULL,       4,      "or dark color scheme" },

//TODO
//  { "scheme",         arg_required,       NULL,       xxx,    "Color scheme name",
//                          "=<name>" },

    { "utf8",           arg_optional,       NULL,       303,    "UTF8 features",
                            "=[no|yes],[[no]combined,seperate],[subst|ncr|ucn|hex]" },

    { "8bit",           arg_optional,       NULL,       304,    "Eight bit terminal encoding, with optional encoding",
                            "=<encoding>" },

    { "7bit",           arg_none,           NULL,       305,    "Seven bit terminal encoding" },

    { "guess",          arg_required,       NULL,       306,    "File encoding search specification",
                            "charset,chardet,mark,bom,magic,utf..." },

    { "buftype",        arg_required,       NULL,       316,    "Default buffer-type",
                            "dos|unix|mac|ansi" },

    { "encoding",       arg_required,       NULL,       317,    "Default file encoding",
                            "<encoding>" },

    { "nograph",        arg_none,           NULL,       5,      "Disable use of graphic characters" },

    { "nounicode",      arg_none,           NULL,       307,    "Disable use of UNICODE graphic characters" },

    { "nounderline",    arg_none,           NULL,       6,      "Disable use of underline mode" },

    { "nohilite",       arg_none,           NULL,       7,      "Disable syntax hiliting" },

    { "nocygwinkb",     arg_none,           NULL,       8,      "Disable use of cygwin raw scancodes" },

    { "nosigtrap",      arg_none,           NULL,       17,     "Disable signal trapping (for debugging)" },

    { "term",           arg_required,       NULL,       308,    "Override the TERM setting",
                            "<termname>" },

    { "grterm",         arg_required,       NULL,       309,    "Override the GRTERM setting",
                            "<termname>" },

    { "grhelp",         arg_required,       NULL,       318,    "Override the GRHELP setting",
                            "<path>" },

    { "grprofile",	arg_required,       NULL,       319,    "Override the GRPROFILE setting",
			    "<user-profile>" },

    { "termcap",        arg_none,           NULL,       310,    "Use termcap if available, otherwise terminfo" },

    { "curses",         arg_none,           NULL,       410,    "Enable/force use of curses tty driver" },

#if defined(HAVE_LIBX11) && defined(HAVE_X11_XLIB_H)
    { "x11",            arg_none,           NULL,       411,    "or enable/force use of x11" },
#endif

    { "noinit",         arg_none,           NULL,       9,      "Disable termcap init/deinit strings" },

    { "nokeypad",       arg_none,           NULL,       10,     "Disable termcap keypad init/deinit strings" },

    { "noborders",      arg_none,           NULL,       12,     "Disable use of borders (main windows/non-popup)" },

    { "notitle",        arg_none,           NULL,       16,     "Disable use/update of console title" },

    { "lazy",           arg_optional,       NULL,       11,     "Limit vt updates to # lines, delayed until stable; for slow links",
                            "=lines" },

    { "jump",           arg_optional,       NULL,       311,    "Scroll jump, paging content not scolling; for slow links",
                            "=lines" },

    { "visible",        arg_optional,       NULL,       312,    "Visible lines above/below cursor",
                            "=lines" },

    { "escdelay",       arg_required,       NULL,       13,     "ESC delay in milliseconds" },

    { "user",           arg_required,       NULL,       'U',    "Execute user profile",
                            "<user-profile>" },

    { "macro",          arg_required,       NULL,       'm',    "Execute named macro",
                            "<macro-name>" },

    { "readonly",       arg_none,           NULL,       'R',    "Read-only mode" },

    { "lockstrict",     arg_none,           NULL,       14,     "Enable strict file-locking" },

    { "spell",          arg_none,           NULL,       15,     "Spell check" },

    { "nospell",        arg_none,           NULL,       15,     "Disable spell" },

    { "spaces",         arg_none,           NULL,       't',    "Use spaces to fill out tabs" },

    { "log",            arg_none,           NULL,       'd',    "Create `" GRLOG_FILE "` diagnostics log" },

    { "logflush",       arg_none,           NULL,       'f',    "Flush log file as created" },

    { "dflags",         arg_required,       NULL,       'P',    "Debugging/profiling flags, comma separated",
                            "regexp,undo,prompt,memory,native,rtc,purify,terminal,vfs,refs,profile" },

    { "warnings",       arg_none,           NULL,       'W',    "Enable warnings" },

    { "logstats",       arg_none,           NULL,       's',    "Report statistics on exit" },

    { "statsref",       arg_none,           NULL,       'r',    "Generate macro reference statistics on exit" },

    { "readchar",       arg_none,           NULL,       'w',    "Allow read_char to return -1" },

    { "test",           arg_optional,       NULL,       313,    "test mode" },

    { "restrict",       arg_none,           NULL,       314,    "restrictive mode" },

    { "help",           arg_none,           NULL,       '?',    "Usage information" },

    { "config",         arg_none,           NULL,       'V',    "Configuration details" },

    { "version",        arg_none,           NULL,       'v',    "Version" },

    { "authors",        arg_none,           NULL,       400,    "Developer details" },

    { "license",        arg_none,           NULL,       401,    "License information" },

    { NULL }
    };

const char *            x_appname = ApplicationName;

const char *            x_progname = "";        /* arg0 */

static int              xf_linenumber = 0;      /* Initial line number. */

static int              xf_dumpstats = FALSE;   /* TRUE if stats reporting on exit. */

static int              xf_dumprefs = FALSE;    /* TRUE if ref status are dumped on exit. */

static int              xf_mouse = TRUE;        /* TRUE enable mouse. */

static int              xf_ttydrv = 't';        /* TTY driver type */

int                     xf_compat = FALSE;      /* TRUE normal stdout I/O otherwise optimised. */

int                     xf_ega43 = 0;           /* TRUE if in 43 line mode. */

int                     xf_usevmin = FALSE;     /* Set to TRUE if we can use VMIN on a SysV.
                                                 *  Only TRUE if mouse in use for fast response.
                                                 */

int                     xf_wait = TRUE;         /* Set to FALSE if read_char should. return -1 on no key pressed.
                                                 * No macros currently use the fact that read_char can return -1
                                                 */

int                     xf_restore = FALSE;     /* TRUE if -a or no files are specified */
int                     xf_readonly = FALSE;    /* TRUE if -R/--readonly specified. */

int                     xf_profile = FALSE;     /* TRUE if profiling on. */

int                     xf_backups = TRUE;      /* TRUE creates backups. */

int                     xf_autosave = TRUE;     /* TRUE perform autosave. */

int                     xf_spell = -1;          /* TRUE/FALSE/-1 enable spell. */

                                                /* TRUE enables tty regions scrolling. */
int                     xf_scrollregions = FALSE;

int                     xf_color = -1;          /* TRUE/FALSE, user specified color mode. */

int                     xf_graph = -1;          /* TRUE/FALSE, user specified graphic mode. */

int                     xf_visbell = FALSE;     /* TRUE/FALSE, visual bell. */

int                     xf_cygwinkb = TRUE;    /* TRUE/FALSE, cygwin raw keyboard. */

int                     xf_sigtrap = TRUE;      /* TRUE/FALSE, control signal traps. */

int                     xf_dumpcore = 0;        /* 1=dumpcore, 2=stack-dump (if available). */

int                     xf_underline = -1;      /* TRUE/FALSE, user specified underline mode. */

int                     xf_title = -1;          /* TRUE/FALSE, user specified console title mode. */

int                     xf_noinit = FALSE;      /* TRUE if no termcap init/deinit. */

int                     xf_nokeypad = FALSE;    /* TRUE if no termcap keypad init/deinit. */

int                     xf_lazyvt = -1;         /* If >0 limit vt updates, 0 disables. */

int                     xf_synhilite = TRUE;    /* TRUE/FALSE enable syntax hiliting. */

int                     xf_strictlock = FALSE;  /* TRUE if strict file-locking. */

int                     xf_escdelay = 0;        /* Non-zero, ESCDELAY in milliseconds. */

int                     xf_warnings = 0;        /* TRUE/FALSE, enable warnings. */

int                     xf_restrict = 0;        /* TRUE/FALSE, restrict search paths. */

int                     xf_utf8 = 0;            /* UTF8 command line options. */

                                                /* Text encoding guess configuration. */
const char *            x_encoding_guess = NULL;

                                                /* Default file encoding. */
int                     x_bftype_default = BFTYP_UNDEFINED;

                                                /* Default file encoding. */
const char *            x_encoding_default = NULL;

uint32_t                xf_test = 0;            /* BITMAP enables test code --- internal use only --- */

int                     x_mflag = FALSE;        /* TRUE whilst processing -m strings to avoid messages. */

                                                /* Set to TRUE when we do a set_term_characters. */
int                     x_display_enabled = -1; /* display status */

int                     x_applevel = 0;         /* Value of GRLEVEL. */

time_t                  x_startup_time = 0;     /* Application startup time. */

int                     x_msglevel = 0;         /* Message level. */

mode_t                  x_umask = 0;            /* Current umask value for creating files. */

int                     x_plevel = 0;           /* Process level. */

int                     x_panycb = 0;           /* Change buffer action. */


static int              m_cnt = 0;              /* -m count. */
static const char *     m_profile;              /* User profile. */
static const char *     m_strings[MAX_M+1];     /* Array of pointer to -m strings. */

BUFFER_t *              curbp = NULL;           /* Current buffer. */
WINDOW_t *              curwp = NULL;           /* Current window. */

int                     ms_cnt;                 /* Macro name stack. */
struct mac_stack        mac_stack[MAX_NESTING + 1];


static void             path_cat(const char *path, const char *sub, char *buf);
static char *           path_cook(const char *name);

static void             argv_init(int *argcp, char **argv);
static int              argv_process(int doerr, int argc, const char **argv);

static void             env_setup(void);
static __CINLINE int    env_iswhite(const char ch);
static int              env_define(const char *cp);

static void             editor_setup(void);
static void             usage(int);

#if defined(HAVE_LIBX11) && defined(HAVE_X11_XLIB_H)
extern void             ttx11(void);
#endif
extern int              cmain(int argc, char **argv);

#if defined(HAVE_ENVIRON)
#if defined(NEED_EXTERN_ENVIRON) || \
            (defined(__GLIBC__) && !defined(__USE_GNU))
extern char **          environ;
#endif
#endif


/*  Function:           cmain
 *      Entry point.
 *
 *  Parameters:
 *      argc - Argument count.
 *      argv - Value Vector.
 *
 *  Returns:
 *      Application return-code.
 *
 *<<GRIEF>> [callback]
    Macro: _startup_complete - Startup event callback.

        void
        _startup_complete(int mode)

    Macro Description:
        The '_startup_complete()' callback is executed by GRIEF upon
        startup.

        It is executed after all command line switches have been process,
        after all command line files have been read in, and after the
        startup() macro has been called.

        The specified 'mode' states the command line status as follows.

            0 - No command line files were specified.

            1 - Command line file were specified.

        This callback is utilised by the restore macro to avoid
        restoring the state of buffers and files when files have been
        specified on the command line.

    Macro Parameters:
        mode - Command line operational mode.

    Macro Returns:
        The '_start_complete()' callback should return the nothing.

    Macro Portability:
        n/a

    Macro See Also:
        Callbacks
 */
int
cmain(int argc, char **argv)
{
    int arg_index, i;

#if defined(HAVE_SIGINTERRUPT)
    siginterrupt(SIGALRM, 1);
#endif

#if defined(HAVE_SETLOCALE)
    setlocale(LC_ALL, "");
#if defined(HAVE_LIBINTL)
#if !defined(PACKAGE)
#define PACKAGE "grief"
#endif
//  bindtextdomain(PACKAGE, _PATH_LOCALEDIR);   /* TODO */
//  bind_textdomain_codeset(PACKAGE, "UTF-8");
//  textdomain(PACKAGE);
#endif
#endif
#if defined(_MSC_VER)
    _tzset();
#else
    tzset();                                    /* localtime requirement */
#endif

    x_umask = (mode_t)fileio_umask(0);          /* our umask */
    if ((int)x_umask < 0) x_umask = 0;
    fileio_umask(x_umask);

    x_progname = argv[0];
    x_startup_time = time(NULL);
    x_progname = sysinfo_execname(x_progname);  /* resolve true name */

    edbt_init(x_progname, 0, stderr);           /* traceback initialisation */
    env_init();
    argv_init(&argc, argv);                     /* prelim argument processing */
    edbt_auto();                                /* enable automatic backtrace if available */

    /* core */
    search_init();                              /* regular expression engine */
    mchar_info_init();
    mchar_guess_init();
    playback_init();
    bookmark_init();
    position_init();
    signals_init(0);
    cm_init(FALSE);
    builtin_init();
    line_init();
    buffer_init();
    macro_init();
    vfs_init();
    flock_init();

    /* argument processing */
    env_setup();
    arg_index = argv_process(TRUE, argc, (const char **)argv);
    if ('c' == xf_ttydrv) ttcurses();
#if defined(HAVE_LIBX11) && defined(HAVE_X11_XLIB_H)
    else if ('x' == xf_ttydrv) ttx11();
#endif
    vtinit(&argc, argv);
    color_setscheme(NULL);
    ttopen();

    /* high-level */
    cmap_init();                                /* character map */
    syntax_init();                              /* syntax hilite engine */
    key_init();
    undo_init();
    editor_setup();                             /* buffers, windows. */
    register_init();                            /* initialise register. */
    if (xf_spell) {
        spell_init();
    }
    vtready();
    if (xf_mouse) {
        if (mouse_init("")) {                   /* mouse interface */
            x_display_ctrl |= DC_MOUSE;
        }
        xf_usevmin = TRUE;
    }
    sym_init();
    sym_globals();
    sym_errno_constants();

    if (macro_startup() < 0) {                  /* execute startup macro */
        undo_close();
        vtclose(TRUE);
        fprintf(stderr, "\n" \
            "*** " ApplicationName " has failed to locate the '" GRINIT_OBJECT "' startup macro.\n" \
            "*** This is due to either your environment variable GRPATH not being\n" \
            "*** setup correctly or macros require to be compiled/recompiled.\n\n");
        exit(1);
    }

    x_plevel = 0;
    if (NULL == m_profile) {                    /* default user profile */
        m_strings[m_cnt++] =
            (m_profile = chk_salloc("profiles/defaultuser"));
    }
    for (i = 0; i < m_cnt; ++i) {
        const char *macro = m_strings[i];

        x_mflag = TRUE;
        trace_log("loading macro : %s\n", macro);
        if (! macro_loaded(macro)) {
            macro_load(macro);
        }
    }

    /*
     *  Execute startup macro and command line macros before reading in files.
     */
    signals_init(1);
    x_plevel = 0;
    execute_str("startup");
    for (i = 0; i < m_cnt; ++i) {
        const char *macro = m_strings[i];

        if (macro_lookup(macro)) {
            x_mflag = TRUE;                     /* -m mode, hide errors */
            x_msglevel = 1;                     /* no warnings */
            trace_log("executing macro : %s\n", macro);
            execute_str(macro);
        }
    }
    x_mflag = FALSE;

    if (arg_index < argc) {                     /* load listed files */
        BUFFER_t *firstbp = NULL;

        while (arg_index < argc) {
            x_msglevel = 1;                     /* no warnings */
            file_edit(argv[arg_index++], EDIT_NORMAL, NULL);
            if (NULL == firstbp) {
                firstbp = curbp;
            }
        }
        buf_show(curbp = firstbp, curwp);

    } else  {                                   /* load default quietly */
        const char *grfile = ggetenv("GRFILE");

        file_load((grfile && *grfile ? grfile : "newfile"), EDIT_NORMAL|EDIT_STARTUP, NULL);
        xf_restore = 1;
    }

    /* Hook to allow restore state macro to get called. */
    x_msglevel = 1;                             /* no warnings */
    if (xf_restore) {
        execute_str("_startup_complete 0");
    } else {
        execute_str("_startup_complete 1");
    }

    if (xf_linenumber) {
        set_hooked();
        mov_gotoline(xf_linenumber);
    }

    trigger(REG_STARTUP);
    main_loop();

    gr_exit(EXIT_SUCCESS);
    return 0;
}


/*  Function:           panic
 *      System panic, restore console, dump current process and exit.
 *
 *  Parameters:
 *      fmt - Format specification.
 *      ... - Message parameters.
 *
 *  Returns:
 *      none
 */
void
panic(const char *fmt, ...)
{
    char buffer[1024];
    va_list ap;
    size_t len;

    va_start(ap, fmt);
    strcpy(buffer, "PANIC: " ApplicationName " - ");
    len = strlen(buffer);
    vsxprintf(buffer + len, sizeof(buffer) - len, fmt, ap);
    va_end(ap);
    eeputs(buffer);
    vtclose(FALSE);
    printf("\n");
    fflush(stdout);
    fflush(stderr);
    sys_abort();
    exit(1);
}


static void
path_cat(const char *path, const char *sub, char *buf)
{
    char t_path[1024] = {0}, t_realpath[1024] = {0};

    if (sub) {                                  /* resolve abs path */
        strcpy(t_path, path);
        strcat(t_path, sub);
        path = t_path;
        if (0 == sys_realpath((const char *)path, t_realpath, sizeof(t_realpath))) {
            path = t_realpath;
        }
    }

    if (NULL == sub ||
            0 == fileio_access(path, 0)) {      /* push if it exists */
        strcat(buf, path);
        strcat(buf, sys_pathdelimiter());
    }
}


static const char *
varend(const char *path, const char delim)
{
    while (*path) {
        if (*path == delim || FILEIO_ISSEP(*path)) {
            return path;
        }
        ++path;
    }
    return path;
}


static char *
path_cook(const char *name)
{
    char buffer[MAX_PATH * 8], *dpend = buffer + (sizeof(buffer) - 16), *dp;

    for (dp = buffer; *name && dp < dpend;) {
        const char ch = *name++;

        if ('$' == ch) {                        /* $ROOT */
            if (0 == strncmp(name + 1, "ROOT", 4)) {
                const size_t rootlen = strlen(_PATH_GRIEF_ROOT);

                if (dp < (dpend - rootlen)) {
                    (void) memcpy(dp, _PATH_GRIEF_ROOT, rootlen);
                    dp += rootlen;
                }
                name += 4;

            } else {                            /* $var/, $(var) and ${var} */
                const char delim = ('(' == *name ? ')' : '{' == *name ? '}' : 0);
                const char *var = (delim ? ++name : name),
                        *end = varend(name, delim);

                if (NULL != (var = ggetnenv(var, end - var))) {
                    const size_t varlen = strlen(var);

                    if (dp < (dpend - varlen)) {
                        (void) memcpy(dp, var, varlen);
                        dp += varlen;
                    }
                }

                if (*end) {
                    if (!delim) *dp++ = *end;
                    ++end;
                }

                name = end;
            }
        } else {
            *dp++ = ch;
        }
    }
    *dp = '\0';

    return chk_salloc(buffer);
}


/*  Function:           argv_init
 *      Check the first argument of the command line, or the actual name of the binary
 *      to determine whether we are running natively under a window system. We remove the
 *      argument from the command line if we can match it.
 *
 *  Parameters:
 *      argcp - Argument count reference.
 *      argv - Value vector.
 *
 *  Returns:
 *      nothing
 */
static void
argv_init(int *argcp, char **argv)
{
    const char *appname = sys_pathend(argv[0]);
    int i;

    appname = (NULL == appname ? argv[0] : appname + 1);

    /* Cook special options, including
     *
     *  Application names:
     *      xgr, X-windows.
     *      wgr[.exe], Windows.
     *
     *  Command Line Options:
     *      -d, -log[flush]
     *      -P and --dflags
     *      --x11
     *      -psn_x_y
     *      --nosig[trap]
     */
    if (0 == strcmp(appname, "xgr") ||
                0 == strcmp(appname, "wgr") || 0 == str_icmp(appname, "wgr.exe")) {
        x_display_ctrl |= DC_WINDOW;
    }

    for (i = 1; i < *argcp; ++i)
        if (argv[i][0] == '-') {
            const char *arg = argv[i];
            int cook = 0;

            if (argv[i][1] == 'd' || 0 == strncmp(arg, "--log", 5))  {
                cook = 1;

            } else if (arg[1] == 'P' || 0 == strncmp(arg, "--dflags", 7)) {
                cook = 2;

            } else if (0 == strcmp(arg, "--x11")) {
                x_display_ctrl |= DC_WINDOW;
                xf_ttydrv = 'x';
                cook = -1;

#if defined(__APPLE__) || defined(MAC_OSX)
            } else if (0 == strncmp(arg, "-psn_", 5)) {
                /*
                 *  -psn_0_<PSN>
                 *      PSN is the Process Serial Number.
                 */
                chdir(getenv("HOME"));          /* exec'ed from Finder, force directory */
                cook = -1;
#endif

            } else if (0 == strncmp(arg, "--nosig", 7)) {
                xf_sigtrap = 0;
            }

            if (cook) {
                if (-1 == cook) {               /* remove */
                    int i2, cnt = *argcp - 1;

                    for (i2 = 1; i2 < cnt; ++i2) {
                        argv[i2] = argv[i2 + 1];
                    }
                    *argcp = cnt;

                } else {
                    const char *av[4] = {0};    /* prog, option [value], null */
                    int ac = 0;

                    av[ac++] = x_progname;
                    av[ac++] = argv[i];
                    if (2 == cook && argv[i + 1]) {
                        av[ac++] = argv[i + 1];
                    }
                    av[ac] = NULL;
                    argv_process(FALSE, ac, av);
                }
            }
        }
}


/*
 *  argv_process ---
 *      Process command line arguments.
 */
static int
argv_process(int doerr, int argc, const char **argv)
{
    struct argparms args = {0};
    int c, errflag = 0;

    arg_initl(&args, argc, argv, (const char *)-1, options, FALSE);

    for (;;) {
        if (-1 == (c = arg_getopt(&args))) {
            const char *cp;

            trace_ilog("end\n");
            if (NULL == argv[args.ind] || '+' != *(cp = argv[args.ind])) {
                break;
            }

            ++cp;                               /* special + operator */
            if (*cp >= '0' && *cp <= '9') {
                xf_linenumber = atoi(cp);
            } else {
                ++errflag;
            }
            arg_next(&args);
            continue;
        }

        switch(c) {
        case 1:             /* [no]mouse */
            xf_mouse = (args.opt == 'm' ? TRUE : FALSE);
            break;

        case 2:             /* tty - [no]scroll regions */
            xf_scrollregions = (args.opt == 's' ? TRUE : FALSE);
            break;

        case 3:             /* tty - [no]color=[depth] */
            if ('c' == args.opt) {
                if (! args.val ||
                        (xf_color = atoi(args.val)) <= 0) {
                    xf_color = 1;
                }
            } else {
                xf_color = 0;
            }
            break;

        case 4:             /* tty - d[ark]|l[ight]. */
            color_setscheme('d' == args.opt ? "dark" : "light");
            break;

        case 5:             /* tty - graphic character. */
            xf_graph = FALSE;
            break;

        case 302:           /* tty - visual bell. */
            xf_visbell = TRUE;
            break;

        case 8:             /* tty - cygwin raw keyboard disabled. */
            xf_cygwinkb = FALSE;
            break;

        case 6:             /* tty - underline mode. */
            xf_underline = FALSE;
            break;

        case 7:             /* tty - disable syntax hiliting. */
            xf_synhilite = FALSE;
            break;

        case 9:             /* disable termcap init/deinit strings. */
            xf_noinit = TRUE;
            break;

        case 10:            /* disable termcap keypad init/deinit strings. */
            xf_nokeypad = TRUE;
            break;

        case 12:            /* window (main) borders. */
            xf_borders = FALSE;
            break;

        case 16:            /* console title mode. */
            xf_title = FALSE;
            break;

        case 11:            /* lazy vt updates. */
            xf_lazyvt = (args.val ? atoi(args.val) : 2);
            break;

        case 311:           /* scroll-jump. */
            x_display_scrollrows = x_display_scrollcols =
                (args.val ? atoi(args.val) : 0xffff);
            break;

        case 312:           /* visible-lines. */
            x_display_minrows = x_display_mincols =
                (args.val ? atoi(args.val) : 0xffff);
            break;

        case 307:           /* limit use of UNICODE characters. */
            x_display_ctrl |= DC_ASCIIONLY;
            break;

        case 13:            /* ESC delay. */
            xf_escdelay = atoi(args.val);
            break;

        case 14:            /* strict file-locking. */
            xf_strictlock = TRUE;
            break;

        case 15:            /* [no]spell */
            xf_spell = ('n' == args.opt ? FALSE : TRUE);
            break;

        case 17:            /* disable signal trapping */
            xf_sigtrap = FALSE;
            break;

        case 'a':           /* additional files */
            xf_restore = TRUE;
            break;

        case 'b':           /* nobackup */
            xf_backups = FALSE;
            break;

        case 300:           /* noautosavep */
            xf_autosave = FALSE;
            break;

        case 'c':           /* compat i/o */
            xf_compat = TRUE;
            break;

        case 'd':           /* debug trace */
            trace_flagsset(trace_flags() | DB_TRACE);
            break;

        case 'D':           /* -D define */
            if (args.val && 0 != strcmp(args.val, "__XdummyX__")) {
                static const struct {
                    const char *from;
                    unsigned    fromlen;
                    const char *to;
                } vm[] = {                      /* map BRIEF definitions */
                    { "BPATH=",   6, "GRPATH"   },
                    { "BHELP=",   6, "GRHELP"   },
                    { "BBACKUP=", 8, "GRBACKUP" },
                    { "BFLAGS=",  7, "GRFLAGS"  },
                    { "BTMP=",    4, "GRTMP"    }
                    };
                const char *val = args.val;
                unsigned v;

                trace_log("-D%s\n", val);
                for (v = 0; v < (sizeof(vm)/sizeof(vm[0])); ++v) {
                    if (0 == strncmp(vm[v].from, val, vm[v].fromlen)) {
                        gputenv2(vm[v].to, val + vm[v].fromlen);
                        trace_log("==> %s=%s\n", vm[v].to, val + vm[v].fromlen);
                        val = NULL;
                        break;
                    }
                }
                gputenv(val);
            }
            break;

        case 'E':
            x_display_enabled = 1;
            break;

        case 'P': {         /* debug/profile flags */
                int nflags = trace_flags();

                if (isdigit(args.val[0])) {
                    nflags |= atoi(args.val);

                } else {
                    static const char * const tokens[] = {
                        /*0 */"regexp",
                        /*1 */"undo",
                        /*2 */"prompt",
                        /*3 */"refs",
                        /*4 */"purify",
                        /*5 */"memory",     /*6 */"mem",
                        /*7 */"native",
                        /*8 */"rtc",
                        /*9 */"terminal",   /*10*/"term",
                        /*11*/"vfs",
                        /*12*/"profile",
                        NULL
                        };
                    char *option = (char *)args.val, *value = NULL;

                    while (option && *option) {
                        switch (arg_subopt(&option, tokens, &value)) {
                        case 0: nflags |= DB_REGEXP; break;
                        case 1: nflags |= DB_UNDO;   break;
                        case 2: nflags |= DB_PROMPT; break;
                        case 3: nflags |= DB_REFS;   break;
                        case 4: nflags |= DB_PURIFY;
                            ++vm_dflag;
                            break;
                        case 5: case 6:
                            nflags |= DB_MEMORY;
                        case 7:             /*see cmain.cpp*/
                        case 8:             /*see cmain.cpp*/
                            ++vm_dflag;
                            break;
                        case 9: case 10:
                            nflags |= DB_TERMINAL;
                            break;
                        case 11: nflags |= DB_VFS;
                            break;
                        case 12:            /*defunct*/
                            xf_profile = TRUE;
                            break;
                        default:
                            fprintf(stderr, "%s: unknown debug flag '%s'\n", x_progname, value);
                            ++errflag;
                            break;
                        }
                    }
                }
                trace_flagsset(nflags);
            }
            break;

        case 'f':           /* debug flush/sync on write */
            trace_flagsset(trace_flags() | DB_FLUSH);
            break;

        case '4':           /* ega43, funct */
            xf_ega43 = TRUE;
            break;

        case 'e':           /* echo flags */
            if (isdigit(args.val[0])) {
                xf_echoflags = atoi(args.val);

            } else {
                static const char * const tokens[] = {
                        "line", "col", "percent", "time", "remember",
                        "cursor", "virt", "charval", NULL
                        };
                char *option = (char *)args.val, *value = NULL;

                xf_echoflags = 0;
                while (option && *option) {
                    switch (arg_subopt(&option, tokens, &value)) {
                    case 0: xf_echoflags |= E_LINE; break;
                    case 1: xf_echoflags |= E_COL; break;
                    case 2: xf_echoflags |= E_PERCENT; break;
                    case 3: xf_echoflags |= E_TIME; break;
                    case 4: xf_echoflags |= E_REMEMBER; break;
                    case 5: xf_echoflags |= E_CURSOR; break;
                    case 6: xf_echoflags |= E_VIRTUAL; break;
                    case 7: xf_echoflags |= E_CHARVALUE; break;
                    default:
                        fprintf(stderr, "%s: unknown echo flag '%s'\n", x_progname, value);
                        ++errflag;
                        break;
                    }
                }
            }
            break;

        case 'i': {         /* interval */
                int t_interval;

                if ((t_interval = atoi(args.val)) < 0) {
                    t_interval = 0;
                }
                xf_interval = t_interval;
            }
            break;

        case 'M':           /* virtual-memory size */
            /*TODO*/
            break;

        case 'U':           /* execute user profile */
            if (NULL == m_profile) {
                if (args.val == sys_basename(args.val)) {
                    char profile[MAX_PATH] = "profiles/";

                    strxcat(profile, args.val, sizeof(profile));
                    m_strings[m_cnt++] =
                        (m_profile = chk_salloc(profile));

                } else {
                    m_strings[m_cnt++] =
                        (m_profile = chk_salloc(args.val));
                }

            } else {
                fprintf(stderr, "%s: multiple -u specified\n", x_progname);
                ++errflag;
            }
            break;

        case 'm':           /* execute macro */
            if (m_cnt < (MAX_M - 1)) {
                m_strings[m_cnt++] = args.val;
            } else {
                fprintf(stderr, "%s: -m limit exceeded, ignored '%s'\n", x_progname, args.val);
            }
            break;

        case 'r':           /* diagnostics */
            xf_dumprefs = TRUE;
            break;

        case 's':           /* run-time statistics */
            xf_dumpstats = TRUE;
            break;

        case 'R':           /* read-only mode */
            x_display_ctrl |= DC_READONLY;
            xf_readonly = TRUE;
            break;

        case 't':           /* use spaces */
            tabchar_set(FALSE);
            break;

        case 'W':           /* enable warnings */
            ++xf_warnings;
            break;

        case 'w':
            xf_wait = FALSE;
            break;

        case 'v':           /* --version */
            usage(0);
            break;

        case 'V':           /* --config */
            usage(2);
            break;

        case 301:           /* --echofmt */
            set_echo_format(args.val);
            break;

        case 303:           /* utf8[=options] */
            if (args.val) {
                static const char * const tokens[] = {
                    /*0*/ "no",
                    /*1*/ "yes",
                    /*2*/ "combined",
                    /*3*/ "nocombined",
                    /*4*/ "seperate",
                    /*5*/ "subst",
                    /*6*/ "ncr",
                    /*7*/ "ucn",
                    /*8*/ "hex",
                    /*9*/ "c99",
                    NULL
                    };
                char *option = (char *)args.val, *value = NULL;

                while (option && *option) {
                    switch (arg_subopt(&option, tokens, &value)) {
                    case 0:         /* no. */
                        xf_disptype = DISPTYPE_8BIT;
                        break;
                    case 1:         /* yes. */
                        xf_disptype = DISPTYPE_UTF8;
                        break;
                    case 2:         /* combined. */
                        xf_disptype = DISPTYPE_UTF8;
                        xf_utf8 |=  DISPUTF8_COMBINED;
                        break;
                    case 3:         /* nocombined. */
                        xf_disptype = DISPTYPE_UTF8;
                        xf_utf8 ^=  ~DISPUTF8_COMBINED;
                        break;
                    case 4:         /* separate -- not implemented as this time. */
                        xf_disptype = DISPTYPE_UTF8;
                        xf_utf8 |=  DISPUTF8_SEPERATE;
                        break;
                    case 5:         /* subst. */
                        xf_utf8 ^= ~DISPUTF8_SUBST_MASK;
                        break;
                    case 6:         /* ncr - Numerical Character Reference. */
                        xf_utf8 ^= ~DISPUTF8_SUBST_MASK;
                        xf_utf8 |= DISPUTF8_NCR;
                        break;
                    case 7:         /* ucn - Universal Character Name. */
                        xf_utf8 ^= ~DISPUTF8_SUBST_MASK;
                        xf_utf8 |= DISPUTF8_UCN;
                        break;
                    case 8:         /* hex - HEX mode. */
                        xf_utf8 ^= ~DISPUTF8_SUBST_MASK;
                        xf_utf8 |= DISPUTF8_HEX;
                        break;
                    case 9:         /* c99. */
                        xf_utf8 ^= ~DISPUTF8_SUBST_MASK;
                        xf_utf8 |= DISPUTF8_C99;
                        break;
                    default:
                        fprintf(stderr, "%s: unknown uft8 option '%s'\n", x_progname, value);
                        ++errflag;
                        break;
                    }
                }
            } else {
                xf_disptype = DISPTYPE_UTF8;    /* UTF8 enable */
            }
            break;

        case 304:           /* --8bit=[encoding] */
            xf_disptype = DISPTYPE_8BIT;
         /* xf_disp_encoding = args.val */
            break;

        case 305:           /* --7bit */
            xf_disptype = DISPTYPE_7BIT;
            break;

        case 306:           /* --guess=[encodings ...] */
            x_encoding_guess = args.val;
            break;

        case 316: {         /* --buftype=<buftype> */
                const char *buftype = args.val;

                if (0 == str_icmp(buftype, "dos")) {
                    x_bftype_default = BFTYP_DOS;
                } else if (0 == str_icmp(buftype, "unix")) {
                    x_bftype_default = BFTYP_UNIX;
                } else if (0 == str_icmp(buftype, "ansi")) {
                    x_bftype_default = BFTYP_ANSI;
                } else if (0 == str_icmp(buftype, "mac")) {
                    x_bftype_default = BFTYP_MAC;
                } else {
                    fprintf(stderr, "%s: unknown buffer type '%s'\n", x_progname, buftype);
                    ++errflag;
                }
            }
            break;

        case 317:           /* --encoding=<encoding> */
            x_encoding_default = args.val;
            break;

        case 308:           /* --term=<TERM-override> */
            gputenv2("TERM", args.val);
            break;

        case 309:           /* --grterm=<GRTERM-override> */
            gputenv2("GRTERM", args.val);
            break;

        case 318:           /* --grhelp=<GRHELP-override> */
            gputenv2("GRHELP", args.val);
            break;

	case 319:           /* --grprofile=<GRPROFILE-override> */
            gputenv2("GRPROFILE", args.val);
            break;

        case 310:           /* --termcap otherwise --terminfo */
            xf_termcap = TRUE;
            break;

        case 313:           /* --test[=<case>] */
            if (NULL == args.val) {
                xf_test |= XF_FLAG(32);
            } else {
                const int val = atoi(args.val);
                if (val >= 1 && val <= 32) {
                    xf_test |= XF_FLAG(val);
                } else {
                    fprintf(stderr, "%s: test out of range '%s'\n", x_progname, args.val);
                    ++errflag;
                }
            }
            break;

        case 314:           /* restrict[ive] */
            ++xf_restrict;
            break;

        case 400:           /* authors */
            usage(3);
            break;

        case 401:           /* license */
            usage(4);
            break;

        case 410:           /* tty - curses driver. */
            xf_ttydrv = 'c';
            break;

        case 411:           /* tty - x11 driver. */
            xf_ttydrv = 'x';
            break;

        case '?':
        case ':':
        default:            /* unknown/error */
            ++errflag;
            break;
        }
    }

    arg_close(&args);
    if (errflag && doerr) {
        usage(1);
    }
    return args.ind;
}


static void
env_setup(void)
{
    const char *execname, *env;
    char binpath[1024] = {0};
    char buf[1024*4] = {0};                     /* upto 4 paths */
    char *cp;

    /* Terminal */
    if (NULL == ggetenv("TERM")) {
        gputenv(x_default_term);
    }

    /* Application directory */
    execname = sysinfo_execname(x_progname);
    strxcpy(binpath, execname ? execname : x_progname, sizeof(binpath));
    if (NULL != (cp = (char *)sys_pathend(binpath))) {
        *++cp = 0;                              /* remove application name */
    } else {
        strcpy(binpath, "./");
        fprintf(stderr, "\nWARNING: unable to resolve application directory, using '%s'\n", binpath);
    }
    trace_log("BINPATH<%s>\n", binpath);

    /* PATHS */
    if (NULL == ggetenv("GRPATH")) {
        char *grpath = path_cook(x_grpath);

        sprintf(buf, "GRPATH=");
        if (binpath[0]) {                       /* rel to binary image */
            path_cat(binpath, "../macros", buf);
#if defined(__MINGW32__)
            path_cat(binpath, "../lib/grief/macros", buf);
#endif
        }
        if (NULL != (env = sysinfo_homedir(NULL, -1))) {
            /* home directory */
            path_cat(env, "/bin/macros", buf);
            path_cat(env, "/macros", buf);
        }
        path_cat(grpath, NULL, buf);            /* default */
        chk_free(grpath);
        gputenv(file_slashes(buf));
    }

    if (NULL == ggetenv("GRHELP")) {
        char *grhelp = path_cook(x_grhelp);

        sprintf(buf, "GRHELP=");
        if (binpath[0]) {                      /* rel to binary image */
            path_cat(binpath, "../help", buf);
#if defined(__MINGW32__)
            path_cat(binpath, "../lib/grief/help", buf);
#endif
        }
        if (NULL != (env = sysinfo_homedir(NULL, -1))) {
            /* home directory */
            path_cat(env, "/bin/help", buf);
            path_cat(env, "/help", buf);
        }
        path_cat(grhelp, NULL, buf);            /* default */
        chk_free(grhelp);
        gputenv(file_slashes(buf));
    }

    if (NULL == ggetenv("GRDICTIONARIES")) {
        sprintf(buf, "GRDICTIONARIES=");
        if (binpath[0]) {                       /* rel to binary image */
            path_cat(binpath, "../dictionaries", buf);
        }
        gputenv(file_slashes(buf));
    }

    /* flags/configuration */
    if (NULL == ggetenv("GRFLAGS")) {
        gputenv(x_grflags);
    }

    if (NULL == ggetenv("GRFILE")) {
        gputenv(x_grfile);
    }

    if (NULL != (env = ggetenv("GRFLAGS"))) {
        const char *av[32+2] = {NULL};          /* limit of 32 plus program and term */
        int ac;

        cp = chk_salloc(env);
        chk_leak(cp);                           /* cannot free 'cp' as reference(s) maybe taken */

        trace_log("GRFLAGS=<%s>\n", cp);
        ac = arg_split(cp, av + 1, 32);         /* split */

        if (ac > 0) {
            int i;

            av[0] = x_progname;                 /* arg0 */
            ++ac;                               /* include within argument-count */

            for (i = 1; i < ac; ++i) {
                const char *v = av[i];

                trace_log(" [%d]: %s\n", i, (v ? v : "n/a"));
                if (v && '-' == v[0] && 'D' == v[1] && v[2]) {
                    if (0 == env_define(v + 2)) {
                        /*
                         *  -DCONST[=value]
                         */
                       av[i] = "-D__XdummyX__";
                    }
                }
            }
            argv_process(FALSE, ac, av);       /* cook options */
        }
    }

    /* nesting level */
    if (NULL != (env = ggetenv("GRLEVEL"))) {
        x_applevel = atoi(env) + 1;
    } else {
        x_applevel = 1;
    }
    sprintf(buf, "GRLEVEL=%d", x_applevel);
    gputenv(buf);

    /* dump the environment */
#if defined(HAVE_ENVIRON) || defined(WIN32)
    {
        const char **envp = (const char **)environ;
        trace_log("environment:\n");
        if (envp)
            while (*envp) {
                trace_log("\t%s\n", *envp);
                ++envp;
            }
    }
#endif  /*HAVE_ENVIRON*/
}


static __CINLINE int
env_iswhite(const char ch)
{
    return (' ' == ch || '\t' == ch);
}


static int
env_define(const char *cp)
{
    int type = F_INT;
    char *name = NULL, *svalue = NULL;
    accint_t ivalue = 1;
    int ret = -1;

    /* parser, integer (default of 1) otherwise string */
    if (NULL == (svalue = strchr(cp, '='))) {
        type = F_INT;
        ivalue = 1;

    } else {
        char *endp = NULL;

        errno = 0;
        *svalue++ = 0;
        ivalue = accstrtoi(svalue, &endp, 10);
        if (errno || (*endp && ! env_iswhite(*endp))) {
            type = F_STR;
        }
    }
    name = chk_salloc(cp);

    /* define CONSTANT */
#if (TODO_CMDLINE_CONSTANTS)
    if (! sym_lookup(name)) {
        SYMBOL *sp;

        if (NULL != (sp = sym_push(TRUE, (const char *)name, type, SF_CONSTANT))) {
            if (F_STR == type) {
               sym_assign_str(sp, (const char *)svalue);
            } else {
               sym_assign_int(sp, ivalue);
            }
            ret = 0;
        }
    } else {
        errorf("redefining '%s', ignored", name);
    }
#endif

    if (F_STR == type) {
        trace_log("-D%s=%s (string)\n", name, svalue);
    } else {
        trace_log("-D%s=%ld (integer)\n", name, ivalue);
    }
    if (svalue) {
        svalue[-1] = '=';                       /* restore */
    }
    return ret;
}


void
gr_exit(int rcode)
{
    if (! xf_dumpcore) {
        LISTV largv[MAX_ARGC];

        (void) memset(largv, 0, MAX_ARGC * sizeof(LISTV));
        margv = largv;                          /* create argument frame for exit */
        margc = 0;
        trigger(REG_EXIT);
    }

    if (xf_dumprefs) {
        trace_flagsset(trace_flags() | DB_TRACE);
        trace_refs();                           /* dump built reference count */
    }

    undo_close();

    tags_shutdown();
    if (xf_spell) {
        spell_close();
    }

    flock_close();
    vfs_shutdown();

    if (xf_mouse) {
        mouse_close();
    }

    vtclose(TRUE);

    if (xf_dumpstats) {
        sym_dump();
    }

    position_dump();                            /* macro resource leaks */
    anchor_dump();

    if ((trace_flags() & (DB_PURIFY|DB_MEMORY)) || vm_dflag) {
        /* track down memory leaks */
        dialog_shutdown();
        register_shutdown();
        buffer_shutdown();
        position_shutdown();
        sym_shutdown();
        syntax_shutdown();
        key_shutdown();
        bookmark_shutdown();
        playback_shutdown();
        cmap_shutdown();
        macro_shutdown();
        cm_shutdown();
        env_shutdown();
    }

    signals_shutdown();
    trace_flush();

    if (xf_dumpcore) {
        if (xf_dumpcore >= 2) {
            sys_core("ALERT - caught fatal exit condition dumping core", NULL, NULL);
        }
        sys_abort();
    }

    if (xf_profile) {
        exit(rcode);
    }

    sys_cleanup();

    if (xf_dumpstats || (trace_flags() & DB_MEMORY) || vm_dflag) {
        chk_stats();
    }

    _exit(rcode);
}


/*
 *  editor_setup ---
 *      Editor startup to initialise the buffer and windowing data structures.
 */
static void
editor_setup(void)
{
    BUFFER_t *bp = NULL;
    WINDOW_t *wp = NULL;

    if (NULL == (bp = buf_find_or_create("/SCRAP/anon")) ||
            NULL == (wp = window_new(NULL))) {
        panic("anon");
    }

    k_init(bp);
    curbp = bp;                                 /* current ones. */
    bp->b_nwnd = 1;                             /* displayed. */
    bp->b_keyboard = NULL;

    wp->w_bufp = bp;
    wp->w_top_line = wp->w_line = wp->w_col = 1;
    wp->w_corner_hints[TL_CORNER] = CORNER_3  | CORNER_6;
    wp->w_corner_hints[TR_CORNER] = CORNER_9  | CORNER_6;
    wp->w_corner_hints[BL_CORNER] = CORNER_12 | CORNER_3;
    wp->w_corner_hints[BR_CORNER] = CORNER_12 | CORNER_9;
    wp->w_type = W_TILED;
    wp->w_tab = 0;                              /* TABLINE */
    window_append(wp);
    curwp = wp;

    cur_line = &bp->b_line;
    cur_col = &bp->b_col;

    wp->w_w = (uint16_t)(ttcols() - 2);         /* 0..78 */
    wp->w_h = (uint16_t)(ttrows() - 3);
    wp->w_status = WFHARD;

    window_title(wp, "*scratch*", "");
}


/*
 *  usage ---
 *      Command line usage.
 */
static void
usage(int what)
{
    static const char *authors[] = {
        "\001Adam Young",
            "\002Author of " ApplicationName " <" ApplicationEmail ">",
            "",

        /*
         *  The code in this file is part of the CRISP package which is (C) P Fox. This code may
         *  be freely used in any product but the copyright remains that of the author. This copyright
         *  notice is present to avoid a conflict of interest and to ensure that CRISP can continue
         *  to be a part of the public domain.
         */
        "\001Paul Fox",
            "\002FoxTrot Systems developed the original free/shareware Crisp as an UnderWare Inc's, later",
            "\002Borland 3.1 BRIEF(tm) clone/emulation, targeted for Unix(tm) and VMS. The last public",
            "\002release was Crisp 2.2 in 1991 prior to becoming that is now CrispEdit(tm) (www.crisp.com)",
            "\002and (www.crisp.demon.co.uk).",
            "\002",
            "\002Paul Fox no longer maintains nor supports the Crisp 2.2 version.  Crisp was based in part",
            "\002on a mix of public domain and specialised components. The final source and several earier",
            "\002versions can be found on old mail archive sites.",
            "",

        "\001Dave Conroy",
            "\002Author of the public domain MicroEmacs upon which Crisp was originally based. MicroEMACS is",
            "\002supported on a variety of machines and operating systems, including MS-DOS VMS and UNIX",
            "\002(several versions); which can be found within CUG archives. Since that time a number of",
            "\002variations have been developed, such as MicroGNUEmacs (or mg) and uEmacs/PK.",
            "",
        };
#define HINDENT         5
    int width;

    fprintf(stderr, "\n"\
        "%s %s compiled %s (cm version %u.%u)\n"\
        "\n", x_appname, x_version, x_compiled, (unsigned) (cm_version / 10), (unsigned) (cm_version % 10));

    if (3 == what) {
        /*
         *  Authors
         */
        unsigned idx;

        for (idx = 0; idx < sizeof(authors)/sizeof(authors[0]); ++idx) {
            const char *line = authors[idx];

            if (*line >= 0x01 && *line <= 0x05) {
                fprintf(stderr, "%*s", *line++ * HINDENT, "");
            }
            fprintf(stderr, "%s\n", line);
        }

    } else if (2 == what) {
        /*
         *  Configuration
         */
        const char *env;

        fprintf(stderr, "PROGNAME=%s\n", x_progname);
        fprintf(stderr, "MACHTYPE=%s\n", x_machtype);
        fprintf(stderr, "GRPATH=%s\n",          (NULL != (env = ggetenv("GRPATH")) ? env : ""));
        fprintf(stderr, "GRHELP=%s\n",          (NULL != (env = ggetenv("GRHELP")) ? env : ""));
        fprintf(stderr, "GRPROFILE=%s\n",       (NULL != (env = ggetenv("GRPROFILE")) ? env : ""));
        fprintf(stderr, "GRLEVEL=%s\n",         (NULL != (env = ggetenv("GRLEVEL")) ? env : ""));
        fprintf(stderr, "%s\n", x_grfile);
        fprintf(stderr, "%s\n", x_grflags);
        fprintf(stderr, "GRBACKUP=%s\n",        (NULL != (env = ggetenv("GRBACKUP")) ? env : ""));
        fprintf(stderr, "GRVERSIONS=%s\n",      (NULL != (env = ggetenv("GRVERSIONS")) ? env : ""));
        fprintf(stderr, "GRDICTIONARIES=%s\n",  (NULL != (env = ggetenv("GRDICTIONARIES")) ? env : ""));
        fprintf(stderr, "GRDICTIONARY=%s\n",    (NULL != (env = ggetenv("GRDICTIONARY")) ? env : ""));
        fprintf(stderr, "GR%s\n", x_default_term);
        if (x_features[0]) {
            unsigned fidx;

            fprintf(stderr, "\n");
            for (fidx = 0; x_features[fidx]; ++fidx) {
                fprintf(stderr, "%s\n", x_features[fidx]);
            }
            fprintf(stderr, "\n");
        }

    } else if (1 == what) {
        /*
         *  Detailed usage
         */
        fprintf(stderr,
            "Usage: cr [options] [+line-number] file ..\n"\
            "\n"\
            "Options:\n");

        width = arg_print(HINDENT, options);

        fprintf(stderr, "%*s%s%*s%s\n\n",
            HINDENT, "", "+nn", width-3, "", "Goto line nn.");

        fprintf(stderr, "Note:\n" \
            "%*sOptions can be system/configuration dependant and shall be"\
            " disregarded if not applicable.\n\n", HINDENT, "");
    }
    fflush(stderr);
    _exit(EXIT_FAILURE);
#undef      HINDENT
}

/*end*/
