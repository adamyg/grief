#include <edidentifier.h>
__CIDENT_RCSID(gr_sysinfo_c,"$Id: sysinfo.c,v 1.52 2022/05/31 16:18:21 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: sysinfo.c,v 1.52 2022/05/31 16:18:21 cvsuser Exp $
 * System information services.
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
#include <edenv.h>                              /* gputenvv(), ggetenv() */
#include <libstr.h>                             /* str_...()/sxprintf() */

#include "sysinfo.h"                            /* public interface */

#if defined(HAVE_PWD_H)
#include <pwd.h>
#endif
#if defined(unix) || defined(__APPLE__) || defined(HAVE_NETDB_H)
#include <netdb.h>
#endif
#if defined(linux)
#ifndef  _GNU_SOURCE
#define  _GNU_SOURCE
#endif
#include <sys/types.h>
#endif

#if defined(unix) || defined(_AIX) || defined(linux) || defined(__APPLE__)
#include <sys/utsname.h>
#endif

#if defined(__CYGWIN__)
#include <sys/cygwin.h>                         /* cygwin_conv_path */
#endif

#if defined(__APPLE__)
#include <libproc.h>                            /* proc_pidpath */
#endif

#if defined(WIN32) || defined(__CYGWIN__)
#if !defined(WINDOWS_MEAN_AND_LEAN)
#define  WINDOWS_MEAN_AND_LEAN
#include <windows.h>
#endif
#include <userenv.h>                            /* GetUserProfileDirectory() */
#include <shlobj.h>                             /* SHGetFolderPath */
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "shfolder.lib")
#pragma comment(lib, "userenv.lib")
#endif   /*WIN32*/

#include "debug.h"
#include "system.h"
#include "getpwd.h"
#include "main.h"

static const char *     tmpdir2(const char *env);
static int              fullqual(const char *host);
static const char *     resolve_execname(const char *arg0);


/*  Function:           sysinfo_username
 *      Determine the username.
 *
 *  Notes:
 *      Result is cached on first call and returned on all others.
 *
 *  Parameters
 *      none
 *
 *  Returns:
 *      nothing
 */
const char *
sysinfo_username(char *buf, int len)
{
    static const char *user = NULL;             /* one-shot */

    if (NULL == user) {
#if defined(HAVE_PWD_H) && !defined(WIN32)
        passwd_t passwd = {0};
        struct passwd *pw;
        if (NULL != (pw = sys_getpwuid(&passwd, getuid()))) {
            user = pw->pw_name;
        }
#endif

        if ((NULL == user || 0 == *user) &&
                (NULL == (user = ggetenv("USER")) || 0 == *user) &&
                (NULL == (user = ggetenv("LOGNAME")) || 0 == *user) &&
                (NULL == (user = ggetenv("USERNAME")) || 0 == *user)) {
            user = "unknown";
        }
        user = chk_salloc(user);
        chk_leak(user);

#if defined(HAVE_PWD_H) && !defined(WIN32)
        sys_getpwend(&passwd, pw);              /* release system resources */
#endif
    }
    return user && buf ? strxcpy(buf, user, len) : user;
}


/*  Function:           sysinfo_homedir
 *      Determine the home directory.
 *
 *  Notes:
 *      Result is cached on first call and returned on all others.
 *
 *  Parameters
 *      none
 *
 *  Returns:
 *      nothing
 */
const char *
sysinfo_homedir(char *buf, int len)
{
    static const char *home = NULL;             /* one-shot */

    if (NULL == home) {
#if defined(WIN32)
        char t_path[MAX_PATH];
#endif
        const char *p = NULL;

#if defined(HAVE_PWD_H) && !defined(WIN32)
        passwd_t passwd = {0};
        struct passwd *pw = NULL;

        if (NULL != (pw = sys_getpwlogin(&passwd))) {
            if (pw->pw_dir && *pw->pw_dir) {
                p = pw->pw_dir;                 /* non-null */
            }
        }
#endif  /*HAVE_PWD_H*/

        if ((NULL == p || !*p) &&               /* common enviromental setting */
                (NULL == (p = ggetenv("HOME")) || 0 == *p)) {
            p = NULL;
        }

#if defined(WIN32)
        if (! p) {
            const char *env;

            t_path[0] = 0;                      /* XP+ */
            if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_PROFILE, NULL, 0, t_path)) && *t_path) {
                t_path[sizeof(t_path) - 1] = 0;
                if (0 == sys_access(t_path, 0)) {
                    p = t_path;
                } else {
                    char buffer[MAX_PATH*2];

                    sxprintf(buffer, sizeof(buffer), "Home directory is not accessible\n<%s>", t_path);
                    MessageBoxA(0, buffer, "Error", MB_OK);
                }
            }
            if (!p) {
                if (NULL != (env = ggetenv("USERPROFILE")) && *env) {
                    p = env;
                }
            }
        }
