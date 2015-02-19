#ifndef TTY_TTY_H_INCLUDDED
#define TTY_TTY_H_INCLUDDED
/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: tty.h,v 1.5 2014/07/11 21:25:50 ayoung Exp $
 * Keyboard definitions for use in the tty-*.m macros
 *
 * $Id: tty.h,v 1.5 2014/07/11 21:25:50 ayoung Exp $
 *
 */

#include "../crisp.h"
#include "../alt.h"

#define CTRL_A_Z            1
#define F1_F12              F(1)
#define SHIFT_F1_F12        SF(1)
#define CTRL_F1_F12         CF(1)
#define CTRLSHIFT_F1_F12    CSF(1)
#define ALT_F1_F12          AF(1)
#define ALT_A_Z             ALT_A
#define ALT_0_9             ALT_0
#define KEYPAD_0_9          KEYPAD_0
#define CTRL_KEYPAD_0_9     CTRL_KEYPAD_0
#define SHIFT_KEYPAD_0_9    SHIFT_KEYPAD_0
#define ALT_KEYPAD_0_9      ALT_KEYPAD_0
#define DEL                 KEY_DEL
#define COPY                KEY_COPY
#define CUT                 KEY_CUT

#if defined(__PROTOTYPES__)
extern void                 ansi_arrows(void);
#endif

#endif /*TTY_TTY_XTERM_H_INCLUDED*/


