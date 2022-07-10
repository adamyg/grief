#ifndef MACSRC_GRIEF_H_INCLUDED
#define MACSRC_GRIEF_H_INCLUDED
/* -*- mode: cr; indent-width: 4; -*- */
/* grief.h --- common definitions
 *
 *  An auto-generated file, do not modify
 */

#include "alt.h"                            /* keycodes */

#if defined(OS2)
#define HPFS
#endif

#if defined(UNIX)  && !defined(unix)
#define unix
#endif
#if defined(LINUX) && !defined(linux)
#define linux
#elif defined(SUN) && !defined(sun)
#define sun
#endif

/*
 *  Booleans
 */
#ifndef TRUE
#define TRUE                    1
#define FALSE                   0
#endif

#define CLICK_TIME_MS           250

#define APPNAME                 "GRIEF"     /* our name */


/*
 *  Terminal/display features.
 */
#define TF_INIT                 1           /* additional init commands */
#define TF_RESET                2           /* additional reset commands */
#define TF_GRAPHIC_MODE         3           /* enable graphics mode */
#define TF_TEXT_MODE            4           /* leave graphics mode */
#define TF_INSERT_CURSOR        5           /* insert cursor */
#define TF_OVERWRITE_CURSOR     6           /* overwrite cursor */
#define TF_VINSERT_CURSOR       7           /* virtual insert cursor */
#define TF_VOVERWRITE_CURSOR    8           /* virtual overwrite cursor */
#define TF_PRINT_SPACE          9           /* print space */
#define TF_PRINT_BITEIGHT       10          /* print 8bit character */
#define TF_PRINT_ESCAPE         11          /* print ESC */
#define TF_REPEAT_LAST          12          /* repeat last character #1 times */
#define TF_CURSOR_RIGHT         13          /* move cursor within current line */
#define TF_MOUSE                14          /* mouse type */
#define TF_MOUSECLKMS           15          /* mouse click timeout */
#define TF_WINDOW_SETSIZE       16          /* window size (row/cols) */
#define TF_WINDOW_SETPOS        17          /* window position (row/col) */

#define TF_COLOR                30          /* supports color (> 1, then defines the default depth) */
#define TF_COLORDEPTH           31          /* color depth (8, 16, 88 or 256) */
#define TF_DEFAULT_FG           32          /* default foreground color */
#define TF_DEFAULT_BG           33          /* default background color */
#define TF_SCHEMEDARK           34          /* *true* if the default color is "dark" */
#define TF_COLORSETFGBG         35
#define TF_COLORSET_FG          36          /* color set foreground control sequence */
#define TF_COLORSET_BG          37          /* color set background control sequence */
#define TF_COLORMAP             38          /* color map (terminal) */
#define TF_COLORPALETTE         39          /* color palette (driver) */
#define TF_COLORSCHEME          40          /* current scheme dark or light */

#define TF_CLEAR_IS_BLACK       50          /* clear is black */
#define TF_DISABLE_INSDEL       51          /* disable ins/del scrolling method */
#define TF_DISABLE_SCROLL       52          /* disable scroll regions scrolling method */
#define TF_SCROLL_MAX           53          /* scroll region limit, optimisations */
#define TF_0M_RESETS_COLOR      53          /* 0m reset color (defunc) */
#define TF_EIGHT_BIT            55          /* supports 8bit characters */
#define TF_CODEPAGE             56          /* codepage */
#define TF_NOALTSCREEN          57          /* not used */
#define TF_LAZYUPDATE           58          /* lazy syntax hilite updates */
#define TF_NAME                 59          /* terminal name */
#define TF_ATTRIBUTES           60          /* terminal attribute flags */
#define TF_TTY_FAST             62          /* fast tty optimisations */
#define TF_TTY_GRAPHICSBOX      63          /* graphics mode required for box characters */

#define TF_SCREEN_ROWS          70          /* screen rows */
#define TF_SCREEN_COLS          71          /* screen cols */
#define TF_LINENO_COLUMNS       72
#define TF_WINDOW_MINROWS       73
#define TF_WINDOW_MINCOLS       74

#define TF_XTERM_CURSOR         80          /* XTERM cursor color support */
#define TF_XTERM_TITLE          81          /* XTERM title support */
#define TF_XTERM_COMPAT         82          /* XTERM compatible termuinal */
#define TF_XTERM_PALETTE        83          /* XTERM palette control */

#define TF_VT_DATYPE            90          /* VT/XTERM Devive Attribute Type */
#define TF_VT_DAVERSION         91          /* VT/XTERM Devive Attribute Version */

#define TF_ENCODING             100         /* terminal character encoding */
#define TF_ENCODING_GUESS       101         /* text encoding guess specification */
#define TF_UNICODE_VERSION      102         /* UNICODE version; eg. "6.0.0" */

/*
 *  Terminal characters flags.
 */
#define TC_TOP_LEFT             1
#define TC_TOP_RIGHT            2
#define TC_BOT_LEFT             3
#define TC_BOT_RIGHT            4
#define TC_VERTICAL             5
#define TC_HORIZONTAL           6
#define TC_TOP_JOIN             7
#define TC_BOT_JOIN             8
#define TC_CROSS                9
#define TC_LEFT_JOIN            10
#define TC_RIGHT_JOIN           11
#define TC_SCROLL               12
#define TC_THUMB                13

/*
 *  Terminal attributes flags.
 */
#define TF_AGRAPHICCHARACTERS   0x0000001   /* Graphic characteres (ACS defined) */
#define TF_AFUNCTIONKEYS        0x0000002   /* F1-F10 function keys */
#define TF_ACYGWIN              0x0000004   /* Cygwin native console */
#define TF_AUTF8ENCODING        0x0000010   /* UTF8 character encoding, Unicode implied */
#define TF_AUNICODEENCODING     0x0000020   /* Unicode character encoding */
#define TF_AMETAKEY             0x0000100   /* Meta keys */

/*
 *  Registered macro types
 */
#define REG_TYPED               0           /* Character typed */
#define REG_EDIT                1           /* Different file edited */
#define REG_ALT_H               2           /* ALT-H pressed in response to a prompt */
#define REG_UNASSIGNED          3           /* Unassigned key pressed */
#define REG_IDLE                4           /* Idle time expired */
#define REG_EXIT                5           /* About to exit */
#define REG_NEW                 6           /* New file edited and readin */
#define REG_CTRLC               7           /* CTRL-C (SIGINT) pressed during macro */
#define REG_INVALID             8           /* Invalid key pressed during response input */
#define REG_INTERNAL            9           /* Internal error */
#define REG_MOUSE               10          /* Mouse callback */
#define REG_PROC_INPUT          11          /* Process input available */
#define REG_KEYBOARD            12          /* Keyboard buffer empty */

#define REG_STARTUP             13          /* Startup complete */

#define REG_BOOKMARK            14          /* Bookmark dropped/deleted. */
#define REG_INSERT_MODE         15          /* Insert mode has changed. */

#define REG_BUFFER_MOD          16          /* Buffer has been modified */
#define REG_BUFFER_WRITE        17          /* Buffer write operation */
#define REG_BUFFER_RENAME       18          /* Buffer rename operation */
#define REG_BUFFER_DELETE       19          /* buffer delete operation */

#define REG_FILE_SAVE           20          /* File write request */
#define REG_FILE_WRITTEN        21          /* File write completion */
#define REG_FILE_CHANGE         22          /* File external change */

#define REG_SIGUSR1             23          /* User defined signal */
#define REG_SIGUSR2             24

#define REG_UNDEFINED_MACRO     25          /* Undefined macro */
#define REG_REGRESS             26

#define REG_ABORT               27          /* abort() */


/*
 *  Hilight types (anchors)
 */
#define MK_NONE                 0
#define MK_NORMAL               1
#define MK_COLUMN               2
#define MK_LINE                 3
#define MK_NONINC               4

/*
 *  Character map, special characters
 *      Note, mapped outside valid ASCII/UNICODE to avoid confusion.
 */
#define CMAP_TABSTART           0x7fffff70
#define CMAP_TABVIRTUAL         0x7fffff71
#define CMAP_TABEND             0x7fffff72
#define CMAP_EOL                0x7fffff73
#define CMAP_EOF                0x7fffff74

