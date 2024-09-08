/*
 *  console key test -- window style definitions.
 */

typedef unsigned char  BOOL;
typedef unsigned char  BYTE;
typedef unsigned short WORD;
typedef unsigned long  DWORD;
#define LOWORD(__x) ((WORD)((__x) >>  0))
#define HIWORD(__x) ((WORD)((__x) >> 16))
typedef unsigned char  CHAR;
typedef unsigned short WCHAR;

#if !defined(TRUE)
#define TRUE 1
#define FALSE 0
#endif
#if !defined(_countof)
#define _countof(__x) (sizeof(__x)/sizeof(__x[0]))
#endif

        /////////////////////////////////////////////////////

typedef struct _KEY_EVENT_RECORD {
        BOOL bKeyDown;
        WORD wRepeatCount;
        WORD wVirtualKeyCode;
        WORD wVirtualScanCode;
        union {
                WCHAR UnicodeChar;
                CHAR AsciiChar;
        } uChar;
        DWORD dwControlKeyState;
#define CAPSLOCK_ON 0x0080
#define ENHANCED_KEY 0x0100
#define LEFT_ALT_PRESSED 0x0002
#define LEFT_CTRL_PRESSED 0x0008
#define NUMLOCK_ON 0x0020
#define RIGHT_ALT_PRESSED 0x0001
#define RIGHT_CTRL_PRESSED 0x0004
#define SCROLLLOCK_ON 0x0040
#define SHIFT_PRESSED 0x0010

} KEY_EVENT_RECORD;

        /////////////////////////////////////////////////////

typedef struct _MOUSE_EVENT_RECORD {
        DWORD dwEventFlags;
#define MOUSE_MOVED 0x01
#define DOUBLE_CLICK 0x02
#define MOUSE_WHEELED 0x04
#define MOUSE_HWHEELED 0x08

        DWORD dwButtonState;
#define FROM_LEFT_1ST_BUTTON_PRESSED 0x01
#define FROM_LEFT_2ND_BUTTON_PRESSED 0x02
#define RIGHTMOST_BUTTON_PRESSED 0x04

        DWORD dwControlKeyState;

        struct MouseCoord {
                WORD X, Y;
        } dwMousePosition;
} MOUSE_EVENT_RECORD;

        ////////////////////////////////////////////////////

typedef struct INPUT_RECORD {
        DWORD EventType;
#define VOID_EVENT 0x00 // extension
#define KEY_EVENT 0x0001
#define MOUSE_EVENT 0x0002
#define FOCUS_EVENT 0x0010

        union {
                KEY_EVENT_RECORD KeyEvent;
                MOUSE_EVENT_RECORD MouseEvent;
        } Event;

} INPUT_RECORD;

/*end*/
