#include <edidentifier.h>
__CIDENT_RCSID(gr_sys_vms_c,"$Id: sys_vms.c,v 1.15 2014/10/22 02:33:20 ayoung Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: sys_vms.c,v 1.15 2014/10/22 02:33:20 ayoung Exp $
 * VMS system support
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
#include <edtermio.h>

#include "accum.h"
#include "builtin.h"
#include "echo.h"
#include "main.h"
#include "m_pty.h"
#include "tty.h"
#include "system.h"

#if defined(_VMS)

#include <ssdef.h>
#include <stsdef.h>
#include <ttdef.h>
#include <tt2def.h>
#include <iodef.h>
#include <descrip.h>

/*
 *  Notes on VMS implementation:
 *
 *  1.  Backups not supported since
 *          (a) VMS does not support link() and unlink()
 *          (b) and VMS supports version numbers anyway.
 * 
 *  2.  Filenames can be in the normal VMS format or the Unix style can be used: /dev/dir1/dir2/file
 *      (same as dev:[dir1.dir2]file ).
 *
 *  3.  Node-names are not supported.
 * 
 *  4.  If the user uses the DEC-shell, then Grief may hang trying to parse unexpected path names.
 * 
 *  5.  Changing directory and editing relative files will fail.
 */
#define CENTISECONDS    5       /* x0.1 Read ahead timeout */

static struct iosb {
    short               status;
    short               offset;
    short               term;
    short               termlen;
}   input_iosb;

static struct sensemode {
    short               status;
    unsigned char       xmit_baud;
    unsigned char       rcv_baud;
    unsigned char       crfill;
    unsigned char       lffill;
    unsigned char       parity;
    unsigned char       unused;
    char                class;
    char                type;
    short               scr_wid;
    unsigned long       tt_char:24, scr_len:8;
    unsigned long       tt2_char;
};

static int              input_ef = 0;
static int              input_chan = 0;
static int              timeout_flag = FALSE;
static $DESCRIPTOR(input_dsc, "TT");
static struct           sensemode old_gtty;
static int              sys_first_time = 0;

void
sys_initialise(void)
{
    struct sensemode sg;

    if (sys_first_time)
        return;
    sys_first_time = 1;

    if (input_chan == 0) {
        int status = SYS$ASSIGN(&input_dsc, &input_chan, 0, 0);

        if (!(status & 1))
            LIB$STOP(status);
    }

    if (input_ef == 0) {
        LIB$GET_EF(&input_ef);
        SYS$CLREF(input_ef);
    }

    SYS$QIOW(0, input_chan, IO$_SENSEMODE, &old_gtty, 0, 0,
        &old_gtty.class, 12, 0, 0, 0, 0);

    sg = old_gtty;
    sg.tt_char |= TT$M_PASSALL | TT$M_NOECHO | TT$M_EIGHTBIT;
    sg.tt_char &= ~TT$M_TTSYNC;
    sg.tt2_char |= TT2$M_PASTHRU | TT2$M_XON;

    SYS$QIOW(0, input_chan, IO$_SETMODE, &input_iosb, 0, 0,
        &sg.class, 12, 0, 0, 0, 0);
}


void
sys_shutdown(void)
{
    if (!sys_first_time)
        return;

    SYS$QIOW(0, input_chan, IO$_SETMODE, &input_iosb, 0, 0,
        &old_gtty.class, 12, 0, 0, 0, 0);
}


int
sys_timeout(int yes)
{
    timeout_flag = yes;
}


#define MAX_INPUT       128

static unsigned char    input_buffer[MAX_INPUT];
static int              iptr = MAX_INPUT + 1;
static int              icnt = 0;


/*  Function:           sys_iocheck
 *      Check for an event input event.
 *
 *  Parameters:
 *      None
 *
 *  Returns:
 *      *true* or *false*.
 */                   
int
sys_iocheck(struct IOEvent *evt)
{
    if (iptr < icnt) {
        evt->type = EVT_KEYRAW;                 /* uncooked key-code */
        evt->code = input_buffer[iptr++];
        return TRUE;
    }
    return FALSE;
}


/*  Function:           sys_getchar
 *      Retrieve the character from the status keyboard stream, within 
 *      the specified timeout 'tmo'.
 *
 *  Parameters:
 *      fd -                File descriptor.
 *      buf -               Output buffer.
 *      tmo -               Timeout, in milliseconds.
 *
 *  Returns:
 *      On success (1), otherwise (0) unless a timeout (-1).
 */
int
sys_getchar(int fd, int *buf, accint_t cnt)
{
    if (iptr >= icnt) {
        SYS$QIOW(0, input_chan,
            IO$_READVBLK | (timeout_flag ? IO$M_TIMED : 0),
            &input_iosb, 0, 0,
            &input_buffer, 1, 2, 0, 0, 0);
        
        if (input_iosb.status != SS$_NORMAL) {
            return 0;
        }

        iptr = 0;
        icnt = 1;
    }
    *buf = input_buffer[iptr++] & 0xff;
    return 1;
}


int
sys_write(int fd, unsigned char *buf, int cnt)
{
    SYS$QIOW(0, input_chan, 
        IO$_WRITEVBLK, 
        &input_iosb, 0, 0, buf, cnt, 0, 0, 0, 0);
}


void
sys_cleanup(void)
{
}


/* 
 *  We need to put in VMS style wild card expansions.
 */
char **
shell_expand(const char *filename)
{
    char **buf = chk_alloc(sizeof(char *) * 2);

    buf[0] = chk_salloc(filename);
    buf[1] = 0;
    return buf;
}


const char *
sys_delim(void)
{
    return "";
}


char *
sys_fname_unix_to_vms(const char * src, char * dst, int size)
{
    int slash_count = 0;
    const char *orig_src = src;
    char *orig_dst = dst;

    size--;
    if (*src != '/' && strchr(src, '/') != NULL) {
        *dst++ = '[';
        *dst++ = '.';
        size -= 2;
        slash_count = 2;
    }

    while (*src) {
        if (*src != '/') {
            if (size-- <= 0)
                break;
            *dst++ = *src++;
            continue;
        }
        
        src++;
        
        switch (slash_count++) {
        case 0:
            break;
        case 1:
            *dst++ = ':';
            size--;
            if (strchr(src, '/')) {
                *dst++ = '[';
                size--;
            }
            break;
        default:
            if (strchr(src, '/') == 0)
                *dst++ = ']';
            else
                *dst++ = '.';
            size--;
            break;
        }
    }
    *dst = 0;
    return orig_dst;
}


char *
vms_filename_canon(char *buf)
{
    register char *cp;
    register char *cp1;

    for (cp = buf; *cp; cp++) {
        /*
         *  Map fred:[dir1.dir2.][dir3]... to fred:[dir1.dir2.dir3]...
         */
        if (*cp == '.' && cp[1] == ']' && cp[2] == '[') {
            strcpy(cp + 1, cp + 3);
            continue;
        }

        /*
         *  Map fred:[dir1.dir2][.dir3]... to fred:[dir1.dir2.dir3]...
         */
        if (*cp == ']' && cp[1] == '[' && cp[2] == '.') {
            strcpy(cp, cp + 2);
            cp--;
            continue;
        }

        /*
         *  Map fred:[dir1.dir2][dir3]... to fred:[dir3]...
         */
        if (*cp == '[') {
            cp1 = strchr(buf, '[');
            if (cp1 != cp)
                strcpy(cp1, cp);
        }
    }

    /* 
     *  Map fred:[dir1.dir2.-.dir3]... to fred:[dir1.dir3]...
     *  we do this after the others so that we can ensure
     *  there's only one PPN left
     */
    for (cp = buf; *cp && *cp != '[';)
        cp++;
    while (*cp && *cp != ']') {
        if (*cp == '.' && cp[1] == '-') {
            for (cp1 = cp - 1; *cp1 != '['; cp1--)
                if (*cp1 == '.')
                break;
            strcpy(cp1, cp + 2);
            cp = cp1;
        }
        else
            cp++;
    }
    return buf;
}


/* 
 *  Run a command. The "cmd" is a pointer to a command string, or NULL if you
 *  want to run a copy of DCL in the subjob (this is how the standard routine
 *  LIB$SPAWN works. You have to do wierd stuff with the terminal on the way in
 *  and the way out, because DCL does not want the channel to be in raw mode.
 */
int
system(const char *cmd)
{
    struct dsc$descriptor cdsc;
    struct dsc$descriptor *cdscp;
    long status;
    long substatus;
    long iosb[2];

    sys_shutdown();
    sys_first_time = 0;
    cdscp = NULL;                               /* Assume DCL.          */
    if (cmd && (strcmp(cmd, "/bin/sh") == 0 || strcmp(cmd, "/bin/csh") == 0))
        cmd = NULL;
    if (cmd != NULL)
    {                                           /* Build descriptor.    */
        cdsc.dsc$a_pointer = (char *)cmd;
        cdsc.dsc$w_length = strlen(cmd);
        cdsc.dsc$b_dtype = DSC$K_DTYPE_T;
        cdsc.dsc$b_class = DSC$K_CLASS_S;
        cdscp = &cdsc;
    }
    status = LIB$SPAWN(cdscp, 0, 0, 0, 0, 0, &substatus, 0, 0, 0);
    if (status != SS$_NORMAL)
        substatus = status;
    sys_initialise();
    if (status != SS$_NORMAL || (iosb[0] & 0xFFFF) != SS$_NORMAL)
        return (FALSE);
    if ((substatus & STS$M_SUCCESS) == 0)       /* Command failed.      */
        return (FALSE);
    return (TRUE);
}


/* 
 *  Set flag to indicate file descriptor should be closed on an exec
 */
void
sys_noinherit(int fd)
{
#if defined(FIOCLEX)
    fcntl(fd, FIOCLEX, 0);
#endif
}

#endif /*_VMS*/