/*
 *  Special character classes/flags.
 */
#define CMAP_DEFAULT            0
#define CMAP_TAB                1
#define CMAP_BACKSPACE          2
#define CMAP_ESCAPE             3

/*
 *  Window flags
 */
#define WF_HIDDEN               0x00000001  /* Hide the window from view, used to hide nested popups/boss mode etc */
#define WF_NO_SHADOW            0x00000002  /* Turnoff the popups shadow */
#define WF_NO_BORDER            0x00000004  /* Turnoff borders, regardless of the borders() setting */
#define WF_SYSTEM               0x00000008  /* Window is a system window (eg menu) */

#define WF_SHOWANCHOR           0x00000010  /* Show anchor regardless of selection status */
#define WF_SELECTED             0x00000020  /* Hilite the title regardless of selection status */
#define WF_LAZYUPDATE           0x00000040  /* Delay any updates until next refresh() */

#define WF_LINE_NUMBERS         0x00000100  /* Line numbers */
#define WF_LINE_STATUS          0x00000200  /* Line status */
#define WF_EOF_DISPLAY          0x00000400  /* Show <EOF> marker */
#define WF_TILDE_DISPLAY        0x00000800  /* Show <~> marker as EOF marker */

#define WF_HIMODIFIED           0x00001000  /* Hilite modified lines using the 'modified' attribute */
#define WF_HIADDITIONAL         0x00002000  /* Hilite additional lines */
#define WF_HICHANGES            0x00004000  /* Hilite in-line changes */

#define WF_EOL_HILITE           0x00010000  /* Limit hilites to the EOL and not screen width */
#define WF_EOL_CURSOR           0x00020000  /* Limit cursor to EOL */


/*
 *  Window control objects
 */
#define WCTRLO_CLOSE_BTN        0
#define WCTRLO_ZOOM_BTN         1
#define WCTRLO_VERT_SCROLL      2
#define WCTRLO_HORZ_SCROLL      3
#define WCTRLO_VERT_THUMB       4
#define WCTRLO_HORZ_THUMB       5


/*
 *  Window control states
 */
#define WCTRLS_DISABLE          0
#define WCTRLS_ENABLE           1
#define WCTRLS_HIDE             2
#define WCTRLS_SHOW             3
#define WCTRLS_ZOOMED           4

/*
 *  Color values
 *
 *      0..15           PC-STYLE/BRIEF
 *      16...           Extensions
 */
#define BLACK                   0
#define BLUE                    1
#define GREEN                   2
#define CYAN                    3
#define RED                     4
#define MAGENTA                 5
#define BROWN                   6
#define WHITE                   7

#define GREY                    8
#define LTBLUE                  9
#define LTGREEN                 10
#define LTCYAN                  11
#define LTRED                   12
#define LTMAGENTA               13
#define YELLOW                  14
#define LTWHITE                 15

#define DKGREY                  16
#define DKBLUE                  17
#define DKGREEN                 18
#define DKCYAN                  19
#define DKRED                   20
#define DKMAGENTA               21
#define DKYELLOW                22
#define LTYELLOW                23

#define COLOR_NONE              24

/*
 *  get_color flags
 *
 *      [<id>,] [<flags>,] <name> = specification
 *
 *      flags =
 *          0x01        foreground only
 *          0x02        background only
 *          0x04        foreground + background
 *
 *      specification =
 *          foreground [,background] [:style] [sticky]link@[none|<attribute>]
 */
#define COLORGET_FNAME          0x02
#define COLORGET_FVALUE         0x04
#define COLORGET_FFLAGS         0x08
#define COLORGET_NAMES          0x10

/*
 *  Basic color names.
 */
#define COLORS_ALL      \
     "black",           \
     "blue",            \
     "green",           \
     "cyan",            \
     "red",             \
     "magenta",         \
     "brown",           \
     "white",           \
     "grey",            \
     "light-blue",      \
     "light-green",     \
     "light-cyan",      \
     "light-red",       \
     "light-magenta",   \
     "yellow",          \
     "light-white",     \
     "dark-grey",       \
     "dark-blue",       \
     "dark-green",      \
     "dark-cyan",       \
     "dark-red",        \
     "dark-magenta",    \
     "dark-yellow",     \
     "light-yellow",    \
     "none"

/*
 *  List of colors when we are only allowed to specify foreground.
 */
#define COLORS_FG       \
    "black",            \
    "blue",             \
    "green",            \
    "cyan",             \
    "red",              \
    "magenta",          \
    "brown",            \
    "white"

/*
 *  Color enumations/
 *
 *      get_color/set_color basic colors
 */
#define COL_BACKGROUND          0
#define COL_FOREGROUND          1
#define COL_SELECTED_WINDOW     2
#define COL_MESSAGES            3
#define COL_ERRORS              4
#define COL_HILITE_BACKGROUND   5
#define COL_HILITE_FOREGROUND   6
#define COL_BORDERS             7
#define COL_INSERT_CURSOR       8
#define COL_OVERTYPE_CURSOR     9
#define COL_SHADOW              10
#define COL_PROMPT              11
#define COL_COMPLETION          12
#define COL_QUESTION            13
#define COL_ECHOLINE            14
#define COL_STANDOUT            15

/*
 *  Display control flags/
 *
 *      display_mode/inq_display_mode
 */
#define DC_WINDOW               0x00000001  /* Running under a windowing system (read-only) */
#define DC_MOUSE                0x00000002  /* Mouse enabled/available (read-only) */
#define DC_READONLY             0x00000004  /* Read-only mode (read-only) */
#define DC_CHARMODE             0x00000008  /* Character-mode with primitive GUI features available (read-only) */

#define DC_SHADOW               0x00000010  /* Display shadow around popups */
#define DC_SHADOW_SHOWTHRU      0x00000020  /* Show-thru shadow around popups */
#define DC_STATUSLINE           0x00000040  /* Utilise window status-lines, when borderless */

#define DC_UNICODE              0x00000100  /* UNICODE character encoding available (read-only) */
#define DC_ASCIIONLY            0x00000200  /* Only utilise ASCII characters within character-sets/dialogs */

#define DC_ROSUFFIX             0x00001000  /* Read-only suffix on titles */
#define DC_MODSUFFIX            0x00002000  /* Modified suffix */

#define DC_EOF_DISPLAY          0x00010000  /* Show <EOF> marker */
#define DC_TILDE_DISPLAY        0x00020000  /* Show <~> marker */
#define DC_EOL_HILITE           0x00040000  /* Limit hilites to EOL */
#define DC_EOL_CURSOR           0x00080000  /* Limit cursor to EOL */
#define DC_HIMODIFIED           0x00100000  /* Hilite modified lines */
#define DC_HIADDITIONAL         0x00200000  /* Hilite additional lines */

/*
 *  Buffer flags ---
 *      Basic buffer characteristics.
 */
#define BF_CHANGED              0x00000001  /* Changed */
#define BF_BACKUP               0x00000002  /* Backup required on next write */
#define BF_RDONLY               0x00000004  /* Read-only */
#define BF_READ                 0x00000008  /* Buffer content still to be read */
#define BF_EXEC                 0x00000010  /* File is executable */
#define BF_PROCESS              0x00000020  /* Buffer has process attached */
#define BF_BINARY               0x00000040  /* Binary buffer */
#define BF_ANSI                 0x00000080  /* If TRUE, ANSI-fication is done */
#define BF_TABS                 0x00000100  /* Buffer inserts real-tabs */
#define BF_SYSBUF               0x00000200  /* Buffer is a system buffer */
#define BF_LOCK                 0x00000400  /* File lock */
#define BF_NO_UNDO              0x00000800  /* Dont keep undo info */
#define BF_NEW_FILE             0x00001000  /* File is a new file, so write even if no changes */
#define BF_CR_MODE              0x00002000  /* Append <CR> to end of each line on output */
#define BF_SYNTAX               0x00004000  /* Enable syntax highlighting (unless ANSI) */
#define FB_STATUSLINE           0x00008000  /* Status line */
#define BF_MAN                  0x00010000  /* If TRUE, man style \b is done */
#define BF_SPELL                0x00020000  /* Enable spell */
#define BF_FOLDING              0x00040000  /* Test folding/hiding */
#define BF_RULER                0x00080000  /* Display ruler */
#define BF_VOLATILE             0x00100000  /* Buffer is volatile */
#define BF_EOF_DISPLAY          0x00200000  /* Show <EOF> markers */
#define BF_HIDDEN               0x00400000  /* Hidden buffer, from buffer list */
//  BF_AUTOREAD         =0x01000000,            /* Automaticly re-read buffer if underlying changes */
#define BF_AUTOWRITE            0x02000000  /* Automaticly write buffer, if modified */
#define BF_SCRAPBUF             0x04000000  /* Scrap buffer */
//  BF_DELAYED          =0x10000000,            /* Content load delayed until first reference */

    /*
     *  BF2_XXXX values ---
     *      UI formatting control.
     */
