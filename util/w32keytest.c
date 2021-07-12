/*
 *  win32 console key-test
 */

#define _WIN32_WINNT 0x0601
#include <windows.h>
#include <wincon.h>
#include <stdio.h>

#define ALT_PRESSED             (LEFT_ALT_PRESSED|RIGHT_ALT_PRESSED)
#define CTRL_PRESSED            (LEFT_CTRL_PRESSED|RIGHT_CTRL_PRESSED)
#define APP_PRESSED             0x0200      /* APPS enabled, extension */

#define CTRLSTATUSMASK          (LEFT_ALT_PRESSED|LEFT_CTRL_PRESSED|RIGHT_ALT_PRESSED|RIGHT_CTRL_PRESSED|SHIFT_PRESSED|APP_PRESSED)

static const struct w32key {
    WORD                wVirtualKeyCode;    /* windows virtual key code */

#define VKMOD_ANY               (0x10000000)
#define VKMOD_META              (0x20000000)
#define VKMOD_CTRL              (0x30000000)
#define VKMOD_SHIFT             (0x40000000)
#define VKMOD_NONSHIFT          (0x50000000)
#define VKMOD_ENHANCED          (0x60000000)
#define VKMOD_NOTENHANCED       (0x70000000)

    DWORD               modifiers;          /* modifiers */
    const wchar_t *     desc;               /* description */

} w32Keys[] = {
    { VK_BACK,          0,                  L"Back"             },
    { VK_TAB,           0,                  L"Tab"              },
    { VK_BACK,          VKMOD_SHIFT,        L"Back"             },
    { VK_TAB,           VKMOD_SHIFT,        L"Tab"              },
    { VK_BACK,          VKMOD_CTRL,         L"Back"             },
    { VK_TAB,           VKMOD_CTRL,         L"Tab"              },
    { VK_BACK,          VKMOD_META,         L"Back"             },
    { VK_TAB,           VKMOD_META,         L"Tab"              },
    { VK_ESCAPE,        VKMOD_ANY,          L"Esc"              },
    { VK_RETURN,        VKMOD_ANY,          L"Return"           },
    { VK_RETURN,        VKMOD_ENHANCED,     L"Keypad-Return"    },
    { VK_PAUSE,         VKMOD_ANY,          L"Keypad-Pause"     },
    { VK_PRIOR,         VKMOD_ANY,          L"PgUp"             },
    { VK_NEXT,          VKMOD_ANY,          L"PgDn"             },
    { VK_END,           VKMOD_ANY,          L"End"              },
    { VK_HOME,          VKMOD_ANY,          L"Home"             },
    { VK_LEFT,          VKMOD_ANY,          L"Left"             },
    { VK_UP,            VKMOD_ANY,          L"Up"               },
    { VK_RIGHT,         VKMOD_ANY,          L"Right"            },
    { VK_DOWN,          VKMOD_ANY,          L"Down"             },
    { VK_INSERT,        VKMOD_ANY,          L"Ins"              },
    { VK_DELETE,        VKMOD_ANY,          L"Delete"           },

    { VK_HELP,          VKMOD_ANY,          L"Help"             },
#if defined(VK_ICO_HELP)
    { VK_ICO_HELP,      VKMOD_ANY,          L"Help"             },
#endif

    { VK_PRIOR,         VKMOD_NOTENHANCED,  L"Keypad-PgUp"      },
    { VK_NEXT,          VKMOD_NOTENHANCED,  L"Keypad-PgDn"      },
    { VK_END,           VKMOD_NOTENHANCED,  L"Keypad-End"       },
    { VK_HOME,          VKMOD_NOTENHANCED,  L"Keypad-Home"      },
    { VK_LEFT,          VKMOD_NOTENHANCED,  L"Keypad-Left"      },
    { VK_CLEAR,         VKMOD_NOTENHANCED,  L"Keypad-5"         },
    { VK_UP,            VKMOD_NOTENHANCED,  L"Keypad-Up"        },
    { VK_RIGHT,         VKMOD_NOTENHANCED,  L"Keypad-Right"     },
    { VK_DOWN,          VKMOD_NOTENHANCED,  L"Keypad-Down"      },
    { VK_INSERT,        VKMOD_NOTENHANCED,  L"Keypad-Ins"       },
    { VK_DELETE,        VKMOD_NOTENHANCED,  L"Keypad-Delete"    },
    { VK_HELP,          VKMOD_NOTENHANCED,  L"Keypad-Help"      },
    { VK_SUBTRACT,      VKMOD_ANY,          L"Keypad-Minus"     },
    { VK_MULTIPLY,      VKMOD_ANY,          L"Keypad-Star"      },
    { VK_ADD,           VKMOD_ANY,          L"Keypad-Plus"      },
    { VK_DIVIDE,        VKMOD_ANY,          L"Keypad-Divide"    },

    /* VK_0 thru VK_9 are the same as ASCII '0' thru '9' (0x30 - 0x39) */
    /* VK_A - VK_Z are the same as ASCII 'A'- 'Z' (0x41 - 0x5A) */

    { 0x30,             VKMOD_CTRL,         L"0"                },
    { 0x31,             VKMOD_CTRL,         L"1"                },
    { 0x32,             VKMOD_CTRL,         L"2"                },
    { 0x33,             VKMOD_CTRL,         L"3"                },
    { 0x34,             VKMOD_CTRL,         L"4"                },
    { 0x35,             VKMOD_CTRL,         L"5"                },
    { 0x36,             VKMOD_CTRL,         L"6"                },
    { 0x37,             VKMOD_CTRL,         L"7"                },
    { 0x38,             VKMOD_CTRL,         L"8"                },
    { 0x39,             VKMOD_CTRL,         L"9"                },

    { VK_F1,            VKMOD_ANY,          L"F1"               },
    { VK_F2,            VKMOD_ANY,          L"F2"               },
    { VK_F3,            VKMOD_ANY,          L"F3"               },
    { VK_F4,            VKMOD_ANY,          L"F4"               },
    { VK_F5,            VKMOD_ANY,          L"F5"               },
    { VK_F6,            VKMOD_ANY,          L"F6"               },
    { VK_F7,            VKMOD_ANY,          L"F7"               },
    { VK_F8,            VKMOD_ANY,          L"F8"               },
    { VK_F9,            VKMOD_ANY,          L"F9"               },
    { VK_F10,           VKMOD_ANY,          L"F10"              },
    { VK_F11,           VKMOD_ANY,          L"F11"              },
    { VK_F12,           VKMOD_ANY,          L"F12"              },
    { VK_F13,           VKMOD_ANY,          L"F13"              },
    { VK_F14,           VKMOD_ANY,          L"F14"              },
    { VK_F15,           VKMOD_ANY,          L"F15"              },
    { VK_F16,           VKMOD_ANY,          L"F16"              },
    { VK_F17,           VKMOD_ANY,          L"F17"              },
    { VK_F18,           VKMOD_ANY,          L"F18"              },
    { VK_F19,           VKMOD_ANY,          L"F19"              },
    { VK_F20,           VKMOD_ANY,          L"F20"              },

    { VK_NUMLOCK,       VKMOD_ANY,          L"Keypad-Numlock"   },
    { VK_SCROLL,        VKMOD_ANY,          L"Keypad-Scroll"    },

    { VK_OEM_PLUS,      VKMOD_NONSHIFT,     L"+"                },
    { VK_OEM_COMMA,     VKMOD_NONSHIFT,     L","                },
    { VK_OEM_MINUS,     VKMOD_NONSHIFT,     L"-"                },
    { VK_OEM_PERIOD,    VKMOD_NONSHIFT,     L"."                },
    { VK_OEM_3,         VKMOD_NONSHIFT,     L"~"                },
    };

