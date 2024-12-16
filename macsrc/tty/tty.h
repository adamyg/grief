#ifndef TTY_TTY_H_INCLUDDED
#define TTY_TTY_H_INCLUDDED
/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: tty.h,v 1.8 2024/11/02 18:11:51 cvsuser Exp $
 * Keyboard definitions for use in the tty-*.m macros
 *
 * $Id: tty.h,v 1.8 2024/11/02 18:11:51 cvsuser Exp $
 *
 */

#include "../grief.h"
#include "../alt.h"

#define CTRL_A_Z                1
#define F1_F12                  F(1)
#define SHIFT_F1_F12            SF(1)
#define CTRL_F1_F12             CF(1)
#define CTRLSHIFT_F1_F12        CSF(1)
#define ALT_F1_F12              AF(1)
#define ALTSHIFT_F1_F12         ASF(1)
#define ALTCTRL_F1_F12          ACF(1)
#define ALTCTRLSHIFT_F1_F12	ACSF(1)

#define ALT_A_Z                 ALT_A
#define ALT_0_9                 ALT_0
#define KEYPAD_0_9              KEYPAD_0
#define SHIFT_KEYPAD_0_9        SHIFT_KEYPAD_0
#define CTRL_KEYPAD_0_9         CTRL_KEYPAD_0
#define CTRLSHIFT_KEYPAD_0_9    CTRLSHIFT_KEYPAD_0
#define ALT_KEYPAD_0_9          ALT_KEYPAD_0
#define ALTSHIFT_KEYPAD_0_9     ALTSHIFT_KEYPAD_0
#define ALTCTRL_KEYPAD_0_9      ALTCTRL_KEYPAD_0
#define ALTCTRLSHIFT_KEYPAD_0_9 ALTCTRLSHIFT_KEYPAD_0
#define DEL                     KEY_DEL
#define COPY                    KEY_COPY
#define CUT                     KEY_CUT

#if defined(__PROTOTYPES__)
extern void                 ansi_arrows(void);
#endif

#endif /*TTY_TTY_XTERM_H_INCLUDED*/