#endif  /*WIN32*/

        if (p) {
            home = chk_salloc(p);               /* clone env variable */
            chk_leak(home);

        } else {                                /* system spec default */
#if defined(DOSISH)         /* X: */
            const char *homepath;
            size_t homepathlen;
            char *doshome;

            if (NULL == (homepath = ggetenv("HOMEPATH"))) {
                homepath = "/";
            }

            homepathlen = strlen(homepath);     /* X:[path]\0 */
            doshome = chk_alloc(4 + homepathlen);

            if (NULL != (p = ggetenv("HOMEDRIVE")) && isalpha((unsigned char)*p)) {
                doshome[0] = *p;
            } else {
                doshome[0] = (char)sys_drive_get();
            }

            doshome[1] = ':';
            memcpy(doshome + 2, homepath, homepathlen + 1);
            home = doshome;
            chk_leak(home);
#else
            home = ".";                         /* current working directory */
#endif
            fprintf(stderr, "\nWARNING: unable to resolve home directory, using '%s'\n", home);
            fflush(stderr);
        }

#if defined(HAVE_PWD_H) && !defined(WIN32)
        sys_getpwend(&passwd, pw);              /* release system resources */
#endif
    }
    return (home && buf ? strxcpy(buf, home, len) : home);
}


/*  Function:           sysinfo_tmpdir
 *      Determine the tmp directory.
 *
 *  Notes:
 *      Result is cached on first call and returned on all others.
 *
 *  Parameters
 *      none
 *
 *  Returns:
 *      nothing
 */
const char *
sysinfo_tmpdir(void)
{
    static const char *tmpdir = NULL;           /* one-shot */

    if (NULL == tmpdir) {
#if defined(__MSDOS__) || defined(WIN32)
        char t_path[MAX_PATH];
#endif
        const char *p = NULL;

        p = tmpdir2("GRTMP");                   /* GRIEF override */
        if (NULL == p || !*p) {
            p = tmpdir2("BTMP");                /* BRIEF override */
        }

#if defined(WIN32) && !defined(__CYGWIN__)
        if (NULL == p || !*p) {
            int pathlen;

            t_path[0] = 0;                      /* XP+ */
            if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, t_path)) && *t_path) {
                t_path[sizeof(t_path) - 1] = 0;
                if (0 == sys_access(t_path, 0)) {
                    p = t_path;
                }
            } else {
                t_path[0] = 0;                  /* XP+ */
                if ((pathlen = GetTempPathA(sizeof(t_path), t_path)) > 0) {
                    if (pathlen > 1 && t_path[pathlen-1] == '\\') {
                        --pathlen;              /* remove trailing delimiter */
                    }
                    t_path[pathlen] = 0;
                    if (0 == sys_access(t_path, 0)) {
                        p = t_path;
                    }
                }
            }
        }