#define BF2_ATTRIBUTES          0x00000001  /* Character attributes (ie. charcell level coloring) */
#define BF2_DIALOG              0x00000002  /* Dialog */

#define BF2_CURSOR_ROW          0x00000010  /* Display cursor crosshair */
#define BF2_CURSOR_COL          0x00000020
#define BF2_TILDE_DISPLAY       0x00000040
#define BF2_EOL_HILITE          0x00000080  /* Limit hilites to EOL */

#define BF2_LINE_NUMBERS        0x00000100  /* Line numbers */
#define BF2_LINE_OLDNUMBERS     0x00000200  /* If has line numbers, display old lines */
#define BF2_LINE_STATUS         0x00000400  /* Markup modified lines. */
#define BF2_LINE_SYNTAX         0x00000800  /* Syntax pre-processor flags */

#define BF2_TITLE_FULL          0x00001000  /* Label window using full path name */
#define BF2_TITLE_SCROLL        0x00002000  /* Scroll title with window */
#define BF2_TITLE_LEFT          0x00004000  /* Left justify title */
#define BF2_TITLE_RIGHT         0x00008000  /* Right justify title */

#define BF2_SUFFIX_RO           0x00010000  /* read-only suffix on title */
#define BF2_SUFFIX_MOD          0x00020000  /* modified suffix on title */
#define BF2_EOL_CURSOR          0x00040000  /* Limit cursor to EOL */
#define BF2_EOF_CURSOR          0x00080000  /* Limit cursor to EOF */

#define BF2_HILITERAL           0x00100000  /* Hilite literal characters */
#define BF2_HIWHITESPACE        0x00200000  /* Hilite whitespace */
#define BF2_HIMODIFIED          0x00400000  /* Hilite modified lines */
#define BF2_HIADDITIONAL        0x00800000  /* Hilite added lines */

    /*
     *  BF3_XXXX values ---
     *      Indirect buffer functionality, generally implemented at a macro level.
     */
#define BF3_AUTOSAVE            0x00000001  /* Auto-save */
#define BF3_AUTOINDENT          0x00000002  /* Auto-indent */
#define BF3_AUTOWRAP            0x00000004  /* Auto-wrap */

#define BF3_PASTE_MODE          0x00000010  /* Paste mode, disables a number of auto functions */

    /*
     *  BF4_xxxx values ----
     *      File input/out conversion
     */
#define BF4_OCVT_TRIMWHITE      0x00010000  /* Output conversion, trim trailing whitespace */

/*
 *  Buffer types
 */
#define BFTYP_UNKNOWN           0x0000

#define BFTYP_UNIX              0x0001      /* LF */
#define BFTYP_DOS               0x0002      /* CR/LF */
#define BFTYP_MAC               0x0003      /* CR */
#define BFTYP_BINARY            0x0004      /* <none> */
#define BFTYP_ANSI              0x0005      /* ANSI */
#define BFTYP_EBCDIC            0x0006      /* EBCDIC */

#define BFTYP_UTF8              0x0010      /* UTF8 */
#define BFTYP_UTF16             0x0011      /* UTF16/USC2 */
#define BFTYP_UTF32             0x0012      /* UTF32/USC4 */
#define BFTYP_UTFEBCDIC         0x0015      /* UTF8/EBCDIC (rare) */
#define BFTYP_BOCU1             0x0016      /* Binary Ordered Compression for Unicode */
#define BFTYP_SCSU              0x0017      /* Standard Compression Scheme for Unicode */
#define BFTYP_UTF7              0x0018      /* 7-bit Unicode Transformation Format */

#define BFTYP_GB                0x0020      /* GB */
#define BFTYP_BIG5              0x0021      /* BIG5 */

#define BFTYP_ISO2022           0x0030      /* ISO-2022 */

#define BFTYP_SBCS              0x0090      /* Single Byte */
#define BFTYP_DBCS              0x0091      /* Double Byte */
#define BFTYP_MBCS              0x0092      /* Multi Byte (non-unicode) */

#define BFTYP_OTHER             0x0099      /* Other supported */
#define BFTYP_UNSUPPORTED       0x00ff      /* Known file-type, yet no internal support */

/*
 *  Line terminator types
 */
#define LTERM_UNDEFINED         0x00        /* <unknown/default> */
#define LTERM_NONE              0x01        /* <none> (i.e. binary) */
#define LTERM_UNIX              0x02        /* CR/LF */
#define LTERM_DOS               0x03        /* LF */
#define LTERM_MAC               0x04        /* CR */
#define LTERM_NEL               0x05        /* NEL */
#define LTERM_UCSNL             0x06        /* Unicode next line */
#define LTERM_USER              0xff                    /* User defined */

/*
 *  User line status flags/
 *      note, exclude hi-bit as macrp types are signed.
 */
#define L_USER_MASK      0xfff00000

#define L_MARKED                0x00100008
#define L_USER1                 0x00200000
#define L_USER2                 0x00400000
#define L_USER3                 0x00800000
#define L_USER4                 0x01000000
#define L_USER5                 0x02000000
#define L_USER6                 0x04000000
#define L_USER7                 0x08000000
#define L_USER8                 0x10000000
#define L_USER9                 0x20000000
#define L_USER10                0x40000000

/*
 *  Definitions for the edit_file primitive
 */
#define EDIT_NORMAL             0x0000000   /* Guess file type */
#define EDIT_BINARY             0x0000001   /* Force file to be read in binary mode */
#define EDIT_ASCII              0x0000002   /* Force file to be read in ascii mode */
#define EDIT_STRIP_CR           0x0000004   /* Force CR removal on input and write them on output */
#define EDIT_STRIP_CTRLZ        0x0000008   /* Force Ctrl-Z removal */
#define EDIT_MASK               0x0000008   /* Mask out internals */

#define EDIT_SYSTEM             0x0000010   /* System buffer */
#define EDIT_RC                 0x0000020   /* Extended return codes */
#define EDIT_QUICK              0x0000040   /* Quick detection, removes more costly character-encoding methods */
#define EDIT_AGAIN              0x0000080   /* Edit_again() implementation */

#define EDIT_LOCAL              0x0000100   /* Edit locally, do not rely on the disk being stable */
#define EDIT_READONLY           0x0000200   /* Set as read-only */
#define EDIT_LOCK               0x0000400   /* Kock on read (strict-locking) */
#define EDIT_NOLOCK             0x0000800   /* Disable auto-locking */

#define EDIT_PREVIEW            0x0001000   /* Limit the load to the window size */


/*
 *  Definitions for the write_file primitive
 */
#define WRITE_APPEND            0x0001      /* Append, otherwise overwrite */
#define WRITE_NOTRIGGER         0x0010      /* Do not generate buffer triggers */
#define WRITE_NOREGION          0x0020      /* Ignore any selected region */
#define WRITE_FORCE             0x0040      /* Force write, even if no change */
#define WRITE_BACKUP            0x0080      /* Generate a backup image regardless whether already performed for this edit session */

/*
 *  Definitions for the set/inq_backup_option primitive
 */
#define BK_MODE                 1
#define BK_AUTOSAVE             2
#define BK_DIR                  3
#define BK_DIRMODE              4
#define BK_VERSIONS             5
#define BK_PREFIX               6
#define BK_SUFFIX               7
#define BK_ONESUFFIX            8
#define BK_ASK                  9
#define BK_DONT                 10

/*
 *  Process flags
 */
