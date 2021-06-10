#include <edidentifier.h>
__CIDENT_RCSID(gr_keywd_c,"$Id: keywd.c,v 1.96 2021/06/10 11:56:06 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: keywd.c,v 1.96 2021/06/10 11:56:06 cvsuser Exp $
 * Keyword table.
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
#include <edcm.h>

#include "keywd.h"

#if defined(GRUNCH)             /*grunch macro compiler*/
#ifndef __NOFUNCTIONS__
#define __NOFUNCTIONS__
#endif
#endif

#if defined(__NOFUNCTIONS__)
 #define MACRO(__name)          NULL
 #define VALUE(__value)         0

#else
 #define MACRO(__name)          ((void (*)(int)) __name)
 #define VALUE(__value)         __value

 #include "anchor.h"
 #include "basic.h"
 #include "bookmark.h"
 #include "buffer.h"
 #include "cmap.h"
 #include "debug.h"
 #include "dialog.h"
 #include "dict.h"
 #include "diff.h"
 #include "display.h"
 #include "eval.h"
 #include "file.h"
 #include "getkey.h"
 #include "keyboard.h"
 #include "kill.h"
 #include "lisp.h"
 #include "mac1.h"
 #include "macros.h"
 #include "main.h"
 #include "maths.h"
 #include "mouse.h"
 #include "playback.h"
 #include "procspawn.h"
 #include "regexp.h"
 #include "region.h"
 #include "register.h"
 #include "syntax.h"
 #include "tags.h"
 #include "tty.h"
 #include "undo.h"

 #include "m_backup.h"
 #include "m_brief.h"
 #include "m_buf.h"
 #include "m_caller.h"
 #include "m_color.h"
 #include "m_debug.h"
 #include "m_display.h"
 #include "m_echo.h"
 #include "m_errno.h"
 #include "m_feature.h"
 #include "m_file.h"
 #include "m_fileio.h"
 #include "m_float.h"
 #include "m_ftp.h"
 #include "m_getopt.h"
 #include "m_hilite.h"
 #include "m_ini.h"
 #include "m_line.h"
 #include "m_macro.h"
 #include "m_mchar.h"
 #include "m_msg.h"
 #include "m_pty.h"
 #include "m_random.h"
 #include "m_region.h"
 #include "m_regress.h"
 #include "m_ruler.h"
 #include "m_scan.h"
 #include "m_screen.h"
 #include "m_search.h"
 #include "m_signal.h"
 #include "m_sort.h"
 #include "m_spell.h"
 #include "m_string.h"
 #include "m_symbol.h"
 #include "m_sysinfo.h"
 #include "m_system.h"
 #include "m_terminal.h"
 #include "m_time.h"
 #include "m_tokenize.h"
 #include "m_userprofile.h"
 #include "m_vfs.h"
 #include "m_window.h"
#endif  /*__NOFUNCTIONS__*/

const int cm_version = CM_VERSION;

#define VERSION_201
#define VERSION_202
#define VERSION_203
#define VERSION_204
#define VERSION_205         /* 01/04/2020, register(), __lexicalblock(), isclose() and cast_xxx() */
#define VERSION_206         /* 06/21, UTF8 */

//  #define VERSION_207     /* array's, staged/experimental */
//  #define VERSION_208     /* not implemented/alpha */

/*
 *  Keyword table, assumed to be in alphabetic order.
 */
BUILTIN builtin[] = {
    {"!", MACRO(do_lnot), ARG_UNDEF, 0, 0,                  /* arith */
    1,  {ARG_NUM | ARG_STRING}},                /* 3.2.0, allow string scalars */

    {"!=", MACRO(do_com_op), ARG_UNDEF, 0, MOP_NE,          /* arith */
    2,  {ARG_ANY, ARG_ANY}},

    {"%", MACRO(do_com_op), ARG_UNDEF, 0, MOP_MODULO,       /* arith */
    2,  {ARG_INT, ARG_INT}},

    {"%=", MACRO(do_com_equ), ARG_UNDEF, 0, MOP_MODULO,     /* arith */
    2,  {ARG_LVAL | ARG_INT, ARG_INT}},

    {"&", MACRO(do_com_op), ARG_UNDEF, 0, MOP_BAND,         /* arith */
    2,  {ARG_INT, ARG_INT}},

    {"&&", MACRO(do_andand), ARG_UNDEF, 0, 0,               /* arith */
    2,  {ARG_INT, ARG_COND}},

    {"&=", MACRO(do_com_equ), ARG_UNDEF, 0, MOP_BAND,       /* arith */
    2,  {ARG_LVAL | ARG_INT, ARG_INT}},

    {"*", MACRO(do_com_op), ARG_UNDEF, 0, MOP_MULTIPLY,     /* arith */
    2,  {ARG_NUM | ARG_STRING, ARG_NUM | ARG_STRING}},

    {"*=", MACRO(do_com_equ), ARG_UNDEF, 0, MOP_MULTIPLY,   /* arith */
    2,  {ARG_LVAL | ARG_NUM, ARG_NUM}},

    {"+", MACRO(do_com_op), ARG_UNDEF, 0, MOP_PLUS,         /* arith */
    2,  {ARG_ANY, ARG_ANY}},

    {"++", MACRO(do_plusplus), ARG_UNDEF, 0, 0,             /* arith */
    1,  {ARG_LVAL | ARG_NUM}},

    {"+=", MACRO(do_com_equ), ARG_UNDEF, 0, MOP_PLUS,       /* arith */
    2,  {ARG_LVAL | ARG_ANY, ARG_ANY}},

    {"-", MACRO(do_com_op), ARG_UNDEF, 0, MOP_MINUS,        /* arith */
    2,  {ARG_NUM, ARG_NUM}},

    {"--", MACRO(do_minusminus), ARG_UNDEF, 0, 0,           /* arith */
    1,  {ARG_LVAL | ARG_NUM}},

    {"-=", MACRO(do_com_equ), ARG_UNDEF, 0, MOP_MINUS,      /* arith */
    2,  {ARG_LVAL | ARG_NUM, ARG_NUM}},

    {"/", MACRO(do_com_op), ARG_UNDEF, 0, MOP_DIVIDE,       /* arith */
    2,  {ARG_NUM, ARG_NUM}},

    {"/=", MACRO(do_com_equ), ARG_UNDEF, 0, MOP_DIVIDE,     /* arith */
    2,  {ARG_LVAL | ARG_NUM, ARG_NUM}},

    {"<", MACRO(do_com_op), ARG_UNDEF, 0, MOP_LT,           /* arith */
    2,  {ARG_NUM | ARG_STRING, ARG_NUM | ARG_STRING}},

    {"<<", MACRO(do_com_op), ARG_UNDEF, 0, MOP_LSHIFT,      /* arith */
    2,  {ARG_INT, ARG_INT}},

    {"<<=", MACRO(do_com_equ), ARG_UNDEF, 0, MOP_LSHIFT,    /* arith */
    2,  {ARG_LVAL | ARG_INT, ARG_INT}},

    {"<=", MACRO(do_com_op), ARG_UNDEF, 0, MOP_LE,          /* arith */
    2,  {ARG_NUM | ARG_STRING, ARG_NUM | ARG_STRING}},

    {"<=>", MACRO(do_com_op), ARG_UNDEF, 0, MOP_CMP,        /* arith */
    2,  {ARG_NUM | ARG_STRING, ARG_NUM | ARG_STRING}},

    /*
     *  perl compatibility (TODO)
     *
     *      eq          String equality (== is numeric equality).
     *      ne          String inequality (!= is numeric inequality).
     *      lt          String less than.
     *      gt          String greater than.
     *      le          String less than or equal.
     *      ge          String greater than or equal.
     *      cmp         String comparison, returning -1, 0, or 1.
     *      <=>         Numeric comparison, returning -1, 0, or 1 (DONE).
     *
     *  Note: Add --perl option to compiler
     *
     *      and/or      #pragma perl=yes
     *      and/or      #pragma crisp=yes
     *      and/or      #pragma grief=yes
     */
    {"=", MACRO(do_com_equ), ARG_UNDEF, 0, MOP_NOOP,        /* arith */
    2,  {ARG_LVAL | ARG_ANY, ARG_OPT | ARG_ANY}},

    {"==", MACRO(do_com_op), ARG_UNDEF, 0, MOP_EQ,          /* arith */
    2,  {ARG_NUM | ARG_STRING, ARG_NUM | ARG_STRING}},

//  {"===", MACRO(do_com_same), ARG_UNDEF, 0, 0,            /* arith */
//  2,  {ARG_NUM | ARG_STRING, ARG_NUM | ARG_STRING}},

    {">", MACRO(do_com_op), ARG_UNDEF, 0, MOP_GT,           /* arith */
    2,  {ARG_NUM | ARG_STRING, ARG_NUM | ARG_STRING}},

    {">=", MACRO(do_com_op), ARG_UNDEF, 0, MOP_GE,          /* arith */
    2,  {ARG_NUM | ARG_STRING, ARG_NUM | ARG_STRING}},

    {">>", MACRO(do_com_op), ARG_UNDEF, 0, MOP_RSHIFT,      /* arith */
    2,  {ARG_INT, ARG_INT}},

    {">>=", MACRO(do_com_equ), ARG_UNDEF, 0, MOP_RSHIFT,    /* arith */
    2,  {ARG_LVAL | ARG_INT, ARG_INT}},

    {"^", MACRO(do_com_op), ARG_UNDEF, 0, MOP_BXOR,         /* arith */
    2,  {ARG_INT, ARG_INT}},

    {"^=", MACRO(do_com_equ), ARG_UNDEF, 0, MOP_BXOR,       /* arith */
    2,  {ARG_LVAL | ARG_INT, ARG_INT}},

    {"__breaksw", MACRO(do_breaksw), ARG_VOID, 0, 0,        /* macro */
    0,  {0}},

#if defined(VERSION_205)
    {"__lexicalblock", MACRO(do_lexicalblock), ARG_VOID, 0, 0, /* var */
    1,  {ARG_COND}},
#endif

    {"__regress_op", MACRO(do_regress_op), ARG_INT, 0, 0,   /* misc */
    2,  {ARG_INT, ARG_OPT | ARG_ANY}},

    {"__regress_replacement", MACRO(do_regress_replacement), ARG_INT, 0, 0, /* misc */
    9,  {ARG_INT,
         ARG_INT,
         ARG_STRING,
         ARG_LIST,
         ARG_ANY,
         ARG_OPT  | ARG_INT,
         ARG_OPT  | ARG_STRING,
         ARG_OPT  | ARG_LIST,
         ARG_OPT  | ARG_ANY}},

    {"_bad_key", MACRO(do_nothing), ARG_STRING, 0, 0,       /* misc */
    0,  {0}},

//  {"_prompt_begin", MACRO(do_nothing), ARG_UNDEF, 0, 0,   /* misc */
//  1,  {ARG_OPT | ARG_STRING}},

//  {"_prompt_end", MACRO(do_nothing), ARG_VOID, 0, 0,      /* misc */
//  0,  {0}},

    {"abort", MACRO(do_abort), ARG_VOID, 0, 0,              /* debug */
    0,  {0}},

    {"above", MACRO(do_com_op), ARG_INT, 0, MOP_ABOVE,      /* arith */
    2,  {ARG_NUM | ARG_STRING, ARG_NUM | ARG_STRING}},

    {"above_eq", MACRO(do_com_op), ARG_INT, 0, MOP_ABOVE_EQ, /* arith */
    2,  {ARG_NUM | ARG_STRING, ARG_NUM | ARG_STRING}},

    {"abs", MACRO(do_abs), ARG_FLOAT, 0, 0,                 /* float, arith */
    1,  {ARG_NUM}},

    {"access", MACRO(do_access), ARG_INT, 0, 0,             /* file */
    2,  {ARG_STRING, ARG_OPT | ARG_INT}},

    {"acos", MACRO(do_acos), ARG_FLOAT, 0, 0,               /* float, arith */
    1,  {ARG_FLOAT}},

    {"arg_list", MACRO(do_arg_list), ARG_LIST, 0, 0,        /* macro, var */
    3,  {ARG_OPT | ARG_INT, ARG_OPT | ARG_INT, ARG_OPT | ARG_INT}},

    {"asin", MACRO(do_asin), ARG_FLOAT, 0, 0,               /* float, arith */
    1,  {ARG_FLOAT}},

    {"assign_to_key", MACRO(do_assign_to_key), ARG_INT, 0, 0, /* kbd */
    2,  {ARG_OPT | ARG_STRING, ARG_OPT | ARG_STRING}},

    {"atan", MACRO(do_atan), ARG_FLOAT, 0, 0,               /* float, arith */
    1,  {ARG_FLOAT}},

    {"atan2", MACRO(do_atan2), ARG_FLOAT, 0, 0,             /* float, arith */
    2,  {ARG_FLOAT, ARG_FLOAT}},

    {"atoi", MACRO(do_atoi), ARG_INT, 0, 0,                 /* string */
    2,  {ARG_STRING, ARG_OPT | ARG_INT}},

    {"attach_buffer", MACRO(do_attach_buffer), ARG_VOID, 0, 0, /* buffer */
    1,  {ARG_INT}},

    {"attach_syntax", MACRO(do_attach_syntax), ARG_INT, 0, 0, /* syntax */
    1,  {ARG_INT | ARG_STRING}},

    {"autoload", MACRO(do_autoload), ARG_VOID, B_NOVALUE, 0, /* macro */
    2,  {ARG_STRING, ARG_REST}},

    {"backspace", MACRO(do_backspace), ARG_VOID, 0, 0,      /* movement */
    1,  {ARG_OPT | ARG_INT}},

    {"basename", MACRO(do_filename), ARG_STRING, 0, 0,      /* file */
    2,  {ARG_STRING, ARG_OPT | ARG_STRING}},

    {"beep", MACRO(do_beep), ARG_VOID, 0, 0,                /* misc */
    2,  {ARG_OPT | ARG_INT, ARG_OPT | ARG_INT}},

    {"beginning_of_line", MACRO(do_beginning_of_line), ARG_INT, 0, 0, /* movement */
    0,  {0}},

    {"below", MACRO(do_com_op), ARG_INT, 0, MOP_BELOW,      /* arith */
    2,  {ARG_NUM | ARG_STRING, ARG_NUM | ARG_STRING}},

    {"below_eq", MACRO(do_com_op), ARG_INT, 0, MOP_BELOW_EQ, /* arith */
    2,  {ARG_NUM | ARG_STRING, ARG_NUM | ARG_STRING}},

#if defined(VERSION_208)
    {"bless", MACRO(do_bless), ARG_INT, 0, 0,               /* macro */
    2,  {ARG_INT, ARG_OPT | ARG_STRING}},
#endif

    {"bookmark_list", MACRO(do_bookmark_list), ARG_LIST, 0, 0, /* movement */
    0,  {0}},

#if defined(DO_INTERNAL) & (0)
    {"bool", MACRO(do_declare), ARG_VOID, B_NOVALUE, F_INT, /* var */
    1,  {ARG_REST}},
#endif

    {"borders", MACRO(do_borders), ARG_INT, 0, 0,           /* screen */
    1,  {ARG_OPT | ARG_INT | ARG_STRING}},

    {"break", MACRO(do_break), ARG_VOID, 0, 0,              /* macro */
    0,  {0}},

    {"call_registered_macro", MACRO(do_call_registered_macro), ARG_INT, 0, 0, /* macro */
    1,  {ARG_INT}},

    {"car", MACRO(do_car), ARG_ANY, 0, 0,                   /* list */
    1,  {ARG_LIST}},

#if defined(VERSION_205)
    {"cast_float", MACRO(do_cast), ARG_FLOAT, 0, F_FLOAT,   /* float, arith */
    1,  {ARG_INT}},

    {"cast_int", MACRO(do_cast), ARG_INT, 0, F_INT,         /*  arith */
    1,  {ARG_FLOAT}},
#endif

    {"cd", MACRO(do_cd), ARG_INT, 0, 0,                     /* env */
    1,  {ARG_OPT | ARG_STRING}},

    {"cdr", MACRO(do_cdr), ARG_ANY, 0, 0,                   /* list */
    1,  {ARG_LIST}},

    {"ceil", MACRO(do_ceil), ARG_FLOAT, 0, 0,               /* float, arith */
    1,  {ARG_FLOAT}},

    {"cftime", MACRO(do_cftime), ARG_STRING, 0, 0,          /* misc, string */
    2,  {ARG_OPT | ARG_STRING, ARG_OPT | ARG_INT}},

    {"change_window", MACRO(do_change_window), ARG_INT, 0, 0, /* window */
    2,  {ARG_OPT | ARG_INT, ARG_OPT | ARG_STRING}},

    {"change_window_pos", MACRO(do_change_window_pos), ARG_VOID, 0, 0, /* window */
    5,  {ARG_OPT | ARG_INT, ARG_OPT | ARG_INT, ARG_OPT | ARG_INT, ARG_OPT | ARG_INT, ARG_OPT | ARG_INT}},

    {"characterat", MACRO(do_characterat), ARG_INT, 0, 0,   /* string */
    2,  {ARG_STRING, ARG_INT}},

    {"chdir", MACRO(do_chdir), ARG_INT, 0, 0,               /* env, file */
    1, {ARG_STRING}},

    {"chmod", MACRO(do_chmod), ARG_INT, 0, 0,               /* file */
    2,  {ARG_STRING, ARG_INT}},

#if defined(VERSION_204)
    {"chown", MACRO(do_chown), ARG_INT, 0, 0,               /* file */
    2,  {ARG_STRING, ARG_INT | ARG_STRING}},
#endif

#if defined(VERSION_204)
    {"close_window", MACRO(do_close_window), ARG_INT, 0, 0, /* window */
    1,  {ARG_OPT | ARG_INT}},
#endif

    {"color", MACRO(do_color), ARG_INT, 0, 0,              /* screen */
    -1, {ARG_OPT | ARG_INT | ARG_STRING}},

    {"color_index", MACRO(do_color_index), ARG_INT, 0, 0,  /* misc */
    1,  {ARG_OPT | ARG_INT}},

    {"command_list", MACRO(do_command_list), ARG_LIST, 0, 0, /* list */
    2,  {ARG_OPT | ARG_INT, ARG_OPT | ARG_STRING}},

    {"compare", MACRO(do_com_op), ARG_UNDEF, 0, MOP_CMP,    /* arith */
    2,  {ARG_NUM | ARG_STRING, ARG_NUM | ARG_STRING}},

    {"compare_files", MACRO(do_compare_files), ARG_INT, 0, 0, /* file */
    2,  {ARG_STRING, ARG_STRING}},

    {"compress", MACRO(do_compress), ARG_STRING, 0, 0,      /* string */
    4,  {ARG_STRING, ARG_OPT | ARG_INT, ARG_OPT | ARG_STRING, ARG_OPT | ARG_INT}},

    {"connect", MACRO(do_connect), ARG_INT, 0, 0,           /* proc */
    3,  {ARG_OPT | ARG_INT, ARG_OPT | ARG_STRING, ARG_OPT | ARG_STRING}},

    {"const", MACRO(do_const), ARG_VOID, 0, 0,              /* var */
    1,  {ARG_REST}},

    {"continue", MACRO(do_continue), ARG_VOID, 0, 0,        /* macro */
    0,  {0}},

    {"copy", MACRO(do_copy), ARG_INT, 0, 0,                 /* scrap */
    2,  {ARG_OPT | ARG_INT, ARG_OPT | ARG_INT}},

    {"copy_keyboard", MACRO(do_copy_keyboard), ARG_INT, 0, 0, /* kbd */
    2,  {ARG_INT, ARG_REST}},

    {"copy_screen", MACRO(do_copy_screen), ARG_INT, 0, 0,   /* screen */
    0,  {0}},

    {"cos", MACRO(do_cos), ARG_FLOAT, 0, 0,                 /* float, arith */
    1,  {ARG_FLOAT}},

    {"cosh", MACRO(do_cosh), ARG_FLOAT, 0, 0,               /* float, arith */
    1,  {ARG_FLOAT}},

    {"create_buffer", MACRO(do_create_buffer), ARG_INT, 0, FALSE, /* buffer */
    5,  {ARG_STRING, ARG_OPT | ARG_STRING, ARG_OPT | ARG_INT,
            ARG_OPT | ARG_INT, ARG_OPT | ARG_STRING}},

    {"create_char_map", MACRO(do_create_char_map), ARG_INT, 0, 0, /* buffer, window, env */
    5,  {ARG_OPT | ARG_INT, ARG_OPT | ARG_INT, ARG_OPT | ARG_INT | ARG_STRING | ARG_LIST, ARG_OPT | ARG_LIST, ARG_OPT | ARG_STRING}},

    {"create_dictionary", MACRO(do_create_dictionary), ARG_INT, 0, 0, /* macro */
    3,  {ARG_OPT | ARG_STRING, ARG_OPT | ARG_INT, ARG_OPT | ARG_INT}},

    {"create_edge", MACRO(do_create_edge), ARG_INT, 0, 0,   /* window */
    1,  {ARG_OPT | ARG_INT}},

    {"create_menu_window", MACRO(do_create_menu_window), ARG_INT, 0, 0, /* window */
    1,  {ARG_OPT | ARG_INT}},

    {"create_nested_buffer", MACRO(do_create_buffer), ARG_INT, 0, TRUE, /* buffer */
    4,  {ARG_STRING,
         ARG_OPT | ARG_STRING,
         ARG_OPT | ARG_INT,
         ARG_OPT | ARG_INT}},

    {"create_syntax", MACRO(do_create_syntax), ARG_INT, 0, 0, /* syntax */
    1,  {ARG_STRING}},

    {"create_tiled_window", MACRO(do_create_tiled_window), ARG_INT, 0, 0, /* window */
    5,  {ARG_INT, ARG_INT, ARG_INT, ARG_INT, ARG_OPT | ARG_INT}},

    {"create_window", MACRO(do_create_window), ARG_INT, 0, 0, /* window */
    5,  {ARG_INT, ARG_INT, ARG_INT, ARG_INT, ARG_OPT | ARG_STRING}},

    {"cursor", MACRO(do_cursor), ARG_INT, 0, 0,             /* screen */
    1,  {ARG_OPT | ARG_INT}},

    {"cut", MACRO(do_cut), ARG_INT, 0, 0,                   /* scrap */
    2,  {ARG_OPT | ARG_INT, ARG_OPT | ARG_INT}},

    {"cvt_to_object", MACRO(do_cvt_to_object), ARG_ANY, 0, 0, /* string, arith */
    2,  {ARG_STRING, ARG_OPT | ARG_LVAL | ARG_INT}},

    {"date", MACRO(do_date), ARG_VOID, 0, 0,                /* misc */
    5,  {ARG_OPT | ARG_LVAL | ARG_INT,
         ARG_OPT | ARG_LVAL | ARG_INT,
         ARG_OPT | ARG_LVAL | ARG_INT,
         ARG_OPT | ARG_LVAL | ARG_STRING,
         ARG_OPT | ARG_LVAL | ARG_STRING}},

    {"debug", MACRO(do_debug), ARG_INT, 0, 0,               /* debug */
    3,  {ARG_OPT | ARG_INT | ARG_STRING, ARG_OPT | ARG_INT, ARG_OPT | ARG_INT}},

    {"debug_support", MACRO(do_debug_support), ARG_UNDEF, 0, 0, /* debug */
    3,  {ARG_OPT | ARG_INT, ARG_OPT | ARG_INT | ARG_STRING, ARG_OPT | ARG_STRING}},

    {"declare", MACRO(do_declare), ARG_VOID, B_NOVALUE, F_POLY, /* var */
    1,  {ARG_REST}},

    {"define_keywords", MACRO(do_define_keywords), ARG_VOID, 0, 0, /* syntax */
    5,  {ARG_INT | ARG_STRING, ARG_STRING | ARG_LIST, ARG_OPT | ARG_INT, ARG_OPT | ARG_INT, ARG_OPT | ARG_INT | ARG_STRING}},

    {"delete_block", MACRO(do_delete_block), ARG_INT, 0, 0, /* scrap */
    0,  {0}},

    {"delete_bookmark", MACRO(do_delete_bookmark), ARG_VOID, 0, 0, /* movement */
    1,  {ARG_INT}},

    {"delete_buffer", MACRO(do_delete_buffer), ARG_VOID, 0, 0, /* buffer */
    1,  {ARG_INT}},

    {"delete_char", MACRO(do_delete_char), ARG_VOID, 0, 0,  /* buffer */
    1,  {ARG_OPT | ARG_INT}},

    {"delete_dictionary", MACRO(do_delete_dictionary/*obj_id*/), ARG_INT, 0, 0, /* macro */
    1,  {ARG_INT}},

    {"delete_edge", MACRO(do_delete_edge), ARG_INT, 0, 0,   /* window */
    1,  {ARG_OPT | ARG_INT}},

    {"delete_line", MACRO(do_delete_line), ARG_VOID, 0, 0,  /* buffer */
    0,  {0}},

    {"delete_macro", MACRO(do_unimp), ARG_VOID, 0, 0,       /* macro */
    1,  {ARG_OPT | ARG_STRING}},

    {"delete_nth", MACRO(do_delete_nth), ARG_VOID, 0, 0,    /* list */
    3,  {ARG_LVAL | ARG_LIST, ARG_INT, ARG_OPT | ARG_INT}},

    {"delete_to_eol", MACRO(do_delete_to_eol), ARG_VOID, 0, 0, /* buffer */
    0,  {0}},

    {"delete_window", MACRO(do_delete_window), ARG_VOID, 0, 0, /* window */
    1,  {ARG_OPT | ARG_INT}},

    {"detach_syntax", MACRO(do_detach_syntax), ARG_INT, 0, 0, /* syntax */
    0,  {0}},

    {"dialog_create", MACRO(do_dialog_create), ARG_INT, 0, 0, /* dialog */
    1,  {ARG_LIST}},

    {"dialog_delete", MACRO(do_dialog_delete), ARG_INT, 0, 0, /* dialog */
    1,  {ARG_INT}},

    {"dialog_exit", MACRO(do_dialog_exit), ARG_INT, 0, 0,   /* dialog */
    2,  {ARG_OPT | ARG_INT, ARG_OPT | ARG_INT}},

    {"dialog_run", MACRO(do_dialog_run), ARG_INT, 0, 0,     /* dialog */
    1,  {ARG_INT}},

#if defined(VERSION_204)
    {"dict_clear", MACRO(do_dict_clear), ARG_INT, 0, 0,     /* macro */
    1,  {ARG_INT}},
#endif

    {"dict_delete", MACRO(do_dict_delete), ARG_INT, 0, 0,   /* macro */
    2,  {ARG_INT, ARG_STRING}},

    {"dict_each", MACRO(do_dict_each), ARG_INT, 0, VALUE(DICT_EACH_PAIR), /* macro */
    3,  {ARG_INT, ARG_OPT | ARG_LVAL | ARG_STRING, ARG_OPT | ARG_LVAL | ARG_ANY}},

    {"dict_exists", MACRO(do_dict_exists), ARG_INT, 0, 0,   /* macro */
    2,  {ARG_INT, ARG_OPT | ARG_STRING}},

    {"dict_keys", MACRO(do_dict_each), ARG_INT, 0, VALUE(DICT_EACH_KEY), /* macro */
    2,  {ARG_INT, ARG_OPT | ARG_LVAL | ARG_STRING}},

    {"dict_list", MACRO(do_dict_list), ARG_LIST, 0, 0,      /* macro */
    2,  {ARG_INT, ARG_OPT | ARG_INT}},

    {"dict_name", MACRO(do_dict_name), ARG_STRING, 0, 0,    /* macro */
    1,  {ARG_INT}},

    {"dict_values", MACRO(do_dict_each), ARG_INT, 0, VALUE(DICT_EACH_VALUE), /* macro */
    2,  {ARG_INT, ARG_OPT | ARG_LVAL | ARG_ANY}},

    {"diff_buffers", MACRO(do_diff_buffers), ARG_UNDEF, 0, 0, /* buffer */
    3,  {ARG_INT, ARG_INT, ARG_INT}},

    {"diff_lines", MACRO(do_diff_lines), ARG_UNDEF, 0, 0,   /* buffer */
    5,  {ARG_INT, ARG_INT, ARG_INT|ARG_OPT, ARG_INT, ARG_INT|ARG_OPT}},

    {"diff_strings", MACRO(do_diff_strings), ARG_INT, 0, 0, /* string */
    3,  {ARG_INT, ARG_STRING, ARG_STRING}},

    {"dirname", MACRO(do_filename), ARG_STRING, 0, 1,       /* file */
    1,  {ARG_STRING}},

    {"disconnect", MACRO(do_disconnect), ARG_INT, 0, 0,     /* proc */
    0,  {0}},

    {"display_mode", MACRO(do_display_mode), ARG_INT, 0, 0, /* screen */
    7,  {ARG_OPT | ARG_INT | ARG_STRING,
         ARG_OPT | ARG_INT | ARG_STRING,
         ARG_OPT | ARG_INT,
         ARG_OPT | ARG_INT,
         ARG_OPT | ARG_INT,
         ARG_OPT | ARG_INT,
         ARG_OPT | ARG_INT}},

    {"display_windows", MACRO(do_display_windows), ARG_INT, 0, 0, /* screen */
    1,  {ARG_OPT | ARG_INT}},

    {"distance_to_indent", MACRO(do_distance_to_indent), ARG_INT, 0, 0, /* buffer, window */
    1,  {ARG_OPT | ARG_INT}},

    {"distance_to_tab", MACRO(do_distance_to_tab), ARG_INT, 0, 0, /* buffer, window */
    1,  {ARG_OPT | ARG_INT}},

    {"do", MACRO(do_do), ARG_VOID, 0, 0,                    /* macro */
    2,  {ARG_OPT | ARG_COND, ARG_OPT | ARG_REST}},

#if defined(DO_INTERNAL) && (0)
    {"double", MACRO(do_declare), ARG_VOID, B_NOVALUE, F_FLOAT, /* var */
    1,  {ARG_REST}},
#endif

    {"down", MACRO(do_down), ARG_INT, 0, 0,                 /* movement */
    1,  {ARG_OPT | ARG_INT}},

    {"dprintf", MACRO(do_dprintf), ARG_INT, 0, 0,           /* debug */
    -2, {ARG_STRING, ARG_OPT | ARG_ANY}},

    {"drop_anchor", MACRO(do_drop_anchor), ARG_INT, 0, 0,   /* scrap */
    1,  {ARG_OPT | ARG_INT}},

    {"drop_bookmark", MACRO(do_drop_bookmark), ARG_INT, 0, 0, /* movement */
    5,  {ARG_OPT | ARG_INT,
         ARG_OPT | ARG_STRING,
         ARG_OPT | ARG_INT,
         ARG_OPT | ARG_INT,
         ARG_OPT | ARG_INT}},

    {"echo_line", MACRO(do_echo_line), ARG_INT, 0, 0,       /* screen */
    1,  {ARG_OPT | ARG_INT}},

    {"edit_file", MACRO(do_edit_file), ARG_INT, 0, 0,       /* file */
    1,  {ARG_OPT | ARG_REST}},

    {"edit_file2", MACRO(do_edit_file), ARG_INT, 0, 2,      /* file */
    1,  {ARG_OPT | ARG_REST}},

    {"ega", MACRO(do_ega), ARG_INT, 0, 0,                   /* screen */
    1,  {ARG_OPT | ARG_INT}},

    {"end_anchor", MACRO(do_end_anchor), ARG_INT, 0, 0,     /* scrap */
    2,  {ARG_OPT | ARG_INT, ARG_OPT | ARG_INT}},

    {"end_of_buffer", MACRO(do_end_of_buffer), ARG_INT, 0, 0, /* movement */
    0,  {0}},

    {"end_of_line", MACRO(do_end_of_line), ARG_INT, 0, 0,   /* movement */
    0,  {0}},

    {"end_of_window", MACRO(do_end_of_window), ARG_INT, 0, 0, /* movement */
    0,  {0}},

    {"error", MACRO(do_error), ARG_INT, 0, 0,               /* debug */
    -2, {ARG_STRING, ARG_OPT | ARG_ANY}},

    {"eval", 0, ARG_ANY, 0, 0,                              /* arith, macro, string */
    1,  {ARG_STRING}},

    {"execute_macro", MACRO(do_execute_macro), ARG_INT, 0, 0, /* macro */
    2,  {ARG_OPT | ARG_STRING, ARG_OPT | ARG_REST}},

    {"exist", MACRO(do_exist), ARG_INT, 0, 0,               /* file */
    2,  {ARG_STRING, ARG_OPT | ARG_INT}},

    {"exit", MACRO(do_exit), ARG_VOID, 0, 0,                /* macro */
    1,  {ARG_OPT | ARG_STRING}},

    {"exp", MACRO(do_exp), ARG_FLOAT, 0, 0,                 /* float, arith */
    1,  {ARG_FLOAT}},

    {"expandpath", MACRO(do_expandpath), ARG_STRING, 0, 0,  /* file */
    2,  {ARG_STRING, ARG_OPT | ARG_INT}},

    {"extern", MACRO(do_extern), ARG_VOID, B_NOVALUE, 0,    /* var */
    1,  {ARG_REST}},

    {"fabs", MACRO(do_fabs), ARG_FLOAT, 0, 0,               /* float, arith */
    1,  {ARG_FLOAT}},

    {"fclose", MACRO(do_fclose), ARG_INT, 0, 0,             /* file */
    1,  {ARG_INT}},

#if defined(VERSION_201)
    {"feof", MACRO(do_feof), ARG_INT, 0, 0,                 /* file */
    1,  {ARG_INT}},
#endif

#if defined(VERSION_201)
    {"ferror", MACRO(do_ferror), ARG_INT, 0, 0,             /* file */
    2,  {ARG_INT, ARG_OPT | ARG_INT}},
#endif

#if defined(VERSION_201)
    {"fflush", MACRO(do_fflush), ARG_INT, 0, 0,             /* file */
    2,  {ARG_INT, ARG_OPT | ARG_INT}},
#endif

#if defined(VERSION_204)
    {"file_canon", MACRO(do_file_canon), ARG_STRING, 0, 0,  /* file */
    1,  {ARG_STRING}},
#endif

    {"file_glob", MACRO(do_file_glob), ARG_LIST, 0, 0,      /* file, list */
    1,  {ARG_STRING}},

    {"file_match", MACRO(do_file_match), ARG_INT, 0, 0,     /* file */
    2,  {ARG_STRING | ARG_LIST, ARG_STRING}},

    {"file_pattern", MACRO(do_file_pattern), ARG_INT, 0, 0, /* file */
    1,  {ARG_STRING}},

    {"filename_match", MACRO(do_filename_match), ARG_INT, 0, 0, /* file */
    2,  {ARG_STRING, ARG_STRING | ARG_LIST}},

    {"filename_realpath", MACRO(do_filename_realpath), ARG_STRING, 0, 0, /* file */
    1,  {ARG_STRING}},

    {"find_file", MACRO(do_find_file), ARG_INT, 0, 1,       /* file */
    5,  {ARG_OPT | ARG_LVAL | ARG_STRING,
         ARG_OPT | ARG_LVAL | ARG_INT,
         ARG_OPT | ARG_LVAL | ARG_INT,
         ARG_OPT | ARG_LVAL | ARG_INT,
         ARG_OPT | ARG_LVAL | ARG_INT}},

#if defined(VERSION_201)
    {"find_file2", MACRO(do_find_file), ARG_INT, 0, 2,      /* file */
    12, {ARG_OPT | ARG_LVAL | ARG_STRING,
         ARG_OPT | ARG_LVAL | ARG_INT,
         ARG_OPT | ARG_LVAL | ARG_INT,
         ARG_OPT | ARG_LVAL | ARG_INT,
         ARG_OPT | ARG_LVAL | ARG_INT,
         ARG_OPT | ARG_LVAL | ARG_INT,
         ARG_OPT | ARG_LVAL | ARG_INT,
         ARG_OPT | ARG_LVAL | ARG_STRING,
         ARG_OPT | ARG_LVAL | ARG_INT,
         ARG_OPT | ARG_LVAL | ARG_STRING,
         ARG_OPT | ARG_LVAL | ARG_INT,
         ARG_OPT | ARG_LVAL | ARG_INT}},
#endif

    {"find_line_flags", MACRO(do_find_line_flags), ARG_INT, 0, 0, /* buffer */
    6,  {ARG_OPT | ARG_INT,
         ARG_OPT | ARG_INT,
         ARG_INT,
         ARG_INT,
         ARG_OPT | ARG_INT,
         ARG_OPT | ARG_INT}},

    {"find_macro", MACRO(do_find_macro), ARG_STRING, 0, 0,  /* macro, file */
    1,  {ARG_STRING}},

    {"find_marker", MACRO(do_find_marker), ARG_INT, 0, 0,   /* buffer */
    1,  {ARG_OPT | ARG_INT}},

    {"fioctl", MACRO(do_fioctl), ARG_INT, 0, 0,             /* file */
    1,  {ARG_INT}},

    {"first_time", MACRO(do_first_time), ARG_INT, 0, 0,     /* macro */
    0,  {0}},

    {"firstof", MACRO(do_firstof), ARG_INT, 0, 0,           /* string */
    3,  {ARG_STRING, ARG_STRING, ARG_OPT | ARG_LVAL | ARG_INT}},

    {"float", MACRO(do_declare), ARG_VOID, B_NOVALUE, F_FLOAT, /* var, float */
    1,  {ARG_REST}},

    {"flock", MACRO(do_flock), ARG_INT, 0, 0,               /* file */
    1,  {ARG_INT}},

    {"floor", MACRO(do_floor), ARG_FLOAT, 0, 0,             /* float, arith */
    1,  {ARG_FLOAT}},

#if defined(VERSION_201)
    {"fmktemp", MACRO(do_mktemp), ARG_INT, 0, 0,            /* file */
    1,  {ARG_LVAL | ARG_STRING}},
#endif

    {"fmod", MACRO(do_fmod), ARG_FLOAT, 0, 0,               /* float, arith */
    2,  {ARG_FLOAT, ARG_FLOAT}},

    {"fopen", MACRO(do_fopen), ARG_INT, 0, 0,               /* file */
    4,  {ARG_STRING, ARG_INT|ARG_STRING, ARG_INT, ARG_INT|ARG_OPT}},

    {"for", MACRO(do_for), ARG_VOID, 0, 0,                  /* macro */
    4,  {ARG_OPT | ARG_ANY,  ARG_OPT | ARG_COND, ARG_OPT | ARG_COND, ARG_OPT | ARG_REST}},

    {"format", MACRO(do_format), ARG_STRING, 0, 0,          /* string */
    -2, {ARG_STRING, ARG_OPT | ARG_ANY}},

    {"fread", MACRO(do_fread), ARG_INT, 0, 0,               /* file */
    1,  {ARG_INT}},

    {"frexp", MACRO(do_frexp), ARG_FLOAT, 0, 0,             /* float, arith */
    2,  {ARG_FLOAT, ARG_LVAL | ARG_INT}},

    {"fseek", MACRO(do_fseek), ARG_INT, 0, 0,               /* file */
    1,  {ARG_INT}},

    {"fstat", MACRO(do_fstat), ARG_INT, 0, 0,               /* file */
    1,  {ARG_INT}},

    {"fstype", MACRO(do_fstype), ARG_INT, 0, 0,             /* macro */
    1,  {ARG_OPT | ARG_STRING}},

    {"ftell", MACRO(do_ftell), ARG_INT, 0, 0,               /* file */
    1,  {ARG_INT}},

    {"ftest", MACRO(do_ftest), ARG_INT, 0, 0,               /* file */
    2,  {ARG_INT|ARG_STRING, ARG_STRING}},

    {"ftp_chdir", MACRO(do_ftp_chdir), ARG_INT, 0, 0,       /* comms */
    2,  {ARG_INT, ARG_STRING}},

    {"ftp_close", MACRO(do_ftp_close), ARG_INT, 0, 0,       /* comms */
    2,  {ARG_INT, ARG_OPT | ARG_INT}},

    {"ftp_connect", MACRO(do_ftp_connect), ARG_INT, 0, 0,   /* comms */
    6,  {ARG_INT, ARG_STRING,
            ARG_OPT | ARG_INT, ARG_OPT | ARG_STRING, ARG_OPT | ARG_STRING, ARG_OPT | ARG_STRING}},

    {"ftp_connection_list", MACRO(do_ftp_connection_list), ARG_LIST, 0, 0, /* comms */
    1,  {ARG_OPT | ARG_STRING}},

    {"ftp_create", MACRO(do_ftp_create), ARG_INT, 0, 0,     /* comms */
    1,  {ARG_OPT | ARG_INT | ARG_STRING}},

    {"ftp_directory", MACRO(do_ftp_directory), ARG_INT, 0, 0, /* comms */
    3,  {ARG_INT, ARG_OPT | ARG_STRING, ARG_LVAL | ARG_LIST}},

    {"ftp_error", MACRO(do_ftp_error), ARG_INT, 0, 0,       /* comms */
    2,  {ARG_INT, ARG_LVAL | ARG_STRING}},

    {"ftp_find_connection", MACRO(do_ftp_find_connection), ARG_INT, 0, 0, /* comms */
    1,  {ARG_STRING}},

    {"ftp_get_file", MACRO(do_ftp_get_file), ARG_INT, 0, 0, /* comms */
    3,  {ARG_INT, ARG_STRING, ARG_STRING}},

//  {"ftp_get_options", MACRO(do_ftp_get_options), ARG_INT, 0, 0, /* comms */
//  3,  {ARG_INT, ARG_OPT | ARG_INT, ARG_LVAL | ARG_STRING | ARG_LIST}},

    {"ftp_getcwd", MACRO(do_ftp_getcwd), ARG_INT, 0, 0,     /* comms */
    2,  {ARG_INT, ARG_LVAL | ARG_STRING}},

    {"ftp_mkdir", MACRO(do_ftp_mkdir), ARG_INT, 0, 0,       /* comms */
    2,  {ARG_INT, ARG_STRING}},

    {"ftp_protocol", MACRO(do_ftp_protocol), ARG_INT, 0, 0, /* comms */
    2,  {ARG_INT, ARG_LVAL | ARG_STRING}},

    {"ftp_put_file", MACRO(do_ftp_put_file), ARG_INT, 0, 0, /* comms */
    3,  {ARG_INT, ARG_STRING, ARG_STRING}},

    {"ftp_register", MACRO(do_ftp_register), ARG_INT, 0, 0, /* comms */
    3,  {ARG_INT, ARG_INT, ARG_STRING}},

    {"ftp_remove", MACRO(do_ftp_remove), ARG_INT, 0, 0,     /* comms */
    2,  {ARG_INT, ARG_STRING}},

    {"ftp_rename", MACRO(do_ftp_rename), ARG_INT, 0, 0,     /* comms */
    3,  {ARG_INT, ARG_STRING, ARG_STRING}},

    {"ftp_set_options", MACRO(do_ftp_set_options), ARG_INT, 0, 0, /* comms */
    3,  {ARG_INT, ARG_OPT | ARG_INT, ARG_STRING | ARG_LIST}},

    {"ftp_sitename", MACRO(do_ftp_sitename), ARG_INT, 0, 0, /* comms */
    2,  {ARG_INT, ARG_STRING}},

    {"ftp_stat", MACRO(do_ftp_stat), ARG_INT, 0, 0,         /* comms */
    6,  {ARG_INT, ARG_STRING,
            ARG_LVAL | ARG_OPT | ARG_INT,
            ARG_LVAL | ARG_OPT | ARG_INT,
            ARG_LVAL | ARG_OPT | ARG_INT,
            ARG_LVAL | ARG_OPT | ARG_INT}},

    {"ftp_timeout", MACRO(do_ftp_timeout), ARG_INT, 0, 0,   /* comms */
    3,  {ARG_INT, ARG_INT, ARG_OPT | ARG_INT}},

    {"ftruncate", MACRO(do_ftruncate), ARG_INT, 0, 0,       /* file */
    2,  {ARG_INT|ARG_STRING, ARG_INT}},

    {"fwrite", MACRO(do_fwrite), ARG_INT, 0, 0,             /* file */
    1,  {ARG_INT}},

    {"get_color", MACRO(do_get_color), ARG_LIST, 0, 0,      /* screen */
    1,  {ARG_OPT | ARG_INT}},

    {"get_color_pair", MACRO(do_get_color_pair), ARG_VOID, 0, 0, /* screen, syntax */
    4,  {ARG_INT | ARG_STRING,
         ARG_OPT | ARG_LVAL | ARG_INT | ARG_STRING,
         ARG_OPT | ARG_LVAL | ARG_INT | ARG_STRING,
         ARG_OPT | ARG_LVAL | ARG_INT | ARG_STRING}},

    {"get_mouse_pos", MACRO(do_get_mouse_pos), ARG_INT, 0, 0, /* kbd */
    8,  {ARG_OPT | ARG_LVAL | ARG_INT,
         ARG_OPT | ARG_LVAL | ARG_INT,
         ARG_OPT | ARG_LVAL | ARG_INT,
         ARG_OPT | ARG_LVAL | ARG_INT,
         ARG_OPT | ARG_LVAL | ARG_INT,
         ARG_OPT | ARG_LVAL | ARG_INT,
         ARG_OPT | ARG_LVAL | ARG_INT,
         ARG_OPT | ARG_LVAL | ARG_INT}},

    {"get_nth", MACRO(do_get_nth), ARG_ANY, 0, 0,           /* list */
    2,  {ARG_LIST, ARG_INT}},

    {"get_parm", MACRO(do_get_parm), ARG_INT, 0, 0,         /* macro, var */
    6,  {ARG_OPT | ARG_INT,
         ARG_ANY | ARG_LVAL,
         ARG_OPT | ARG_STRING,
         ARG_OPT | ARG_INT,
         ARG_OPT | ARG_ANY,
         ARG_OPT | ARG_INT}},

    {"get_property", MACRO(do_get_property/*obj_id, key*/), ARG_ANY, 0, 0, /* macro */
    2,  {ARG_INT, ARG_STRING}},

    {"get_region", MACRO(do_get_region), ARG_STRING, 0, 0,  /* scrap */
    1,  {ARG_OPT | ARG_INT}},

#if defined(VERSION_207)
    {"get_system_resources", MACRO(do_get_system_resources), ARG_STRING, 0, 0,  /* env */
    1,  {ARG_OPT | ARG_INT}},
#endif

    {"get_term_characters", MACRO(do_get_term_characters), ARG_VOID, B_NOVALUE, 0, /* screen, env */
    1,  {ARG_REST | ARG_OPT}},

    {"get_term_feature", MACRO(do_get_term_feature), ARG_INT, 0, 0, /* screen, env */
    2,  {ARG_INT | ARG_STRING, ARG_OPT | ARG_LVAL | ARG_INT | ARG_STRING }},

    {"get_term_features", MACRO(do_get_term_features), ARG_VOID, B_NOVALUE, 0, /* screen, env */
    1,  {ARG_REST | ARG_OPT}},

    {"get_term_keyboard", MACRO(do_get_term_keyboard), ARG_INT, 0, 0, /* screen, env */
    1,  {ARG_OPT | ARG_INT}},

    {"getenv", MACRO(do_getenv), ARG_STRING, 0, 0,          /* env */
    1,  {ARG_STRING}},

#if defined(VERSION_204)
    {"geteuid", MACRO(do_geteuid), ARG_INT, 0, 0,           /* env */
    0,  {0}},
#endif

    {"getopt", MACRO(do_getopt), ARG_INT, 0, 0,             /* misc, var */
    7,  {ARG_LVAL | ARG_STRING,
         ARG_OPT  | ARG_STRING,
         ARG_OPT  | ARG_LIST,
         ARG_OPT  | ARG_STRING | ARG_LIST,
         ARG_OPT  | ARG_STRING,
         ARG_OPT  | ARG_INT,
         ARG_OPT  | ARG_INT}},

    {"getpid", MACRO(do_getpid), ARG_INT, 0, 0,             /* env */
    0,  {0}},

    {"getsubopt", MACRO(do_getsubopt), ARG_INT, 0, 0,       /* misc, var */
    4,  {ARG_LVAL | ARG_STRING,
         ARG_OPT  | ARG_LIST,
         ARG_OPT  | ARG_STRING | ARG_LIST,
         ARG_OPT  | ARG_STRING}},

#if defined(VERSION_201)
    {"getuid", MACRO(do_getuid), ARG_INT, 0, 0,             /* env */
    0,  {0}},
#endif

    {"getwd", MACRO(do_getwd), ARG_INT, 0, 0,               /* env */
    2,  {ARG_OPT | ARG_STRING, ARG_LVAL | ARG_STRING}},

    {"glob", MACRO(do_glob), ARG_STRING, 0, 0,              /* file */
    1,  {ARG_STRING}},

    {"global", MACRO(do_global), ARG_VOID, B_NOVALUE, 0,    /* var */
    1,  {ARG_REST}},

    {"gmtime", MACRO(do_gmtime), ARG_VOID, 0, 0,            /* misc */
    9,  {ARG_OPT | ARG_INT,
         ARG_OPT | ARG_LVAL | ARG_INT,
         ARG_OPT | ARG_LVAL | ARG_INT,
         ARG_OPT | ARG_LVAL | ARG_INT,
         ARG_OPT | ARG_LVAL | ARG_STRING,
         ARG_OPT | ARG_LVAL | ARG_STRING,
         ARG_OPT | ARG_LVAL | ARG_INT,
         ARG_OPT | ARG_LVAL | ARG_INT,
         ARG_OPT | ARG_LVAL | ARG_INT}},

    {"goto_bookmark", MACRO(do_goto_bookmark), ARG_INT, 0, 0, /* buffer */
    4,  {ARG_OPT  | ARG_INT,
         ARG_OPT | ARG_LVAL | ARG_INT,
         ARG_OPT | ARG_LVAL | ARG_INT,
         ARG_OPT | ARG_LVAL | ARG_INT}},

    {"goto_line", MACRO(do_goto_line), ARG_INT, 0, 0,       /* movement */
    1,  {ARG_OPT | ARG_INT}},

    {"goto_old_line", MACRO(do_goto_old_line), ARG_INT, 0, 0, /* movement */
    1,  {ARG_OPT | ARG_INT}},

#if defined(VERSION_201)
    {"grief_version", MACRO(do_grief_version), ARG_INT, 0, 0, /* misc */
    1,  {ARG_OPT | ARG_INT}},
#endif

    {"hilite_create", MACRO(do_hilite_create), ARG_INT, 0, 0, /* buffer,syntax */
    9, {ARG_OPT | ARG_INT, ARG_OPT | ARG_INT, ARG_OPT | ARG_INT,
        ARG_OPT | ARG_INT, ARG_OPT | ARG_INT, ARG_OPT | ARG_INT, ARG_OPT | ARG_INT,
        ARG_OPT | ARG_INT | ARG_STRING, ARG_OPT | ARG_INT}},

#if defined(VERSION_201)
    {"hilite_delete", MACRO(do_hilite_delete), ARG_INT, 0, 0, /* buffer,syntax */
    2,  {ARG_OPT | ARG_INT, ARG_INT}},
#endif

    {"hilite_destroy", MACRO(do_hilite_destroy), ARG_INT, 0, 0, /* buffer,syntax */
    2,  {ARG_OPT | ARG_INT, ARG_OPT | ARG_INT}},

    {"if", MACRO(do_if), ARG_VOID, 0, 0,                    /* macro */
    3,  {ARG_NUM | ARG_STRING | ARG_LIST, ARG_COND, ARG_OPT | ARG_COND}},

    {"index", MACRO(do_index), ARG_INT, 0, 0,               /* string */
    2,  {ARG_STRING, ARG_INT | ARG_STRING}},

#if defined(VERSION_203)
    {"iniclose", MACRO(do_iniclose), ARG_UNDEF, 0, 0,       /* file */
    1,  {ARG_INT}},

    {"iniexport", MACRO(do_iniexport), ARG_INT, 0, 0,       /* file */
    2,  {ARG_INT, ARG_OPT | ARG_STRING}},

    {"inifirst", MACRO(do_inifirst), ARG_INT, 0, 0,         /* file */
    5,  {ARG_INT, ARG_OPT | ARG_LVAL | ARG_STRING, ARG_OPT | ARG_LVAL | ARG_STRING, ARG_OPT | ARG_LVAL | ARG_STRING, ARG_OPT | ARG_STRING}},

    {"ininext", MACRO(do_ininext), ARG_INT, 0, 0,           /* file */
    4,  {ARG_INT, ARG_OPT | ARG_LVAL | ARG_STRING, ARG_OPT | ARG_LVAL | ARG_STRING, ARG_OPT | ARG_LVAL | ARG_STRING}},

    {"iniopen", MACRO(do_iniopen), ARG_INT, 0, 0,           /* file */
    2,  {ARG_OPT | ARG_STRING, ARG_OPT | ARG_INT}},

    {"iniproperties", MACRO(do_iniproperties), ARG_LIST, 0, 0, /* file */
    2,  {ARG_INT, ARG_STRING}},

    {"inipush", MACRO(do_inipush), ARG_INT, 0, 0,           /* file */
    6,  {ARG_INT, ARG_OPT | ARG_STRING, ARG_OPT | ARG_STRING, ARG_OPT | ARG_STRING, ARG_OPT | ARG_STRING, ARG_OPT | ARG_INT}},

    {"iniquery", MACRO(do_iniquery), ARG_STRING, 0, 0,      /* file */
    3,  {ARG_INT, ARG_STRING, ARG_STRING}},

    {"iniremove", MACRO(do_iniremove), ARG_STRING, 0, 0,    /* file */
    4,  {ARG_INT, ARG_OPT | ARG_STRING, ARG_OPT | ARG_STRING, ARG_OPT | ARG_INT}},
#endif

    {"input_mode", MACRO(do_input_mode), ARG_INT, 0, 0,     /* kbd */
    2,  {ARG_INT, ARG_INT}},

    {"inq_assignment", MACRO(inq_assignment), ARG_STRING, 0, 0, /* kbd */
    2,  {ARG_INT | ARG_STRING, ARG_OPT | ARG_INT}},

    {"inq_attribute", MACRO(inq_attribute), ARG_INT, 0, 0,  /* buffer */
    2,  {ARG_OPT | ARG_INT | ARG_LVAL, ARG_OPT | ARG_INT}},

#if defined(VERSION_201)
    {"inq_backup", MACRO(inq_backup), ARG_INT, 0, 0,        /* env */
    1,  {ARG_OPT | ARG_INT}},
#endif

    {"inq_backup_option", MACRO(inq_backup_option), ARG_INT, 0, 0, /* env */
    2,  {ARG_INT, ARG_OPT | ARG_INT}},

    {"inq_borders", MACRO(inq_borders), ARG_INT, 0, 0,      /* screen */
    0,  {0}},

    {"inq_brief_level", MACRO(do_unimp), ARG_INT, 0, 0,     /* env */
    0,  {0}},

#if defined(VERSION_204)
    {"inq_btn2_action", MACRO(inq_btn2_action), ARG_STRING, 0, 0, /* macro */
    0,  {0}},
#endif

    {"inq_buffer", MACRO(inq_buffer), ARG_INT, 0, 0,        /* buffer, file */
    1,  {ARG_OPT | ARG_STRING}},

    {"inq_buffer_flags", MACRO(inq_buffer_flags), ARG_INT, 0, 0, /* buffer */
    3,  {ARG_OPT | ARG_INT, ARG_OPT | ARG_INT | ARG_STRING, ARG_OPT | ARG_STRING | ARG_LVAL}},

    {"inq_buffer_title", MACRO(inq_buffer_title), ARG_STRING, 0, 0, /* buffer */
    1,  {ARG_OPT | ARG_INT}},

    {"inq_buffer_type", MACRO(inq_buffer_type), ARG_INT, 0, 0, /* buffer */
    3,  {ARG_OPT | ARG_INT, ARG_OPT | ARG_STRING | ARG_LVAL, ARG_OPT | ARG_STRING | ARG_LVAL}},

#if defined(VERSION_204)
    {"inq_byte_pos", MACRO(inq_byte_pos), ARG_INT, 0, 0,    /* buffer */
    4,  {ARG_OPT | ARG_INT, ARG_OPT | ARG_INT, ARG_OPT | ARG_INT, ARG_OPT | ARG_INT}},
#endif

    {"inq_called", MACRO(inq_called), ARG_STRING, 0, 0,     /* macro */
    0,  {0}},

    {"inq_char_map", MACRO(inq_char_map), ARG_INT, 0, 0,    /* buffer, env, window */
    2,  {ARG_OPT | ARG_INT, ARG_OPT | ARG_STRING | ARG_LVAL}},

    {"inq_char_timeout", MACRO(inq_char_timeout), ARG_INT, 0, 0, /* screen, env */
    1,  {ARG_OPT | ARG_INT | ARG_LVAL}},

    {"inq_clock", MACRO(inq_clock), ARG_INT, 0, 0,          /* misc */
    0,  {0}},

    {"inq_cmd_line", MACRO(inq_cmd_line), ARG_STRING, 0, 0, /* screen */
    0,  {0}},

    {"inq_color", MACRO(inq_color), ARG_STRING, 0, 0,       /* screen */
    -1, {ARG_OPT | ARG_INT | ARG_STRING | ARG_LVAL}},

    {"inq_command", MACRO(inq_command), ARG_STRING, 0, 0,   /* screen */
    0,  {0}},

#if defined(VERSION_204)
    {"inq_connection", MACRO(inq_connection), ARG_INT, 0, 0,  /* proc */
    3,  {ARG_OPT | ARG_INT, ARG_OPT | ARG_INT | ARG_LVAL, ARG_OPT | ARG_INT | ARG_LVAL}},
#endif

    {"inq_ctrl_state", MACRO(inq_ctrl_state), ARG_INT, 0, 0, /* window */
    2,  {ARG_INT, ARG_OPT | ARG_INT}},

    {"inq_debug", MACRO(inq_debug), ARG_INT, 0, 0,          /* debug */    0,  {0}},

    {"inq_dialog", MACRO(inq_dialog), ARG_INT, 0, 0,        /* dialog */
    0,  {0}},

    {"inq_display_mode", MACRO(inq_display_mode), ARG_INT, 0, 0, /* screen */
    2,  {ARG_OPT | ARG_STRING, ARG_OPT | ARG_STRING | ARG_LVAL}},

    {"inq_echo_format", MACRO(inq_echo_format), ARG_STRING, 0, 0,  /* screen */
    0,  {0}},

    {"inq_echo_line", MACRO(inq_echo_line), ARG_INT, 0, 0,  /* screen */
    0,  {0}},

    {"inq_encoding", MACRO(inq_encoding), ARG_INT, 0, 0,    /* buffer */
    1,  {ARG_OPT | ARG_INT}},

    {"inq_feature", MACRO(inq_feature), ARG_LIST | ARG_INT, 0, 0, /* env */
    2,  {ARG_OPT | ARG_STRING, ARG_OPT | ARG_STRING | ARG_LVAL}},

    {"inq_file_change", MACRO(inq_file_change), ARG_INT, 0, 0, /* buffer */
    1,  {ARG_OPT | ARG_INT}},

    {"inq_file_magic", MACRO(inq_file_magic), ARG_STRING, 0, 0, /* file */
    2,  {ARG_OPT | ARG_INT | ARG_LVAL, ARG_OPT | ARG_INT | ARG_LVAL}},

    {"inq_font", MACRO(do_inq_font), ARG_INT, 0, 0,         /* screen */
    2,  {ARG_OPT | ARG_STRING | ARG_LVAL, ARG_OPT | ARG_STRING | ARG_LVAL}},

#if defined(VERSION_201)
    {"inq_hilite", MACRO(inq_home), ARG_INT, 0, 0,          /* buffer,syntax */
    6,  {ARG_OPT | ARG_INT,
        ARG_OPT | ARG_INT,
        ARG_OPT | ARG_INT,
        ARG_OPT | ARG_STRING | ARG_LVAL,
        ARG_OPT | ARG_STRING | ARG_LVAL,
        ARG_OPT | ARG_STRING | ARG_LVAL}},
#endif

    {"inq_home", MACRO(inq_home), ARG_STRING, 0, 0,         /* file */
    0,  {0}},

    {"inq_hostname", MACRO(inq_hostname), ARG_STRING, 0, 0, /* env */
    0,  {0}},

    {"inq_idle_default", MACRO(inq_idle_default), ARG_INT, 0, 0, /* env */
    0,  {0}},

    {"inq_idle_time", MACRO(inq_idle_time), ARG_INT, 0, 0,  /* env */
    0,  {0}},

    {"inq_indent", MACRO(inq_indent), ARG_INT, 0, 0,        /* buffer */
    0,  {0}},

    {"inq_kbd_char", MACRO(inq_kbd_char), ARG_INT, 0, 0,    /* kbd */
    0,  {0}},

#if defined(VERSION_201)
    {"inq_kbd_flags", MACRO(inq_kbd_flags), ARG_INT, 0, 0,  /* kbd */
    0,  {0}},
#endif

    {"inq_kbd_name", MACRO(inq_kbd_name), ARG_STRING, 0, 0, /* kbd */
    1,  {ARG_OPT | ARG_INT}},

    {"inq_keyboard", MACRO(inq_keyboard), ARG_INT, 0, 0,    /* kbd */
    0,  {0}},

    {"inq_keystroke_macro", MACRO(inq_keystroke_macro), ARG_STRING, 0, 0, /* kbd */
    2,  {ARG_OPT | ARG_INT, ARG_OPT | ARG_LVAL | ARG_INT}},

#if defined(VERSION_204)
    {"inq_keystroke_status", MACRO(inq_keystroke_status), ARG_INT, 0, 0, /* kbd */
    1,  {ARG_OPT | ARG_LVAL | ARG_INT}},
#endif

    {"inq_line_col", MACRO(inq_line_col), ARG_STRING, 0, 0, /* screen */
    0,  {0}},

    {"inq_line_flags", MACRO(inq_line_col), ARG_INT, 0, 0,  /* buffer */
    3,  {ARG_OPT | ARG_INT, ARG_OPT | ARG_INT, ARG_OPT | ARG_LVAL | ARG_INT}},

    {"inq_line_length", MACRO(inq_line_length), ARG_INT, 0, 0, /* buffer */
    1,  {ARG_OPT | ARG_INT}},

    {"inq_lines", MACRO(inq_lines), ARG_INT, 0, 0,          /* buffer */
    1,  {ARG_OPT | ARG_INT}},

    {"inq_local_keyboard", MACRO(inq_local_keyboard), ARG_INT, 0, 0, /* kbd */
    0,  {0}},

    {"inq_macro", MACRO(inq_macro), ARG_INT, 0, 0,          /* macro */
    2,  {ARG_STRING, ARG_OPT | ARG_INT}},

    {"inq_macro_history", MACRO(inq_macro_history), ARG_STRING, 0, 0, /* macro */
    1,  {ARG_OPT | ARG_INT}},

    {"inq_margins", MACRO(inq_margins), ARG_INT, 0, 0,      /* buffer */
    6,  {ARG_OPT | ARG_INT,
         ARG_OPT | ARG_LVAL | ARG_INT,
         ARG_OPT | ARG_LVAL | ARG_INT,
         ARG_OPT | ARG_LVAL | ARG_INT,
         ARG_OPT | ARG_LVAL | ARG_INT,
         ARG_OPT | ARG_INT}},

    {"inq_mark_size", MACRO(inq_mark_size), ARG_INT, 0, 0,  /* scrap */
    0,  {0}},

    {"inq_marked", MACRO(inq_marked), ARG_INT, 0, 0,        /* scrap */
    4,  {ARG_OPT | ARG_LVAL | ARG_INT,
         ARG_OPT | ARG_LVAL | ARG_INT,
         ARG_OPT | ARG_LVAL | ARG_INT,
         ARG_OPT | ARG_LVAL | ARG_INT}},

#if defined(VERSION_201)
    {"inq_marked_size", MACRO(inq_marked_size), ARG_INT, 0, 0, /* scrap */
    0,  {0}},
#endif

    {"inq_message", MACRO(inq_message), ARG_STRING, 0, 0,   /* screen */
    0,  {0}},

    {"inq_mode", MACRO(inq_mode), ARG_INT, 0, 0,            /* buffer, window */
    2,  {ARG_OPT | ARG_INT, ARG_OPT | ARG_LVAL | ARG_INT}},

    {"inq_modified", MACRO(inq_modified), ARG_INT, 0, 0,    /* buffer */
    1,  {ARG_OPT | ARG_INT}},

    {"inq_module", MACRO(inq_module), ARG_INT, 0, 0,        /* macro */
    1,  {ARG_OPT | ARG_INT}},

#if defined(VERSION_204)
    {"inq_mouse_action", MACRO(inq_mouse_action), ARG_STRING, 0, 0, /* kbd */
    0,  {0}},
#endif

#if defined(VERSION_204)
    {"inq_mouse_type", MACRO(inq_mouse_type), ARG_INT, 0, 0, /* kbd */
    0,  {0}},
#endif

    {"inq_msg_level", MACRO(inq_msg_level), ARG_INT, 0, 0,  /* macro */
    0,  {0}},

    {"inq_names", MACRO(inq_names), ARG_VOID, 0, 0,         /* buffer */
    4,  {ARG_OPT | ARG_LVAL | ARG_STRING,
         ARG_OPT | ARG_LVAL | ARG_STRING,
         ARG_OPT | ARG_LVAL | ARG_STRING,
         ARG_OPT | ARG_INT}},

    {"inq_position", MACRO(inq_position), ARG_INT, 0, 0,    /* window, buffer */
    2,  {ARG_OPT | ARG_LVAL | ARG_INT,
         ARG_OPT | ARG_LVAL | ARG_INT}},

    {"inq_process_position", MACRO(inq_process_position), ARG_INT, 0, 0, /* proc, buffer */
    2,  {ARG_OPT | ARG_LVAL | ARG_INT,
         ARG_OPT | ARG_LVAL | ARG_INT}},

    {"inq_profile", MACRO(inq_profile), ARG_STRING, 0, 0,   /* env */
    0,  {0}},

    {"inq_prompt", MACRO(inq_prompt), ARG_INT, 0, 0,        /* screen */
    0,  {0}},

#if defined(VERSION_207)
    {"inq_remember_buffer", MACRO(inq_remember_buffer), ARG_STRING, 0, 0, /* kbd */
    1,  {ARG_OPT | ARG_INT}},
#endif

    {"inq_ruler", MACRO(inq_ruler), ARG_ANY, 0, 0,          /* buffer */
    3,  {ARG_OPT | ARG_INT, ARG_OPT | ARG_INT, ARG_OPT | ARG_INT}},

    {"inq_scrap", MACRO(inq_scrap), ARG_INT, 0, 0,          /* scrap */
    2,  {ARG_OPT | ARG_LVAL | ARG_INT, ARG_OPT | ARG_LVAL | ARG_INT}},

    {"inq_screen_size", MACRO(inq_screen_size), ARG_INT, 0, 0, /* screen */
    3,  {ARG_OPT | ARG_LVAL | ARG_INT, ARG_OPT | ARG_LVAL | ARG_INT, ARG_OPT | ARG_LVAL | ARG_INT}},

    {"inq_symbol", MACRO(inq_symbol), ARG_INT, 0, 0,        /* var */
    1,  {ARG_STRING}},

    {"inq_syntax", MACRO(inq_syntax), ARG_INT, 0, 0,        /* syntax */
    2,  {ARG_OPT | ARG_LVAL | ARG_INT, ARG_OPT | ARG_INT | ARG_STRING}},

    {"inq_system", MACRO(inq_system), ARG_INT, 0, 0,        /* buffer */
    1,  {ARG_OPT | ARG_INT}},

    {"inq_tab", MACRO(inq_tab), ARG_INT, 0, 0,              /* buffer */
    1,  {ARG_OPT | ARG_INT}},

    {"inq_tabs", MACRO(inq_tabs), ARG_ANY, 0, 0,            /* buffer */
    3,  {ARG_OPT | ARG_INT, ARG_OPT | ARG_INT, ARG_OPT | ARG_INT}},

    {"inq_terminator", MACRO(inq_terminator), ARG_INT, 0, 0, /* buffer */
    2,  {ARG_OPT | ARG_INT, ARG_OPT | ARG_LVAL | ARG_STRING}},

    {"inq_time", MACRO(inq_time), ARG_INT, 0, 0,            /* buffer */
    2,  {ARG_OPT | ARG_INT, ARG_OPT | ARG_LVAL | ARG_INT}},

    {"inq_tmpdir", MACRO(inq_tmpdir), ARG_STRING, 0, 0,     /* file */
    0,  {0}},

    {"inq_top_left", MACRO(inq_top_left), ARG_INT, 0, 0,    /* buffer, window */
    6,  {ARG_OPT | ARG_LVAL | ARG_INT,
         ARG_OPT | ARG_LVAL | ARG_INT,
                    ARG_OPT | ARG_INT,
         ARG_OPT | ARG_LVAL | ARG_INT,
         ARG_OPT | ARG_LVAL | ARG_INT,
         ARG_OPT | ARG_LVAL | ARG_INT}},

    {"inq_username", MACRO(inq_username), ARG_STRING, 0, 0, /* env */
    0,  {0}},

    {"inq_vfs_mounts", MACRO(inq_vfs_mounts), ARG_LIST, 0, 0, /* file */
    0,  {0}},

    {"inq_views", MACRO(inq_views), ARG_INT, 0, 0,          /* buffer, window */
    1,  {ARG_OPT | ARG_INT}},

    {"inq_window", MACRO(inq_window), ARG_INT, 0, 0,        /* window */
    0,  {0}},

    {"inq_window_buf", MACRO(inq_window_buf), ARG_INT, 0, 0, /* buffer, window */
    1,  {ARG_OPT | ARG_INT}},

    {"inq_window_color", MACRO(inq_window_color), ARG_INT, 0, 0, /* window, screen */
    1,  {ARG_OPT | ARG_INT}},

    {"inq_window_flags", MACRO(inq_window_flags), ARG_INT, 0, 0, /* window */
    2,  {ARG_OPT | ARG_INT, ARG_OPT | ARG_STRING}},

    {"inq_window_info", MACRO(inq_window_info), ARG_INT, 0, VALUE(0), /* window */
    8,  {ARG_OPT | ARG_LVAL | ARG_INT,
         ARG_OPT | ARG_LVAL | ARG_INT,
         ARG_OPT | ARG_LVAL | ARG_INT,
         ARG_OPT | ARG_LVAL | ARG_INT,
         ARG_OPT | ARG_LVAL | ARG_INT,
         ARG_OPT | ARG_LVAL | ARG_INT,
         ARG_OPT | ARG_LVAL | ARG_STRING,
         ARG_OPT | ARG_LVAL | ARG_STRING}},

#if defined(VERSION_201)
    {"inq_window_infox", MACRO(inq_window_info), ARG_INT, 0, VALUE(1), /* window */
    8, {ARG_OPT | ARG_INT,
        ARG_OPT | ARG_LVAL | ARG_INT,
        ARG_OPT | ARG_LVAL | ARG_INT,
        ARG_OPT | ARG_LVAL | ARG_INT,
        ARG_OPT | ARG_LVAL | ARG_INT,
        ARG_OPT | ARG_LVAL | ARG_INT,
        ARG_OPT | ARG_LVAL | ARG_STRING,
        ARG_OPT | ARG_LVAL | ARG_STRING}},
#endif

    {"inq_window_priority", MACRO(inq_window_priority), ARG_INT, 0, 0, /* screen */
    1,  {ARG_OPT | ARG_INT}},

    {"inq_window_size", MACRO(inq_window_size), ARG_INT, 0, 0, /* window, screen */
    5,  {ARG_OPT | ARG_LVAL | ARG_INT,
         ARG_OPT | ARG_LVAL | ARG_INT,
         ARG_OPT | ARG_LVAL | ARG_INT,
         ARG_OPT | ARG_LVAL | ARG_INT,
         ARG_OPT | ARG_LVAL | ARG_INT}},

    {"insert", MACRO(do_insert), ARG_INT, 0, FALSE,         /* buffer, window */
    2,  {ARG_INT | ARG_STRING, ARG_OPT | ARG_INT}},

    {"insert_buffer", MACRO(do_insert_buffer), ARG_INT, 0, FALSE, /* buffer, window */
    -3, {ARG_INT, ARG_STRING, ARG_OPT | ARG_ANY}},

    {"insert_mode", MACRO(do_insert_mode), ARG_INT, 0, 0,   /* buffer, window */
    2,  {ARG_OPT | ARG_INT, ARG_OPT | ARG_INT}},

    {"insert_process", MACRO(do_insert), ARG_INT, 0, TRUE,  /* buffer, window, proc */
    2,  {ARG_STRING, ARG_OPT | ARG_INT}},

    {"insertf", MACRO(do_insertf), ARG_INT, 0, FALSE,       /* buffer, window */
    -2, {ARG_STRING, ARG_OPT | ARG_ANY}},

#if defined(VERSION_204)
    {"inside_region", MACRO(do_inside_region), ARG_INT, 0, 0, /* scrap */
    3,  {ARG_OPT | ARG_INT, ARG_OPT | ARG_INT, ARG_OPT | ARG_INT}},
#endif

    {"int", MACRO(do_declare), ARG_VOID, B_NOVALUE, F_INT,  /* var */
    1,  {ARG_REST}},

    {"int_to_key", MACRO(do_int_to_key), ARG_STRING, 0, 0,  /* kbd */
    1,  {ARG_INT}},

#if defined(VERSION_207)
#if defined(DO_ARRAY)
    {"is_array", MACRO(do_is_type), ARG_INT, 0, F_ARRAY,    /* var */
    1,  {ARG_LVAL | ARG_ANY}},
#endif
#endif

    {"is_float", MACRO(do_is_type), ARG_INT, 0, F_FLOAT,    /* var */
    1,  {ARG_LVAL | ARG_ANY}},

    {"is_integer", MACRO(do_is_type), ARG_INT, 0, F_INT,    /* var */
    1,  {ARG_LVAL | ARG_ANY}},

    {"is_list", MACRO(do_is_type), ARG_INT, 0, F_LIST,      /* var */
    1,  {ARG_LVAL | ARG_ANY}},

    {"is_null", MACRO(do_is_type), ARG_INT, 0, F_NULL,      /* var */
    1,  {ARG_LVAL | ARG_ANY}},

    {"is_string", MACRO(do_is_type), ARG_INT, 0, F_STR,     /* var */
    1,  {ARG_LVAL | ARG_ANY}},

    {"is_type", MACRO(do_is_type), ARG_INT, 0, 255,         /* var */
    2,  {ARG_LVAL | ARG_ANY, ARG_INT | ARG_STRING}},

    {"isalnum", MACRO(do_isalnum), ARG_INT, 0, 0,           /* string */
    2,  {ARG_INT | ARG_STRING, ARG_OPT | ARG_INT}},

    {"isalpha", MACRO(do_isalpha), ARG_INT, 0, 0,           /* string */
    2,  {ARG_INT | ARG_STRING, ARG_OPT | ARG_INT}},

    {"isascii", MACRO(do_isascii), ARG_INT, 0, 0,           /* string */
    2,  {ARG_INT | ARG_STRING, ARG_OPT | ARG_INT}},

    {"isblank", MACRO(do_isblank), ARG_INT, 0, 0,           /* string */
    2,  {ARG_INT | ARG_STRING, ARG_OPT | ARG_INT}},

#if defined(VERSION_205)
    {"isclose", MACRO(do_isclose), ARG_INT, 0, 0,           /* float, arith */
    4,  {ARG_FLOAT, ARG_FLOAT, ARG_OPT | ARG_FLOAT, ARG_OPT | ARG_FLOAT}},
#endif

    {"iscntrl", MACRO(do_iscntrl), ARG_INT, 0, 0,           /* string */
    2,  {ARG_INT | ARG_STRING, ARG_OPT | ARG_INT}},

    {"iscsym", MACRO(do_iscsym), ARG_INT, 0, 0,             /* string */
    2,  {ARG_INT | ARG_STRING, ARG_OPT | ARG_INT}},

    {"isdigit", MACRO(do_isdigit), ARG_INT, 0, 0,           /* string */
    2,  {ARG_INT | ARG_STRING, ARG_OPT | ARG_INT}},

    {"isfinite", MACRO(do_isfinite), ARG_INT, 0, 0,         /* float, arith */
    1,  {ARG_FLOAT}},

#if defined(VERSION_207)
    {"isgold", MACRO(do_isgold), ARG_INT, 0, 0,             /* string */
    2,  {ARG_INT | ARG_STRING, ARG_OPT | ARG_INT}},
#endif

    {"isgraph", MACRO(do_isgraph), ARG_INT, 0, 0,           /* string */
    2,  {ARG_INT | ARG_STRING, ARG_OPT | ARG_INT}},

    {"isinf", MACRO(do_isinf), ARG_INT, 0, 0,               /* float, arith */
    1,  {ARG_FLOAT}},

    {"islower", MACRO(do_islower), ARG_INT, 0, 0,           /* string */
    2,  {ARG_INT | ARG_STRING, ARG_OPT | ARG_INT}},

    {"isnan", MACRO(do_isnan), ARG_INT, 0, 0,               /* float, arith */
    1,  {ARG_FLOAT}},

    {"isprint", MACRO(do_isprint), ARG_INT, 0, 0,           /* string */
    2,  {ARG_INT | ARG_STRING, ARG_OPT | ARG_INT}},

    {"ispunct", MACRO(do_ispunct), ARG_INT, 0, 0,           /* string */
    2,  {ARG_INT | ARG_STRING, ARG_OPT | ARG_INT}},

    {"isspace", MACRO(do_isspace), ARG_INT, 0, 0,           /* string */
    2,  {ARG_INT | ARG_STRING, ARG_OPT | ARG_INT}},

    {"isupper", MACRO(do_isupper), ARG_INT, 0, 0,           /* string */
    2,  {ARG_INT | ARG_STRING, ARG_OPT | ARG_INT}},

    {"isword", MACRO(do_isword), ARG_INT, 0, 0,             /* string */
    2,  {ARG_INT | ARG_STRING, ARG_OPT | ARG_INT}},

    {"isxdigit", MACRO(do_isxdigit), ARG_INT, 0, 0,         /* string */
    2,  {ARG_INT | ARG_STRING, ARG_OPT | ARG_INT}},

#if defined(VERSION_204)
    {"itoa", MACRO(do_itoa), ARG_STRING, 0, 0,              /* string */
    2,  {ARG_INT, ARG_OPT | ARG_INT}},
#endif

    {"key_list", MACRO(do_key_list), ARG_LIST, 0, 0,        /* kbd */
    3,  {ARG_OPT | ARG_INT,
         ARG_OPT | ARG_INT,
         ARG_OPT | ARG_INT}},

    {"key_to_int", MACRO(do_key_to_int), ARG_INT, 0, 0,     /* kbd */
    2,  {ARG_STRING, ARG_OPT | ARG_INT}},

    {"keyboard_flush", MACRO(do_keyboard_flush), ARG_VOID, 0, 0, /* kbd */
    0,  {0}},

    {"keyboard_pop", MACRO(do_keyboard_pop), ARG_VOID, 0, 0, /* kbd */
    1,  {ARG_OPT | ARG_INT}},

    {"keyboard_push", MACRO(do_keyboard_push), ARG_INT, 0, 0, /* kbd */
    1,  {ARG_OPT | ARG_INT}},

    {"keyboard_typeables", MACRO(do_keyboard_typeables), ARG_VOID, 0, 0, /* kbd */
    0,  {0}},

    {"lastof", MACRO(do_lastof), ARG_INT, 0, 0,             /* string */
    3,  {ARG_STRING, ARG_STRING, ARG_OPT | ARG_LVAL | ARG_INT }},

    {"ldexp", MACRO(do_ldexp), ARG_FLOAT, 0, 0,             /* float, arith */
    2,  {ARG_FLOAT, ARG_INT}},

    {"left", MACRO(do_left), ARG_INT, 0, 0,                 /* movement */
    2,  {ARG_OPT | ARG_INT, ARG_OPT | ARG_INT}},

    {"length_of_list", MACRO(do_length_of_list), ARG_INT, 0, 0, /* list */
    1,  {ARG_LIST}},

#if defined(VERSION_207)
    {"link", MACRO(do_link), ARG_INT, 0, 0,                 /* file */
    3,  {ARG_STR, ARG_STR, ARG_OPT | ARG_INT}},
#endif

    {"list", MACRO(do_declare), ARG_VOID, B_NOVALUE, F_LIST, /* var, list */
    1,  {ARG_REST}},

    {"list_each", MACRO(do_list_each), ARG_INT, 0, 0,       /* var, list */
    3,  {ARG_LIST, ARG_ANY | ARG_LVAL, ARG_OPT | ARG_INT}},

    {"list_extract", MACRO(do_list_extract), ARG_LIST, 0, 0, /* var, list */
    4,  {ARG_LIST, ARG_OPT | ARG_INT, ARG_OPT | ARG_INT, ARG_OPT | ARG_INT}},

    {"list_of_dictionaries", MACRO(do_list_of_dictionaries), ARG_LIST, 0, 0, /* macro */
    1,  {ARG_OPT | ARG_INT}},

    {"list_reset", MACRO(do_list_reset), ARG_INT, 0, 0,     /* var, list */
    2,  {ARG_LIST, ARG_OPT | ARG_INT}},

    {"load_keystroke_macro", MACRO(do_load_keystroke_macro), ARG_INT, 0, 0, /* kbd */
    1,  {ARG_STRING}},

    {"load_macro", MACRO(do_load_macro), ARG_INT, 0, 0,     /* macro */
    2,  {ARG_OPT | ARG_STRING, ARG_OPT | ARG_INT}},

    {"localtime", MACRO(do_localtime), ARG_VOID, 0, 0,      /* misc */
    9,  {ARG_OPT | ARG_INT,
         ARG_OPT | ARG_LVAL | ARG_INT,
         ARG_OPT | ARG_LVAL | ARG_INT,
         ARG_OPT | ARG_LVAL | ARG_INT,
         ARG_OPT | ARG_LVAL | ARG_STRING,
         ARG_OPT | ARG_LVAL | ARG_STRING,
         ARG_OPT | ARG_LVAL | ARG_INT,
         ARG_OPT | ARG_LVAL | ARG_INT,
         ARG_OPT | ARG_LVAL | ARG_INT}},

    {"log", MACRO(do_log), ARG_FLOAT, 0, 0,                 /* float, arith */
    1,  {ARG_FLOAT}},

    {"log10", MACRO(do_log10), ARG_FLOAT, 0, 0,             /* float, arith */
    1,  {ARG_FLOAT}},

    {"lower", MACRO(do_lower), ARG_STRING, 0, 0,            /* string */
    1,  {ARG_INT | ARG_STRING}},

    {"lstat", MACRO(do_lstat), ARG_INT, 0, 0,               /* file */
    11, {ARG_OPT | ARG_STRING,
         ARG_OPT | ARG_LVAL | ARG_INT,
         ARG_OPT | ARG_LVAL | ARG_INT,
         ARG_OPT | ARG_LVAL | ARG_INT,
         ARG_OPT | ARG_LVAL | ARG_INT,
         ARG_OPT | ARG_LVAL | ARG_INT,
         ARG_OPT | ARG_LVAL | ARG_INT,
         ARG_OPT | ARG_LVAL | ARG_STRING,
         ARG_OPT | ARG_LVAL | ARG_INT,
         ARG_OPT | ARG_LVAL | ARG_STRING,
         ARG_OPT | ARG_LVAL | ARG_INT}},

    {"ltrim", MACRO(do_ltrim), ARG_STRING, 0, 0,            /* string */
    2,  {ARG_STRING, ARG_OPT | ARG_STRING}},

    {"macro", MACRO(do_macro), ARG_VOID, B_NOVALUE, 0,      /* macro */
    1,  {ARG_REST}},

    {"macro_list", MACRO(do_macro_list), ARG_LIST, 0, 0,    /* list, string */
    1,  {ARG_OPT | ARG_STRING}},

    {"make_list", MACRO(do_make_list), ARG_LIST, 0, 0,      /* list */
    -1, {ARG_OPT | ARG_ANY}},

    {"make_local_variable", MACRO(do_make_local_variable), ARG_VOID, B_NOVALUE, 0, /* var */
    1,  {ARG_REST}},

    {"mark", MACRO(do_mark), ARG_INT, 0, 0,                 /* scrap */
    1,  {ARG_OPT | ARG_INT}},

    {"mark_line", MACRO(do_mark_line), ARG_INT, 0, 0,       /* buffer */
    5,  {ARG_OPT | ARG_INT, ARG_OPT | ARG_INT, ARG_OPT | ARG_INT, ARG_OPT | ARG_INT, ARG_OPT | ARG_INT}},

    {"message", MACRO(do_message), ARG_VOID, B_NOVALUE, 0,  /* debug */
    -2, {ARG_STRING, ARG_OPT | ARG_ANY}},

    {"mkdir", MACRO(do_mkdir), ARG_INT, 0, 0,               /* file */
    2,  {ARG_STRING, ARG_OPT | ARG_INT}},

    {"mktemp", MACRO(do_mktemp), ARG_STRING, 0, 0,          /* file */
    1,  {ARG_STRING}},

    {"mode_string", MACRO(do_mode_string), ARG_STRING, 0, 0, /* misc, buffer */
    3,  {ARG_OPT | ARG_INT, ARG_OPT | ARG_STRING, ARG_OPT | ARG_INT}},

    {"modf", MACRO(do_modf), ARG_FLOAT, 0, 0,               /* float, arith */
    2,  {ARG_FLOAT, ARG_LVAL | ARG_FLOAT}},

    {"module", MACRO(do_module), ARG_INT, 0, 0,             /* macro */
    1,  {ARG_STRING}},

    {"move_abs", MACRO(do_move_abs), ARG_INT, 0, 0,         /* movement */
    4,  {ARG_OPT | ARG_INT, ARG_OPT | ARG_INT, ARG_OPT | ARG_INT, ARG_OPT | ARG_INT}},

    {"move_edge", MACRO(do_move_edge), ARG_INT, 0, 0,       /* window */
    2,  {ARG_OPT | ARG_INT, ARG_OPT | ARG_INT}},

    {"move_rel", MACRO(do_move_rel), ARG_INT, 0, 0,         /* movement */
    2,  {ARG_OPT | ARG_INT, ARG_OPT | ARG_INT}},

    {"next_buffer", MACRO(do_next_buffer), ARG_INT, 0, 0,   /* buffer */
    2,  {ARG_OPT | ARG_INT, ARG_OPT | ARG_INT}},

    {"next_char", MACRO(do_next_char), ARG_INT, 0, 0,       /* movement */
    1,  {ARG_OPT | ARG_INT}},

    {"next_window", MACRO(do_next_window), ARG_INT, 0, 0,   /* window */
    1,  {ARG_OPT | ARG_INT}},

    {"nothing", MACRO(do_nothing), ARG_VOID, 0, 0,          /* macro */
    0,  {0}},

    {"nth", MACRO(do_nth), ARG_ANY, B_SAFE, 0,              /* list */
    6,  {ARG_LIST, ARG_INT, ARG_OPT | ARG_INT, ARG_OPT | ARG_INT, ARG_OPT | ARG_INT, ARG_OPT | ARG_INT}},

    {"output_file", MACRO(do_output_file), ARG_INT, 0, 0,   /* file */
    1,  {ARG_OPT | ARG_STRING}},

    {"page_down", MACRO(do_page_down), ARG_INT, 0, 0,       /* movement */
    1,  {ARG_OPT | ARG_INT}},

    {"page_up", MACRO(do_page_up), ARG_INT, 0, 0,           /* movement */
    1,  {ARG_OPT | ARG_INT}},

#if defined(VERSION_202)
    {"parse_filename", MACRO(do_parse_filename), ARG_INT, 0, 0, /* movement */
    5,  {ARG_STRING,
        ARG_OPT | ARG_LVAL | ARG_STRING,
        ARG_OPT | ARG_LVAL | ARG_STRING,
        ARG_OPT | ARG_LVAL | ARG_STRING,
        ARG_OPT | ARG_LVAL | ARG_STRING}},
#endif

    {"paste", MACRO(do_paste), ARG_INT, 0, 0,               /* scrap */
    1,  {ARG_OPT | ARG_INT}},

    {"pause", MACRO(do_pause), ARG_VOID, 0, 0,              /* kbd */
    0,  {0}},

    {"pause_on_error", MACRO(do_pause_on_error), ARG_INT, 0, 0, /* debug */
    2,  {ARG_OPT | ARG_INT, ARG_OPT | ARG_INT}},

#if defined(VERSION_201)
    {"pause_on_message", MACRO(do_pause_on_message), ARG_INT, 0, 0, /* debug */
    2,  {ARG_OPT | ARG_INT, ARG_OPT | ARG_INT}},
#endif

    {"perror", MACRO(do_perror), ARG_VOID, B_NOVALUE, 0,    /* misc, proc */
    -3, {ARG_OPT | ARG_INT, ARG_STRING, ARG_OPT | ARG_ANY}},

    {"playback", MACRO(do_playback), ARG_INT, 0, 0,         /* kbd */
    1,  {ARG_OPT | ARG_INT}},

    {"pop", MACRO(do_pop), ARG_ANY, 0, 0,                   /* list */
    1,  {ARG_LVAL | ARG_LIST}},

    {"post++", MACRO(do_post_plusplus), ARG_ANY, 0, 0,      /* arith */
    1,  {ARG_LVAL | ARG_NUM}},

    {"post--", MACRO(do_post_minusminus), ARG_ANY, 0, 0,    /* arith */
    1,  {ARG_LVAL | ARG_NUM}},

    {"pow", MACRO(do_pow), ARG_FLOAT, 0, 0,                 /* float, arith */
    2,  {ARG_FLOAT, ARG_FLOAT}},

    {"prev_char", MACRO(do_prev_char), ARG_INT, 0, 0,       /* movement */
    1,  {ARG_OPT | ARG_INT}},

#if defined(VERSION_201)
    {"previous_buffer", MACRO(do_next_buffer), ARG_INT, 0, 1, /* buffer */
    2,  {ARG_OPT | ARG_INT, ARG_OPT | ARG_INT}},
#endif

    {"print", MACRO(do_print), ARG_INT, 0, 0,               /* buffer */
    0,  {0}},

    {"printf", MACRO(do_printf), ARG_INT, 0, 0,             /* debug */
    -2, {ARG_STRING, ARG_OPT | ARG_ANY}},

    {"process", MACRO(do_process), ARG_VOID, 0, 0,          /* kbd */
    0,  {0}},

    {"process_mouse", MACRO(do_process_mouse), ARG_VOID, 0, 0, /* kbd, misc */
    5,  {ARG_INT, ARG_INT, ARG_INT, ARG_INT, ARG_INT}},

    {"profile", MACRO(do_profile), ARG_VOID, 0, 0,          /* debug */
    1,  {ARG_OPT | ARG_INT}},

    {"push", MACRO(do_push), ARG_ANY, 0, 0,                 /* list */
    -2, {ARG_LVAL | ARG_LIST, ARG_OPT | ARG_ANY}},

    {"push_back", MACRO(do_push_back), ARG_VOID, 0, 0,      /* kbd */
    4,  {ARG_INT, ARG_OPT | ARG_INT, ARG_OPT | ARG_INT, ARG_OPT | ARG_INT}},

    {"put_nth", MACRO(do_put_nth), ARG_ANY, 0, 0,           /* list */
    6,  {ARG_LVAL | ARG_LIST, ARG_INT,
         ARG_OPT | ARG_ANY, ARG_OPT | ARG_ANY,
         ARG_OPT | ARG_ANY, ARG_OPT | ARG_ANY }},

    {"put_parm", MACRO(do_put_parm), ARG_INT, 0, 0,         /* macro, var */
    2,  {ARG_INT, ARG_ANY}},

    {"putenv", MACRO(do_putenv), ARG_INT, 0, 0,             /* env */
    2,  {ARG_STRING, ARG_STRING}},

    {"quote_list", MACRO(do_quote_list), ARG_LIST, 0, 0,    /* list */
    1,  {ARG_REST}},

    {"quote_regexp", MACRO(do_quote_regexp), ARG_STRING, 0, 0, /* string, search */
    1,  {ARG_STRING}},

    {"raise_anchor", MACRO(do_raise_anchor), ARG_INT, 0, 0, /* scrap */
    0,  {0}},

    {"rand", MACRO(do_rand), ARG_INT, 0, 0,                 /* misc */
    1,  {ARG_OPT | ARG_INT}},

#if defined(VERSION_201)
    {"re_comp", MACRO(do_re_comp), ARG_LIST, 0, 0,          /* search */
    2,  {ARG_OPT | ARG_INT, ARG_STRING}},
#endif

#if defined(VERSION_201)
    {"re_delete", MACRO(do_re_delete), ARG_INT, 0, 0,       /* search */
    1,  {ARG_INT}},
#endif

#if defined(VERSION_201)
    {"re_result", MACRO(do_re_result), ARG_INT, 0, 0,       /* search */
    2,  {ARG_OPT | ARG_INT, ARG_STRING | ARG_LVAL}},
#endif

    {"re_search", MACRO(do_re_search), ARG_INT, 0, 0,       /* search */
    5,  {ARG_OPT | ARG_INT,
         ARG_STRING,
         ARG_OPT | ARG_LIST | ARG_STRING,
         ARG_OPT | ARG_INT,
         ARG_OPT | ARG_LVAL | ARG_INT}},

    {"re_syntax", MACRO(do_re_syntax), ARG_INT, 0, 0,       /* search */
    3,  {ARG_OPT | ARG_INT, ARG_OPT | ARG_INT, ARG_OPT | ARG_INT}},

    {"re_translate", MACRO(do_re_translate), ARG_UNDEF, 0, 0, /* search */
            /* returns: int|string */
    4,  {ARG_OPT | ARG_INT,
         ARG_OPT | ARG_STRING,
         ARG_OPT | ARG_STRING,
         ARG_OPT | ARG_STRING}},

    {"read", MACRO(do_read), ARG_STRING, 0, 0,              /* string */
    2,  {ARG_OPT | ARG_INT, ARG_OPT | ARG_LVAL | ARG_INT}},

    {"read_char", MACRO(do_read_char), ARG_INT, 0, 0,       /* kbd */
    2,  {ARG_OPT | ARG_INT, ARG_OPT | ARG_INT}},

    {"read_file", MACRO(do_read_file), ARG_INT, 0, 0,       /* file */
    3,  {ARG_OPT | ARG_STRING, ARG_OPT | ARG_INT, ARG_OPT | ARG_STRING}},

    {"readlink", MACRO(do_readlink), ARG_ANY, 0, 0,         /* file */
    2,  {ARG_STRING, ARG_OPT | ARG_LVAL | ARG_STRING}},

    {"realpath", MACRO(do_realpath), ARG_INT, 0, 0,         /* file */
    2,  {ARG_STRING, ARG_OPT | ARG_LVAL | ARG_STRING}},

    {"redraw", MACRO(do_redraw), ARG_VOID, 0, 0,            /* screen */
    1,  {ARG_OPT | ARG_INT}},

    {"ref_parm", MACRO(do_ref_parm), ARG_VOID, 0, 0,        /* macro, var */
    3,  {ARG_INT, ARG_STRING, ARG_OPT | ARG_INT}},

    {"refresh", MACRO(do_refresh), ARG_VOID, 0, 0,          /* screen */
    0,  {0}},

#if defined(VERSION_205)
    {"register", MACRO(do_register), ARG_VOID, 0, 0,        /* var */
    1,  {ARG_REST}},
#endif

    {"register_macro", MACRO(do_register_macro), ARG_INT, 0, FALSE, /* macro */
    3,  {ARG_INT, ARG_STRING, ARG_OPT | ARG_INT}},

#if defined(VERSION_204)
    {"reload_buffer", MACRO(do_reload_buffer), ARG_INT, 0, FALSE, /* macro */
    1,  {ARG_OPT | ARG_INT}},
#endif

    {"remember", MACRO(do_remember), ARG_INT, 0, 0,         /* kbd */
    2,  {ARG_OPT | ARG_INT | ARG_STRING, ARG_OPT | ARG_INT}},

    {"remove", MACRO(do_remove), ARG_INT, 0, 0,             /* file */
    1,  {ARG_STRING}},

    {"rename", MACRO(do_rename), ARG_INT, 0, 0,             /* file */
    2,  {ARG_STRING, ARG_STRING}},

    {"replacement", MACRO(do_macro), ARG_VOID, B_NOVALUE, TRUE, /* macro */
    2,  {ARG_COND, ARG_REST}},

    {"require", MACRO(do_require), ARG_INT, 0, 0,           /* macro */
    1,  {ARG_STRING}},

    {"reregister_macro", MACRO(do_register_macro), ARG_INT, 0, TRUE, /* macro */
    3,  {ARG_INT, ARG_STRING, ARG_OPT | ARG_INT}},

    {"restore_position", MACRO(do_restore_position), ARG_INT, 0, 0, /* movement, macro */
    1,  {ARG_OPT | ARG_INT}},

    {"return", MACRO(do_returns), ARG_ANY, 0, FALSE,        /* macro */
    1,  {ARG_OPT | ARG_ANY}},

    {"returns", MACRO(do_returns), ARG_ANY, 0, TRUE,        /* macro */
    1,  {ARG_ANY}},

    {"right", MACRO(do_right), ARG_INT, 0, 0,               /* buffer, window */
    2,  {ARG_OPT | ARG_INT, ARG_OPT | ARG_INT}},

    {"rindex", MACRO(do_rindex), ARG_INT, 0, 0,             /* string */
    2,  {ARG_STRING, ARG_INT | ARG_STRING}},

    {"rmdir", MACRO(do_rmdir), ARG_INT, 0, 0,               /* file */
    1,  {ARG_STRING}},

    {"rtrim", MACRO(do_rtrim), ARG_STRING, 0, 0,            /* string */
    2,  {ARG_STRING, ARG_OPT | ARG_STRING}},

    {"save_keystroke_macro", MACRO(do_save_keystroke_macro), ARG_INT, 0, 0, /* kbd */
    1,  {ARG_STRING}},

    {"save_position", MACRO(do_save_position), ARG_VOID, 0, 0, /* movement, macro */
    0,  {0}},

    {"screen_dump", MACRO(do_screen_dump), ARG_INT, 0, 0,   /* screen */
    2,  {ARG_OPT | ARG_STRING, ARG_OPT | ARG_STRING}},

    {"search_back", MACRO(do_search_buf), ARG_INT, 0, FALSE,  /* movement, search */
    5,  {ARG_OPT | ARG_STRING, ARG_OPT | ARG_INT, ARG_OPT | ARG_INT, ARG_OPT | ARG_INT, ARG_OPT | ARG_INT}},

    {"search_case", MACRO(do_search_case), ARG_INT, 0, 0,   /* search */
    1,  {ARG_OPT | ARG_INT}},

    {"search_fwd", MACRO(do_search_buf), ARG_INT, 0, TRUE,  /* movement, search */
    5,  {ARG_OPT | ARG_STRING, ARG_OPT | ARG_INT, ARG_OPT | ARG_INT, ARG_OPT | ARG_INT, ARG_OPT | ARG_INT}},

    {"search_list", MACRO(do_search_list), ARG_INT, 0, 0,   /* search, list */
    5,  {ARG_OPT | ARG_INT, ARG_STRING, ARG_LIST, ARG_OPT | ARG_INT, ARG_OPT | ARG_INT}},

    {"search_string", MACRO(do_search_string), ARG_INT, 0, 0, /* search, string */
    5,  {ARG_OPT | ARG_STRING,
                   ARG_STRING,
         ARG_OPT | ARG_LVAL | ARG_INT,
         ARG_OPT | ARG_INT,
         ARG_OPT | ARG_INT}},

#if defined(VERSION_204)
    {"searchpath", MACRO(do_searchpath), ARG_INT, 0, 0,     /* file */
    6,  {ARG_STRING, ARG_OPT | ARG_STRING, ARG_OPT | ARG_STRING, ARG_LVAL | ARG_STRING,
                ARG_OPT | ARG_INT, ARG_OPT | ARG_INT}},
#endif

    {"self_insert", MACRO(do_self_insert), ARG_VOID, 0, 0,  /* kbd, buffer, window */
    1,  {ARG_OPT | ARG_INT}},

    {"send_signal", MACRO(do_send_signal), ARG_VOID, 0, 0,  /* proc */
    1,  {ARG_INT}},

    {"set_attribute", MACRO(do_set_attribute), ARG_INT, 0, 0, /* buffer */
    3,  {ARG_OPT | ARG_INT | ARG_STRING, ARG_OPT | ARG_INT, ARG_OPT | ARG_INT}},

    {"set_backup", MACRO(do_set_backup), ARG_INT, 0, 0,     /* env */
    2,  {ARG_OPT | ARG_INT, ARG_OPT | ARG_INT}},

    {"set_backup_option", MACRO(do_set_backup_option), ARG_INT, 0, 0, /* env */
    3,  {ARG_INT, ARG_OPT | ARG_INT, ARG_OPT | ARG_INT | ARG_STRING}},

    {"set_binary_size", MACRO(do_set_binary_size), ARG_INT, 0, 0, /* buffer, file */
    1,  {ARG_INT}},

#if defined(VERSION_204)
    {"set_btn2_action", MACRO(do_set_btn2_action), ARG_INT, 0, 0, /* macro */
    1,  {ARG_INT}},
#endif

    {"set_buffer", MACRO(do_set_buffer), ARG_INT, 0, 0,     /* buffer */
    1,  {ARG_INT}},

    {"set_buffer_cmap", MACRO(do_set_buffer_cmap), ARG_INT, 0, 0, /* buffer, env, window */
    2,  {ARG_OPT | ARG_INT | ARG_STRING, ARG_OPT | ARG_INT}},

    {"set_buffer_flags", MACRO(do_set_buffer_flags), ARG_VOID, 0, 0, /* buffer */
    4,  {ARG_OPT | ARG_INT, ARG_OPT | ARG_INT | ARG_STRING, ARG_OPT | ARG_INT | ARG_STRING, ARG_OPT | ARG_INT}},

    {"set_buffer_title", MACRO(do_set_buffer_title), ARG_INT, 0, 0, /* buffer */
    2,  {ARG_OPT | ARG_INT, ARG_OPT | ARG_STRING}},

    {"set_buffer_type", MACRO(do_set_buffer_type), ARG_INT, 0, 0, /* buffer */
    2,  {ARG_OPT | ARG_INT, ARG_INT}},

    {"set_calling_name", MACRO(do_set_calling_name), ARG_VOID, 0, 0, /* macro */
    1,  {ARG_OPT | ARG_STRING}},

    {"set_char_timeout", MACRO(do_set_char_timeout), ARG_INT, 0, 0, /* screen, env */
    1,  {ARG_OPT | ARG_STRING}},

    {"set_color", MACRO(do_set_color), ARG_INT, 0, 0,       /* screen */
    2,  {ARG_OPT | ARG_STRING | ARG_LIST, ARG_OPT | ARG_INT}},

    {"set_color_pair", MACRO(do_set_color_pair), ARG_VOID, 0, 0, /* screen, syntax */
    4,  {ARG_INT | ARG_STRING,
         ARG_OPT | ARG_INT | ARG_STRING,
         ARG_OPT | ARG_INT | ARG_STRING,
         ARG_OPT | ARG_INT | ARG_STRING}},

    {"set_ctrl_state", MACRO(do_set_ctrl_state), ARG_INT, 0, 0, /* window */
    3,  {ARG_INT, ARG_INT, ARG_OPT | ARG_INT}},

    {"set_echo_format", MACRO(do_set_echo_format), ARG_VOID, 0, 0, /* screen */
    1,  {ARG_OPT | ARG_STRING}},

    {"set_encoding", MACRO(do_set_encoding), ARG_INT, 0, 0, /* buffer */
    2,  {ARG_OPT | ARG_STRING, ARG_OPT | ARG_INT}},

    {"set_feature", MACRO(do_set_feature), ARG_INT, 0, 0,   /* screen, window */
    2,  {ARG_OPT | ARG_STRING, ARG_OPT | ARG_STRING}},

    {"set_file_magic", MACRO(do_set_file_magic), ARG_INT, 0, 0, /* file */
    2,  {ARG_OPT | ARG_STRING, ARG_OPT | ARG_INT}},

    {"set_font", MACRO(do_set_font), ARG_INT, 0, 0,         /* screen, window */
    2,  {ARG_OPT | ARG_STRING, ARG_OPT | ARG_STRING}},

    {"set_idle_default", MACRO(do_set_idle_default), ARG_INT, 0, 0, /* env */
    1,  {ARG_INT}},

    {"set_indent", MACRO(do_set_indent), ARG_INT, 0, 0,     /* buffer */
    1,  {ARG_OPT | ARG_INT}},

    {"set_kbd_name", MACRO(do_set_kbd_name), ARG_VOID, 0, 0, /* kbd */
    2,  {ARG_STRING, ARG_OPT | ARG_INT}},

    {"set_line_flags", MACRO(do_set_line_flags), ARG_INT, 0, 0, /* buffer */
    5,  {ARG_OPT | ARG_INT,
         ARG_OPT | ARG_INT,
         ARG_OPT | ARG_INT,
         ARG_INT,
         ARG_INT}},

    {"set_macro_history", MACRO(do_set_macro_history), ARG_STRING, 0, 0, /* macro */
    2,  {ARG_OPT | ARG_INT, ARG_OPT | ARG_STRING}},

    {"set_margins", MACRO(do_set_margins), ARG_INT, 0, 0,   /* buffer */
    5,  {ARG_OPT | ARG_INT, ARG_OPT | ARG_INT, ARG_OPT | ARG_INT, ARG_OPT | ARG_INT, ARG_OPT | ARG_INT}},

#if defined(VERSION_204)
    {"set_mouse_action", MACRO(do_set_mouse_action), ARG_STRING, 0, 0, /* kbd */
    1, {ARG_STRING}},
#endif

#if defined(VERSION_204)
    {"set_mouse_type", MACRO(do_set_mouse_type), ARG_INT, 0, 0, /* kbd */
    2, {ARG_INT, ARG_OPT | ARG_INT}},
#endif

    {"set_msg_level", MACRO(do_set_msg_level), ARG_INT, 0, 0, /* misc */
    1,  {ARG_INT}},

    {"set_process_position", MACRO(set_process_position), ARG_INT, 0, 0, /* proc, buffer */
    2,  {ARG_OPT | ARG_INT, ARG_OPT | ARG_INT}},

    {"set_property", MACRO(do_set_property/*obj_id, key, value*/), ARG_INT, 0, 0, /* macro */
    3,  {ARG_INT, ARG_STRING, ARG_ANY}},

    {"set_ruler", MACRO(do_set_ruler), ARG_INT, 0, 0,       /* buffer */
    2,  {ARG_OPT | ARG_INT, ARG_OPT | ARG_STRING | ARG_LIST}},

    {"set_scrap_info", MACRO(do_set_scrap_info), ARG_INT, 0, 0, /* scrap */
    3,  {ARG_OPT | ARG_INT, ARG_OPT | ARG_INT, ARG_OPT | ARG_INT}},

    {"set_syntax_flags", MACRO(do_set_syntax_flags), ARG_INT, 0, 0, /* syntax */
    2,  {ARG_INT, ARG_OPT | ARG_INT | ARG_STRING}},

    {"set_tab", MACRO(do_set_tab), ARG_INT, 0, 0,           /* buffer */
    2,  {ARG_OPT | ARG_INT, ARG_OPT | ARG_INT}},

    {"set_term_characters", MACRO(do_set_term_characters), ARG_VOID, B_NOVALUE, 0, /* screen, env */
    1,  {ARG_REST}},

    {"set_term_feature", MACRO(do_set_term_feature), ARG_INT, 0, 0, /* screen, env */
    2,  {ARG_INT | ARG_STRING, ARG_OPT | ARG_ANY}},

    {"set_term_features", MACRO(do_set_term_features), ARG_VOID, B_NOVALUE, 0, /* screen, env */
    1,  {ARG_REST}},

    {"set_term_keyboard", MACRO(do_set_term_keyboard), ARG_VOID, B_NOVALUE, 0, /* screen, env */
    1,  {ARG_REST}},

    {"set_terminator", MACRO(do_set_terminator), ARG_INT, 0, 0, /* buffer */
    2,  {ARG_OPT | ARG_INT, ARG_OPT | ARG_INT | ARG_STRING}},

    {"set_top_left", MACRO(do_set_top_left), ARG_INT, 0, 0, /* movement, buffer, window */
    6,  {ARG_OPT | ARG_INT,
         ARG_OPT | ARG_INT,
         ARG_OPT | ARG_INT,
         ARG_OPT | ARG_INT,
         ARG_OPT | ARG_INT,
         ARG_OPT | ARG_INT}},

    {"set_window", MACRO(do_set_window), ARG_INT, 0, 0,     /* window */
    1,  {ARG_INT}},

    {"set_window_cmap", MACRO(do_set_window_cmap), ARG_INT, 0, 0, /* buffer, env, window */
    2,  {ARG_OPT | ARG_INT | ARG_STRING, ARG_OPT | ARG_INT}},

    {"set_window_flags", MACRO(do_set_window_flags), ARG_VOID, 0, 0, /* window */
    3,  {ARG_OPT | ARG_INT, ARG_OPT | ARG_INT | ARG_STRING, ARG_OPT | ARG_INT | ARG_STRING}},

    {"set_window_priority", MACRO(do_set_window_priority), ARG_INT, 0, 0, /* screen */
    2,  {ARG_INT, ARG_OPT | ARG_INT}},

    {"set_wm_name", MACRO(do_set_wm_name), ARG_INT, 0, 0,   /* screen, env, window */
    2,  {ARG_OPT | ARG_STRING, ARG_OPT | ARG_STRING}},

    {"shell", MACRO(do_shell), ARG_INT, 0, 0,               /* env, proc */
    8,  {ARG_OPT | ARG_STRING,      /* command  */
         ARG_OPT | ARG_INT,         /* redraw   */
         ARG_OPT | ARG_STRING,      /* macro    */
         ARG_OPT | ARG_STRING,      /* stdin    */
         ARG_OPT | ARG_STRING,      /* stdout   */
         ARG_OPT | ARG_STRING,      /* stderr   */
         ARG_OPT | ARG_INT,         /* mode     */
         ARG_OPT | ARG_STRING}},    /* spec     */

    {"shift", MACRO(do_shift), ARG_ANY, 0, 0,               /* list */
    1,  {ARG_LVAL | ARG_LIST}},

    {"sin", MACRO(do_sin), ARG_FLOAT, 0, 0,                 /* float, arith */
    1,  {ARG_FLOAT}},

    {"sinh", MACRO(do_sinh), ARG_FLOAT, 0, 0,               /* float, arith */
    1,  {ARG_FLOAT}},

    {"sleep", MACRO(do_sleep), ARG_VOID, 0, 0,              /* proc */
    2,  {ARG_OPT | ARG_INT, ARG_OPT | ARG_INT}},

    {"sort_buffer", MACRO(do_sort_buffer), ARG_INT, 0, 0,   /* buffer */
    5,  {ARG_OPT | ARG_INT,
         ARG_OPT | ARG_INT | ARG_STRING,
         ARG_OPT | ARG_INT,
         ARG_OPT | ARG_INT,
         ARG_OPT | ARG_INT}},

    {"sort_list", MACRO(do_sort_list), ARG_LIST, 0, 0,      /* list */
    3,  {ARG_OPT | ARG_LIST, ARG_OPT | ARG_INT | ARG_STRING, ARG_OPT | ARG_INT}},

    {"spell_buffer", MACRO(do_spell_check), ARG_UNDEF, 0, 1, /* spell */
    5,  {ARG_OPT | ARG_INT, ARG_OPT | ARG_INT, ARG_OPT | ARG_INT, ARG_OPT | ARG_INT, ARG_OPT | ARG_INT}},

    {"spell_control", MACRO(do_spell_control), ARG_UNDEF, 0, 0, /* spell */
    2,  {ARG_INT, ARG_OPT | ARG_INT | ARG_STRING}},

    {"spell_distance", MACRO(do_spell_distance), ARG_UNDEF, 0, 0, /* spell */
    2,  {ARG_STRING, ARG_STRING}},

#if defined(VERSION_207)
    {"spell_dictionary", MACRO(do_spell_dictionary), ARG_UNDEF, 0, 0, /* spell */
    -3, {ARG_INT, ARG_OPT|ARG_INT, ARG_STRING|ARG_LIST}},
#endif

    {"spell_string", MACRO(do_spell_check), ARG_UNDEF, 0, 2, /* spell */
    4,  {ARG_STRING, ARG_OPT | ARG_INT, ARG_OPT | ARG_INT, ARG_OPT | ARG_INT}},

    {"spell_suggest", MACRO(do_spell_suggest), ARG_UNDEF, 0, 0, /* spell */
    1,  {ARG_STRING}},

    {"splice", MACRO(do_splice), ARG_VOID, 0, 0,            /* list */
    -4, {ARG_LVAL | ARG_LIST, ARG_INT, ARG_OPT | ARG_INT, ARG_OPT | ARG_ANY}},

    {"split", MACRO(do_split), ARG_LIST, 0, 0,              /* string */
    5,  {ARG_STRING, ARG_INT | ARG_STRING, ARG_OPT | ARG_INT, ARG_OPT | ARG_INT, ARG_OPT | ARG_INT}},

    {"split_arguments", MACRO(do_split_arguments), ARG_LIST, 0, 0, /* string */
    1,  {ARG_OPT | ARG_STRING}},

    {"sprintf", MACRO(do_sprintf), ARG_INT, 0, 0,           /* string */
    -3, {ARG_LVAL | ARG_STRING,
         ARG_STRING,
         ARG_OPT | ARG_ANY}},

    {"sqrt", MACRO(do_sqrt), ARG_FLOAT, 0, 0,               /* float, arith */
    1,  {ARG_FLOAT}},

    {"srand", MACRO(do_srand), ARG_INT, 0, 0,               /* misc */
    2,  {ARG_OPT | ARG_INT, ARG_OPT | ARG_INT}},

    {"sscanf", MACRO(do_sscanf), ARG_INT, 0, 0,             /* string */
    -3, {ARG_STRING, ARG_STRING, ARG_OPT | ARG_LVAL | ARG_ANY}},

    {"stat", MACRO(do_stat), ARG_INT, 0, 0,                 /* file */
    11, {ARG_OPT | ARG_STRING,
         ARG_OPT | ARG_LVAL | ARG_INT,
         ARG_OPT | ARG_LVAL | ARG_INT,
         ARG_OPT | ARG_LVAL | ARG_INT,
         ARG_OPT | ARG_LVAL | ARG_INT,
         ARG_OPT | ARG_LVAL | ARG_INT,
         ARG_OPT | ARG_LVAL | ARG_INT,
         ARG_OPT | ARG_LVAL | ARG_STRING,
         ARG_OPT | ARG_LVAL | ARG_INT,
         ARG_OPT | ARG_LVAL | ARG_STRING,
         ARG_OPT | ARG_LVAL | ARG_INT}},

    {"static", MACRO(do_static), ARG_VOID, B_NOVALUE, 0,    /* var */
    1,  {ARG_REST}},

    {"strcasecmp", MACRO(do_strcasecmp), ARG_INT, 0, 0,     /* string */
    3,  {ARG_STRING, ARG_STRING, ARG_OPT | ARG_INT}},

    {"strcasestr", MACRO(do_strcasestr), ARG_INT, 0, 0,     /* string */
    2,  {ARG_STRING, ARG_STRING}},

    {"strcmp", MACRO(do_strcmp), ARG_INT, 0, 0,             /* string */
    3,  {ARG_STRING, ARG_STRING, ARG_OPT | ARG_INT}},

    {"strerror", MACRO(do_strerror), ARG_STRING, 0, 0,      /* misc, proc */
    3,  {ARG_OPT | ARG_INT, ARG_OPT | ARG_LVAL | ARG_STRING, ARG_OPT | ARG_INT}},

    {"strfilecmp", MACRO(do_strfilecmp), ARG_INT, 0, 0,     /* string */
    3,  {ARG_STRING, ARG_STRING, ARG_OPT | ARG_INT}},

    {"strftime", MACRO(do_strftime), ARG_STRING, 0, 0,      /* string */
    2,  {ARG_OPT | ARG_STRING, ARG_OPT | ARG_INT}},

    {"string", MACRO(do_declare), ARG_VOID, B_NOVALUE, F_STR, /* var */
    1,  {ARG_REST}},

    {"string_count", MACRO(do_string_count), ARG_INT, 0, 0, /* string */
    2,  {ARG_STRING, ARG_INT | ARG_STRING}},

    {"strlen", MACRO(do_strlen), ARG_INT, 0, 0,             /* string, list */
    2,  {ARG_STRING | ARG_LIST, ARG_OPT | ARG_INT}},

    {"strnlen", MACRO(do_strnlen), ARG_INT, 0, 0,           /* string, list */
    3,  {ARG_STRING | ARG_LIST, ARG_INT, ARG_OPT | ARG_INT}},

    {"strpbrk", MACRO(do_strpbrk), ARG_INT, 0, 0,           /* string */
    2,  {ARG_STRING, ARG_STRING}},

    {"strpop", MACRO(do_strpop), ARG_STRING, 0, 0,          /* string */
    2,  {ARG_STRING | ARG_LVAL, ARG_OPT | ARG_INT}},

    {"strrstr", MACRO(do_strrstr), ARG_INT, 0, 0,           /* string */
    2,  {ARG_STRING, ARG_STRING}},

    {"strsignal", MACRO(do_strsignal), ARG_INT, 0, 0,       /* misc, proc */
    3,  {ARG_OPT | ARG_INT, ARG_OPT | ARG_LVAL | ARG_STRING, ARG_OPT | ARG_INT}},

    {"strstr", MACRO(do_strstr), ARG_INT, 0, 0,             /* string */
    2,  {ARG_STRING, ARG_STRING}},

    {"strtod", MACRO(do_strtod), ARG_FLOAT, 0, 0,           /* string */
    2,  {ARG_STRING, ARG_OPT | ARG_LVAL | ARG_INT}},

    {"strtof", MACRO(do_strtof), ARG_FLOAT, 0, 0,           /* string */
    2,  {ARG_STRING, ARG_OPT | ARG_LVAL | ARG_INT}},

    {"strtol", MACRO(do_strtol), ARG_INT, 0, 0,             /* string */
    3,  {ARG_STRING, ARG_OPT | ARG_LVAL | ARG_INT, ARG_OPT | ARG_INT}},

    {"strverscmp", MACRO(do_strverscmp), ARG_INT, 0, 0,     /* string */
    2,  {ARG_STRING, ARG_STRING}},

    {"substr", MACRO(do_substr), ARG_STRING, 0, 0,          /* string */
    3,  {ARG_STRING, ARG_INT, ARG_OPT | ARG_INT}},

    {"suspend", MACRO(do_suspend), ARG_INT, 0, 0,           /* proc */
    0,  {0}},

    {"swap_anchor", MACRO(do_swap_anchor), ARG_INT, 0, 0,   /* scrap, movement */
    0,  {0}},

    {"switch", MACRO(do_switch), ARG_VOID, 0, 0,            /* macro */
    2,  {ARG_NUM | ARG_STRING, ARG_REST}},

#if defined(VERSION_201)
    {"symlink", MACRO(do_symlink), ARG_VOID, 0, 0,          /* file */
    2,  {ARG_STRING, ARG_STRING}},
#endif

    {"syntax_build", MACRO(do_syntax_build), ARG_INT, 0, 0, /* syntax */
    3,  {ARG_OPT | ARG_INT, ARG_OPT | ARG_STRING, ARG_OPT | ARG_INT | ARG_STRING}},

    {"syntax_column_ruler", MACRO(do_syntax_column_ruler), ARG_INT, 0, 0, /* syntax */
    3,  {ARG_LIST | ARG_OPT, ARG_OPT | ARG_STRING, ARG_OPT | ARG_INT | ARG_STRING}},

    {"syntax_rule", MACRO(do_syntax_rule), ARG_VOID, 0, 0,  /* syntax */
    3,  {ARG_STRING, ARG_STRING, ARG_OPT | ARG_INT | ARG_STRING}},

    {"syntax_token", MACRO(do_syntax_token), ARG_VOID, 0, 0, /* syntax */
    4,  {ARG_INT | ARG_STRING, ARG_OPT | ARG_INT | ARG_STRING, ARG_OPT | ARG_INT | ARG_STRING, ARG_OPT | ARG_INT | ARG_STRING}},

    {"tabs", MACRO(do_tabs), ARG_INT, 0, 0,                 /* buffer */
    9,  {ARG_OPT | ARG_INT | ARG_STRING | ARG_LIST,
         ARG_OPT | ARG_INT, ARG_OPT | ARG_INT,
         ARG_OPT | ARG_INT, ARG_OPT | ARG_INT,
         ARG_OPT | ARG_INT, ARG_OPT | ARG_INT,
         ARG_OPT | ARG_INT, ARG_OPT | ARG_INT}},

    {"tagdb_close", MACRO(do_tagdb_close), ARG_INT, 0, 0,   /* buffer */
    1,  {ARG_NUM}},

    {"tagdb_open", MACRO(do_tagdb_open), ARG_INT, 0, 0,     /* buffer */
    3,  {ARG_STRING, ARG_OPT | ARG_INT, ARG_OPT | ARG_INT}},

    {"tagdb_search", MACRO(do_tagdb_search), ARG_INT, 0, 0, /* buffer */
    3,  {ARG_NUM, ARG_STRING, ARG_OPT | ARG_INT}},

    {"tan", MACRO(do_tan), ARG_FLOAT, 0, 0,                 /* float, arith */
    1,  {ARG_FLOAT}},

    {"tanh", MACRO(do_tanh), ARG_FLOAT, 0, 0,               /* float, arith */
    1,  {ARG_FLOAT}},

    {"throw", MACRO(do_throw), ARG_VOID, 0, 0,              /* macro */
    1,  {ARG_INT | ARG_STRING}},

    {"time", MACRO(do_time), ARG_INT, 0, 0,                 /* misc */
    4,  {ARG_OPT | ARG_LVAL | ARG_INT,
         ARG_OPT | ARG_LVAL | ARG_INT,
         ARG_OPT | ARG_LVAL | ARG_INT,
         ARG_OPT | ARG_LVAL | ARG_INT}},

    {"tokenize", MACRO(do_tokenize), ARG_LIST, 0, 0,        /* string */
    3,  {ARG_STRING, ARG_INT | ARG_STRING, ARG_OPT | ARG_INT}},

    {"top_of_buffer", MACRO(do_top_of_buffer), ARG_INT, 0, 0, /* movement */
    0,  {0}},

    {"top_of_window", MACRO(do_top_of_window), ARG_INT, 0, 0, /* movement */
    0,  {0}},

    {"transfer", MACRO(do_transfer), ARG_INT, 0, 0,         /* scrap */
    5,  {ARG_INT, ARG_INT, ARG_INT, ARG_OPT | ARG_INT, ARG_OPT | ARG_INT}},

    {"translate", MACRO(do_translate), ARG_INT, 0, 0,       /* search */
    7,  {ARG_OPT | ARG_STRING,
         ARG_OPT | ARG_STRING,
         ARG_OPT | ARG_INT,
         ARG_OPT | ARG_INT,
         ARG_OPT | ARG_INT,
         ARG_OPT | ARG_INT,
         ARG_OPT | ARG_INT}},

    {"translate_pos", MACRO(do_translate_pos), ARG_INT, 0, 0, /* kbd, window */
    5,  {ARG_INT,
         ARG_INT,
         ARG_OPT | ARG_LVAL | ARG_INT,
         ARG_OPT | ARG_LVAL | ARG_INT,
         ARG_OPT | ARG_LVAL | ARG_INT}},

    {"trim", MACRO(do_trim), ARG_STRING, 0, 0,              /* string */
    2,  {ARG_STRING, ARG_OPT | ARG_STRING}},

    {"try", MACRO(do_try), ARG_INT, 0, 0,                   /* macro */
    3,  {ARG_OPT | ARG_COND, ARG_OPT | ARG_COND, ARG_OPT | ARG_COND}},

    {"typeof", MACRO(do_typeof), ARG_STRING, 0, 0,          /* var */
    1,  {ARG_OPT | ARG_ANY}},

    {"umask", MACRO(do_umask), ARG_INT, 0, 0,               /* file */
    1,  {ARG_OPT | ARG_INT}},

    {"uname", MACRO(do_uname), ARG_INT, 0, 0,               /* env */
    5,  {ARG_OPT | ARG_LVAL | ARG_STRING,
         ARG_OPT | ARG_LVAL | ARG_STRING,
         ARG_OPT | ARG_LVAL | ARG_STRING,
         ARG_OPT | ARG_LVAL | ARG_STRING,
         ARG_OPT | ARG_LVAL | ARG_STRING}},

    {"undo", MACRO(do_undo), ARG_INT, 0, -1,                /* buffer, kbd */
    3,  {ARG_OPT | ARG_INT, ARG_OPT | ARG_INT, ARG_OPT | ARG_INT}},

#if defined(VERSION_207)
    {"unlink", MACRO(do_unlink), ARG_INT, 0, 0,             /* file */
    2,  {ARG_STR, ARG_OPT | ARG_INT}},
#endif

    {"unregister_macro", MACRO(do_unregister_macro), ARG_INT, 0, 0, /* macro */
    3,  {ARG_INT, ARG_STRING, ARG_OPT | ARG_INT}},

    {"unshift", MACRO(do_unshift), ARG_INT, 0, 0,           /* list */
    -2, {ARG_LVAL | ARG_LIST, ARG_OPT | ARG_ANY}},

    {"up", MACRO(do_up), ARG_INT, 0, 0,                     /* buffer, window */
    1,  {ARG_OPT | ARG_INT}},

    {"upper", MACRO(do_upper), ARG_STRING, 0, 0,            /* string */
    1,  {ARG_INT | ARG_STRING}},

    {"use_local_keyboard", MACRO(do_use_local_keyboard), ARG_INT, 0, 0, /* kbd */
    1,  {ARG_INT}},

    {"use_tab_char", MACRO(do_use_tab_char), ARG_INT, 0, 0, /* env, screen */
    2,  {ARG_OPT | ARG_INT | ARG_STRING,
         ARG_OPT | ARG_INT}},

    {"version", MACRO(do_version), ARG_INT, 0, 0,           /* misc */
    9,  {ARG_OPT | ARG_LVAL | ARG_INT | ARG_STRING,
         ARG_OPT | ARG_LVAL | ARG_INT,
         ARG_OPT | ARG_LVAL | ARG_INT,
         ARG_OPT | ARG_LVAL | ARG_INT,
         ARG_OPT | ARG_LVAL | ARG_STRING,
         ARG_OPT | ARG_LVAL | ARG_STRING,
         ARG_OPT | ARG_LVAL | ARG_INT,
         ARG_OPT | ARG_LVAL | ARG_STRING,
         ARG_OPT | ARG_LVAL | ARG_STRING}},

    {"vfs_mount", MACRO(do_vfs_mount), ARG_INT, 0, 0,       /* file */
    4,  {ARG_STRING, ARG_OPT | ARG_INT, ARG_STRING, ARG_OPT | ARG_STRING}},

    {"vfs_unmount", MACRO(do_vfs_unmount), ARG_INT, 0, 0,   /* file */
    2,  {ARG_STRING,
         ARG_OPT | ARG_INT}},

    {"view_screen", MACRO(do_view_screen), ARG_INT, 0, 0,   /* screen */
    0,  {0}},

    {"wait", MACRO(do_wait), ARG_INT, 0, 0,                 /* proc */
    1,  {ARG_OPT | ARG_LVAL | ARG_INT}},

    {"wait_for", MACRO(do_wait_for), ARG_INT, 0, 0,         /* proc */
    3,  {ARG_OPT | ARG_INT, ARG_OPT | ARG_STRING | ARG_LIST, ARG_OPT | ARG_INT}},

    {"watch", MACRO(do_unimp), ARG_VOID, 0, 0,              /* debug */
    0,  {0}},

#if defined(VERSION_206)
    {"wcharacterat", MACRO(do_wcharacterat), ARG_INT, 0, 0, /* string */
    2,  {ARG_STRING, ARG_INT}},

    {"wcwidth", MACRO(do_wcwidth), ARG_INT, 0, 0,           /* string */
    2,  {ARG_INT | ARG_STRING, ARG_OPT | ARG_INT}},

    {"wfirstof", MACRO(do_wfirstof), ARG_INT, 0, 0,         /* string */
    3,  {ARG_STRING, ARG_STRING, ARG_OPT | ARG_LVAL | ARG_INT}},

    {"windex", MACRO(do_windex), ARG_INT, 0, 0,             /* string */
    1,  {ARG_OPT | ARG_INT}},

    {"wlastof", MACRO(do_wlastof), ARG_INT, 0, 0,           /* string */
    3,  {ARG_STRING, ARG_STRING, ARG_OPT | ARG_LVAL | ARG_INT }},

    {"wlower", MACRO(do_wlower), ARG_STRING, 0, 0,          /* string */
    1,  {ARG_INT | ARG_STRING}},
#endif

    {"while", MACRO(do_while), ARG_VOID, 0, 0,              /* macro */
    2,  {ARG_COND, ARG_OPT | ARG_REST}},

    {"widget_get", MACRO(do_widget_get), ARG_ANY, 0, 0,     /* dialog */
    4,  {ARG_OPT | ARG_INT, ARG_INT | ARG_STRING, ARG_OPT | ARG_INT, ARG_OPT | ARG_INT}},

    {"widget_set", MACRO(do_widget_set), ARG_ANY, 0, 0,     /* dialog */
    5,  {ARG_OPT | ARG_INT,
         ARG_INT | ARG_STRING,
         ARG_OPT | ARG_INT | ARG_STRING | ARG_LIST,
         ARG_OPT | ARG_INT,
         ARG_OPT | ARG_INT}},

    {"window_color", MACRO(do_window_color), ARG_INT, 0, 0, /* window, screen */
    2,  {ARG_OPT | ARG_INT, ARG_OPT | ARG_INT}},

    {"write_block", MACRO(do_write_block), ARG_INT, 0, 0,   /* file, scrap */
    4,  {ARG_OPT | ARG_STRING, ARG_OPT | ARG_INT, ARG_OPT | ARG_INT, ARG_OPT | ARG_INT}},

#if defined(VERSION_206)
    {"wrindex", MACRO(do_wrindex), ARG_INT, 0, 0,           /* string */
    2,  {ARG_STRING, ARG_INT | ARG_STRING}},
#endif

    {"write_buffer", MACRO(do_write_buffer), ARG_INT, 0, 0, /* file, buffer */
    2,  {ARG_OPT | ARG_STRING, ARG_OPT | ARG_INT}},

    {"write_buffer", MACRO(do_write_buffer), ARG_INT, 0, 0, /* file, buffer */
    2,  {ARG_OPT | ARG_STRING, ARG_OPT | ARG_INT}},

#if defined(VERSION_206)
    {"wstrcasecmp", MACRO(do_wstrcasecmp), ARG_INT, 0, 0,   /* string */
    3,  {ARG_STRING, ARG_STRING, ARG_OPT | ARG_INT}},

    {"wstrcmp",  MACRO(do_wstrcmp), ARG_INT, 0, 0,          /* string */
    3,  {ARG_STRING, ARG_STRING, ARG_OPT | ARG_INT}},

    {"wstrlen", MACRO(do_wstrlen), ARG_INT, 0, 0,           /* string */
    2,  {ARG_STRING | ARG_LIST, ARG_OPT | ARG_INT}},

    {"wstrnlen", MACRO(do_wstrnlen), ARG_INT, 0, 0,         /* string */
    3,  {ARG_STRING | ARG_LIST, ARG_INT, ARG_OPT | ARG_INT}},

    {"wstrpbrk", MACRO(do_wstrpbrk), ARG_INT, 0, 0,         /* string */
    2,  {ARG_STRING, ARG_STRING}},

    {"wstrrstr", MACRO(do_wstrrstr), ARG_INT, 0, 0,         /* string */
    2,  {ARG_STRING, ARG_STRING}},

    {"wstrstr", MACRO(do_wstrstr), ARG_INT, 0, 0,           /* string */
    2,  {ARG_STRING, ARG_STRING}},

    {"wsubstr", MACRO(do_wsubstr), ARG_STRING, 0, 0,        /* string */
    3,  {ARG_STRING, ARG_INT, ARG_OPT | ARG_INT}},

    {"wupper", MACRO(do_wupper), ARG_STRING, 0, 0,          /* string */
    1,  {ARG_INT | ARG_STRING}},
#endif

    {"|", MACRO(do_com_op), ARG_UNDEF, 0, MOP_BOR,          /* arith */
    2,  {ARG_INT, ARG_INT}},

    {"|=", MACRO(do_com_equ), ARG_UNDEF, 0, MOP_BOR,        /* arith */
    2,  {ARG_LVAL | ARG_INT, ARG_INT}},

    {"||", MACRO(do_oror), ARG_UNDEF, 0, 0,                 /* arith */
    2,  {ARG_INT, ARG_COND}},

    {"~", MACRO(do_com_op), ARG_UNDEF, 0, MOP_BNOT,         /* arith */
    1,  {ARG_INT}}
};


const unsigned builtin_count = (sizeof(builtin)/sizeof(BUILTIN));
uint32_t builtin_signature = 0;


static int
b_compare(void const *key, void const *b2)
{
    const BUILTIN *x = b2;
    return strcmp(key, x->b_name);
}


static int
b_sort(void const *k1, void const *k2)
{
    const BUILTIN *b1 = k1, *b2 = k2;
    return strcmp(b1->b_name, b2->b_name);
}


void
builtin_init(void)
{
    BUILTIN *bp;
    unsigned i;

    /* Runtime checks */
#if !defined(__NOFUNCTIONS__)
    trace_log("keywd: count = %u\n", builtin_count);
#endif

    assert(MAX_BUILTIN_ARGS <= MAX_ARGC);

    for (i = 0, bp = builtin; i < builtin_count; ++i, ++bp) {
        assert(0 == bp->b_magic);
        bp->b_magic = BUILTIN_MAGIC;            /* structure magic */

        if (bp->b_arg_count < 0) {              /* variable argument count primer */
            bp->b_arg_count = -bp->b_arg_count;
            bp->b_flags |= B_VARARGS;
        }

#if !defined(NDEBUG) 
        {   const int argc = bp->b_arg_count;
            int iarg;
      
            assert(argc <= MAX_BUILTIN_ARGS);
            for (iarg = 0; iarg < argc; ++iarg) {
                assert(bp->b_arg_types[iarg]);
            }
            assert(0 == bp->b_arg_types[argc]); /* terminator */
        }
#endif  //NDEBUG

#if !defined(__NOFUNCTIONS__)
        if (i && b_sort(bp - 1, bp) > 0)  {
            trace_log("\t'%s' and '%s' are not ordered\n", bp[-1].b_name, bp->b_name);
        }
#endif
    }

    /* Sort and build signature */
    qsort(builtin, (size_t) builtin_count, sizeof(BUILTIN), b_sort);
    builtin_signature = (uint32_t)-1;
    for (i = 0, bp = builtin; i < builtin_count; ++i, ++bp) {
        builtin_signature =
            crc32_EDB88320(bp->b_name, strlen(bp->b_name), builtin_signature);
    }
}


BUILTIN *
builtin_lookup(const char *str)
{
    return (BUILTIN *) bsearch(str, builtin,
                (size_t) builtin_count, sizeof(BUILTIN), b_compare);
}


/*
 *  Return index of a builtin primitive -- used by crunch
 */
int
builtin_index(const char *str)
{
    BUILTIN *bp = builtin_lookup(str);
    if (bp) {
        return bp - builtin;
    }
    return -1;
}

/*end*/
