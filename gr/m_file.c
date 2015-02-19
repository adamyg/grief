#include <edidentifier.h>
__CIDENT_RCSID(gr_m_file_c,"$Id: m_file.c,v 1.38 2015/02/17 23:26:17 ayoung Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: m_file.c,v 1.38 2015/02/17 23:26:17 ayoung Exp $
 * File primitives.
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
#include <edenv.h>
#include <libstr.h>                             /* str_...()/sxprintf() */

#include <errno.h>
#include "../libvfs/vfs.h"

#include "m_file.h"

#include "accum.h"
#include "builtin.h"
#include "debug.h"
#include "echo.h"
#include "eval.h"
#include "file.h"
#include "lisp.h"
#include "main.h"
#include "symbol.h"
#include "sysinfo.h"
#include "system.h"
#include "wild.h"
#include "word.h"

#ifndef IOBUF_SIZ                               /* local i/o buffer */
#define IOBUF_SIZ       (BUFSIZ * 16)
#endif

#ifndef TEST_SIZE
#define TEST_SIZE       (4 * 1024)              /* see line.c */
#endif

#define ISSEP(c)        ('\\' == (c) || '/' == (c))

static void             file_match(int parg, int farg, int flags);

static int              x_open(const char *path, int, int);
static int              x_stat(const char *path, struct stat *stat_buf);
static int              x_access(const char *path, int what);
static int              is_binary(const char *path);

static void             stat_assign(struct stat *sb);

static vfs_dir_t *      x_dirp = NULL;          /* used by find_file & file_pattern */

static char             x_match_file[DIRSIZ+1]; /* pattern to match. */

static char             x_match_dir[MAX_PATH];  /* and directory name for above. */


/*  Function:           do_compare_files
 *      compare_file primitive.
 *
 *<<GRIEF>>
    Macro: compare_files - Binary file compare.

        int
        compare_files([int flags], string file1, string file2)

    Macro Description:
        The 'compare_files' performs simple comparison byte-for-byte
        of the on disk images of the two file 'file1' and 'file2'.

        The 'flags' are intended for future options, supporting
        simple line processing.

        It can be used to quickly determine if two files are
        identical (byte for byte). Note that if either file is an
        active buffer, no unsaved changes shall be included.

        In contrast, the <diff_buffers> primitive is used to compare
        files and mark up the differences.

    Macro Returns:
        Returns 1 if the files are identical, 0 if they differ. -1 if
        the first file cannot be read, -2 if the second file cannot
        be read.

    Macro Portability:
        A Grief extension.

    Macro See Also:
        diff_buffers
*/
void
do_compare_files(void)          /* ([int flags = 0], string filea, string fileb) */
{
//  const int flags = get_xinteger(1, 0);       //TODO
    const char *file1 = get_str(2);
    const char *file2 = get_str(3);
    struct stat sb1, sb2;
    int ret = 0;

    if (x_stat(file1, &sb1) < 0) {
        ret = -1;
    } else if (x_stat(file2, &sb2) < 0) {
        ret = -2;
    } else if (sb1.st_size != sb2.st_size) {
        ret = 0;
    } else {
        int fd1, fd2 = -1;

        if ((fd1 = x_open(file1, OPEN_R_BINARY | O_RDONLY, 0660)) < 0) {
            ret = -1;

        } else if ((fd2 = x_open(file2, OPEN_R_BINARY | O_RDONLY, 0660)) < 0) {
            ret = -2;

        } else {
            char b1[IOBUF_SIZ], b2[IOBUF_SIZ];
            int i, j;

            for (;;) {
                if ((i = vfs_read(fd1, b1, sizeof(b1))) < 0) {
                    ret = -1;                   /* i/o error */
                    break;
                }

                if ((j = vfs_read(fd2, b2, sizeof(b2))) < 0) {
                    ret = -2;                   /* i/o error */
                    break;
                }

                if (i != j) {
                    ret = (i < j ? -1 : -2);
                    break;                      /* short read */
                }

                if (i == 0) {
                    ret = 1;                    /* identical */
                    break;
                }

                if (memcmp(b1, b2, i) != 0) {
                    break;
                }
            }
        }

        if (fd1 >= 0) {
            vfs_close(fd1);
            if (fd2 >= 0) {
                vfs_close(fd2);
            }
        }
    }
    acc_assign_int(ret);
}


/*  Function:           do_expandpath
 *      expandpath primitive.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 *
 *<<GRIEF>>
    Macro: expandpath - Expand the path.

        string
        expandpath(string path, [int env])

    Macro Description:
        The 'expandpath()' primitive expands any shell style home
        directory ~[user] references/short-hands contained within the
        specified 'path', returning the result.

        The supported shell constructs are.

            ~/ -        is expanded to your current home directory.

            ~user/ -    is expanded to the specified 'users'
                        home directory (unix only).

        If 'env' is specified and is non-zero, then the any embedded
        environment variable references are also expanded following the
        following syntax. When a macro is not defined it expands to \"\"
        (an empty string), and {} are synonyms for ().

            $(name) -   Expands to value of 'name'.

            ${name} -   Expands to value of 'name'.

            $name/ -    Expands to value of 'name'.

    Macro Returns:
        The 'expandpath()' primitive returns a string containing the
        expanded path following the above rules, otherwise an empty
        string.

    Macro Portability:
        A Grief extension.

    Macro See Also:
        strfilecmp, file_canon, file_glob, getenv, searchpath
 */
void
do_expandpath(void)             /* (string path, [int env = FALSE]) */
{
    char path[MAX_PATH];

    if (NULL == file_tilder(get_str(1), path, sizeof(path))) {
        path[0] = '\0';
    }
    if (get_xinteger(2, FALSE)) {               /* TRUE or FALSE */
        file_getenv(path, sizeof(path));
    }
    acc_assign_str(path, -1);
}


/*  Function:           do_searchpath
 *      searchpath primitive.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 *
 *<<GRIEF>>
    Macro: searchpath - Searches for a given file in a specified path.

        int
        searchpath([string path], string filename,
            [string extension], string &result, [int mode], [int expand = FALSE])

    Macro Description:
        The 'searchpath()' primitives searches the directory path
        'path' for a instance of the file name 'file'. 'path' is a
        list of delimiter separated directory names, with the same
        syntax as the shell variable 'GRPATH'.

    Macro Parameters:
        path - Optional string containing the path to be searched for
            the file. If omitted the system 'PATH' specification
            shall be referenced.

        filename - String containing the name of the file for which
            to search.

        extension - Optional string containing the file extension to
            be added to the file name when searching for the file.
            The first character of the file name extension must be a
            period (.). The extension is added only if the specified
            file name does not end with an extension. If a file name
            extension is not required or if the file name contains an
            extension, this parameter can be NULL.

        result - String variable reference to be populated with the
            first instance of the file resolved along the given
            search path.

    Macro Returns:
        The 'expandsearch()' primitive returns the length of the
        string that is copied to the buffer 'result'; otherwise on
        failure returns a value of zero.

    Macro Portability:
        A Grief extension.

    Macro See Also:
        expandpath
 */
void
do_searchpath(void)             /* int (string searchpath, string file, [string extension], string &result,
                                            [int expand = FALSE], [int mode = NULL]) */
{
    const char *searchpath = get_xstr(1);
    const char *filename = get_xstr(2),
            *extension = get_xstr(3);
    const int expand = get_xinteger(5, FALSE);
//  const int mode = get_xinteger(6, -1);       /* TODO */
    int ret = -1;

    if (NULL == searchpath) {
        searchpath = ggetenv("GRPATH");
    }

    if (searchpath && filename && *filename) {

        const char delimiter[2] = {FILEIO_DIRDELIM, 0},
                separator[2] = {FILEIO_PATHSEP, 0};
        char path[MAX_PATH], abspath[MAX_PATH];
        char *cp, *buf = chk_salloc(searchpath);

        if (extension) {
            if ('.' != *extension || strrchr(filename, '.')) {
                extension = NULL;               /* invalid/not required */
            }
        }

        for (cp = strtok(buf, delimiter);       /* system delimiter */
                    cp != NULL ; cp = strtok(NULL, delimiter)) {

            if (NULL != file_expand(cp, path, sizeof(path))) {
                struct stat sb = {0};

                strxcat(path, separator, sizeof(path));
                strxcat(path, filename, sizeof(path));
                if (extension) {                /* optional extension */
                    strxcat(path, extension, sizeof(path));
                }

                if (0 == stat(path, &sb)) {
                    const char *result = path;

                    if (expand) {
                        if (0 == sys_realpath((const char *)result, abspath, sizeof(abspath)) && abspath[0]) {
                            result = abspath;
                        }
                    }
                    ret = strlen(result);
                    argv_assign_str(4, result);
                }
            }
        }
        chk_free(buf);
    }
    acc_assign_int(ret);
}


/*  Function:           do_file_glob
 *      file_glob primitive, returning a list of matching filenames.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 *
 *<<GRIEF>>
    Macro: file_glob - Return names of files that match patterns.

        list
        file_glob(string files)

    Macro Description:
        The 'file_glob()' primitive performs file name 'globbing' in a
        fashion similar to the 'csh' shell. It returns a list of the
        files whose names match any of the pattern arguments.

        No particular order is guaranteed in the list, so if a sorted
        list is required the caller should use <sort_list>.

        The pattern arguments may contain any of the following
        special characters:

            ~[user/] -  Home directory of either the current or the
                        specified user.

            ? -         Matches any single character.

            * -         Matches any sequence of zero or more characters.

            [ch] -      Matches any single character in chars. If ch's
                        contains a sequence of the form 'a-b' then any
                        character between 'a' and 'b' (inclusive) will
                        match.

            \x -        Matches the character x.

    Macro Returns:
        The 'file_glob()' primitive returns a list of strings
        corresponding to the filenames which match the wild card
        expression in string. When no matches or error, an empty list
        is returned.

    Macro See Also:
        glob, file_canon, file_pattern, find_file, expandpath
 */
void
do_file_glob(void)              /* list ([string files]) */
{
    char **files;
    LIST *newlp, *lp;
    int atoms = 0;
    int llen;
    int i;

    if (NULL != (files = shell_expand(get_str(1)))) {
        for (i = 0; files[i]; ++i)
            if (files[i][0]) {
                ++atoms;
            }
    }

    llen = (atoms * sizeof_atoms[F_RSTR]) + sizeof_atoms[F_HALT];
    if (0 == atoms || NULL == (newlp = lst_alloc(llen, atoms))) {
        acc_assign_null();
        return;
    }

    lp = newlp;
    for (i = 0; files && files[i]; ++i) {
        if (files[i][0]) {
            lp = atom_push_str(lp, files[i]);
        }
    }
    atom_push_halt(lp);

    acc_donate_list(newlp, llen);
    shell_release(files);
}


/*  Function:           do_file_canon
 *      file_canon primitive
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 *
 *<<GRIEF>>
    Macro: file_canon - Canonicalize a path.

       string
       file_canon(string filepath)

    Macro Description:
        The 'file_canon()' primitive performs canonicalizes the
        specified path 'filepath' and returns a new path.

        The new path differs from path in:

        o Slashes are normalised with '\' replaced with '/'.

        o Relative paths are prefixed with the current
            working directory.

        o Multiple `/'s are collapsed to a single `/'.

        o Leading `./'s and trailing `/.'s are removed.

        o Trailing `/'s are removed.

        o Non-leading `../'s and trailing `..'s are handled by
            removing portions of the path.

    Macro Returns:
        The 'file_canon()' primitive returns the canonicalized file
        path.

    Macro Portability:
        A Grief extension.

    Macro See Also:
        glob, file_canon, file_pattern, find_file, expandpath
 */
void
do_file_canon(void)             /* string (string filespec) */
{
    const char *filespec = get_str(1);
    char canonicalize[MAX_PATH];

    if (filespec) {
        filespec = file_canonicalize(filespec, canonicalize, sizeof(canonicalize));
    }
    acc_assign_str(filespec ? filespec : "", -1);
}


/*  Function:           do_file_match
 *      file_match primitive, which performs wild-card name matching
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 *
 *<<GRIEF>>
    Macro: file_match - File match utility.

        int
        file_match(pattern, file, [flags])

    Macro Description:
        The 'file_match()' primitive performs wild-card name matching,
        which has two major uses. It could be used by an application
        or utility that needs to read a directory and apply a pattern
        against each entry.

        The 'find_file()' primitive is an example of this. It can
        also be used by the ts macro to process its pattern operands,
        or by applications that need to match strings in a similar
        manner.

        The 'file_match()' primitive is intended to imply filename
        match, rather than pathname match. The default action of this
        primitive is to match filenames, rather than path names,
        since it gives no special significance to the slash character.

        'pattern' can be a list of search expressions or a string
        containing a single expression. 'file' is the file-name that
        shall be tested.

        If supplied, 'flags' allows control over how directory
        delimiters slash(/) and dots (.) are matched within
        file-name. Otherwise it defaults to setting the MATCH_PERIODA
        and MATCH_NOCASE based on the current operating environment
        similar to ones used during edit_file() pattern matching.

    Pattern:

        The following patterns matching a single character match a
        single character: ordinary characters, special pattern
        characters and pattern bracket expressions. The pattern
        bracket expression will also match a single collating element.

        An ordinary character is a pattern that matches itself. It
        can be any character in the supported character set except
        for NUL, those special shell characters that require quoting,
        and the following three special pattern characters.

        When unquoted and outside a bracket expression, the following
        three characters will have special meaning in the
        specification of patterns:

            ? - A question-mark is a pattern that will match any
                character.

            * - An asterisk is a pattern that will match multiple
                characters, as described in Patterns Matching Multiple
                Characters, see below.

            [ - The open bracket will introduce a pattern bracket
                expression, see below.

    Patterns Matching Multiple Characters:

        The following rules are used to construct patterns matching
        multiple characters from patterns matching a single
        character:

            o The asterisk (*) is a pattern that will match any
            string, including the null string.

            o The concatenation of patterns matching a single
            character is a valid pattern that will match the
            concatenation of the single characters or collating
            elements matched by each of the concatenated patterns.

            o The concatenation of one or more patterns matching a
            single character with one or more asterisks is a valid
            pattern. In such patterns, each asterisk will match a
            string of zero or more characters, matching the greatest
            possible number of characters that still allows the
            remainder of the pattern to match the string.

    Character Set Range Match:

        '[' introduces a pattern bracket expression, that will
        matches a single collating element contained in the
        non-empty set of collating elements. The following rules
        apply:

            o A bracket expression is either a matching list
            expression or a non-matching list expression. It consists
            of one or more expressions.

            o A matching list expression specifies a list that
            matches any one of the expressions represented in the
            list. The first character in the list must not be the
            circumflex (^). For example, [abc] is a pattern that
            matches any of the characters 'a', 'b' or 'c'.

            o A non-matching list expression begins with a circumflex
            (^), and specifies a list that matches any character or
            collating element except for the expressions represented
            in the list after the leading circumflex. The circumflex
            will have this special meaning only when it occurs first
            in the list, immediately following the left-bracket.

            o A range expression represents the set of collating
            elements that fall between two elements in the current
            collation sequence, inclusively. It is expressed as the
            starting point and the ending point separated by a hyphen
            (-). For example, [a-z] is a pattern that matches any of
            the characters a to z inclusive.

            o A bracket expression followed by '+' means one or more
            times.

    Character Classes:
        Within bracket expressions, the name of a character class
        enclosed in [: and :] stands for the list of all characters
        belonging to that class.

        Standard (POSIX style) character classes are;

            alnum -  An alphanumeric (letter or digit).

            alpha -  A letter.

            blank -  A space or tab character.

            cntrl -  A control character.

            csym  -  An alphanumeric (letter or digit) or or
                     underscore character.

            digit -  A decimal digit.

            graph -  A character with a visible representation.

            lower -  A lower-case letter.

            print -  An alphanumeric (same as alnum).

            punct -  A punctuation character.

            space -  A character producing white space in
                     displayed text, space or a tab.

            upper -  An upper-case letter.

            xdigit - A hexadecimal digit.

    Flags:

        If supplied the flags argument shall modify the interpretation
        of pattern and string. It is the bitwise-inclusive OR of
        zero or more of the flags defined in 'grief.h'.

        MATCH_PATHNAME -
            If the MATCH_PATHNAME flag is set in flags, then a slash
            character ('/') in string shall be explicitly matched by
            a slash in pattern; it shall not be matched by either the
            asterisk ('*') or question-mark ('?') special characters,
            nor by a bracket expression ('[]'). If the MATCH_PATHNAME
            flag is not set, the slash character shall be treated as
            an ordinary character.

        MATCH_NOCASE -
            If the MATCH_NOCASE flag is set in flags, then the
            pattern is treated non-case sensitively, i.e. A matches
            a. Otherwise the search is performed with case being
            sensitive.

        MATCH_NOESCAPE -
            If MATCH_NOESCAPE is not set in flags, a backslash
            character ('\\') in pattern followed by any other
            character shall match that second character in string. In
            particular, "\\" shall match a backslash in string. If
            MATCH_NOESCAPE is set, a backslash character shall be
            treated as an ordinary character.

        MATCH_PERIOD -
            If MATCH_PERIOD is set, a leading period in string will
            match a period in pattern; where the location of
            "leading" is indicated by the value of MATCH_PATHNAME as
            follows:

            If not set, no special restrictions are placed on
            matching a period.

                o is set, a period is "leading" if it is the first
                character in string or if it immediately follows a
                slash.

                o is not set, a period is "leading" only if it is the
                first character of string.

        MATCH_PERIODA, MATCH_PERIODQ and MATCH_PERIODB -
            If set these allow selective control whether the asterisk
            (*) or question mark (?) special characters, nor by a
            bracket ([]) expression have affect.

    Examples:

>       a[bc]

            matches the strings ab and ac.

>       a*d

            matches the strings ad, abd and abcd, but not the string abc.

>       a*d*

            matches the strings ad, abcd, abcdef, aaaad and adddd.

>       *a*d

            matches the strings ad, abcd, efabcd, aaaad and adddd.

    Note:
        file_match() does not perform tilde expansion (see expandpath).

    Macro Returns:
        The 'file_match()' primitive returns 1 if file matches the
        specified pattern; returns 0 if pattern isnt matched. If
        pattern is a list, then the index into the list that matched
        the expression or -1.

    Macro Portability:
        A Grief extension.

    Macro See Also:
        file_pattern, filename_match
 */
void
do_file_match(void)             /* int (string pattern, string file, int flags = -1) */
{
    int flags = -1;

    if (isa_integer(3)) {
        flags = get_xinteger(3, 0) & 0xffff;    /* user specific flags */
    }
    file_match(1, 2, flags);
}


/*  Function:           do_filename_match
 *      filename_match primitive, which performs wild-card name matching
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 *
 *<<GRIEF>>
    Macro: filename_match - Perform file pattern matching.

        int
        filename_match(string file, declare pattern)

    Macro Description:
        The 'filename_match()' primitive is similar to the file_match()
        primitive but is provided for CRiSPEdit compatibility. It is used
        to compare a filename to see if it matches a filename regular
        expression.

        A filename expression is a regular expression similar to that
        accepted by the command line shells on Unix systems (e.g. you
        can use '*' for wildcard, '?' for wild-character, and [..] to
        select a range of characters).

        'file' is the filename that shall be tested. 'pattern' can be a
        list of search expressions or a string containing a single
        expression (see file_match) for details on the expression syntax.

    Macro Returns:
        The 'filname_match()' primitive return value is dependent on
        the pattern type. If pattern is a string, then 1 if the
        filename matches; 0 otherwise. If pattern is a list, then the
        index into the list that matched the expression or -1.

    Macro Portability:
        Functionality has not been formally verified against CRiSPEdit
        and whether it supports the [..]+ expression construct.

    Macro See Also:
        file_match, strfilecmp, file_glob
 */
void
do_filename_match(void)         /* int (string file, declare pattern) */
{
    file_match(2, 1, 0);
}


static void
file_match(int parg, int farg, int flags)
{
    const char *pattern = get_xstr(parg);       /* string || list */
    const char *file = get_str(farg);
    int ret;

    if (NULL != pattern) {
        /*
         *  pattern
         */
        if (flags >= 0) {                       /* user specific flags */
            ret = wild_match(file, pattern, flags);
        } else {                                /* standard flags */
            ret = wild_file(file, pattern);
        }

    } else {
        /*
         *  list of patterns
         */
        const LIST *nextlp, *lp = get_xlist(parg);

        ret = -1;
        if (lp) {
            int idx;

            for (idx = 0; -1 == ret && (nextlp = atom_next(lp)) != lp; lp = nextlp, ++idx) {
                if (NULL != (pattern = atom_xstr(lp))) {
                    if (flags >= 0) {           /* user specific flags */
                        if (wild_match(file, pattern, flags)) {
                            ret = idx;
                        }
                    } else {                    /* standard flags */
                        if (wild_file(file, pattern)) {
                            ret = idx;
                        }
                    }
                }
            }
        }
    }

    acc_assign_int(ret);
}


/*  Function:           do_glob
 *      glob primitive, which performs command line globbing.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 *
 *<<GRIEF>>
    Macro: glob - Generate pathnames matching a pattern

        string
        glob(string pattern)

    Macro Description:
        The 'glob()' primitive expands the specified 'pattern' into
        single string similar to that which occurs on the shell
        command line.

            *  -    Match any string of characters.

            [] -    Character class.

            ?  -    Match any single character.

            ~  -    User name home directory.

            \x -    Quote the next metacharacter 'x'.

    Macro Returns:
        The 'glob()' primitive returns a string containing the result
        of the pattern expansion on success, otherwise an empty string.

    Macro Portability:
        A Grief extension.

    Macro See Also:
        file_glob, file_pattern, find_file, expandpath
 */
void
do_glob(void)                   /* string (string pattern) */
{
    char path[MAX_PATH];
    char *out;

    if (NULL == file_tilder(get_str(1), path, sizeof(path))) {
        path[0] = '\0';
    }
    out = wild_glob(path);                      /* FIXME, use bsd_glob() */
    acc_assign_str((const char *)(out ? out : NULL), -1);
    wild_globfree(out);
}


/*  Function:           do_getwd
 *      getwd primitive.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 *
 *<<GRIEF>>
    Macro: getwd - Get current working directory.

   int
        getwd(int ignored, string dir)

    Macro Description:
        The 'getwd()' primitive primitive retrieves the name of the
        current working directory.

   The 'ignored' parameter exists for compatibility with BRIEF.

    Macro Returns:
        The 'getwd()' primitive returns 1 if successful otherwise
        zero on error. When an error has occurred, the global
        <errno> contains a value indicating the type of error that
        has been detected.

    Macro See Also:
        cd, mkdir, rmdir
 */
void
do_getwd(void)                  /* int (int drive-ignored, string dir) */
{
    argv_assign_str(2, file_cwd(NULL, 0));
    acc_assign_int(1);
}


/*  Function:           do_cd
 *      cd primitive.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 *
 *<<GRIEF>>
    Macro: cd - Change directory.

        int
        cd([string dir])

    Macro Description:
        The 'cd()' primitive changes the current directory on the
        specified drive to the specified path. Unlike <chdir> shell
        expansion shall occur when the path contains special
        characters, expanding home references '~', wild cards and
        environment variables.

>      cd(newdir);

   Under DOS/WIN32, if no drive is specified in path then the
   current drive is assumed. The path can be either relative to the
   current directory on the specified drive or it can be an
   absolute path name

   If 'dir' is omitted, the current directory shall be echoed on
   the command prompt without effecting any change.

>      cd();

    Macro Returns:
        The 'cd()' primitive returns 1 if successful otherwise zero
        on error. When an error has occurred, the global <errno>
        contains a value indicating the type of error that has been
        detected.

    Macro See Also:
        mkdir, rmdir
 */
void
do_cd(void)                     /* int ([string dir]) */
{
    const char *cp;
    int j, ret = 0;
    char **files;
    char *mem = NULL;

    /* NULL argument, echo current working directory */
    if (isa_undef(1)) {
        ewprintf("%s", file_cwd(NULL, 0));
        acc_assign_int(1);                      /* success */
        return;
    }

    /* Perform wildcard and variable substitution on directory name. */
    cp = get_str(1);

    if (strpbrk(cp, "~*?[$")) {
        if ((files = shell_expand(cp)) != NULL) {
            cp = NULL;

            for (j = 0; files[j]; j++) {
                if (cp == NULL && files[j][0]) {
                    cp = mem = files[j];        /* take first match */
                } else {
                    chk_free(files[j]);
                }
            }
            chk_free(files);
        }
    }

    if (*cp) {                                  /* change directory */
        ret = file_chdir(cp);
    } else {
        errno = EINVAL;
        ret = -1;
    }

#if defined(DOSISH)             /* / and \ */
    if (ret && *cp) {
        char buf[MAX_PATH], *p;
        int trailing = 0;

        strxcpy(buf, cp, sizeof(buf));
        p = buf + strlen(buf) - 1;
        while (p > buf + 1 && (*p == '/' || *p == '\\')) {
            *p-- = '\0';                        /* remove trailing slashes */
            ++trailing;
        }
        if (trailing) ret = file_chdir(buf);
    }
#endif

    system_call(ret);
    acc_assign_int(ret ? 0 : 1);                /* success(1) otherwise (0) */
    if (mem) {
        chk_free(mem);
    }
}


/*  Function:           do_find_file
 *      find_file primitive.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 *
 *<<GRIEF>>
    Macro: find_file - Read next directory entry.

        int
        find_file([string &filename], [int &size],
                [int &mtime], [int &ctime], [int &mode])

    Macro Description:
        The 'find_file()' primitive retrieves the next file which matches
        the current file pattern. The active source directory and
        pattern are controlled using <file_pattern>.

    Example::

       The following example retrieves are .txt files within the current
       working directory.

>           string name;
>           int size;
>
>           file_pattern("*.txt");
>           while (file_find(name)) {
>               parsename(name, size);
>           }

        This primitive is used in conjunction with <file_pattern>.

    Macro Parameters:
        filename - Optional string, is populated with the base filename;
            without any path.

        size - Optional integer, if specified is populated with the size
            of the related file in bytes.

                o For regular files, the file size in bytes.
                o For symbolic links, the length in bytes of the
                    path-name contained in the symbolic link.
                o For a shared memory object, the length in bytes.
                o For a typed memory object, the length in bytes.
                o For other file types, the use of this field is
                    unspecified.

        mtime - Optional integer, populated with the files modification
            time (see time).

        ctime - Optional integer, populated with the time of the last
            status change.

        mode - Mode of file (see stat).

    Macro Returns:
        Returns zero if there are no more files; returns 1 if next
        directory entry successfully received.

    Macro Portability:
        The 'mtime', 'ctime' and 'mode' parameters are Grief extensions.

    Macro See Also:
        file_pattern, find_file2, file_glob, expandpath, stat,
        mode_string.

 *<<GRIEF>>
    Macro: find_file2 - Extended read next directory entry.

        int
        find_file2(string filename, [int &size],
                [int &mtime], [int &ctime], [int &atime], [int &mode],
                [int &uid], [string &uid2name],
                [int &gid], [string &gid2name],
                    [int &nlink], [int &inode])

    Macro Description:
        The 'find_file2()' primitive is an extended version of
        <find_file> returning additional file information on which
        matching file.

    Macro Parameters:
        filename - Optional string, is populated with the base filename;
            without any path.

        size - Optional integer, if specified is populated with the size
            of the related file in bytes.

                o For regular files, the file size in bytes.
                o For symbolic links, the length in bytes of the
                    path-name contained in the symbolic link.
                o For a shared memory object, the length in bytes.
                o For a typed memory object, the length in bytes.
                o For other file types, the use of this field is
                    unspecified.

        mtime - Optional integer, populated with the files modification
            time (see time).

        ctime - Optional integer, populated with the time of the last
            status change.

        atime - Optional integer, populated with the last access time.

        mode - Optional integer, mode of file (see stat).

        uid - Optional integer, user identifier of the file.

        uid2name - User name associated with the file uid.

        gid - Optional integer, group identifier of the file.

        gid2name - Group name associated with the file gid.

        nlink - Optional integer, number of hard links to the file.

        inode - Optional integer, populated with the file-system
            internal node number.

    Macro Returns:
        Returns zero if there are no more files; returns 1 if next
        directory entry successfully received.

    Macro Portability:
        A Grief extension.

    Macro See Also:
        file_pattern, find_file, file_glob, expandpath, stat,
        mode_string.
*/
void
do_find_file(int mode)              /* ([string filename], ...) */
{
    vfs_dirent_t *de;
    struct stat sb;
    char buf[MAX_PATH];

    acc_assign_int(0);
    if (NULL == x_dirp)
        return;                                 /* no open directory */

    errno = 0;
    while ((de = vfs_readdir(x_dirp)) != (vfs_dirent_t *) NULL) {

        if (! wild_file(de->d_name, x_match_file)) {
            continue;
        }

        strxcpy(buf, x_match_dir, sizeof(buf));
        strxcat(buf, de->d_name, sizeof(buf));

        if (0 != vfs_statdir(de, &sb)) {        /* cached/dir level stat info */
#if defined(HAVE_LSTAT)
            if (-1 == vfs_lstat(buf, &sb)) {    /* link level */
#endif
                if (-1 == vfs_stat(buf, &sb)) { /* file level */
                    ewprintx("find_file: cannot stat '%s'", buf);
                    memset(&sb, 0, sizeof(sb));
                }
#if defined(HAVE_LSTAT)
            }
#endif
        }

        argv_assign_str(1, de->d_name);
        if (2 == mode) {
            stat_assign(&sb);
        } else {
            argv_assign_int(2, (accint_t) sb.st_size);
            argv_assign_int(3, (accint_t) sb.st_mtime);
            argv_assign_int(4, (accint_t) sb.st_ctime);
            argv_assign_int(5, (accint_t) sb.st_mode);
        }

        acc_assign_int(1);
        return;
    }
    system_call(-1);
}


/*  Function:           do_file_pattern
 *      file_pattern primitive.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 *
 *<<GRIEF>>
    Macro: file_pattern - Directory file pattern.

        int
        file_pattern(string filespec)

    Macro Description:
        The 'file_pattern()' primitive sets the search pattern for
        files, using <find_file>, and reset the internal status to
        the first matching file.

    Macro Parameters:
        filespec - String containing the file pattern specification
            which should evaluate to a file name or a wild-card
            filename, see the <file_match> for details regarding
            the supported wildcard.

    Macro Returns:
        The 'file_pattern()' primitive returns 0 on success,
        otherwise -1 on error. When an error has occurred, the
        global <errno> contains a value indicating the type of
        error that has been detected.

    Notes:
        This interface enforces the rule (MATCH_PERIODA) that a
        leading "*" wont match any files which begin with a "." as
        such use "?*" to match all files contained within a
        directory. See <file_match> for details on the 'filespec'
        syntax.

    Macro See Also:
        file_match, glob, expandpath
 */
void
do_file_pattern(void)           /* int (string pattern) */
{
    const char *pattern = get_str(1);
    char *cp, *fname;
#if defined(DOSISH)             /* / and \ */
    char *fname2;
#endif
    char buf[MAX_PATH];

    while (*pattern == ' ') {                   /* isolate directory name */
        ++pattern;
    }
    strxcpy(buf, pattern, sizeof(buf));
    cp = buf;
    strxcpy(x_match_file, cp, sizeof(x_match_file));
    fname = strrchr(cp, '/');
#if defined(DOSISH)             /* / and \ */
    if ((fname2 = strrchr(cp, '\\')) != NULL && fname2 > fname) {
        fname = fname2;
    }
#endif

    if (x_dirp) {                               /* release previous */
        vfs_closedir(x_dirp);
        x_dirp = NULL;
    }

    if (NULL == fname) {                        /* no directory, open current directory */
#if defined(DOSISH)             /* X: */
        if (*cp && ':' == cp[1] && isalpha((unsigned char)*cp)) {
            char dir[3];

            dir[0] = *cp;
            dir[1] = ':';
            dir[2] = '\0';
            x_dirp = vfs_opendir(dir);
            fname = cp + 2;
        } else
#endif
        {
            x_dirp = vfs_opendir(".");
            fname = cp;
        }
        strcpy(x_match_dir, "./");

    } else {                                    /* explicit directory */
        *fname++ = '\0';
        x_dirp = vfs_opendir(cp);
        strcpy(x_match_dir, cp);
        strcat(x_match_dir, "/");
    }

    acc_assign_int(x_dirp ? 0 : -1);
    if (NULL == x_dirp) {
        system_call(-1);
    }

    strxcpy(x_match_file, fname, sizeof(x_match_file));
}


/*  Function:           do_fstype
 *      fstype primitive.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 *
 *<<GRIEF>>
    Macro: fstype - File system type.

        int
        fstype([string path])

    Macro Description:
        The 'fstype()' primitive retrieves the underlying file-system
        type.

    Macro Parameters:
        path - Optional string containing path on the file-system to
            be tested, if omitted the current working directory is
            referenced.

    Macro Returns:
        The 'fstype()' primitive returns a system dependent file-system
        identifier, otherwise -1 on error or not supported.

    Macro Portability:
        A Grief extension.

    Macro See Also:
        stat
*/
void
do_fstype(void)                 /* int ([string path]) */
{
#if defined(WIN32) || defined(__OS2__)
    acc_assign_int(sys_fstype(get_xstr(1)));
#else
    acc_assign_int(-1);
#endif
}


/*  Function:           do_access
 *      access primitive.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 *
 *<<GRIEF>>
    Macro: access - Test file accessibility.

        int
        access(string path, int mode)

    Macro Description:
        The 'access()' primitive is used to check the accessibility
        of a file. The 'file' parameter is the name of a directory or
        file which is to be tested and 'mode' are a set of access
        bits which are used to check the effective access.

        When the value of 'mode' is zero, only the existence of
        the file is verified. The meaning of mode is operating
        system dependent yet most operating systems support the
        following definitions:

            F_OK - Check if file exists (0).
            X_OK - Check to see if file is executable (1).
            W_OK - Check if file is writable (2).
            X_OK - Check if file is readable (3).

    Macro Returns:
        On successful return this primitive returns >= 0; -1 is
        returned on an error, and the global variable <errno> is
        set to the reason for the failure.

    Macro Portability:
        A Grief extension.

    Macro See Also:
        exist, stat, lstat, ftest, chmod
*/
void
do_access(void)                 /* int (string file, int mode) */
{
    const char *path = get_str(1);
    int ret;

  //path = file_tilder(path, t_path, sizeof(t_path));
    ret = x_access(path, get_xinteger(2, 0));
    acc_assign_int(ret);
}


/*  Function:           do_mkdir
 *      mkdir primitive.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 *
 *<<GRIEF>>
    Macro: mkdir - Create a directory.

        int
        mkdir(string pathname, int mode = 0755)

    Macro Description:
        The 'mkdir()' primitive creates a new subdirectory with
        name path. The path can be either relative to the current
        working directory or it can be an absolute path name.

        'mode' is the protection codes used to create the
        directory. If omitted, the value *0755* ('-rwxr-xr-x')
        will be used.

    Macro Returns:
        The 'mkdir()' primitive returns zero if successful, and a
        non-zero value (-1) otherwise. When an error has occurred,
        the global <errno> contains a value indicating the type of
        error that has been detected.

    Macro See Also:
        rmdir, cd, getwd
 */
void
do_mkdir(void)                  /* (string pathname, int mode = 0755) */
{
    const char *pathname = get_str(1);
    const int mode = get_xinteger(2, 0755);
    int ret;

    ret = system_call(vfs_mkdir(pathname, mode));
#if defined(__CYGWIN__)
    if (ret && EEXIST != ret) {
        struct stat sb;

        if (0 == stat(pathname, &sb) && S_ISDIR(sb.st_mode)) {
            ret = 0;                            /* ignore, false error */
        }
    }
#endif
    acc_assign_int(ret);
}


/*  Function:           do_chdir
 *      chdir primitive.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 *
 *<<GRIEF>>
    Macro: chdir - Change directory.

        int
        chdir(string path)

    Macro Description:
        The 'chdir()' primitive changes the current directory on
        the specified drive to the specified path.

        Unlike <cd> no shell expansion shall occur.

    Macro Returns:
        The 'chdir()' primitive returns zero if successful, and a
        non-zero value (-1) otherwise. When an error has occurred,
        the global <errno> contains a value indicating the type of
        error that has been detected.

    Macro Portability:
        A Grief extension.

    Macro See Also:
        cd, mkdir, rmdir, getwd
 */
void
do_chdir(void)       /* (string dir) */
{
    int ret;

    ret = system_call(vfs_chdir(get_str(1)));
    acc_assign_int(ret);
}


/*  Function:           do_rmdir
 *      rmdir primitive.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 *
 *<<GRIEF>>
    Macro: rmdir - Remove directory.

        int
        rmdir(string path)

    Macro Description:
        The 'rmdir()' primitive removes (deletes) the specified
        directory. The directory must not contain any files or
        directories. The path can be either relative to the current
        working directory or it can be an absolute path name.

    Macro Returns:
        The 'rmdir()' primitive returns zero if successful, and a
        non-zero value (-1) otherwise. When an error has occurred,
        the global <errno> contains a value indicating the type of
        error that has been detected.

    Macro See Also:
        mkdir, chdir, cd, getwd
 */
void
do_rmdir(void)                  /* (string dir) */
{
    int ret;

    ret = system_call(vfs_rmdir(get_str(1)));
    acc_assign_int(ret);
}


/*  Function:           do_rename
 *      rename primitive.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 *
 *<<GRIEF>>
    Macro: rename - Rename a file.

        int
        rename(string old, string new)

    Macro Description:
        The 'rename()' primitive causes the file whose name is
        indicated by the string 'old' to be renamed to the name
        given by the string 'new'.

    Macro Returns:
        The 'rename()' primitive returns zero if successful, and a
        non-zero value (-1) otherwise. When an error has occurred,
        the global <errno> contains a value indicating the type of
        error that has been detected.

    Macro See Also:
        remove, create_buffer, link, unlink
 */
void
do_rename(void)                 /* (string old, string new) */
{
    int ret;

    ret = vfs_rename(get_str(1), get_str(2));
    system_call(ret);
    acc_assign_int(ret);
}


/*  Function:           do_remove
 *      remove primitive.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 *
 *<<GRIEF>>
    Macro: remove - Remove a file.

        int
        remove(string filename)

    Macro Description:
        The 'remove()' primitive deletes the file whose name is
        contained within the string 'filename'.

    Macro Returns:
        The 'remove()' primitive returns zero if successful, and a
        non-zero value (-1)f otherwise. When an error has occurred,
        the global <errno> contains a value indicating the type of
        error that has been detected.

    Macro See Also:
        exist, access
 */
void
do_remove(void)                 /* int (string path) */
{
    char path[MAX_PATH];
    int ret;

    file_tilder(get_str(1), path, sizeof(path));
    ret = system_call(vfs_unlink(path));
    acc_assign_int(ret);
}


/*  Function:           do_exist
 *      exist primitive.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 *
 *<<GRIEF>>
    Macro: exist - Check file existence.

        int
        exist(string path, [int canon = TRUE])

    Macro Description:
        The 'exist()' primitive tests whether of the specified file
        'path' exists.

        If 'canon' is not specified, then the filename is
        canonised first. This involves expanding any tilde
        prefixes and converting DOS style filenames to Unix style
        ones.

    Macro Returns:
        The 'exist()' primitive returns non-zero if the file exists,
        otherwise 0 on error. When an error has occurred, the
        global <errno> contains a value indicating the type of
        error that has been detected.

    Macro See Also:
        access, stat, ftest
*/
void
do_exist(void)                  /* int (string path, [int canon = TRUE]) */
{
    const char *path = get_str(1);
    const int canon = get_xinteger(2, TRUE);
    char t_path[MAX_PATH];
    struct stat sb;

    if (canon) path = file_tilder(get_str(1), t_path, sizeof(t_path));
    if (NULL == path || x_stat(path, &sb) < 0) {
        acc_assign_int(0);
    } else {
        acc_assign_int(1);
    }
}


/*  Function:           do_ftest
 *      ftest primitive.
 *
 *  Description:
 *      File test operations.
 *
 *  Returns:
 *      nothing.
 *
 *<<GRIEF>>
    Macro: ftest - Test file type.

        int
        ftest(int|string condition, string path)

    Macro Description:
        The 'ftest()' primitive tests the type of the specified file
        'path' against the type 'condition'. The file test operations
        are modelled on standard unix test operators as follows.

            a -     *True* if file exists.

            b -     *True* if file exists and is a block special file.

            B -     *True* if file exists and is a possible binary stream.

            c -     *True* if file exists and is a character special file.

            d -     *True* if file exists and is a directory.

            e -     *True* if file exists.

            f -     *True* if file exists and is a regular file.

            g -     *True* if file exists and its set group ID flag is set.

            G -     *True* if file exists and its group matches the effective
                    group ID of this process.

            h -     *True* if file exists and is a symbolic link.

            k -     *True* if file exists and has its sticky bit set.

            L -     *True* if file exists and is a symbolic link.

            N -     *True* if file exists and has been modified since last access.

            O -     *True* if file exists and is owned by the effective user ID
                    of this process.

            p -     *True* if file is a named pipe (FIFO).

            r -     *True* if file exists and is readable.

            s -     *True* if file exists and has a size greater than zero.

            S -     *True* if file exists and is a socket.

            u -     *True* if file exists and its set-user-ID flag is set.

            w -     *True* if file exists and is writable. *True* will indicate only
                    that the write flag is on. The file will not be writable on a
                    read-only file system even if this test indicates true.

            x -     *True* if file exists and is executable. *True* will indicate
                    only that the execute flag is on. If file is a directory,
                    true indicates that file can be searched.

    Macro Returns:
        The 'ftest()' primitive returns the result of the test operation
        either 1 when *true* otherwise 0.

    Macro Portability:
        A Grief extenions.

    Macro See Also:
        access, exist, stat, chmod, chown
 */
void
do_ftest(void)                  /* int (string condition, string path) */
{
    const char *cmd = get_xstr(1);
    const int cmdch = (cmd ? *cmd : get_xinteger(1, 0));
    const char *path = get_str(2);
    struct stat sb;
    int ret = 0;

  //path = file_tilder(path, t_path, sizeof(t_path));
    switch (cmdch) {
    /* access */
    case 'a':
    case 'e':
        ret = (x_stat(path, &sb) == 0);
        break;

    case 'r':
        ret = (x_access(path, R_OK) == 0);
        break;

    case 'w':
        ret = (x_access(path, W_OK) == 0);
        break;

    case 'x':
        ret = (x_access(path, X_OK) == 0);
        break;

    /* ownership */
    case 'O':
#if defined(unix) || defined(__APPLE__)
        ret = (x_stat(path, &sb) == 0 &&
                geteuid() == (uid_t) sb.st_uid);
#endif
        break;

    case 'G':
#if defined(unix) || defined(__APPLE__)
        ret = (x_stat(path, &sb) == 0 &&
                getegid() == (gid_t) sb.st_gid);
#endif
        break;

    case 'N':
        ret = (x_stat(path, &sb) == 0 &&
                sb.st_atime <= sb.st_mtime);
        break;

    /* type */
    case 'c':
#if defined(S_ISCHR)
        ret = (x_stat(path, &sb) == 0 && S_ISCHR(sb.st_mode));
#endif
        break;

    case 'b':
#if defined(S_ISBLK)
        ret = (x_stat(path, &sb) == 0 && S_ISBLK(sb.st_mode));
#endif
        break;

    case 'B':
        ret = is_binary(path);
        break;

    case 'd':
        ret = (x_stat(path, &sb) == 0 && S_ISDIR(sb.st_mode));
        break;

    case 'f':
        ret = (x_stat(path, &sb) == 0 && S_ISREG(sb.st_mode));
        break;

    case 's':
        ret = (x_stat(path, &sb) == 0 && sb.st_size > (off_t) 0);
        break;

    case 'S':
#if defined(S_ISSOCK)
        ret = (x_stat(path, &sb) == 0 && S_ISSOCK(sb.st_mode));
#endif
        break;

    case 'p':
#if defined(S_ISFIFO)
        ret = (x_stat(path, &sb) == 0 && S_ISFIFO(sb.st_mode));
#endif
    break;

    case 'h':
    case 'L':
#if defined(S_ISLNK) && defined(HAVE_LSTAT)
        ret = ((path[0] != '\0') &&
                    (vfs_lstat(path, &sb) == 0) && S_ISLNK (sb.st_mode));
#endif
        break;

    case 'u':
#if defined(S_ISUID)
        ret = (x_stat(path, &sb) == 0 && (sb.st_mode & S_ISUID) != 0);
#endif
        break;

    case 'g':
#if defined(S_ISGID)
        ret = (x_stat(path, &sb) == 0 && (sb.st_mode & S_ISGID) != 0);
#endif
        break;

    case 'k':
#if defined(S_ISVTX)
        ret = (x_stat(path, &sb) == 0 && (sb.st_mode & S_ISVTX) != 0);
#endif
        break;

    default:
        break;
    }

    acc_assign_int((accint_t) ret);
}


static int
x_open(const char *path, int oflags, int mode)
{
    if (NULL == path || path[0] == '\0') {
        errno = ENOENT;
        return system_call(-1);
    }
    return (system_call(vfs_open(path, oflags, mode)));
}


static int
x_stat(const char *path, struct stat *sb)
{
    if (NULL == path || path[0] == '\0') {
        errno = ENOENT;
        return system_call(-1);
    }
    return (system_call(vfs_stat(path, sb)));
}


static int
x_access(const char *path, int what)
{
    if (NULL == path || path[0] == '\0') {
        errno = ENOENT;
        return system_call(-1);
    }
    return (system_call(vfs_access(path, what)));
}


static int
is_binary(const char *path)
{
    struct stat sb;
    FSIZE_t offset = 0;
    int fd, ret = 0;

    if ((fd = vfs_open(path, OPEN_R_BINARY | O_RDONLY, 0)) >= 0) {
        if (0 == vfs_fstat(fd, &sb) && S_ISREG(sb.st_mode)) {
            /*
             *  regular file
             */
            unsigned char *buf;
            int len;

            if (NULL != (buf = chk_alloc(TEST_SIZE))) {
                len = vfs_read(fd, buf, TEST_SIZE);
                if (len > 0) {
                    const unsigned char *cursor = buf, *bpend = cursor + len;

                    while (cursor < bpend && 0 == ret) {
                        /*
                         *  Basic guess functionality/
                         *      TODO - use guess logic.
                         */
                        const unsigned char ch = *cursor++;

                        if (ch < 0x20) {
                            switch (ch) {
                            case '\r': case '\n':
                            case '\t': case '\v':
                            case '\f':
                            case '\b':
                            case 0x1b:          /* ESC */
                                break;
                            default:            /* skip leading, possible BOM */
                                if (offset >= 4) {
                                    ret = 1;
                                }
                                break;
                            }
                        }
                        ++offset;
                    }
                }
                chk_free(buf);
            }
        } else {
            /*
             *  directory, pipe etc
             */
            ret = 1;                            /* assume binary */
        }
        vfs_close(fd);
    }
    return ret;
}


/*  Function:           do_chmod
 *      chmod primitive.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 *
 *<<GRIEF>>
    Macro: chmod - Change mode.

        int
        chmod(string path, int mode)

    Macro Description:
        The 'chmod()' primitive changes the permissions for a file
        specified by 'path' to be the settings in the 'mode' given by
        permission.

        The access permissions for the file or directory are specified
        as a combination of bits which are operating system specific.

    Macro Returns:
        The 'chmod()' primitive returns zero if successful, and a
        non-zero value (-1) otherwise. When an error has occurred,
        the global <errno> contains a value indicating the type of
        error that has been detected.

    Macro See Also:
        chown, stat, lstat, umask
 */
void
do_chmod(void)                  /* int (string path, int mode) */
{
    int ret;

  //path = file_tilder(path, t_path, sizeof(t_path));
    ret = vfs_chmod(get_str(1), get_xinteger(2, 0));
    system_call(ret);
    acc_assign_int((accint_t) ret);
}



/*  Function:           do_chmod
 *      chmod primitive.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 *
 *<<GRIEF>>
    Macro: chown - Change owner.

        int
        chown(string path, int owner, int group)

    Macro Description:
        The 'chown()' primitive is reserved for future use.

    Macro Returns:
        The 'chown()' primitive returns zero if successful, and a
        non-zero value (-1) otherwise. When an error has occurred,
        the global <errno> contains a value indicating the type of
        error that has been detected.

    Macro See Also:
        chmod, stat, lstat, umask
 */
void
do_chown(void)                  /* int (string path, int owner, int group) */
{
    int ret;

    ret = vfs_chown(get_str(1), get_xinteger(2, 0), get_xinteger(3, 0));
    system_call(ret);
    acc_assign_int((accint_t) ret);
}


/*  Function:           do_umask
 *      umask primitive.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 *
 *<<GRIEF>>
    Macro: umask - Set and get the file mode creation mask

        int
        umask(int cmask = NULL)

    Macro Description:
        The 'umask()' primitive shall set the processes file mode
        creation mask to 'cmask' and return the previous value of
        the mask.

        Only the file permission bits of 'cmask' are used; the
        meaning of the other bits is implementation-defined.

        The process' file mode creation mask is used to turn off
        permission bits in the mode argument supplied during calls to
        the following functions:

            o <fopen>
            o <create_buffer>
            o <mkdir>

    Macro Returns:
        The file permission bits in the value returned by umask()
        shall be the previous value of the file mode creation mask.

    Macro See Also:
        chmod, stat, lstat
 */
void
do_umask(void)                  /* (int [int mask = NULL]) */
{
    int ret = x_umask;

    assert(sizeof(mode_t) <= sizeof(accint_t));
    argv_assign_int(2, x_umask);
    if (isa_integer(1)) {
        x_umask = (mode_t) get_xinteger(1, 0);
        ret = fileio_umask(x_umask);
    }
    acc_assign_int(ret);
}


/*  Function:           acc_assign_stat
 *      Common return argument function for stat and lstat and primitives.
 *
 *  Parameters:
 *      sb - Status buffer.
 *      rc - Return code.
 *
 *  Returns:
 *      nothing.
 */
void
acc_assign_stat(struct stat *sb, int rc)
{
    if (0 == rc) {
        stat_assign(sb);                        /* arguments 1... */
    }
    system_call(rc);
    acc_assign_int((accint_t) rc);
}


/*  Function:           stat_assign
 *      Common return argument function for stat, lstat and find_file primitives.
 *
 *  Parameters:
 *      sb - Status buffer.
 *
 *  Returns:
 *      nothing.
 */
static void
stat_assign(struct stat *sb)
{
    argv_assign_int(2,  (accint_t) sb->st_size);
    argv_assign_int(3,  (accint_t) sb->st_mtime);
    argv_assign_int(4,  (accint_t) sb->st_ctime);
    argv_assign_int(5,  (accint_t) sb->st_atime);
    argv_assign_int(6,  (accint_t) sb->st_mode);
    argv_assign_int(7,  (accint_t) sb->st_uid);
//  argv_assign_str(8,  (accint_t) sys_uid2name(sb->st_uid));   //TODO
    argv_assign_int(9,  (accint_t) sb->st_gid);
//  argv_assign_str(10, (accint_t) sys_gid2name(sb->st_gid));   //TODO
    argv_assign_int(11, (accint_t) sb->st_nlink);
    argv_assign_int(12, (accint_t) sb->st_ino);
//  argv_assign_int(13, (accint_t) sb->st_dev);
}


/*  Function:           do_stat
 *      stat primitive.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 *
 *<<GRIEF>>
    Macro: stat - Obtain file information.

        int
        stat([string path], [int size],
                [int mtime], [int ctime],
                [int atime], [int mode], [int uid], [string uid2name],
                [int gid], [string gid2name],
                [int nlink], [int inode])

    Macro Description:
        The 'stat()' primitive obtain information about the file or
        directory referenced in 'path'.

        This information is returned in the parameters following
        the 'path' parameter (if supported by the underlying
        filesystem).

            o size     - Total file size, in bytes.

            o mode     - File mode (see File Modes).

            o mtime    - The files "last modified" time (see time).

            o atime    - Time the file was "last accessed".

            o ctime    - Time of the files "last status change".

            o uid      - User identifier.

            o uid2name - User name associated with the file 'uid'.

            o gid      - Group identifier.

            o gid2name - Group name associated with the file 'gid'.

            o nlink    - Number of hard links.

            o inode    - File-system internal node number.

    Macro Parameters:
        path - String containing the file path. If omitted the
            statistics of the current buffer shall be retrieved.

        size - Optional integer, if specified is populated with the size
            of the related file in bytes.

                o For regular files, the file size in bytes.
      o For symbolic links, the length in bytes of the
          path-name contained in the symbolic link.
                o For a shared memory object, the length in bytes.
                o For a typed memory object, the length in bytes.
                o For other file types, the use of this field is
                    unspecified.

        mtime - Optional integer, populated with the files modification
            time (see time).

        ctime - Optional integer, populated with the time of the last
            status change.

        atime - Optional integer, populated with the last access time.

        mode - Optional integer, mode of file (see File Modes).

        uid - Optional integer, user identifier of the file.

        uid2name - User name associated with the file uid.

        gid - Optional integer, group identifier of the file.

        gid2name - Group name associated with the file gid.

        nlink - Optional integer, number of hard links to the file.

        inode - Optional integer, populated with the file-system
            internal node number.

    Macro Returns:
        The 'stat()' primitive returns zero if successful, and a
        non-zero value (-1) otherwise. When an error has occurred,
        the global <errno> contains a value indicating the type of
        error that has been detected.

    Macro Portability:
        n/a

    Macro See Also:
        lstat, access, exist, ftest

 *<<GRIEF>> [file]
    Topic: File Modes

        Traditional Unix file mode consist of a number of components
        including type, permissions including special bits.

        The <mode_string> primitive creates a human readable string
        version of these bits in a system independent form.

    Type:

        The file type is represented by the following constants.

            S_IFBLK  - Block special.
            S_IFCHR  - Character special.
            S_IFIFO  - FIFO special.
            S_IFREG  - Regular.
            S_IFDIR  - Directory.
            S_IFLNK  - Symbolic link.
            S_IFSOCK - Socket.

    Classes:

        Permissions on Unix-like systems are managed in three
        distinct classes. These classes are known as user, group,
        and others.

        Files and directories are owned by a 'user'. The owner
        determines the file's owner class. Distinct permissions
        apply to the owner.

        Files and directories are assigned a 'group', which define
        the file's group class. Distinct permissions apply to
        members of the file's group. The owner may be a member of
        the file's group.

        Users who are not the owner, nor a member of the group,
        comprise a file's 'others' class. Distinct permissions
        apply to 'others'.

        The effective permissions are determined based on the
        user's class. For example, the user who is the owner of the
        file will have the permissions given to the owner class
        regardless of the permissions assigned to the group class
        or others class.

    Permissions:

        For each group there are three specific permissions on
        Unix-like systems that apply to each class:

        o Read permission - grants the ability to read a file. When
        set for a directory, this permission grants the ability to
        read the names of files in the directory (but not to find
        out any further information about them such as contents,
        file type, size, ownership, permissions, etc.)

        o Write permission - grants the ability to modify a file.
        When set for a directory, this permission grants the
        ability to modify[clarify] entries[clarify] in the
        directory. This includes creating files, deleting files,
        and renaming files.

        o Execute permission - grants the ability to execute a
        file. This permission must be set for executable binaries
        (for example, a compiled C++ program) or shell scripts (for
        example, a Perl program) in order to allow the operating
        system to run them. When set for a directory, this
        permission grants the ability to access file contents and
        metainfo if its name is known, but not list files inside
        the directory (unless read is set).

        The file permissions are represented by the following
        constants.

        Read, write, execute/search by owner.

            S_IRUSR - Read permission, owner.
            S_IWUSR - Write permission, owner.
            S_IXUSR - Execute/search permission, owner.

        Read, write, execute/search by group.

            S_IRGRP - Read permission, group.
            S_IWGRP - Write permission, group.
            S_IXGRP - Execute/search permission, group.

        Read, write, execute/search by others.

            S_IROTH - Read permission, others.
            S_IWOTH - Write permission, others.
            S_IXOTH - Execute/search permission, others.

    Special Bits:

        Unix-like systems typically employ three additional modes.
        These are actually attributes but are referred to as
        permissions or modes. These special modes are for a file or
        directory overall, not by a class.

        o The set user ID, setuid, or SUID mode. When a file with
        setuid is executed, the resulting process will assume the
        effective user ID given to the owner class. This enables
        users to be treated temporarily as root (or another user).

        o The set group ID, setgid, or SGID permission. When a file
        with setgid is executed, the resulting process will assume
        the group ID given to the group class. When setgid is
        applied to a directory, new files and directories created
        under that directory will inherit the group from that
        directory. (Default behaviour is to use the primary group
        of the effective user when setting the group of new files
        and directories.)

        o The sticky mode, when on a directory, the sticky
        permission prevents users from renaming, moving or deleting
        contained files owned by users other than themselves, even
        if they have write permission to the directory. Only the
        directory owner and superuser are exempt from this.

        The special bits are represented by the following constants.

            S_ISUID - Set-user-ID on execution.
            S_ISGID - Set-group-ID on execution.
            S_ISVTX - On directories, restricted deletion flag. [Option End]

 */
void
do_stat(void)                   /* ([string path], [int size], [int mtime], [int ctime], [int atime],
                                        [int mode], [int uid], [string uid2name], [int gid], [string gid2name], [int nlink] */
{
    const char *path = get_xstr(1);
    struct stat sb = {0};

    if (NULL == path) {                         /* extension, current buffer */
        int ret = -1;

        if (curbp) {
            if ((ret = vfs_stat(curbp->b_fname, &sb)) < 0) {
//              sb.st_size  = (is_undef(2) ? -1 : buf_size(curbp));
                sb.st_mtime = curbp->b_mtime;
                sb.st_ctime = curbp->b_ctime;
                sb.st_atime = 0;
                sb.st_mode  = curbp->b_mode;
                sb.st_uid   = curbp->b_uid;
                sb.st_gid   = curbp->b_gid;
                ret = 0;
            }
        }
        acc_assign_stat(&sb, ret);
    } else {
//      path = file_tilder(path, t_path, sizeof(t_path));
        acc_assign_stat(&sb, vfs_stat(get_str(1), &sb));
    }
}


/*  Function:           do_lstat
 *      lstat primitive.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 *
 *<<GRIEF>>
    Macro: lstat - Get symbolic link status.

        int
        lstat(string path, [int size],
                [int mtime], [int ctime], [int atime], [int mode],
                [int uid], [string uid2name],
                [int gid], [string gid2name],
                [int nlink], [int inode])

    Macro Description:
        The 'lstat()' primitive shall be equivalent to <stat>, except when
        path refers to a symbolic link. In that case 'lstat' shall
        return information about the link, while 'stat' shall return
        information about the file the link references.

        For symbolic links, the 'mode' member shall contain meaningful
        information when used with the file type macros, and the 'size'
        member shall contain the length of the pathname contained in the
        symbolic link. File mode bits and the contents of the remaining
        members of the stat structure are unspecified. The value
        returned in the 'size' member is the length of the contents of
        the symbolic link, and does not count any trailing null.

    Macro Parameters:
        path - String containing the file path.

        size - Optional integer, if specified is populated with the
            size of the related file in bytes.

                o For regular files, the file size in bytes.
                o For symbolic links, the length in bytes of the
                    path-name contained in the symbolic link.
                o For a shared memory object, the length in bytes.
                o For a typed memory object, the length in bytes.
                o For other file types, the use of this field is
                    unspecified.

        mtime - Optional integer, populated with the files
            modification time (see time).

        ctime - Optional integer, populated with the time of the last
            status change.

        atime - Optional integer, populated with the last access time.

        mode - Optional integer, mode of file (see File Modes).

        uid - Optional integer, user identifier of the file.

        uid2name - User name associated with the file uid.

        gid - Optional integer, group identifier of the file.

        gid2name - Group name associated with the file gid.

        nlink - Optional integer, number of hard links to the file.

        inode - Optional integer, populated with the file-system
            internal node number.

    Macro Returns:
        The 'lstat()' primitive returns zero if successful, and a
        non-zero value (-1) otherwise. When an error has occurred,
        the global <errno> contains a value indicating the type of
        error that has been detected.

    Macro Portability:
        A Grief extension.

    Macro See Also:
        stat
 */
void
do_lstat(void)                  /* (string path, [int size], [int mtime], [int ctime], [int atime],
                                        [int mode], [int uid], [string uid2name], [int gid], [string gid2name], [int nlink] */
{
    struct stat st;

  //path = file_tilder(path, t_path, sizeof(t_path));
    acc_assign_stat(&st, vfs_lstat(get_str(1), &st));
}


/*<<GRIEF>> [file]
    Macro: set_ea - Set file extended information.

        int
        set_ea(string filename, ...)

    Macro Description:
        The 'set_ea()' primitive is reserved for future BRIEF
        compatibility.

    Macro Parameters:
        n/a

    Macro Returns:
        n/a

    Macro Portability:
        Provided for BRIEF compatibility.

    Macro See Also:
        read_ea, copy_ea_info, set_ea
 */
void
do_set_ea(void)
{
    //TODO
    acc_assign_int((accint_t) -1);
}


/*<<GRIEF>> [file]
    Macro: copy_ea_info - Copy file extended information.

        int
        copy_ea_info(string sourcename, string destname)

    Macro Description:
        The 'copy_ea()' primitive is reserved for future BRIEF
        compatibility.

    Macro Parameters:
        n/a

    Macro Returns:
        n/a

    Macro Portability:
        Provided for BRIEF compatibility.

    Macro See Also:
        read_ea, copy_ea_info, set_ea
 */
void
do_copy_ea_info(void)
{
    //TODO
    acc_assign_int((accint_t) -1);
}


/*<<GRIEF>> [file]
    Macro: read_ea - Read file extended information.

        int
        read_ea(string filename, ...)

    Macro Description:
        The 'read_ea()' primitive is reserved for future BRIEF
        compatibility.

    Macro Parameters:
        n/a

    Macro Returns:
        n/a

    Macro Portability:
        Provided for BRIEF compatibility.

    Macro See Also:
        read_ea, copy_ea_info, set_ea
 */
void
do_read_ea(void)
{
    //TODO
    acc_assign_int((accint_t) -1);
}


/*  Function:           do_mode_string
 *      mode_string primitive.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 *
 *<<GRIEF>>
    Macro: mode_string - Conversion stat mode to a string representation.

        string
        mode_string([int mode], [int format = 0], [string path])

    Macro Description:
        The 'mode_string()' primitive decodes the specified 'mode'
        into a human readable form using a style similar to 'ls' long
        listing format output detailing type and permissions, for
        example

>           drwxr-xr-x-

    Mode String:

        The decoded mode string consists of a ten character string,
        using the following format (see File Modes)

>           <type> <owner> <group> <other> <sticky>

      Type::

        The first character indicates the file 'type' and is not
        related to permissions, when format is omitted or (0) shall
        be on of the following:

            'd' -   Directory.
            'c' -   Character-device.
            'b' -   Block-device.
            'l' -   Link.
            'p' -   Fifo/pipe.
            's' -   Sockets.
            'n' -   Name.
            'D' -   Door.
            '-' -   Normal.

        The alternative format shall be used when 'format' is given
        as a non-zero value. In addition if specified 'source' shall
        be utilised to verify the status of the link.

            '/' -   Directories.
            '-' -   Character devices.
            '+' -   Block devices.
            '~' -   Directory link.
            '!' -   Broken link.
            '@' -   Link.
            '|' -   Fifo/pipe.
            '=' -   Sockets.
            '$' -   Name/door.
            '*' -   Executable.
            ' ' -   Normal (space).

     Permissions::

        Following are three permission sets defining the 'user',
        'group' and 'other' access rights.

        Each of the three characters represent the read, write, and
        execute permissions for each of the groups in the order (rwx).

            'r' -   Read permission.
            'w' -   Write permission.
            'x' -   Execute permission
            '-' -   No associated permission read, write or execute.

     Sticky::

        The trailing character details the one of two special
        attributes.

            'S' -   S_ISUID is set.
            'T' -   S_ISVTX is set.

    Macro Parameters:
        mode - Optional mode specification, otherwise the associate
            mode of current buffer is decoded.

        format - Optional format, when stated and non-zero the <type>
            field is decoded using an alternative form.

        path - Optional source of the mode, is supplied shall be
            utilised to verify the status of links.

    Macro Returns:
        Returns the decoded mode string.

    Macro Portability:
        A Grief extension.

    Macro See Also:
        File Modes, stat, lstat
 */
void
do_mode_string(void)            /* string ([int mode], [string path], [int type = 0]) */
{
    const mode_t mode = (mode_t) get_xinteger(1, (curbp ? curbp->b_mode : 0));
    const char *source = get_xstr(2);
    const int type = (int) get_xinteger(3, 0);
    char buffer[16];

    file_modedesc(mode, source, type, buffer, sizeof(buffer));
    acc_assign_str((const char *)buffer, -1);
}


/*  Function:           do_readlink
 *      readlink primitive.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 *
 *<<GRIEF>>
    Macro: readlink - Read the contents of a symbolic link.

        string|int
        readlink(string path, [string &link])

    Macro Description:
        The 'readlink()' primitive shall place the contents of the
        symbolic link referred to by 'path' in the string 'link'.

    Macro Returns:
        If 'link' is omitted the return value is a 'string'. Upon
        successful completion readlink() shall return the resolved
        link, otherwise it shall return an empty string and set the
        global <errno> to indicate the error.

        Then 'link' is given the return value is an 'int'. Upon
        successful completion, 'readlink' shall return the count of
        character placed in the string. Otherwise, it shall return
        a value of -1, and set the global <errno> to indicate the
        error.

    Macro Portability:
        A Grief extension.

    Macro See Also:
        lstat, realpath, symlink
 */
void
do_readlink(void)               /* string (string path, [string &link]) */
{
    const char *name = get_str(1);
    char linkpath[MAX_PATH];
    int ret;

//TODO
//  path = file_tilder(path, t_path, sizeof(t_path));
//  if (vfs_readlink(name, link, sizeof(link)-1) == -1) {
#if defined(unix) || defined(__APPLE__)
    if ((ret = readlink(name, linkpath, sizeof(linkpath)-1)) <= -1) {
#else
    if ((ret = sys_readlink(name, linkpath, sizeof(linkpath)-1)) <= -1) {
#endif
        acc_assign_int(errno == ENOSYS ? 0 : -1);

    } else {
        if (! isa_undef(2)) {
            argv_assign_str(2, (const char *)linkpath);
            acc_assign_int(ret ? ret : 1);

        } else {
            acc_assign_str((const char *)linkpath, -1);
        }
    }
}


/*<<GRIEF>>
    Macro: link - Link a file.

        int
        link(string path1, string path2)

    Macro Description:
        The 'link()' primitive is reserved for future use.

        The 'link()' primitive shall create a new link (directory entry)
        for the existing file, 'path1'.

        The 'path1' argument points to a pathname naming an existing
        file. The 'path2' argument points to a pathname naming the new
        directory entry to be created. The 'link()' primitive shall
        atomically create a new link for the existing file and the link
        count of the file shall be incremented by one.

        If 'path1' names a directory, link() shall fail unless the
        process has appropriate privileges and the implementation
        supports using link() on directories.

        If 'path1' names a symbolic link, it is implementation-defined
        whether link() follows the symbolic link, or creates a new link
        to the symbolic link itself.

        If 'link()' fails, no link shall be created and the link count of
        the file shall remain unchanged.

    Macro Returns:
        Upon successful completion, 'link' shall return 0;
        otherwise, it shall return -1 and set the global <errno> to
        indicate the error.

    Macro Portability:
        A Grief extension.

    Macro See Also:
        symlink, unlink, stat, remove
 */
void
do_link(void)                   /* int (string a, string b) */
{
    //TODO
    acc_assign_int(-1);
}


/*<<GRIEF>>
    Macro: unlink - Unlink a file.

        int
        unlink(string path)

    Macro Description:
        The 'unlink()' primitive is reserved for future use.

        The 'unlink()' primitive shall remove a link to a file. If path
        names a symbolic link, unlink() shall remove the symbolic link
        named by path and shall not affect any file or directory named
        by the contents of the symbolic link. Otherwise, unlink() shall
        remove the link named by the pathname pointed to by path and
        shall decrement the link count of the file referenced by the link.

        When the file's link count becomes 0 and no process has the file
        open, the space occupied by the file shall be freed and the file
        shall no longer be accessible. If one or more processes have the
        file open when the last link is removed, the link shall be
        removed before unlink() returns, but the removal of the file
        contents shall be postponed until all references to the file are
        closed.

        The path argument shall not name a directory unless the process
        has appropriate privileges and the implementation supports using
        unlink() on directories.

    Macro Returns:
        Upon successful completion, 'link' shall return 0;
        otherwise, it shall return -1 and set the global <errno> to
        indicate the error.

    Macro Portability:
        A Grief extension.

    Macro See Also:
        link, symlink, stat, remove
 */
void
do_unlink(void)                 /* int (string path) */
{
    //TODO
    acc_assign_int(-1);
}


/*  Function:           do_symlink
 *      symlink primitive.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 *
 *<<GRIEF>>
    Macro: symlink - Create a symbolic link.

        int
        symlink(string path1, string path2)

    Macro Description:
        The 'symlink()' primitive shall create a symbolic link called
        'path2' that contains the string 'path1'.

        In other words, 'path2' is the name of the symbolic link created,
        'path1' is the string contained in the symbolic link.

        If the symlink() primitive fails for any reason other than any
        file named by path2 shall be unaffected.

    Macro Returns:
        Upon successful completion, symlink() shall return 0;
        otherwise, it shall return -1 and set the global <errno> to
        indicate the error.

    Macro Portability:
        A Grief extension.

    Macro See Also:
        lstat, readlink
 */
void
do_symlink(void)                /* int (string, string) */
{
    const char *path1= get_str(1);
    const char *path2 = get_str(2);
    int ret = -1;

    if (path1 && path2) {
#if defined(unix) || defined(__APPLE__)
        if ((ret = symlink(path1, path2)) <= -1)
#else
        if ((ret = sys_symlink(path1, path2)) <= -1)
#endif
        {
            system_call(ret);
        }
    } else {
        system_errno(EINVAL);
    }
    acc_assign_int(ret);
}


/*  Function:           do_realpath
 *      realpath primitive.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 *
 *<<GRIEF>>
    Macro: realpath - Resolve a pathname.

        int
        realpath(string pathname, string resolved_path)

    Macro Description:
        The 'realpath()' primitive shall derive, from the 'pathname' an
        absolute pathname that names the same file, whose resolution
        does not involve '.', '..', or symbolic links.

    Macro Returns:
        The 'realpath()' primitive returns 0 if successful otherwise
        -1 on error. When an error has occurred, the global <errno>
        contains a value indicating the type of error that has been
        detected.

    Macro Portability:
        A Grief extension.

    Macro See Also:
        getwd(), symlink()
 */
void
do_realpath(void)               /* int (string pathname, string resolved_path) */
{
    char path[MAX_PATH], t_realpath[MAX_PATH+1], *name;
    int ret;

    name = path;

    file_tilder(get_str(1), path, sizeof(path));
//TODO
//  if (0 == (ret = vfs_realpath((const char *)path, t_realpath))) {
    if (0 == (ret = sys_realpath((const char *)path, t_realpath, sizeof(t_realpath)))) {
        name = t_realpath;
    }
    argv_assign_str(2, name);
    acc_assign_int((accint_t) ret);
}


/*  Function:           do_filename_realpath
 *      filename_realpath primitive.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 *
 *<<GRIEF>>
    Macro: filename_realpath - Return a resolved pathname.

        string
        filename_realpath(string pathname)

    Macro Description:
        The 'filename_realpath()' primitive shall derive, from the
        'pathname' an absolute pathname that names the same file,
        whose resolution does not involve '.', '..', or symbolic links.

    Macro Returns:
        The 'file_realpath()' primitive returns the resolved file if
        successful otherwise the original 'pathname'.

    Macro See Also:
        realpath
 */
void
do_filename_realpath(void)      /* string (string pathname) */
{
    char path[MAX_PATH], t_realpath[MAX_PATH], *name;

    name = path;
    file_tilder(get_str(1), path, sizeof(path));
//TODO
//  if (0 == (ret = vfs_realpath((const char *)path, t_realpath))) {
    if (0 == sys_realpath((const char *)path, t_realpath, sizeof(t_realpath))) {
        name = t_realpath;
    }
    acc_assign_str((const char *)name, -1);
}



/*
 *<<GRIEF>>
    Macro: parse_filename - Parse a file into its components

        int
        parse_filename(string fullname, [string &drive],
                [string &path], [string &filename], [string &ext])

    Macro Description:
        The 'parse_filename()' primitive parsing and brakes the file
        name 'fullname' into it components.

        Note!:
        Since this primitive is not portable outside of a DOS/Windows
        environment, its use is not adviced.

    Macro Parameters:
        fullname - A string containing the file-name to be parsed.

        drive - Optional string variable when supplied to be
            populated with the drive component, if any.

        path - Optional string variable when supplied to be populated
            with the path component.

        filename - Optional string variable when supplied to be
            populated with the file-name component.

        ext - Optional string variable when supplied to be populated
            with the file extension.

    Macro Returns:
        The 'parse_filename()' primitive returns non-zero on success
        denoted the fullname was parsed, otherwise zero was
        unsuccessful and -1 if an empty fullname was supplied.

    Macro Portability:
        Provided for BRIEF compatibility.

    Macro See Also:
        dirname, basename
 */
void
do_parse_filename(void)         /* int (string fullname, [string &drive],
                                        [string &path], [string &filename], [string &ext]) */
{
    const unsigned char *path = (unsigned char *)get_xstr(1),
        *end = path + (get_strlen(1) - 1), *p;
    unsigned char c;

    if (!path || !*path) {
        argv_assign_str(2, "");
        argv_assign_str(3, "");
        argv_assign_str(4, "");
        argv_assign_str(5, "");
        acc_assign_int(-1);
        return;
    }

    /* drive */
//#if defined(DOSISH)           /* X: */
    if (isalpha(*path) && ':' == path[1]) {
        argv_assign_nstr(2, (const char *)path, 2);
        path += 2;
    } else {
        argv_assign_str(2, "");
    }
//#else
//  argv_assign_str(2, "");
//#endif

    /* extension */
    for (p = end; p > path && 0 != (c = *--p) && !ISSEP(c);) {
        if ('.' == c) {
            if (p > path && !ISSEP(p[-1])) {    /* not /.xxx */
                end = p;
            }
            break;
        }
    }
    argv_assign_str(5, ('.' == *end ? (const char *)(end + 1) : ""));

    /* file */
    for (p = end; p > path && 0 != (c = *--p);) {
        if (ISSEP(c)) {
            ++p;
            break;
        }
    }
    if (p < end) {
        argv_assign_nstr(4, (const char *)p, end - p);
    } else {
        argv_assign_str(4, "");
    }

    /* directory */
    if (path < p) {
        argv_assign_nstr(3, (const char *)path, p - path);
    } else {
        argv_assign_str(3, "");
    }

    acc_assign_int(1);
}


/*  Function:           do_filename
 *      basename and dirname primitives.
 *
 *  Aliases:            do_basename
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 */
/*
<<GRIEF>>
    Macro: basename - Return the last component of a pathname.

        int
        basename(string pathname, [string suffix])

    Macro Description:
        The 'basename()' primitive returns a string containing the
        final component of a 'pathname' contained in the string
        path argument, deleting trailing path separators.

        To accomplish this, basename first checks to see if name
        consists of nothing. but slash (/) or backslash (\)
        characters. If so, basename replaces name with a single
        slash and the process is complete. If not, basename removes
        any trailing slashes. If slashes still remain, basename
        strips off all leading characters up to and including the
        final slash.

        If you specify 'suffix' and the remaining portion of name
        contains a suffix which matches suffix, basename removes
        that suffix.

    Macro Returns:
        The 'basename()' primitive returns a string containing the
        final component of the specified 'pathname' after
        processing following the above rules.

    Macro See Also:
        dirname, strfilecmp

<<GRIEF>>
     Macro: dirname - Report the parent directory name of a file pathname.

        int
        dirname(string path)

    Macro Description:
        The 'dirname()' primitive shall take a string 'path' that
        contains a pathname, and return a string containing that
        is a pathname of the parent directory of that file.
        Trailing directory separators ('/' and '\') characters in
        the path are not counted as part of the path.

        If 'path' does not contain a directory separator ('/' and
        '\') or is an empty string, then 'dirname' shall return a
        string ".".

    Macro Returns:
        The 'dirname()' primitive returns string containing the
        parent directory name of a file pathname, otherwise a
        string containing ".".

    Macro Portiablity:
        A Grief extension.

    Macro See Also:
        basename
 */
void
do_filename(int dirname)        /* (string path, [string suffix]) */
{
    const char *fname = get_str(1);
    const char *suffix = get_xstr(2);
    const char *p, *end;

    /* if all separators, return a single */
    if (NULL == fname || !*fname) {
        acc_assign_str(dirname ? "." : "", -1);
        return;
    }

    for (p = fname;; ++p) {
        if (!*p) {                              /* EOS */
            acc_assign_str("/", -1);
            return;
        }
        if (!ISSEP(*p)) {                       /* non-separator */
            break;
        }
    }

    /* remove trailing */
    for (; *p; ++p) {                           /* find end */
        continue;
    }
    while (p > fname && ISSEP(p[-1])) {
        --p;                                    /* trim separators */
    }
    end = p;

    /* now find start of component */
    while (--p >= fname) {
        const unsigned char ch = (unsigned char)*p;

#if defined(DOSISH)
        if (ISSEP(ch) || (':' == ch && (p - 1) == fname)) {
            break;
        }
#else
        if (ISSEP(ch)) {
            break;
        }
#endif
    }

    /* dirname mode, return directory */
    if (dirname) {
        int flen;

        if (p > fname) {
            --p;                                /* consume separator */
            while (p > (fname + 1) && ISSEP(p[-1])) {
                --p;                            /* trim off trailing separators */
            }
        }

        if ((flen = (p - fname) + 1) > 0) {
            acc_assign_str(fname, flen);
        } else {
            acc_assign_str(".", 1);
        }
        return;
    }

    /* otherwise basename, if suffix and a match remove */
    ++p;                                        /* consume separator */
    if (suffix) {
        int slen = strlen(suffix);              /* suffix length */
        int flen = end - p;                     /* filename (fname) length */

        if (slen < flen) {
            if (0 == strncmp(suffix, end - slen, slen)) {
                end -= slen;
            }
        }
    }
    acc_assign_str(p, end - p);
}


/*  Function:           do_strfilecmp
 *      strfilecmp primitive.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 *
 *<<GRIEF>>
    Macro: strfilecmp - Filename comparison.

        int
        strfilecmp(string file1, string file2, [int length])

    Macro Description:
        The 'strfilecmp()' primitive lexicographically compares the
        filenames 'name1' and 'name2' and returns a value
        indicating their relationship, taking into account any
        filesystem implementation specifics.

        If specified 'length' defines the number of significant
        characters of each path which shall be compared, ignoring
        any characters after the length characters of each.

    Macro Notes:
        Under DOS, Windows and OS/2 this primitive is case
        insensitive and permits the intermixing of both back(\)
        and forward(/) slashes as directory delimiters.

        Under Unix(tm) this primitive is the same as an equality
        (==) operation between two strings.

    Macro Returns:
        The return indicates the lexicographic relation of name1
        and name2.

>           <0 - name1 less than name2.
>            0 - name1 identical to name2.
>           >0 - name1 greater than name2.

    Macro Portability:
        A Grief extenions.

    Macro See Also:
        strcmp
*/
void
do_strfilecmp(void)             /* (string file1, string file2, [int length]) */
{
    const char *s1 = get_str(1);
    const char *s2 = get_str(2);
    const int value3 = get_xinteger(3, -1);

    if (value3 > 0) {
        acc_assign_int(file_ncmp(s1, s2, value3));
    } else {
        acc_assign_int(file_cmp(s1, s2));
    }
}


/*  Function:           do_mktemp
 *      mktemp primitive.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 *
 *<<GRIEF>>
    Macro: mktemp - Make a temporary filename.

        string
        mktemp(string path)

    Macro Description:
        The 'mktemp()' primitive shall replace the contents of the
        string 'path' to by template by a unique filename and
        return template.

        The application shall initialize template to be a
        filename with six trailing 'X's. mktemp() shall replace
        each 'X' with a single byte character from the portable
        filename character set.

    Macro Returns:
        The 'mktemp()' primitive shall return the string template. If a
        unique name cannot be created, template shall point to an
        empty string.

    Macro See Also:
        fmktemp, create_buffer
 */
void
do_mktemp(void)                 /* string (string path) */
{
    char path[MAX_PATH];
    int ret;

    strxcpy(path, get_str(1), sizeof(path));
    if ((ret = sys_mkstemp(path)) >= 0) {
        acc_assign_str(path, -1);
        fileio_close(ret);

    } else {
        acc_assign_str("", -1);
    }
}
/*end*/