#define PF_ECHO                 0x0001      /* Local echo on */
#define PF_NOINSERT             0x0002      /* if on, dont insert to process */
#define PF_LOCALECHO            0x0004
#define PF_OVERWRITE            0x4000      /* overwrite process input (CR/LF conversion) */
#define PF_WAITING              0x8000      /* waiting for text */

/*
 *  file_match flags
 */
#define MATCH_TRAILINGWHITE     0x01
        /*  If set, ignores trailing whitespace
        **/

#define MATCH_NOCASE            0x02
        /*  If set, ignore cases during character comparisons
        **/

#define MATCH_AUTOCASE          0x04
        /*  If set, sets NOCASE based on target operating system
        **/

#define MATCH_NOESCAPE          0x08
        /*  If not set, a backslash character (\) in pattern followed by
         *  any other character will match that second character in string.
         *  In particular, "\\" will match a backslash in string.
         *
         *  If set, a backslash character will be treated as an ordinary
         *  character.
        **/

#define MATCH_PATHNAME          0x10
        /*  If set, a slash (/) character in string will be explicitly
         *  matched by a slash in pattern; it will not be matched by either
         *  the asterisk (*) or question-mark (?) special characters, nor by
         *  a bracket ([]) expression
         *
         *  If not set, the slash character is treated as an ordinary
         *  character.
        **/

#define MATCH_PERIODA           0x20        /* asterisk (*) */
#define MATCH_PERIODQ           0x40        /* question mark (?) */
#define MATCH_PERIODB           0x80        /* bracket ([]) */
#define MATCH_PERIOD            0xE0        /* MATCH_PERIODA|MATCH_PERIODQ|MATCH_PERIODB */
        /*  If set, a leading period in string will match a period in
         *  pattern; where the location of "leading" is indicated by the
         *  value of MATCH_PATHNAME as follows:
         *
         *     o is set, a period is "leading" if it is the first character
         *       in string or if it immediately follows a slash.
         *
         *     o is not set, a period is "leading" only if it is the first
         *       character of string.
         *
         *  If not set, no special restrictions are placed on matching a
         *  period.
         *
         *  The three MATCH_PERIODA, MATCH_PERIODQ and MATCH_PERIODB allow
         *  selective control whether the asterisk (*) or question mark (?)
         *  special characters, nor by a bracket ([]) expression have affect.
         *
        **/

/*
 *  iniopen() flags
 */
#define IFILE_STANDARD          0x0001      /* standard comment syntax (;). */
#define IFILE_STANDARDEOL       0x0002      /* ; end-of-ine comments. */
#define IFILE_EXTENDED          0x0004      /* extended syntax (#) comments. */
#define IFILE_EXTENDEDEOL       0x0008      /* ## end-of-line comments. */
#define IFILE_COMMENTSEOL       (IFILE_STANDARDEOL|IFILE_EXTENDEDEOL)
                                                /* EOL comments */

#define IFILE_DUPLICATES        0x0010      /* allow duplicate sections. */
#define IFILE_COLON             0x0020      /* colon (:) as key/value delimiter, otherwise equal (=). */
                                                /* colon (:) and equal (=) as key/value delimiter. */
#define IFILE_EQUALCOLON        (0x0040|IFILE_COLON)
#define IFILE_BACKSLASH         0x0080      /* backslash quoting. */

#define IFILE_QUOTED            0x0100      /* quoted strings. */
#define IFILE_QUOTES            0x0200      /* preserve quotes. */
#define IFILE_CREATE            0x0400      /* create a new image. */
#define IFILE_COMMENTS          0x0800      /* preserve comments. */

/*
 *  re_escape expression modes.
 */
#define RE_BRIEF                0
#define RE_UNIX                 1
#define RE_EXTENDED             2
#define RE_PERL                 3
#define RE_RUBY                 4

#define RE_PERLVARS             2
#define RE_AWKVARS              1

/*
 *  Search and translate flags
 */
#define SF_BACKWARDS            0x00001     /* Search in backwards direction. */
#define SF_IGNORE_CASE          0x00002     /* Ignore/fold case. */
#define SF_ICASE                SF_IGNORE_CASE
#define SF_BLOCK                0x00004     /* Restrict search to current block. */
#define SF_LINE                 0x00008     /* Line mode */

#define SF_LENGTH               0x00010     /* Return length of match. */
#define SF_MAXIMAL              0x00020     /* Maximal/greedy search mode (Non-BRIEF default). */
#define SF_MINIMAL              0x40000     /* Minimal/non-greedy search mode (BRIEF default). */
#define SF_CAPTURES             0x00040     /* Capture elements are retained. */
#define SF_QUIET                0x00080     /* Don't display progress messages. */

#define SF_GLOBAL               0x00100     /* Global translate. */
#define SF_PROMPT               0x00200     /* Prompt for translate changes. */
#define SF_AWK                  0x00400     /* awk(1) style capture references. */
#define SF_PERLVARS             0x00800     /* perl(1) style capture references. */

#define SF_BRIEF                0x00000     /* BRIEF expressions. */
#define SF_CRISP                SF_BRIEF
#define SF_NOT_REGEXP           0x01000     /* Treat as plain test, not as a regular expression. */
#define SF_UNIX                 0x02000     /* Unix regular expressions. */
#define SF_EXTENDED             0x04000     /* POSIX extended syntax. */
#define SF_PERL                 0x08000     /* PERL syntax. */
#define SF_RUBY                 0x10000     /* Ruby syntax. */
#define SF_TRE                  0x20000     /* TRE syntax. */

/*
 *  re_result, special captures
 */
#define CAPTURE_BEFORE          -110
#define CAPTURE_AFTER           -111
#define CAPTURE_ARENA           -112
#define CAPTURE_LAST            -113

/*
 *  Split numeric forms
 */
#define SPLIT_NONUMERIC         0
#define SPLIT_NUMERIC           1
#define SPLIT_NUMERIC_STRTOL    2
#define SPLIT_NUMERIC_STRICT    3

/*
 *  Tokenize flags
 */
        /*  General
        //
        //  o TOK_COLLAPSE_MULTIPLE
        //      Collapes occurrences of the repeated delimiter characters
        //      treating them as single delimiter, in other words empty
        //      elements with the delimited text shall not be returned.
        */
#define TOK_COLLAPSE_MULTIPLE   (1 << 1)

        /*  Numeric field conversion
        //
        //  o TOK_NUMERIC
        //      Fields which begin with a digit are converted into thier
        //      decimal numeric value and returned as integer element rather
        //      than a string.
        //
        //  o TOK_NUMERIC_STROL
        //      Numeric fields are converted within strtol() allowing support
        //      leading base specifications hexidecimal (0x), octal (0) and
        //      binary (0b).
        //
        //  o TOK_NUMERIC_STRICT
        //      Strict conversion of numeric fields where by any invalid
        //      values, for example trailing non-numeric characters, result
        //      in the the field being returned as a string elemment and not
        //      a integer element.
        */
#define TOK_NUMERIC             (1 << 3)
#define TOK_NUMERIC_STRTOL      (1 << 4)
#define TOK_NUMERIC_STRICT      (1 << 5)

        /*  Parsing options
        //
        //  o TOK_WHITESPACE
        //      Allow leading and trailng whitespace around quoted and
        //      numeric element.
        //
        //  o TOK_BACKSLASHES
        //      Allow backslahes to escape the meaning of any delimiter
        //      characters and both single and double.
        //
        //  o TOK_ESCAPE
        //      Enable backslash escape sequence processing.
        //
        //  o TOK_ESCAPEALL
        //      Control the behaviour of TOK_ESCAPE to escape all characters
        //      preceeded with a backslashes, otherwise by default unknown
        //      escape sequences are ignored.
        */
#define TOK_WHITESPACE          (1 << 6)    /* permit leading/trailing whitespace */
#define TOK_BACKSLASHES         (1 << 7)    /* backslashes */
#define TOK_ESCAPE              (1 << 8)    /* escape sequence support */
#define TOK_ESCAPEALL           (1 << 9)    /* escape 'all' characters */

        /*  Quote options
        //
        //  o TOK_DOUBLE_QUOTES
        //      Enables double quote support where all characters enclosed
        //      within a pair of matching quotes are treated as a single
        //      element including any embedded delimiters.
        //
        //  o TOK_DOUBLE_QUOTES
        //      Same as TOK_DOUBLE_QUOTES but enables single quote support.
        //
        //  o TOK_QUOTE_STRINGS
        //      When single or double quoted support is enabled allow the element
        //      is be enclosed within a extra pair of quotes, for example
        //
        //          ""hello world"".
        //
        //  o TOK_PRESERVE_QUOTES
        //      When an element is enclosed in quotes and the quote character
        //      is specified in 'delims' then the returned element shall also be
        //      enclosed within the enountered quotes.
        */