#define ISHEX(_uc) \
    ((_uc >= '0' && _uc <= '9') || (_uc >= 'a' && _uc <= 'f') || (_uc >= 'A' && _uc <= 'F') ? 1 : 0)

static void Usage(const struct argparms *args);
static void Process(HANDLE in);
static int AltPlusEvent(const KEY_EVENT_RECORD *ke, int offset);
static int AltPlusEnabled(void);
static const wchar_t *key_description(const KEY_EVENT_RECORD *ker);
static const wchar_t *control_state(DWORD dwControlKeyState);
static const wchar_t *virtual_description(WORD wVirtualKeyCode);

static int xf_altcode = -1;                     /* 1=enable,0=disable,-1=auto */
static int xf_mouse = 1;


int
main(void)
{
    HANDLE in = GetStdHandle(STD_INPUT_HANDLE);
    DWORD mode = 0;

    GetConsoleMode(in, &mode);
    if (xf_mouse) {                             /* mouse enabled */
        if (! SetConsoleMode(in, ENABLE_EXTENDED_FLAGS|\
                    ENABLE_WINDOW_INPUT|ENABLE_MOUSE_INPUT|ENABLE_PROCESSED_INPUT)) {
                // Note: Stating ENABLE_EXTENDED_FLAGS disables ENABLE_INSERT_MODE and/or ENABLE_QUICK_EDIT_MODE.
                //  required for correct mouse operation; restored within sys_shutdown().
            SetConsoleMode(in, ENABLE_WINDOW_INPUT|ENABLE_MOUSE_INPUT|ENABLE_PROCESSED_INPUT);
                // No extended support/XP.
        }
    } else {
        SetConsoleMode(in, ENABLE_WINDOW_INPUT|ENABLE_PROCESSED_INPUT);
    }
    Process(in);
    SetConsoleMode(in, mode);

    return 0;
}


