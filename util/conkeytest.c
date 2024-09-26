/* -*- mode: c; indent-width: 4; -*- */
/* $Id: conkeytest.c,v 1.11 2024/09/20 03:21:34 cvsuser Exp $
 * console key-test -- ttyconsole
 * Build: gcc -o conkeytest conkeytest.c conkey.c
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

#include <termios.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <assert.h>
#include <unistd.h>
#include <getopt.h>

#include "conkeywin.h"
#include "conkey.h"

enum KeyboardMode {
        KEYBOARD_STANDARD,
        KEYBOARD_CYGWIN,
        KEYBOARD_MSTERMINAL,
        KEYBOARD_XTERM_META,
        KEYBOARD_XTERM_MOK2,
        KEYBOARD_KITTY
};

enum MouseMode {
        MOUSE_OFF,
        MOUSE_XTERM,
        MOUSE_XTERM_X11,
        MOUSE_XTERM_SGR
};

struct uversion {
#define UVERSION_CYGWIN 0x01
#define UVERSION_LINUX 0x02
        unsigned umajor;
        unsigned uminor;
        unsigned upatch;
};

static void usage(void);
static const char *progname_get(const char *argv0);

static void runner(const enum MouseMode mmode, const enum KeyboardMode kmode);
static void hex(const void *buf, unsigned len);
static void prints(const char *str);
static void mouse_mode(const enum MouseMode mode, int state);
static void keyboard_mode(const enum KeyboardMode mode, int state);

static int uversion_get(struct uversion *uv);
static int uversion_cmp(const struct uversion *uv, unsigned umajor, unsigned uminor, unsigned upatch);

static const char *progname = "";
static const char *short_options = "k:m:b:qh";
static struct option long_options[] = {
        {"key", required_argument, NULL, 'k'},
        {"mouse", required_argument, NULL, 'm'},
        {"bufsiz", required_argument, NULL, 'b'},
        {"quiet", no_argument, NULL, 'q'},
        {"help", no_argument, NULL, 'h'},
        {NULL}
};

static unsigned overbose = 1;
static size_t obufsiz = 128;


int
main(int argc, char **argv)
{
        enum MouseMode mmode = MOUSE_OFF;
        enum KeyboardMode kmode = KEYBOARD_STANDARD;
        int optidx = 0, c;

        progname = progname_get(argv[0]);
        while ((c = getopt_long(argc, argv, short_options, long_options, &optidx)) != EOF) {
                switch (c) {
                case 'v':   // -v,--verbose
                        break;

                case 'k':   // -k,--key=<mode>
                        if (0 == strcmp(optarg, "standard")) {
                                kmode = KEYBOARD_STANDARD;
                        } else if (0 == strcmp(optarg, "cygwin")) {
                                kmode = KEYBOARD_CYGWIN;
                        } else if (0 == strcmp(optarg, "msterminal")) {
                                struct uversion uv = {0};

                                 if (UVERSION_CYGWIN == uversion_get(&uv) && uversion_cmp(&uv, 3, 5, 5) < 0) {
                                        printf("cygwin: %u.%u.%u, wont correctly support win-input-mode\n",
                                            uv.umajor, uv.uminor, uv.upatch);
                                        return EXIT_FAILURE;
                                }
                                kmode = KEYBOARD_MSTERMINAL;
                        } else if (0 == strcmp(optarg, "xterm-meta")) {
                                kmode = KEYBOARD_XTERM_META;
                        } else if (0 == strcmp(optarg, "xterm-mok2")) {
                                kmode = KEYBOARD_XTERM_MOK2;
                        } else if (0 == strcmp(optarg, "kitty")) {
                                kmode = KEYBOARD_KITTY;
                        } else {
                                printf("%s: unexpected raw-mode <%s>\n", progname, optarg);
                                usage();
                        }
                        break;

                case 'm':   // -m,--mouse=<mode>
                        if (0 == strcmp(optarg, "off")) {
                                mmode = MOUSE_OFF;
                        } else if (0 == strcmp(optarg, "xterm")) {
                                mmode = MOUSE_XTERM;
                        } else if (0 == strcmp(optarg, "x11")) {
                                mmode = MOUSE_XTERM_X11;
                        } else if (0 == strcmp(optarg, "sgr")) {
                                mmode = MOUSE_XTERM_SGR;
                        } else {
                                printf("%s: unexpected mouse-mode <%s>\n", progname, optarg);
                                usage();
                        }
                        break;

                case 'q':   // -q,--quiet
                        overbose = 0;
                        break;

                case 'b':   // -b,--bufsize=bytes
                        if ((obufsiz = strtoul(optarg, NULL, 0)) < 16) {
                                obufsiz = 16;
                        }
                        break;

                case 'h':   // -h,--help
                        usage();
                        return EXIT_FAILURE;

                default:
                        return EXIT_FAILURE;
                }
        }

        argv += optind;
        if ((argc -= optind) != 0) {
                printf("%s: unexpected argument(s) %s\n", progname, argv[0]);
                usage();
        }

        runner(mmode, kmode);
        return 0;
}


static void
usage(void)
{
        static const char *options[] = {
                "",
                "Options:",
                "   -k,--key=<mode>         Keyboard mode (standard [default],cygwin,msterminal,xterm-meta,xterm-mok2,kitty)",
                "   -m,--mouse=<mode>       Mouse mode (off [default],xterm,x11,sgr)",
                "",
                "   -h,--help               Command line usage",
                ""
                };

        printf("\n%s [options]\n", progname);
        for (unsigned i = 0; i != (sizeof(options)/sizeof(options[0])); ++i)
                printf("%s\n", options[i]);
        exit(EXIT_FAILURE);
}


static const char *
progname_get(const char *argv0)
{
        const char *s1 = rindex(argv0 , '/'),
            *s2 = rindex(argv0, '\\');

        return (s1 > s2 ? (s1 + 1) : (s2 ? (s2 + 1) : argv0));
}


/*
 *  key poller
 */
