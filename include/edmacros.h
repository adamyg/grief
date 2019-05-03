#ifndef GR_EDMACROS_H_INCLUDED
#define GR_EDMACROS_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_edmacros_h,"$Id: edmacros.h,v 1.18 2019/03/15 23:03:08 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: edmacros.h,v 1.18 2019/03/15 23:03:08 cvsuser Exp $
 * Macro and symbolic interpreter information.
 *
 * Copyright (c) 1998 - 2019, Adam Young.
 * All rights reserved.
 *
 * This file is part of the GRIEF Editor.
 *
 * The GRIEF Editor is free software: you can redistribute it
 * and/or modify it under the terms of the GRIEF Editor License.
 *
 * Redistributions of source code must retain the above copyright
 * notice, and must be distributed with the license document above.
 *
 * Redistributions in binary form must reproduce the above copyright
 * notice, and must include the license document above in
 * the documentation and/or other materials provided with the
 * distribution.
 *
 * The GRIEF Editor is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * License for more details.
 * ==end==
 */

#include <edtypes.h>
#include <edopcode.h>

__CBEGIN_DECLS

#define MAX_ARGC            16                  /* primitive fixed argument limit */
#define MAX_MACROS          512                 /* object macro limit */
#define MAX_NESTING         256                 /* macro run-time exectuion nesting limit */
#define MAX_CMDLINE         512                 /* prompt command line limit */

enum __macrotype {
/*
 *  Also see trace_triggger
 */
/*--export--enum--*/
/*
 *  Registered macro types
 */
    REG_TYPED       =0,                         /* Character typed */
    REG_EDIT        =1,                         /* Different file edited */
    REG_ALT_H       =2,                         /* ALT-H pressed in response to a prompt */
    REG_UNASSIGNED  =3,                         /* Unassigned key pressed */
    REG_IDLE        =4,                         /* Idle time expired */
    REG_EXIT        =5,                         /* About to exit */
    REG_NEW         =6,                         /* New file edited and readin */
    REG_CTRLC       =7,                         /* CTRL-C (SIGINT) pressed during macro */
    REG_INVALID     =8,                         /* Invalid key pressed during response input */
    REG_INTERNAL    =9,                         /* Internal error */
    REG_MOUSE       =10,                        /* Mouse callback */
    REG_PROC_INPUT  =11,                        /* Process input available */
    REG_KEYBOARD    =12,                        /* Keyboard buffer empty */

    REG_STARTUP,                                /* Startup complete */

    REG_BOOKMARK,                               /* Bookmark dropped/deleted. */
    REG_INSERT_MODE,                            /* Insert mode has changed. */

    REG_BUFFER_MOD,                             /* Buffer has been modified */
    REG_BUFFER_WRITE,                           /* Buffer write operation */
    REG_BUFFER_RENAME,                          /* Buffer rename operation */
    REG_BUFFER_DELETE,                          /* buffer delete operation */

    REG_FILE_SAVE,                              /* File write request */
    REG_FILE_WRITTEN,                           /* File write completion */
    REG_FILE_CHANGE,                            /* File external change */

    REG_SIGUSR1,                                /* User defined signal */
    REG_SIGUSR2,

    REG_UNDEFINED_MACRO,                        /* Undefined macro */
    REG_REGRESS,

    REG_ABORT,                                  /* abort() */

/*--end--*/
    REG_OMAX        =REG_KEYBOARD,
    REG_MAX         =REG_REGRESS
};


enum _sflag {       /*uint16_t*/
    SF_POLY         =0x01,                      /* Symbol is polymorphic */
    SF_CONSTANT     =0x02,                      /* Symbol is constant/read-only */
    SF_REFERENCE    =0x04,                      /* Symbol is a reference */
    SF_DYNAMIC      =0x08,                      /* Symbol is dyanmic */
    SF_SYSTEM       =0x10                       /* Symbol variable */
};


#define M_AUTOLOAD          0x01                /* Macro needs to be autoloaded on reference */
#define M_STATIC            0x02                /* Static scope */
#define M_OVERLOAD          0x04                /* Overloads a builtin */
#define M_UNRESOLVED        0x10