static void
Process(HANDLE in)
{
    int alt_state = 0, esc_state = 0, event_count = 0;
    DWORD apps_control = 0;
    int offset = 0;

#define COLUMN1     100

    printf("\nConsole input test, ESC+ESC to quit.");
    if (xf_altcode < 0) {
        if (0 == xf_altcode ||
                (xf_altcode < 0 && ! AltPlusEnabled())) {
            wprintf(L"\nAlt-Plus-KeyCode not enabled");
            xf_altcode = 0;
        } else {
            wprintf(L"\nAlt-Plus-KeyCode enabled");
            xf_altcode = 1;
        }
    }

    printf("\n\n#     UC     Chr Dir  Rpt VK                SC   State\n");
    fflush(stdout);
        //  nnnn  U+uuuu c   1111 111 111111111111 1111 1111 xxxxxxxxxxxxxxx

    for (;;) {
        INPUT_RECORD ir[1] = {0};
        DWORD cEventsRead = 0;

        if (! ReadConsoleInputW(in, ir, 1, &cEventsRead)) {
            puts("ReadConsoleInput failed!");
            return;
        }

        for (DWORD i = 0; i < cEventsRead; ++i, ++event_count) {
            if (ir[i].EventType & KEY_EVENT) {
                const KEY_EVENT_RECORD *ker = &ir[i].Event.KeyEvent;

                // Special filters

                alt_state = AltPlusEvent(ker, offset);
                if (ker->bKeyDown) {
                    if (VK_APPS == ker->wVirtualKeyCode) {
                        apps_control = APP_PRESSED;
                    }
                } else {
                    if (VK_APPS == ker->wVirtualKeyCode) {
                        apps_control = 0;
                    }
                }

                // Key details

                wprintf(L"\n");
                offset = wprintf(L"%4d: U+%04x %c   %s %03u %04x %-12s %04x %08x %s = %d",
                    event_count, (WORD) ker->uChar.UnicodeChar,
                        (ker->uChar.UnicodeChar > 32 ? ker->uChar.UnicodeChar : ' '),
                    (ker->bKeyDown ? L"down" : L" up "), ker->wRepeatCount,
                    ker->wVirtualKeyCode, virtual_description(ker->wVirtualKeyCode), ker->wVirtualScanCode,
                    ker->dwControlKeyState, control_state(ker->dwControlKeyState|apps_control),
                    alt_state);

                if (ker->bKeyDown) {
                    if (alt_state <= 0) {       // "Alt +" inactive
                        KEY_EVENT_RECORD t_ker = *ker;
                        const wchar_t *kd;

                        t_ker.dwControlKeyState |= apps_control;
                        if (NULL != (kd = key_description(&t_ker))) {
                            wprintf(L"%*s<%s>", COLUMN1 - offset, L"", kd);
                            offset = COLUMN1;
                        }
                    }

                    if (0 == (CTRLSTATUSMASK & ker->dwControlKeyState) &&
                                0x1b == ker->uChar.UnicodeChar) {
                        if (++esc_state >= 2) { // Escape
                            puts("\nbye...");
                            return;
                        }
                    } else {
                        esc_state = 0;
                    }
                }
            }
        }
    }
}