#define TOK_DOUBLE_QUOTES       (1 << 10)   /* "xxxx" */
#define TOK_SINGLE_QUOTES       (1 << 11)   /* 'xxxxx' */
#define TOK_QUOTE_STRINGS       (1 << 12)   /* ""xxxxx"" */
#define TOK_PRESERVE_QUOTES     (1 << 13)

        /*  Result processing options
        //
        //  o TOK_TRIM_LEADING
        //      Remove any leading whitespace from non-quoted string elements.
        //      Whitespace is defined as any space, tab or newline character unless
        //      they exist within the set of specified delimiters.
        //
        //  o TOK_TRIM_TRAILING
        //      Remove any trailing whitespace from string elements.
        //
        //  o TOK_TRIM
        //      Remove any leading and trailing whitespace characters.
        //
        //  O TOK_TRIM_QUOTED
        //      Apply trim logic to quoted strings.
        */
#define TOK_TRIM_LEADING        (1 << 14)
#define TOK_TRIM_TRAILING       (1 << 15)
#define TOK_TRIM                (TOK_TRIM_LEADING|TOK_TRIM_TRAILING)
#define TOK_TRIM_QUOTED         (1 << 16)

/*
 *  Spell control options
 */
#define SPELL_ADD               0x0001
#define SPELL_IGNORE            0x0002
#define SPELL_REPLACE           0x0003

#define SPELL_SAVE              0x0100
#define SPELL_LOAD              0x0101

#define SPELL_LANG_ADD          0x0200
#define SPELL_LANG_PRIMARY      0x0201
#define SPELL_LANG_REMOVE       0x0202

#define SPELL_DESCRIPTION       0x0300
#define SPELL_DICTIONARIES      0x0301

/*
 *  Dialog widget classes
 *      Note, not all are implemented at the time; shall occur on a as needed basic.
 */
#define DLGC_MIN                0x2000
#define DLGC_NULL               0x2000

#define DLGC_CONTAINER          0x2001      /* Widget container */
#define DLGC_GROUP              0x2002      /* Group start */
#define DLGC_TAB                0x2003      /* Tab panel */
#define DLGC_END                0x200f      /* End of current container */

#define DLGC_PUSH_BUTTON        0x2011      /* Push button */
#define DLGC_RADIO_BUTTON       0x2012      /* Radio button */
#define DLGC_CHECK_BOX          0x2013      /* Check box */
#define DLGC_TOGGLE             0x2014      /* Toggle button */
#define DLGC_LABEL              0x2015      /* label */
#define DLGC_LIST_BOX           0x2016      /* List box */
#define DLGC_EDIT_FIELD         0x2017      /* Edit field */
#define DLGC_NUMERIC_FIELD      0x2018      /* Numeric edit field */
#define DLGC_COMBO_FIELD        0x2019      /* Edit field and drop list */

#define DLGC_SPACER             0x2030      /* Display spacer */
#define DLGC_SEPARATOR_HORIZONTAL 0x2031
#define DLGC_SEPARATOR_VERTICAL 0x2032

#define DLGC_TREE               0x2040      /* *not* implemented */
#define DLGC_GAUGE              0x2041      /* *not* implemented */
#define DLGC_SLIDER             0x2042      /* *not* implemented */
#define DLGC_VSCROLLBAR         0x2043      /* *not* implemented */
#define DLGC_HSCROLLBAR         0x2044      /* *not* implemented */
#define DLGC_GRID               0x2070      /* *not* implemented */
#define DLGC_MAX                0x2100

/*
 *  Dialog widget attributes.
 */
#define DLGA_MIN                0x3000
#define DLGA_NULL               0x3000      /* MIN */

#define DLGA_TITLE              0x3001      /* Title for certain widgets. */
#define DLGA_NAME               0x3002      /* Widget name. */
#define DLGA_IDENT              0x3003      /* Identifier (user supplied). */
#define DLGA_CALLBACK           0x3004      /* User callback function/macro. */
#define DLGA_HELP               0x3005      /* Help topic/command. */
#define DLGA_TOOLTIP            0x3006      /* Tooltip text. */
#define DLGA_X                  0x3007
#define DLGA_Y                  0x3008
#define DLGA_COLS               0x3009
#define DLGA_ROWS               0x300a
#define DLGA_VERSION            0x300b      /* Wiget specific version/feature set identifier */

#define DLGA_ATTACH_BOTTOM      0x3010      /* Attachment of widget within dialog box. */
#define DLGA_ATTACH_TOP         0x3011
#define DLGA_ATTACH_LEFT        0x3012
#define DLGA_ATTACH_RIGHT       0x3013

#define DLGA_ALIGN_N            0x3020      /* Alignment of widget within frame. */
#define DLGA_ALIGN_NE           0x3021
#define DLGA_ALIGN_E            0x3022
#define DLGA_ALIGN_SE           0x3023
#define DLGA_ALIGN_S            0x3024
#define DLGA_ALIGN_SW           0x3025
#define DLGA_ALIGN_W            0x3026
#define DLGA_ALIGN_NW           0x3027
#define DLGA_ALIGN_CENTER       0x3028

#define DLGA_ALLOW_RESIZE       0x3030
#define DLGA_ALLOW_EXPAND       0x3031
#define DLGA_ALLOW_FILLX        0x3032
#define DLGA_ALLOW_FILLY        0x3033
#define DLGA_PROPAGATE          0x3034

#define DLGA_CANCEL_BUTTON      0x3040
#define DLGA_DEFAULT_BUTTON     0x3041
#define DLGA_DEFAULT_FOCUS      0x3042      /* Move cursor to specified widget. */
#define DLGA_ACTIVATES_DEFAULT  0x3043      /* Whether <enter> actives default button */
#define DLGA_AUTOMOVE           0x3044      /* Auto-move status */

#define DLGA_VALUE              0x3050      /* Set value of a check/radio/list-box/edit-field/combo-box. */
#define DLGA_LABEL              0x3051      /* Label text. */

#define DLGA_ACCELERATOR        0x3060      /* Accelerator key. */
#define DLGA_SENSITIVE          0x3061      /* Sensitive to input. */
#define DLGA_GREYED             0x3062      /* Greyed, non-sensitive to input */
#define DLGA_ACTIVE             0x3063      /* Active, sensitive to input */
#define DLGA_PADX               0x3064
#define DLGA_PADY               0x3065
#define DLGA_ORIENTATION        0x3066      /* Vertical (0)/horizontal (1) */
#define DLGA_HIDDEN             0x3067
#define DLGA_VISIBLE            0x3068
#define DLGA_KEYDOWN            0x3069      /* Control keydown event */
#define DLGA_TABSTOP            0x306a
#define DLGA_HOTKEY             0x306b

#define DLGA_TEXT_ONLY          0x3070      /* Text dialog widgets only. */
#define DLGA_GUI_ONLY           0x3071      /* GUI dialog widgets only. */

#define DLGA_EDEDITABLE         0x3080
#define DLGA_EDMAXLENGTH        0x3081      /* Maximum length of the entry (limit 32k) */
#define DLGA_EDVISIBLITY        0x3082      /* Content visible status (eg. password entry) */
#define DLGA_EDINVISIBLECHAR    0x3083      /* Character displayed in place of the real characters */
#define DLGA_EDPOSITION         0x3084      /* Cursor position within the the text entry */
#define DLGA_EDPLACEHOLDER      0x3085      /* Placeholder text, displayed when the field is empty and unfocused */
#define DLGA_EDVALIDATE         0x3086      /* Validate events */

