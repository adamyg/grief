#ifndef MACSRC_DEBUG_H_INCLUDED
#define MACSRC_DEBUG_H_INCLUDED
/* -*- mode: cr; indent-width: 4; -*- */
/*
 *  An auto-generated file, do not modify
 */

/*
 *  debug_support() object types
 */
#define DBG_STACK_TRACE         0
#define DBG_NEST_LEVEL          1           /* Level of nesting of execution stack */

#define DBG_INQ_VARS            2           /* Return list of variable values.  */
#define DBG_INQ_VAR_INFO        3           /* Return info about an actual variable. */

#define DBG_INQ_BVARS           4           /* Return list of buffer variable values. */
#define DBG_INQ_BVAR_INFO       5           /* Return info about an actual variable. */

#define DBG_INQ_MVARS           6           /* Return list of moduler variable values. */
#define DBG_INQ_MVAR_INFO       7           /* Return info about an actual variable. */

#define DBG_INQ_OPCODES         8           /* OPCODE descriptions */


/*
 *  Debug flags
 */
#define DB_TRACE                0x00000001  /* General trace */
#define DB_REGEXP               0x00000002  /* Regular expression debug output */
#define DB_UNDO                 0x00000004  /* Undo trace */
#define DB_FLUSH                0x00000008  /* Flush output */
#define DB_TIME                 0x00000010  /* Time stamp */
#define DB_TERMINAL             0x00000020  /* Terminal */
#define DB_VFS                  0x00000040  /* Virtual filesystem */
#define DB_NOTRAP               0x00000080  /* Disable SIGBUS/SIGSEGV trap handling */
#define DB_MEMORY               0x00001000  /* Target specific debug services */
#define DB_REFS                 0x00002000  /* Variable refs */
#define DB_PROMPT               0x00004000  /* Debug prompting code */
#define DB_PURIFY               0x00008000  /* Running under Purify(tm) */
#define DB_HISTORY              0x00010000

#endif  //MACSRC_DEBUG_H_INCLUDED
/*end*/
