#include <edidentifier.h>
__CIDENT_RCSID(gr_m_buf_c,"$Id: m_buf.c,v 1.53 2020/04/21 00:01:55 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: m_buf.c,v 1.53 2020/04/21 00:01:55 cvsuser Exp $
 * Buffer primitives.
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
#include <edalt.h>
#include <libstr.h>                             /* str_...()/sxprintf() */

#include "m_buf.h"                              /* public header */

#include "accum.h"                              /* acc_...() */
#include "anchor.h"                             /* anchor_...() */
#include "basic.h"
#include "border.h"
#include "buffer.h"
#include "builtin.h"
#include "color.h"
#include "debug.h"                              /* trace_...() */
#include "display.h"
#include "echo.h"
#include "eval.h"
#include "file.h"
#include "getkey.h"
#include "kill.h"
#include "lisp.h"
#include "m_color.h"                            /* color_attr */
#include "macros.h"
#include "main.h"
#include "map.h"
#include "mchar.h"                              /* mchar_...() */
#include "position.h"
#include "register.h"                           /* trigger() */
#include "symbol.h"
#include "system.h"                             /* sys_...() */
#include "tty.h"
#include "wild.h"
#include "window.h"
#include "word.h"

enum {                                          /* window directions */
    WD_UP               = 0,
    WD_RIGHT            = 1,
    WD_DOWN             = 2,
    WD_LEFT             = 3,
    WD_MOVE             = 4,
    WD_MAX              = WD_MOVE
};

static const struct flag {
    const char *        f_name;                 /* name/label */
    int                 f_length;
    uint32_t            f_set;                  /* associated flag set */
    uint32_t            f_value;                /* flag value */

} x_bufflgnames[] = {
#define FLAGSETS        4
#define NFIELD(__x)     __x, (sizeof(__x) - 1)
    /*
     *  BF_... values ---
     *      Primitive buffer characteristics.
     */
    { NFIELD("changed"),            1,  BF_CHANGED              },      /* Changed/modified */
    { NFIELD("backup"),             1,  BF_BACKUP               },      /* Need to make a backup */
    { NFIELD("readonly"),           1,  BF_RDONLY               },      /* Read-only */
    { NFIELD("read"),               1,  BF_READ                 },      /* Buffer content still to be read */
    { NFIELD("exec"),               1,  BF_EXEC                 },      /* File is executable */
    { NFIELD("process"),            1,  BF_PROCESS              },      /* Buffer has process attached */
    { NFIELD("binary"),             1,  BF_BINARY               },      /* Binary buffer */
    { NFIELD("volatile"),           1,  BF_VOLATILE             },      /* Buffer is volatile (ie. may change) */
    { NFIELD("tabs"),               1,  BF_TABS                 },      /* Buffer inserts real-tabs */
    { NFIELD("sysbuf"),             1,  BF_SYSBUF               },      /* Buffer is a system buffer */
    { NFIELD("hidden"),             1,  BF_HIDDEN               },      /* Buffer is hidden within the buffer list */
    { NFIELD("noundo"),             1,  BF_NO_UNDO              },      /* Disable undo functionality */
    { NFIELD("newfile"),            1,  BF_NEW_FILE             },      /* File is a new file, so write even if no changes */
    { NFIELD("lock"),               1,  BF_LOCK                 },      /* File lock */
    { NFIELD("ocvt_crmode"),        1,  BF_CR_MODE              },      /* Append <CR> to end of each line on output */
    { NFIELD("syntax"),             1,  BF_SYNTAX               },      /* Enable syntax highlighting (unless ANSI) */
    { NFIELD("ansi"),               1,  BF_ANSI                 },      /* If TRUE, ANSI-fication is done */
    { NFIELD("man"),                1,  BF_MAN                  },      /* If TRUE, man style \b is done */
    { NFIELD("ruler"),              1,  BF_RULER                },      /* Display ruler */
    { NFIELD("eof"),                1,  BF_EOF_DISPLAY          },      /* Show <EOF> at EOF */
    { NFIELD("spell"),              1,  BF_SPELL                },      /* Enable spell functionality */
    { NFIELD("folding"),            1,  BF_FOLDING              },      /* Text folding/hiding */
    { NFIELD("autowrite"),          1,  BF_AUTOWRITE            },      /* Automaticly write the buffer if modified */
    { NFIELD("scrapbuf"),           1,  BF_SCRAPBUF             },      /* Scrap buffer */

    /*
     *  BF2_... values ---
     *      UI formatting control.
     */
    { NFIELD("attributes"),         2,  BF2_ATTRIBUTES          },      /* Enable buffer attributes */
    { NFIELD("dialog"),             2,  BF2_DIALOG              },      /* Dialog */

    { NFIELD("cursor_row"),         2,  BF2_CURSOR_ROW          },      /* Display cursor crosshair */
    { NFIELD("cursor_col"),         2,  BF2_CURSOR_COL          },
    { NFIELD("eol_cursor"),         2,  BF2_EOL_CURSOR          },
    { NFIELD("eof_cursor"),         2,  BF2_EOF_CURSOR          },

    { NFIELD("line_numbers"),       2,  BF2_LINE_NUMBERS        },      /* Has line numbers */
    { NFIELD("line_oldnumbers"),    2,  BF2_LINE_OLDNUMBERS     },      /* If has line numbers, display old lines */
    { NFIELD("line_status"),        2,  BF2_LINE_STATUS         },      /* Markup modified lines. */

    { NFIELD("title_full"),         2,  BF2_TITLE_FULL          },      /* Label window using full path name */
    { NFIELD("title_scroll"),       2,  BF2_TITLE_SCROLL        },      /* Scroll title with window */
    { NFIELD("title_left"),         2,  BF2_TITLE_LEFT          },      /* Left justify title */
    { NFIELD("title_right"),        2,  BF2_TITLE_RIGHT         },      /* Right justify title */

    { NFIELD("suffix_readonly"),    2,  BF2_SUFFIX_RO           },      /* Read-only suffix */
    { NFIELD("suffix_modified"),    2,  BF2_SUFFIX_MOD          },      /* Modified suffix */
    { NFIELD("eol_hilite"),         2,  BF2_EOL_HILITE          },      /* Limit hilites to EOL */
    { NFIELD("tilde"),              2,  BF2_TILDE_DISPLAY       },      /* Show <~> at EOF */

    { NFIELD("hiliteral"),          2,  BF2_HILITERAL           },      /* Hilite literal characters */
    { NFIELD("hiwhitespace"),       2,  BF2_HIWHITESPACE        },      /* Hilite whitespace */
    { NFIELD("himodified"),         2,  BF2_HIMODIFIED          },      /* Hilite modified lines */
    { NFIELD("hiadditional"),       2,  BF2_HIADDITIONAL        },      /* Hilite additional lines */

    /*
     *  BF3_... values ---
     *      Indirect buffer functionality, generally implemented by macro/plugins.
     */
    { NFIELD("autosave"),           3,  BF3_AUTOSAVE            },      /* Auto-save */
    { NFIELD("autoindent"),         3,  BF3_AUTOINDENT          },      /* Auto-indent */
    { NFIELD("autowrap"),           3,  BF3_AUTOWRAP            },      /* Auto-wrap */
    { NFIELD("paste_mode"),         3,  BF3_PASTE_MODE          },      /* Enable paste mode */

    /*
     *  BF4_... values ---
     *      File input/output processing.
     */
    { NFIELD("ocvt_trimwhite"),     4,  BF4_OCVT_TRIMWHITE      },      /* Output conversion, trim trailing whitespace */

    /*
     *  Additional/future options.
     *
    { NFIELD("ocvt_trimblank"),     4,  BF4_OCVT_TRIMBLANK      },         Output conversion, trim trailing blank lines

    { NFIELD("hex"),                2,  BF2_HEX                 },         HEX mode display engine
    { NFIELD("hex_aacii"),          2,  BF2_HEX_ASCII           },         ASCII column has focus, otherwise hex

    { NFIELD("ctrlz_strip"),        2,  BF2_CTRLZ_STRIP         },         Strip ^Z at end of file
    { NFIELD("ctrlz_append"),       2,  BF2_CTRLZ_APPEND        },         Append ^Z when writing file

    { NFIELD("icvt_tab2space"),     2,  BF2_ICVT_TAB_TO_SPACES  },         Input conversion, tabs to spaces
    { NFIELD("icvt_space2tab"),     2,  BF2_ICVT_SPACES_TO_TABS },         Input conversion, spaced to tabs
    { NFIELD("icvt_trimwhite"),     2,  BF2_ICVT_TRIMWHITE      },         Input conversion, trim trailing whitespace
    { NFIELD("icvt_trimblank"),     2,  BF2_ICVT_TRIMBLANK      },         Input conversion, trim trailing blank lines

    { NFIELD("ocvt_tab2spaces"),    2,  BF2_OCVT_TAB_TO_SPACES  },         Output conversion, tabs to spaces
    { NFIELD("ocvt_space2tab"),     2,  BF2_OCVT_SPACES_TO_TABS },         Output conversion, spaces to spaces
    { NFIELD("ocvt_embedded"),      2,  BF2_OCVT_EMBEDDED       },         Output conversion, convert inline spaces/tabs
    { NFIELD("ocvt_buffertabs"),    2,  BF2_OCVT_BUFFER_TABS    },         Output conversion, user buffer rules not default of 8.
     */
#undef  NFIELD
    };

static int                      flag_decode(int mode, const char *spec, uint32_t *values);
static const struct flag *      flag_lookup(const char *name, int length);

static int                      get_dir(int argi, const char *str);
static WINDOW_t *               get_window(int dir);
static WINDOW_t *               get_edge(int dir);

static int                      sortcompare_forward(const void *l1, const void *l2);
static int                      sortcompare_backward(const void *l1, const void *l2);
static int                      sortcompare_macro(void *callback, const void *l1, const void *l2);


/*  Function:           do_create_buffer
 *      create_buffer and created_next_buffer primitives.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: create_buffer - Create and load a buffer.

        int
        create_buffer(string bufname, [string filename],
                [int sysflag = FALSE], [int editflags = 0],
                    [string encoding = ""])

    Macro Description:
        The 'create_buffer()' primitive creates a buffer containing
        the content of an optional underlying file 'filename', with
        the filename being ae full or relative path name of a file
        which should be read into the buffer. If this parameter is
        omitted, then an initially empty buffer will be created.

        The buffer name 'bufname' should be unique, and if the buffer
        already exists a unique name shall be derived by prefixing
        its name with " [x]"; 'x' being the next available unique
        sequence.

        The optional arguments 'sysflag', 'editflags' and 'encoding'
        control a variety of the buffer features.

     Callbacks::

        Upon buffer loads firstly the <_extension> callback is executed
        and either an extension specific callback of the form '_ext' or
        <_default> shall be executed.

        In addition any related <register_macro> callbacks are executed.

     Example::

        The following creates the buffer named "Read-Me" and populates
        it with the content of the file "readme.txt".

>	    int buf;
>
>           if ((buf = create_buffer("Read-Me", "readme.txt")) >= 0) {
>               attach_buffer(newbuf);
>               return buf;
>           }
>           message("error loading buffer ...");
>           return -1;

    Macro Parameters:
        bufname - String containing the unique buffer name. The name
            is used as the buffer title, which is usually the same as
            the underlying filename yet it need not be; for example
            an abbreviated form. The buffer names does not affect the
            file that shall be loaded into the buffer.

        filename - Optional string containing the file that the
            buffer should contain, if omitted an empty buffer is
            created.

        sysflag - Optional integer boolean flag stating whether or
            not the buffer is a system buffer, FALSE for non-system
            otherwise TRUE for system; if omitted the buffer is
            assumed to be a non-system. See <inq_system> is more
            details on system buffers.

        editflags - Optional buffer creation flags. These flags
            control the file mode, see the <edit_file> primitive for
            additional information describing this field.

        encoding - Optional buffer encoding hint.

    Macro Returns:
        The 'create_buffer()' primitive returns the buffer identifier
        associated with the newly created buffer, otherwise -1 if the
        buffer was not created.

        Note the buffer does not become the current buffer until the
        <set_buffer> primitive is used against the returned buffer
        identifier.

    Macro Portability:
        n/a

    Macro See Also:
        attach_buffer, delete_buffer, set_buffer, create_nested_buffer,
        set_buffer_title

 *<<GRIEF>>
    Macro: create_nested_buffer - Create or reference a buffer.

        int
        create_nested_buffer(string bufname,
                [string filename], [int sysflag], [int editflags],
                    [string encoding])

    Macro Description:
        The 'create_nested_buffer()' primitive is similar to the
        <create_buffer> primitive yet if the buffer already exists
        its reference counter is incremented.

        For each 'nested' buffer increment <delete_buffer> must be
        called, with the buffer only being removed upon the reference
        count being zero.

        This primitive but is provided for convenience when temporary
        access to a buffer is required, see the 'ff' macro for a
        working example.

    Macro Parameters:
        bufname - String containing the unique buffer name. The name
            is used as the buffer title, which is usually the same as
            the underlying filename yet it need not be; for example
            an abbreviated form. The buffer names does not affect the
            file that shall be loaded into the buffer.

        filename - Optional string containing the file that the
            buffer should contain, if omitted an empty buffer is
            created.

        sysflag - Optional integer boolean flag stating whether or
            not the buffer is a system buffer, FALSE for non-system
            otherwise TRUE for system; if omitted the buffer is
            assumed to be a non-system. See <inq_system> is more
            details on system buffers.

        editflags - Optional buffer creation flags. These flags
            control the file mode, see the <edit_file> primitive for
            additional information describing this field.

        encoding - Optional buffer encoding hint.

    Macro Returns:
        The 'create_nested_buffer()' primitive returns the buffer
        identifier associated with the newly created buffer, 
        otherwise -1 if the buffer was not created.

    Macro Portability:
        n/a

    Macro See Also:
        attach_buffer, delete_buffer, set_buffer, create_buffer,
            set_buffer_title

 */
void
do_create_buffer(int nested)    /* int (string bufname, [string filename],
                                        [int sysflag], [int editflags], [string encoding]) */
{
    const char *raw_name = get_str(1);
    const char *name = file_canonicalize(raw_name, NULL, 0);
    const accint_t sysflag = get_xinteger(3, 0);
    const accint_t editflags = get_xinteger(4, EDIT_NORMAL);
    const char *encoding = get_xstr(5);         /* MCHAR??? */
    BUFFER_t *bp;

    acc_assign_int(-1);

    /* Exists ? */
    if (NULL == (bp = buf_find(name))) {
        /*
         *  create new
         */
        if (NULL == (bp = buf_find2(name, TRUE, encoding))) {
            return;
        }
        bp->b_title = chk_salloc(raw_name);     /* default "title" */

    } else if (nested) {
        /*
         *  increment reference count
         */
        acc_assign_int((accint_t) bp->b_bufnum);
        chk_free((void *)name);
        ++bp->b_refs;                           /* create/reference count */
        return;

    } else {
        /*
         *  otherwise, create new one with a different name.
         */
        char *newname = (name && name[0] ? chk_alloc(strlen(name) + 10) : NULL);
        int buf_id = 0;

        if (NULL == newname) {
            ewprintf("create_buffer: missing buffer name");
            acc_assign_int((accint_t) -1);
            return;
        }

        while (1) {
            sprintf(newname, "%s[%d]", name, ++buf_id);
            if (NULL == buf_find(newname)) {    /* not found */
                if (NULL == (bp = buf_find_or_create(newname))) {
                    return;
                }
                break;                          /* created */
            }
        }

        sprintf(newname, "%s[%d]", raw_name, buf_id);
        bp->b_title = newname;                  /* default "title" */
    }
    chk_free((void *)name);
    name = NULL;

    /*
     *  Populate
     */
    if (sysflag || (EDIT_SYSTEM | editflags)) {
        BFSET(bp, BF_SYSBUF);
        BFSET(bp, BF_NO_UNDO);
        bp->b_imode = TRUE;                     /* ignore user level imode */
    }
    acc_assign_int((accint_t) bp->b_bufnum);
    buf_clear(bp);

    if (!isa_undef(2)) {
        char path[MAX_PATH], *fname;

        shell_expand0(get_str(2), path, sizeof(path));
        fname = file_canonicalize(path, NULL, 0);
        if (file_readin(bp, fname, editflags, encoding) < 0) {
            acc_assign_int((accint_t) -1);
            buf_kill(bp->b_bufnum);
            return;
        }
        buf_name(bp, fname);
        bp->b_line = 1;
        bp->b_col = 1;
    }
    BFSET(bp, BF_READ);

    /*
     *  Verify and assign buffer type.
     */
    if (BFTYP_UNDEFINED == bp->b_type) {
        if (sysflag) {                          /* system buffer */
            buf_type_set(bp, BFTYP_UNIX);
            buf_encoding_set(bp, NULL);
        } else {                                /* user buffer */
            buf_type_default(bp);
        }
    }
}


/*  Function:           do_delete_buffer
 *      delete_buffer primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: delete_buffer - Delete a buffer.

        void
        delete_buffer(int bufnum)

    Macro Description:
        The 'delete_buffer()' primitive deletes the specified buffer, 
        the buffer contents and all associated resources are released.

        Any changes made to the buffer since it was last written
        shall be lost, as such if required the content should be
        written using <write_buffer>.

        In the case of a process buffer, the underlying sub-process
        is shutdown.

        Once deleted, the associated buffer handle is invalid.

    Macro Parameters:
        bufnum - Non-optional buffer number.

    Macro Returns:
        nothing

    Macro Portability:
        n/a

    Macro See Also:
        create_buffer, create_nested_buffer, set_buffer
 */
void
do_delete_buffer(void)          /* void (int bufnum) */
{
    buf_kill(get_xinteger(1, -1));
}


/*  Function:           do_set_buffer_flags
 *      set_buffer_flags primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: set_buffer_flags - Set buffer flags.

        void
        set_buffer_flags([int bufnum],
            [string|int or_mask], [string|int and_mask],
                [int set = 1])

    Macro Description:
        The 'set_buffer_flags()' primitive modifies the internal
        flags associated with the specified buffer, see <Buffer Flags>.

        If specified one or more flags shall be cleared using the
        'and_mask', in additional one or more flags are set using the
        'or_mask'.

        Each buffer maintains several sets of integer flags which can
        be modified. Against the selected flag 'set' the optional
        'and_mask' (clear) and then the optional 'or_mask' (set) is
        applied.

    Macro Parameters:
        bufnum - Optional buffer number, if omitted the current buffer
            shall be referenced.

        set_mask - Optional mask of flags to set. May either be an
            integer of AND'ed together flag constants, or
            alternatively a string of comma separated flag names.

        clear_mask - Optional mask of flags to clear. May either be an
            integer of AND'ed together flag constants, or
            alternatively a string of comma separated flag names.

        set - Optional integer stating the flag set to be modified, if
            omitted defaults to the primary set(1).

    Macro Returns:
        nothing

    Macro Portability:
        The string mask variants and 'set' parameter are GRIEF
        extension.

        Many of the flags are GRIEF specific; CRiSPEdit has a similar
        primitive yet as the two were developed independently
        features differ.

    Macro See Also:
        inq_buffer_flags

                        ------------------------

 *<<GRIEF>> [buffer, noprototype]
    Constants: Buffer Flags - Buffer attribute constants

    Description:

        Buffer flags are represented by bit-fields grouped in one of
        four sets. The following section describes the possible
        values for theses flags:

      First::

        First flag set, representing status and control primary
        buffer options.

(start table,format=nd)
        [Constant               [Description                                    ]
      ! BF_CHANGED              Changed.
      ! BF_BACKUP               Backup required on next write.
      ! BF_RDONLY               Read-only.
      ! BF_READ                 Buffer content still to be read.
      ! BF_EXEC                 File is executable.
      ! BF_PROCESS              Buffer has process attached.
      ! BF_BINARY               Binary buffer.
      ! BF_ANSI                 If TRUE, ANSI-fication is done.
      ! BF_TABS                 Buffer inserts real-tabs.
      ! BF_SYSBUF               Buffer is a system buffer.
      ! BF_LOCK                 File lock.
      ! BF_NO_UNDO              Dont keep undo info.
      ! BF_NEW_FILE             File is a new file, so write even if no changes.
      ! BF_CR_MODE              Append <CR> to end of each line on output.
      ! BF_SYNTAX               Enable syntax highlighting (unless ANSI).
      ! BF_STATUSLINE           Status line.
      ! BF_MAN                  If TRUE, man style \b is done.
      ! BF_SPELL                Enable spell.
      ! BF_FOLDING              Test folding/hiding.
      ! BF_RULER                Display ruler.
      ! BF_VOLATILE             Buffer is volatile.
      ! BF_EOF_DISPLAY          Show <EOF> markers.
      ! BF_HIDDEN               Hidden buffer, from buffer list
      ! BF_AUTOREAD             Automatically re-read buffer,
                                if underlying changes.
      ! BF_AUTOWRITE            Automatically write buffer, if modified
      ! BF_SCRAPBUF             Scrap buffer.
      ! BF_DELAYED              Content load delayed until first reference.
(end table)

      Second::

        Second buffer set, controlling general UI formatting options.

(start table,format=nd)
        [Constant               [Description                                    ]
      ! BF2_ATTRIBUTES          Character attributes, enables character
                                cell coloring.

      ! BF2_DIALOG              Dialog

      ! BF2_CURSOR_ROW          Display cursor cross-hair.
      ! BF2_CURSOR_COL
      ! BF2_TILDE_DISPLAY
      ! BF2_EOL_HILITE          Limit hilites to EOL.

      ! BF2_LINE_NUMBERS        Line numbers
      ! BF2_LINE_OLDNUMBERS     If has line numbers, display old lines
      ! BF2_LINE_STATUS         Markup modified lines.
      ! BF2_LINE_SYNTAX         Syntax preprocessor flags

      ! BF2_TITLE_FULL          Label window using full path name
      ! BF2_TITLE_SCROLL        Scroll title with window
      ! BF2_TITLE_LEFT          Left justify title
      ! BF2_TITLE_RIGHT         Right justify title

      ! BF2_SUFFIX_RO           Read-only suffix on title
      ! BF2_SUFFIX_MOD          Modified suffix on title
      ! BF2_EOL_CURSOR          Limit cursor to EOL
      ! BF2_EOF_CURSOR          Limit cursor to EOF

      ! BF2_HILITERAL           Hilite literal characters
      ! BF2_HIWHITESPACE        Hilite whitespace
      ! BF2_HIMODIFIED          Hilite modified lines
      ! BF2_HIADDITIONAL        Hilite added lines
(end table)

      Third::

        Third flag set, controlling indirect buffer functionality
        which are generally implemented at a macro level.

(start table,format=nd)
        [Constant               [Description                                    ]
      ! BF3_AUTOSAVE            Auto-save
      ! BF3_AUTOINDENT          Auto-indent
      ! BF3_AUTOWRAP            Auto-wrap
      ! BF3_PASTE_MODE          Paste mode, disables a number of auto functions.
(end table)

      Fourth::

        Fourth flag set, controlling file conversion options.

(start table,format=nd)
        [Constant               [Description                                    ]
      ! BF4_OCVT_TRIMWHITE      Output conversion, trim trailing whitespace
(end table)

    Constants See Also:
        inq_buffer_flags, set_buffer_flags
 */
void
do_set_buffer_flags(void)       /* void ([int bufnum], [string|int or_mask], [string|int and_mask], [int set]) */
{
    BUFFER_t *bp = buf_argument(1);
    const accint_t set = get_xinteger(4, 1);
    int bchg = BFTST(bp, BF_CHANGED);

    /* and/clear */
    if (isa_string(3)) {                        /* extension */
        uint32_t values[FLAGSETS];

        if (flag_decode(1, get_xstr(3), values) > 0) {
            bp->b_flag1 &= values[0];
            bp->b_flag2 &= values[1];
            bp->b_flag3 &= values[2];
            bp->b_flag4 &= values[3];
       }

    } else if (isa_integer(3)) {
        const uint32_t value = (uint32_t) get_xinteger(3, 0);

        switch (set) {
        case 1: bp->b_flag1 &= value; break;
        case 2: bp->b_flag2 &= value; break;
        case 3: bp->b_flag3 &= value; break;
        case 4: bp->b_flag4 &= value; break;
        case 9: bp->b_flagu &= value; break;
        }
    }

    if (bchg && !BFTST(bp, BF_CHANGED)) {
        bp->b_nummod = 0;
        bchg = FALSE;
    }

    /* or/set */
    if (isa_string(2)) {                        /* extension */
        uint32_t values[FLAGSETS];

        if (flag_decode(0, get_xstr(2), values) > 0) {
            bp->b_flag1 |= values[0];
            bp->b_flag2 |= values[1];
            bp->b_flag3 |= values[2];
            bp->b_flag4 |= values[3];
        }
    } else if (isa_integer(2)) {
        const uint32_t value = (uint32_t) get_xinteger(2, 0);

        switch (set) {
        case 1: bp->b_flag1 |= value; break;
        case 2: bp->b_flag2 |= value; break;
        case 3: bp->b_flag3 |= value; break;
        case 4: bp->b_flag4 |= value; break;
        case 9: bp->b_flagu |= value; break;
        }
    }

    if (!bchg && BFTST(bp, BF_CHANGED)) {
        bp->b_nummod = 1;
    }
}


/*  Function:           inq_buffer_flags
 *      inq_buffer_flags primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: inq_buffer_flags - Retrieve buffer flags.

        int
        inq_buffer_flags([int bufnum],
            [string flag|int set = 1], [string ~flags])

    Macro Description:
        The 'inq_buffer_flags()' primitive retrieves one of the set
        of flags associated with the specific buffers, see <Buffer Flags>.

    Macro Parameters:
        bufnum - Optional buffer number, if omitted the current buffer
            shall be referenced.

        flag/set - Optional internal set identifier, if omitted the
            primary set(1) shall be referenced.

        ... - Optional string of comma separated flag names.

     Flags::

        The following table summaries the existing flags, for
        additional on a specific flag consult the <set_buffer_flags>
        primitive.

    Macro Returns:
        The 'inq_buffer_flags()' primitive returns the value
        associated with the selected set of flags.

    Macro Portability:
        The string flag parameter variant is a GRIEF extension.

        Many of the flags are GRIEF specific; CRiSPEdit has a similar
        primitive yet as the two were developed independently
        features differ.

    Macro See Also:
        set_buffer_flags
 */
void
inq_buffer_flags(void)          /* int ([int bufnum], [string flag|int set = 1], [string ~flags]) */
{
    const BUFFER_t *bp = buf_argument(1);
    accint_t val = 0;                           /* unknown/error */

    if (bp) {
        if (isa_string(2)) {                    /* extension, by-name */
            uint32_t values[FLAGSETS];

            if (flag_decode(0, get_xstr(2), values) > 0) {
                if (values[0]) {
                    val = (bp->b_flag1 & values[0]);
                } else if (values[1]) {
                    val = (bp->b_flag2 & values[1]);
                } else if (values[2]) {
                    val = (bp->b_flag3 & values[2]);
                } else if (values[3]) {
                    val = (bp->b_flag4 & values[3]);
                }
            }

        } else {
            const accint_t set = get_xinteger(2, 1);

            if (isa_string(3)) {                /* extension, export flags */
                const unsigned FLAGSIZE = (16 * sizeof(x_bufflgnames)/sizeof(x_bufflgnames[0]));
                const struct flag *flag = x_bufflgnames;
                const char *delimiter = "";
                unsigned i, idx = 0;
                char *flags;

                if (NULL != (flags = (char *)chk_alloc(FLAGSIZE))) {
                    const uint32_t flag1 = bp->b_flag1, flag2 = bp->b_flag2;

                    for (i = 0; i < (unsigned)(sizeof(x_bufflgnames)/sizeof(x_bufflgnames[0])); ++i, ++flag) {
                        const uint32_t f_set = flag->f_set;
                        const uint32_t f_value = (1 == f_set ? flag1 : flag2);

                        if (-1 == set) {
                            idx += sxprintf(flags + idx, FLAGSIZE - idx, "%s%s=%s", delimiter,
                                        flag->f_name, (f_value & flag->f_value ? "yes" : "no"));
                            delimiter = ",";

                        } else if (0 == set || f_set == (uint32_t)set) {
                            if (f_value & flag->f_value) {
                                idx += sxprintf(flags + idx, FLAGSIZE - idx, "%s%s", delimiter, flag->f_name);
                                delimiter = ",";
                            }
                        }
                    }
                    argv_assign_str(3, (const char *)flags);
                    chk_free((void *)flags);
                }
            }

            switch (set) {                      /* flag value */
            case 1: val = bp->b_flag1; break;
            case 2: val = bp->b_flag2; break;
            case 3: val = bp->b_flag3; break;
            case 4: val = bp->b_flag4; break;
            case 9: val = bp->b_flagu; break;
            }
        }
    }
    acc_assign_int(val);
}

static int
flag_decode(int mode, const char *spec, uint32_t *values)
{
    static const char who[] = "set_buffer_flags";
    const char *comma, *cursor = spec;
    uint32_t nvalues[FLAGSETS] = {0};
    const struct flag *flag;
    int i, flags = 0;

    trace_ilog("flag_decode(mode:%d, spec:%s)\n", mode, spec);

    while (NULL != (comma = strchr(cursor, ',')) || *cursor) {
        if (NULL != (flag = (NULL == comma ?    /* <value>[,<value>] */
                flag_lookup(cursor, strlen(cursor)) : flag_lookup(cursor, comma - cursor)))) {
            const int set = flag->f_set - 1;

            assert(set >= 0);
            assert(set < FLAGSETS);
            nvalues[set] |= flag->f_value;
            flags |= (0x01 << set);
        } else {
            if (comma)  {
                errorf("%s: unknown flag '%*s'.", who, comma - spec, spec);
            } else {
                errorf("%s: unknown flag '%s'.", who, spec);
            }
            return -1;
        }
        if (NULL == comma) break;
        cursor = comma + 1;
    }

    trace_log(" == flags=0x%x [", flags);
    for (i = 0; i < FLAGSETS; ++i) {            /* assign, flip if required */
        values[i] = (1 == mode ? ~nvalues[i] : nvalues[i]);
        trace_log(" %d:0x%08x", i, values[i]);
    }
    trace_log(" ]\n");
    return flags;
}


static const struct flag *
flag_lookup(const char *name, int length)
{
    if (NULL != (name = str_trim(name, &length)) && length > 0) {
        unsigned i;

        trace_ilog("\t %*s\n", length, name);
        for (i = 0; i < (unsigned)(sizeof(x_bufflgnames)/sizeof(x_bufflgnames[0])); ++i)
            if (length == x_bufflgnames[i].f_length &&
                    0 == str_nicmp(x_bufflgnames[i].f_name, name, length)) {
                return x_bufflgnames + i;
            }
    }
    return NULL;
}


/*  Function:           do_set_buffer_title
 *      set_buffer_title primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: set_buffer_title - Set a buffers title.

        int
        set_buffer_title(
            [int bufnum], [string title])

    Macro Description:
        The 'set_buffer_title()' primitive sets the buffer title of
        the stated buffer otherwise the current buffer when omitted.
        The specified title is displayed on the top edge of the
        buffers associated window.

    Macro Parameters:
        bufnum - Optional buffer number, if omitted the current buffer
            shall be referenced.

        title - Optional string value of the title to associated. If omitted
            specified, then the buffer title is remove with the
            buffers underlying filename being used.

    Macro Returns:
        The 'set_buffer_title()' primitives return zero on success, 
        otherwise -1 if the specified buffer does not exist.

    Macro Portability:
        n/a

    Macro See Also:
        inq_buffer_title, create_buffer
 */
void
do_set_buffer_title(void)       /* int ([int bufnum], [string title]) */
{
    BUFFER_t *bp = buf_argument(1);

    if (NULL == bp) {
        acc_assign_int(-1);

    } else {
        const char *title = get_xstr(2);

        if (bp->b_title) {
            chk_free((void *)bp->b_title);
        }
        bp->b_title = title ? chk_salloc(title) : NULL;
        acc_assign_int(0);
        if (bp == curbp && curwp->w_bufp == curbp) {
            win_modify(WFHARD);                 /* force update */
            vtupdate();
        }
    }
}


/*  Function:           inq_buffer_title
 *      inq_buffer_title primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: inq_buffer_title - Retrieve a buffer title.

        string
        inq_buffer_title([int bufnum])

    Macro Description:
        The 'inq_buffer_title()' primitive retrieves the title associated
        with the specified buffer.

    Macro Parameters:
        bufnum - Optional buffer number, if omitted the current buffer
            shall be referenced.

    Macro Returns:
        String containing the current buffer title.

    Macro Portability:
        n/a

    Macro See Also:
        create_buffer, set_buffer_title
 */
void
inq_buffer_title(void)          /* string ([int bufnum] */
{
    BUFFER_t *bp = buf_argument(1);

    acc_assign_str(bp && bp->b_title ? bp->b_title : sys_basename(bp->b_fname), -1);
}


/*  Function:           do_set_buffer_type
 *      set_buffer_type primitive.
 *
 *  Macro See Also:
 *      inq_buffer, inq_terminator and set_terminator
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: set_buffer_type - Set the buffer storage type.

        int
        set_buffer_type([int bufnum],
            [int type = NULL], [string encoding = NULL])

    Macro Description:
        The 'set_buffer_type()' primitive optionally set the buffer
        type and/or the character encoding associated with the
        specified buffer.

        Note that the specified 'encoding' has priority over the
        buffer type, in that an incompatible encoding with the stated
        'type' or pre-existing buffer type shall imply the default
        buffer type associated with the encoding. The
        'inq_buffer_type()' primitive should be used to determine the
        resulting buffer type on completion.

    Macro Parameters:
        bufnum - Optional buffer number, if omitted the current buffer
            shall be referenced.

        type - Optional integer buffer type which states the basic
            buffer encoding include an implied line termination.

        encoding - Optional string which sets the specific buffer
            encoding beyond the buffer type, for example the page code
            utilized by a BFTYPE_DOS buffer.

     Buffer Types::

        The following manifest constants define the available Buffer Types.

(start table,format=nd)
        [Constant           [Description                            ]

      ! BFTYP_UNKNOWN       Unknown buffer type.

      ! BFTYP_UNIX          Unix, LF line termination.
      ! BFTYP_DOS           DOS, CF/LF line termination.
      ! BFTYP_MAC           Old style MAX, CR termination.
      ! BFTYP_BINARY        Binary.
      ! BFTYP_ANSI          ANSI.
      ! BFTYP_EBCDIC        EBCDIC.

      ! BFTYP_UTF8          UTF8.
      ! BFTYP_UTF16         UTF16/USC2.
      ! BFTYP_UTF32         UTF32/USC4.
      ! BFTYP_UTFEBCDIC     UTF8/EBCDIC.
      ! BFTYP_BOCU1         Binary Ordered Compression for Unicode.
      ! BFTYP_SCSU          Standard Compression Scheme for Unicode.
      ! BFTYP_UTF7          7-bit Unicode Transformation Format.

      ! BFTYP_GB            GB.
      ! BFTYP_BIG5          BIG5.

      ! BFTYP_ISO2022       ISO-2022.

      ! BFTYP_SBCS          Single Byte.
      ! BFTYP_DBCS          Double Byte.
      ! BFTYP_MBCS          Multi-Byte (Non Unicode).

      ! BFTYP_OTHER         Other supported.
      ! BFTYP_UNSUPPORTED   Known file-type, yet no internal support.
(end table)

    Macro Returns:
        The 'set_buffer_type()' primitive returns the 0 on success ,
        otherwise -1 on error.

    Macro Portability:
        A Grief extension.

    Macro See Also:
        inq_buffer_type

 */
void
do_set_buffer_type(void)        /* int ([int bufnum], [int type = NULL], [string encoding = NULL]) */
{
    static const char who[] = "set_buffer_type";
    BUFFER_t *bp = buf_argument(1);
    accint_t ret = (bp ? (accint_t)bp->b_type : 0);

    if (bp) {
        const int16_t obtype = bp->b_type;
        const char *encoding = get_xstr(3);
        mcharcharsetinfo_t info;

        if (!isa_undef(2)) {                    /* buffer-type */
            const int16_t nbtype = (int16_t) get_xinteger(2, BFTYP_DEFAULT);

            if (BFTYP_UNKNOWN == nbtype || BFTYP_UNDEFINED == nbtype ||
                    NULL == buf_type_desc(nbtype, NULL)) {
                ewprintf("%s: invalid buffer-type '%d'", who, nbtype);
                encoding = NULL;
                ret = -1;

            } else if (obtype != nbtype) {
                buf_type_set(bp, nbtype);

                if (NULL == encoding &&         /* check/derive encoding */
                        NULL == (encoding = buf_type_encoding(nbtype))) {
                    if (bp->b_encoding) {       /* compatible encoding? */
                        if (mchar_info(&info, bp->b_encoding, -1)) {
                            if (buf_type_base(nbtype) != info.cs_type) {
                                buf_encoding_set(bp, NULL);
                            }
                        }
                    }
                }
            }
        }

        if (encoding) {
            if (mchar_info(&info, encoding, -1)) {
                if (! isa_undef(2)) {           /* implied buffer-type */
                    if (buf_type_base(obtype) != info.cs_type) {
                        buf_type_set(bp, info.cs_type);
                    }
                }
            } else {
                ewprintf("%s: unknown encoding '%s'", who, encoding);
                encoding = NULL;
                ret = -1;
            }

            if (encoding) {                     /* update encoding */
                buf_encoding_set(bp, encoding);
            }
        }
    }
    acc_assign_int(ret);
}


/*  Function:           inq_buffer_type
 *      inq_buffer_type primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: inq_buffer_type - Retrieve buffer type.

        int
        inq_buffer_type([int bufnum],
            [string &desc], [string &encoding])

    Macro Description:
        The 'inq_buffer_type()' primitive retrieves the buffer type
        of the buffer 'bufnum'.

    Macro Parameters:
        bufnum - Optional buffer number, if omitted the current buffer
            shall be referenced.

        desc - Optional string variable reference to be populated
            with the buffer encoding description.

        encoding - Optional string variable reference to be populated
            with the buffers character encoding name.

    Macro Returns:
        The 'inq_buffer_type()' primitive returns on the following
        manifest constants representing the base encoding of the
        referenced buffer.

    Buffer Types::

        The following manifest constants define the available
        Buffer Types.

(start table,format=nd)
        [Constant           [Description                            ]

      ! BFTYP_UNKNOWN       Unknown buffer type.

      ! BFTYP_UNIX          Unix, LF line termination.
      ! BFTYP_DOS           DOS, CF/LF line termination.
      ! BFTYP_MAC           Old style MAX, CR termination.
      ! BFTYP_BINARY        Binary.
      ! BFTYP_ANSI          ANSI.
      ! BFTYP_EBCDIC        EBCDIC.

      ! BFTYP_UTF8          UTF8.
      ! BFTYP_UTF16         UTF16/USC2.
      ! BFTYP_UTF32         UTF32/USC4.
      ! BFTYP_UTFEBCDIC     UTF8/EBCDIC.
      ! BFTYP_BOCU1         Binary Ordered Compression for Unicode.
      ! BFTYP_SCSU          Standard Compression Scheme for Unicode.
      ! BFTYP_UTF7          7-bit Unicode Transformation Format.

      ! BFTYP_GB            GB.
      ! BFTYP_BIG5          BIG5.

      ! BFTYP_ISO2022       ISO-2022.

      ! BFTYP_SBCS          Single Byte.
      ! BFTYP_DBCS          Double Byte.
      ! BFTYP_MBCS          Multi-Byte (Non Unicode).

      ! BFTYP_OTHER         Other supported.
      ! BFTYP_UNSUPPORTED   Known file-type, yet no internal support.
(end table)

    Macro Portability:
        n/a

    Macro See Also:
        set_buffer_type
 */
void
inq_buffer_type(void)           /* int ([int bufnum], [string desc], [string encoding]) */
{
    const BUFFER_t *bp = buf_argument(1);
    char encoding[64] = {0};
    const char *desc = "";

    if (bp) {
        desc = buf_type_desc(bp->b_type, "n/a");
        if (bp->b_encoding) {
            strxcpy(encoding, bp->b_encoding, sizeof(encoding));
            if (bp->b_termtype && encoding[0]) {
                const char *termdesc =
                        (LTERM_UNDEFINED == bp->b_termtype ?
                            NULL : buf_termtype_desc(bp->b_termtype, NULL));

                if (termdesc && termdesc[0]) {
                    strxcat(encoding, "-", sizeof(encoding));
                    strxcat(encoding, termdesc, sizeof(encoding));
                    str_lower(encoding);
                }
            }
        }
    }
    argv_assign_str(2, desc);
    argv_assign_str(3, encoding);
    acc_assign_int(bp ? bp->b_type : 0);
}


/*  Function:           do_set_attribute
 *       set_attribute primitive.
 *
 *   Macro Parameters:
 *       current - Current text attribute value or name.
 *       normal - Clear/padding atrtibute value or name.
 *      [bufnum] - Optional buffer-identifier otherwise current.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: set_attribute - Set the color attributes.

        int
        set_attribute(
            [int|string text], [int|string normal],
                [int bufnum])

    Macro Description:
        The 'set_attribute()' primitive set the text and/or normal
        attributes for the specified buffer 'bufnum'.

    Macro Parameters:
        text - Optional text attribute either by value or name.

        normal - Optional clear/normal attribute either by value or name.

        bufnum - Optional buffer number, if omitted the current buffer
            shall be referenced.

    Macro Returns:
        The 'set_attribute()' primitive returns the previous text
        attribute, otherwise -1 on error.

    Macro Portability:
        A Grief extension.

    Macro See Also:
        inq_attribute
 */
void
do_set_attribute(void)          /* int ([int|string current], [int|string normal], [int bufum]) */
{
    const int ATTRMAX = (1 == sizeof(LINEATTR) ? 0xff : 2048);
    BUFFER_t *bp = buf_argument(3);

    if (bp && BF2TST(bp, BF2_ATTRIBUTES)) {
        const char *name = get_xstr(1);
        int attr = -1;

        /*
         *  text attribute
         */
        acc_assign_int((accint_t) bp->b_attrcurrent);
        if (NULL != name) {
            if ((attr = attribute_value(name)) >= 0) {
                bp->b_attrcurrent = (LINEATTR) attr;
            }
        } else if (isa_integer(1)) {
            if ((attr = get_xinteger(1, -1)) >= 0 && attr <= ATTRMAX) {
                bp->b_attrcurrent = (LINEATTR) attr;
            }
        }

        /*
         *  clear/normal attribute
         */
        if (NULL != (name = get_xstr(2))) {
            if ((attr = attribute_value(name)) >= 0 && attr <= ATTRMAX) {
                bp->b_attrnormal = (LINEATTR) attr;
            }
        } else if (isa_integer(2)) {
            if ((attr = get_xinteger(2, -1)) >= 0 && attr <= ATTRMAX) {
                bp->b_attrnormal = (LINEATTR) attr;
            }
        }

    } else {
        acc_assign_int((accint_t) -1);
    }
}


/*  Function:           inq_attribute
 *      inq_attribute primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: inq_attribute - Retrieve the current attributes.

        int
        inq_attribute(
            [int &normal], [int bufnum])

    Macro Description:
        The 'inq_attribute()' primitive retrieves the text and optionally
        the normal attribute for the specified buffer 'bufnum'.

    Macro Parameters:
        normal - Optional integer reference, if stated is populated with
            the clear/normal attribute value.

        bufnum - Optional buffer number, if omitted the current buffer
            shall be referenced.

    Macro Returns:
        The 'inq_attribute()' primitive returns the current text attribute,
        otherwise -1 on error.

    Macro Portability:
        A Grief extension.

    Macro See Also:
        set_attribute
 */
void
inq_attribute(void)             /* int ([int bufnum]) */
{
    const BUFFER_t *bp = buf_argument(1);
    const accint_t val = (bp ? (accint_t)bp->b_attrcurrent : -1);

    if (bp) argv_assign_int(2, (accint_t)bp->b_attrnormal);
    acc_assign_int(val);
}


/*  Function:           do_inq_mode
 *      inq_mode primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: inq_mode - Returns the overstrike mode.

        int
        inq_mode([int bufnum], [int &localised])

    Macro Description:
        The 'inq_mode()' primitive retrieves the current
        insert/overstrike (also known as overtype) mode.

    Macro Parameters:
        bufnum - Optional buffer number, if omitted the current buffer
            shall be referenced.

        localised - Optional integer reference, if specified is
            populated with the localisation status. If *true* then the
            mode is specific to the referenced buffer, otherwise the
            global mode is active.

    Macro Returns:
        The 'inq_mode()' primitive retrieves the non-zero if in
        insert mode, otherwise zero if in overstrike mode.

    Macro Portability:
        The localised status is a Grief extension.

    Macro See Also:
        insert_mode, insert
 */
void
inq_mode(void)                  /* int ([int bufnum], [int &localised]) */
{
    const BUFFER_t *bp = buf_argument(1);
    const int imode = buf_imode(bp);
    const int localised = (bp && bp->b_imode >= 0 ? TRUE : FALSE);

    argv_assign_int(2, (accint_t) localised);   /* 24/09/10 */
    acc_assign_int(imode);
}


/*  Function:           do_insert_mode
 *      insert_mode primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: insert_mode - Set the buffer insert/overstrike mode.

        int
        insert_mode([int value], [int bufnum])

    Macro Description:
        The 'insert_mode()' primitive sets the insert/over-strike
        mode to 'value' otherwise toggles if omitted.

        The applied mode shall either be localised to the specified
        buffer 'bufname', otherwise if omitted the global (default)
        mode that applies to all buffers within a localised setting.

    Macro Parameters:
        value - Optional integer stating the insert mode being zero for
            over-strike and non-zero for insert. For localised modes -1 
            clears the active localisation, restoring use of the
            global (default) mode. If omitted the current mode is
            toggled.

        bufnum - Optional buffer number when stated the buffer specific
            insert mode shall be modified, if omitted the global
            insert mode is modified.

    Macro Returns:
        The 'insert_mode()' primitive returns the previous insert mode.

    Macro Portability:
        n/a

    Macro See Also:
        inq_mode
 */
void
do_insert_mode(void)            /* int ([int value], [int bufnum]) */
{
    int oimode, nimode;

    /* global or buffer-specific */
    if (isa_undef(2)) {
        oimode = x_imode;
        nimode = get_xinteger(1, !oimode);      /* value or toggle */
        x_imode = nimode;

    } else {                                    /* 01/09/10 */
        BUFFER_t *bp = buf_argument(2);

        oimode = buf_imode(bp);
        nimode = get_xinteger(1, !oimode);
        if (nimode < 0) {                       /* -1 == clear */
            bp->b_imode = -1;
            nimode = x_imode;
        } else {
            bp->b_imode = nimode;
        }
    }

    /* mode change */
    if (oimode != nimode) {
        if (NULL == x_scrfn.scr_cursor)  {
            elinecol(TRUE);
        }
        ecursor(nimode);
        trigger(REG_INSERT_MODE);
    }

    acc_assign_int((accint_t) oimode);          /* old mode */
}


/*  Function:           inq_modified
 *      inq_modified primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: inq_modified - Determine a buffers modification status.

        int
        inq_modified([int bufnum])

    Macro Description:
        The 'inq_modified()' primitive determine whether the specified
        buffer 'bufnum' has been modified.

    Macro Parameters:
        bufnum - Optional buffer number, if omitted the current buffer
            shall be referenced.

    Macro Example:

        The following echos to the command prompt the current buffers
        modification status.

>           message("Buffer has %sbeen modified.",
>                       inq_modified() ? "" : "not ");

    Macro Returns:
        The 'inq_modified()' primitive returns the modification status,
        *true* when modified otherwise *false* if the buffer has no
        changes since loading or the last save.

    Macro Portability:
        n/a

    Macro See Also:
        inq_time, inq_system
 */
void
inq_modified(void)              /* int ([int bufnum]) */
{
    const BUFFER_t *bp = buf_argument(1);
    const accint_t val = (bp ? (accint_t)bp->b_nummod : 0);

    acc_assign_int(val);
}


/*  Function:           inq_system
 *      inq_system primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: inq_system - Determine if buffer is a system buffer.

        int
        inq_system([int bufnum])

    Macro Description:
        The 'inq_system()' primitive determines whether the specified
        buffer 'bufnum' is marked as a system buffer.

        System buffers do not appear in buffer lists, are not editable
        by users and are handled specially by many macros. System buffer
        are generally utilised by macros for internal work pads.

    Macro Parameters:
        bufnum - Optional buffer number, if omitted the current buffer
            shall be referenced.

    Macro Returns:
        The 'inq_system()' primitive returns non-zero if the associated
        buffer is a system buffer, otherwise 0 if the buffer is a
        normal buffer.

    Macro Portability:
        n/a

    Macro See Also:
        create_buffer, set_buffer_flags, inq_modified
 */
void
inq_system(void)                /* int ([int bufnum]) */
{
    const BUFFER_t *bp = buf_argument(1);
    const accint_t val = (bp ? BFTST(bp, BF_SYSBUF) : 0);

    acc_assign_int(val);
}



/*  Function:           inq_time
 *      inq_time primitive, retrieves the time of the last buffer modification.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: inq_time - Retrieve the last modification time.

        int
        inq_time([int bufnum], [int &ctime])

    Macro Description:
        The 'inq_time()' primitive returns the time at which the last
        modification occurred of the specified buffer 'bufnum', 
        represented by the number seconds since the beginning of the
        current edit session.

    Macro Parameters:
        bufnum - Optional buffer number, if omitted the current buffer
            shall be referenced.
        ctime - Optional integer value reference, if specified shall be
            populated with the associated system time, being the
            number of second since 1970/01/01 UTC.

    Macro Returns:
        The 'inq_time()' primitive returns the time in seconds of
        last modification, 0 if the buffer has not been modified
        during the current edit session, otherwise -1 if the buffer
        is invalid.

    Macro Portability:
        n/a

    Macro See Also:
        inq_modified
 */
void
inq_time(void)                  /* int ([int bufnum], [int ctime]) */
{
    const BUFFER_t *bp = buf_argument(1);

    if (NULL == bp) {
        acc_assign_int(-1);                     /* unknown */

    } else {
        const accint_t val = (accint_t) bp->b_ctime;

        argv_assign_int(2, val);
        if (val) {
            acc_assign_int((accint_t)((val - x_startup_time) + 1));
        } else {
            acc_assign_int(0);                  /* no local changes */
        }
    }
}


/*  Function:           do_inq_names
 *      inq_names primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: inq_names - Retrieve associated buffer names.

        int
        inq_names([string fullname], [string ext],
            [string bufname], [int bufnum])

    Macro Description:
        The 'inq_names()' primitive retrieves the file and/or buffer names
        associated with the specified buffer 'bufnum'.

    Macro Parameters:
        fullname - Optional string variable reference, if specified
            shall be populated with the full path name of the underlying
            file, that is used on <write_buffer> calls.

        ext - Optional string variable reference, if specified shall be
            populated the file extension taken from the full path.

        bufname - Optional string variable reference, if specified shall
            be populated with buffer name which is used as the buffer
            title, see <set_buffer_title>; which is usually the basename,
            i.e. full path without the path.

        bufnum - Optional buffer number, if omitted the current buffer
            shall be referenced.

    Macro Returns:
        The 'inq_names()' primitive returns 0 on success, otherwise -1 on
        error.

    Macro Portability:
        n/a

    Macro See Also:
        create_buffer, set_buffer_title
 */
void
inq_names(void)                 /* int ([string fullname], [string ext], [string bufname], [int bufnum]) */
{
    const BUFFER_t *bp = buf_argument(4);

    if (NULL == bp) {
        acc_assign_int(-1);                     /* unknown */

    } else {
        const char *fname, *ext;

        fname = bp->b_fname;
        ext = strrchr(fname, '.');
        if (NULL == ext || ext < strrchr(fname, '/')) {
            ext = "";                           /* no extension */
        } else {
            ++ext;
        }
        argv_assign_str(1, fname);
        argv_assign_str(2, ext);
        if (!isa_undef(3)) {
            sym_assign_str(get_symbol(3), sys_basename(fname));
        }
        acc_assign_int(0);                      /* success */
    }
}


/*  Function:           do_next_buffer
 *      next_buffer primitive.
 *
 *  Description:
 *      This macro is used to select the next (or previous) buffer. First argument says
 *      whether to include system buffers, and second argument is non-zero if we want
 *      to go backwards in the list. *
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: next_buffer - Identifier of the next buffer.

        int
        next_buffer([int sysflag = 0],
            [int previous], [int tab])

    Macro Description:
        The 'next_buffer()' primitive retrieves the buffer identifier of
        the next buffer after the current buffer in the buffer list,
        optionally filtering system buffers.

        The buffer list, which is maintained by the GRIEF kernel, is a
        circular list of all buffers. Upon the end of list being reached,
        the first buffer on the list is returned as the next.

        Note!:
        The next_buffer primitive does not alter the current buffer, the
        <set_buffer> can be used to select the returned buffer.

    Macro Parameters:
        sysflag - Optional system buffer filter selection. If either
            omitted or zero system buffers shall be filtered from the
            returned identifiers. Otherwise all buffers including system
            shall be returned.

        prev - Optional boolean flag, if stated as non-zero then the
            previous buffer in the buffer list is retrieved.

        tab - Reserved for future use; tab identifier.

    Macro Returns:
        The 'next_buffer()' primitive returns the buffer identifier
        of the next or previous buffer.

    Macro Portability:
        n/a

    Macro See Also:
        previous_buffer, set_buffer, create_buffer, inq_buffer, inq_system

 *<<GRIEF>>
    Macro: previous_buffer - Identifier of the previous buffer.

        int
        previous_buffer([int sysflag = 0], [int tab])

    Macro Description:
        The 'previous_buffer()' primitive retrieves the buffer
        identifier of the previous buffer after the current buffer in
        the buffer list, optionally filtering system buffers.

        The buffer list, which is maintained by the GRIEF kernel, is
        a circular list of all buffers. Upon the beginning of list
        being reached, the last buffer on the list is returned as the
        previous.

        Note!:
        The previous_buffer primitive does not alter the current
        buffer, the <set_buffer> can be used to select the returned
        buffer.

    Macro Parameters:
        sysflag - Optional system buffer filter selection. If either
            omitted or zero system buffers shall be filtered from the
            returned identifiers. Otherwise all buffers including
            system shall be returned.

        tab - Reserved for future use; tab identifier.

    Macro Returns:
        The 'previous_buffer()' primitive returns the buffer
        identifier of the previous buffer.

    Macro Portability:
        n/a

    Macro See Also:
        next_buffer, set_buffer, create_buffer, inq_buffer, inq_system
 */
void
do_next_buffer(int prev)        /* int ([int sysflag], [int previous], [int tab]) */
{
    const int sysbufs = get_xinteger(1, FALSE);
    const int previous = (prev ? TRUE : get_xinteger(2, FALSE));
 /* const int tab = get_xinteger(prev ? 2 : 3, -1);   * TABLINE/TODO */
    const BUFFER_t *bp;

    if (NULL == curbp) {
        acc_assign_int(-1);
        return;
    }

    if (previous) {
        /*
         *  previous buffer
         */
        for (bp = buf_prev(curbp);; bp = buf_prev((BUFFER_t *)bp)) {
            if (NULL == bp) {
                bp = buf_last();                /* loop */
            }
            if (!sysbufs) {
                if (k_isscrap(bp)) {
                    continue;                   /* scrap buffer */
                }
                if (BF2TST(bp, BF2_DIALOG)) {
                    continue;                   /* dialog buffer */
                }
            }
            if (bp == curbp || !BFTST(bp, BF_SYSBUF) || sysbufs) {
                break;                          /* match or complete */
            }
        }

    } else {
        /*
         *  next buffer
         */
        for (bp = buf_next(curbp);; bp = buf_next((BUFFER_t *)bp)) {
            if (NULL == bp) {
                bp = buf_first();               /* loop */
            }
            if (!sysbufs) {
                if (k_isscrap(bp)) {
                    continue;                   /* scrap buffer */
                }
                if (BF2TST(bp, BF2_DIALOG)) {
                    continue;                   /* dialog buffer */
                }
            }
            if (bp == curbp || !BFTST(bp, BF_SYSBUF) || sysbufs) {
                break;                          /* match or complete */
            }
        }
    }
    acc_assign_int((accint_t) bp->b_bufnum);
}


/*  Function:           do_set_buffer
 *      set_buffer primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: set_buffer - Set the current buffer.

        int
        set_buffer(int bufnum)

    Macro Description:
        The 'set_buffer()' primitive makes the buffer identifier
        specified by 'bufnum' the current buffer, without effecting
        the current window. The current is the one referenced by all
        buffer operations which are not given an explicit buffer
        identifier.

        Generally 'set_buffer' is utilised in one of two ways;

            o Temporarily changing the buffer so to perform specific
                buffer processing, for example searching for text, and
                on completion the previous is then restored to the
                current.

            o Changing the active buffer, which should also involve
                changing the current window using <set_window> or
                associating the new buffer with the current window using
                <attach_buffer>.

        The 'set_buffer()' primitive unlike <edit_file> does not
        cause any registered macros to be executed.

        Warning!:
        The referenced buffer does not always need to be attached to
        a window nor the one currently associated with the current
        window <set_window>, yet upon macro exit the current buffer
        and current window *should* be attached otherwise the
        side-effects may be disastrous.

    Macro Parameters:
        bufnum - Buffer identifier to be selected.

    Macro Returns:
        The 'set_buffer()' primitive returns the identifier of the
        previous current buffer otherwise -1 if an invalid buffer
        identifier was stated.

        On failure the following diagnostics message shall be echoed
        on the command prompt.

>           set_buffer: no such buffer

    Macro Portability:
        n/a

    Macro See Also:
        inq_buffer, create_buffer, next_buffer, previous_buffer
 */
void
do_set_buffer(void)             /* int (int bufnum) */
{
 /* const int tab = get_xinteger(1, -1);    * TABLINE/TODO */
    BUFFER_t *bp;

    if ((bp = buf_lookup(get_xinteger(1, 0))) == NULL) {
        ewprintf("set_buffer: no such buffer");
        acc_assign_int(-1);
        return;
    }
    acc_assign_int((accint_t)(curbp ? curbp->b_bufnum : 0));

//TODO
//  if (BFTST(curbp, BF_DELAYED)) {
//      <populate buffer>
//  } else if (BFTST(curbp, BF_VOLATILE)) {
//      <append-to-buffer>
//  }

    if (curwp && curwp->w_bufp == curbp && curbp) {
        curbp->b_line = curwp->w_line;
        curbp->b_col  = curwp->w_col;
        curbp->b_top  = curwp->w_top_line;
    }
    curbp = bp;
    set_hooked();
}


/*  Function:           inq_buffer
 *      inq_buffer primitive
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: inq_buffer - Retrieve a buffer identifier.

        int
        inq_buffer([string filename])

    Macro Description:
        The 'inq_buffer()' primitive retrieves either the identifier
        associated with the buffer containing the specified file
        'filename', otherwise if omitted the identifier of the
        current buffer.

    Macro Parameters:
        bufname - Optional string containing the file name to be matched
            against existing buffers.

    Macro Returns:
        The 'inq_buffer()' primitive returns the unique identifier
        associated with the referenced file or the current buffer if
        no 'file_name' was specified. If the specified 'file_name'
        was not matched, zero is returned.

        If omitted then the current buffer is returned.

    Macro Portability:
        Unlike BRIEF partial matches do not occur.

    Macro See Also:
        attach_buffer, create_buffer, delete_buffer, next_buffer, set_buffer
 */
void
inq_buffer(void)                /* int ([string file_name], [int tab/TODO]]) */
{
    if (isa_undef(1)) {                         /* current buffer */
        acc_assign_int(curbp ? (accint_t) curbp->b_bufnum : 0);

    } else {                                    /* otherwise by name */
        const BUFFER_t *bp = buf_find(get_str(1));
        acc_assign_int(bp ? (accint_t) bp->b_bufnum : 0);
    }
}


/*  Function:           do_create_edge
 *      create_edge primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: create_edge - Create an edge, splitting the window.

        int
        create_edge([int direction])

    Macro Description:
        The 'create_edge()' primitive creates a new edge, splitting
        the current window in half resulting in a new window.

        The window local in the specified 'direction' is split
        identified as follows,

            0 - Above/up.
            1 - Right.
            2 - Below/down.
            3 - Left.

        If 'direction' is omitted the user is prompted for the split
        direction; the user indicates the split direction by use of
        the arrow keys.

>           Select new side [<^v>]

        The selected window edge should have suitable screen space to
        permit the split operation otherwise the request is ignored
        and the user is informed as follows:

>           Window would be too small.

    Macro Parameters:
        direction - Optional integer direction stating the edge on which
            the split operation occurs creating the new window (as
            above), if omitted the user is prompted.

    Macro Returns:
        The 'create_edge()' primitive returns 1 on success, 0 if the
        window was too small too split, otherwise -1 if the user
        aborted.

    Macro Portability:
        n/a

    Macro See Also:
        delete_edge, move_edge
 */
void
do_create_edge(void)            /* int ([int direction]) */
{
    const int i = get_dir(1, "Select new side");
    int ret = -1;

    if (i >= 0) {
        WINDOW_t *wp;

        eclear();
        if (WD_LEFT == i || WD_RIGHT == i) {
            wp = window_vsplit();
        } else {
            wp = window_hsplit();
        }

        if (wp) {
            if (WD_RIGHT == i || WD_DOWN == i) {
                curwp = wp;
                set_hooked();
            }
            ret = 1;
        } else {
            ret = 0;
        }
    }
    acc_assign_int(ret);
}


/*  Function:           do_delete_edge
 *      delete_edge primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: delete_edge - Delete an edge, combining a split window.

        int
        delete_edge([int direction])

    Macro Description:
        The 'delete_edge()' primitive deletes an edge, combined two
        windows sharing an adjoining edge into a single window.

        The windows located in the specified 'direction' are joined
        as follows,

            0 - Above/up.
            1 - Right.
            2 - Below/down.
            3 - Left.

        If 'direction' is omitted the user is prompted for the split
        direction; the user indicates the split direction by use of
        the arrow keys.

>           Select edge to delete [<^v>]

        Once a window is deleted with 'delete_edge', its window
        identifier become invalid.

    Macro Parameters:
        direction - Optional integer direction stating the edge on which
            the join operation occurs deleting the associated windows
            (as above), if omitted the user is prompted.

    Macro Returns:
        The 'delete_edge()' primitive returns 1 on success, 0 if the edge
        does exist, otherwise -1 if the user aborted.

    Macro Portability:
        n/a

    Macro See Also:
        create_edge, move_edge
 */
void
do_delete_edge(void)            /* int ([int direction]) */
{
    int ret = -1;

    if (W_TILED != curwp->w_type) {
        ret = 0;                                /* non-tiled, no edges */

    } else {
        const int i = get_dir(1, "Select edge to delete");

        if (i >= 0) {
            WINDOW_t *adj_wp;

            if (NULL == (adj_wp = get_edge(i))) {
                ret = 0;                        /* invalid edge */

            } else {
                switch (i) {
                case WD_UP:
                    curwp->w_y = adj_wp->w_y;
                    curwp->w_h = (uint16_t)(curwp->w_h + adj_wp->w_h + 1);
                    break;
                case WD_DOWN:
                    curwp->w_h = (uint16_t)(curwp->w_h + adj_wp->w_h + 1);
                    break;
                case WD_LEFT:
                    curwp->w_x = adj_wp->w_x;
                    curwp->w_w = (uint16_t)(curwp->w_w + adj_wp->w_w + 1);
                    break;
                case WD_RIGHT:
                    curwp->w_w = (uint16_t)(curwp->w_w + adj_wp->w_w + 1);
                    break;
                }

                if (1 == adj_wp->w_bufp->b_nwnd) {
                    adj_wp->w_bufp->b_line = adj_wp->w_line;
                    adj_wp->w_bufp->b_col = adj_wp->w_col;
                }

                window_delete(adj_wp);
                vtupdate();
                eclear();
                ret = 1;                        /* success */
            }
        }
    }
    acc_assign_int(ret);
}


static int
get_dir(int argi, const char *str)
{
    char msg[MAX_CMDLINE] = {0};
    KEY c;

    /*
     *  If argument passed to macro, then make sure its in range.
     */
    if (isa_integer(argi)) {
        const int i = get_xinteger(argi, 0);
        return (i >= 0 && i <= WD_MAX) ? i : -1;
    }

    /*
     *  Otherwise prompt for the direction.
     *      allow user to abort the selection (by typing <Esc>)
     */
    sxprintf(msg, sizeof(msg), "%s [<^v>]", str);
    while (1) {
        ewprintf("%s", msg);
        c = (KEY)io_get_key(0);
        switch (c) {
        case KEY_UP:
        case WHEEL_UP:
        case '^':
            return WD_UP;
        case KEY_DOWN:
        case WHEEL_DOWN:
        case 'v':
            return WD_DOWN;
        case KEY_LEFT:
        case '<':
            return WD_LEFT;
        case KEY_RIGHT:
        case '>':
            return WD_RIGHT;
        case F(1):
            return WD_MOVE;
        case KEY_ESC:
            return -1;
        }
        sxprintf(msg, sizeof(msg), "%s (use cursor keys)", str);
    }
}


/*  Function:           do_move_edge
 *      move_edge primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: move_edge - Modify a window.

        int
        move_edge([int direction], [int amount])

    Macro Description:
        The 'move_edge()' primitive modifies the edges of tiled window,
        whereas for popup's allow the user to increase or decrease the
        size, and also move the window around the screen.

    Macro Example:
        Moves the lower edge of the current window up four lines.

>           move_edge(2, -4);

    Macro Parameters:
        direction - Optional integer direction stating the edge on which
            the move operation occurs resizing the associated windows
            (as above), if omitted the user is prompted.

        amount - Optional integer expression stating the number or
            characters or lines to move the window. The number is
            relative to the top left of the screen, so positive numbers
            move away from the origin (0, 0) and negative numbers
            towards the origin. If not specified, the user will be
            prompted to move the edge with the arrow keys.

    Macro Returns:
        The 'move_edge()' primitive returns non-zero if cursor moved, 
        otherwise 0 if cursor did not move.

    Macro Portability:
        n/a

    Macro See Also:
        create_edge, delete_edge
 */
void
do_move_edge(void)              /* int ([int direction], [int amount]) */
{
    const int amount_supplied = isa_integer(2);
    int num_to_move = get_xinteger(2, 0);
    WINDOW_t *adj_wp = curwp;
    int ch, i = 0;

    acc_assign_int(0);

    if (W_MENU == curwp->w_type) {
        return;
    }

    if (W_TILED == curwp->w_type) {
        i = get_dir(1, "Select an edge to move");
        if (NULL == (adj_wp = get_edge(i))) {
            return;
        }
        if (! amount_supplied) {
            ewprintf("Move to new edge position and press Enter.");
        }

    } else if (! amount_supplied) {
        ewprintf("Use arrow keys to move window.");
    }

    acc_assign_int(1);
    while (1) {
        if (amount_supplied) {
            if (0 == num_to_move)  {
                break;
            }

            if (i == WD_UP || i == WD_DOWN) {
                if (num_to_move > 0) {
                    ch = KEY_DOWN;
                    --num_to_move;
                } else {
                    ch = KEY_UP;
                    ++num_to_move;
                }
            } else {
                if (num_to_move > 0) {
                    ch = KEY_RIGHT;
                    --num_to_move;
                } else {
                    ch = KEY_LEFT;
                    ++num_to_move;
                }
            }
        } else {
            if ((ch = io_get_key((accint_t) 0)) == '\n' || ch == '\r') {
                break;
            }
        }

        if (W_POPUP == curwp->w_type) {
            switch (ch) {
            case KEY_WUP:
                if (curwp->w_y > 0) {
                    ++curwp->w_h;
                    --curwp->w_y;
                }
                break;

            case KEY_UP:
            case WHEEL_UP:
                if (curwp->w_y > 0) {
                    --curwp->w_y;
                }
                break;

            case KEY_WDOWN:
                if (curwp->w_y + curwp->w_h < ttrows()) {
                    ++curwp->w_h;
                }
                break;

            case KEY_DOWN:
            case WHEEL_DOWN:
                if (curwp->w_y + curwp->w_h < ttrows()) {
                    ++curwp->w_y;
                }
                break;

            case KEY_WLEFT:
            case KEY_WLEFT2:
                if (curwp->w_w > 4) {
                    --curwp->w_w;
                }
                break;

            case KEY_LEFT:
                if (curwp->w_x > 1) {
                    --curwp->w_x;
                }
                break;

            case KEY_RIGHT:
                /* Keep at least 4 columns of window on screen. */
                if (curwp->w_x + 4 < ttcols()) {
                    ++curwp->w_x;
                }
                break;

            case KEY_WRIGHT:
            case KEY_WRIGHT2:
                ++curwp->w_w;
                break;
            }

        } else {
            switch (i) {
            case WD_UP:
                if (ch == KEY_UP && adj_wp->w_h > 1) {
                    --curwp->w_y;
                    ++curwp->w_h;
                    --adj_wp->w_h;
                } else if (ch == KEY_DOWN && curwp->w_h > 1) {
                    ++adj_wp->w_h;
                    ++curwp->w_y;
                    --curwp->w_h;
                }
                break;

            case WD_DOWN:
                if (ch == KEY_DOWN && adj_wp->w_h > 1) {
                    ++curwp->w_h;
                    ++adj_wp->w_y;
                    --adj_wp->w_h;
                } else if (ch == KEY_UP && curwp->w_h > 1) {
                    ++adj_wp->w_h;
                    --adj_wp->w_y;
                    --curwp->w_h;
                }
                break;

            case WD_LEFT:
                if (ch == KEY_LEFT && adj_wp->w_w > 16) {
                    --adj_wp->w_w;
                    ++curwp->w_w;
                    --curwp->w_x;
                } else if (ch == KEY_RIGHT && curwp->w_w > 16) {
                    ++adj_wp->w_w;
                    --curwp->w_w;
                    ++curwp->w_x;
                }
                break;

            case WD_RIGHT:
                if (ch == KEY_RIGHT && adj_wp->w_w > 16) {
                    --adj_wp->w_w;
                    ++adj_wp->w_x;
                    ++curwp->w_w;
                } else if (ch == KEY_LEFT && curwp->w_w > 16) {
                    ++adj_wp->w_w;
                    --adj_wp->w_x;
                    --curwp->w_w;
                }
                break;
            }
        }

        /*
         *  If we're moving a popup window, then do the hard work of working out what's
         *  changed on the screen. If it is not a popup then just mark the two windows
         *  involved as needing some work done to them.
         */
        window_sort();
        if (W_POPUP == curwp->w_type) {
            window_harden();
        } else {
            window_corners();
            adj_wp->w_status |= WFHARD;
            win_modify(WFHARD);
        }
        vtupdate();
    }
    eclear();
}


static WINDOW_t *
get_edge(int dir)
{
    register WINDOW_t *wp;

    if (dir < 0) {
        return NULL;
    }

    if (W_TILED != curwp->w_type) {
        return curwp;
    }

    if (WD_UP == dir || WD_DOWN == dir) {
        for (wp = window_first(); wp; wp = window_next(wp))
            if (wp->w_w == curwp->w_w && wp->w_x == curwp->w_x &&
                    ((WD_UP    == dir && wp->w_y + wp->w_h + 1       == curwp->w_y) ||
                     (WD_DOWN  == dir && curwp->w_y + curwp->w_h + 1 == wp->w_y))) {
                return wp;
            }

    } else {
        for (wp = window_first(); wp; wp = window_next(wp))
            if (wp->w_h == curwp->w_h && wp->w_y == curwp->w_y &&
                    ((WD_LEFT  == dir && wp->w_x + wp->w_w + 1       == curwp->w_x) ||
                     (WD_RIGHT == dir && curwp->w_x + curwp->w_w + 1 == wp->w_x))) {
                return wp;
            }
    }

    errorf("Edge does not have just two adjoining windows.");
    return NULL;
}


/*  Function:           do_change_window
 *      change_window primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: change_window - Selects a new window.

        void
        change_window([int direction], [string message])

    Macro Description:
        The 'change_window()' primitive selects an adjoining window
        as the current located in the specified 'direction'
        identified as follows,

            0 - Above/up.
            1 - Right.
            2 - Below/down.
            3 - Left.

        If 'direction' is omitted the user is prompted; the user
        indicates the change direction by use of the arrow keys.

>           Change direction [<^V>]

        If the selected edge has no other window associated, then
        the user is informed as follows:

>           No window available

    Macro Parameters:
        direction - Optional integer direction stating the edge on
            which the change operation occurs resizing the
            associated window (as above), if omitted the user is
            prompted.

        message - Optional message string to be used as the prompt,
            if omitted the default of "Change direction" is used.

    Macro Returns:
        The 'change_window()' primitive returns 1 on success, 0 if
        the edge does exist, otherwise -1 if the user aborted.

    Macro Portability:
        n/a

    Macro See Also:
        set_window, next_window
 */
void
do_change_window(void)          /* ([int direction], [string message]) */
{
    const char *cp = get_str(2);
    int i, ret = -1;

    if (NULL == cp || !*cp) cp = "Change direction";

    if ((i = get_dir(1, cp)) < 0) {
        eclear();

    } else {
        WINDOW_t *wp;

        if (NULL == (wp = get_window(i))) {
            ret = 0;

        } else if (W_MENU == wp->w_type) {
            errorf("No window available; only the menu.");
            ret = 0;

        } else {
            buf_change_window(wp);
            eclear();
            ret = 1;
        }
    }
    acc_assign_int(ret);
}


/*  Function:           get_window
 *      Determine the window to the required direction from the current.
 *
 *  Parameters:
 *      dir - Direction.
 *
 *  Returns:
 *      nothing
 */
static WINDOW_t *
get_window(int dir)
{
    const int line =
        win_tline(curwp)   + (curwp->w_line - curwp->w_top_line);
    const int col =
        win_fcolumn(curwp) + (curwp->w_col  - curwp->w_left_offset);
    WINDOW_t *wp, *nwp = NULL;

    ED_TRACE(("get_window(line:%d,col:%d", line, col));
    switch (dir) {
    case WD_UP:
        for (wp = window_first(); wp; wp = window_next(wp)) {
            ED_TRACE(("\tup,col:%d,%d-%d,%d\n",
                win_lborder(wp), win_lcolumn(wp), win_rcolumn(wp), win_rborder(wp)));

            if (wp != curwp && wp->w_y < curwp->w_y &&
                    col >= win_lcolumn(wp) && col <= (win_rcolumn(wp) + win_rborder(wp))) {
                ED_TRACE("\t\tmatch\n");
                if (NULL == nwp) {
                    nwp = wp;
                } else if (nwp->w_y < wp->w_y) {
                    nwp = wp;
                }
            }
        }
        break;

    case WD_DOWN:
        for (wp = window_first(); wp; wp = window_next(wp)) {
            ED_TRACE(("\tdn,col:%d,%d-%d,%d\n",
                win_lborder(wp), win_lcolumn(wp), win_rcolumn(wp), win_rborder(wp)));

            if (wp != curwp && wp->w_y > curwp->w_y &&
                    col >= win_lcolumn(wp) && col <= (win_rcolumn(wp) + win_rborder(wp))) {
                ED_TRACE(("\t\tmatch\n"));
                if (NULL == nwp) {
                    nwp = wp;
                } else if (nwp->w_y > wp->w_y) {
                    nwp = wp;
                }
            }
        }
        break;

    case WD_LEFT:
        for (wp = window_first(); wp; wp = window_next(wp))
            if (wp != curwp && wp->w_x < curwp->w_x &&
                    line >= win_tline(wp) && line <= (win_bline(wp) + win_bborder(wp))) {
                if (NULL == nwp) {
                    nwp = wp;
                } else if (nwp->w_x < wp->w_x) {
                    nwp = wp;
                }
            }
        break;

    case WD_RIGHT:
        for (wp = window_first(); wp; wp = window_next(wp))
            if (wp != curwp && wp->w_x > curwp->w_x &&
                    line >= win_tline(wp) && line <= (win_bline(wp) + win_bborder(wp))) {
                if (NULL == nwp) {
                    nwp = wp;
                } else if (nwp->w_x > wp->w_x) {
                    nwp = wp;
                }
            }
        break;

    case WD_MOVE:
        if (NULL == (nwp = window_next(curwp))) {
            nwp = window_first();
        }
        break;
    }

    if (nwp) {
        acc_assign_int(1);
        return nwp;
    }

    errorf("No window available.");
    acc_assign_int(0);
    return 0;
}


/*  Function:           do_save_position
 *      save_position primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: save_position - Saves current cursor/buffer state.

        void
        save_position()

    Macro Description:
        The 'save_position()' primitive pushes the current buffer, 
        window and cursor position into the position stack, allowing
        them to be restored later using <restore_position>.

        As these states are maintained by a stack in LIFO (Last In
        First Out) order each invocation of 'save_position' should
        have a corresponding invocation of <restore_position>, 
        otherwise saved positions shall accumulate consuming system
        resources.

        Note!:
        That the position stack is an independent of any buffer
        permitting <restore_position> to be called to pop off the top
        entry of the stack even when the current buffer is not the
        same as buffer referenced by the top of the saved position
        stack.

    Macro Parameters:
        none

    Macro Returns:
        nothing

    Macro Portability:
        n/a

    Macro See Also:
        restore_position
 */
void
do_save_position(void)          /* void () */
{
    position_save();
}


/*  Function:           do_restore_position
 *      restore_position primitive
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: restore_position - Restore a previously saved position.

        int
        restore_position([int what = 1])

    Macro Description:
        The 'restore_position()' primitive restores a previously
        saved position from the position stack. The argument 'what'
        is an optional integer expression which controls the state
        that is restored.

        When states 'what', specifies the information which is
        restored. If omitted what is equivalent to one;

            0 - Nothing, with the save information being discarded.

            1 - The cursor position is restored.

            2 - The buffer is restored, with the cursor located in the
                current window at its previous position.

            3 - Reserved.

            4 - The previous buffer and window are restored, with the
                cursor located at its previous position.

    Macro Parameters:
        what - Optional integer states the what elements of the
            position state to be restored.

    Macro Returns:
        The 'restore_position()' primitive returns 1 on success,
        otherwise zero or less on error.

    Macro Portability:
        n/a

    Macro See Also:
        save_position
 */
void
do_restore_position(void)       /* int ([int what = 1]) */
{
    const int what = get_xinteger(1, 1);
    int ret = position_restore(what);

    acc_assign_int(ret);
}


/*  Function:           inq_views
 *      inq_views primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: inq_views - Determine window count.

        int
        inq_views([int bufnum])

    Macro Description:
        The 'inq_views()' primitive determines the number of windows that
        are viewing the specified buffer 'bufnum'.

    Macro Parameters:
        bufnum - Optional buffer number, if omitted the current buffer
            shall be referenced.

    Macro Returns:
        The 'inq_views()' primitive returns the number of windows attached
        to the specified buffer, otherwise 0 on error.

    Macro Portability:
        n/a

    Macro See Also:
        set_window
 */
void
inq_views(void)                 /* int ([int bufnum]) */
{
    const BUFFER_t *bp = buf_argument(1);
    accint_t val = (bp ? (accint_t)bp->b_nwnd : 0);

    acc_assign_int(val);
}


/*  Function:           inq_lines
 *      inq_lines primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: inq_lines - Retrieve the line count.

        int
        inq_lines([int bufnum])

    Macro Description:
        The 'inq_lines()' primitive returns the current line number of the
        specified buffer 'bufnum'.

    Macro Parameters:
        bufnum - Optional buffer number, if omitted the current buffer
            shall be referenced.

    Macro Returns:
        The 'inq_lines()' primitive returns the line count within the
        referenced buffer, otherwise -1 on error.

    Macro Portability:
        n/a

    Macro See Also:
        inq_line_length
 */
void
inq_lines(void)                 /* ([int bufnum]) */
{
    const BUFFER_t *bp = buf_argument(1);
    const accint_t val = (bp ? bp->b_numlines : -1);    /* NEWLINE */
    acc_assign_int(val);
}


/*  Function:           inq_line_length
 *      inq_line_length primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: inq_line_length - Determine the longest line length.

        int
        inq_line_length([int bufnum])

    Macro Description:
        The 'inq_line_length()' primitive determines the length of
        the longest line within the specified buffer 'bufnum'.

        The calculate line length corresponds to the logical column
        position at the end of the line, taking into account any tabs
        and control characters.

        If the designated buffer contains a marked region, then only
        the lines within the marked region are including within the
        result.

    Macro Parameters:
        bufnum - Optional buffer number, if omitted the current buffer
            shall be referenced.

    Macro Returns:
        The 'inq_line_length()' primitive returns the longest line within
        the referenced buffer or region, otherwise -1 on error.

    Macro Portability:
        Unlike BRIEF the longest current line is returned. BRIEF
        returned the upper global line length rounded up the the next
        multiple of 16, for example 202 would have been rounded to 208,
        not a buffer specific value.

    Macro See Also:
        inq_lines
 */
void
inq_line_length(void)           /* int ([int bufnum]) */
{
    const BUFFER_t *bp = buf_argument(1);

    if (NULL == bp) {
        acc_assign_int(-1);
        return;
    }
    acc_assign_int((accint_t) buf_line_length(bp, TRUE));
}


/*  Function:           do_sort_buffer
 *      sort_buffer primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: sort_buffer - Sort buffer content

        int
        sort_buffer([int bufnum], [string|int comparator = 0],
            [int start], [int end], [int type = 3])

    Macro Description:
        The 'sort_buffer()' primitive sorts the lines in the current
        buffer or the buffer specified by 'bufnum'. If the buffer
        specified has a region marked, then only those lines within
        the region are sorted.

        By default lines are sorted alphabetically yet the sort can
        be modified using the user specified macro or using one of
        the predefined system sort macros.

    Macro Parameters:
        bufnum - Optional buffer number, if omitted the current buffer
            shall be referenced.

        comparator - Optional string comparison macro or integer
            direction. A string value states the comparison macro to
            be executed. Whereas an integer value states the
            direction, with zero selecting the built
            "sort_buffer::forward" comparator and a non-zero value
            selecting "sort_buffer::backwards". If omitted the
            comparator defaults to forward.

        start - Optional integer line number stating the start of the
            region to be sorted, if omitted the buffer top is used
            unless a marked region is active.

        end - Optional integer line number stating the end of the region
            to be sorted, if omitted the buffer end is used unless a
            marked region is active.

        type - Optional integer stating the sort type, being

                1 - quicksort.
                2 - mergesort
                3 - heapsort (default).

    Macro Returns:
        The 'sort_buffer()' primitive returns the number of lines
        sorted, otherwise a negative value on error.

    Macro Portability:
        Second argument allowing either a sort-order or user specified
        callback plus type selection are Grief extensions.

    Macro See Also:
        sort_list
 */
void
do_sort_buffer(void)            /* ([int bufnum], [string macro],
                                        [int start], [int end], [int type]) */
{
    BUFFER_t *bp = buf_argument(1);
    const char *macro = get_xstr(2);
    const int type = get_xinteger(5, -1);       /* 2010 */
    sortcmp_t cmp = NULL;
    ANCHOR_t anchor = {0};
    LINENO startline, endline, numlines, n;
    LINE_t **lps, *lp, *nlp;

    /* select/size arena */
    if (NULL == bp) {
        acc_assign_int(-1);
        return;

    } else if (0 == BFTST(bp, BF_NO_UNDO) || file_rdonly(bp, "sort_buffer")) {
        acc_assign_int(-2);
        return;
    }

    startline = (LINENO) get_xinteger(3, -1);
    if (startline > 0) {                        /* explicit region */
        endline = get_xinteger(4, bp->b_numlines);

    } else if (bp && anchor_get(NULL, bp, &anchor)) {
        startline = anchor.start_line;
        endline = anchor.end_line;

    } else {                                    /* full buffer */
        startline = 1;
        endline = bp->b_numlines;
    }
    if (startline > endline) {
        GR_SWAP(startline, endline, n);
    }

    if (startline < 1) {
        startline = 1;
    }
    if (endline > bp->b_numlines) {
        endline = bp->b_numlines;
    }
    numlines = (endline - startline) + 1;
    if (numlines <= 1) {
        acc_assign_int(0);
        return;
    }

    /* order */
    if (macro && macro[0]) {
        if (0 == strcmp(macro, "sort_buffer::forward")) {
            cmp = sortcompare_forward;
            macro = NULL;

        } else if (0 == strcmp(macro, "sort_buffer::backward")) {
            cmp = sortcompare_backward;
            macro = NULL;

        } else {
            if (! macro_exist(macro, "sort_buffer")) {
                acc_assign_int(-3);
                return;
            }
            cmp = NULL;
        }
    } else {
        cmp = (get_xinteger(2, 0) ?             /* default forward=0 */
                    sortcompare_backward : sortcompare_forward);
        macro = NULL;
    }

    if (NULL == (lps = chk_alloc(numlines * sizeof(LINE_t *)))) {
        acc_assign_int(-4);
        return;
    }

    /* sort */
    for (n = 0, lp = linepx(bp, startline); n < numlines;) {
        nlp = lforw(lp);
        TAILQ_REMOVE(&bp->b_lineq, lp, l_node);
        --bp->b_numlines;
        lps[n++] = lp;
        lp = nlp;
    }
    linep_flush(bp);

    switch (type) {
    case 3:     /* unstable heapsort */
        if (cmp) {
            bsd_heapsort(lps, numlines, sizeof(LINE_t *), cmp);
        } else {
            bsd_heapsort_r(lps, numlines, sizeof(LINE_t *), (void *)macro, sortcompare_macro);
        }
        break;
    case 2:     /* stable merge sort */
    default:    /* +2.5.5 */
        if (cmp) {
            bsd_mergesort(lps, numlines, sizeof(LINE_t *), cmp);
        } else {
            bsd_mergesort_r(lps, numlines, sizeof(LINE_t *), (void *)macro, sortcompare_macro);
        }
        break;
    case 1:     /* unstable qsort */
        if (cmp) {
            bsd_qsort(lps, numlines, sizeof(LINE_t *), cmp);
        } else {
            bsd_qsort_r(lps, numlines, sizeof(LINE_t *), (void *)macro, sortcompare_macro);
        }
        break;
    }

    /* reinsert */
    if (startline <= 1) {
        for (n = numlines; n > 0;) {
            nlp = lps[--n];
            TAILQ_INSERT_HEAD(&bp->b_lineq, nlp, l_node);
            ++bp->b_numlines;
        }

    } else {
        lp = linepx(bp, startline - 1);
        for (n = 0; n < numlines;) {
            nlp = lps[n++];
            TAILQ_INSERT_AFTER(&bp->b_lineq, lp, nlp, l_node);
            ++bp->b_numlines;
            lp = nlp;
        }
    }
    linep_flush(bp);

    chk_free(lps);
    win_modify(WFHARD);
    acc_assign_int((accint_t) numlines);
}


/*  Function:           sortcompare_fwd
 *      do_sort_buffer worker function.
 *
 *  Parameters:
 *      l1 - First line image.
 *      l2 - Second image.
 *
 *  Returns:
 *      Comparison result.
 */
static int
sortcompare_forward(const void *l1, const void *l2)
{
    const LINE_t *line1 = *((const LINE_t **)l1), *line2 = *((const LINE_t **)l2);
    const LINECHAR *cp1 = ltext(line1), *cp2 = ltext(line2);
    LINENO len = llength(line1);
    int ret;

    if ((LINENO)llength(line2) < len) {
        len = llength(line2);
    }

    while (len-- > 0) {
        ret = *cp1++ - *cp2++;
        if (ret) {
            return ret;
        }
    }
    return llength(line1) - llength(line2);
}


static int
sortcompare_backward(const void *l1, const void *l2)
{
    const int ret = sortcompare_forward(l1, l2);

    if (ret > 0) {
        return -1;
    } else if (ret < 0) {
        return 1;
    }
    return 0;
}


/*  Function:           sortcompare_macro
 *      sort_buffer worker function.
 *
 *  Parameters:
 *      l1 - First line image.
 *      l2 - Second image.
 *
 *  Returns:
 *      Comparison result.
 */
static int
sortcompare_macro(void *callback, const void *l1, const void *l2)
{
    const LINE_t *line1 = *((const LINE_t **)l1), *line2 = *((const LINE_t **)l2);
    LIST tmpl[LIST_SIZEOF(5)], *lp = tmpl;      /* 5 atoms */
    const void *p;

    lp = atom_push_sym(lp, callback);           /* macro-name */
    lp = atom_push_int(lp, (accint_t) llength(line1));
    lp = atom_push_const(lp, (NULL == (p = ltext(line1)) ? "" : p));
    lp = atom_push_int(lp, (accint_t) llength(line2));
    lp = atom_push_const(lp, (NULL == (p = ltext(line2)) ? "" : p));
    atom_push_halt(lp);

    assert(lp < (tmpl + sizeof(tmpl)));
    acc_assign_int(0);
    execute_nmacro(tmpl);                      /* execute callback */
    return (int)acc_get_ival();
}
/*eof*/