#endif

        if ((NULL == p || !*p)
                && NULL == (p = tmpdir2("TMP")) /* standard unix tmp */
#if defined(__MSDOS__) || defined(__CYGWIN__)
                && NULL == (p = tmpdir2("TEMP"))
                && NULL == (p = tmpdir2("TMPDIR"))
#endif
            )
        {
#if defined(_VMS)
            p = "[-]";
#else
            static const char *tmpdirs[] = {
                "/tmp",
#if defined(unix) || defined(__APPLE__)
                "/var/tmp",
#endif
#if defined(__MSDOS__)
                "c:/tmp",
                "c:/temp",
                "c:/windows/temp",
                "/temp",
                "/windows/temp",
#endif
#if defined(__CYGWIN__)
                "/cygdrive/c/temp",
                "/cygdrive/c/windows/temp",
#endif
                NULL
                };
#if defined(__MSDOS__)
            const char *homedir = ggetenv("HOMEDRIVE");
#endif
            int d;

            p = sysinfo_homedir(NULL, -1);      /* home directory, default */
            for (d = 0; tmpdirs[d]; ++d) {
                const char *xtmpdir = tmpdirs[d];

#if defined(__MSDOS__)
                if (homedir && *homedir && ':' == xtmpdir[1]) {
                    if (isalpha((unsigned char)*homedir)) {
                        strxcpy(t_path, xtmpdir, sizeof(t_path));
                        t_path[0] = *homedir;
                        xtmpdir = t_path;
                    } else {
                        homedir = NULL;
                    }
                }
#endif
                if (0 == sys_access(xtmpdir, W_OK)) {
                    p = tmpdirs[d];
                    break;
                }
            }
#endif  /*!VMS*/
        }

        tmpdir = chk_salloc(p);
        chk_leak(tmpdir);
    }
    return tmpdir;
}


static const char *
tmpdir2(const char *env)
{
    const char *p;

    if ((p = ggetenv(env)) != NULL && p[0] &&
                0 == sys_access(p, 0)) {
        return p;
    }
    return NULL;
}


const char *
sysinfo_execname(const char *arg0)
{
    static const char *execname = NULL;         /* one-shot */

    if (NULL == execname) {
        execname = resolve_execname(arg0);
    }
    return (execname ? execname : arg0);
}


