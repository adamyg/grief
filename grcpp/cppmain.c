#include <edidentifier.h>
__CIDENT_RCSID(cppmain_c,"$Id: cppmain.c,v 1.4 2014/11/04 17:07:20 ayoung Exp $")

/* -*- mode: c; indent-width: 8; -*- */
/*
 * GRIEF grcpp 1.0.1, (c) A.Young 2010 - 2014
 *
 * Original Code is the 'C and T preprocessor, and integrated lexer (ucpp 1.3)'
 *
 * The initial Developer of the Original Code is Thomas Pornin
 * Portions created by Thomas Pornin are Copyright (C) Thomas Pornin 1999 - 2002
 * under the Revised BSD license.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 4. The name of the authors may not be used to endorse or promote
 *    products derived from this software without specific prior written
 *    permission.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <setjmp.h>
#include <stddef.h>
#include <limits.h>
#include <time.h>

#include "ucppi.h"
#include "mem.h"
#include "nhash.h"

#ifdef UCPP_MMAP
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <fcntl.h>
#endif

#define GRCPP_LABEL     "GRIEF grcpp"
#define GRCPP_VERSION   "1.0.1"

static char *		include_path_std[] = { STD_INCLUDE_PATH, 0 };
static char *		system_macros_def[] = { STD_MACROS, 0 };
static char *		system_assertions_def[] = { STD_ASSERT, 0 };


/*
 *  print some help
 */
static void 
usage(char *command_name)
{
	fprintf(stderr,
                GRCPP_LABEL " " GRCPP_VERSION " (" __DATE__ " / " __TIME__ ")\n"
		"\n"
		"Usage: %s [options] [file]\n"
		"\n"
		"language options:\n"
		"  -C                     keep comments in output\n"
		"  -s                     keep '#' when no cpp directive is recognized\n"
		"  -l                     do not emit line numbers\n"
		"  -lg                    emit gcc-like line numbers\n"
		"  -CC                    disable C++-like comments\n"
		"  -a, -na, -a0           handle (or not) assertions\n"
		"  -V                     disable macros with extra arguments\n"
		"  -u                     understand UTF-8 in source\n"
		"  -X                     enable -a, -u and -Y\n"
		"  -c90                   mimic C90 behaviour\n"
		"  -t                     disable trigraph support\n"
		"\n"
		"warning options:\n"
		"  -wt                    emit a final warning when trigaphs are encountered\n"
		"  -wtt                   emit warnings for each trigaph encountered\n"
		"  -wa                    emit warnings that are usually useless\n"
		"  -w0                    disable standard warnings\n"
		"\n"
		"directory options:\n"
		"  -I directory           add 'directory' before the standard include path\n"
		"  -J directory           add 'directory' after the standard include path\n"
		"  -zI                    do not use the standard include path\n"
		"  -M                     emit Makefile-like dependencies instead of normal output\n"
		"  -Ma                    emit also dependancies for system files\n"
		"  -o file                store output in file\n"
		"\n"
		"macro and assertion options:\n"
		"  -Dmacro                predefine 'macro'\n"
		"  -Dmacro=def            predefine 'macro' with 'def' content\n"
		"  -Umacro                undefine 'macro'\n"
		"  -Afoo(bar)             assert foo(bar)\n"
		"  -Bfoo(bar)             unassert foo(bar)\n"
		"  -Y                     predefine system-dependant macros\n"
		"  -Z                     do not predefine special macros\n"
		"  -d                     emit defined macros\n"
		"  -e                     emit assertions\n"
		"\n"
		"misc options:\n"
		"  -v                     print version number and settings\n"
		"  -notice                display copyright/author informmation\n"
		"  -h, --help             show this help\n"
		"\n",
		command_name);
}


static void
print_copyright(void)
{
	static const char *copyright[] = {
		"\n",
                "  " GRCPP_LABEL " " GRCPP_VERSION ", (c) A Young 2010 - 2014\n"
		"  All Rights Reserved\n",
		"\n",
		"  Original Code is the 'C and T preprocessor, and integrated lexer (ucpp 1.3)'\n",
		"\n",
		"  The initial Developer of the Original Code is Thomas Pornin\n",
		"  Portions created by Thomas Pornin are Copyright (C) Thomas Pornin 1999 - 2002\n",
		"  under the Revised BSD license.\n",
		"\n",
		"  Redistribution and use in source and binary forms, with or without\n",
		"  modification, are permitted provided that the following conditions\n",
		"  are met:\n",
		"  1. Redistributions of source code must retain the above copyright\n",
		"     notice, this list of conditions and the following disclaimer.\n",
		"  2. Redistributions in binary form must reproduce the above copyright\n",
		"     notice, this list of conditions and the following disclaimer in the\n",
		"     documentation and/or other materials provided with the distribution.\n",
		"  4. The name of the authors may not be used to endorse or promote\n",
		"     products derived from this software without specific prior written\n",
		"     permission.\n",
		"\n",
		"  THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR\n",
		"  IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED\n",
		"  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE\n",
		"  ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE\n",
		"  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR\n",
		"  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT\n",
		"  OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR\n",
		"  BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,\n",
		"  WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE\n",
		"  OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,\n",
		"  EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.\n",
		"\n",
		NULL };
	unsigned i;

	for (i = 0; copyright[i]; ++i) {
		fputs(copyright[i], stderr);
	}
}