static void
runner(const enum MouseMode mmode, const enum KeyboardMode kmode)
{
        struct termios bak, cfg;

        char *buf = malloc(obufsiz);
        unsigned event_count = 0;
        INPUT_RECORD ir;
        const char *cursor, *end;
        int cnt, q;

        tcgetattr(STDIN_FILENO, &cfg);
        bak = cfg;
        cfmakeraw(&cfg);
        cfg.c_lflag &= ~ECHO;
        tcsetattr(STDIN_FILENO, TCSADRAIN, &cfg);

        mouse_mode(mmode, 1);
        keyboard_mode(kmode, 1);

        printf("\n\rkey-test (press 'q' twice to exit, kmode=%u, mmode=%u, bufsiz=%u, verbose=%u):\n\r",
                (unsigned)kmode, (unsigned)mmode, obufsiz, overbose);
        fflush(stdout);

        for (q = 0; (cnt = read(STDIN_FILENO, buf, obufsiz)) > 0;) {
                WCHAR firstchar = (cnt == 1 ? buf[0] : 0);

                if (overbose) {
                        hex(buf, cnt);
                }

                for (cursor = buf, end = cursor + cnt; cursor < end; ) {
                        if (kmode) {            // key events
                                if (cursor[0] == 0x1b && (cursor[1] == '{' || cursor[1] == '[')) {
                                        const char *term;

                                        if (NULL != (term = DecodeCygwinKey(&ir, cursor, end)) ||
                                            NULL != (term = DecodeMSTerminalKey(&ir, cursor, end)) ||
                                            NULL != (term = DecodeXTermKey(&ir, cursor, end))) {
                                                const KEY_EVENT_RECORD* ke = &ir.Event.KeyEvent;
                                                const WCHAR ch = ke->uChar.UnicodeChar;

                                                printf("%4d: %s %02u VC=0x%04x/%u, SC=0x%04x/%u, KEY=0x%x/'%c', CTRL=%08llx %s\n\r",
                                                    event_count, (ke->bKeyDown ? "Dn" : "Up"), (unsigned)ke->wRepeatCount,
                                                    (unsigned)ke->wVirtualKeyCode, (unsigned)ke->wVirtualKeyCode,
                                                    (unsigned)ke->wVirtualScanCode, (unsigned)ke->wVirtualScanCode,
                                                        (unsigned)ch, (ch >= ' ' && ch < 0x7f ? ch : '.'), (unsigned long long)ke->dwControlKeyState,
                                                    key_description(ke));
                                                if (ke->bKeyDown && 0 == firstchar) {
                                                        firstchar = ch;
                                                }
                                                ++event_count;
                                                cursor = term;
                                                continue;
                                        }
                                }
                        }
                        if (mmode) {            // mouse events
                                if (cursor[0] == 0x1b && cursor[1] == '[') {
                                        const char *term;

                                        if (NULL != (term = DecodeXTermMouse(&ir, cursor, end)) ||
                                            NULL != (term = DecodeSGRMouse(&ir, cursor, end))) {
                                                const MOUSE_EVENT_RECORD* me = &ir.Event.MouseEvent;

                                                printf("%4d: %03u %04u %03u %s\n\r",
                                                    event_count, (unsigned)me->dwEventFlags,
                                                        (unsigned)LOWORD(me->dwButtonState), (unsigned)me->dwControlKeyState,
                                                    mouse_description(me));
                                                ++event_count;
                                                cursor = term;
                                                continue;
                                        }
                                }
                        }
                        ++cursor;
                }

                if (firstchar) {
                        if (firstchar == 'q' || firstchar == 'Q') {
                                if (++q == 2)
                                        break;
                        } else {
                                q = 0;
                        }
                }

                //prints("\n\r");
        }

        keyboard_mode(kmode, 0);
        mouse_mode(mmode, 0);
        fflush(stdout);

        tcsetattr(STDIN_FILENO, TCSADRAIN, &bak);
}


