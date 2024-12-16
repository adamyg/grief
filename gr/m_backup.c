#include <edidentifier.h>
__CIDENT_RCSID(gr_m_backup_c,"$Id: m_backup.c,v 1.21 2024/12/05 19:00:11 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: m_backup.c,v 1.21 2024/12/05 19:00:11 cvsuser Exp $
 * File backup option/configuration primitives.
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
#include <errno.h>
#include <edenv.h>                              /* gputenvv(), ggetenv() */
#include <libstr.h>                             /* str_...()/sxprintf() */

#include "m_backup.h"                           /* public interface */

#include "system.h"
#include "accum.h"                              /* acc_...() */
#include "buffer.h"                             /* buf_...() */
#include "debug.h"                              /* trace_...() */
#include "echo.h"                               /* errorf, infof */
#include "eval.h"                               /* get_...() */
#include "main.h"                               /* xf_... globals */

#define KBYTES          1024
#if defined(ONLY_ONE_EXTENSION)
#define ONEEXT_DEFAULT  1
#else
#define ONEEXT_DEFAULT  0
#endif

static int              x_backup_oneext = ONEEXT_DEFAULT;

static int              x_backup_versions = -1; /* no versions */
static FSIZE_t          x_backup_ask = 0;       /* no limit */
static FSIZE_t          x_backup_dont = 0;      /* no limit */

static char             x_backup_dir[MAX_PATH] = {0};
static char             x_backup_prefix[32+1] = "";
static char             x_backup_suffix[32+1] = ".bak";


/*  Function:           bkcfg_dir
 *      Retrieve the backup base path.
 *
 *  Parameters:
 *      bp - Buffer object address.
 *
 *  Returns:
 *      Dir buffer.
 */
const char *
bkcfg_dir(BUFFER_t *bp)
{
    if (!x_backup_dir[0]) {
        const char *env = ggetenv("GRBACKUP");

        strxcpy(x_backup_dir, env ? env : "", sizeof(x_backup_dir));
    }

    if (bp && bp->b_bkdir) {
        return bp->b_bkdir;
    }
    return x_backup_dir;
}


/*  Function:           bkcfg_versions
 *      Retrieve the backup version count.
 *
 *  Parameters:
 *      bp - Buffer object address.
 *
 *  Returns:
 *      Versions.
 */
int
bkcfg_versions(BUFFER_t *bp)
{
    if (x_backup_versions < 0) {
        const char *env;

        if (NULL == (env = ggetenv("GRVERSIONS")) ||
                (x_backup_versions = atoi(env)) < 0) {
            x_backup_versions = 0;
        }
    }

    if (bp && bp->b_bkversions >= 0) {
        return bp->b_bkversions;
    }
    return x_backup_versions;
}


int
bkcfg_oneext(void)
{
    return x_backup_oneext;
}


/*  Function:           bkcfg_prefix
 *      Retrieve the backup file-name prefix.
 *
 *  Parameters:
 *      bp - Buffer object address.
 *
 *  Returns:
 *      Prefix buffer.
 */
const char *
bkcfg_prefix(BUFFER_t *bp)
{
    if (bp && bp->b_bkprefix) {
        return bp->b_bkprefix;
    }
    return x_backup_prefix;
}


/*  Function:           bkcfg_suffix
 *      Retrieve the backup file-name suffix.
 *
 *  Parameters:
 *      bp - Buffer object address.
 *
 *  Returns:
 *      Suffix buffer.
 */
const char *
bkcfg_suffix(BUFFER_t *bp)
{
    if (bp && bp->b_bksuffix) {
        return bp->b_bksuffix;
    }
    return x_backup_suffix;
}


/*  Function:           bkcfg_askmsg
 *      Create large file backup message.
 *
 *  Parameters:
 *      buf - Input buffer.
 *      length - Buffer length, in bytes.
 *      sb - File stat buffer.
 *
 *  Returns:
 *      Address of message buffer.
 *
 */
char *
bkcfg_askmsg(char *buf, size_t length, size_t size)
{
    static const char *units[] = {
        "Kb", "Mb", "Gb", "Tb", "Pb", "Eb"
        };

    if (size >= KBYTES) {                   /* "Large image 42.90Mb, backup" */
        double sz = ((double)size) / KBYTES;
        int unit = 0;

        while (sz >= KBYTES && unit < (int)(sizeof(units)/sizeof(units[0]))) {
            sz /= KBYTES; ++unit;
        }
        sxprintf(buf, (int)length, "Large image %.2f%s, backup", sz, units[unit]);

    } else {                                /* "Large image 1023 bytes, backup" */
        sxprintf(buf, (int)length, "Large image %d bytes, backup", (int)size);
    }
    return buf;
}


/*  Function:           bkcfg_ask
 *      Should backups be prompted?
 *
 *  Parameters:
 *      fname - Filename.
 *
 *  Returns:
 *      *TRUE* otherwise *FALSE*.
 */
int
bkcfg_ask(const char *fname)
{
    char prompt[MAX_CMDLINE];
    struct stat sb;
    int ret = TRUE;

    if (! xf_backups) {
        ret = FALSE;
    } else {
        if (sys_stat(fname, &sb) >= 0) {
            if (x_backup_dont && sb.st_size >= x_backup_dont) {
                ret = FALSE;
            } else if (x_backup_ask && sb.st_size >= x_backup_ask) {
                ret = eyesno(bkcfg_askmsg(prompt, sizeof(prompt), sb.st_size));
                if (TRUE != ret) {
                    ret = FALSE;                /* ABORT */
                }
            }
            if (ret) {
                infof("Writing backups ...");
            }
        }
    }
    return ret;
}


/*  Function:           do_set_backup
 *      set_backup primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: set_backup - Set backup creation mode.

        int
        set_backup(int mode, [int bufnum])

    Macro Description:
        The 'set_backup()' primitive either toggles, set or retrieves
        the current backup flag of the associated object. If
        'bufnum' is specified, then the stated buffer is effected,
        otherwise upon being omitted the global (default) backup mode
        shall be effected.

        The backup flag is tested each time a buffer is written to
        its underlying storage (see write_buffer). The flag maybe one
        of two states, either 'off' or 'on'; when 'on', then a backup
        files shall be created.

        If 'mode' is not specified, then the value of the associated
        backup flag is toggled from 'on' to 'off' or 'off' to 'no'.
        If given as zero backups shall be disabled, with a positive
        non-zero value enabling backups.

        The 'set_backup()' primitive may also be used to inquire the
        value of the associated backup flag. If the specified
        'mode' is -1 then the current flag value of the associated
        object shall be returned without any other effects.

        When invoked from the command, one of the following
        messages shall be echoed

>              "Backups will be created".
>           or "Backups will not be created."

>              "Backups will be created for the buffer".
>           or "Backups will not be created for the buffer."

    Macro Returns:
        The 'set_backup()' primitive returns the previous value of
        the backup flag.

    Macro Portability:
        n/a

    Macro See Also:
        inq_backup, set_backup_option, inq_backup_option
 */
void
do_set_backup(void)             /* ([int mode], [int bufnum]) */
{
    BUFFER_t *bp = NULL;
    int oflag, nflag;

    /*
     *  buffer specific
     */
    if (!isa_undef(2)) {
        if (NULL == (bp = buf_lookup(get_xinteger(2, 0)))) {
            ewprintf("set_backup: no such buffer");
            acc_assign_int(-1);
            return;
        }

        oflag = BFTST(bp, BF_BACKUP);
        if (isa_undef(1)) {                     /* toggle */
            if (BFTST(bp, BF_BACKUP)) {
                BFCLR(bp, BF_BACKUP);

            } else {
                BFSET(bp, BF_BACKUP);
            }

        } else {
            const int value1 = get_xinteger(1, -1);

            if (value1 >= 0) {                  /* set */
                if (value1) {
                    BFSET(bp, BF_BACKUP);

                } else {
                    BFCLR(bp, BF_BACKUP);
                }
            }
        }
        nflag = BFTST(bp, BF_BACKUP);

    /*
     *  global
     */
    } else {
        oflag = xf_backups;
        if (isa_undef(1)) {                     /* toggle */
            xf_backups = !xf_backups;

        } else {
            const int value1 = get_xinteger(1, -1);

            if (value1 >= 0) {                  /* set */
                xf_backups = value1;
            }
        }
        nflag = xf_backups;
    }

    acc_assign_int((accint_t) oflag);
    infof("Backups will %sbe created%s.", nflag ? "" : "not ", bp ? " for the buffer" : "");
}


/*  Function:           inq_backup
 *      inq_backup primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: inq_backup - Get the backup creation mode.

        int
        inq_backup([int bufnum])

    Macro Description:
        The 'inq_backup()' primitive retrieves the value of the
        associated objects backup mode. If 'bufnum' is specified,
        then the value of the stated buffer is returned, otherwise
        upon being omitted the global (default) backup mode shall be
        returned.

    Macro Returns:
        The 'inq_backup()' primitive returns the current value of the
        backup flag.

    Macro Portability:
        A Grief extension.

    Macro See Also:
        set_backup, set_backup_option, inq_backup_option
 */
void
inq_backup(void)                /* int ([int bufnum]) */
{
    BUFFER_t *bp = NULL;
    int oflag;

    if (!isa_undef(2)) {
        if (NULL == (bp = buf_lookup(get_xinteger(2, 0)))) {
            ewprintf("inq_backup: no such buffer");
            acc_assign_int(-1);
            return;
        }
        oflag = BFTST(bp, BF_BACKUP);

    } else {
        oflag = xf_backups;
    }

    acc_assign_int((accint_t) oflag);
}


/*  Function:           do_set_backup_option
 *      set_backup_option primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: set_backup_option - Set backup options.

        int
        set_backup_option(int what, [int bufnum], parameter)

    Macro Description:
        The 'set_backup_option()' primitive sets the value of the
        backup option 'what' for the associated object. backup mode.
        If 'bufnum' is specified, then the value of the stated buffer
        is modified, otherwise upon being omitted the global
        (default) backup mode shall be modified.

        'parameter' shall be 'what' specific, with the following
        options available;

            o *BK_MODE* -
                Backup creation mode, see <set_backup> and
                <inq_backup>. A zero integer 'parameter' shall
                disable backups, with a non-zero value enabling
                backups.

            o *BK_AUTOSAVE* -
                Buffer auto-save flag. A zero integer 'parameter'
                shall disable auto-backups, with a non-zero value
                enabling auto-backups.

            o *BK_DIR* -
                Backup directory. 'parameter' should a string
                containing the backup directory path.

            o *BK_DIRMODE* -
                Directory creation mode. 'parameter' should be a
                integer value specifying the file creation <umask>.

            o *BK_VERSIONS* -
                Number of versions to be maintained. 'parameter'
                should be an integer value specifying the number of
                backup versions to be kept in the range [1 .. 99];
                values outside shall set the versions to the
                lower/upper bounds.

            o *BK_PREFIX* -
                Backup filename prefix. 'parameter' should be a
                string containing the prefix, an empty string shall
                clear the current suffix.

            o *BK_SUFFIX* -
                Backup filename suffix/extension. 'parameter' should
                be a string containing the suffix, an empty string
                shall clear the current prefix.

            o *BK_ONESUFFIX* -
                Whether only a single suffix to used replacing any
                existing, otherwise append the suffix shall be
                appended. 'parameter' should be integer, with a
                non-zero enabling or zero disabling one-suffix
                filtering on backup filenames.

            o *BK_ASK* -
                File-size watermark at which point backups shall be
                prompted before created a backup image. 'parameter'
                should be a positive integer value being the
                watermark file-size in bytes, with a value of zero
                disabling any prompts.

            o *BK_DONT* -
                Filesize watermark at which point backups shall not
                be created regardless of the current backup mode.
                'parameter' should be a positive integer value being
                the watermark filesize in bytes, with a value of zero
                disabling any affected.

    Macro Returns:
        The 'set_backup_option()' on success returns zero, otherwise
        -1 on error.

    Macro Portability:
        A Grief extension.

    Macro See Also:
        inq_backup_option, set_backup, inq_backup
 */
void
do_set_backup_option(void)      /* int (int what, [int bufnum], parameter) */
{
    static const char who[] = "set_backup_option";
    const int what = get_xinteger(1, -1);
    BUFFER_t *bp = NULL;
    const char *s;

    if (!isa_undef(2))
        if (NULL == (bp = buf_lookup(get_xinteger(2, 0)))) {
            ewprintf("%s: no such buffer", who);
            acc_assign_int((accint_t) -1);
            return;
        }

    switch(what) {
    case BK_MODE:
        if (bp) {
            if (isa_integer(3)) {
                if (get_xinteger(3, 0)) {
                    BFSET(bp, BF_BACKUP);
                } else {
                    BFCLR(bp, BF_BACKUP);
                }
            }
        } else {
            xf_backups = get_xinteger(3, 0);
        }
        break;

    case BK_AUTOSAVE:
        if (bp) {
            if (isa_integer(3)) {
                if (get_xinteger(3, 0)) {
                    BF3SET(bp, BF3_AUTOSAVE);
                } else {
                    BF3CLR(bp, BF3_AUTOSAVE);
                }
            }
        } else {
            xf_autosave = get_xinteger(3, 0);
        }
        break;

    case BK_DIR:
        s = get_xstr(3);
        if (bp) {
            chk_free((void *)bp->b_bkdir);
            bp->b_bkdir = (NULL != s ? chk_salloc(s) : NULL);
        } else {
            strxcpy(x_backup_dir, s ? s : "", sizeof(x_backup_dir));
            gputenv2("GRBACKUP", x_backup_dir);
        }
        break;

    case BK_VERSIONS:
        if (bp) {
            if ((bp->b_bkversions = get_xinteger(3, -1)) < 0) {
                bp->b_bkversions = 0;
            } else if (bp->b_bkversions > 99) {
                bp->b_bkversions = 99;
            }
        } else {
            char bversion[32];

            if ((x_backup_versions = get_xinteger(3, 0)) < 0) {
                x_backup_versions = 0;
            } else if (x_backup_versions > 99) {
                x_backup_versions = 99;
            }
            sxprintf(bversion, sizeof(bversion), "%d", (int)x_backup_versions);
            gputenv2("GRVERSIONS", bversion);
        }
        break;

#if defined(BK_DIRMODE)
    case BK_DIRMODE:
        if (NULL == bp) {                       /*not buffer specific*/
        }
        break;
#endif

    case BK_PREFIX:
        s = get_xstr(3);
        if (bp) {
            chk_free((void *)bp->b_bkprefix);
            bp->b_bkprefix = (NULL != s ? chk_salloc(s) : NULL);
        } else {
            strxcpy(x_backup_prefix, s ? s : "", sizeof(x_backup_prefix));
        }
        break;

    case BK_SUFFIX:
        s = get_xstr(3);
        if (bp) {
            chk_free((void *)bp->b_bksuffix);
            bp->b_bksuffix = (NULL != s ? chk_salloc(s) : NULL);
        } else {
            strxcpy(x_backup_suffix, s ? s : "", sizeof(x_backup_suffix));
        }
        break;

    case BK_ONESUFFIX:
        if (NULL == bp) {                       /*not buffer specific*/
            x_backup_oneext = get_xinteger(3, ONEEXT_DEFAULT);
        }
        break;

    case BK_ASK:
        if (bp) {
            bp->b_bkask = (isa_integer(3) ? get_xinteger(3, 0) * KBYTES : -1);
        } else {
            x_backup_ask = (isa_integer(3) ? get_xinteger(3, 0) * KBYTES : 0);
        }
        break;

    case BK_DONT:
        if (bp) {
            bp->b_bkdont = (isa_integer(3) ? get_xinteger(3, 0) * KBYTES : -1);
        } else {
            x_backup_dont = (isa_integer(3) ? get_xinteger(3, 0) * KBYTES : 0);
        }
        break;

    default:
        acc_assign_int(-1);
        return;
    }
    acc_assign_int(0);
}


/*  Function:           inq_backup_option
 *      inq_backup_option primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: inq_backup_option - Get backup options.

        declare
        inq_backup_option(int what, [int bufnum])

    Macro Description:
        The 'inq_backup_option()' primitive retrieves the value of
        the backup option 'what' for the associated object. If
        'bufnum' is specified, then the value of the stated buffer is
        returned, otherwise upon being omitted the global (default)
        backup option shall be modified.

    Macro Returns:
        The 'inq_backup_option()' primitive returns the current value
        associated with the attribute 'what', as follows

            o *BK_MODE* -
                Backup creation mode, as an integer.

            o *BK_AUTOSAVE* -
                Buffer autosave flag, as an integer.

            o *BK_DIR* -
                Backup directory, as a string.

            o *BK_DIRMODE* -
                Directory creation mode, as an integer.

            o *BK_VERSIONS* -
                Number of versions to be maintained, as an integer.

            o *BK_PREFIX* -
                Backup filename prefix, as a string.

            o *BK_SUFFIX* -
                Backup filename suffix, as a string.

            o *BK_ONESUFFIX* -
                Whether only a single suffix to used replacing any
                existing, as an integer.

            o *BK_ASK* -
                Filesize watermark at which point backups shall be
                prompted before created a backup image, as an
                integer.

            o *BK_DONT* -
                Filesize watermark at which point backups shall not
                be created regardless of the current backup mode,
                as an integer.

    Macro Portability:
        A Grief extension.

    Macro See Also:
        inq_backup_option, set_backup, inq_backup
 */
void
inq_backup_option(void)         /* string|int (int what, [int bufnum]) */
{
    static const char who[] = "inq_backup_option";
    const int what = get_xinteger(1, -1);
    BUFFER_t *bp = NULL;

    if (!isa_undef(2))
        if (NULL == (bp = buf_lookup(get_xinteger(2, 0)))) {
            ewprintf("%s: no such buffer", who);
            acc_assign_int(-1);
            return;
        }

    switch(what) {
    case BK_MODE:
        if (bp) {
            acc_assign_int((accint_t) BFTST(bp, BF_BACKUP));
        } else {
            acc_assign_int((accint_t) xf_backups);
        }
        break;

    case BK_AUTOSAVE:
        if (bp) {
            acc_assign_int((accint_t) BFTST(bp, BF3_AUTOSAVE));
        } else {
            acc_assign_int((accint_t) xf_autosave);
        }
        break;

    case BK_DIR:
        acc_assign_str(bkcfg_dir(bp), -1);
        break;

    case BK_VERSIONS:
        acc_assign_int((accint_t) bkcfg_versions(bp));
        break;

#if defined(BK_DIRMODE)
    case BK_DIRMODE:
        if (NULL == bp) {                       /*not buffer specific*/
        }
        break;
#endif

    case BK_PREFIX:
        acc_assign_str(bkcfg_prefix(bp), -1);
        break;

    case BK_SUFFIX:
        acc_assign_str(bkcfg_suffix(bp), -1);
        break;

    case BK_ONESUFFIX:
        /*NOT BUFFER SPECIFIC*/
        acc_assign_int((accint_t) x_backup_oneext);
        break;

    case BK_ASK:
        if (bp && bp->b_bkask >= 0) {
            acc_assign_int((accint_t) bp->b_bkask / KBYTES);
        } else {
            acc_assign_int((accint_t) x_backup_ask / KBYTES);
        }
        break;

    case BK_DONT:
        if (bp && bp->b_bkdont >= 0) {
            acc_assign_int((accint_t) bp->b_bkdont / KBYTES);
        } else {
            acc_assign_int((accint_t) x_backup_dont / KBYTES);
        }
        break;

    default:
        acc_assign_int(-1);
        break;
    }
}

/*end*/
