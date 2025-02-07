#include <edidentifier.h>
__CIDENT_RCSID(gr_wild_c,"$Id: wild.c,v 1.43 2025/02/07 03:03:22 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: wild.c,v 1.43 2025/02/07 03:03:22 cvsuser Exp $
 * Wild card and basic pattern (not regexp) matching support.
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
#include <edfileio.h>
#include <eddir.h>
#include <chkalloc.h>
#include <patmatch.h>
#include <libstr.h>                             /* str_...()/sxprintf() */
#include "../libvfs/vfs.h"

#include "buffer.h"
#include "echo.h"
#include "file.h"
#include "sysinfo.h"
#include "system.h"
#include "wild.h"

#if defined(STANDALONE)
#undef  chk_alloc
#undef  chk_free
#undef  chk_salloc
#define chk_alloc           malloc
#define chk_free            free
#define chk_salloc          strdup
#define ggetenv             getenv
#if defined(__OS2__)    
    #define INCL_BASE
    #define INCL_NOPM
    #include <os2.h>
#endif
#endif

#define SHE_TOO_MANY_NAMES  -1
#define MAX_NAMES           4096
#define NAME_SIZE           MAX_PATH

#if defined(DOSISH)
#define ISSLASH(c)          ('/' == (c) || '\\' == (c))
#else
#define ISSLASH(c)          ('/' == (c))
#endif

#if !defined(_VMS)

typedef struct {
    int wild_;                                  /* wild card match */
    vfs_dir_t *dirp_;                           /* current open directory */
    char name_[DIRSIZ + 1];                     /* MAGIC */
} shell_dir_t;

static char **              shell_wild(const char *file);

static shell_dir_t *        shell_dir_open(const char *dirname, int wild);
static const char *         shell_dir_read(shell_dir_t *dir, const char *suffix);
static void                 shell_dir_close(shell_dir_t *dir);

static int                  shell_compare(const void *a1, const void *a2);
static void                 shell_free(char **names, int name_cnt);
static const char *         shell_namesplit(const char *file, char *buf);


/*  Function:           shell_expand
 *      The following function attempts to emulate the Unix shells
 *      attempt at globbing.
 *
 *      We return an array of pointers to strings containing the names
 *      of the files which match.
 *
 *      Both the strings and the array need to be freed by the caller,
 *      using chk_free(), alternatively using shell_release().
 */
char **
shell_expand(const char *file)
{
    char name[MAX_PATH];
    struct stat st;

    if (NULL == file_tilder(file, name, sizeof(name))) {
        return NULL;                            /* expand home directory references */
    }

#if defined(HAS_LSTAT)
    if (vfs_lstat(name, &st) > 0 && S_ISDIR(st.st_mode)) {
#else
    if (vfs_stat(name, &st) > 0 && S_ISDIR(st.st_mode)) {
#endif
        strcat(name, "/*");                     /* directory, auto-descend */
    }

#if defined(DOSISH)         /* X: */
    if (*name && ':' == name[1]) {
        /*
         *  remove drive designator, set, expand and then restore drive.
         */
        const int curdriv = sys_drive_get(),
                    newdrive = (unsigned char)(name[0]);
        char **array = NULL;

        if (isalpha(newdrive) && -1 != sys_drive_set(newdrive)) {
            if (NULL != (array = shell_wild(name + 2))) {
                char buf[NAME_SIZE];
                unsigned i;

                for (i = 0; array[i]; ++i) {
                    if (array[i][0]) {
                        buf[0] = name[0];
                        buf[1] = ':';
                        strcpy(buf + 2, array[i]);
                        strcpy(array[i], buf);
                    }
                }
            }
            sys_drive_set(curdriv);
        }
        return array;
    }
#endif  /*DOSISH*/

    return shell_wild(name);
}


void
shell_release(char **files)
{
    unsigned i;

    if (files) {
        for (i = 0; files[i]; ++i) {
            chk_free(files[i]);
        }
        chk_free(files);
    }
}


/*  Function:           shell_expand0
 *      Expand a filename into a single thing (e.g. when ~ occurs or whatever).
 *
 */
const char *
shell_expand0(const char *file, char *buf, int len)
{
    char **files;

    buf[0] = 0;

    if (NULL == strpbrk((char *)file, "~*?[$")) {
        if (buf != file) {
            strxcpy(buf, file, len);            /* no specials, just return */
        }

    } else if (NULL != (files = shell_expand(file))) {
        unsigned j;

        for (j = 0; files[j]; ++j) {
            if (buf[0] == 0 && files[j][0]) {   /* return first match */
                strxcpy(buf, (const char *)files[j], len);
            }
            chk_free(files[j]);
        }
        chk_free(files);
    }
    return buf;
}


/*  Functon:            wild_file
 *      Following function is called to perform a low level regular
 *      expression comparison of str against 'file'.
 *
 *      This interface enforces the rule that a leading '*' wont match
 *      any files which begin with a '.' as such use '?*' to match all
 *      files.
 *
 *   Parameters:
 *      file - File buffer.
 *      pattern - Pattern.
 *
 *  Returns:
 *       true if matches and false if not.
 */
int
wild_file(const char *file, const char *pattern)
{
    int flags = 0;

    flags |= MATCH_PERIODA;                     /* leading star(*) wont match '.' */
#if defined(NOCASE_FILENAMES)
    flags |= MATCH_NOCASE;                      /* TODO: volume specific */
#endif
    return patmatchx((const char *)pattern, (const char *)file, flags, errorf);
}


/*  Functon:            wild_match
 *      Following function is called to perform a low level regular expression
 *      comparison of str against 'file'.
 *
 *   Parameters:
 *      file - File buffer.
 *      pattern - Pattern.
 *      flags - Control files.
 *
 *  Returns:
 *      true if matches otherwise false if not.
 */
int
wild_match(const char *file, const char *pattern, int flags)
{
#if defined(NOCASE_FILENAMES)
    if (flags & MATCH_AUTOCASE) {
        flags |= MATCH_NOCASE;
    }
#endif
    return patmatchx((const char *)pattern, (const char *)file, flags, errorf);
}


/*  Function:           shell_purge
 *      This function is called to prune the globbed names.
 *
 *      The algorithm can end up putting invalid file names into the
 *      array so we make sure here that they really refer to real gfile.
 *
 *      If we find a file which doesn't exist, we set the first
 *      character to a null.
 */
static char **
shell_purge(char **files, int wild)
{
    const char *cp;
    int i;

    if (NULL == files || !wild) {
        return files;
    }

    for (i = 0; files[i]; ++i) {
        cp = files[i];
        if (cp[0]) {
            struct stat sb;

            while (*cp == '/' && cp[1] == '/') {
                ++cp;
            }

            if (vfs_stat(cp, &sb) < 0) {
                files[i][0] = 0;
            } else {
                strcpy(files[i], cp);
            }
        }
    }
    return files;
}


/*  Function:           shell_wild
 *      Function to perform the real work of filename globbing.
 *
 *   Parameters:
 *      file - File pattern.
 *
 *   Returns:
 *      NULL terminated array of file paths.
 */
static char **
shell_wild(const char *file)
{
    char suffix[MAX_PATH];                      /* was 256 */
    int i, j, k, ecnt;
    char **names;
    int found_wild = FALSE;
    int cnt = 0;

    if (NULL == (names = chk_alloc(MAX_NAMES * sizeof(char *)))) {
        return NULL;
    }

    if ('/' == file[0]) {                       /* absolute */
        names[0] = chk_alloc(NAME_SIZE);
        if ('/' == file[1]) {                   /* UNC */
            strcpy(names[0], "//");
            file += 2;
        } else {
            strcpy(names[0], "/");
            ++file;
        }
        cnt = 1;
    }

    do {
        char *prefix;

        *suffix = 0;
        file = shell_namesplit(file, suffix);   /* split spec (fred/... ==> 'fred' and '...' */

        if (suffix[0] == '$') {                 /* $env within file specs, expand */
            const char *env = getenv(suffix + 1);
            if (env) {
                strcpy(suffix, env);
            }
        }

        if (suffix[0] && strpbrk(suffix, "*?[")) {
            found_wild = TRUE;
            j = cnt;

            for (i = 0; i <= j; ++i) {
                shell_dir_t *dir = NULL;

                if (i == j) {
                    if (j == 0) {
                        prefix = NULL;
                    } else {
                        break;
                    }
                } else {
                    prefix = names[i];
                    if (0 == *prefix) {
                        continue;
                    }
                }

                if (NULL == (dir = shell_dir_open(prefix, 1))) {
                    return NULL;                /* source directory not accessible */
                }

                for (ecnt = 0;; ++ecnt) {
                    const char *cp;

                    if (NULL == (cp = shell_dir_read(dir, suffix))) {
                        if (j) {
                            names[i][0] = 0;
                        }
                        shell_dir_close(dir);
                        break;
                    }

                    names[cnt] = chk_alloc(NAME_SIZE);
                    if (prefix) {
                        if (prefix[strlen(prefix) - 1] == '/') {
                            sprintf(names[cnt], "%s%s", prefix, cp);
                        } else {
                            sprintf(names[cnt], "%s/%s", prefix, cp);
                        }
                    } else {
                        strcpy(names[cnt], cp);
                    }

                    if (++cnt >= MAX_NAMES - 1) {
                        shell_free(names, cnt);
                        shell_dir_close(dir);
                        return NULL;
                    }
                }
            }
        } else {
            if (0 == cnt) {
                names[0] = chk_alloc(NAME_SIZE);
                strxcpy(names[0], suffix, NAME_SIZE);
                ++cnt;
            } else {
                for (k = 0; k < cnt; ++k) {
                    const size_t namelen = strlen(names[k]);

                    if (0 == namelen || '/' != names[k][namelen-1]) {
                        strcat(names[k], "/");
                    }
                    strcat(names[k], suffix);
                }
            }
        }
    } while (file);
    names[cnt] = NULL;

    qsort(names, (size_t) cnt, sizeof(char *), shell_compare);
    return shell_purge(names, found_wild);
}



/*
 *  Glob filenames within a string
 */
char *
wild_glob(const char *pattern)
{
    static char const toks[] = " \t\r\n";

    const size_t patlen = strlen(pattern) + 1;
    size_t outlen = (patlen < 256 ? 512 : patlen * 2);

    const char *cp;
    char *pat, *out;
    int cursor = 0;

    if (NULL == (pat = chk_alloc(patlen)) || NULL == (out = chk_alloc(outlen))) {
        chk_free(pat);
        return NULL;
    }
    memcpy(pat, pattern, patlen);

    for (cp = strtok((char *)pat, toks); NULL != cp; cp = strtok(NULL, toks)) {
        char **files = shell_wild(cp);

        if (files) {
            unsigned i;
            size_t sz = 0;

            for (i = 0; files[i]; ++i) {        /* size result */
                if (files[i][0]) {
                    sz += strlen(files[i]) + 1;
                }
            }

            if (sz) {                           /* check and expand if required */
                if ((cursor + --sz) >= outlen) {
                    outlen += sz + 511;
                    outlen -= (outlen % 512);
                    out = chk_realloc(out, outlen);
                }

                for (i = 0; files[i]; ++i) {    /* flatten result */
                    if (files[i][0]) {
                        cursor += sprintf(out + cursor, "%s%s", (cursor ? " " : ""), files[i]);
                    }
                    assert(cursor < (int)outlen);
                }
            }
            shell_release(files);

        } else {
            cursor += sprintf(out + cursor, "%s%s", cursor ? " " : "", cp);
            assert(cursor < (int)outlen);
        }
    }
    chk_free(pat);
    return out;
}


void
wild_globfree(char *out)
{
    chk_free(out);
}


/*  Function:           shell_compare
 *      Compare to filenames, being the result of a expand operation.
 *
 */
static int
shell_compare(const void *a1, const void *a2)
{
    const void * const * p1 = a1;
    const void * const * p2 = a2;

    return file_cmp(*p1, *p2);
}


/*  Function:           shell_free
 *      Release shell_expand() storage, internal version.
 *
 */
static void
shell_free(char **files, int cnt)
{
    if (files) {
        while (cnt-- > 0) {
            chk_free(files[cnt]);
        }
        chk_free(files);
    }
}


/*  Function:           shell_dir_xxx
 *      Directory iterator.
 *
 */
static shell_dir_t *
shell_dir_open(const char *dirname, int wild)
{
    const char *t_dirname = (dirname && *dirname ? dirname : ".");
    shell_dir_t *dir = NULL;
    vfs_dir_t *dirp = NULL;                     /* current open directory */
    struct stat sb;

    if (vfs_stat(t_dirname, &sb) < 0 || (sb.st_mode & S_IFDIR) == 0) {
        return NULL;                            /* not a directory */
    }

    if (NULL == (dirp = vfs_opendir(t_dirname)) ||
            NULL == (dir = chk_calloc(sizeof(*dir), 1))) {
        if (dirp) {                             /* access/memory */
            vfs_closedir(dirp);
        }
        return NULL;
    }
    dir->wild_ = wild ? 0x1234 : 0;
    dir->dirp_ = dirp;
    return dir;
}


static const char *
shell_dir_read(shell_dir_t *dir, const char *suffix)
{
    vfs_dir_t *dirp = NULL;                     /* current open directory */
    vfs_dirent_t *de;

    if (dir && NULL != (dirp = dir->dirp_)) {
        const int wild = dir->wild_;
        char *name = dir->name_;

        while ((de = vfs_readdir(dirp)) != NULL) {
            size_t len = strlen(de->d_name);

            if (len > DIRSIZ) len = DIRSIZ - 1;
            memcpy(name, de->d_name, len);
            name[len] = 0;

            if (0 == file_cmp(name, suffix) ||  /* abs or wild-match */
                    (wild && wild_file(name, suffix))) {
                return name;
            }
        }
    }
    return NULL;
}


static void
shell_dir_close(shell_dir_t *dir)
{
    if (dir) {
        assert(0x1234 == dir->wild_ || 0 == dir->wild_);
        vfs_closedir(dir->dirp_);
        chk_free(dir);
    }
}


static const char *
shell_namesplit(const char *file, register char *buf)
{
    if (0 == *file) {
        return NULL;
    }
    while (*file && !ISSLASH(*((const unsigned char *)file))) {
        *buf++ = *file++;
    }
    if (ISSLASH(*((const unsigned char *)file))) {
        ++file;
    }
    *buf = 0;
    return (*file ? file : NULL);
}


#if defined(STANDALONE)
#if defined(DOSISH)                             /* test only */

int
sys_drive_get(void)
{
    ULONG drives;

#if defined(__FLAT__)
    ULONG cur_drive;
    DosQueryCurrentDisk(&cur_drive, &drives);
#else
    USHORT cur_drive;
    DosQCurDisk(&cur_drive, &drives);
#endif
    return cur_drive + 'A' - 1;
}


/*
 *  Set the current drive.
 */
void
sys_drive_set(int ch)
{
    if (islower(ch)) {
        ch = toupper(ch);
    }

#if defined(__MSDOS__)
    {
        union REGS regs;
        regs.h.ah = SELECT;
        regs.h.dl = ch - 'A';
        intdos(&regs, &regs);
    }
#else
#if defined(__FLAT__)
    DosSetDefaultDisk(ch - 'A' + 1);
#else
    DosSelectDisk(ch - 'A' + 1);
#endif
#endif
}

#endif  /*DOSISH*/

int
main(int argc, char ** argv)
{
    char lbuf[MAX_PATH];
    char **files;
    int i;

    while (1) {
        printf("Filename: ");
        gets(lbuf);

        if (lbuf[strlen(lbuf) - 1] == '\n') {
            lbuf[strlen(lbuf) - 1] = 0;
        }
        files = shell_expand(lbuf);
        if (files == (char **) NULL) {
            printf("Expansion error.\n");
            continue;
        }

        for (i = 0; files[i]; ++i) {
            if (files[i][0]) {
                printf("%s ", files[i]);
            }
        }

        shell_release(files);
        putchar('\n');
    }
}
#endif  /*STANDALONE*/
#endif  /*VMS*/

/*end*/
