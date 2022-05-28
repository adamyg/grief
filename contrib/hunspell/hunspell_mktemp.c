/* -*- mode: c; indent-width: 4; -*- */
/*
 *  hunspell mkstemp/mkdtemp implementation
 */

#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>

#if defined(_MSC_VER)
#include <direct.h>
#endif
#include <process.h>
#include <io.h>

#include <Windows.h>

#include "hunspell_mktemp.h"
#undef mkstemp
#undef mkdtemp

static int tmpdir(const char *path, char *t_path, unsigned tmpsiz);
static int gettempfile(char *path, int *fildes);
static int gettempdir(char *path);

#define MKSUCCESS       0
#define MKERROR         -1

#if defined(__MINGW32__)
static __thread char x_templ[MAX_PATH];
#else
__declspec(thread) static char x_templ[MAX_PATH];
#endif


FILE *
hunspell_fdopen(int fd, const char *mode)
{
        return _fdopen(fd, mode);
}


/*
//  NAME
//      mkstemp - make a unique filename
//
//  SYNOPSIS
//      #include <stdlib.h>
//
//      int mkstemp(char *template);
//
//  DESCRIPTION
//
//      The mkstemp() function shall replace the contents of the string pointed to by
//      template by a unique filename, and return a file descriptor for the file open for
//      reading and writing. The function thus prevents any possible race condition between
//      testing whether the file exists and opening it for use.
//
//      Each successful call to mkstemp modifies template. In each subsequent call from the same
//      process or thread with the same template argument, mkstemp checks for filenames that
//      match names returned by mkstemp in previous calls. If no file exists for a given name,
//      mkstemp returns that name. If files exist for all previously returned names, mkstemp
//      creates a new name by replacing the alphabetic character it used in the previously
//      returned name with the next available lowercase letter, in order, from 'a' through 'z'.
//
//  RETURN VALUE
//
//      Upon successful completion, mkstemp() shall return an open file descriptor.
//      Otherwise, -1 shall be returned if no suitable file could be created.
//
//  ERRORS
//      No errors are defined.
*/

int
hunspell_mkstemp(char *templ)
{
        int fd = -1, ret;

        ret = tmpdir(templ, x_templ, sizeof(x_templ));
        if (ret) {
                if (ret > 0) {
                        if (MKSUCCESS == gettempfile(x_templ, &fd)) {
                                return fd;
                        }
                }
                return -1;
        }
        return (MKSUCCESS == gettempfile(templ, &fd) ? fd : -1);
}


/*
//  NAME
//      mkdtemp - create a unique temporary directory.
//
//  SYNOPSIS
//      #include <stdlib.h>
//
//      char *mkdtemp(char *template);
//
//  DESCRIPTION
//      The mkdtemp() function shall create a directory with a unique name derived from
//      template. The application shall ensure that the string provided in template is a
//      pathname ending with at least six trailing 'X' characters. The mkdtemp() function
//      shall modify the contents of template by replacing six or more 'X' characters at
//      the end of the pathname with the same number of characters from the portable
//      filename character set. The characters shall be chosen such that the resulting
//      pathname does not duplicate the name of an existing file at the time of the call
//      to mkdtemp(). The mkdtemp() function shall use the resulting pathname to create
//      the new directory as if by a call to:
//
//          mkdir(pathname, S_IRWXU)
//
//      The mkstemp() function shall create a regular file with a unique name derived from
//      template and return a file descriptor for the file open for reading and writing.
//      The application shall ensure that the string provided in template is a pathname
//      ending with at least six trailing 'X' characters. The mkstemp() function shall
//      modify the contents of template by replacing six or more 'X' characters at the
//      end of the pathname with the same number of characters from the portable filename
//      character set. The characters shall be chosen such that the resulting pathname
//      does not duplicate the name of an existing file at the time of the call to mkstemp().
//      The mkstemp() function shall use the resulting pathname to create the file, and
//      obtain a file descriptor for it, as if by a call to:
//
//          open(pathname, O_RDWR|O_CREAT|O_EXCL, S_IRUSR|S_IWUSR)
//
//      By behaving as if the O_EXCL flag for open() is set, the function prevents any possible
//      race condition between testing whether the file exists and opening it for use.
//
//  RETURN VALUE
//      Upon successful completion, the mkdtemp() function shall return the value of template.
//      Otherwise, it shall return a null pointer and shall set errno to indicate the error.
*/