/*
 *  Buffer hex dump.
 */
static void
hex(const void *buf, unsigned len)
{
#define HEXWIDTH 16
#define HEXCOLUMN 4
        const unsigned char *cursor = buf, *end = cursor + len;
        unsigned offset = 0;

        for (offset = 0; cursor != end; offset += HEXWIDTH) {
                const unsigned char *data = cursor;

                printf("%04x: ", offset);
                for (unsigned col = 0; col < HEXWIDTH; ++col) {
                        if ((col % HEXCOLUMN) == 0 && col)
                                prints(" |");
                        if (data == end) {
                                prints("   ");
                        } else {
                                printf(" %02x", *data++);
                        }
                }

                prints("   ");
                for (unsigned col = 0; col < HEXWIDTH; ++col) {
                        if (cursor == end) {
                                printf("%*s", HEXWIDTH - col, "");
                                break;
                        } else {
                                const unsigned char c = *cursor++;
                                printf("%c", (c >= ' ' && c < 0x7f ? c : '.'));
                        }
                }
                prints("\n\r");
        }
}


static void
prints(const char *str)
{
        fputs(str, stdout);
}


/*
 *  Mouse mode configuration.
 */
static void
mouse_mode(const enum MouseMode mode, int state)
{
        if (mode == MOUSE_OFF)
                return;

        if (state) {
                if (mode == MOUSE_XTERM) {
                        prints("\033[?1000h");              // enable mouse presses.

                } else { // XTERM X11 & SGR
                        prints("\033[?1002h");              // enable cell-motion tracking tracking.
                        if (mode == MOUSE_XTERM_SGR) {
                                prints("\033[?1006h");      // enable SGR extended mouse mode.
                        }
                        prints("\033[?1004h");              // enable mouse focus events.
                }
        } else {
                if (mode == MOUSE_XTERM) {
                        prints("\033[?1000l");              // disable mouse presses.

                } else { // XTERM X11 & SGR
                        prints("\033[?1004l");              // disable mouse focus events.
                        if (mode == MOUSE_XTERM_SGR) {
                                prints("\033[?1006l");      // disable SGR extended mouse mode.
                        }
                        prints("\033[?1002l");              // disable cell-motion tracking tracking.
                }
        }
}