/*
 *  parse_opt() initializes many things according to the command-line options.
 * 
 *  Return values:
 *  0  on success
 *  1  on semantic error (redefinition of a special macro, for instance)
 *  2  on syntaxic error (unknown options for instance)
 */
static int
parse_opt(int argc, char *argv[], struct lexer_state *ls)
{
	int	print_version = 0, print_defs = 0, print_asserts = 0;
	int	system_macros = 0, standard_assertions = 1;
	int	with_std_incpath = 1;
	char *	filename = 0;
	int	i, ret = 0;

	init_lexer_state(ls);
	ls->flags = DEFAULT_CPP_FLAGS;
	emit_output = ls->output = stdout;
	for (i = 1; i < argc; i ++) if (argv[i][0] == '-') {
		if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help")) {
			return 2;

		} else if (!strcmp(argv[i], "-C")) {
			ls->flags &= ~DISCARD_COMMENTS;

		} else if (!strcmp(argv[i], "-CC")) {
			ls->flags &= ~CPLUSPLUS_COMMENTS;

		} else if (!strcmp(argv[i], "-a")) {
			ls->flags |= HANDLE_ASSERTIONS;

		} else if (!strcmp(argv[i], "-na")) {
			ls->flags |= HANDLE_ASSERTIONS;
			standard_assertions = 0;

		} else if (!strcmp(argv[i], "-a0")) {
			ls->flags &= ~HANDLE_ASSERTIONS;

		} else if (!strcmp(argv[i], "-V")) {
			ls->flags &= ~MACRO_VAARG;

		} else if (!strcmp(argv[i], "-u")) {
			ls->flags |= UTF8_SOURCE;

		} else if (!strcmp(argv[i], "-X")) {
			ls->flags |= HANDLE_ASSERTIONS;
			ls->flags |= UTF8_SOURCE;
			system_macros = 1;
		
		} else if (!strcmp(argv[i], "-c90")) {
			ls->flags &= ~MACRO_VAARG;
			ls->flags &= ~CPLUSPLUS_COMMENTS;
			c99_compliant = 0;
			c99_hosted = -1;

		} else if (!strcmp(argv[i], "-t")) {
			ls->flags &= ~HANDLE_TRIGRAPHS;

		} else if (!strcmp(argv[i], "-wt")) {
			ls->flags |= WARN_TRIGRAPHS;

		} else if (!strcmp(argv[i], "-wtt")) {
			ls->flags |= WARN_TRIGRAPHS_MORE;

		} else if (!strcmp(argv[i], "-wa")) {
			ls->flags |= WARN_ANNOYING;

		} else if (!strcmp(argv[i], "-w0")) {
			ls->flags &= ~WARN_STANDARD;
			ls->flags &= ~WARN_PRAGMA;

		} else if (!strcmp(argv[i], "-s")) {
			ls->flags &= ~FAIL_SHARP;

		} else if (!strcmp(argv[i], "-l")) {
			ls->flags &= ~LINE_NUM;

		} else if (!strcmp(argv[i], "-lg")) {
			ls->flags |= GCC_LINE_NUM;

		} else if (!strcmp(argv[i], "-M")) {
			ls->flags &= ~KEEP_OUTPUT;
			emit_dependencies = 1;

		} else if (!strcmp(argv[i], "-Ma")) {
			ls->flags &= ~KEEP_OUTPUT;
			emit_dependencies = 2;

		} else if (!strcmp(argv[i], "-Y")) {
			system_macros = 1;

		} else if (!strcmp(argv[i], "-Z")) {
			no_special_macros = 1;

		} else if (!strcmp(argv[i], "-d")) {
			ls->flags &= ~KEEP_OUTPUT;
			print_defs = 1;

		} else if (!strcmp(argv[i], "-e")) {
			ls->flags &= ~KEEP_OUTPUT;
			print_asserts = 1;

		} else if (!strcmp(argv[i], "-zI")) {
			with_std_incpath = 0;

		} else if (!strcmp(argv[i], "-I") || !strcmp(argv[i], "-J")) {
			i ++;

		} else if (!strcmp(argv[i], "-o")) {
			if ((++ i) >= argc) {
				error(-1, "missing filename after -o");
				return 2;
			}

			if (emit_output) {
				error(-1, "multiple -o specificed");
			} else {
				if (argv[i][0] == '-' && argv[i][1] == 0) {
					emit_output = ls->output = stdout;
				} else {
					ls->output = fopen(argv[i], "w");
					if (!ls->output) {
						error(-1, "failed to open for writing: %s", argv[i]);
						return 2;
					}
					emit_output = ls->output;
				}
			}

		} else if (!strcmp(argv[i], "-v")) {
			print_version = 1;

		} else if (!strcmp(argv[i], "-notice")) {
			print_version = 2;

		} else if (argv[i][1] != 'I' && argv[i][1] != 'J'
				&& argv[i][1] != 'D' && argv[i][1] != 'U'
				&& argv[i][1] != 'A' && argv[i][1] != 'B') {
			warning(-1, "unknown option '%s'", argv[i]);
                }

	} else {
		if (filename != 0) {
			error(-1, "spurious filename '%s'", argv[i]);
			return 2;
		}
		filename = argv[i];
	}

	init_tables(ls->flags & HANDLE_ASSERTIONS);
	init_include_path(0);
	if (filename) {
#ifdef UCPP_MMAP
		FILE *f = fopen_mmap_file(filename);

		ls->input = 0;
		if (f) set_input_file(ls, f);
#else
		ls->input = fopen(filename, "r");
#endif
		if (!ls->input) {
			error(-1, "file '%s' not found", filename);
			return 1;
		}
#ifdef NO_LIBC_BUF
		setbuf(ls->input, 0);
#endif
		set_init_filename(filename, 1);
	} else {
		ls->input = stdin;
		set_init_filename("<stdin>", 0);
	}

	for (i = 1; i < argc; i ++)
		if (argv[i][0] == '-' && argv[i][1] == 'I')
			add_incpath(argv[i][2] ? argv[i] + 2 : argv[i + 1]);
	if (system_macros) for (i = 0; system_macros_def[i]; i ++)
		ret = ret || define_macro(ls, system_macros_def[i]);
	for (i = 1; i < argc; i ++)
		if (argv[i][0] == '-' && argv[i][1] == 'D')
			ret = ret || define_macro(ls, argv[i] + 2);
	for (i = 1; i < argc; i ++)
		if (argv[i][0] == '-' && argv[i][1] == 'U')
			ret = ret || undef_macro(ls, argv[i] + 2);
	if (ls->flags & HANDLE_ASSERTIONS) {
		if (standard_assertions)
			for (i = 0; system_assertions_def[i]; i ++)
				make_assertion(system_assertions_def[i]);
		for (i = 1; i < argc; i ++)
			if (argv[i][0] == '-' && argv[i][1] == 'A')
				ret = ret || make_assertion(argv[i] + 2);
		for (i = 1; i < argc; i ++)
			if (argv[i][0] == '-' && argv[i][1] == 'B')
				ret = ret || destroy_assertion(argv[i] + 2);
	} else {
		for (i = 1; i < argc; i ++)
			if (argv[i][0] == '-' && (argv[i][1] == 'A' || argv[i][1] == 'B'))
				warning(-1, "assertions disabled");
	}
	if (with_std_incpath) {
		for (i = 0; include_path_std[i]; i ++)
			add_incpath(include_path_std[i]);
	}
	for (i = 1; i < argc; i ++)
		if (argv[i][0] == '-' && argv[i][1] == 'J')
			add_incpath(argv[i][2] ? argv[i] + 2 : argv[i + 1]);

	if (print_version) {
		if (2 == print_version) {
			print_copyright();
		} else {
			ucpp_version();
		}
		return 1;
	}
	if (print_defs) {
		print_defines();
		emit_defines = 1;
	}
	if (print_asserts && (ls->flags & HANDLE_ASSERTIONS)) {
		print_assertions();
		emit_assertions = 1;
	}
	return ret;
}


int
main(int argc, char *argv[])
{
	struct lexer_state ls;
	int r, fr = 0;

	init_cpp();
	if ((r = parse_opt(argc, argv, &ls)) != 0) {
		if (r == 2) usage(argv[0]);
		return EXIT_FAILURE;
	}
	enter_file(&ls, ls.flags);
	while ((r = cpp(&ls)) < CPPERR_EOF) fr = fr || (r > 0);
	fr = fr || check_cpp_errors(&ls);
	free_lexer_state(&ls);
	wipeout();
#ifdef MEM_DEBUG
	report_leaks();
#endif
	return (fr ? EXIT_FAILURE : EXIT_SUCCESS);
}