static const char *
resolve_execname(const char *name)
{
    char t_name[MAX_PATH + 1] = {0};
    size_t namelen = 0;
    char *cp = NULL;

    __CUNUSED(namelen)
    __CUNUSED(cp)

#if defined(WIN32) || defined(__CYGWIN__)
    namelen = GetModuleFileNameA(NULL, t_name, sizeof(t_name)-1);
    trace_log("exename: <%s/%u>\n", t_name, (unsigned)namelen);
    t_name[namelen] = 0;

#if defined(__CYGWIN__)
    if (namelen > 0) {
        namelen = cygwin_conv_path(CCP_WIN_A_TO_POSIX|CCP_ABSOLUTE, t_name, NULL, 0);
        if (namelen > 0 &&
                NULL != (cp = chk_alloc(namelen + 1))) {
            (void) cygwin_conv_path(CCP_WIN_A_TO_POSIX|CCP_ABSOLUTE, t_name, cp, namelen + 1);
            name = cp;
            goto complete;
        }
    }
#endif  /*CYGWIN*/

#else   /*unix and friends */
    char t_path[MAX_PATH+1] = {0}, t_self[64];
    struct stat sb = {0};
    int source = -1;
    const char *path;

    trace_log("execname: <%s>\n", name);

#if defined(HAVE_GETEXECNAME)
    if (realpath(getexecname(), t_name)) {
        name = t_name;
    }
#endif

    if (name != t_name) {
#if defined(linux) || defined(__linux__)
        sxprintf(t_self, sizeof(t_self), "/proc/%d/exe", getpid());
        if ((namelen = readlink("/proc/self/exe", t_name, sizeof(t_name))) > 0 ||
                (namelen = readlink(t_self, t_name, sizeof(t_name))) > 0) {
            t_name[namelen] = 0;
            name = t_name;
            source = 1;
        }

#elif defined(__sun__) || defined(sun)
        if ((namelen = readlink("/proc/self/paths/a.out", t_name, sizeof(t_name))) > 0) {
            t_name[namelen] = 0;
            name = t_name;
            source = 2;
        }

#elif defined(__APPLE__)
        if (proc_pidpath(getpid(), t_name, sizeof(t_name)) != 0) {
            name = t_name;
            source = 3;

        } else if (_NSGetExecutablePath(t_name, sizeof(t_name)) != -1) {
            name = t_name;                      /* Mac 10.2+ */
            source = 4;
        }

#elif defined(BSD)
        if ((namelen = readlink("/proc/curproc/file", t_name, sizeof(t_name))) > 0) {
            t_name[namelen] = 0;
            name = t_name;                      /* procfs */
            source = 5;

        } else {
            int mib[4] = {CTL_KERN, KERN_PROC, KERN_PROC_PATHNAME, -1};
            size cb = sizeof(t_name);

            if (-1 != sysctl(mib, 4, t_name, &cb, NULL, 0)) {
                name = t_name;                  /* alt method */
                source = 6;
            }
        }
#endif  /*unix||linux*/
    }

    if (NULL == name || 0 == *name) {
        return NULL;
    }

    if (name != t_name) {                       /* try arg0 */
        strxcpy(t_name, name, sizeof(t_name));
        name = t_name;
    }

    trace_log("execname: [%d], <%s>\n", source, name);

#if defined(FILEIO_EXEEXT)
    if ((namelen = strlen(t_name)) < 4 ||
            0 != str_icmp((t_name + namelen) - (sizeof(FILEIO_EXEEXT) - 1), FILEIO_EXEEXT)) {
        strcat(t_name, FILEIO_EXEEXT);
    }
#endif

    if (sys_isabspath(name)) {                  /* absolute path */
        if (-1 != sys_stat(name, &sb) && !S_ISDIR(sb.st_mode)) {
            trace_log("execname: [abs], <%s>\n", name);
            goto done;
        }
                                                /* relative, resolve and test */
    } else if (0 == sys_realpath((const char *)name, t_path, sizeof(t_path)) && t_path[0]) {
        if (-1 != sys_stat(t_path, &sb) && !S_ISDIR(sb.st_mode)) {
            trace_log("execname: [rel], <%s>\n", t_path);
            name = t_path;
            goto done;
        }
    }

    if (NULL != (path = ggetenv("PATH"))) {
        const char *app = sys_basename(name);
        const char *delim = NULL;

        while (path && *path) {
            if (NULL != (delim = strchr(path, FILEIO_DIRDELIM))) {
                if (delim == path) {
                    ++path;
                    continue;
                }
                sxprintf(t_path, sizeof(t_path), "%.*s%c%s", (int)(delim - path), path, FILEIO_PATHSEP, app);
                path = delim + 1;
            } else {
                sxprintf(t_path, sizeof(t_path), "%s%c%s", path, FILEIO_PATHSEP, app);
                path = NULL;
            }

            trace_log("execname: [path], <%s>\n", t_path);
            if (sys_stat(t_path, &sb) >= 0 && !S_ISDIR(sb.st_mode)) {
                name = t_path;
                goto done;
            }
        }
    }

done:;
#endif  /*WIN32*/

    name = chk_salloc(name);
#if defined(DOSISH)
    cp = (char *)name;
    while (NULL != (cp = strchr(cp, '\\'))) {
        *cp++ = '/';
    }
#endif

#if defined(__CYGWIN__)
complete:
#endif
    trace_log("appname: <%s>\n", name);
    return name;
}


/*  Function:           sysinfo_hostname
 *      Determine the hostname.
 *
 *  Notes:
 *      Result is cached on first call and returned on all others.
 *
 *  Parameters
 *      none
 *
 *  Returns:
 *      nothing
 */
