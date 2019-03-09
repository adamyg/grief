#include <edidentifier.h>
__CIDENT_RCSID(gr_playback_c,"$Id: playback.c,v 1.32 2019/01/26 22:27:08 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: playback.c,v 1.32 2019/01/26 22:27:08 cvsuser Exp $
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
#include <libstr.h>                             /* str_...()/sxprintf() */

#include "playback.h"                           /* public interface */

#include "accum.h"
#include "buffer.h"
#include "builtin.h"
#include "echo.h"
#include "eval.h"
#include "file.h"
#include "keyboard.h"
#include "line.h"
#include "main.h"
#include "symbol.h"
#include "undo.h"

#define PLAYBACK_INCR   32                      /* Keystroke chunk size */

typedef TAILQ_HEAD(_PlaybackList, _playback)
                        PLAYBACKLIST_t;

typedef struct _playback {
    MAGIC_t             p_magic;
#define PLAYBACK_MAGIC      MKMAGIC('R','e','G',' ')
    TAILQ_ENTRY(_playback)
                        p_node;
    IDENTIFIER_t        p_ident;                /* Keystroke macro identifier. */
    IDENTIFIER_t        p_bufnum;               /* Buffer containing disassembled version of keys. */
    ref_t *             p_ref;                  /* Buffer containing keystrokes. */
    unsigned            p_cursor;               /* Index for next character to retrieve. */
} playback_t;

static playback_t *     playback_get(int id, int create_flag);

static PLAYBACKLIST_t   x_playback_queue;

static IDENTIFIER_t     x_playback_ident = 0;

static playback_t *     x_playback_current = NULL;
static int              x_playback_recording = FALSE;
static int              x_playback_playing = FALSE;

static int              rem_nest_level = -1;
static int              rem_doing_self_insert;

const char *            x_rem_string = "  ";


void
playback_init(void)
{
    TAILQ_INIT(&x_playback_queue);
}


enum playbackstate
playback_status(void)
{
    if (x_playback_recording) {
        return PLAYBACK_RECORDING;

    } else if (x_playback_playing) {
        return PLAYBACK_PLAYING;
    }
    return PLAYBACK_NONE;
}


void
playback_shutdown(void)
{
    PLAYBACKLIST_t *pq = &x_playback_queue;
    playback_t *pb;

    while (NULL != (pb = TAILQ_FIRST(pq))) {
        assert(PLAYBACK_MAGIC == pb->p_magic);
        TAILQ_REMOVE(pq, pb, p_node);
        pb->p_magic = 0xDEADBEEF;
        chk_free((void *)pb);
    }
}


/*
 *  Return pointer to a playback structure. create_flag says whether we should create entry
 *  if not already there. The first arg is playback macro index.
 *
 *  If not specified then use the last macro (or next one).
 */
static playback_t *
playback_get(int id, int create_flag)
{
    PLAYBACKLIST_t *pq = &x_playback_queue;
    playback_t *pb = NULL;
    int search = TRUE;

    if (id < 0) {
        if (create_flag) {
            id = ++x_playback_ident;            /* implied creation */
            search = FALSE;
        } else {
            id = x_playback_ident;              /* current */
        }
    }

    if (search) {
        TAILQ_FOREACH(pb, pq, p_node) {
            assert(PLAYBACK_MAGIC == pb->p_magic);
            if (pb->p_ident == id) {
                break;
            }
        }
    }

    if (NULL == pb)  {
        if (create_flag &&
                NULL != (pb = (playback_t *) chk_alloc(sizeof(playback_t)))) {
            pb->p_magic  = PLAYBACK_MAGIC;
            pb->p_ident  = id;
            pb->p_bufnum = -1;
            pb->p_ref    = NULL;
            pb->p_cursor = 0;
            TAILQ_INSERT_HEAD(pq, pb, p_node);
        }
    }

    return pb;
}


/*  Function:           do_pause
 *      pause primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: pause - Pause keystroke definition.

        void
        pause()

    Macro Description:
        The 'pause()' primitive pauses a keyboard macro definition.

        Usually all keyboard input typed during a <remember> sequence is
        saved in the keyboard macro buffer. Pressing a key assigned to
        'pause' causes the <remember> sequence to suspend saving the
        characters.

    Macro Parameters:
        none

    Macro Returns:
        nothing

    Macro Portability:
        n/a

    Macro See Also:
        remember
*/
void
do_pause(void)                  /* void () */
{
    if (x_playback_recording) {
        x_rem_string = (x_rem_string[0] == 'P' ? "RE" : "PA");

    } else if (x_playback_playing) {
        x_rem_string = (x_rem_string[0] == 'P' ? "  " : "PA");

    } else {
        return;
    }
    elinecol(LC_FORCE);
}


/*  Function:           do_remember
 *      remember primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: remember - Start remembering keystrokes.

        int
        remember([string|int overwrite], [int macroid])

    Macro Description:
        The 'remember()' primitive control macro recording, starts
        remembering keystrokes, which can be later replayed with the
        <playback> function.

        Recording is stopped by a second call to the 'remember'
        macro.

        The 'remember()' primitive is not usually called as part of a
        macro but is usually bound to a keyboard key, for example (<F7>).

        Unlike BRIEF multiple keystroke macros can be maintained.
        Each remember execution shall create a buffer named
        'KBD-MACRO-#' where '#' is the associated keyboard macro
        identifier. This buffer maybe then edited and/or saved for
        later use. The <inq_remember_buffer> primitive shall derive
        the buffer name.

        Note!:
        Only the macros directly executed by the user are saved
        within the macro. In other words top level macros, for
        example the resulting dialog, shall not be reported.

        The <pause> primitive temporarily stops recording the current
        keystroke sequence; by default assigned to (<Shift+F7>). This
        feature is useful if part of the keystroke macro being
        remembered is different each time the macro is played back.
        Before the variable part of the macro, press *Shift+F7*.
        Perform the variable part and press *Shift+F7* to resume
        recording.

        When the macro is played back, it will pause at the point
        *Shift+F7* was pressed to allow the user to perform the
        variable portion of the sequence. Pressing Shift+F7 resumes
        playback.

    Macro Parameters:
        overwrite - Optional string containing the whether or not the
            current macro should be overwritten. If either a case
            insensitive string contained "y[es]" or a non-zero the
            macro shall be overwritten. If omitted the user shall
            be prompted.

        macroid - Optional integer macro identifier specifies the
            keyboard against which to store the keystrokes, if
            omitted the next free keyboard identifier is utilised.

    Macro Returns:
        The 'remember()' primitive returns the positive identifier
        assigned to the keystroke macro on completion, 0 if the
        remember was cancelled, otherwise -1 on error or the
        recording began.

    Macro Portability:
        n/a

    Macro See Also:
        pause
*/
void
do_remember(void)               /* int ([string|int overwrite], [int macroid]) */
{
    const char *overwrite = get_xstr(1);
    const char ch =
            (overwrite ? *overwrite :
                (isa_integer(1) ? (get_integer(1) ? 'y' : 'n') : 0));

    acc_assign_int((accint_t) -1);
    if (x_playback_playing) {
        return;
    }

    if (! x_playback_recording) {
        if (NULL != x_playback_current) {
            if (ch != 'y' && ch != 'Y' &&
                    !eyorn("Overwrite existing keystroke macro")) {
                acc_assign_int((accint_t) 0);
                return;
            }
        }

        if (NULL == (x_playback_current =
                playback_get(get_xinteger(2, -1), TRUE))) {
            return;
        }
    }

    if (x_playback_current->p_bufnum < 0) {
        char path[MAX_PATH], buf[32];
        BUFFER_t *bp;

        sxprintf(buf, sizeof(buf), "KBD-MACRO-%d", x_playback_current->p_ident);
        if (NULL == (bp =
                buf_find_or_create(file_canonicalize(buf, path, sizeof(path))))) {
            return;
        }
        BFSET(bp, BF_SYSBUF);
        BFSET(bp, BF_NO_UNDO);
        BFSET(bp, BF_READ);
        x_playback_current->p_bufnum = bp->b_bufnum;
    }

    if (! x_playback_recording) {
        if (NULL == x_playback_current->p_ref) {
            x_playback_current->p_ref = r_string("");
            if (ch == 'n' || ch == 'N') {
                return;
            }
            ewprintf("Defining keystroke macro.");
            x_rem_string = "RE";
        }
        x_playback_recording = TRUE;
        buf_clear(buf_lookup(x_playback_current->p_bufnum));
        rem_nest_level = -1;
        rem_doing_self_insert = FALSE;
        r_clear(x_playback_current->p_ref);
        x_playback_current->p_cursor = 0;

    } else {
        x_playback_recording = FALSE;
        ewprintf("Keystroke macro defined.");
        x_rem_string = "  ";
        x_playback_current->p_cursor = 0;
        acc_assign_int(x_playback_current->p_ident);
    }

    elinecol(LC_FORCE);
}


/*  Function:           inq_remember_buffer
 *      inq_remember_buffer primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: inq_remember_buffer - Determine the keystroke buffer name.

        string
        inq_remember_buffer([int macroid])

    Macro Description:
        The 'inq_remember_buffer()' primitive derives the buffer name
        associated with the keyboard macro 'macro'.

        Unlike BRIEF multiple keystroke macros can be maintained.
        Each remember execution shall create a buffer named
        'KBD-MACRO-#' where '#' is the associated keyboard macro
        identifier. This buffer maybe then edited and/or saved to
        later use.

    Macro Parameters:
        macroid - Optional integer macro identifier specifies the
            keyboard against which to derive the buffer name, if
            omitted the current macro identifier is utilised.

    Macro Returns:
        The 'inq_remember_buffer()' primitive returns the buffer name
        associated with the remember buffer.

    Macro Portability:
        n/a

    Macro See Also:
        remember
*/
void
inq_remember_buffer(void)       /* ([int macroid]) */
{
    const int macroid = get_xinteger(1, x_playback_ident);
    char buf[32];

    sxprintf(buf, sizeof(buf), "KBD-MACRO-%d", macroid);
    acc_assign_str(buf, -1);
}


/*  Function:           do_playback
 *      playback primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: playback - Replay a keystroke macro.

        int
        playback([int macroid])

    Macro Description:
        The 'playback()' primitive replays the previously saved
        keyboard macro 'macro'.

    Macro Parameters:
        macroid - Optional integer macro identifier specifies the
            keyboard against which to derive the buffer name, if
            omitted the current macro identifier is utilised.

    Macro Returns:
        The 'playback()' primitive returns greater than or equal to
        zero if playback was successful, otherwise less than zero
        on error.

    Macro Portability:
        n/a

    Macro See Also:
        remember
*/
void
do_playback(void)               /* int ([int macroid]) */
{
    x_playback_current =                        /* playback macro identifier */
            playback_get(get_xinteger(1, -1), FALSE);

    if (x_playback_playing) {
        return;
    }

    if (NULL == x_playback_current) {
        infof("Playback macro not found.");
        acc_assign_int(-1);
        return;
    }

    if (x_playback_recording) {
        infof("Can't play back while remembering.");
        acc_assign_int(-1);
        return;
    }

    infof("Playing back keystroke macro.");
    x_playback_playing = TRUE;
    x_playback_current->p_cursor = 0;
    acc_assign_int(0);
}


/*  Function:           inq_keystroke_macro
 *      inq_keystroke_macro primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: inq_keystroke_macro - Retrieve the current keystroke macro.

        string
        inq_keystroke_macro([int macroid], [int &bufnum])

    Macro Description:
        The 'inq_keystroke_macro()' primitive retrieves a string
        containing the definition of the current keyboard macro,
        which may then be saved in a file and reloaded.

        The returned definition may edited and/or saved for future
        use; see <load_keystroke_macro>.

        Note!:
        The primitive and the returned definition is a useful
        reference when developing customised macros.

        Consult the 'remember' source as an example how this buffer
        can be saved.

    Macro Parameters:
        macroid - Optional integer macro identifier specifies the
            keyboard against which to derive the buffer name, if
            omitted the current macro identifier is utilised.

        bufnum - Omitted integer variable if supplied shall be
            populated with the associated buffer number.

    Macro Returns:
        nothing

    Macro Portability:
        Under BRIEF this primitive behaved like the functionality
        available via <inq_keystroke_status>.

    Macro See Also:
        remember, load_keystroke_macro
*/
void
inq_keystroke_macro(void)       /* string ([int macroid], [int &bufnum]) */
{
    const char *kp;
    playback_t *pb;
    const uint16_t *usp, *end;
    ref_t *rp;

    if (NULL == (pb =
            (isa_undef(1) ? x_playback_current : playback_get(get_xinteger(1, 0), FALSE)))) {
        acc_assign_str("", 1);
        return;
    }
    argv_assign_int(2, (accint_t) pb->p_bufnum);

    rp = r_string("");
    usp = (const uint16_t *) r_ptr(pb->p_ref);
    end = usp + (r_used(pb->p_ref) / sizeof(uint16_t));
    while (usp < end) {
        kp = key_code2name((int) *usp++);
        r_cat(rp, kp);
    }
    acc_assign_ref(rp);
}


/*  Function:           inq_keystroke_macro
 *      inq_keystroke_macro primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: inq_keystroke_status - Determine keystroke macro status.

        int
        inq_keystroke_macro([int &macroid])

    Macro Description:
        The 'inq_keystroke_macro()' primitive determines whether
        keystroke macro record or playback is active.

    Macro Parameters:
        macroid - Optional integer variable if supplied shall be
            populated with the current macro identifier.

    Macro Returns:
        The 'inq_keystroke_macro()' primitive returns greater than
        zero if a keystroke macro is being recorded or played back.

    Macro Portability:
        Under BRIEF this primitive is named <inq_keystroke_macro>.

    Macro See Also:
        remember, playback
 */
void
inq_keystroke_status(void)      /* ([int &macroid]) */
{
    int ret = 0;

    if (x_playback_recording) {
        argv_assign_int(1, (accint_t) x_playback_current->p_ident);
        ret = 1;

    } else if (x_playback_playing) {
        argv_assign_int(1, (accint_t) x_playback_current->p_ident);
        ret = 2;
    }
    acc_assign_int(ret);
}


/*  Function:           load_keystroke_macro
 *      load_keystroke_macro primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: load_keystroke_macro - Load a recorded macro from a file.

        int
        load_keystroke_macro(string def)

    Macro Description:
        The 'load_keystroke_macro()' primitive loads the specified
        keystroke macro 'macro', being a string in a similar format
        returned by <inq_keystroke_macro>. This primitive is designed
        to allow user macros to load macros from external files.

    Macro Parameters:
        macro - String containing the macro definition.

    Macro Returns:
        The 'load_keystroke_macro()' primitive returns the newly
        associated macro identifier, otherwise -1 on error.

    Macro Portability:
        n/a

    Macro See Also:
        inq_keystroke_macro
*/
void
do_load_keystroke_macro(void)   /* int (string macro) */
{
    const char *macro = get_str(1);
    playback_t *pb;
    const KEY *keys;
    int size, ret = -1;

    pb = playback_get(-1, TRUE);                    /* create buffer buffer */
    keys = key_string2seq(macro, &size);
    if (keys && size > 0) {
        pb->p_ref = r_nstring((void *)keys, size * sizeof(KEY));
        ret = pb->p_ident;
    } else {
        errorf("load_keystroke_maro: invalid macro");
    }
    acc_assign_int(ret);

}


/*  Function:           save_keystroke_macro
 *      save_keystroke_macro primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: save_keystroke_macro - Save the current keystroke macro.

        int
        save_keystroke_macro(string filename)

    Macro Description:
        The 'save_keystroke_macro()' primitive saves the current
        remembered keystrokes macro to the specified file 'filename'.

    Macro Parameters:
        filename - String containing the name of the destination file
            to which the keystroke macro is to be saved. If no file
            extension is stated, the extension '.km' shall be used.

    Macro Returns:
        The 'save_keystroke_macro()' primitive returns greater than
        zero on success, otherwise -1 on error.

    Macro See Also:
        playback, remember
 */
void
do_save_keystroke_macro(void)   /* int (string filename) */
{
    //TODO
    acc_assign_int(-1);
}


/*  Function:           playback_store
 *      Store the specific character into the current playback macro.
 *
 *  Parameters:
 *      ch - Character value.
 *
 *  Returns:
 *      nothing
 */
void
playback_store(int ch)
{
    const uint16_t us = (uint16_t) ch;

    if (x_playback_recording && x_rem_string[0] != 'P') {
        r_append(x_playback_current->p_ref, (const void *) &us, sizeof(us), PLAYBACK_INCR);
    }
}


/*  Function:           playback_grab
 *      Retrieve the next character from the current playback macro.
 *
 *  Parameters:
 *      popit - Determine whether the character should be popped/removed
 *          from the playback stack.
 *
 *  Returns:
 *      Character value, otherwise 0.
 */
int
playback_grab(int popit)
{
    if (x_playback_playing) {
        const uint16_t *usp;
        uint16_t ch;

        if (x_rem_string[0] == 'P') {
            return 0;
        }

        if (x_playback_current->p_cursor >= (r_used(x_playback_current->p_ref) / sizeof(uint16_t))) {
            x_playback_playing = FALSE;
            u_chain();
            infof("Playback successful.");
            return 0;
        }

        usp = (const uint16_t *) r_ptr(x_playback_current->p_ref);
        ch = usp[x_playback_current->p_cursor];  /* next character */
        if (popit) {
            ++x_playback_current->p_cursor;      /* consume */
        }
        return ch;
    }
    return 0;
}


/*
 *  Function called when a macro is executed as a result of user
 *  hitting a key. Used to store the disassembled version of the
 *  keystroke macro.
 */
void
playback_macro(const char *cp)
{
    BUFFER_t *bp, *saved_bp = curbp;

    if (!x_playback_recording || x_rem_string[0] == 'P') {
        return;
    }

    if (-1 == rem_nest_level) {
        rem_nest_level = x_nest_level;
    } else if (rem_nest_level != x_nest_level) {
        return;
    }

    /*
     *  Find the buffer associated with this keyboard macro. If the buffer has
     *  disappeared, ie. user has deleted it then don't bother trying to save the
     *  disassembled keystroke info.
     */
    if (NULL == (bp = buf_lookup(x_playback_current->p_bufnum))) {
        return;
    }

    curbp = bp;
    set_hooked();
    BFSET(curbp, BF_NO_UNDO);

    if ('s' == *cp && 0 == strcmp(cp, "self_insert")) {
        char buf[5];
        char *b = buf;

        if (rem_doing_self_insert) {
            switch (x_character) {
            case '\\':
            case '"':
                *b++ = '\\';
                break;
            }
            *b++ = (char)x_character;
            linsert(buf, (uint32_t) (b - buf), FALSE);

        } else {
            linsert("insert(", (uint32_t) 7, FALSE);
            *b++ = '"';
            switch (x_character) {
            case '\\':
            case '"':
                *b++ = '\\';
                break;
            }
            *b++ = (char)x_character;
            linsert(buf, (uint32_t) (b - buf), FALSE);
            rem_doing_self_insert = TRUE;
        }

    } else {
        if (rem_doing_self_insert) {
            linsert("\");", (uint32_t) 3, TRUE);
        }

        if ('r' == *cp && 0 == strcmp(cp, "remember")) {
            const char *msg = "/* End of macro */";

            linsert(msg, (uint32_t) strlen(msg), TRUE);

        } else {
            /*
             *  Need to put commas and brackets around arguments.
             */
            int done_open = FALSE;

            while (' ' == *cp)
                ++cp;
            while (*cp) {
                linsert(cp, (uint32_t) 1, FALSE);
                ++cp;

                if (' ' == *cp) {
                    if (done_open) {
                        linsert(", ", (uint32_t) 2, FALSE);
                    } else {
                        linsert("(", (uint32_t) 1, FALSE);
                    } done_open = TRUE;
                    ++cp;
                    continue;
                }
            }

            if (!done_open) {
                linsert("();", (uint32_t) 3, TRUE);
            } else {
                linsert(");", (uint32_t) 2, TRUE);
            }
        }
        rem_doing_self_insert = FALSE;
    }
    curbp = saved_bp;
    set_hooked();
}
/*end*/