typedef struct MACRO {
    MAGIC_t         m_magic;
#define MACRO_MAGIC         MKMAGIC('M','M','a','c')
 /* const char *    m_macro; */                 /* Macro object name */
    const char *    m_module;                   /* Module name $xxxx */
    unsigned        m_ident;                    /* Identifier */
    const char *    m_name;                     /* Macro name */
    short           m_flags;                    /* Flags M_xxxx */
    short           m_ftime;                    /* First-time flag - TRUE/FALSE */
    const LIST *    m_list;                     /* Macro definition */
    int             m_size;                     /* Size of macro in atoms */
    struct MACRO *  m_next;                     /* Next macro in list for this file */
 /* unsigned        m_hits;  */                 /* Execution hits */
 /* TimeStat_t      m_usage; */                 /* Execution usage */
} MACRO;


/*
 *  Typedef for list of macro files. All this info is needed in case we
 *  delete the macro.
 */
typedef struct MACROF {
    char *          mf_name;
    char *          mf_list;
    MACRO *         mf_macro;
} MACROF;

#define MAX_BUILTIN_ARGS    12                  /* 15/02/10 was 8 */

/*
 *  The following flag definitions are used to define the
 *  argument types to the builtin primitives (2^16)
 *
 *  Warning:
 *      Changing the values of based types INT, FLOAT, STRING, LIST also
 *      requires changes to the conversions tables within builtin.c
 */
enum _argflags {
    /*arguments types (unsigned char, see BUILTIN) */
    ARG_INT         =0x0001,
    ARG_FLOAT       =0x0002,
    ARG_STRING      =0x0004,
    ARG_LIST        =0x0008,

    ARG_ARRAY       =0x0010,                    /* new, not fully implemented */

    ARG_COND        =0x0100,
    ARG_REST        =0x0200,                    /* ... */
    ARG_OPT         =0x0400,                    /* Optional (maybe NULL) */
    ARG_LVAL        =0x0800,                    /* Value reference */

    ARG_NUM         =(ARG_INT | ARG_FLOAT),
    ARG_ANY         =(ARG_NUM | ARG_STRING | ARG_LIST),

    /*return-types*/
    ARG_UNDEF       =0x1000,
    ARG_VOID        =0x2000,
    ARG_SYMBOL      =0x4000,
    ARG_NULL        =0x8000
};

typedef unsigned short argtype_t;               /* Argument type */


/*
 *  Built-in macro flags
 */
enum _biflags {
    B_REDEFINE      =0x0001,                    /* Builtin macro has been redefined */
    B_NOVALUE       =0x0002,                    /* Dont print accumulator result in debug mode */
    B_SAFE          =0x0004                     /* Macro doesn't/cannot modify parameters,
                                                 * ie. its safe to not take a copy of arguments */
};


/*
 *  Structure used to define built-in macros
 */
typedef struct BUILTIN {
    /*
     *  Static components
     */
    const char *    b_name;                     /* Function name */
    void          (*b_func)(int);               /* Implementation */
    argtype_t       b_rtntype;                  /* Return type */
    unsigned        b_flags;                    /* Flags (see above) */
    int             b_parameter;                /* Parameter passed to caller */
    int             b_argc;                     /* Argument count */
    argtype_t       b_arg_types[MAX_BUILTIN_ARGS];

    /*
     *  Dynamic components
     */
    MAGIC_t         b_magic;                    /* Structure magic */
#define BUILTIN_MAGIC       MKMAGIC('B','l','I','n')
    const LIST *    b_ovargv;                   /* Overload argument vector, saved on initial call */
    MACRO *         b_first_macro;              /* Overload/replacement chain */
    MACRO *         b_macro;                    /* Current macro being executed in recursive keyword execution */
    unsigned        b_reference;                /* Reference/execution count */
    unsigned        b_replacement;              /* Replace execution count */
    unsigned        b_profile;                  /* Execution profile */
} BUILTIN;


/*
 *  Macro stack structure
 */
struct mac_stack {
    const char *    module;                     /* $xxxx */
    const char *    name;                       /* Macro name */
    const char *    caller;                     /* set_calling_name() override */
    const LIST *    argv;                       /* Argument vector */
    int             argc;                       /* Argument count */
    int             level;                      /* Level where symbols are defined for debugger */
};

__CEND_DECLS

#endif  /*GR_EDMACROS_H_INCLUDED*/