#define DLGA_NUMDIGITS          0x3090      /* Number of decimal points (default = 0) */
#define DLGA_NUMMIN             0x3091
#define DLGA_NUMMAX             0x3092
#define DLGA_NUMINCREMENT       0x3093
#define DLGA_NUMWRAP            0x3094
#define DLGA_NUMSNAP            0x3095

    /*
    //  DLGA_CBEDITABLE
    //      Determines whether the combo-field is editable.  An editable combo-box allows the user to type
    //      into the field or select an item from the list to initialise the field, after which it can be
    //      edited.,  An non-editable field displays the selected item in the field, but the selection
    //      cannot be modified.
    //
    //  DLGA_CBRELAXED
    //      Determines whether an editable combo-field may contain text which is not restricted to the
    //      values contained within the collection.
    //
    //  DLGA_CBPOPUPMODE
    //      -1  = Hidden.
    //      0   = Normal.
    //      1   = Open.
    //      2   = Focus.
    //
    //  DLGA_CBAUTOCOMPLETE
    //      0=None
    //          Disables the automatic completion feature for the combo-box and edit-field controls.
    //
    //      1=Suggest
    //          Displays the auxiliary drop-down list associated with the edit control. The drop-down
    //          is populated with one or more suggested completion strings.
    //
    //      2=Append
    //          Appends the remainder of the most likely candidate string to the existing characters,
    //          highlighting the appended characters.
    //
    //      3=SuggestAppend
    //          Applies both Suggest and Append options.
    */
#define DLGA_CBEDITABLE         0x30a1
#define DLGA_CBRELAXMODE        0x30a2
#define DLGA_CBAUTOCOMPLETEMODE 0x30a3
#define DLGA_CBPOPUPMODE        0x30a4      /* Popup mode. */
#define DLGA_CBPOPUPSTATE       0x30a5      /* Popup status. */

#define DLGA_LBCOUNT            0x30b0      /* Item count. */
#define DLGA_LBELEMENTS         0x30b1      /* Collection elements. */
#define DLGA_LBINSERT           0x30b2      /* Insert an item. */
#define DLGA_LBREMOVE           0x30b3      /* Remove an item. */
#define DLGA_LBCLEAR            0x30b4      /* Clear all items. */
#define DLGA_LBSORTMODE         0x30b5
#define DLGA_LBHASSTRINGS       0x30b6
#define DLGA_LBICASESTRINGS     0x30b7

#define DLGA_LBCURSOR           0x30c0      /* Item under cursor. */
#define DLGA_LBACTIVE           0x30c1      /* Active/selected item(s). */
#define DLGA_LBDISPLAYTEXT      0x30c2
#define DLGA_LBDISPLAYTEXTLEN   0x30c3
#define DLGA_LBTEXT             0x30c4
#define DLGA_LBTEXTLEN          0x30c5
#define DLGA_LBPAGEMODE         0x30c6
#define DLGA_LBINDEXMODE        0x30c7
#define DLGA_LBROWS             0x30c8
#define DLGA_LBCOLS             0x30c9
#define DLGA_LBCOLUMNS          0x30ca
#define DLGA_LBWIDTH            0x30cb
#define DLGA_LBDUPLICATES       0x30cc

#define DLGA_GAUGEMIN           0x30e0      /* Minimum value. */
#define DLGA_GAUGEMAX           0x30e1      /* Maximum value. */


/*
 *  Dialog callback events

    DLGE_KEYDOWN

    DLGE_COMMAND
        The DLGE_COMMAND message is sent when the user selects a command
        item from a menu, when a control sends a notification message to
        its parent window, or when an accelerator is encountered.

            p1 - If the message is from an accelerator, this value is 1.
            If the message is from a menu, this value is zero.

            p2 - Menu identifier which caused the event, otherwise the
            key code.

    DLGE_HELP
        Indicates that the user pressed the F1 key.

        If a menu is active when F1 is pressed, WM_HELP is sent to the
        window associated with the menu; otherwise, WM_HELP is sent to
        the widget that has the keyboard focus. If no widget has the
        focus, WM_HELP is sent to the currently active window.

 */
#define DLGE_INIT               0           /* Initlisation */
#define DLGE_CANCEL             1           /* Dialog cancelled */
#define DLGE_BUTTON             2           /* Button selected */
#define DLGE_VALIDATE           3           /* Validation */
#define DLGE_CHANGE             4           /* Object value change */
#define DLGE_SELECT             5           /* Selection (focus/unfocus) */
#define DLGE_KEYDOWN            6           /* Keydown event */
#define DLGE_COMMAND            7           /* Accelerator/Menu command */
#define DLGE_HELP               8           /* Help event */

/*
 *  create_notice
 *      Button types.
 */
#define DLMB_OK                 0x0001
#define DLMB_YESNO              0x0010
#define DLMB_RETRY              0x0100
#define DLMB_CANCEL             0x1000
#define DLMB_OKCANCEL           (DLMB_OK|DLMB_CANCEL)
#define DLMB_YESNOCANCEL        (DLMB_YESNO|DLMB_CANCEL)
#define DLMB_RETRYCANCEL        (DLMB_RETRY|DLMB_CANCEL)

/*
 *  create_notice
 *      Button identifiers.
 */
#define DLIDFIRST               1
#define DLIDSECOND              2
#define DLIDTHIRD               3
#define DLIDABORT               100
#define DLIDCANCEL              101
#define DLIDRETRY               102
#define DLIDNO                  103
#define DLIDYES                 104

/*
 *  echo line flags.
 */
#define E_LINE                  0x0001      /* Line: ..                     */
#define E_COL                   0x0002      /* Col: ..                      */
#define E_PERCENT               0x0004      /* nn%                          */
#define E_TIME                  0x0008      /* hh:mm a/pm                   */
#define E_REMEMBER              0x0010      /* RE / PA String.              */
#define E_CURSOR                0x0020      /* IN / OV cursor type.         */
#define E_FROZEN                0x0040      /* Echo line is frozen, ie not updated. */
#define E_VIRTUAL               0x0080      /* Virtual character indicator  */
#define E_CHARVALUE             0x0100      /* Character value              */
#define E_TIME24                0x1000      /* Time in 24hour form (HH:MM)  */
#define E_FORMAT                0x8000      /* Format override active       */

/*
 *  Mouse position
 */
#define MOBJ_NOWHERE            0           /* Not in any window */

#define MOBJ_LEFT_EDGE          1           /* Left bar of window */
#define MOBJ_RIGHT_EDGE         2           /* Right bar of window */
#define MOBJ_TOP_EDGE           3           /* Top line of window */
#define MOBJ_BOTTOM_EDGE        4           /* Bottom line of window */
#define MOBJ_INSIDE             5           /* Mouse inside window */
#define MOBJ_TITLE              6           /* On title */

#define MOBJ_VSCROLL            20          /* Vertical scroll area */
#define MOBJ_VTHUMB             21          /* Vertical scroll area */
#define MOBJ_HSCROLL            22          /* Horz scroll area */
#define MOBJ_HTHUMB             23          /* Horz scroll area */

#define MOBJ_ZOOM               30          /* Zoom button */
#define MOBJ_CLOSE              31          /* Close */
#define MOBJ_SYSMENU            32                 /* System Menu */

/*
 *  Syntax rules/types
 */
#define SYNT_COMMENT            1
#define SYNT_PREPROCESSOR       2

#define SYNT_QUOTE              10
#define SYNT_CHARACTER          11
#define SYNT_STRING             12
#define SYNT_LITERAL            13

#define SYNT_HTML               20
#define SYNT_BRACKET            21

#define SYNT_OPERATOR           30
#define SYNT_DELIMITER          31
#define SYNT_WORD               40
#define SYNT_NUMERIC            41
#define SYNT_KEYWORD            42

#define SYNT_FORTRAN            100
#define SYNT_CSTYLE             101
#define SYNT_LINECONT           102
#define SYNT_LINEJOIN           103

