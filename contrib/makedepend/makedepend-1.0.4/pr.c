/*

Copyright (c) 1993, 1994, 1998 The Open Group
Modified Adam Young, changes are public domain.

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation.

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of The Open Group shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from The Open Group.

*/

#include "def.h"
#include <assert.h>

void
add_include(struct filepointer *filep, struct inclist *file,
    struct inclist *file_red, const char *include, int type, boolean failOK)
{
	register struct inclist	*newfile;
	register struct filepointer *content;

	/*
	 * First decide what the pathname of this include file really is.
	 */
	newfile = inc_path(file->i_file, include, type);
	if (newfile == NULL) {
		if (failOK)
		    return;
		if (file != file_red)
			warning("%s (reading %s, line %ld): ",
				file_red->i_file, file->i_file, filep->f_line);
		else
			warning("%s, line %ld: ", file->i_file, filep->f_line);
		warning1("cannot find include file \"%s\"\n", include);
		show_where_not = TRUE;
		newfile = inc_path(file->i_file, include, type);
		show_where_not = FALSE;
		if (MFLAG_GENERATED & mflags) { /*APY*/
			warning1("assumed as generated \"%s\"\n", include);
			newfile = newinclude(include, include);
			newfile->i_flags |= SEARCHED;
		}
	}

	if (newfile) {
		included_by(file, newfile);
		if (!(newfile->i_flags & SEARCHED)) {
			newfile->i_flags |= SEARCHED;
			content = getfile(newfile->i_file);
			find_includes(content, newfile, file_red, 0, failOK);
			freefile(content);
		}
	}
}

static void
pr(struct inclist *ip, const char *file, const char *base)
{
	static const char *lastfile;
	static int current_len;
	register int len, i;

	/*
	 * mask system directories
	 */
	if (MFLAG_NOSYSTEM & mflags) {			/*APY*/
//		char path[ BUFSIZ ];
		const char **pp;

		for (pp = systemdirs; *pp; ++pp) {
			if (0 == strncmp(*pp, ip->i_file, strlen(*pp)))
				return;
//			(void) strncpy(path, *pp, sizeof(path));
//			path[sizeof(path)-1] = 0;
//			remove_dotdot(path);
//			if (0 == strncmp(path, ip->i_file, strlen(path)))
//				return;
		}
	}

	/* dump file */
	if (MFLAG_PHONY & mflags) {			/*APY*/
		if (file != lastfile) {
			fprintf(stdout, "\n%s: %s", mtarget, file);
		}
	}
	printed = TRUE;
	len = strlen(ip->i_file)+1;
	if (current_len + len > width || file != lastfile) {
		lastfile = file;
		if (MFLAG_TARGET & mflags) {		/*APY*/
			len = fprintf(stdout, "\n%s: %s", mtarget, ip->i_file);
		} else {
			len = fprintf(stdout, "\n%s%s%s: %s", objprefix, base, objsuffix, ip->i_file);
		}
		current_len = len;
	}
	else {
		fprintf(stdout, " %s", ip->i_file);
		current_len += len;
	}

	/*
	 * If verbose is set, then print out what this file includes.
	 */
	if (! verbose || ip->i_list == NULL || ip->i_flags & NOTIFIED)
		return;
	ip->i_flags |= NOTIFIED;
	lastfile = NULL;
	printf("\n# %s includes:", ip->i_file);
	for (i=0; i<ip->i_listlen; i++)
		printf("\n#\t%s", ip->i_list[ i ]->i_incstring);
}

void
recursive_pr_include(struct inclist *head, const char *file, const char *base)
{
	int	i;

	if (head->i_flags & MARKED)
		return;
	head->i_flags |= MARKED;
	if (head->i_file != file)
		pr(head, file, base);
	for (i=0; i<head->i_listlen; i++)
		recursive_pr_include(head->i_list[ i ], file, base);
}
