#include <edidentifier.h>
__CIDENT_RCSID(gr_echo_c,"$Id: echo.c,v 1.71 2021/07/18 23:03:19 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: echo.c,v 1.71 2021/07/18 23:03:19 cvsuser Exp $
 * Command/echo line implementation/interface.
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
#include <stdarg.h>
#include <edalt.h>
#include <libstr.h>                             /* str_...()/sxprintf() */

#include "m_echo.h"                             /* public interface */
#include "echo.h"

#include "accum.h"
#include "buffer.h"
#include "builtin.h"
#include "cmap.h"
#include "color.h"
#include "debug.h"
#include "display.h"
#include "eval.h"
#include "file.h"
#include "getkey.h"
#include "keyboard.h"
#include "kill.h"
#include "m_time.h"
#include "macros.h"                             /* macro_lookup */
#include "mchar.h"
#include "main.h"
#include "map.h"
#include "playback.h"
#include "register.h"
#include "symbol.h"                             /* argv_ ... */
#include "system.h"                             /* sys_... */
#include "tty.h"

#define MSG_SIZE        128                     /* cache content of message, display optimisation */

#define P_ECHOLINE      0x01
#define P_LINECOL       0x02

struct _estate {
    MAGIC_t     magic;
#define ESTATE_MAGIC    MKMAGIC('E','s','t','E')

    time_t      tm_time;
    int         tm_year;
    int         tm_month;
    int         tm_day;
    int         tm_hour;
    int         tm_min;
    int         bf_line;
    int         bf_col;
    int         vf_status;
    int         vf_values[4];
    int         position;
#define buffer_end(__state) \
            (__state->buffer + (_countof(__state->buffer) - 1 /*nul*/))
    WChar_t     buffer[200];
    MAGIC_t     magic2;
};

static void             eprint(const char *prompt, const char *defstr);

static int              ereplyask(const char *prompt, const char *defstr, char *buf, int nbuf, int attr, int standout, int one);
static int              elineedit(const char *prompt, const char *defstr, char *buf, int nbuf, int one);
static void             eprompt(int force, int buflen);
static void             edisplay(void);

static int              eposition(const WChar_t *buf, int bpos, int bleft);

static int              emaxcols(void);
static int              eputc(int col, int c, vbyte_t attr);
static int              eputs(int col, const vbyte_t *buf, int len, vbyte_t attr);

static int              eprintlen(const WChar_t *str);
static int              eprintable(const vbyte_t ch, vbyte_t *buf, int *buflen);

static void             ef_format(const char *fmt, struct _estate *s);
static WChar_t *        ef_space(WChar_t *cp, const struct _estate *s);
static WChar_t *        ef_buffer(WChar_t *cp, const char *buf, const struct _estate *s);
static WChar_t *        ef_utf8(WChar_t *cp, const char *buf, const struct _estate *s);
static WChar_t *        ef_integer(WChar_t *cp, int ivalue, int width, const struct _estate *s);
static WChar_t *        ef_ovmode(WChar_t *cp, const struct _estate *s);
static WChar_t *        ef_charvalue(WChar_t *cp, struct _estate *s);
static WChar_t *        ef_virtual(WChar_t *cp, struct _estate *s);
static WChar_t *        ef_imode(WChar_t *cp, const struct _estate *s);
static WChar_t *        ef_line(WChar_t *cp, const struct _estate *s);
static WChar_t *        ef_numlines(WChar_t *cp, const struct _estate *s);
static WChar_t *        ef_filemode(WChar_t *cp, const struct _estate *s);
static WChar_t *        ef_col(WChar_t *cp, const struct _estate *s);
static WChar_t *        ef_percent(WChar_t *cp, const struct _estate *s);
static WChar_t *        ef_time(WChar_t *cp, int hour24, const struct _estate *s);
static WChar_t *        ef_date(WChar_t *cp, int format, const struct _estate *s);
static WChar_t *        ef_version(WChar_t *cp, const struct _estate *s);

int                     xf_echoflags =          /* echo line status */
                            E_CHARVALUE | E_VIRTUAL | E_LINE | E_COL | E_CURSOR | E_REMEMBER | E_TIME;

const char *            xf_echofmt = 0;         /* echo format */

int                     x_prompting = FALSE;    /* TRUE if user is being prompted */

static int              lc_disabled;            /* TRUE is line/col is disabled */

static int              lc_length;              /* length of line/col display */

static int              lc_column;              /* column where line/col message starts. */

static struct _estate   lc_state;               /* current echo_line state */

static vbyte_t          echo_color = ATTR_MESSAGE;
static vbyte_t          echo_standout = 0;

#define ECHOLINESZ          (MAX_CMDLINE + EBUFSIZ)

static WChar_t          wecho_line[ECHOLINESZ]; /* echo line buffer */
static unsigned char    echo_attr[ECHOLINESZ];  /* character attributes 0=normal,1=standout */

static int              echo_prompt_len;        /* length of prompt in characters */
static int              echo_prompt_width;      /* display width of prompt, in characters */

static int              echo_default;           /* length of default abs() +active,-inactive */

static int              echo_offset;            /* displayed offset within echo line when prompt > vtcols */

static const char *     echo_cmdline;           /* pointer to string just typed in by user (see inq_cmd_line) */

static char             last_msg[MSG_SIZE];     /* last message printed. */

int                     x_pause_on_error = 0;
int                     x_pause_on_message = 0;


/*  Function:           eyorn
 *      Ask "yes" or "no" question.
 *
 *      No formatting services are available nor newline required.
 *
 *  Parameters:
 *      msg - Message buffer.
 *
 *  Returns:
 *      ABORT if the user answers the question with the abort ("^G") character.
 *      Otherwise returns FALSE for "no" and TRUE for "yes".
 */
int
eyorn(const char *msg)
{
    const vbyte_t t_echo_color = echo_color;
    KEY s;

    echo_color = ATTR_QUESTION;
    while (1) {
        ewprintf("%s [yn]? ", msg);
        s = (KEY) io_get_key(0);
        if (IS_CHARACTER(s)) {
            if (strchr("yYnN\033", s)) {        /* Y[es], N[o] or ESC */
                break;
            }
        }
    }
    echo_color = t_echo_color;

    eclear();
    switch (s) {
    case 'y': case 'Y':
        return TRUE;
    case 'n': case 'N':
        return FALSE;
    default:
        return ABORT;
    }
}


/*  Function:           eyeson
 *      Ask "yes" or "no" question. No formatting services are available
 *      nor newline required.
 *
 *      Like eyorn, but for more important question. User must type either
 *      all of yes" or "no", and the trailing newline.
 *
 *  Parameters:
 *      sp - Question buffer.
 *
 *  Returns:
 *      ABORT if the user answers the question with the abort ("^G") character.
 *      Otherwise returns FALSE for "no" and TRUE for "yes".
 */
int
eyesno(const char *sp)
{
    char buf[32+1], prompt[MAX_CMDLINE];
    int s;

    sxprintf(prompt, sizeof(prompt), "\001%s? (^y^e^s or ^n^o)", sp);
    s = ereply(prompt, buf, sizeof(buf));
    for (;;) {

        if (ABORT == s) {
            return ABORT;
        }

        if (FALSE != s) {
            if ((buf[0] == 'y' || buf[0] == 'Y') &&
                (buf[1] == 'e' || buf[1] == 'E') &&
                (buf[2] == 's' || buf[2] == 'S')) {
                return TRUE;                    /* yes, ignore case */
            }

            if ((buf[0] == 'n' || buf[0] == 'N') &&
                (buf[1] == 'o' || buf[1] == 'O')) {
                return FALSE;                   /* no, ignore case */
            }
        }

        sxprintf(prompt, sizeof(prompt), "\001Please answer (^y^e^s or ^n^o) .. %s?", sp);
        s = ereply(prompt, buf, sizeof(buf));
    }
}


/*  Function:           ecursor
 *      Update the echo region to reflect the stated insert mode 'imode'.
 *
 *  Parameters:
 *      imode - Insert mode.
 *
 *  Returns:
 *      nothing
 *
 *  Externals:
 *      line_current_status()
 *          Boolean value stating whether the cursor is on a virtual
 *          character, being within a TAB or pass the EOL/EOF.
 */
void
ecursor(int imode)
{
    static int old_mode = -2;
    static int old_virtual_space = -2;
    int virtual_space;

    virtual_space = line_current_status(NULL, -1);
    if (old_virtual_space == virtual_space && imode == old_mode) {
        return;
    }
    ttcursor(1, imode, virtual_space);
    old_mode = imode;
    old_virtual_space = virtual_space;
}


/*  Function:           ereply
 *      Prompt the user for a reply, no default.
 *
 *  Parameters:
 *      prompt - Prompt.
 *      buf - Reply buffer.
 *      nbuf - Buffer length, in bytes.
 *
 *  Returns:
 *      ABORT if the user answers the question with the abort ("^G") character;
 *      otherwise TRUE with result contained within 'buf'.
 */
int
ereply(const char *prompt, char *buf, int nbuf)
{
    return ereplyask(prompt, NULL, buf, nbuf, ATTR_PROMPT, ATTR_PROMPT_STANDOUT, FALSE);
}


/*  Function:           ereply1
 *      Prompt the user for a single character reply, no default.
 *
 *  Parameters:
 *      prompt - Prompt.
 *      buf - Reply buffer.
 *      nbuf - Buffer length, in bytes.
 *
 *  Returns:
 *      ABORT if the user answers the question with the abort ("^G") character;
 *      otherwise TRUE with result contained within 'buf'.
 */
int
ereply1(const char *prompt, char *buf, int nbuf)
{
    return ereplyask(prompt, NULL, buf, nbuf, ATTR_PROMPT, ATTR_PROMPT_STANDOUT, TRUE);
}


/*  Function:           egetparm
 *      Prompt the user for a parameter; underlying get_parm() user interface.
 *
 *  Parameters:
 *      prompt - Prompt.
 *      defstr - Default reply, otherwise NULL.
 *      buf - Reply buffer.
 *      nbuf - Buffer length, in bytes.
 *      one - *true* if single character mode.
 *
 *  Returns:
 *      ABORT if the user answers the question with the abort ("^G") character;
 *      otherwise TRUE with result contained within 'buf'.
 */
int
egetparm(const char *prompt, const char *defstr, char *buf, int nbuf, int one)
{
    return ereplyask(prompt, defstr, buf, nbuf, ATTR_PROMPT, ATTR_PROMPT_STANDOUT, one);
}


/*  Function:           equestion
 *      Prompt the user within a question, no default.
 *
 *  Parameters:
 *      prompt - Prompt
 *      buf - Reply buffer.
 *      nbuf - Buffer length, in bytes.
 *
 *  Returns:
 *      ABORT if the user answers the question with the abort ("^G") character;
 *      otherwise TRUE with result contained within 'buf'.
 */
int
equestion(const char *prompt, char *buf, int nbuf)
{
    return ereplyask(prompt, NULL, buf, nbuf, ATTR_QUESTION, ATTR_STANDOUT, FALSE);
}


/*  Function:           ereplyask
 *      Reply prompt implementation. main engine for eyesno, ereply and friends.
 *
 *  Parameters:
 *      prompt - Prompt.
 *      defstr - Optional default reply.
 *      buf - Reply buffer.
 *      nbuf - Buffer length, in bytes.
 *      attr - Attribute.
 *      standout - Stand-out attribute.
 *      one - *true* if single character mode.
 *
 *  Returns:
 *      ABORT if the user answers the question with the
 *      abort ("^G") character; otherwise TRUE with result
 *      contained within 'buf'.
 *
 *<<GRIEF>> [callback]
    Macro: _prompt_begin - Command prompt session begin callback.

        void
        _prompt_begin(string prompt)

    Macro Description:
        The '_prompt_begin()' callback is executed at the beginning of a
        command prompt session.

        The interface allows the command line history and abbreviations
        mechanisms to be implemented.

    Macro Parameters:
        prompt - String containing the prompt.

    Macro Returns:
        nothing

    Macro Portability:
        n/a

    Macro See Also:
        get_parm, _prompt_begin, _prompt_end, _bad_key, inq_command, inq_cmd_line

 *<<GRIEF>> [callback]
    Macro: _prompt_end - Command prompt session end callback.

        void
        _prompt_end()

    Macro Description:
        The '_prompt_end()' callback is executed at the termination of a
        command prompt session.

    Macro Parameters:
        none

    Macro Returns:
        nothing

    Macro Portability:
        n/a

    Macro See Also:
        get_parm, _prompt_begin, _prompt_end, _bad_key, inq_command, inq_cmd_line

 *<<GRIEF>> [callback]
    Macro: _bad_key - Command prompt unknown key callback.

        string
        _bad_key()

    Macro Description:
        The '_bad_key()' callback is executed by GRIEF upon an
        unknown key being pressed during a command prompt session.

    Macro Parameters:
        none

    Macro Returns:
        The '_bad_key()' callback should return the replacement
        string otherwise NULL.

    Macro Portability:
        n/a

    Macro See Also:
        get_parm, _prompt_begin, _prompt_end, _bad_key, inq_command, inq_cmd_line

 */
static int
ereplyask(const char *prompt, const char *defstr,
    char *buf, int bufsiz, int attr, int standout, int one)
{
    const int saved_flags = trace_flags();
    const int omsglevel = x_msglevel;
    const vbyte_t t_echo_color = echo_color,
            t_echo_standout = echo_standout;
    int ret, nflags = saved_flags;

  //assert(EBUFSIZ >= MAX_PATH);
    assert(EBUFSIZ >= MAX_CMDLINE);

    /* display prompt */
    x_msglevel = 0;
    echo_color = attr;
    echo_standout = standout;
    eprint(prompt, (defstr ? defstr : ""));
    if ('\001' == *prompt) {
        ++prompt;                               /* consume hilite marker \001 */
    }

    trace_log("ereplyask: <%s>(%d:%d)\n", prompt, bufsiz, one);

    if (nflags && 0 == (nflags & DB_PROMPT)) {
        nflags = 0;                             /* disable debug prompt */
    }

    /* trigger start */
    if (!one) {
        static const char _prompt_begin[] = "_prompt_begin";

        if (macro_lookup(_prompt_begin)) {
            char ebuf[EBUFSIZ + 20];

            sxprintf(ebuf, sizeof(ebuf), "%s \"%s\"", _prompt_begin, prompt);
            trace_flagsset(nflags);
            execute_str(ebuf);
            /*
             *  TODO, BRIEF compat ---
             *      The new default response should be returned as a
             *      string if changed, otherwise return an integer/null
             *      to indicate no change.
             */
            trace_flagsset(saved_flags);
        } else {
            trace_log("ereplyask: _prompt_begin not found\n");
        }
    }

    /* line-editor implementation */
    ret = elineedit(prompt, defstr, buf, bufsiz, one);

    /* completion */
    echo_standout = t_echo_standout;
    echo_color = t_echo_color;
    echo_offset = 0;

    if (!one) {
        static const char _prompt_end[] = "_prompt_end";

        /*
         *  Save a global pointer to the typed in text so that inq_cmd_line() can get
         *  the whole command string
         *
         *  On completions, echo_cmdline is clear so that inq_cmd_line() only works
         *  partially, i.e. it can only get the bit on display, and not the full string
         */
        assert(NULL == echo_cmdline);
        echo_cmdline = buf;
        if (macro_lookup(_prompt_end)) {
            trace_flagsset(nflags);
            execute_str(_prompt_end);
            trace_flagsset(saved_flags);
        } else {
            trace_log("ereplyask: _prompt_end not found\n");
        }
        echo_cmdline = NULL;
    }

    /* restore message level */
    x_msglevel = omsglevel;
    set_hooked();
    return ret;
}


/*  Function:           elineedit
 *      Command line-editor, main engine for eyesno, ereply and friends.
 *
 *  Parameters:
 *      prompt - Prompt.
 *      defstr - Default reply.
 *      buf - Reply buffer.
 *      bufsiz - Buffer length, in bytes.
 *      one - *true* if single character mode.
 *
 *  Navigation/actions keys:
 *
 *      Key                         Function
 *
 *      Right, Left                 Move cursor the back/forward one character.
 *
 *      Ctrl+Right, Ctrl+Left       Move cursor the start/end of the current word.
 *
 *      Home, End                   Move to first/last character within the edit field.
 *
 *      Alt+I, Ctrl-O               Toggle insert/overstrike mode.
 *
 *      DEL                         Delete character under the cursor.
 *
 *      Backspace, Ctrl+H           Delete character prior to the cursor.
 *
 *      Alt+K                       Delete from cursor to the end of line.
 *
 *      Insert                      Paste from scape.
 *
 *      Backspace, Ctrl+H           Delete character prior to the cursor.
 *
 *      ESC                         Abort current edit, restoring original content.
 *
 *      Alt-D                       Delete current line.
 *
 *      Alt-K                       Delete from cursor to the end of line.
 *
 *      Alt+Q, Ctrl+Q               Quote next character.
 *
 *      Enter                       Process change.
 *
 *      ALT+H                       Help.
 *
 *      Ctrl+A (*)                  Move cursor to beginning of line.
 *
 *      Ctrl+D (*)                  Delete character under cursor.
 *
 *      Ctrl+K (*)                  Delete from cursor to the end of line.
 *
 *      Ctrl+X, Ctrl+U (*)          Delete current line.
 *
 *      Ctrl+V, Ctrl+Y (*)          Paste from clipboard.
 *
 *  TODO (plus key mapping):
 *
 *      Ctrl+C, Ctrl+W (*)          Cut to clipboard.
 *
 *      Ctrl+Del, Alt+D (*)         Delete current cursor to end of word.
 *
 *      Ctrl+Backspace (*)          Delete from cursor to start of word.
 *
 *      Ctrl+Space (*)              Set mark.
 *
 *      Alt+Space (*)               Delete all white-space around cursor, reinsert single space.
 *
 *      Alt+\ (*)                   Delete all white-space around cursor.
 *
 *      (*) Emacs style key mappings.
 *
 *  Returns:
 *      ABORT if the user answers the question with the abort ("^G") character;
 *      otherwise TRUE with result contained within 'buf'.
 */
static int
elineedit(const char *prompt, const char *defstr, char *result, int bufsiz, int one)
{
    WChar_t buf[EBUFSIZ];
    char ndefstr[EBUFSIZ];
    int bleft = 0, bpos = 0, imode = TRUE;
    int first_key = TRUE;
    KEY c = 0;

    assert(bufsiz >= 2 /*&& bufsiz <= EBUFSIZ*/);
    if (bufsiz > _countof(buf)) bufsiz = _countof(buf);

    memset(buf, 0, sizeof(buf));
    memset(ndefstr, 0, sizeof(ndefstr));

    if (NULL == defstr) defstr = "";
    Wcsfromutf8(defstr, buf, bufsiz);

    last_msg[0] = '\0';
    x_prompting = TRUE;
    ecursor(imode);
    ttcolornormal();

    trace_log("elineedit:\n");

    while (1) {
        /* update prompt/cursor */
        assert(bpos >= 0 && bpos <= bufsiz);
        if (! first_key) {
            if (0 == bpos) {
                echo_offset = bleft = 0;        /* home */
            }
            eprompt(FALSE, (int) Wcslen(buf));
            bleft = eposition(buf, bpos, bleft);
        }

        /* next key */
        ttflush();
        c = (KEY) io_get_key(0);

        if (one) {
            /*
             *  one character mode/
             *      exit after a single non-ESC character is typed,
             *      otherwise ESC implies ABORT.
             */
            if (KEY_ESC != c) {
                buf[0] = (c > 0x7f ? 0 : (char)c);
                buf[1] = '\0';
                goto done;
            }
            goto cancel;

        } else if (first_key) {
            switch(c) {
                /*
                 *  First key and it is a special one,
                 *      then user has accepted the default prompt unmark.
                 */
            case KEY_TAB:    case CTRL_H:
            case KEY_HOME:   case KEY_END:
            case KEY_WLEFT:  case KEY_WRIGHT:
            case KEY_LEFT:   case KEY_RIGHT:
            case KEY_DELETE: case KEY_DEL:
                bpos = Wcsfromutf8(defstr, buf, bufsiz);
                break;

                /*
                 *  Unless <enter>, erase the default value from the screen
                 */
            case KEY_ENTER:
            case KEY_NEWLINE:
                break;
            default:
                buf[0] = '\0';
                bpos = 0;
                break;
            }
            first_key = FALSE;
        }

        defstr = "";                            /* clear 'default' value */
        echo_default = 0;

        /* cook the key */
        switch (c) {
        case KEY_ENTER:     /* <Enter> */
        case KEY_NEWLINE:
            goto done;

        case ALT_H:         /* <Help> */
        case KEY_HELP:
            x_prompting = FALSE;
            trigger(REG_ALT_H);
            x_prompting = TRUE;
            break;

        case KEY_ESC:       /* <ESC> */
cancel:;    ecursor(buf_imode(curbp));
            ewprintf("Command cancelled.");
            x_prompting = FALSE;
            return ABORT;

        case KEY_TAB:       /* Specials */
        case ALT_A:  case ALT_B:  case ALT_C:     /*ALT_D*/ case ALT_E:
        case ALT_F:  case ALT_G:     /*ALT_H*/    /*ALT_I*/ case ALT_J:
           /*ALT_K*/ case ALT_L:  case ALT_M:  case ALT_N:  case ALT_O:
        case ALT_P:     /*ALT_Q*/ case ALT_R:  case ALT_S:  case ALT_T:
        case ALT_U:  case ALT_V:  case ALT_W:  case ALT_X:  case ALT_Y:
        case ALT_Z:
        case KEY_DOWN:
        case KEY_UP:
        case WHEEL_UP:
        case WHEEL_DOWN:
badkey:;    {   const char *oecho_cmdline = echo_cmdline;
                const int t_bufsiz = Wcslen(buf) * 4;
                const char *sacc;
                char *t_buf = NULL;

                trace_log("elineedit: badkey\n");
                if (t_bufsiz) {
                    if (NULL == (t_buf = malloc(t_bufsiz))) {
                        goto cancel;
                    }
                    Wcstoutf8(buf, t_buf, t_bufsiz);
                }

                key_cache_key(x_push_ref, c, FALSE);
                trigger(REG_INVALID);           /* old interface, use of _bad_key() preferred */

                echo_cmdline = (t_buf ? t_buf : "");
                x_prompting  = FALSE;
                execute_str("_bad_key");        /* pass the keycode?? */
                echo_cmdline = oecho_cmdline;
                x_prompting  = TRUE;
                free(t_buf);

                if (NULL == (sacc = acc_get_sval())) {
                    goto cancel;
                }

                strxcpy(ndefstr, sacc, sizeof(ndefstr));
                Wcsfromutf8(sacc, buf, bufsiz);

                eprint(prompt, ndefstr);
                defstr = ndefstr;
                first_key = TRUE;
            }
            break;

        case KEY_WLEFT:     /* <Left>, cursor movement */
        case KEY_WLEFT2:
            while (bpos > 0 && isspace(buf[bpos])) {
                --bpos;
            }
            while (bpos > 0 && !isspace(buf[bpos])) {
                --bpos;
            }
            break;

        case KEY_WRIGHT:    /* <Right>, cursor movement */
        case KEY_WRIGHT2:
            while (bpos < bufsiz && isspace(buf[bpos]) && buf[bpos]) {
                ++bpos;
            }
            while (bpos < bufsiz && !isspace(buf[bpos]) && buf[bpos]) {
                ++bpos;
            }
            break;

        case KEY_RIGHT:     /* <Right>, cursor movement */
            if (bpos < bufsiz && buf[bpos]) {
                ++bpos;
            }
            break;

        case KEY_LEFT:      /* <Left>, cursor movement */
            if (bpos > 0) {
                --bpos;
            }
            break;

        case KEY_HOME:      /* <home>,  cursor movement */
        case CTRL_A:
            bpos = 0;
            break;

        case KEY_END:       /* <End>,   cursor movement */
            bpos = (int)Wcslen(buf);
            break;

        case ALT_I:         /* <Alt-i>, toggle insert/overstrike mode */
        case CTRL_O:
            imode = !imode;
            ecursor(imode);
            ttflush();
            break;

        case CTRL_H:        /* <Ctrl-h>, delete previous character */
            if (0 == bpos) {
                break;
            }
            --bpos;
            /*FALLTHRU*/

        case KEY_DEL:       /* <Del>,   delete character under the cursor */
        case 0x7F:
        case CTRL_D: {
                WChar_t *bcursor = buf + bpos;
                size_t rlen = Wcslen(bcursor);

                if (rlen) {
                    if (--rlen) Wmemmove(bcursor, bcursor + 1, rlen);
                    bcursor[rlen] = 0;
                }
            }
            break;

        case ALT_K:         /* <Alt-k>, delete characters to end of input */
        case CTRL_K:
            buf[bpos] = '\0';
            break;

        case ALT_D:         /* <Alt-d>, delete line/buffer */
        case CTRL_X:
        case CTRL_U:
            buf[0] = '\0';
            bpos = 0;
            break;

        case KEY_INS:       /* <Ins>,   insert scrap into prompt */
        case KEY_PASTE:
        case CTRL_V:
        case CTRL_Y: {
                WChar_t t_buf[sizeof(buf)] = {0};
                const char *scrap_buffer = NULL;
                int scrap_size = 0;

                if (bpos >= (bufsiz - 1)) {
                    ttbeep();                   /* end-of-buffer */
                    break;
                }

                k_seek();                       /* MCHAR/???, utf8 assumption */
                while (0 == (scrap_size = k_read(&scrap_buffer))) {
                    continue;                   /* first non-empty line */
                }

                if (scrap_size < 0 /*eof*/) {
                    ttbeep();                   /* nothing available */

                } else {
                    const int blen = Wcslen(buf);
                    const int trailing = (bpos < blen ? blen - bpos : 0);
                    const int remaining = bufsiz - (bpos + (imode ? trailing : 0));

                    assert(bpos <= blen);
                    if (remaining > imode) {    /* space available, inc nul */
                        int copied;

                        memcpy(t_buf, buf, sizeof(buf));
                        if ((copied = Wcsfromutf8(scrap_buffer, buf + bpos, remaining)) > 0) {
                            assert((bpos + copied) < bufsiz);
                            if (trailing) {
                                const int binsert = bpos + copied;
                                if (imode) {    /* append original */
                                    assert((binsert + trailing) < bufsiz);
                                    Wmemcpy(buf + binsert, t_buf + bpos, trailing + 1 /*nul*/);
                                } else {        /* replace null with previous value */
                                    if (binsert < blen) {
                                        buf[binsert] = t_buf[binsert];
                                    }
                                }
                            }
                            assert(Wcslen(buf) < (size_t)bufsiz);
                        }
                        scrap_size -= copied;
                    }

                    if (scrap_size) {
                        ttbeep();               /* overflow */
                    }
                }
            }
            break;

        case 0:
        case KEY_VOID:
            break;

        case KEY_WINCH:                         /* resize event */
            vtwinch(ttcols(), ttrows());
            vtupdate();
            elinecol(LC_DONTENABLE);
            break;

        case ALT_Q:         /* <Alt-q>, quote the next input character */
        case CTRL_Q:
            c = (KEY) io_get_raw(0);
            if (0 == c || c >= KEY_VOID) {
                break;
            }
            /*FALLTHRU*/

        default:
            if (0 == IS_CHARACTER(c)) {
                goto badkey;                    /* invalid key */

            } else if (bpos >= (bufsiz - 1)) {
                ttbeep();                       /* end-of-buffer */

            } else {
                const int blen = Wcslen(buf);
                const int trailing = (bpos < blen ? blen - bpos : 0);
                const int remaining = bufsiz - (bpos + (imode ? trailing : 0));

                assert(bpos <= blen);           /* space available, minus nul */
                if (remaining > imode) {
                    WChar_t *binsert = buf + bpos++;
                    if (*binsert) {
                        assert(trailing);
                        if (imode) {            /* insert, otherwise replace */
                            assert((binsert + trailing) < (buf + bufsiz));
                            Wmemmove(binsert + 1, binsert, trailing + 1 /*nul*/);
                        }
                    } else {
                        binsert[1] = '\0';      /* end-of-string */
                    }
                    *binsert = c;
                    assert(Wcslen(buf) < (size_t)bufsiz);

                } else {
                    ttbeep();                   /* overflow */
                }
            }
            break;
        }
    }

done:;
    assert(Wcslen(buf) < (size_t)bufsiz);
    Wcstoutf8(buf, result, bufsiz);
    assert((int)strlen(result) < bufsiz);
    x_prompting = FALSE;
    ecursor(buf_imode(curbp));
    return TRUE;
}


/*  Function:           eprompt
 *      Calculate the prompt offset and redraw if required.
 *
 *  Parameters:
 *      force - Force redraw flag.
 *      buflen - User buffer length (>=0), otherwise assume default.
 *
 *  Return:
 *      nothing
 */
static void
eprompt(int force, int buflen)
{
    const int odisabled = lc_disabled;
    const int ooffset   = echo_offset;
    const int length    = echo_prompt_width +   /* total length */
                (buflen >= 0 ? buflen : echo_default);

    /* disable line/col? */
    if (! lc_disabled) {
        lc_disabled = (length >= lc_column ||
                length >= ttcols() - lc_length ? TRUE : FALSE);
    }

    /* set prompt left offset */
    if (length < (ttcols() - 1)) {
        echo_offset = 0;                        /* home */

    } else  {                                   /* >3/4 shall be prompt */
        echo_offset = echo_prompt_width - (ttcols() * 3) / 4;
        if (echo_offset < 0) {
            echo_offset = 0;
        }
    }

    /* reflect changes */
    if (force || odisabled != lc_disabled) {
        elinecol(LC_FORCE|LC_DONTENABLE);       /* update line/col, if enabled */
    }

    if (force || ooffset != echo_offset ||
            (odisabled != lc_disabled && lc_disabled)) {
        edisplay();                             /* update echo_line, shall also clear line-col if disabled */
    }
}


/*  Function:           eposition
 *      Render the user reply buffer, if requirer reframing the input so the
 *      cursor is visible.
 *
 *  Parameters:
 *      buf - Address of user buffer.
 *      bpos - Current buffer position (cursor).
 *      bleft - Storage of working variable contained the 'left' margin.
 *
 *  Return:
 *      Resultng 'left' value.
 */
static int
eposition(const WChar_t *buf, int bpos, int bleft)
{
    const vbyte_t attr = VBYTE_ATTR(echo_color);
    int col = echo_prompt_width - echo_offset;  /* user buffer base column */
    int cursor = -1;                            /* cursor position */
    int i, pos = 0;

    /*
     *  Size output buffer ...
     */
    for (i = 0; i < bpos && buf[i];) {
        pos += eprintable(buf[i++], NULL, NULL);
    }

    /*
     *  if cursor off screen, reframe
     */
    if (bpos < bleft) {
        while (i > 0 && bleft > bpos) {
            pos -= eprintable(buf[--i], NULL, NULL);
            --bleft;
        }

    } else if (col + pos - bleft >= lc_column) {
        pos = lc_column;
        while (pos >= col && i >= 0) {
            const int width = eprintable(buf[i--], NULL, NULL);
            if ((pos - width) <= col) {
                ++i;
                break;
            }
            pos -= width;
        }
        bleft = i;
    }

    /*
     *  Redraw prompt, pad with trailing spaces.
     */
    buf += bleft;
    for (i = bleft; col < lc_column; ++i) {
        const int ch = (*buf ? *buf++ : ' ');
        vbyte_t vbuf[16] = {0};
        int vlen = _countof(vbuf);

        if (i == bpos) cursor = col;            /* cursor located */

        eprintable(ch, vbuf, &vlen);
        col = eputs(col, vbuf, vlen, attr);
    }

    vtupdate_bottom(cursor);

    return bleft;
}


/*  Function:           edisplay
 *      display the echo line.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 */
static void
edisplay(void)
{
    const vbyte_t normal = VBYTE_ATTR(echo_color);
    const vbyte_t standout = VBYTE_ATTR(echo_standout);
    const vbyte_t completion = VBYTE_ATTR(ATTR_PROMPT_COMPLETE);
    const WChar_t *cp, *ep;                     /* buffer working pointers */
    const unsigned char *ap;
    int offset = echo_offset, cursor, col = 0;

    trace_log("edisplay: prompt[%d,%d]:<", offset, echo_prompt_width);

    for (cp = wecho_line, ep = cp + echo_prompt_len,
            ap = echo_attr; *cp; ++cp, ++ap) {
        const int32_t wch = *cp;

        if (offset > 0) {                       /* consume off-screen */
            --offset;
            continue;
        }

        if (lc_column && col >= lc_column) {
            break;                              /* trim */
        }

        trace_log("%c", wch);

        if (standout && col < echo_prompt_width && *ap) {
            col = eputc(col, wch, standout);    /* prompt */

        } else if (echo_default > 0 && cp >= ep) {
            col = eputc(col, wch, completion);  /* default/completion */

        } else {
            col = eputc(col, wch, normal);      /* user text */
        }
    }

    cursor = col;
    trace_log("> (col:%d)\n", cursor);

    while (col < lc_column) {                   /* space fill */
        const int t_col = eputc(col, ' ', VBYTE_ATTR(ATTR_NORMAL));
        if (t_col == col) break;
        col = t_col;
    }

    vtupdate_bottom(cursor);                    /* flush results */
}


/*  Function:           inq_message
 *      inq_message primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: inq_message - Retrieve the prompt line.

        string
        inq_message()

    Macro Description:
        The 'inq_message()' primitive returns the string that is
        currently displayed on the command prompt.

    Macro Parameters:
        none

    Macro Returns:
        The 'inq_message()' primitive returns a string being what is
        currently displayed on the message line.

    Macro Portability:
        n/a

    Macro See Also:
        inq_cmd_line, error, message
 */
void
inq_message(void)
{
    char t_echo_line[ECHOLINESZ] = {0};

    Wcstoutf8(wecho_line, t_echo_line, sizeof(t_echo_line));
    acc_assign_str((const char *)t_echo_line, -1);
}


/*  Function:           inq_prompt
 *      inq_prompt primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: inq_prompt - Retrieve the prompt status.

        int
        inq_prompt()

    Macro Description:
        The 'inq_prompt()' primitive retrieves the prompt status flag,
        which can be used to detrmine whether the user prompt is
        current active.

    Macro Parameters:
        none

    Macro Returns:
        The 'inq_prompt()' primitive returns a boolean value stating
        whether or not the user is currently being prompted for input
        on the command line.

    Macro Portability:
        n/a

    Macro See Also:
        get_parm
 */
void
inq_prompt(void)
{
    acc_assign_int(x_prompting);
}


/*  Function:           inq_cmd_line
 *      inq_cmd_line primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: inq_cmd_line - Retrieve the command line message.

        string
        inq_cmd_line()

    Macro Description:
        The 'inq_cmd_line()' primitive returns the current prompt
        response displayed on the message line.

    Macro Parameters:
        none

    Macro Returns:
        The 'inq_cmd_line' returns a string containing the current prompt.

    Macro Portability:
        n/a

    Macro See Also:
        inq_message
 */
void
inq_cmd_line(void)
{
    if (echo_cmdline) {
        acc_assign_str((const char *)echo_cmdline, -1);

    } else {
        char t_cmd_line[EBUFSIZ] = {0};

        Wcstoutf8(wecho_line + echo_prompt_len, t_cmd_line, sizeof(t_cmd_line));
        acc_assign_str((const char *)t_cmd_line, -1);
    }
}


/*  Function:           inq_line_col
 *      inq_col_line primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: inq_line_col - Retrieve the echo_line content.

        string
        inq_line_col()

    Macro Description:
        The 'inq_line_col()' primitive retrieves the current content of
        the echo_line.

    Macro Parameters:
        none

    Macro Returns:
        The 'inq_line_col()' primitive returns the current echo line status.

    Macro Portability:
        n/a

    Macro See Also:
        inq_cmd_line
 */
void
inq_line_col(void)
{
    char t_line_col[_countof(lc_state.buffer) * 4] = {0};

    Wcstoutf8(lc_state.buffer, t_line_col, sizeof(t_line_col));
    acc_assign_str(t_line_col, -1);
}


/*  Function:           ewprintf
 *      Display a printf style string on the status line.
 *
 *  Parameters:
 *      str - Format specification.
 *      ... - Parameters.
 *
 *  Returns:
 *      nothing
 */
void
ewprintf(const char *fmt, ...)
{
    char iobuf[EBUFSIZ];
    va_list ap;

    va_start(ap, fmt);
    vsxprintf(iobuf, sizeof(iobuf), fmt, ap);
    ewputs(iobuf);
    va_end(ap);
}


void
ewprintx(const char *fmt, ...)
{
    const int xerrno = errno;
    char iobuf[EBUFSIZ];
    va_list ap;
    int iolen;

    va_start(ap, fmt);
    iolen = vsxprintf(iobuf, sizeof(iobuf) - 32, fmt, ap);
    if (xerrno) {
        sxprintf(iobuf + iolen , sizeof(iobuf) - iolen,
            ": %s (%d)", str_error(xerrno), xerrno);
    }
    va_end(ap);
    ewputs(iobuf);
}


/*  Function:           eeprintf
 *      Display a printf style error on the status line.
 *
 *  Parameters:
 *      fmt - Format specification.
 *      ... - Parameters.
 *
 *  Returns:
 *      nothing
 */
void
eeprintf(const char *fmt, ...)
{
    char iobuf[EBUFSIZ];
    va_list ap;

    va_start(ap, fmt);
    vsxprintf(iobuf, sizeof(iobuf), fmt, ap);
    eeputs(iobuf);
    va_end(ap);
}


void
eeprintx(const char *fmt, ...)
{
    const int xerrno = errno;
    char iobuf[EBUFSIZ];
    va_list ap;
    int iolen;

    va_start(ap, fmt);
    iolen = vsxprintf(iobuf, sizeof(iobuf) - 32, fmt, ap);
    if (xerrno) {
        sxprintf(iobuf + iolen , sizeof(iobuf) - iolen, ": %s (%d)", str_error(xerrno), xerrno);
    }
    va_end(ap);
    eeputs(iobuf);
}


/*  Function:           eclear
 *      Clear the display line.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 */
void
eclear(void)
{
    ewputs("");
}


/*  Function:           eeputs
 *      Display an error message on the status line.
 *
 *  Parameters:
 *      msg - Message buffer.
 *
 *  Returns:
 *      nothing
 */
void
eeputs(const char *msg)
{
    const vbyte_t t_echo_color = echo_color;

    echo_color = ATTR_ERROR;
    eprint(msg, NULL);
    echo_color = t_echo_color;
}


/*  Function:           ewputs
 *      Display a warning message on the status line.
 *
 *  Parameters:
 *      msg - Message buffer.
 *
 *  Returns:
 *      nothing
 */
void
ewputs(const char *msg)
{
    eprint(msg, NULL);
}


/*  Function:           eprintf
 *      Display a string on the status line.
 *
 *  Parameters:
 *      prompt - Question prompt/message.
 *      defstr - Default value, otherwise NULL.
 *
 *  Returns:
 *      nothing
 */
static void
eprint(const char *prompt, const char *defstr)
{
    int hilite = FALSE;

    if (NULL == prompt) prompt = "";

    if ('\001' == *prompt) {
        hilite = TRUE;
        ++prompt;                               /* consume hilite marker \001 */
    }

    if (! vtinited()) {                         /* display initialised? */
        fprintf(stderr, "\n*** %s\n", prompt);
        fflush(stderr);
        return;
    }

    if (defstr) {                               /* default prompt */
        last_msg[0] = '\0';

    } else if (*prompt &&
            0 == strncmp(last_msg, prompt, sizeof(last_msg)-1)) {
        return;                                 /* same message, ignore update */

    } else {
        strxcpy(last_msg, prompt, sizeof(last_msg));
    }

    /* initialise lc_column */
    if (0 == lc_column) {
        elinecol(0);
        if (lc_column <= 0) {
            lc_column = emaxcols();             /* disabled etc */
        }
    }

    /*
     *  import prompt, default and display
     */
    echo_prompt_len = Wcsfromutf8(prompt, wecho_line, _countof(wecho_line));

    memset(echo_attr, 0, sizeof(echo_attr));
    if (hilite) {
        WChar_t *cp = wecho_line, *out = cp,
            *end = cp + (_countof(wecho_line) - 1), wch;
        unsigned char *ap = echo_attr;

        while (cp < end && 0 != (wch = *cp++)) {
            if ('^' == wch && *cp) {            /* hilite next character */
                --echo_prompt_len;
                *ap = 1;
                continue;
            }
            *out++ = wch;
            ++ap;
        }
        *out = 0;
    }

    echo_prompt_width = eprintlen(wecho_line);  /* display width */

    if (!defstr || !*defstr) {
        echo_default = 0;                       /* no default */
    } else {
        echo_default = Wcsfromutf8(defstr, wecho_line + echo_prompt_len, _countof(wecho_line) - echo_prompt_len);
    }

    eprompt(TRUE, -1);
}


/*  Function:           emaxcols
 *      Determine the usable columns by the echo-line.
 *
 *  Parameter:
 *      none
 *
 *  Returns:
 *      Max column usable by the echo-line.
 */
static int
emaxcols(void)
{
    int cols = ttcols();

    if (! ttlastsafe()) {
        --cols;                                 /* ignore last column */
    }
    return (cols - 1);
}


/*  Function:           eputc
 *      Write to the virtual display, using the existing attribute.
 *      The specified character is converted to it printable form prior to being displayed.
 *
 *  Parameter:
 *      pp - Cursor.
 *      c - Character value.
 *      attr - Attribute.
 *
 *  Returns:
 *      Resulting cursor.
 */
static int
eputc(int col, int c, vbyte_t attr)
{
    vbyte_t vbuf[16] = {0};
    int vlen = _countof(vbuf);

    if ('\r' == c || '\n' == c) {
        return col;
    }
    eprintable(c, vbuf, &vlen);
    return eputs(col, vbuf, vlen, attr);
}


/*  Function:           eputs
 *      Write to the virtual display, using the specified attribute.
 *
 *  Parameter:
 *      col - Cursor.
 *      buf - Character values.
 *      len - Length of the character vector.
 *      attr - Attribute.
 *
 *  Returns:
 *      Resulting cursor position.
 */
static int
eputs(int col, const vbyte_t *buf, int len, vbyte_t attr)
{
    assert(col >= 0);
    assert(col < ttcols());
    assert(lc_column > 0);
    assert(lc_column <= ttcols());

    if (col >= 0) {
        while (len-- > 0 && col < lc_column) {
            const int ncol = vtpute((vbyte_t)(*buf++ | attr), col);
            if (-1 == ncol) {
                break;                          /* off-screen */
            }
            col = ncol;
        }
    }
    return col;
}


static const struct tm *
my_localtime(time_t *tm)
{
    static struct tm x_ltm;
    static time_t x_ctm;
    const time_t ctm = sys_time(NULL);

    if (ctm != x_ctm) {
        if (ctm == (x_ctm + 1)) {
            if (++x_ltm.tm_sec >= 60) {
                x_ltm.tm_sec = 0;
                if (++x_ltm.tm_min >= 60) {
                    x_ltm.tm_min = 0;
                    if (++x_ltm.tm_hour >= 24) {
#if defined(HAVE_LOCALTIME_R)
                        (void) localtime_r(&ctm, &x_ltm);
#else
                        x_ltm = *localtime(&ctm);
#endif
                    }
                }
            }
        } else {
#if defined(HAVE_LOCALTIME_R)
            (void) localtime_r(&ctm, &x_ltm);
#else
            x_ltm = *localtime(&ctm);
#endif
        }
        x_ctm = ctm;
    }
    *tm = ctm;
    return &x_ltm;
}


/*  Function:           elinecol
 *      Update the echo line.
 *
 *  Parameter:
 *      flags - Control bits.
 *
 *  Returns:
 *      nothing
 */
void
elinecol(int flags)
{
    struct _estate state = {0};
    const struct tm *tp;

    /* line/col disabled? */
    if (lc_disabled) {
        if (! x_prompting && 0 == (LC_DONTENABLE & flags)) {
            lc_disabled = FALSE;                /* fuse disable */
            flags |= LC_FORCE;
        } else {
            lc_column = emaxcols();
            lc_state.position = 0;
        }
    }

    if (lc_disabled || (E_FROZEN & xf_echoflags)) {
        if (LC_FORCE & flags) {
            lc_state.tm_hour = -1;              /* force on next update */
        }
        return;
    }

    tp = my_localtime(&state.tm_time);
    state.magic      = ESTATE_MAGIC;
    state.bf_line    = (curwp ? curwp->w_line : 0);
    state.bf_col     = (curwp ? curwp->w_col : 0);
    state.vf_status  = -999;
    state.tm_year    = tp->tm_year + 1900;
    state.tm_month   = tp->tm_mon + 1;
    state.tm_day     = tp->tm_mday;
    state.tm_hour    = tp->tm_hour;
    state.tm_min     = tp->tm_min;
    state.magic2     = ESTATE_MAGIC;

    if (0 == (LC_FORCE & flags)) {
        if (state.bf_line == lc_state.bf_line && state.bf_col == lc_state.bf_col &&
                state.tm_hour == lc_state.tm_hour && state.tm_min == lc_state.tm_min) {
            return;                             /* no update required */
        }
    }

    lc_state.bf_line = state.bf_line;
    lc_state.bf_col  = state.bf_col;
    lc_state.tm_hour = state.tm_hour;
    lc_state.tm_min  = state.tm_min;

    /* generate image */
    state.buffer[0] = '\0';

    if (xf_echofmt && (E_FORMAT & xf_echoflags)) {
        ef_format(xf_echofmt, &state);

    } else {
        const int echoflags = xf_echoflags;
        WChar_t *cp = state.buffer;

        if (curwp && W_TILED == curwp->w_type) {
            if (echoflags & E_CHARVALUE) {
                cp = ef_charvalue(cp, &state);  /* character value */
            }
            if (echoflags & E_VIRTUAL) {
                cp = ef_virtual(cp, &state);    /* virtual cursor */
            }
        }

        if (echoflags & E_LINE)  {              /* line */
            cp = ef_line(ef_space(cp, &state), &state);
        }

        if (echoflags & E_COL) {                /* column */
            cp = ef_col(ef_space(cp, &state), &state);
        }

        if (echoflags & E_PERCENT)  {           /* cursor percentage within file */
            cp = ef_percent(ef_space(cp, &state), &state);
        }

        if (echoflags & E_CURSOR) {
            cp = ef_ovmode(cp, &state);         /* only if a visual cursor isn't available */
        }

        if ((echoflags & E_REMEMBER) && x_rem_string[0] != ' ') {
            cp = ef_buffer(cp, x_rem_string, &state); /* RE/PA */
            cp = ef_space(cp, &state);
        }

        if (echoflags & (E_TIME|E_TIME24)) {    /* current time */
            cp = ef_time(ef_space(cp, &state), (E_TIME24 & echoflags) ? TRUE : FALSE, &state);
        }
    }

    assert(ESTATE_MAGIC == state.magic);
    assert(ESTATE_MAGIC == state.magic2);
    lc_length = (int)Wcslen(state.buffer);      /* line length */
    assert(lc_length < (int) sizeof(state.buffer));

    /* update */
    lc_column = emaxcols() - lc_length;         /* left column */

    if ((LC_FORCE & flags) || lc_state.position != lc_column ||
            0 != Wcscmp(lc_state.buffer, state.buffer)) {
        const vbyte_t attr = VBYTE_ATTR(ATTR_ECHOLINE);
        const WChar_t *cp;
        int col = 0;
                                                /* erase characters longer needed */
        if (0 == lc_state.position || lc_state.position >= lc_column) {
            col = lc_column;
        } else {
            col += lc_state.position;
            while (col >= 0 && col < lc_column) {
                const int ncol = vtpute(' ' | attr, col);
                if (-1 == ncol) {
                    break;                      /* off-screen */
                }
                col = ncol;
            }
        }

        for (cp = state.buffer; *cp; ++cp) {
            col = vtpute(*cp | attr, col);      /* now draw in the line/col string */
        }

        if (! ttlastsafe()) {                   /* clear last character */
            col = vtpute(' ' | VBYTE_ATTR(ATTR_NORMAL), col);
        }

        vtupdate_bottom(-1);                    /* updated */
        Wcscpy(lc_state.buffer, state.buffer);
        lc_state.position = lc_column;
    }
}


/*  Function:           do_echo_line
 *      echo_line primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: echo_line - Set echo line flags.

        int
        echo_line([int flags])

    Macro Description:
        The 'echo_line()' primitive controls the fields which are to
        be visible within the status area.

        The status area layout can be defined by one of two means.
        The 'echo_line' specification controls the elements visible
        using a fixed layout whereas 'set_echo_format' allows a user
        defined format specification.

        The default echo_line configuration is.

>           E_CHARVALUE | E_VIRTUAL | E_LINE | E_COL | \
>               E_CURSOR | E_REMEMBER | E_TIME

    Macro Parameters:
        flags - Optional integer flags, one or more of the
            following flags OR'ed together control the components to
            be reported against each attribute. If omitted then only
            the current flag values are returned without any change.

        Flags::

(start table,format=nd)
        [Constant       [Order  [Definition                             ]
      ! E_CHARVALUE     1       Character value.
      ! E_VIRTUAL       2       Virtual character indicator.
      ! E_LINE          3       Line: field.
      ! E_COL           4       Col: field.
      ! E_PERCENT       5       nn%
      ! E_CURSOR        7       Insert/overstrike status (OV or blank).
      ! E_REMEMBER      6       Remember status (RE and PA).
      ! E_TIME          8       HH:MM a/pm
      ! E_TIME24        8       Time in 24hour form (HH:MM).

      ! E_FORMAT        n/a     Format override active.
      ! E_FROZEN        n/a     Echo line is frozen, ie not updated.
(end table)

        When an status area user specified format is active then the
        *E_FORMAT* flag is enabled (see set_echo_format). Setting a
        format also sets *E_FORMAT* similarly clearing the format
        clears *E_FORMAT*.

        The *E_FROZEN* flag disables status area updates whilst set.
        E_FROZEN can be utilised by macros to reduce screen updates
        during buffer operations.

     Active Character::

        The *E_CHARVALUE* element identifies the character under the
        cursor. Normal printable characters are enclosed within a set
        of square brackets, with non-printable characters represented
        by their hexadecimal value. When the cursor is positioned
        past the end of the current line, 'EOL' is displayed, and
        when past the end of file, 'EOF' is displayed.

     Virtual Character::

        The *E_VIRTUAL* element indicates whether the current
        character is either a physical or virtual character in the
        case of tabs, EOL and EOF conditions.

        The virtual character status is represented by one of the
        following otherwise blank if a normal character.

           X    - Virtual space, for example logical space created as
                    the result of tab expansion.

           $    - End of line.

           +    - Position is past the end-of-line.

           #    - Is a Unicode encoded character.

           !    - Is an illegal Unicode character code.

     Buffer Coordinates::

        The *E_LINE* and *E_COL* elements represents the line (row)
        and column where the cursor is located. Unless invoked with
        restore enabled, when GRIEF is started the cursor is located
        at top of the current buffer, being line one(1) and column
        one(1).

     Cursor Mode::

        The *E_CURSOR* element represents the optional cursor mode.

        On systems which have means of controlling the cursor, the
        current insert/overstrike mode is represented by the cursor
        shape; when overstrike mode a large/block cursor is used
        wheras insert mode shall utilise a small/underline cursor.

        Otherwise on systems without cursor control a mode indicator
        shall be displayed, 'OV' when overstrike is active otherwise
        blank when in insert mode.

     Remember Status::

        The *E_REMEMBER* element represents the remember status.

        When macro recording is active or paused, 'RE' and 'PA'
        respectively are displayed.

     Time::

        Last item in the status area is the time, which is displayed
        in hours and minutes, with a colon as a separator either
        using a 12-hour format *E_TIME* or a 24-hour format *E_TIME24*.

    Macro Returns:
        The 'echo_line()' primitive returns the previous flags value.

    Macro Portability:
        n/a

    Macro See Also:
        inq_line_col, inq_prompt, set_echo_format, inq_echo_format
 */
void
do_echo_line(void)              /* int ([int flags]) */
{
    const accint_t oflags = (accint_t)xf_echoflags;

    if (!isa_undef(1)) {
        xf_echoflags = get_xinteger(1, 0);
        if (x_display_enabled > 0) {
            elinecol(LC_FORCE);                 /* recalc/display */
        }
    }
    acc_assign_int(oflags);
}


/*  Function:           do_set_echo_format
 *      set_echo_format primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: set_echo_format - Set the echo line format.

        void
        set_echo_format([string format = NULL])

    Macro Description:
        The 'set_echo_format()' primitive sets or clears the current
        status area format specification. The format shall be applied
        to the echo-line overriding the fixed layout implied by
        <echo_line>

        The status area layout can be defined by one of two means.
        The 'echo_line' specification controls the elements visible
        using a fixed layout whereas 'set_echo_format' allows a user
        defined format specification.

        The following example specification

>           "grief %v: %b (%m) %l %c %t"

        results in

>           grief v2.6.1: (-rw-rw-rw-rw) Line: 165 Col: 1 3:14pm

        The echo-line format is applied when the *E_FORMAT* flag is
        enabled (see echo_line). Setting an echo line format implies
        *E_FORMAT* similarly clearing the format disables *E_FORMAT*.

        Note!:
        When errors are encountered while evaluating the format
        specification the incorrect section shall be simply echoed
        within the echo-line; otherwise screen updating would loop.

    Macro Parameters:
        format - Optional echo line format specified, if omitted the
            format is cleared the current <echo_line> shall take
            effect.

        Format Specification::

        A format specification, which consists of optional and
        required fields, has the following form:

>           %[flags][modifier]<type>

        Each field of the format specification is a single character
        or a number signifying a particular format option. The
        simplest format specification contains only the percent sign
        and a type character (for example, %b). If a percent sign is
        followed by a character that has no meaning as a format field,
        the character is simply copied. For example, to print a
        percent-sign character, use %%.

        The optional fields, which appear before the type character,
        control other aspects of the formatting, as follows:

        type - Required character that determines whether the
            associated argument is interpreted as a character, a
            string, or a number.

        flags - Optional character or characters that control
            justification of output and printing of signs, blanks,
            decimal points, and octal and hexadecimal prefixes. More
            than one flag can appear in a format specification.

        modifier - Optional number that specifies a format modifier,
            selected an alternative form of the associated 'type'. If
            omitted the modifier assumes a value of 0.

        Following is a description of the possible echo-line
        attributes. The second character in "item" is the type:

(start table,format=nd)
        [Type       [Description                                        ]
    !   b           Buffer details.

                        [[Modifier  [Value                          ]
                     !! 0           Buffer title.
                     !! 1           Buffer number as decimal.
                     !! 2           Buffer number as hex.

    !   f           Buffer flags.

                        [[Modifier  [Value                          ]
                     !! 0           Flags0
                     !! 1           Flags1
                     !! 2           Flags2
                     !! 3           Flags3
                     !! 4           Flags4

    !   n           File name with directory.

    !   N           File name without the directory.

    !   p           Percent string.

    !   c           Column number.

                        [[Modifier  [Value                          ]
                     !! 0           Col: xxx
                     !! 1           xxx

    !   m           Mode string (i.e. --rw-rw-rw).

    !   o           Overwrite mode, " OV" otherwise "".

    !   O           Overwrite/insert flag, like %o yet shows OV/RE.

    !   C           Character value.

    !   V           Virtual character indicator.

    !   r           Remember flag.

    !   l           Line number.

                        [[Modifier  [Value                          ]
                     !! 0           Line: xxx
                     !! 1           xxx

    !   L           Number of lines in the file.

    !   t           Current time.

                        [[Modifier  [Value                          ]
                     !! 0           12 hour
                     !! 1           24 hour

    !   d           Current date.

                        [[Modifier  [Value                          ]
                     !! 0           yyyy-mm[m]-dd
                     !! 1           yy-mm[m]-dd
                     !! 2           dd-mm[m]-yyyy
                     !! 3           mm[m]-dd-yyyy
                     !! 4           dd-mm[m]-yy
                     !! 5           mm[m]-dd-yy
                     !! 6           dd-mm[m]
                     !! 7           mm-dd

    !   v           GRIEF Version.

    !   Y           Current Year.

                        [[Modifier  [Value                          ]
                     !! 0           YYYY
                     !! 1           YY

    !   M           Current month of the year.

                        [[Modifier  [Value                          ]
                     !! 0           MM
                     !! 1           M[M]
                     !! 2           Name
                     !! 3           Abbrev

    !   D           Current day of the month.

                        [[Modifier  [Value                          ]
                     !! 0           DD
                     !! 1           D[D]
                     !! 2           Name
                     !! 3           Abbrev
(end table)

    The following flags are supported;

(start table,format=nd)

    [Flag   [Description                            [Default            ]

 !  -       Left align the result within the        Right align.
            given field width.

 !  +       Prefix the output value with a          Sign appears only
            sign (+ or -) if the output value       for negative signed
            is of a signed type.                    values ().

 !  0       If width is prefixed with '0',          No padding.
            zeros are added until the minimum
            width is reached. If '0' and
            appear, the '0' is ignored. If
            '0' is specified with an integer
            format ('i', 'u', 'x', 'X', 'o',
            'd') the 0 is ignored.

 !  <space> Prefix the output value with a          No blank appears.
            blank if the output value is
            signed and positive; the blank is
            ignored if both the blank and +
            flags appear.
(end table)

     Macro Parameters:
        format - Echo line format specification, if omitted or an
            empty string the specification is cleared.

    Macro Returns:
        nothing

    Macro Portability:
        A Grief extension.

    Macro See Also:
        set_echo_format, inq_echo_format, echo_line, inq_echo_line
 */
void
do_set_echo_format(void)          /* void (string format) */
{
    set_echo_format(get_xstr(1));
}


/*  Function:           set_echo_format
 *      set_echo_format implementation.
 *
 *  Parameters:
 *      fmt - Echo line specification.
 *
 *  Returns:
 *      nothing
 */
void
set_echo_format(const char *fmt)
{
    int change = 0;

    if (xf_echofmt) {
        chk_free((char *) xf_echofmt);
        xf_echoflags &= ~E_FORMAT;
        xf_echofmt = NULL;
        ++change;
    }

    if (fmt && *fmt) {                          /* non-empty string */
        xf_echofmt = chk_salloc(fmt);
        xf_echoflags |= E_FORMAT;
        ++change;
    }

    if (change && (x_display_enabled > 0)) {
        elinecol(LC_FORCE);                     /* recalc/display */
    }
}


/*  Function:           inq_echo_line
 *      inq_echo_line primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: inq_echo_line - Retrieve the echo-line flags.

        int
        inq_echo_line()

    Macro Description:
        The 'inq_echo_line()' primitive retrieves the current
        echo-line flags.

    Macro Parameters:
        none

    Macro Returns:
        The 'inq_echo_line()' primitive returns the current
        echo_line flags.

    Macro Portability:
        A Grief extension.

    Macro See Also:
        set_echo_format, inq_echo_format, echo_line, inq_echo_line
 */
void
inq_echo_line(void)             /* int () */
{
    acc_assign_int(xf_echoflags);
}


/*  Function:           inq_echo_format
 *      inq_echo_format primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: inq_echo_format - Retrieve the echo-line format.

        int
        inq_echo_format()

    Macro Description:
        The 'inq_echo_format()' primitive retrieves the current
        echo-line format specification.

    Macro Parameters:
        none

    Macro Returns:
        The 'inq_echo_format' returns the current echo line format
        specification string, otherwise an empty string.

    Macro Portability:
        A Grief extension.

    Macro See Also:
        set_echo_format, inq_echo_format, echo_line, inq_echo_line
 */
void
inq_echo_format(void)           /* string () */
{
    acc_assign_str(xf_echofmt ? xf_echofmt : "", -1);
}


/*  Function:           infof_truncated
 *      Print a message contained a filename, truncating the filename
 *      if too long for the echo line.
 *
 *      The format argument must be a sprintf() style string containing
 *      *TWO* %s format specifications, one for the leading and the
 *      other for the trailing filename components.
 *
 *  Parameters:
 *      fmt - Format specification,
 *      filename - File name.
 *
 *  Returns:
 *      nothing
 */
void
infof_truncated(const char *fmt, const char *filename)
{
    const char *dots = "";

    if (lc_column) {
        const int len = (int)(strlen(fmt) - 2 + strlen(filename));

        if (len >= lc_column) {
            filename += len - lc_column + 3;
            dots = "...";
        }
    }
    infof(fmt, dots, filename);
}


/*  Function:           infof, infos, errorf, errorfx
 *      Message level filtered information and error messages.
 *
 *  Parameters:
 *      fmt - Format specification.
 *      ... - Parameters.
 *
 *  Returns:
 *      nothing
 */
void
infof(const char *str, ...)
{
    if (x_msglevel <= 0 || 2 == x_msglevel) {   /* 0 or 2 */
        char lbuf[EBUFSIZ];
        va_list ap;

        va_start(ap, str);
        if (str) {
            int iolen = vsxprintf(lbuf, sizeof(lbuf) - 4, str, ap);
            if (x_pause_on_message) {
                strcpy(lbuf + iolen, "..");
            }
            str = lbuf;
        } else {
            str = va_arg(ap, const char *);
        }
        va_end(ap);

        ewputs(str);
        if (x_pause_on_message) {
            (void)io_get_key(0);
        }
    }
}


void
infos(const char *str)
{
    if (x_msglevel <= 0 || 2 == x_msglevel) {   /* 0 or 2 */
        ewputs(str);
        if (x_pause_on_message) {
            (void)io_get_key(0);
        }
    }
}


void
errorf(const char *str, ...)
{
    if (x_msglevel <= 1) {                      /* 0 or 1 */
        char lbuf[EBUFSIZ];
        va_list ap;

        va_start(ap, str);
        if (str && strcmp("%s", str)) {
            int iolen = vsxprintf(lbuf, sizeof(lbuf) - 4, str, ap);
            if (x_pause_on_error) {
                strcpy(lbuf + iolen, "..");
            }
            str = lbuf;
        } else {
            str = va_arg(ap, const char *);
        }
        va_end(ap);

        eeputs(str);
        if (x_pause_on_error) {
            (void)io_get_key(0);
        }
    }
}


void
errorfx(const char *fmt, ...)
{
    if (x_msglevel <= 1) {                      /* 0 or 1 */
        const int xerrno = errno;
        char iobuf[EBUFSIZ];
        va_list ap;
        int iolen;

        va_start(ap, fmt);
        iolen = vsxprintf(iobuf, sizeof(iobuf) - 36, fmt, ap);
        if (xerrno) {
            iolen += sxprintf(iobuf + iolen , sizeof(iobuf) - (iolen + 4),
                        ": %s (%d)", str_error(xerrno), xerrno);
        }
        va_end(ap);

        if (x_pause_on_error) {
            strcpy(iobuf + iolen, "..");
        }
        eeputs(iobuf);
        if (x_pause_on_error) {
            (void)io_get_key(0);
        }
    }
}


/*  Function:           eprintlen
 *      Determine the display length of the specified buffer 'str'.
 *
 *  Parameters:
 *      str - Buffer.
 *
 *  Returns:
 *      Length of the buffer in bytes.
 */
static int
eprintlen(const WChar_t *str)
{
    WChar_t ch;
    int len = 0;
    if (str) {
        while (0 != (ch = *str++)) {
            len += eprintable(ch, NULL, NULL);
        }
    }
    return len;
}


/*  Function:           eprintable
 *      Converts a character to a printable format taking into account whether
 *      the terminal can support printable 8-bit chars, etc.
 *
 *  Parameters:
 *      ch - Character value.
 *      buf - Destination buffer.
 *      buflen - Length, in bytes, of the output buffer.
 *
 *  Returns:
 *      Display width.
 */
static int
eprintable(const vbyte_t c, vbyte_t *buf, int *buflen)
{
    const vbyte_t attr = VBYTE_ATTR_GET(c);
    vbyte_t ch = VBYTE_CHAR_GET(c);
    char t_buffer[32];
    int idx, len = 0;

    if (ch <= 0xff) {
        /*
         *  normal characters, apply base character-map
         */
        const cmapchr_t *mc = cmapchr_lookup(x_base_cmap, ch);

        len = mc->mc_length;
        if (buf) {
            const unsigned char *mccp = (unsigned char *) mc->mc_str;
            if (NULL == mccp) {
                if ((ch = mc->mc_chr) > 0 && ch < 0x80) {
                    *buf++ = ch | attr;
                } else {
                    goto ischaracter;
                }
            } else {
                assert(len < *buflen);
                for (idx = 0; idx < len; ++idx) {
                    *buf++ = mccp[idx] | attr;
                }
            }
            *buflen = len;
        }
        return len;
    }

    /*
     *  extended character values
     */
ischaracter:;
    if (vtisunicode() || vtisutf8()) {
        const int width = Wcwidth(ch);
        if (width >= 0) {
            if (buf) {
                buf[0] = (ch & VBYTE_CHAR_MASK) | attr;
                buf[1] = 0;
                *buflen = 1;
            }
            return width;
        }
    }
    len = sxprintf(t_buffer, sizeof(t_buffer), "u%04x", ch);
    if (buf) {
        assert(len < *buflen);
        for (idx = 0; idx < len; ++idx) {
            *buf++ = t_buffer[idx] | attr;
        }
        *buflen = len;
    }
    return len;
}


/*  Function:           eredraw
 *      Rerender the echo line, this is normally performed when the
 *      screen is redrawn; for example when the window is resized.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 */
void
eredraw(void)
{
    eprompt(TRUE, -1);
}


/*  Function:           ef_format
 *      format the echo-line.
 *
 *>         %[#-+0 -][modifier]<type>
 *
 *  Parameters:
 *      fmt - Echo line format specification.
 *      cp - Buffer pointer.
 *      status - Edit state information, buffer and time/date.
 *
 *  Returns:
 *      nothing
 */
static void
ef_format(const char *fmt, struct _estate *s)
{
    WChar_t *cp = s->buffer;
    if (fmt) {
        char ch;

        while (0 != (ch = *fmt++)) {
            int altform = 0, justification = 0, pad = 0, modifier = 0;

            __CUNUSED(altform)                  /* TODO */
            __CUNUSED(justification)
            __CUNUSED(pad)

            if ('%' != ch) {                    /* format otherwise echo */
                *cp++ = ch;
                continue;
            }

            if (0 == (ch = *fmt++)) {           /* NUL, echo and exit */
                *cp++ = '%';
                break;
            } else if ('%' == ch) {             /* literal */
                *cp++ = '%';
                continue;
            }

            while (strchr("#-+0 ", ch)) {       /* justification/padding */
                switch (ch) {
                case '#': altform = 1; break;
                case '-': case '+': justification = ch; break;
                case '0': case ' ': pad = ch; break;
                }
                ch = *++fmt;
            }

            if (isdigit(ch)) {                  /* optional modifier */
                do {
                    modifier = (modifier * 10) + (ch - '0');
                    ch = *fmt++;
                } while (ch && isdigit(ch));
                if (0 == ch)  {
                    break;
                }
            }

            /* format */
            switch (ch) {
            case 'b':       /* Buffer */
                if (curbp) {
                    switch (modifier) {
                    case 2:         /* Number as hex */
                        break;
                    case 1:         /* Number as decimal */
                        break;
                    default:        /* Title */
                        cp = ef_utf8(cp, curbp->b_title, s);
                        break;
                    }
                }
                break;

            case 'f':       /* Buffer flags */
                if (curbp) {
                    switch (modifier) {
                    case 4:         /* Flags4 */
                        break;
                    case 3:         /* Flags3 */
                        break;
                    case 2:         /* Flags2 */
                        break;
                    case 1:         /* Flags1 */
                        break;
                    default:
                        if ('m' == *fmt) {          // modified status
                        } else if ('r' == *fmt) {   // read-only status
                        }
                        break;
                    }
                }
                break;

            case 'n':       /* File name with directory */
                if (curbp) {
                    cp = ef_utf8(cp, curbp->b_fname, s);
                }
                break;

            case 'N':       /* File name without the directory */
                if (curbp && curbp->b_fname) {
                    cp = ef_utf8(cp, sys_basename(curbp->b_fname), s);
                }
                break;

            case 'p':       /* Percent string */
                cp = ef_percent(cp, s);
                break;

            case 'c':       /* Column number */
                switch (modifier) {
                case 1:             /* xxx */
                    cp = ef_integer(cp, s->bf_col, 0, s);
                    break;
                default:            /* Col: xxx */
                    cp = ef_col(cp, s);
                }
                break;

            case 'm':       /* Mode string (i.e. --rw-rw-rw) */
                cp = ef_filemode(cp, s);
                break;

            case 'o':       /* Overwrite mode, " OV" otherwise "" */
                cp = ef_ovmode(cp, s);
                break;

            case 'O':       /* Overwrite/insert flag - like %o, but shows OV/RE */
                cp = ef_imode(cp, s);
                break;

            case 'C':       /* Character value */
                cp = ef_charvalue(cp, s);
                break;

            case 'V':       /* Virtual character indicator */
                cp = ef_virtual(cp, s);
                break;

            case 'r':       /* Remember flag */
                if (x_rem_string[0] != ' ') {
                    cp = ef_buffer(cp, x_rem_string, s);
                    cp = ef_space(cp, s);
                }
                break;

            case 'l':       /* Line number */
                switch (modifier) {
                case 1:             /* xxx */
                    cp = ef_integer(cp, s->bf_line, 0, s);
                    break;
                default:            /* Line: xxx */
                    cp = ef_line(cp, s);
                }
                break;

            case 'L':       /* Number of lines in the file */
                cp = ef_numlines(cp, s);
                break;

            case 't':       /* Time (12 or 24 hour) */
                cp = ef_time(cp, modifier ? TRUE : FALSE, s);
                break;

            case 'd':       /* Date, optional format */
                cp = ef_date(cp, modifier, s);
                break;

            case 'v':       /* Version */
                cp = ef_version(cp, s);
                break;

            case 'Y':       /* Year */
                switch (modifier) {
                case 1:             /* YY */
                    cp = ef_integer(cp, s->tm_year%100, 2, s);
                    break;
                default:            /* YYYY */
                    cp = ef_integer(cp, s->tm_year, 4, s);
                    break;
                }
                break;

            case 'M':       /* Month of the year */
                switch (modifier) {
                case 3:             /* Abbrev */
                    cp = ef_buffer(cp, tm_month_abbrev(s->tm_month - 1), s);
                    break;
                case 2:             /* Name */
                    cp = ef_buffer(cp, tm_month_name(s->tm_month - 1), s);
                    break;
                case 1:             /* M[M] */
                    cp = ef_integer(cp, s->tm_month, 0, s);
                    break;
                default:            /* MM */
                    cp = ef_integer(cp, s->tm_month, 2, s);
                    break;
                }
                break;

            case 'D':       /* Day of the month */
                switch (modifier) {
                case 3:             /* Abbrev */
                    cp = ef_buffer(cp, tm_day_abbrev(s->tm_day - 1), s);
                    break;
                case 2:             /* Name */
                    cp = ef_buffer(cp, tm_day_abbrev(s->tm_day - 1), s);
                    break;
                case 1:             /* D[D] */
                    cp = ef_integer(cp, s->tm_day, 0, s);
                    break;
                default:            /* DD */
                    cp = ef_integer(cp, s->tm_day, 2, s);
                    break;
                }
                break;

            default:        /* Unknown, just echo */
                *cp++ = '%';
                *cp++ = ch;
                break;
            }
        }
    }
    *cp = 0;
}


/*  Function:           ef_space
 *      Insert a space attribute into the echo_line buffer.
 *
 *  Parameters:
 *      cp - Echo-line buffer cursor.
 *      s - Edit state.
 *
 *  Returns:
 *      resulting buffer cursor.
 */
static WChar_t *
ef_space(WChar_t *cp, const struct _estate *s)
{
    const WChar_t *end = buffer_end(s);
    if (cp < end) {
        *cp++ = ' ';
        *cp = 0;
    }
    return cp;
}


/*  Function:           ef_buffer
 *      Insert the specfied buffer 'buf' into the echo_line buffer.
 *
 *  Parameters:
 *      cp - Echo-line buffer cursor.
 *      buf - Buffer to be inserted.
 *      s - Edit state.
 *
 *  Returns:
 *      resulting buffer cursor.
 */
static WChar_t *
ef_buffer(WChar_t *cp, const char *buf, const struct _estate *s)
{
    const WChar_t *end = buffer_end(s);
    if (buf && cp < end) {
        WChar_t c;
        while (cp < end && 0 != (c = *buf++)) {
            *cp++ = c;
        }
        *cp = 0;
    }
    return cp;
}


/*  Function:           ef_utf8
 *      Insert the specfied utf8 encoded buffer 'buf' into the echo_line buffer.
 *
 *  Parameters:
 *      cp - Echo-line buffer cursor.
 *      buf - Buffer to be inserted.
 *      s - Edit state.
 *
 *  Returns:
 *      resulting buffer cursor.
 */
static WChar_t *
ef_utf8(WChar_t *cp, const char *buf, const struct _estate *s)
{
    return cp + (buf ? Wcsfromutf8(buf, cp, buffer_end(s) - cp) : 0);
}


/*  Function:           ef_integer
 *      Insert an integer attribute into the echo_line buffer.
 *
 *  Parameters:
 *      cp - Echo-line buffer cursor.
 *      ivalue - Integer value.
 *      width - Optional field width.
 *      s - Edit state.
 *
 *  Returns:
 *      resulting buffer cursor
 */
static WChar_t *
ef_integer(WChar_t *cp, int ivalue, int width, const struct _estate *s)
{
    char t_buffer[32];
    if (width > 0) {
        sprintf(t_buffer, "%0*d", width, ivalue);
    } else if (width < 0) {
        sprintf(t_buffer, "%*d", width * -1, ivalue);
    } else {
        sprintf(t_buffer, "%d", ivalue);
    }
    return ef_buffer(cp, t_buffer, s);
}



/*  Function:           ef_ovmode
 *      Insert the ovmode into the echo_line buffer.
 *
 *  Parameters:
 *      cp - Echo-line buffer cursor.
 *      s - Edit state.
 *
 *  Returns:
 *      resulting buffer cursor.
 */
static WChar_t *
ef_ovmode(WChar_t *cp, const struct _estate *s)
{
    __CUNUSED(s)
    if (0 == x_pt.pt_icursor[0]) {              /* visual cursor not available */
        *cp++ = ' ';
        if (FALSE == buf_imode(curbp)) {
            *cp++ = 'O';
            *cp++ = 'V';
        }
        *cp = '\0';
    }
    return cp;
}


/*  Function:           ef_charvalue
 *      Insert the current character value into the echo_line buffer.
 *
 *  Parameters:
 *      cp - Character value.
 *      s - Edit state.
 *
 *  Returns:
 *      resulting buffer cursor.
 */
static WChar_t *
ef_charvalue(WChar_t *cp, struct _estate *s)
{
    int vstatus, charvalue;
    char t_buffer[32];

    __CUNUSED(s)

    t_buffer[0] = 0;
    if (-999 == (vstatus = s->vf_status)) {     /* loaded? */
        s->vf_status = vstatus = line_current_status(s->vf_values, 4);
    }
    charvalue = s->vf_values[0];

    if (0 == (BUFFERVSTATUS_ILLEGAL & vstatus)) {
        if ((BUFFERVSTATUS_EOL|BUFFERVSTATUS_XEOL|BUFFERVSTATUS_PEOL) & vstatus) {
            strcpy(t_buffer, " EOL ");          /* <EOL> */

        } else if (BUFFERVSTATUS_EOF == vstatus) {
            strcpy(t_buffer, " EOF ");          /* <EOF> */

        } else if (charvalue <= 0) {
            strcpy(t_buffer, " NUL ");
                                                /* control */
        } else if (charvalue <= 0x1f) {
            sprintf(t_buffer, " ^%c  ", 'A' + (charvalue - 1));

                                                /* ascii */
        } else if (charvalue <= 0x7f && isprint(charvalue)) {
            sprintf(t_buffer, " [%c] ", charvalue);
        }
    }

    if (0 == t_buffer[0]) {
        if (0 == charvalue) {                   /* others */
            strcpy(t_buffer, "u0000 ");

        } else {
            unsigned i;
            for (i = 0; i < 4 && (charvalue = s->vf_values[i]) > 0; ++i) {
                if (charvalue <= 0xff) {
                    sprintf(t_buffer, "%c0x%02x ", (i ? '+' : ' '), charvalue);

                } else if (charvalue <= 0xffff) {
                    sprintf(t_buffer, "%cu%04x ", (i ? '+' : ' '), charvalue);

                } else {
                    sprintf(t_buffer, "%cU%06x ", (i ? '+' : ' '), charvalue);
                }
            }
        }
    }

    return ef_buffer(cp, t_buffer, s);
}


/*  Function:           ef_virtual
 *      Insert the virtual-space into the echo_line buffer.
 *
 *  Parameters:
 *      cp - Echo-line buffer cursor.
 *      s - Edit state.
 *
 *  Returns:
 *      resulting buffer cursor.
 */
static WChar_t *
ef_virtual(WChar_t *cp, struct _estate *s)
{
    int status;

    __CUNUSED(s)

    if (-999 == (status = s->vf_status)) {      /* loaded? */
        s->vf_status = status = line_current_status(s->vf_values, 4);
    }

    *cp++ = (BUFFERVSTATUS_ILLEGAL & status ? '!' :
                (BUFFERVSTATUS_XEOL & status ? '>' :
                    (BUFFERVSTATUS_PEOL & status ? '+' :
                        (BUFFERVSTATUS_EOL & status ? '$' :
                (BUFFERVSTATUS_VIRTUAL & status ? 'X' :
                (BUFFERVSTATUS_MULTIBYTE & status ? '#' : ' '))))));
    *cp = '\0';
    return cp;
}


/*  Function:           ef_imode
 *      Insert the insert-mode into the echo_line buffer.
 *
 *  Parameters:
 *      cp - Echo-line buffer cursor.
 *      s - Edit state.
 *
 *  Returns:
 *      resulting buffer cursor.
 */
static WChar_t *
ef_imode(WChar_t *cp, const struct _estate *s)
{
    __CUNUSED(s)
    if (buf_imode(curbp)) {
        *cp++ = 'R';
        *cp++ = 'E';
    } else {
        *cp++ = 'O';
        *cp++ = 'V';
    }
    *cp = '\0';
    return cp;
}


/*  Function:           ef_numlines
 *      Insert the buffer total line number into the echo_line buffer.
 *
 *  Parameters:
 *      cp - Echo-line buffer cursor.
 *      s - Edit state.
 *
 *  Returns:
 *      resulting buffer cursor.
 */
static WChar_t *
ef_numlines(WChar_t *cp, const struct _estate *s)
{
    const int numlines = (int)(curbp ? curbp->b_numlines : 1);
    char t_buffer[32];

    __CUNUSED(s)
    sprintf(t_buffer, numlines > 9999 ? "%u" : "%-4u", numlines);
    return ef_buffer(cp, t_buffer, s);
}


/*  Function:           ef_filemode
 *      Insert the buffer filemode into the echo_line buffer.
 *
 *  Parameters:
 *      cp - Echo-line buffer cursor.
 *      s - Edit state.
 *
 *  Returns:
 *      resulting buffer cursor
 */
static WChar_t *
ef_filemode(WChar_t *cp, const struct _estate *s)
{
    int mode = (curbp ? curbp->b_mode : 0);
    char t_buffer[16];

    __CUNUSED(s)
    file_modedesc((mode_t)mode, NULL, 0, t_buffer, sizeof(t_buffer));
    return ef_buffer(cp, t_buffer, s);
}


/*  Function:           ef_line
 *      Insert the buffer cursor line into the echo_line buffer.
 *
 *  Parameters:
 *      cp - Echo-line buffer cursor.
 *      s - Edit state.
 *
 *  Returns:
 *      resulting buffer cursor.
 */
static WChar_t *
ef_line(WChar_t *cp, const struct _estate *s)
{
    char t_buffer[32];
    sprintf(t_buffer, s->bf_line > 9999 ? "Line:%-5u" : "Line: %-4u", s->bf_line);
    return ef_buffer(cp, t_buffer, s);
}


/*  Function:           ef_col
 *      Insert the buffer cursor column into the echo_line buffer.
 *
 *  Parameters:
 *      cp - Echo-line buffer cursor.
 *      s - Edit state.
 *
 *  Returns:
 *      resulting buffer cursor
 */
static WChar_t *
ef_col(WChar_t *cp, const struct _estate *s)
{
    char t_buffer[32];
    sprintf(t_buffer, s->bf_col > 99 ? "Col:%-3u" : "Col: %-2u", s->bf_col);
    return ef_buffer(cp, t_buffer, s);
}


/*  Function:           ef_percent
 *      Insert the buffer cursor percent (by line) into the echo_line buffer.
 *
 *  Parameters:
 *      cp - Echo-line buffer cursor.
 *      s - Edit state.
 *
 *  Returns:
 *      resulting buffer cursor.
 */
static WChar_t *
ef_percent(WChar_t *cp, const struct _estate *s)
{
    char t_buffer[32];
    accint_t perc;

    perc = ((accint_t)s->bf_line * 100) / (accint_t)(curbp && curbp->b_numlines > 0 ? curbp->b_numlines : 1);
    if (perc >= 100) {
        strcpy(t_buffer, "END");
    } else {
        sprintf(t_buffer, "%2lu%%", perc);
    }
    return ef_buffer(cp, t_buffer, s);
}


/*  Function:           ef_time
 *      Insert the current time the echo_line buffer.
 *
 *  Parameters:
 *      cp - Echo-line buffer cursor.
 *      s - Edit state.
  *
 *  Returns:
 *      resulting buffer cursor.
 */
static WChar_t *
ef_time(WChar_t *cp, int hour24, const struct _estate *s)
{
    char t_buffer[32];

    if (hour24) {
        sprintf(t_buffer, "%02d:%02d", s->tm_hour, s->tm_min);
    } else {
        sprintf(t_buffer, "%d:%02d%cm",
            (s->tm_hour > 12 ? s->tm_hour - 12 : (s->tm_hour == 0 ? 12 : s->tm_hour)),
            s->tm_min, s->tm_hour >= 12 ? 'p' : 'a');
    }
    return ef_buffer(cp, t_buffer, s);
}


/*  Function:           ef_date
 *      Insert the current date the echo_line buffer.
 *
 *  Parameters:
 *      cp - Echo-line buffer cursor.
 *      s - Edit state.
 *
 *  Returns:
 *      resulting buffer cursor
 */
static WChar_t *
ef_date(WChar_t *cp, int format, const struct _estate *s)
{
    char t_buffer[32], t_month[16];
    char delim = '-';

    /* 30.. delimiter as '/' */
    if (format >= 30) {
        format -= 20;
        delim = '/';
    }

    /* 10.. month abbrev */
    if (format >= 10 && format <= 17) {
        strcpy(t_month, tm_month_abbrev(s->tm_month - 1));
        format -= 10;

    /* otherwise numeric date */
    } else {
        sprintf(t_month, "%02d", s->tm_month);
    }

    switch (format) {
    case 7:             /* mm-dd */
        sprintf(t_buffer, "%s%c%02d", t_month, delim, s->tm_day);
        break;
    case 6:             /* dd-mm[m] */
        sprintf(t_buffer, "%02d%c%s", s->tm_day, delim, t_month);
        break;
    case 5:             /* mm[m]-dd-yy */
        sprintf(t_buffer, "%s%c%02d%c%02d", t_month, delim, s->tm_day, delim, s->tm_year%100);
        break;
    case 4:             /* dd-mm[m]-yy */
        sprintf(t_buffer, "%02d%c%s%c%02d", s->tm_day, delim, t_month, delim, s->tm_year%100);
        break;
    case 3:             /* mm[m]-dd-yyyy */
        sprintf(t_buffer, "%s%c%02d%c%04d", t_month, delim, s->tm_day, delim, s->tm_year);
        break;
    case 2:             /* dd-mm[m]-yyyy */
        sprintf(t_buffer, "%02d%c%s%c%04d", s->tm_day, delim, t_month, delim, s->tm_year);
        break;
    case 1:             /* yy-mm[m]-dd */
        sprintf(t_buffer, "%02d%c%s%c%02d", s->tm_year%100, delim, t_month, delim, s->tm_day);
        break;
    case 0:             /* yyyy-mm[m]-dd */
    default:
        sprintf(t_buffer, "%04d%c%s%c%02d", s->tm_year, delim, t_month, delim, s->tm_day);
        break;
    }
    return ef_buffer(cp, t_buffer, s);
}


/*  Function:           ef_version
 *      Insert the editor version into the echo_line buffer.
 *
 *  Parameters:
 *      cp - Echo-line buffer cursor.
 *      s - Edit state.
 *
 *  Returns:
 *      resulting buffer cursor
 */
static WChar_t *
ef_version(WChar_t *cp, const struct _estate *s)
{
    __CUNUSED(s)
    return ef_buffer(cp, x_version, s);
}

/*end*/