/*
 *  Syntax flags
 *
 *      Flag                        Description
 *  ----------------------------------------------------------------------------
 *      SYNF_CASEINSENSITIVE        Case insensitive language tokens.
 *      SYNF_FORTRAN                FORTRAN style language.
 *      SYNF_STRING_ONELINE         String definitions don't continue over line breaks.
 *      SYNF_LITERAL_NOQUOTES       Literal strings don't translate quoted characters.
 *      SYNF_STRING_MATCHED         String open/close must be matched; otherwise ignored.
 *
 *      SYNF_COMMENTS_LEADINGWS     xxx
 *      SYNF_COMMENTS_TRAILINGWS    xxx
 *      SYNF_COMMENTS_QUOTE         xxx
 *      SYNF_COMMENTS_CSTYLE        C-style comments.
 *
 *      SYNF_PREPROCESSOR_WS        xxx
 *      SYNF_LINECONT_WS            xxx
 *      SYNF_MANDOC                 xxx
 *
 *      SYNF_HILITE_WS              Hilite white-space.
 *      SYNF_HILITE_LINECONT        Hilite line continuations.
 *      SYNF_HILITE_PREPROCESSOR    Hilite preprocessor directives.
 *
 *      SYNF_SPELL_WORD             Enable word spell check.
 *      SYNF_SPELL_COMMENT          Enable comment spell check.
 *
 */
#define SYNF_CASEINSENSITIVE    0x0001
#define SYNF_FORTRAN            0x0002
#define SYNF_STRING_ONELINE     0x0004
#define SYNF_LITERAL_NOQUOTES   0x0008
#define SYNF_STRING_MATCHED     0x4000

#define SYNF_COMMENTS_LEADINGWS 0x0010
#define SYNF_COMMENTS_TRAILINGWS 0x0020
#define SYNF_COMMENTS_QUOTE     0x0040
#define SYNF_COMMENTS_CSTYLE    0x0080

#define SYNF_PREPROCESSOR_WS    0x0100
#define SYNF_LINECONT_WS        0x0200
#define SYNF_MANDOC             0x0400

#define SYNF_HILITE_WS          0x1000
#define SYNF_HILITE_LINECONT    0x2000
#define SYNF_HILITE_PREPROCESSOR 0x0400

#define SYNF_SPELL_WORD         0x1000
#define SYNF_SPELL_COMMENT      0x2000

/*
 *  Keywords, standard table usage
 *
 *      Attribute                   Description
 *  ----------------------------------------------------------------------------
 *      SYNK_PRIMARY(0)             Language reserved words.
 *      SYNK_FUNCTIONS              Standard definitions, functions etc.
 *      SYNK_EXTENSIONS             Extensions.
 *      SYNK_TYPE                   Types.
 *      SYNK_STORAGECLASS           Storage classes.
 *      SYNK_DEFINITION             Definitions.
 *      SYNK_CONDITIONAL            Conditional statements.
 *      SYNK_REPEAT                 Repeat statements.
 *      SYNK_EXCEPTION              Exception.
 *      SYNK_DEBUG                  Debug statements.
 *      SYNK_LABEL                  Label's.
 *      SYNK_STRUCTURE              Structure definitions.
 *      SYNK_TYPEDEF                Type definitions.
 *      SYNK_CONSTANTS              System constants.
 *      SYNK_OPERATOR               Operators.
 *      SYNK_BOOLEAN                Boolean constants.
 *
 *      SYNK_PREPROCESSOR           Preprocessor primitives.
 *      SYNK_PREPROCESSOR_INCLUDE   Preprocessor #include primitive.
 *      SYNK_PREPROCESSOR_DEFINE    Preprocessor #define primitive.
 *      SYNK_PREPROCESSOR_COND      Preprocessor conditional primitives.
 *
 *      SYNK_TODO                   Magic comment keywords, additional dictionary words.
 *      SYNK_MARKUP                 Comment markups (e.g. doxygen).
 */
enum {
    SYNK_PRIMARY,
    SYNK_FUNCTION,
    SYNK_EXTENSION,
    SYNK_TYPE,
    SYNK_STORAGECLASS,
    SYNK_DEFINITION,
    SYNK_CONDITIONAL,
    SYNK_REPEAT,
    SYNK_EXCEPTION,
    SYNK_DEBUG,
    SYNK_LABEL,
    SYNK_STRUCTURE,
    SYNK_TYPEDEF,
    SYNK_CONSTANT,
    SYNK_OPERATOR,
    SYNK_BOOLEAN,

    SYNK_PREPROCESSOR,
    SYNK_PREPROCESSOR_INCLUDE,
    SYNK_PREPROCESSOR_DEFINE,
    SYNK_PREPROCESSOR_CONDITIONAL,

    SYNK_TODO,
    SYNK_MARKUP,

    SYNK_MAX
};

/*
 *  Keywords flags.
 *
 *      Flag                        Description
 *  ----------------------------------------------------------------------------
 *      SYNF_IGNORECASE             Ignore case.
 *      SYNK_NATCHCASE              Match case.
 *      SYNF_PATTERN                Pattern match (glob style).
 */
enum {
    SYNF_IGNORECASE         = 1,
    SYNK_NATCHCASE          = 2,
    SYNF_PATTERN            = 4
};


/*
 * Name mangling, compat for older macros
 */
#define PF_WAIT                 PF_WAITING
#define BF_READONLY             BF_RDONLY
#define EDIT_CR                 EDIT_STRIP_CR


/*
 *  Manifest constants returned by inq_mode
 */
#define MODE_OVERTYPE           0
#define MODE_INSERT             1


/*
 *  Flags for select_buffer() macro.
 */
#define SEL_NORMAL              0x0000          /* Text is not centered */
#define SEL_CENTER              0x0001          /* Text is centered */
#define SEL_TOP_OF_WINDOW       0x0002          /* Make indicated line top of window */


/*
 *  File Mode Bits
 */
extern const int                S_IFMT;
extern const int                S_IFDIR;
extern const int                S_IFCHR;
extern const int                S_IFIFO;
extern const int                S_IFREG;
extern const int                S_IFLNK;
extern const int                S_IFSOCK;

extern const int                S_IRUSR;
extern const int                S_IWUSR;
extern const int                S_IXUSR;
extern const int                S_IRGRP;
extern const int                S_IWGRP;
extern const int                S_IXGRP;
extern const int                S_IROTH;
extern const int                S_IWOTH;
extern const int                S_IXOTH;

extern const int                S_ISUID;
extern const int                S_ISGID;
extern const int                S_ISVTX;

/*
 *  File open flags
 */
extern const int                O_CREAT;
extern const int                O_EXCL;
extern const int                O_RDONLY;
extern const int                O_RDWR;
extern const int                O_TRUNC;
extern const int                O_WRONLY;
extern const int                O_BINARY;

/*
 *  access
 */
extern const int                F_OK;
extern const int                R_OK;
extern const int                W_OK;
extern const int                X_OK;

/*
 *  seek
 */
extern const int                SEEK_SET;
extern const int                SEEK_CUR;
extern const int                SEEK_END;


/*
 *  System values
 */
extern const int                current_buffer;
extern const int                current_window;
extern const int                current_line;
extern const int                current_col;

/*
 *  errno
 *
 *      TODO - publish base error number manifest constants
 */
extern int                      errno;


/*
 *  Simple debug
 */
#if defined(MACRO_DEBUG)
#define DBG(x)                  debug_pause(x)
#else
#define DBG(x)
#endif


/*
 *  Following definitions are used to support the
 *  BRIEF macro names for compatability.
 */
#define inq_environment(s)      getenv(s)


/*
 *  System type (OS/2, Win32, MACOSX, DOS and UNIX)
 *
 *      TODO - change DELIM, SLASH and DIRSEP to const system values
 */
extern const string             CRISP_OPSYS;

extern string                   CRISP_DELIM;
extern string                   CRISP_SLASH;
extern string                   CRISP_DIRSEP;

/*
 *  Profile and configuration names.
 */
extern const string             GRRESTORE_FILE;
extern const string             GRSTATE_FILE;
extern const string             GRSTATE_DB;
extern const string             GRINIT_FILE;
extern const string             GRLOG_FILE;

extern const string             GREXTENSION;
extern const string             GRPROGNAME;

extern const int                GRVERSIONMAJOR;
extern const int                GRVERSIONMINOR;

extern const string             C_TERM_CHR;

/*
 *  Advanced save/restore functionality
 */
#define save_excursion()        save_position()
#define restore_excursion()     restore_position(2)


/*
 *  Prototypes
 */
#if defined(__PROTOTYPES__)
                                /*grief.cr*/