/*
 *  Keyboard mode configuration.
 */
static void
keyboard_mode(const enum KeyboardMode mode, int state)
{
        if (mode == KEYBOARD_STANDARD)
                return;

        if (state) {
                if (mode == KEYBOARD_CYGWIN) {
                        prints("\033[?2000h");              // enable cygwin-raw-mode.

                } else if (mode == KEYBOARD_MSTERMINAL) {
                        prints("\033[?9001h");              // enable win32-input-mode.

                } else if (mode == KEYBOARD_XTERM_META) {
                        prints("\033[?1034h");              // enable meta-8bit-mode.

                } else if (mode == KEYBOARD_XTERM_MOK2) {
                        //
                        //  \e[27;<modifier>;<char>~ or     xterm
                        //  \e[<char>;<modifier>u           formatOtherKeys=1 in xterm; which is the mintty default.
                        //
                        //  https://invisible-island.net/xterm/modified-keys-gb-altgr-intl.html#other_modifiable_keycodes
                        //
                        prints("\033[>4;2m");               // enable modifyOtherKeys

                } else if (mode == KEYBOARD_KITTY) {
                        //
                        //  \e[ number ; modifiers [u~]
                        //  \e[ 1; modifiers [ABCDEFHPQS]
                        //  0x0d - for the Enter key
                        //  0x7f or 0x08 - for Backspace
                        //  0x09 - for Tab
                        //
                        //  https://sw.kovidgoyal.net/kitty/keyboard-protocol/
                        //  http://www.leonerd.org.uk/hacks/fixterms/
                        //
                        prints("\033[>1u");                 // enable kitty-keyboard-protocol
                }
        } else {
                if (mode == KEYBOARD_CYGWIN) {
                        prints("\033[?2000l");              // disable cygwin-raw-mode.

                } else if (mode == KEYBOARD_MSTERMINAL) {
                        prints("\033[?9001l");              // disable win32-input-mode.

                } else if (mode == KEYBOARD_XTERM_META) {
                        prints("\033[?1034l");              // disable meta-8bit-mode.

                } else if (mode == KEYBOARD_XTERM_MOK2) {
                        prints("\033[>4;0m");               // disable modifyOtherKeys

                } else if (mode == KEYBOARD_KITTY) {
                        prints("\033[<u");                  // disable kitty-keyboard-protocol
                }
        }
}


/*
 *  Host version
 */

#include <sys/utsname.h>

static int
uversion_get(struct uversion *uv)
{
        struct utsname uts = {0};
        int fd;

        if (0 == uname(&uts)) {
                if (0 == memcmp(uts.sysname, "CYGWIN", 6)) {
                        if (3 == sscanf(uts.release, "%u.%u.%u", &uv->umajor, &uv->uminor, &uv->upatch)) {
                                return UVERSION_CYGWIN;
                        }

                } else if (0 == memcmp(uts.sysname, "Linux", 5)) {
                        if (3 == sscanf(uts.release, "%u.%u.%u", &uv->umajor, &uv->uminor, &uv->upatch)) {
                                return UVERSION_LINUX;
                        }
                }
                return 0;
        }
        return -1;
}


static int
uversion_cmp(const struct uversion *uv, unsigned umajor, unsigned uminor, unsigned upatch)
{
        if (uv->umajor > umajor)
                return 1;                                   // greater
        if (uv->umajor == umajor) {
                if (uv->uminor > uminor)
                        return 1;                           // greater
                if (uv->uminor == uminor) {
                        if (uv->upatch > upatch)
                                return 1;                   // greater
                        if (uv->upatch == upatch)
                                return 0;                   // equal
                }
        }
        return -1;
}

//end