const char *
sysinfo_hostname(char *buf, int len)
{
    static const char * host = NULL;            /* one-shot */

    if (NULL == host) {
#if !defined(MAXHOSTNAMELEN)
#define MAXHOSTNAMELEN          256
#endif
#if defined(unix) || defined(__APPLE__)
        char host_buf[MAXHOSTNAMELEN];
#endif
        char domain_buf[MAXHOSTNAMELEN];
#if defined(unix) || defined(__APPLE__) || defined(HAVE_NETDB_H)
        struct hostent *hent = NULL;
#endif
        const char * p = NULL;

        /* default gethostname(), then env */
#if defined(unix) || defined(__APPLE__)
        if (0 == gethostname(host_buf, sizeof(host_buf)) && host_buf[0]) {
            p = host_buf;
        }
#endif

        if ((NULL == p || '\0' == p[0]) &&      /* scan env */
                (NULL == (p = ggetenv("HOSTNAME")) || '\0' == p[0]) &&
                (NULL == (p = ggetenv("COMPUTERNAME")) || '\0' == p[0])) {
            return NULL;
        }

        host = (char *)p;

        /* gethostbyname seems a better option */
#if defined(unix) || defined(__APPLE__) || defined(HAVE_NETDB_H)
        if (NULL != (hent = gethostbyname (host)) &&
                    NULL != hent->h_name && hent->h_name[0] != 0) {

            host = hent->h_name;

            if (!fullqual(host)) {              /* test 4 a better match */
                char **aliases;

                if (NULL != (aliases = hent->h_aliases))
                    while (*aliases != NULL) {
                        if (fullqual(*aliases)) {
                            host = *aliases;
                            break;
                        }
                        aliases++;
                    }
            }
        }
#endif  /*unix*/

        if (fullqual(host) ||
                NULL == sysinfo_domainname(domain_buf, sizeof(domain_buf))) {
            trace_log("HOST=%s\n", host);
            host = chk_salloc(host);
            chk_leak(host);

        } else {
            const char *h = host;
            const char *d = (domain_buf[0] == '.' ? domain_buf + 1 : domain_buf);
            size_t sz;

            if (0 == strcmp(d, "(none)")) {     /* domain unknown, remove */
                d = "";
            }

            sz = strlen(h) + strlen(d) + 2;
            if (NULL != (host = chk_alloc(sz))) {
                sprintf((char *)host, (*d ? "%s.%s" : "%s"), h, d);
                trace_log("HOST=%s\n", host);
                chk_leak(host);
            }
        }
    }
    return host && buf ? strxcpy(buf, host, len) : host;
}


static int
fullqual(const char *netname)
{
    const char *p;

    if (NULL == (p = strchr(netname, '.')) || p == netname) {
        return 0;
    }
    if ((unsigned int)((p - netname) + 1) == strlen(netname)) {
        return 0;
    }
    return 1;
}


/*  Function:           sysinfo_domainname
 *      Determine the domainname.
 *
 *  Notes:
 *      Result is cached on first call and returned on all others.
 *
 *  Parameters
 *      none
 *
 *  Returns:
 *      nothing
 */
const char *
sysinfo_domainname(char *name, int len)
{
    static const char * domain = NULL;          /* one-shot */

    if (NULL == domain) {
#if defined(linux)
        struct utsname u;
#endif
        char buf[1024], buf2[128];              /* magic */

#if defined(linux) && defined(_UTSNAME_DOMAIN_LENGTH)
        if (uname(&u) != -1) {
#if defined(__USE_GNU)
            domain = u.domainname;              /* linux specific uname() extension */
#else
            domain = u.__domainname;
#endif
        }
#endif  /*linux && _UTSNAME_DOMAIN_LENGTH*/

        if (NULL == domain) {
            FILE *resolv;
            char *p, *s;
                                                /* std unix locations */
            if (NULL != (resolv = (fopen("/etc/resolv.conf", "r"))) ||
                    NULL != (resolv = (fopen("/etc/config/resolv.conf", "r")))) {

                while (fgets(buf, sizeof(buf), resolv)) {
                    p = buf;

                    /* Comments (and then consume leading white) */
                    if (p[0] == '#') continue;
                    while (isspace(*p)) ++p;

                    /* The keyword and value must appear on a single line.  Start
                       the line with the keyword */
                    if (strncmp(p, "domain", 6) && strncmp(p, "search", 6) == 0)
                        continue;

                    s = (p += 6);               /* skip keyword */

                    while (isspace(*p)) ++p;    /* ship whitespace */
                    if (*p == 0 || p == s) continue;

                    s = p;
                    while (*p != '\n' && isgraph(*p)) ++p;
                    if (p == s) continue;

                    *p = 0;                     /* terminate buffer */

                    strxcpy(buf2, (const char *)s, sizeof(buf2));
                    domain = buf2;              /* found */

                    /*last one takes precedence, keep searching*/
                }
            }
        }

#if defined(WIN32)                              /* scan env */
        if ((NULL == domain || '\0' == domain[0]) &&
                (NULL == (domain = ggetenv("USERDNSDOMAIN")) || '\0' == domain[0])) {
            domain = NULL;
        }
#endif

        if (domain) {
            trace_log("DOMAIN=%s\n", domain);
            domain = chk_salloc(domain);
            chk_leak(domain);
        }
    }
    return domain && name ? strxcpy(name, domain, len) : domain;
}
/*end*/