char *
hunspell_mkdtemp(char *templ)
{
        int ret;

        ret = tmpdir(templ, x_templ, sizeof(x_templ));
        if (ret) {
                if (ret > 0) {
                        if (MKSUCCESS == gettempdir(x_templ)) {
                                return x_templ;
                        }
                }
                return NULL;
        }
        return (MKSUCCESS == gettempdir(templ) ? templ : NULL);
}


static int
tmpdir(const char *templ, char *t_templ, unsigned tmpsiz)
{
        /*
         *  "/tmp/", reference system temporary path
         */
        if (templ && 0 == memcmp(templ, "/tmp/", 5)) {
                unsigned pathlen = strlen(templ + 5),
                        tmplen = (int)GetTempPathA(tmpsiz, t_templ);
                                // TMP, TEMP, USERPROFILE environment variables, default windows directory.

                if (pathlen && tmplen) {
                        if ((pathlen + tmplen) >= tmpsiz) {
                                errno = ENAMETOOLONG;
                                return -1;

                        } else {
                                char *p;

                                if (t_templ[tmplen - 1] != '\\') {
                                        t_templ[tmplen - 1] = '\\', ++tmplen;
                                }
                                memcpy(t_templ + tmplen, templ + 5, pathlen + 1 /*nul*/);
                                for (p = t_templ; NULL != (p = strchr(p, '/'));) {
                                        *p++ = '\\'; /* convert */
                                }
                                return 1;
                        }
                }
        }
        return 0;
}


static unsigned
generate_seed(void)
{
        static unsigned seed;
        if (0 == seed) seed = (_getpid() * GetTickCount());
        seed = (1103515245 * seed + 12345);
        return seed;
}


static int
gettempfile(char *path, int *fd)
{
        char *start, *trv, *end, c;
        unsigned seed;
        int rc;

        for (trv = path; *trv; ++trv);
        if ((trv - path) >= MAX_PATH) {
                errno = ENAMETOOLONG;
                return MKERROR;
        }

        end = trv;
        if ((end - path) <= 6 || strcmp(end - 6, "XXXXXX")) {
                /* too short or missing 'XXXXXX' */
                errno = EINVAL;
                return MKERROR;
        }

        seed = generate_seed();
        while (--trv >= path && *trv == 'X') {
                *trv = (char)((seed % 10) + '0');
                seed /= 10;                     /* extra X's get set to 0's */
        }

        if ((trv + 1) == end) {                 /* missing template? */
                errno = EINVAL;
                return MKERROR;
        }

        /*
         *  check the target directory; if you have six X's and it
         *  doesn't exist this runs for a *very* long time.
         */
        for (start = trv + 1;; --trv) {
                if (trv <= path) {
                        break;
                }

                if ((c = *trv) == '/' || c == '\\') {
                    struct _stat sb = {0};

                    if (trv[-1] == ':') {
                            break;
                    }
                    *trv = '\0';
                    rc = _stat(path, &sb);      /* exists? */
                    *trv = c;
                    if (rc) {
                            return MKERROR;
                    }
                    if (!(sb.st_mode & S_IFDIR)) {
                            errno = ENOTDIR;
                            return MKERROR;
                    }
                    break;
                }
        }

        for (;;) {
                errno = 0;

#define O_MODE  (O_CREAT|O_EXCL|O_RDWR|O_BINARY)

                if ((*fd = _open(path, O_MODE, 0600)) >= 0) {
                        return MKSUCCESS;
                }
                if (EEXIST != errno) {
                        return MKERROR;
                }

                /* next is sequence */
                for (trv = start;;) {
                        if (trv == end) {       /* EOS */
                                return MKERROR;
                        }

                        if ('z' == *trv) {      /* 0..9a..z */
                                *trv++ = 'a';
                        } else {
                                if (isdigit(*trv)) {
                                        *trv = 'a';
                                } else {
                                        ++*trv;
                                }
                                break;
                        }
                }
        }
        /*NOTREACHED*/
}