//  Alt+<key-code> event handler
//
//      Alt+KeyCode works and behaves well when character only input is required, by simply
//      reporting any down or up key events which populate the 'UnicodeChar' value. Whereas
//      when extended keystroke handling is required, for example arrow and numpad keys,
//      additional effort is needed.
//
//      Alt+Keycodes are only reported within the 'UnicodeChar' value of up event on a "ALT" key
//      post the valid entry of one-or-more hex characters. During KeyCode entry the API unfortunately
//      does not publiciy indicate this state plus continues to return the associated virtual keys,
//      including the leading 'keypad-plus' and any associated key-code elements, wherefore we need
//      to filter.  Furthermore, if during the key-code entry an invalid non-hex key combination is
//      given, the key-code is invalidated and UnicodeChar=0 is returned on the ALT release.
//
//      Notes:
//       o Requires the registry REG_SZ value "EnableHexNumpad" under
//         "HKEY_Current_User/Control Panel/Input Method" to be "1".
//
//       o Hex-value overflow goes unreported, limiting input to a 16-bit result.
//
static int
AltPlusEvent(const KEY_EVENT_RECORD *ke, int offset)
{
#define ISXDIGIT(_uc) \
            ((_uc >= '0' && _uc <= '9') || (_uc >= 'a' && _uc <= 'f') || (_uc >= 'A' && _uc <= 'F') ? 1 : 0)

    static int alt_code = 0;
    static DWORD alt_control = 0;
    wchar_t completion[64];

    if (! xf_altcode) return -1;                // enabled?

    completion[0] = 0;
    if (ke->bKeyDown) {                         // down event
        const unsigned controlKeyState = (CTRLSTATUSMASK & ke->dwControlKeyState);

        if (VK_ADD == ke->wVirtualKeyCode &&
                (LEFT_ALT_PRESSED == controlKeyState || RIGHT_ALT_PRESSED == controlKeyState)) {
            // "Alt + ..." event
            alt_control = controlKeyState;
            if (alt_code == 0) {
                alt_code = 1;
            }
            return 1;                           // consume

        } else if (alt_code) {
            if (alt_control != controlKeyState ||
                    (ke->uChar.UnicodeChar && 0 == ISXDIGIT(ke->uChar.UnicodeChar))) {
                // new control status or non-hex, emit "Alt-Plus" and reset state
                swprintf(completion, _countof(completion), L"Alt-Plus");
                alt_code = 0;

            } else {
                ++alt_code;                     // associated key count
                return alt_code;                // consume
            }
        }

    } else if (alt_code) {                      // up event
        if (VK_MENU == ke->wVirtualKeyCode &&
                (0 == (ke->dwControlKeyState & ALT_PRESSED))) {
            // Alt completion
            const int oalt_code = alt_code;

            alt_code = 0;
            if (1 == oalt_code && 0 == ke->uChar.UnicodeChar) {
                // "Alt-Plus" only, emit
                swprintf(completion, _countof(completion), L"Alt-Plus");

            } else if (ke->uChar.UnicodeChar) {
                // "Alt-Plus keycode", return keycode.
                swprintf(completion, _countof(completion), L"Alt-Plus-#%d", ke->uChar.UnicodeChar);
            }
        }
    }

    if (completion[0]) {                        // completion
        wprintf(L"%*s<%s>", COLUMN1 - offset, L"", completion);
        return 0;
    }

    return -1;                                  // unhandled
}


