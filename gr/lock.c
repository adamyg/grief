#include <edidentifier.h>
__CIDENT_RCSID(gr_lock_c,"$Id: lock.c,v 1.33 2025/02/07 03:03:21 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: lock.c,v 1.33 2025/02/07 03:03:21 cvsuser Exp $
 * File locking support.

    When two users edit the same file at the same time, they are likely
    to interfere with each other. GRIEF tries to prevent this situation
    from arising by recording a file lock when a file is being modified.
    Grief can then detect the first attempt to modify a buffer visiting
    a file that is locked by another instance of GRIEF and asks the user
    what to do.

    The locking mechanism creates a lock at the point you start to edit a
    file. When you start editing, if a lock already exists then GRIEF will
    tell you who is editing the file, and gives you the option of continuing
    the edit or cancelling the changes you are about to made.

    The advisory file lock is really a file, a symbolic link with a special
    name, stored in the same directory as the file you are editing

    The protocol is this:

        If in the directory of the file is a symbolic link with name ".#FILE", 
        the FILE is considered to be locked by the process specified by the
        link.

    Here are the scenerios requiring a lock:

        1. When a buffer attached to a file becomes modified/is read.
        2. When the output name is changed on a modified buffer.
        3. When appending or writing to a file.

    In order for the locking mechanism to work, you need to enable it and
    specify a directory where lock files are kept. If not defined then the
    same directory as the file opened shall be used.

    These lock files are compatible with the Emacs, JED and GRIEF (and
    others) locking mechanism so you can detect files being edited by any of
    these editors.

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

#include <sys/stat.h>
#include <editor.h>
#include <chkalloc.h>
#include <tailqueue.h>
#include <libstr.h>                             /* str_...()/sxprintf() */

#include "debug.h"
#include "echo.h"
#include "file.h"
#include "system.h"
#include "sysinfo.h"
#include "lock.h"

#define NEED_LOCKS

#if !defined(NEED_LOCKS)
int
flock_init(void)
{
    return 0;
}


void
flock_close(void)
{
}


int
flock_set(const char *file, int ask)
{
    return 0;
}


void
flock_clear(const char *file)
{
}


#else   /*lock*/

#define INFO_SAME       0
#define INFO_HOST       1
#define INFO_USER       2

#define LCK_APROCEED    0
#define LCK_ASTEAL      1
#define LCK_AABORT      -1

typedef TAILQ_HEAD(LockList, _lockn)
                        LockList_t;             /* lock list */

#if (SIZEOF_INT >= SIZEOF_LONG)
typedef unsigned Pid_t;
#else
typedef unsigned long Pid_t;
#endif

typedef struct {
    const char *        i_user;
    const char *        i_host;
    const char *        i_file;
    Pid_t               i_pid;
    time_t              i_boot;
} Info_t;

typedef struct _lockn {
    const char *        l_file;
    unsigned int        l_status;
#define LCK_SSUCCESS            0
#define LCK_SFORCED             1
#define LCK_SNOTLOCKED          2
    unsigned int        l_refs;
    Info_t              l_info;
    TAILQ_ENTRY(_lockn) l_node;                 /* linear queue of locks */
} Lock_t;

static char *           lock_filename(const char *file);

static char *           info_mak(const Info_t *i);
static int              info_set(Info_t *i, const char *filename);
static void             info_free(Info_t *i);
static int              info_cmp(const Info_t *a, const Info_t *b);
static int              info_get(const char *ifile, Info_t *i);

static Lock_t *         node_new(const char *file);
static void             node_free(Lock_t *n);
static Lock_t *         node_find(const char *file);

static time_t           lockboot;
static LockList_t       lockq;                  /* lock list */


/*  Function:           hostlength
 *      Determine the length of first component within the hostname.
 *
 *  Parameters:
 *      host -              Host name buffer.
 *
 *  Return:
 *      Length of the host name.
 */
static int
hostlength(const char *host)
{
    const char *p;

    if (NULL == (p = strchr(host, '.')) || p == host) {
        return (int)strlen(host);
    }
    return (int)(p - host);
}


/*  Function:           info_mak
 *      Create the symbolic used to represent the lock state, using the form:
 *          "username@host.pid:boot_time"
 *
 *  Parameters:
 *      i -                 Information object.
 *
 *  Returns:
 *      Dynamic buffer.
 */
static char *
info_mak(const Info_t *i)
{
    char *buf;
    size_t len;

    len = strlen (i->i_host) + strlen (i->i_user) + (16 * 2);
    if (NULL == (buf = chk_alloc(len))) {
        return NULL;
    }
    sxprintf(buf, len, "%s@%s.%lu:%lu", i->i_user, i->i_host,
        (unsigned long)i->i_pid, (unsigned long)lockboot);
    trace_log("\tmak: %s\n", buf);
    return buf;
}


static int
info_set(Info_t *i, const char *filename)
{
    memset((char *)i, 0, sizeof (*i));          /* zap structure */

    i->i_user = chk_salloc(sysinfo_username(NULL, 0));
    i->i_host = chk_salloc(sysinfo_hostname(NULL, 0));
    i->i_pid  = (Pid_t) sys_getpid();
    i->i_boot = lockboot;

    if (NULL == i->i_user || NULL == i->i_host) {
        if (i->i_user) {
            chk_free((char *)i->i_user), i->i_user = NULL;
        }
        return (-1);
    }

    trace_log("\tset: user=%s,host=%s,pid=%lu,boot=%lu\n",
        i->i_user, i->i_host, (unsigned long)i->i_pid, (unsigned long)i->i_boot);
    if (filename) {
        i->i_file = lock_filename(filename);
        trace_log("\t     lock=%s\n", i->i_file);
    }
    return (0);
}


static void
info_free(Info_t *i)
{
    if (i) {
        chk_free((char *)i->i_host), i->i_host = NULL;
        chk_free((char *)i->i_user), i->i_user = NULL;
        chk_free((char *)i->i_file), i->i_file = NULL;
        i->i_pid = 0;
        i->i_boot = 0;
    }
}


static int
info_cmp(const Info_t *a, const Info_t *b)
{
    if (strcmp(a->i_host, b->i_host) != 0) {
        return INFO_HOST;
    }
    if (strcmp(a->i_user, b->i_user) != 0 ||
            a->i_pid != b->i_pid || (a->i_boot && b->i_boot && a->i_boot != b->i_boot)) {
        return INFO_USER;
    }
    return INFO_SAME;
}


static int
info_get(const char *lfile, Info_t *i)
{
    struct stat st;
    char buf[1024], *b;
    char *user, *host, *pid, *boot;
    int n;

    memset((char *)i, 0, sizeof (*i));          /* zap */

    /* is symbolic link? */
    if (sys_lstat(lfile, &st) == -1)
    {
        if (errno == ENOENT) {
            return 1;                           /* doesnt exist */
        }
        return -1;
    }
#if defined(S_IFLNK)
    if (((st.st_mode & S_IFMT) & S_IFLNK) == 0) {
        return -1;
    }
#endif

    /* read */
//  if ((n = vfs_readlink(lfile, buf, sizeof (buf)-1)) == -1 ||
//              n >= (int)sizeof(buf)-1) {
//      return -1;
//  }
    if ((n = sys_readlink(lfile, buf, sizeof (buf)-1)) == -1 ||
            n >= (int)sizeof(buf)-1) {
        return -1;
    }
    buf[n] = '\0';

    /* user */
    user = buf;
    if (NULL == (b = strchr(buf, '@'))) {
        return -1;
    }
    *b++ = '\0';

#if defined(__MSDOS__)
    if (NULL != (pid = strchr(user, '\\')) ||
            NULL != (pid = strchr(user, '/'))) {
        user = pid + 1;                         /* remove drive spec */
    }
#endif

    /* host (assume host may contain dots, [boot_time] optional) */
    host = b;
    pid  = NULL;
    boot = NULL;

    b += strlen (b);                            /* skip to end */
    while (b > host && NULL == pid) {
        b--;                                    /* scan-backwards */
        if (*b == ':') {                        /* boot_time */
            if (NULL == boot) {
                boot = b + 1;
                *b = '\0';
            }
        } else if (*b == '.') {                 /* pid */
            pid = b + 1;
            *b = '\0';                          /* terminate hostname */
        }
    }
    if (NULL == pid)
        return -1;

    /* assign */
    if (NULL == (user = chk_salloc(user)) || NULL == (host = chk_salloc (host))) {
        if (user) {
            chk_free(user);
        }
        return -1;
    }

    i->i_user = user;
    i->i_host = host;
    i->i_pid  = (Pid_t) strtol(pid, NULL, 0);
    i->i_boot = (NULL == boot ? 0 : (time_t) strtol(boot, NULL, 0));

    trace_log("\tget: user=%s,host=%s,pid=%lu,boot=%lu\n",
        i->i_user, i->i_host, (unsigned long)i->i_pid, (unsigned long)i->i_boot);
    return (0);
}


/*  Function:           lck_create
 *      Low-level create.
 *
 *  Parameters:
 *      i -                 Lock information.
 *
 *      buf -               Symbolic link buffer.
 *
 *  Returns:
 *      0 if sucessful, otherwise non-zero error code.
 */
static int
lck_create(const Info_t *i, const char *buf)
{
    return sys_symlink(buf, i->i_file);
}


/*  Function:           lck_remove
 *      Low-level remove.
 *
 *  Parameters:
 *      i -                 Lock information.
 *
 *  Returns:
 *      0 if sucessful, otherwise non-zero error code.
 */
static int
lck_remove(const Info_t *i)
{
    if (i->i_file) {
        return sys_unlink(i->i_file);
    }
    return -1;
}


/*  Function:           lock_filename
 *      Create the file to be used as the lock. The file shall be located within the
 *      same directory as the file image with the form:
 *
 *          ".#FILE"
 *
 *  Notes:
 *      If you have a symlink to a file and a lock is created using the symlink, which
 *      means you could have two people editing the same file without locking warning
 *      them about this.
 *
 *      Dereference the symlink to get the real file and use that name for the locking
 *      so that all instances no matter which symlink they go through would end up with
 *      the same lock file.
 */
static char *
lock_filename(const char *file)
{
    char t_realpath[MAX_PATH+1];
    const char *dirend, *name;
    size_t dirlen, len;
    char *buf;

    /* resolve symlinks (if any) */
    if (0 == sys_realpath((const char *)file, t_realpath, sizeof(t_realpath))) {
        file = t_realpath;
    }

    /* find directory end */
    if (NULL == (dirend = sys_pathend(file))) {
        dirlen = 0;
        name = file;
    } else {
        dirlen = (dirend - file) + 1;
        name = ++dirend;
    }

    len = dirlen + strlen(name) + 7;
    if (NULL != (buf = chk_alloc(len))) {
        if (dirlen) {
            strncpy(buf, file, dirlen);
        }
#if defined(__MSDOS__)
        sxprintf(buf+dirlen, len, ".#%s.lnk", name);
#else
        sxprintf(buf+dirlen, len, ".#%s", name);
#endif
    }
    return buf;
}


/*  Function:           lock_create
 *      Create the lock file image, decoding the result creation status.
 *
 *      When using file locks, care must be taken to ensure that operations are atomic.
 *      When creating the lock, the process must verify that it does not exist and then
 *      create it, but without allowing another process the opportunity to create it in
 *      the meantime. Hence when stealing the caller must verify that it is truly the
 *      owner.
 *
 *  Returns:
 *      -2              Exists.
 *      -1              Error.
 *      0               Created.
 *      1               No support available.
 *      2               Stolen (maybe !!).
 *
 */
static int
lock_create(int force, const Info_t *i)
{
    const char *lfile = i->i_file;
    char *buf;
    int ret = -1;

    if (NULL == (buf = info_mak(i))) {
        return -1;
    }

    if (lck_create(i, buf) == 0) {
        ret = 0;
    } else {
        int x_errno = errno;

        switch(x_errno) {
#if defined(ENOSYS) || defined(EPERM)
#if defined(ENOSYS)
        case ENOSYS:
#endif
#if defined(EPERM)
        case EPERM:
#endif
            ewprintf("Symbolic links not supported - locking disabled.");
            ret = 1;
            break;
#endif

#if defined(EACCES) || defined(EROFS)
#ifdef EACCES
        case EACCES:
#endif
#ifdef EROFS
        case EROFS:
#endif
            ewprintf("No permission to lock file - locking disabled.");
            ret = 1;
            break;
#endif
        case EEXIST:
            if (! force) {
                ret = -2;                       /* inform caller, inturn ask user */

            } else if (lck_remove(i) == 0 && lck_create(i, buf) == 0) {
                ret = 2;                        /* stolen */
            }
            /*FALLTHRU*/

        default:
            if (ret == -1)
                ewprintf("Unable to create lockfile %s : %d", lfile, x_errno);
            break;
        }
    }
    chk_free(buf);
    return ret;
}


/*  Function:           lock_ask
 *      Queries the action to take on the specified file.
 *
 *  Description:
 *      Queries the action to take on the specified file.
 *
 *          y/p             Proceed. Go ahead and edit the file despite its being
 *                          locked by someone else.
 *
 *          n/q             Quit. This causes an error (file-locked) and the
 *                          modification you were trying to make in the buffer does
 *                          not actually take place.
 *
 *          s               Steal the lock. Whoever was already changing the file
 *                          loses the lock, and you gain the lock.
 *
 *  Parameters:
 *      file -              Filename.
 *
 *  Return:
 *      If 0 is returned, then proceed with no lock. If 1, then force the
 *      lock, if -1 then abort with no lock.
 *
 */
static int
lock_ask(const char *file, Info_t *i)
{
    int ret = LCK_AABORT;                       /* default return */
    int hostlen = hostlength(i->i_host);        /* length of first component */
    char reply[4];
    char *prompt;
    size_t len;

    len = strlen(file) + strlen(i->i_host) + strlen(i->i_user) + 100;
    if (NULL != (prompt = chk_alloc(len))) {
        int retries;				/* retry count */

        sxprintf(prompt, len, "\001%s locked by %s@%.*s.%lu, edit [^y^n^s]?",
            file, i->i_user, hostlen, i->i_host, (unsigned long)i->i_pid);

        for (retries = 0; retries < 10; ++retries) {
            int ch;

            /*
             *  allow Emacs and GRIEF style replies
             */
            reply[0] = '\0';
            if (ABORT == ereply1(prompt, reply, sizeof(reply))) {
                break;                          /* ESC/abort */
            }
            if (*reply) {
                ch = tolower(reply[0]);
                if (ch == 's') {
                    ret = LCK_ASTEAL;           /* steal */
                    break;
                } else if (ch == 'p' || ch == 'y') {
                    ret = LCK_APROCEED;         /* yes/proceed */
                    break;
                } else if (ch == 'a' || ch == 'n') {
                    break;                      /* no/abort */
                }
            }
        }
        chk_free(prompt);
    }
    return ret;
}


static int
lock_try(const char *file, Info_t *i, int ask)
{
    Info_t i_curr;
    int status = LCK_SSUCCESS;
#define SERROR                  -1
#define SFORCE                  -2
    int retries = 0;

    trace_log("lock_try(%s,%d)\n", file, ask);
    do {
        int ret;				 /* lock loop */

        /* create or read-current */
        if ((ret = lock_create (status == SFORCE ? TRUE : FALSE, i)) == -1) {
            status = SERROR;                    /* error */
            break;
        }
        if (1 == ret) {
            status = LCK_SNOTLOCKED;            /* locking not supported */
            break;
        }

        /* file either exists or was (re)created */
        if (info_get(i->i_file, &i_curr) == -1) {
            if (++retries > 5) {
                status = SERROR;                /* error */
                break;
            }
            continue;                           /* retry */
        }
        retries = 0;

        /* compare current owner details */
        switch (info_cmp(i, &i_curr)) {
        case INFO_SAME:     /* same/us */
            trace_log("\towner\n");
            status = (status == SFORCE ? LCK_SFORCED : LCK_SSUCCESS);
            break;

        case INFO_USER:     /* diff owner */
            if (sys_running(i_curr.i_pid) == 0) {
                trace_log("\tdiff owner (dead)\n");
                status = SFORCE;
                break;
            }
            /*FALLTHRU*/

        case INFO_HOST:     /* diff host or (owner still running, fallthru) */
            trace_log("\tdiff owner\n");
            if (ask && (ret = lock_ask(file, &i_curr)) >= 0)  {
                if (ret == LCK_APROCEED) {
                    status = LCK_SNOTLOCKED;    /* dont lock */
                } else if (ret == LCK_ASTEAL) {
                    status = SFORCE;            /* next pass, force lock */
                } else {
                    status = SERROR;            /* others?? */
                }
                eclear();
            } else {
                ewprintf("%s is locked by another process", file);
                status = SERROR;
            }
            break;
        }
        info_free (&i_curr);                    /* release local */
    } while (status == SFORCE);

    if (status < 0) {
        return -1;
    }
    return status;
}


static Lock_t *
node_new(const char *file)
{
    Lock_t *l;

    if (NULL != (l = chk_calloc(sizeof(Lock_t), 1))) {
        if (NULL == (l->l_file = chk_salloc(file)) ||
                info_set(&l->l_info, file) != 0) {
            node_free(l);
            l = NULL;
        }
    }
    return (l);
}


static void
node_free(Lock_t *l)
{
    if (l) {
        info_free(&l->l_info);
        chk_free((char *)l->l_file);
        chk_free(l);
    }
}


static Lock_t *
node_find(const char *file)
{
    LockList_t *q = &lockq;                     /* lock list */
    Lock_t *l;

    //lock
    for (l = TAILQ_FIRST(q); l; l = TAILQ_NEXT(l, l_node)) {
        if (file_cmp(file, l->l_file) == 0)
            return l;
    }
    //unlock
    return NULL;
}


static void
node_release(Lock_t *l)
{
    if (l) {
        trace_log("lock_release(%s,%d)\n", l->l_file, l->l_status);
        trace_log("\t     lock=%s\n", l->l_info.i_file);

	//lock
        if (l->l_status != LCK_SNOTLOCKED) {
            lck_remove(&l->l_info);
        }
        TAILQ_REMOVE(&lockq, l, l_node);
	node_free(l);
	//unlock
    }
}


int
flock_init(void)
{
    lockboot = time(NULL);
    TAILQ_INIT(&lockq);                         /* initialise lock chain */
    return 0;
}


void
flock_close(void)
{
    Lock_t *l;

    while (NULL != (l = TAILQ_FIRST(&lockq))) {
        node_release(l);
    }
}


int
flock_set(const char *file, int ask)
{
    char path[MAX_PATH];
    int status = -1;
    Lock_t *l;

    if (NULL == file || !*file) {
        return 0;                               /* empty file specification */
    }
    file_canonicalize(file, path, sizeof(path));
    trace_log("flock_set(%s)\n", file);
    if (NULL != (l = node_find(file))) {	/* pre-existing? */
        ++l->l_refs;                            /* decrement ref count */
        return l->l_status;                     /* return current status */
    }

    if (NULL != (l = node_new(file))) {
        status = lock_try(l->l_file, &l->l_info, ask);
    }
    if (status >= 0) {
	//lock
        TAILQ_INSERT_TAIL(&lockq, l, l_node);
        l->l_status = status;
        l->l_refs = 1;
	//unlock
    } else {
        node_free(l);
    }
    return status;
}


void
flock_clear(const char *file)
{
    Lock_t *l;

    if (NULL != file && file[0] != '\0') {
        trace_log("flock_clear(%s)\n", file);

        if (NULL != (l = node_find(file))) {
            if (--l->l_refs == 0)               /* decrement ref count */
                node_release(l);
        }
    }
}
#endif  /*lock*/