extern string                   inq_grinit(void);
extern string                   grinit_query(string section, string key);
extern int                      grinit_update(string section, string key, string value);
extern void                     grinit_onload(void);
extern void                     grinit_onexit(void);

extern void                     load_indent(void);
extern void                     load_compile(void);
extern void                     load_package(void);
extern void                     clear_buffer(void);

                                /*modeline.cr*/
extern void                     mode(~string);
extern void                     modeline(void);

                                /*colors.cr*/
extern void                     coloriser(~ string);
extern int                      colorscheme(~ string scheme, ...);
extern string                   inq_coloriser(void);

#define VIM_16DEPTH             (1 << 1)
#define VIM_88DEPTH             (1 << 2)
#define VIM_256DEPTH            (1 << 3)
#define VIM_GUIDEPTH            (1 << 4)

extern int                      vim_colorscheme(string label, int colors, ~string base, list spec, int asgui);
extern int                      vim_colorschemex(string label, int colors, ~string base, list spec, int asgui, int &gui);

                                /*command.cr*/
extern string                   fixslash(string str);
extern int                      perform_command(string cmd, ~string header, ~int, ~int);

                                /*select.cr*/
extern void                     select_editable(void);
extern int                      select_list(string title, string message_string, int step,
                                    declare l, ~int flags, ~declare help_var, ~list do_list, ~int, ~string);
extern int                      select_slim_list(string title, string message_string,
                                    declare l, int flags, ~declare help_var, ~string do_list, ~int step);
extern int                      select_buffer(int buf, int win, ~int flags, ~declare, ~list do_list,
                                    ~declare help_list, ~int start_line, ~int keep_window);

extern void                     sel_down(void);
extern void                     sel_end(void);
extern void                     sel_enter(void);
extern void                     sel_esc(void);
extern void                     sel_exit(~ int retval);
extern void                     sel_home(void);
extern void                     sel_pgdn(void);
extern void                     sel_pgup(void);
extern void                     sel_up(void);
extern void                     sel_warp(void);

extern void                     buffer_list(~int shortmode, ~int sysbuffers);

extern string                   select_file(string wild_card, ~string title, int dirs);
extern list                     select_files(string wild_card, ~string title, int dirs);

extern int                      sized_window(int lines, int width, ~string msg, ~int, ~int);

extern list                     field_list(string title, list result, list arg, ~int, int escnull = 0);

                                /*extra.cr*/
extern string                   buftype_description(int type);

                                /*debug.cr*/
extern void                     trace(~string);
extern void                     vars(~int);
extern void                     bvars(~int);
extern int                      inq_nest_level(void);

extern void                     __dbg_init(void);
extern void                     __dbg_trace__(~int, ~string, ~string);

                                /*feature.cr*/
extern int                      select_feature(list lst, int width, ~int dohelp);

                                /*ff.cr*/
extern string                   ff(~string, ~string, ~int);
extern string                   dir(~string, ~string);
extern string                   tree(~string);
extern void                     treecd(~string);
extern string                   bs(~string, ...);
extern string                   ts(~string, ~string, ...);

                                /*scrblank.cr*/
extern void                     scrblank(string arg);
extern void                     scr__blank(void);
extern void                     screen_blank(void);

                                /*restore.cr*/
extern void                     save_state(void);

                                /*autosave.cr*/
extern void                     autosave_disable(string filename);
extern int                      autosave_state(void);

                                /*search.cr*/
extern int                      search_hilite(int match_len);
extern void                     search_options(void);
extern void                     translate__fwd(void);
extern void                     translate__back(void);
extern void                     translate_again(void);
extern void                     search__fwd(void);
extern void                     search__back(void);
extern void                     search_next(void);
extern void                     search_prev(void);
extern void                     i_search(void);

                                /*setcolor.cr*/
extern void                     setcolor(~ string mode);

                                /*objects.cr*/
extern void                     objects(string function, ~declare arg1);
extern void                     shift_right(void);
extern void                     shiftr(void);
extern void                     shift_left(~declare);
extern void                     shiftl(~declare);

extern void                     default_shift_right(void);
extern void                     default_shift_left(void);
extern void                     default_delete_word_right(void);
extern void                     default_delete_word_left(void);
extern int                      default_word_left(void);
extern void                     default_word_right(void);

extern void                     delete_word(~declare);
extern int                      word_left(string pat);
extern void                     word_right(void);

                                /*options.cr*/
extern void                     options(void);
extern void                     echo_line_options(void);
extern void                     tab_options(void);
extern void                     indent_options(void);

                                /*region.cr*/
extern void                     block_upper_case(void);
extern void                     block_lower_case(void);
extern void                     block_delete(void);

                                /*history.cr*/
extern int                      _inq_history(void);
extern void                     _prompt_begin(void);
extern void                     _prompt_end(void);
extern void                     prompt_help(void);

extern string                   completion(string word, string prompt);
extern string                   compl_buffer(int what);
extern string                   compl_paste(void);
extern string                   compl_cmd(~string file);
extern string                   compl_readfile(~string file);
extern string                   compl_editfile(~string file);
extern string                   compl_bookmark(~string str);
extern string                   compl_cd(~string cmd);
extern string                   compl_history(~string str);

                                /*help.cr*/
extern void                     help(void);
extern int                      explain(~string,...);
extern void                     cshelp(string dir, string topic);
extern string                   help_resolve(string filename);
extern void                     help_display(string file, string title, declare section);
extern string                   help_window(int type, int buf, int lines, int width,
                                        int initial_line, ~int level, ~string msg);

                                /*indent.cr*/
extern void                     _slide_in();
extern void                     _slide_out();

                                /*menu.cr*/
extern void                     menu(void);
extern void                     menubar(void);
extern void                     menuon(void);
extern void                     menuoff(void);

                                /*mouse.cr*/
extern void                     mouse_enable(void);
extern void                     mouse_disable(void);
extern void                     mouse_buttons_enable(void);

                                /*misc.cr*/
extern void                     _indent(void);
extern string                   search_path(string path, string filename);
extern string                   add_to_path(string path, string name);
extern void                     delete_curr_buffer(void);
extern void                     edit_next_buffer(void);
extern void                     edit_previous_buffer(void);
extern void                     previous_edited_buffer(void);
extern void                     previous_alpha_buffer(void);
extern void                     redo(void);
extern void                     insert_tab(void);
extern void                     insert_backtab(void);
extern void                     previous_tab(void);
extern void                     tab_to_col(int col);
extern void                     display_file_name(void);
extern void                     repeat(void);
extern void                     home(void);
extern void                     end(void);
extern void                     quote(void);
extern string                   sub(string r, string s, string t);
extern string                   gsub(string r, string s, string t);
extern void                     join_line(void);
extern void                     delete_character(void);
extern void                     force_input(string str);

                                /*keys.cr*/
extern void                     _back_tab(void);
extern void                     _open_line(void);

                                /*set.cr*/
extern void                     set(~ string, ~string);
extern void                     setenv(~string);
extern string                   inq_env(~string, ~int);

                                /*shell,cr*/
extern int                      create_shell(string shell_path, string buffer_name, ...);
extern void                     sh_char_mode(void);
extern void                     sh_next_cmd(void);
extern void                     sh_line_mode(void);

                                /*tabs.cr*/
extern void                     show_tabs(void);
extern string                   detab_str(string line);
extern string                   entab_str(string line);

                                /*telnet.cr*/
extern void                     telnet(~string);
extern void                     rlogin(~string);
extern void                     ftp(~string);
extern void                     ncftp(~string);
extern string                   get_host_entry(void);

                                /*text.cr*/
extern void                     wc(void);
extern void                     grep(~string, ~string);
extern void                     fgrep(~string, ~string);
extern void                     egrep(~string, ~string);

                                /*view.cr*/
extern void                     view(string arg);
extern void                     literal(void);

                                /*wp.cr*/
extern int                      autoindent(~string arg);

                                /*window.cr*/
extern void                     goto_left_edge(void);
extern void                     goto_right_edge(void);
extern void                     set_top_of_window(void);
extern void                     set_bottom_of_window(void);
extern void                     set_center_of_window(void);

                                /*zoom.cr*/
extern void                     zoom(void);
extern void                     unzoom(void);
#endif

/*end*/
#endif /*MACSRC_GRIEF_H_INCLUDED*/