static int
AltPlusEnabled(void)
{
    HKEY hKey = 0;
    int enabled = 0;

    if (RegOpenKeyExA(HKEY_CURRENT_USER,
            "Control Panel\\Input Method", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        char szEnableHexNumpad[100] = { 0 };
        DWORD dwSize = _countof(szEnableHexNumpad);
        if (RegQueryValueExA(hKey, "EnableHexNumpad", NULL, NULL, (LPBYTE) szEnableHexNumpad, &dwSize) == ERROR_SUCCESS) {
            if (szEnableHexNumpad[0] == '1') {
                enabled = 1;
            }
        }
        RegCloseKey(hKey);
    }
    return enabled;
}


static const wchar_t *
key_description(const KEY_EVENT_RECORD *ker)
{
    static wchar_t t_buffer[200];
    wchar_t *cursor = t_buffer, *end = cursor + _countof(t_buffer);

    const DWORD dwControlKeyState = ker->dwControlKeyState;
    const WORD wVirtualKeyCode = ker->wVirtualKeyCode;
    const struct w32key *key = w32Keys + _countof(w32Keys);

    // Virtual key mapping
    while (--key >= w32Keys) {
        if (key->wVirtualKeyCode == wVirtualKeyCode) {
            const DWORD modifiers = key->modifiers;
            if (VKMOD_ANY == modifiers) {
                break;
            } else if (VKMOD_SHIFT == modifiers) {
                if (0 != (dwControlKeyState & SHIFT_PRESSED)) {
                    break;
                }
            } else if (VKMOD_NONSHIFT == modifiers) {
                if (0 == (dwControlKeyState & SHIFT_PRESSED)) {
                    break;
                }
            } else if (VKMOD_ENHANCED == modifiers) {
                if (0 != (dwControlKeyState & ENHANCED_KEY)) {
                    break;
                }
            } else if (VKMOD_NOTENHANCED == modifiers) {
                if (0 == (dwControlKeyState & ENHANCED_KEY)) {
                    break;
                }
            } else if (0 == modifiers || (dwControlKeyState & modifiers)) {
                break;
            }
        }
    }
    if (key < w32Keys) {
        if (0 == ker->uChar.UnicodeChar) {
            return NULL;
        }
        key = NULL;
    }

    // Control states
    if (dwControlKeyState & ALT_PRESSED)
        cursor += swprintf(cursor, end - cursor, L"Alt-");

    if (dwControlKeyState & APP_PRESSED)
        cursor += swprintf(cursor, end - cursor, L"App-");

    if (dwControlKeyState & CTRL_PRESSED)
        cursor += swprintf(cursor, end - cursor, L"Ctrl-");

    if (dwControlKeyState & SHIFT_PRESSED)
        cursor += swprintf(cursor, end - cursor, L"Shift-");

    // Key code
    if (key) {
        cursor += swprintf(cursor, end - cursor, L"%s", key->desc);

    } else if (ker->uChar.UnicodeChar) {
        const WORD uc = ker->uChar.UnicodeChar;
        const wchar_t *desc = NULL;

        switch (uc) {
        case '\b': desc = L"Backspace"; break;
        case '\r': desc = L"Enter"; break;
        case '\t': desc = L"Tab"; break;
        case 0x1b: desc = L"Esc"; break;
        case ' ':
            if (dwControlKeyState & CTRLSTATUSMASK) {
                desc = L"Space";
            }
            break;
        }

        if (desc) {
            cursor += swprintf(cursor, end - cursor, desc);
        } else {
            if (uc > ' ' && uc < 0xff) {        // ASCII
                if ((dwControlKeyState & CTRLSTATUSMASK) && uc >= 'a' && uc <= 'z') {
                    *cursor++ = uc - 'a' + 'A'; // upper-case meta controls.
                } else {
                    *cursor++ = uc;
                }
                *cursor++ = 0;

            } else if (uc < ' ' && (dwControlKeyState & CTRL_PRESSED)) {
                *cursor++ = ('A' - 1) + uc;     // controls
                *cursor++ = 0;

            } else {                            // Unicode
                cursor += swprintf(cursor, end - cursor, L"#%u", uc);
            }
        }
    }

    return (cursor > t_buffer ? t_buffer :  NULL);
}


static const wchar_t *
control_state(DWORD dwControlKeyState)
{
    static wchar_t t_buffer[200];
    wchar_t *cursor = t_buffer;

    t_buffer[0] = 0;
    if (dwControlKeyState & CAPSLOCK_ON)        wcscpy(cursor, L"CapLk,"),    cursor += wcslen(cursor);
    if (dwControlKeyState & ENHANCED_KEY)       wcscpy(cursor, L"Enhanced,"), cursor += wcslen(cursor);
    if (dwControlKeyState & LEFT_ALT_PRESSED)   wcscpy(cursor, L"LeftAlt,"),  cursor += wcslen(cursor);
    if (dwControlKeyState & LEFT_CTRL_PRESSED)  wcscpy(cursor, L"LeftCtl,"),  cursor += wcslen(cursor);
    if (dwControlKeyState & NUMLOCK_ON)         wcscpy(cursor, L"NumLck,"),   cursor += wcslen(cursor);
    if (dwControlKeyState & RIGHT_ALT_PRESSED)  wcscpy(cursor, L"RightAlt,"), cursor += wcslen(cursor);
    if (dwControlKeyState & RIGHT_CTRL_PRESSED) wcscpy(cursor, L"RightCtl,"), cursor += wcslen(cursor);
    if (dwControlKeyState & SCROLLLOCK_ON)      wcscpy(cursor, L"ScrLk,"),    cursor += wcslen(cursor);
    if (dwControlKeyState & SHIFT_PRESSED)      wcscpy(cursor, L"Shift,"),    cursor += wcslen(cursor);
    if (dwControlKeyState & APP_PRESSED)        wcscpy(cursor, L"App,"),      cursor += wcslen(cursor);

    if (cursor > t_buffer) cursor[-1] = 0;
    return t_buffer;
}


static const wchar_t *
virtual_description(WORD wVirtualKeyCode)
{
    switch(wVirtualKeyCode) {
    case VK_LBUTTON             :  /*0x01*/ return L"LBUTTON";
    case VK_RBUTTON             :  /*0x02*/ return L"RBUTTON";
    case VK_CANCEL              :  /*0x03*/ return L"CANCEL";
    case VK_MBUTTON             :  /*0x04*/ return L"MBUTTON";

    case VK_XBUTTON1            :  /*0x05*/ return L"XBUTTON1";
    case VK_XBUTTON2            :  /*0x06*/ return L"XBUTTON2";

    case VK_BACK                :  /*0x08*/ return L"BACK";
    case VK_TAB                 :  /*0x09*/ return L"TAB";

    case VK_CLEAR               :  /*0x0C*/ return L"CLEAR";
    case VK_RETURN              :  /*0x0D*/ return L"RETURN";

    case VK_SHIFT               :  /*0x10*/ return L"SHIFT";
    case VK_CONTROL             :  /*0x11*/ return L"CONTROL";
    case VK_MENU                :  /*0x12*/ return L"MENU";
    case VK_PAUSE               :  /*0x13*/ return L"PAUSE";
    case VK_CAPITAL             :  /*0x14*/ return L"CAPITAL";

    case VK_KANA                :  /*0x15*/ return L"KANA";
//  case VK_HANGEUL             :  /*0x15*/ return L"HANGEUL";
//  case VK_HANGUL              :  /*0x15*/ return L"HANGUL";
    case VK_JUNJA               :  /*0x17*/ return L"JUNJA";
    case VK_FINAL               :  /*0x18*/ return L"FINAL";
    case VK_HANJA               :  /*0x19*/ return L"HANJA";
//  case VK_KANJI               :  /*0x19*/ return L"KANJI";

    case VK_ESCAPE              :  /*0x1B*/ return L"ESCAPE";

    case VK_CONVERT             :  /*0x1C*/ return L"CONVERT";
    case VK_NONCONVERT          :  /*0x1D*/ return L"NONCONVERT";
    case VK_ACCEPT              :  /*0x1E*/ return L"ACCEPT";
    case VK_MODECHANGE          :  /*0x1F*/ return L"MODECHANGE";

    case VK_SPACE               :  /*0x20*/ return L"SPACE";
    case VK_PRIOR               :  /*0x21*/ return L"PRIOR";
    case VK_NEXT                :  /*0x22*/ return L"NEXT";
    case VK_END                 :  /*0x23*/ return L"END";
    case VK_HOME                :  /*0x24*/ return L"HOME";
    case VK_LEFT                :  /*0x25*/ return L"LEFT";
    case VK_UP                  :  /*0x26*/ return L"UP";
    case VK_RIGHT               :  /*0x27*/ return L"RIGHT";
    case VK_DOWN                :  /*0x28*/ return L"DOWN";
    case VK_SELECT              :  /*0x29*/ return L"SELECT";
    case VK_PRINT               :  /*0x2A*/ return L"PRINT";
    case VK_EXECUTE             :  /*0x2B*/ return L"EXECUTE";
    case VK_SNAPSHOT            :  /*0x2C*/ return L"SNAPSHOT";
    case VK_INSERT              :  /*0x2D*/ return L"INSERT";
    case VK_DELETE              :  /*0x2E*/ return L"DELETE";
    case VK_HELP                :  /*0x2F*/ return L"HELP";

    case VK_LWIN                :  /*0x5B*/ return L"LWIN";
    case VK_RWIN                :  /*0x5C*/ return L"RWIN";
    case VK_APPS                :  /*0x5D*/ return L"APPS";

    case VK_SLEEP               :  /*0x5F*/ return L"SLEEP";

    case VK_NUMPAD0             :  /*0x60*/ return L"NUMPAD0";
    case VK_NUMPAD1             :  /*0x61*/ return L"NUMPAD1";
    case VK_NUMPAD2             :  /*0x62*/ return L"NUMPAD2";
    case VK_NUMPAD3             :  /*0x63*/ return L"NUMPAD3";
    case VK_NUMPAD4             :  /*0x64*/ return L"NUMPAD4";
    case VK_NUMPAD5             :  /*0x65*/ return L"NUMPAD5";
    case VK_NUMPAD6             :  /*0x66*/ return L"NUMPAD6";
    case VK_NUMPAD7             :  /*0x67*/ return L"NUMPAD7";
    case VK_NUMPAD8             :  /*0x68*/ return L"NUMPAD8";
    case VK_NUMPAD9             :  /*0x69*/ return L"NUMPAD9";
    case VK_MULTIPLY            :  /*0x6A*/ return L"MULTIPLY";
    case VK_ADD                 :  /*0x6B*/ return L"ADD";
    case VK_SEPARATOR           :  /*0x6C*/ return L"SEPARATOR";
    case VK_SUBTRACT            :  /*0x6D*/ return L"SUBTRACT";
    case VK_DECIMAL             :  /*0x6E*/ return L"DECIMAL";
    case VK_DIVIDE              :  /*0x6F*/ return L"DIVIDE";
    case VK_F1                  :  /*0x70*/ return L"F1";
    case VK_F2                  :  /*0x71*/ return L"F2";
    case VK_F3                  :  /*0x72*/ return L"F3";
    case VK_F4                  :  /*0x73*/ return L"F4";
    case VK_F5                  :  /*0x74*/ return L"F5";
    case VK_F6                  :  /*0x75*/ return L"F6";
    case VK_F7                  :  /*0x76*/ return L"F7";
    case VK_F8                  :  /*0x77*/ return L"F8";
    case VK_F9                  :  /*0x78*/ return L"F9";
    case VK_F10                 :  /*0x79*/ return L"F10";
    case VK_F11                 :  /*0x7A*/ return L"F11";
    case VK_F12                 :  /*0x7B*/ return L"F12";
    case VK_F13                 :  /*0x7C*/ return L"F13";
    case VK_F14                 :  /*0x7D*/ return L"F14";
    case VK_F15                 :  /*0x7E*/ return L"F15";
    case VK_F16                 :  /*0x7F*/ return L"F16";
    case VK_F17                 :  /*0x80*/ return L"F17";
    case VK_F18                 :  /*0x81*/ return L"F18";
    case VK_F19                 :  /*0x82*/ return L"F19";
    case VK_F20                 :  /*0x83*/ return L"F20";
    case VK_F21                 :  /*0x84*/ return L"F21";
    case VK_F22                 :  /*0x85*/ return L"F22";
    case VK_F23                 :  /*0x86*/ return L"F23";
    case VK_F24                 :  /*0x87*/ return L"F24";

    case VK_NUMLOCK             :  /*0x90*/ return L"NUMLOCK";
    case VK_SCROLL              :  /*0x91*/ return L"SCROLL";

    case VK_OEM_NEC_EQUAL       :  /*0x92*/ return L"OEM_NEC_EQUAL";

//  case VK_OEM_FJ_JISHO        :  /*0x92*/ return L"OEM_FJ_JISHO";
    case VK_OEM_FJ_MASSHOU      :  /*0x93*/ return L"OEM_FJ_MASSHOU";
    case VK_OEM_FJ_TOUROKU      :  /*0x94*/ return L"OEM_FJ_TOUROKU";
    case VK_OEM_FJ_LOYA         :  /*0x95*/ return L"OEM_FJ_LOYA";
    case VK_OEM_FJ_ROYA         :  /*0x96*/ return L"OEM_FJ_ROYA";

    case VK_LSHIFT              :  /*0xA0*/ return L"LSHIFT";
    case VK_RSHIFT              :  /*0xA1*/ return L"RSHIFT";
    case VK_LCONTROL            :  /*0xA2*/ return L"LCONTROL";
    case VK_RCONTROL            :  /*0xA3*/ return L"RCONTROL";
    case VK_LMENU               :  /*0xA4*/ return L"LMENU";
    case VK_RMENU               :  /*0xA5*/ return L"RMENU";

    case VK_BROWSER_BACK        :  /*0xA6*/ return L"BROWSER_BACK";
    case VK_BROWSER_FORWARD     :  /*0xA7*/ return L"BROWSER_FORWARD";
    case VK_BROWSER_REFRESH     :  /*0xA8*/ return L"BROWSER_REFRESH";
    case VK_BROWSER_STOP        :  /*0xA9*/ return L"BROWSER_STOP";
    case VK_BROWSER_SEARCH      :  /*0xAA*/ return L"BROWSER_SEARCH";
    case VK_BROWSER_FAVORITES   :  /*0xAB*/ return L"BROWSER_FAVORITES";
    case VK_BROWSER_HOME        :  /*0xAC*/ return L"BROWSER_HOME";

    case VK_VOLUME_MUTE         :  /*0xAD*/ return L"VOLUME_MUTE";
    case VK_VOLUME_DOWN         :  /*0xAE*/ return L"VOLUME_DOWN";
    case VK_VOLUME_UP           :  /*0xAF*/ return L"VOLUME_UP";
    case VK_MEDIA_NEXT_TRACK    :  /*0xB0*/ return L"MEDIA_NEXT_TRACK";
    case VK_MEDIA_PREV_TRACK    :  /*0xB1*/ return L"MEDIA_PREV_TRACK";
    case VK_MEDIA_STOP          :  /*0xB2*/ return L"MEDIA_STOP";
    case VK_MEDIA_PLAY_PAUSE    :  /*0xB3*/ return L"MEDIA_PLAY_PAUSE";
    case VK_LAUNCH_MAIL         :  /*0xB4*/ return L"LAUNCH_MAIL";
    case VK_LAUNCH_MEDIA_SELECT :  /*0xB5*/ return L"LAUNCH_MEDIA_SELECT";
    case VK_LAUNCH_APP1         :  /*0xB6*/ return L"LAUNCH_APP1";
    case VK_LAUNCH_APP2         :  /*0xB7*/ return L"LAUNCH_APP2";

    case VK_OEM_1               :  /*0xBA*/ return L"OEM_1";
    case VK_OEM_PLUS            :  /*0xBB*/ return L"OEM_PLUS";
    case VK_OEM_COMMA           :  /*0xBC*/ return L"OEM_COMMA";
    case VK_OEM_MINUS           :  /*0xBD*/ return L"OEM_MINUS";
    case VK_OEM_PERIOD          :  /*0xBE*/ return L"OEM_PERIOD";
    case VK_OEM_2               :  /*0xBF*/ return L"OEM_2";
    case VK_OEM_3               :  /*0xC0*/ return L"OEM_3";

    case VK_OEM_4               :  /*0xDB*/ return L"OEM_4";
    case VK_OEM_5               :  /*0xDC*/ return L"OEM_5";
    case VK_OEM_6               :  /*0xDD*/ return L"OEM_6";
    case VK_OEM_7               :  /*0xDE*/ return L"OEM_7";
    case VK_OEM_8               :  /*0xDF*/ return L"OEM_8";

    case VK_OEM_AX              :  /*0xE1*/ return L"OEM_AX";
    case VK_OEM_102             :  /*0xE2*/ return L"OEM_102";
    case VK_ICO_HELP            :  /*0xE3*/ return L"ICO_HELP";
    case VK_ICO_00              :  /*0xE4*/ return L"ICO_00";

    case VK_PROCESSKEY          :  /*0xE5*/ return L"PROCESSKEY";

    case VK_ICO_CLEAR           :  /*0xE6*/ return L"ICO_CLEAR";

    case VK_PACKET              :  /*0xE7*/ return L"PACKET";

    case VK_OEM_RESET           :  /*0xE9*/ return L"OEM_RESET";
    case VK_OEM_JUMP            :  /*0xEA*/ return L"OEM_JUMP";
    case VK_OEM_PA1             :  /*0xEB*/ return L"OEM_PA1";
    case VK_OEM_PA2             :  /*0xEC*/ return L"OEM_PA2";
    case VK_OEM_PA3             :  /*0xED*/ return L"OEM_PA3";
    case VK_OEM_WSCTRL          :  /*0xEE*/ return L"OEM_WSCTRL";
    case VK_OEM_CUSEL           :  /*0xEF*/ return L"OEM_CUSEL";
    case VK_OEM_ATTN            :  /*0xF0*/ return L"OEM_ATTN";
    case VK_OEM_FINISH          :  /*0xF1*/ return L"OEM_FINISH";
    case VK_OEM_COPY            :  /*0xF2*/ return L"OEM_COPY";
    case VK_OEM_AUTO            :  /*0xF3*/ return L"OEM_AUTO";
    case VK_OEM_ENLW            :  /*0xF4*/ return L"OEM_ENLW";
    case VK_OEM_BACKTAB         :  /*0xF5*/ return L"OEM_BACKTAB";

    case VK_ATTN                :  /*0xF6*/ return L"ATTN";
    case VK_CRSEL               :  /*0xF7*/ return L"CRSEL";
    case VK_EXSEL               :  /*0xF8*/ return L"EXSEL";
    case VK_EREOF               :  /*0xF9*/ return L"EREOF";
    case VK_PLAY                :  /*0xFA*/ return L"PLAY";
    case VK_ZOOM                :  /*0xFB*/ return L"ZOOM";
    case VK_NONAME              :  /*0xFC*/ return L"NONAME";
    case VK_PA1                 :  /*0xFD*/ return L"PA1";
    case VK_OEM_CLEAR           :  /*0xFE*/ return L"OEM_CLEAR";

    default:
        return L"";
    }
}


#if (0)
#pragma comment(lib, "Imm32.lib")
static int
ImmTest(void)
{
    HWND hWnd = GetConsoleWindow();
    HWND hIME = ImmGetDefaultIMEWnd(hWnd);
    LRESULT status = SendMessage(hIME, WM_IME_CONTROL, IMC_GETOPENSTATUS, 0);
    HIMC imc = ImmCreateContext(),
        oldimc = ImmAssociateContext(hWnd, imc),
        retimc = ImmGetContext(hWnd);
    ImmSetOpenStatus(imc, TRUE);
    BOOL isopen = ImmGetOpenStatus(imc);
    Sleep(2 * 2000);
    ImmAssociateContext(hWnd, oldimc);
    ImmReleaseContext(hWnd, imc);
    Sleep(100);
}
#endif