static int
gettempdir(char *path)
{
        char *start, *trv, *end, c;
        unsigned seed;
        int rc;

        for (trv = path; *trv; ++trv);
        if ((trv - path) >= MAX_PATH) {
                errno = ENAMETOOLONG;
                return MKERROR;
        }

        end = trv;
        if ((end - path) <= 6 || strcmp(end - 6, "XXXXXX")) {
                /* too short or missing 'XXXXXX' */
                errno = EINVAL;
                return MKERROR;
        }

        seed = generate_seed();
        while (--trv >= path && *trv == 'X') {
                *trv = (char)((seed % 10) + '0');
                seed /= 10;                     /* extra X's get set to 0's */
        }

        if ((trv + 1) == end) {                 /* missing template? */
                errno = EINVAL;
                return MKERROR;
        }

        /*
         *  check the target directory; if you have six X's and it
         *  doesn't exist this runs for a *very* long time.
         */
        for (start = trv + 1;; --trv) {
                if (trv <= path) {
                        break;
                }

                if ((c = *trv) == '/' || c == '\\') {
                        struct _stat sb = {0};

                        if (trv[-1] == ':') {
                                break;
                        }
                        *trv = '\0';
                        rc = _stat(path, &sb);  /* exists? */
                        *trv = c;
                        if (rc) {
                                return MKERROR;
                        }
                        if (!(sb.st_mode & S_IFDIR)) {
                                errno = ENOTDIR;
                                return MKERROR;
                        }
                        break;
                }
        }

        for (;;) {
                errno = 0;
                if (0 == _mkdir(path)) {
                        return MKSUCCESS;
                }
                if (EEXIST != errno) {
                        return MKERROR;
                }

                /* next is sequence */
                for (trv = start;;) {
                        if (trv == end) {       /* EOS */
                                return MKERROR;
                        }

                        if ('z' == *trv) {      /* 0..9a..z */
                                *trv++ = 'a';
                        } else {
                                if (isdigit(*trv)) {
                                        *trv = 'a';
                                } else {
                                        ++*trv;
                                }
                                break;
                        }
                }
        }
        /*NOTREACHED*/
}


#if defined(LOCAL_MAIN)
#define false   0
#define true    1

static int
test(char *templ, int expect_success)
{
        char *rv = hunspell_mkdtemp(templ);

        if (NULL == rv) {
                int errsv = errno;
                printf("%s: mkdtemp failed: %s\n", __FUNCTION__, strerror(errsv));
                return (false == expect_success);
        } else {
                printf("%s: mkdtemp created: %s\n", __FUNCTION__, rv);
                if (0 != _rmdir(rv)) {
                        int errsv = errno;
                        printf("%s: rmdir failed: %s\n", __FUNCTION__, strerror(errsv));
                }
                return (true == expect_success);
        }
}


static void
test_mkdtemp()
{

        int success = 0, failure = 0;

        // Normal: should pass
        {       char templ1[] = "/tmp/asdf.XXXXXX";
                if (test(templ1, true)) {
                        printf("*** Test case 1: PASS\n");
                        ++success;
                } else {
                        printf("*** Test case 1: FAIL\n");
                        ++failure;
                }
        }

        // Too short: should fail
        {       char templ2[] = "XXXXXX";
                if (test(templ2, false)) {
                        printf("*** Test case 2: PASS\n");
                        ++success;
                } else {
                        printf("*** Test case 2: FAIL\n");
                        ++failure;
                }
        }

        // Not enough replacement Xs: should fail
        {       char templ3[] = "/tmp/asdf.XXXXX";
                if (test(templ3, false)) {
                        printf("*** Test case 3: PASS\n");
                        ++success;
                } else {
                        printf("*** Test case 3: FAIL\n");
                        ++failure;
                }
        }

        // Make sure it only replaces the end: should pass
        {       char templ4[] = "/tmp/asdfXXXXXX.XXXXXX";
                if (test(templ4, true)) {
                        printf("*** Test case 4: PASS\n");
                        ++success;
                } else {
                        printf("*** Test case 4: FAIL\n");
                        ++failure;
                }
        }

        // Really long: should fail
        {       char templ5[] = 
                        "/tmp/asdfaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
                        "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
                        "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
                        "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
                        "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
                        "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
                        "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
                        "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaXXXXXX";
                if (test(templ5, false)) {
                        printf("*** Test case 5: PASS\n");
                        ++success;
                } else {
                        printf("*** Test case 5: FAIL\n");
                        ++failure;
                }
        }

        // Unwriteable path: should fail
        {       char templ6[] = "/asdfkjavblkjadv/asdf.XXXX";
                if (test(templ6, false)) {
                        printf("*** Test case 6: PASS\n");
                        ++success;
                } else {
                        printf("*** Test case 6: FAIL\n");
                        ++failure;
                }
        }

        printf("TEST SUMMARY: Success=%d, Failure=%d\n", success, failure);
        return failure;
}

int
main()
{
        test_mkdtemp();
}
#endif

/*end*/

