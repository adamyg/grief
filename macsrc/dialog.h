#ifndef MACSRC_DIALOG_H_INCLUDED
#define MACSRC_DIALOG_H_INCLUDED
/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: dialog.h,v 1.11 2014/10/27 23:28:20 ayoung Exp $
 * Common definitions, for older BRIEF style dialogs.
 *
 *
 */

/* Dialog manager modes */
#define DIALOG_MENU_MODE        0x01        /* Dialog status */

/* Dialog events */
#define DIALOG_INIT             0x1000      /* Dialog events */
#define DIALOG_TERM             0x1001
#define DIALOG_ESCAPE           0x1003
#define DIALOG_BACK             0x1004
#define DIALOG_CREATE_MENU      0x1200
#define DIALOG_MOVE_MENU        0x1201
#define DIALOG_PICK_MENU        0x1202
#define DIALOG_FUNC_KEY         0x2001
#define DIALOG_TYPEABLE         0x2002

/* Dialog key values */
#define DIALOG_KEY_F1           1
#define DIALOG_KEY_F2           2
#define DIALOG_KEY_F3           3
#define DIALOG_KEY_F4           4
#define DIALOG_KEY_F5           5
#define DIALOG_KEY_F6           6
#define DIALOG_KEY_F7           7
#define DIALOG_KEY_F8           8
#define DIALOG_KEY_F9           9
#define DIALOG_KEY_F10          10
#define DIALOG_KEY_SF1          101
#define DIALOG_KEY_SF2          102
#define DIALOG_KEY_SF3          103
#define DIALOG_KEY_SF4          104
#define DIALOG_KEY_SF5          105
#define DIALOG_KEY_SF6          106
#define DIALOG_KEY_SF7          107
#define DIALOG_KEY_SF8          108
#define DIALOG_KEY_SF9          109
#define DIALOG_KEY_SF10         110

/* Prototypes */
#if defined(__PROTOTYPES__)
extern int                      _dialog_menu(int lx, int by, int rx, int ty,
                                    ~ string, ~string, ~string, ~int, ~string notused, ~int, ~int, ~string);
extern void                     _dialog_esc(void);
extern void                     _dialog_back(void);
extern void                     _dialog_menu_search(int nextflag, int sizeflag);
#endif //__PROTOTYPES__

#endif //MACSRC_DIALOG_H_INCLUDED
