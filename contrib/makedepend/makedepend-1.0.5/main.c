/*

Copyright (c) 1993, 1994, 1998 The Open Group
Copyright (c) 2012-2018, Adam Young.

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
THE OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of The Open Group shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from The Open Group.

Modifications:

     =              trailing '=' allowed on -D and -I.

     -i[=], -d[=]   alternative switches.

     -Y@<env>       import system-includes from specified environment variable.

     -alias         include=alias-include, include file aliasing.

     -Mxxx          gcc style switches, where
	
        -MM		    omit system headers.
        -MG		    treated missing header as generated.
        -MT		    target override.
        -MC		    configuration profile.

*/

#include "def.h"
#ifdef hpux
#define sigvec sigvector
#endif /* hpux */

#ifdef X_POSIX_C_SOURCE
#define _POSIX_C_SOURCE X_POSIX_C_SOURCE
#include <signal.h>
#undef _POSIX_C_SOURCE
#else
#if defined(X_NOT_POSIX) || defined(_POSIX_SOURCE)
#include <signal.h>
#else
#define _POSIX_SOURCE
#include <signal.h>
#undef _POSIX_SOURCE
#endif
#endif

#include <stdarg.h>

#ifdef __sun
# include <sys/utsname.h>
#endif

#ifdef DEBUG
int	_debugmask;
#endif

/* #define DEBUG_DUMP */
#ifdef DEBUG_DUMP
#define DBG_PRINT(file, fmt, args)   fprintf(file, fmt, args)
#else
#define DBG_PRINT(file, fmt, args)   /* empty */
#endif

#define DASH_INC_PRE    "#include \""
#define DASH_INC_POST   "\""

const char *ProgramName;

const char * const directives[] = {
	"if",
	"ifdef",
	"ifndef",
	"else",
	"endif",
	"define",
	"undef",
	"include",
	"line",
	"pragma",
	"error",
	"ident",
	"sccs",
	"elif",
	"eject",
	"warning",
	"include_next",
	NULL
};

#include "imakemdep.h"

struct	inclist inclist[ MAXFILES ] = {0},
		*inclistp = inclist,
		*inclistnext = inclist,
		maininclist;

static char	*filelist[ MAXFILES ] = {0};
const char	*includedirs[ MAXDIRS + 1 ] = {0},
		**includedirsnext = includedirs;
char		*notdotdot[ MAXDIRS ] = {0};
static int	cmdinc_count = 0;
static char	*cmdinc_list[ 2 * MAXINCFILES ] = {0};
const char	*objprefix = "";
const char	*objsuffix = OBJSUFFIX;
static const char	*startat = "# DO NOT DELETE";
int		width = 78;
static boolean	append = FALSE;
boolean		printed = FALSE;
boolean		verbose = FALSE;
boolean		show_where_not = FALSE;
/* Warn on multiple includes of same file */
boolean 	warn_multiple = FALSE;

/*APY*/
struct aliaslist includealias[ MAXDIRS + 1 ] = {0};
const char **systemdirs = NULL;
const char *mtarget = NULL;
int mflags = 0;

static void profile(const char *name);

static void setfile_cmdinc(struct filepointer *filep, long count, char **list);
static void redirect(const char *line, const char *makefile);

static void help(void);
static void copyright(void);

static
#if defined(RETSIGTYPE)
RETSIGTYPE
#elif defined(SIGNALRETURNSINT)
int
#else
void
#endif
sigcatch(int sig)
{
	fflush (stdout);
	fatalerr ("got signal %d\n", sig);
	/*NOT REACHED*/
#if defined(SIGNALRETURNSINT)
	return 0;
#endif	
}

#if defined(USG) || (defined(i386) && defined(SYSV)) || defined(WIN32) || defined(Lynx_22) || defined(__CYGWIN__)
#define USGISH
#endif

#ifndef USGISH
#ifdef X_NOT_POSIX
#define sigaction sigvec
#define sa_handler sv_handler
#define sa_mask sv_mask
#define sa_flags sv_flags
#endif
static struct sigaction sig_act;
#endif /* USGISH */

#ifndef USING_AUTOCONF
# if !defined(USGISH) && !defined(_SEQUENT_) && !defined(MINIX)
#  define HAVE_FCHMOD	1
# endif
#endif

/*APY, see profile() */
static const char *x_defincdir = NULL;
static const char **x_incp = NULL;
static struct aliaslist *x_aliascp = NULL;


int
main(int argc, char *argv[])
{
	char **fp = filelist;
	const char **incp = includedirs;
	char *p;
	struct inclist *ip;
	char *makefile = NULL;
        struct filepointer *filecontent;
	const struct symtab *psymp = predefs;
	const char *endmarker = NULL;
	const char *defincdir = NULL;
	char **undeflist = NULL;
	struct aliaslist *aliascp = includealias; /*APY*/
	int numundefs = 0, i;

	ProgramName = argv[0];

	while (psymp->s_name)
	{
	    define2(psymp->s_name, psymp->s_value, &maininclist);
	    psymp++;
	}
#ifdef __sun
	/* Solaris predefined values that are computed, not hardcoded */
	{
	    struct utsname name;

	    if (uname(&name) >= 0) {
		char osrevdef[SYS_NMLN + SYS_NMLN + 5];

		snprintf(osrevdef, sizeof(osrevdef), "__%s_%s", name.sysname, name.release);
		for (p = osrevdef; *p != '\0'; p++) {
		    if (!isalnum(*p)) {
			*p = '_';
		    }
		}
		define2(osrevdef, "1", &maininclist);
	    }
	}
#endif

	if (argc == 2 && argv[1][0] == '@') {
		struct stat ast;
		int afd;
		char *args;
		char **nargv;
		int nargc;
		char quotechar = '\0';

		nargc = 1;
		if ((afd = open(argv[1]+1, O_RDONLY)) < 0)
			fatalerr("cannot open \"%s\"\n", argv[1]+1);
		fstat(afd, &ast);
		args = (char *)malloc(ast.st_size + 1);
		if ((ast.st_size = read(afd, args, ast.st_size)) < 0)
			fatalerr("failed to read %s\n", argv[1]+1);
		args[ast.st_size] = '\0';
		close(afd);
		for (p = args; *p; p++) {
			if (quotechar) {
				if (quotechar == '\\' ||
						(*p == quotechar && p[-1] != '\\'))
				quotechar = '\0';
				continue;
			}
			switch (*p) {
			case '\\':
			case '"':
			case '\'':
				quotechar = *p;
				break;
			case ' ':
			case '\n':
		    	*p = '\0';
				if (p > args && p[-1])
					nargc++;
				break;
			}
		}
		if (p[-1])
			nargc++;
		nargv = (char **)malloc(nargc * sizeof(char *));
		nargv[0] = argv[0];
		argc = 1;
		for (p = args; argc < nargc; p += strlen(p) + 1)
			if (*p) nargv[argc++] = p;
				argv = nargv;
	}

	for(argc--, argv++; argc; argc--, argv++) {
	    	/* if looking for endmarker then check before parsing */
		if (endmarker && strcmp (endmarker, *argv) == 0) {
			endmarker = NULL;
			continue;
		}
		if (**argv != '-') {
			/* treat +thing as an option for C++ */
			if (endmarker && **argv == '+')
				continue;
			*fp++ = argv[0];
			continue;
		}
		switch(argv[0][1]) {
		case '-':
			if (0 == strcmp(argv[0], "--help")) {
				help(); /*APY*/
			} else if (0 == strcmp(argv[0], "--copyright")) {
				copyright(); /*APY*/
			}
			endmarker = &argv[0][2];
			if (endmarker[0] == '\0') endmarker = "--";
			break;
		case 'd':			/* -d<name>[=<value] */
		case 'D':			/* -D<name>[=<value] */
			if (argv[0][2] == '\0') {
				if (argc < 2)
					fatalerr("Missing argument for -D\n");
				argv++;
				argc--;
			}
			for (p=argv[0] + 2; *p ; p++)
				if (*p == '=') {
					*p = ' ';
					break;
				}
			define(argv[0] + 2, &maininclist);
			break;
		case 'I': {			/* -I[=]<include> */
				const char *include = argv[0]+2;

				if ('=' == *include) ++include;
				if (!*include) {
					if (argc < 2)
						fatalerr("Missing argument for -I\n");
					include = *(++argv);
					--argc;
				}
				if (incp >= includedirs + MAXDIRS)
					fatalerr("too many -I flags.\n");
				*incp++ = include;
			}
			break;
		case 'U':
			/* Undef's override all -D's so save them up */
			numundefs++;
			if (numundefs == 1)
			    undeflist = (char **)malloc(sizeof(char *));
			else
			    undeflist = (char **)realloc(undeflist, numundefs * sizeof(char *));
			if (argv[0][2] == '\0') {
				if (argc < 2)
					fatalerr("Missing argument for -U\n");
				argv++;
				argc--;
			}
			undeflist[numundefs - 1] = argv[0] + 2;
			break;
		case 'Y':
			defincdir = argv[0]+2;
			break;
		case 'M': /*APY*/
			for (p = argv[0] + 1; p && *++p;) {
				char c;
				switch (c = *p) {
				case 'M': /* -MM */
					mflags |= MFLAG_NOSYSTEM;
					break;
				case 'G': /* -MG */
					//  In conjunction with an option such as -M requesting dependency generation, -MG assumes missing header files are
					//  generated files and adds them to the dependency list without raising an error. The dependency filename is taken
					//  directly from the #include directive without prepending any path. -MG also suppresses preprocessed output, as a
					//  missing header file renders this useless.
					//
					//	This feature is used in automatic updating of makefiles.
					//
					mflags |= MFLAG_GENERATED;
					break;
				case 'P': /* -MP */
					//  This option instructs CPP to add a phony target for each dependency other than the main file, causing each to
					//  depend on nothing. These dummy rules work around errors make gives if you remove header files without updating
					//  the Makefile to match.
					//
					//  This is typical output:
					//
					//	test.o: test.c test.h
					//
					mflags |= MFLAG_PHONY;
					break;
				case 'F': /* -MF <filename> */
					//  When used with -M or -MM, specifies a file to write the dependencies to. If no -MF switch is given the
					//  preprocessor sends the rules to the same place it would have sent preprocessed output.
					//
					//  When used with the driver options -MD or -MMD, -MF overrides the default dependency output file.
					//
					makefile = ++p;
					if (!*makefile) {
						makefile = *(++argv);
						--argc;
					}
					p = NULL;
					break;
				case 'Q': /* -MQ <target> */
					//  Same as -MT, but it quotes any characters which are special to Make.
					//
					//  For example -MQ '$(objpfx)foo.o' gives
					//
					//	$$(objpfx)foo.o: foo.c
					//
					mflags |= MFLAG_QUOTE; //TODO
					/*FALLTHRU*/
				case 'T': /* -MT <target> */ {
					//  Change the target of the rule emitted by dependency generation. By default CPP takes the name of the main input
					//  file, deletes any directory components and any file suffix such as ‘.c’, and appends the platform's usual object
					//  suffix. The result is the target.
					//
					//  An -MT option will set the target to be exactly the string you specify. If you want multiple targets, you can
					//  specify them as a single argument to -MT, or use multiple -MT options.
					//
					//  For example, -MT '$(objpfx)foo.o' might give
					//
					//	$(objpfx)foo.o: foo.c
					//
						const char *target = ++p;

						mflags |= MFLAG_TARGET;
						if (!*target) {
							target = *(++argv);
							--argc;
						}
						if (!mtarget) {
							mtarget = strdup(target);
						} else {
							const size_t oldlen = strlen(mtarget),
								newlen = strlen(target);
							char *xtarget;

							if (NULL == (xtarget = realloc((void *)mtarget, oldlen + newlen + 2))) {
								fatalerr("out of memory at -M%c <target>\n", c);
							}
							xtarget[oldlen] = ' ';
							memcpy(xtarget + oldlen + 1, (const char *)target, newlen + 1);
							mtarget = xtarget;
						}
						p = NULL;
					}
					break;
				case 'C': /* -MC <profile> */ {
						const char *name = ++p;

						if (!*name) {
							name = *(++argv);
							--argc;
						}
						x_incp = incp, x_defincdir = defincdir, x_aliascp = aliascp;
						profile(name);
						incp = x_incp, defincdir = x_defincdir, aliascp = x_aliascp;
					}
					p = NULL;
					break;
				default:
					warning("ignoring option -M%c\n", *p);
					break;
				}
			}
			break;

		/* do not use if endmarker processing */
		case 'a':			/* -alias <include>=<alias> */
			if (0 == strcmp(argv[0]+1, "alias")) {
				const char *name, *alias = NULL;

				name = argv[0]+6;
				if (!*name) {
					name = *(++argv);
					--argc;
				}
				for (p = (char *)name; *p; ++p)
					if ('=' == *p) {
					        *p++ = 0;
					        if (*p) alias = p;
						break;
					}
				if (*name && alias) {
					if (aliascp >= includealias + MAXDIRS)
						fatalerr("too many -alias flags.\n");
					aliascp->name = strdup(name);
					aliascp->alias = strdup(alias);
					++aliascp;
				} else {
					fatalerr("-alias should be of the form name=alias\n");
				}
			} else {
				if (endmarker) break;
				append = TRUE;
			}
			break;
		case 'w':
			if (endmarker) break;
			if (argv[0][2] == '\0') {
				if (argc < 2)
					fatalerr("Missing argument for -w\n");
				argv++;
				argc--;
				width = atoi(argv[0]);
			} else
				width = atoi(argv[0]+2);
			break;
		case 'o':
			if (endmarker) break;
			if (argv[0][2] == '\0') {
				if (argc < 2)
					fatalerr("Missing argument for -o\n");
				argv++;
				argc--;
				objsuffix = argv[0];
			} else
				objsuffix = argv[0]+2;
			break;
		case 'p':
			if (endmarker) break;
			if (argv[0][2] == '\0') {
				if (argc < 2)
					fatalerr("Missing argument for -p\n");
				argv++;
				argc--;
				objprefix = argv[0];
			} else
				objprefix = argv[0]+2;
			break;
		case 'v':
			if (endmarker) break;
			verbose = TRUE;
#ifdef DEBUG
			if (argv[0][2])
				_debugmask = atoi(argv[0]+2);
#endif
			break;
		case 's':
			if (endmarker) break;
			startat = argv[0]+2;
			if (*startat == '\0') {
				if (argc < 2)
					fatalerr("Missing argument for -s\n");
				startat = *(++argv);
				argc--;
			}
			if (*startat != '#')
				fatalerr("-s flag's value should start with '#'\n");
			break;
		case 'f':
			if (endmarker) break;
			makefile = argv[0]+2;
			if (*makefile == '\0') {
				if (argc < 2)
					fatalerr("Missing argument for -f\n");
				makefile = *(++argv);
				argc--;
			}
			break;

		case 'm':
			warn_multiple = TRUE;
			break;

		/* Ignore -O, -g so we can just pass ${CFLAGS} to makedepend */
		case 'O':
		case 'g':
			break;
		case 'i':
			if ('=' == argv[0][2]) {	/* -i=<include> */
				const char *include = argv[0]+2;

				if ('=' == *include) ++include;
				if (!*include) {
					if (argc<2)
						fatalerr("option -i is a missing its parameter\n");
					include = *(++argv);
					--argc;
				}
				if (incp >= includedirs + MAXDIRS)
					fatalerr("too many -I flags.\n");
				*incp++ = include;
				break;
										
			} else if (strcmp(&argv[0][1], "include") == 0) { /* -include <include> */
				char *buf;
				if (argc<2)
					fatalerr("option -include is a missing its parameter\n");
				if (cmdinc_count >= MAXINCFILES)
					fatalerr("Too many -include flags.\n");
				argc--;
				argv++;
				buf = malloc(strlen(DASH_INC_PRE) + strlen(argv[0]) + strlen(DASH_INC_POST) + 1);
				if(!buf)
					fatalerr("out of memory at -include string\n");
				cmdinc_list[2 * cmdinc_count + 0] = argv[0];
				cmdinc_list[2 * cmdinc_count + 1] = buf;
				cmdinc_count++;
				break;
			}
			/* intentional fall through */
		default:
			if (0 == strcmp(argv[0], "-?")) {
				help();	/*APY*/
			}
			if (endmarker) break;
	/*		fatalerr("unknown opt = %s\n", argv[0]); */
			warning("ignoring option %s\n", argv[0]);
		}
	}
	/* Now do the undefs from the command line */
	for (i = 0; i < numundefs; i++)
	    undefine(undeflist[i], &maininclist);
	if (numundefs > 0)
	    free(undeflist);

#if defined(WIN32)
	systemdirs = incp; /* mark beginning of system directories */
#endif

	if (!defincdir) {
#ifdef PREINCDIR
	    if (incp >= includedirs + MAXDIRS)
			fatalerr("Too many -I flags.\n");
	    *incp++ = PREINCDIR;
#endif

#if defined(WIN32)
		{	const char *emxinc = getenv("C_INCLUDE_PATH");

			/* can have more than one component */
			if (emxinc) {
				char *beg, *end;
				beg= (char*)strdup(emxinc);
				for (;;) {
					end = (char*)strchr(beg,';');
					if (end) *end = 0;
					if (incp >= includedirs + MAXDIRS)
						fatalerr("Too many include dirs\n");
					*incp++ = beg;
					if (!end) break;
					beg = end+1;
				}
			}
	    }
#endif

#ifdef INCLUDEDIR
		if (incp >= includedirs + MAXDIRS)
			fatalerr("Too many -I flags.\n");
		*incp++ = INCLUDEDIR;
#endif

#ifdef EXTRAINCDIR
	    if (incp >= includedirs + MAXDIRS)
			fatalerr("Too many -I flags.\n");
	    *incp++ = EXTRAINCDIR;
#endif

#ifdef POSTINCDIR
		if (incp >= includedirs + MAXDIRS)
			fatalerr("Too many -I flags.\n");
		*incp++ = POSTINCDIR;
#endif
	} else if (*defincdir) {
#if defined(WIN32)
		if ('@' == *defincdir) { /*APY*/
			const char *envinc = getenv(defincdir + 1);

			if (envinc) { /* can have more than one component */
				char *beg, *end;
		
				beg = (char*) strdup(envinc);
				for (;;) {
					end = (char*)strchr(beg,';');
					if (end) *end = 0;
					if (*beg) { /* ignore empty specifications */
						if (incp >= includedirs + MAXDIRS)			
							fatalerr("Too many include dirs\n");
						*incp++ = beg;
					}
					if (!end) break;
					beg = end+1;
				}
			}
		} else {
			if (incp >= includedirs + MAXDIRS)
				fatalerr("too many -I flags.\n");
			*incp++ = defincdir;
		}
#else
	    if (incp >= includedirs + MAXDIRS)
			fatalerr("Too many -I flags.\n");
	    *incp++ = defincdir;
#endif
	}

#if defined(WIN32) /*APY*/
	/*
	 *  normalize slashes
	 */
	{	register char **pp, *p;

		for (pp = (char **)includedirs; *pp; ++pp) {
			p = *pp;
			do {
				if ((p = strchr(p, '\\')) != NULL) {
					*p = '/';
				}
			} while (p && *p);
		}
	}

	/*
	 *  terminate each directory with '/', allowing
	 *      simple -MM detection.
	 */
	if (MFLAG_NOSYSTEM & mflags) {
		register char **pp, *p;
		register int len;

		for (pp = (char **)systemdirs; *pp; ++pp) {
			len = strlen(*pp);
			if ( (*pp)[ len-1 ] != '/' ) {
				p = malloc(len+2);
				(void) strcpy(p, (const char *)*pp);
				p[ len ] = '/', p[ len+1 ] = '\0';
				debug(1, ("conv -Y%s\n", p));
				*pp = p;
			}
		}
	}
#endif	//WIN32

	redirect(startat, makefile);

	/*
	 * catch signals.
	 */
#ifdef USGISH
/*  should really reset SIGINT to SIG_IGN if it was.  */
#ifdef SIGHUP
	signal (SIGHUP, sigcatch);
#endif
	signal (SIGINT, sigcatch);
#ifdef SIGQUIT
	signal (SIGQUIT, sigcatch);
#endif
	signal (SIGILL, sigcatch);
#ifdef SIGBUS
	signal (SIGBUS, sigcatch);
#endif
	signal (SIGSEGV, sigcatch);
#ifdef SIGSYS
	signal (SIGSYS, sigcatch);
#endif
#else
	sig_act.sa_handler = sigcatch;
#if defined(_POSIX_SOURCE) || !defined(X_NOT_POSIX)
	sigemptyset(&sig_act.sa_mask);
	sigaddset(&sig_act.sa_mask, SIGINT);
	sigaddset(&sig_act.sa_mask, SIGQUIT);
#ifdef SIGBUS
	sigaddset(&sig_act.sa_mask, SIGBUS);
#endif
	sigaddset(&sig_act.sa_mask, SIGILL);
	sigaddset(&sig_act.sa_mask, SIGSEGV);
	sigaddset(&sig_act.sa_mask, SIGHUP);
	sigaddset(&sig_act.sa_mask, SIGPIPE);
#ifdef SIGSYS
	sigaddset(&sig_act.sa_mask, SIGSYS);
#endif
#else
	sig_act.sa_mask = ((1<<(SIGINT -1))
			   |(1<<(SIGQUIT-1))
#ifdef SIGBUS
			   |(1<<(SIGBUS-1))
#endif
			   |(1<<(SIGILL-1))
			   |(1<<(SIGSEGV-1))
			   |(1<<(SIGHUP-1))
			   |(1<<(SIGPIPE-1))
#ifdef SIGSYS
			   |(1<<(SIGSYS-1))
#endif
			   );
#endif /* _POSIX_SOURCE */
	sig_act.sa_flags = 0;
	sigaction(SIGHUP, &sig_act, (struct sigaction *)0);
	sigaction(SIGINT, &sig_act, (struct sigaction *)0);
	sigaction(SIGQUIT, &sig_act, (struct sigaction *)0);
	sigaction(SIGILL, &sig_act, (struct sigaction *)0);
#ifdef SIGBUS
	sigaction(SIGBUS, &sig_act, (struct sigaction *)0);
#endif
	sigaction(SIGSEGV, &sig_act, (struct sigaction *)0);
#ifdef SIGSYS
	sigaction(SIGSYS, &sig_act, (struct sigaction *)0);
#endif
#endif /* USGISH */

	/*
	 * now peruse through the list of files.
	 */
	for(fp=filelist; *fp; fp++) {
		DBG_PRINT(stderr,"file: %s\n",*fp);
		filecontent = getfile(*fp);
		setfile_cmdinc(filecontent, cmdinc_count, cmdinc_list);
		ip = newinclude(*fp, (char *)NULL);

		find_includes(filecontent, ip, ip, 0, FALSE);
		freefile(filecontent);
		recursive_pr_include(ip, ip->i_file, base_name(*fp));
		inc_clean();
	}
	if (printed)
		printf("\n");
	return 0;
}


static void /*APY*/
profile(const char *name)
{
	char buf[ BUFSIZ ];
	FILE *fdin;

	if (NULL == (fdin = fopen(name, "r"))) {
		fatalerr("cannot open \"%s\"\n", name);
	}

	while (fgets(buf, BUFSIZ, fdin)) {
		char *name, *value, *end;

		end = buf + strlen(buf); /* remove trailing white-space */
		while (end-- > buf) {
			if (strchr(" \t\r\n", (unsigned char)*end)) {
				*end = 0;
				continue;
			}
			break;
		}

		name = buf;	/* name */
		while (' ' == *name || '\t' == *name) {
			++name;
		}

		if (!*name || '#' == *name)
			continue;

		if ('-' == *name) {	/* -<switch> <value> */
			switch (name[1]) {
			case 'A': { 
					char *alias = strchr(name += 2, '=');

					if (*name && alias) {
						for (end = alias; end-- > name;) {
							if (' ' == *end || '\t' == *end) {
								*end = 0; /* trailing white-space */
								continue;
							}
							break;
						}
						*alias++ = 0;
						while (' ' == *alias || '\t' == *alias) {
							++alias; /* leading white-space */
						}
						if (*name && *alias) {
							if (x_aliascp >= includealias + MAXDIRS)
								fatalerr("profile: too many -A flags.\n");
							x_aliascp->name = strdup(name);
							x_aliascp->alias = strdup(alias);
							++x_aliascp;
						}
					}
				}
				break;
			case 'I':
				if (x_incp >= includedirs + MAXDIRS)
					fatalerr("profile: too many -I flags.\n");
				*x_incp++ = strdup(name + 2);
				break;
			case 'Y':
				if (x_defincdir)
					fatalerr("profile: multiple -Y flags.\n");
				x_defincdir = strdup(name + 2);
				break;
			default:
				warning("profile: ignoring <%s>\n", buf);
				break;
			}
			
		} else { /* name[=value] */
			end = name;
			while (isalnum((unsigned char)*end) || *end == '_')
				++end;
			value = end;
			while (' ' == *value || '\t' == *value) {
				++value;
			}

			if (!*value) { /* no-value */
				*end = 0;
				define2(name, "1", &maininclist);

			} else if ('=' == *value++) {
				while (' ' == *value || '\t' == *value) {
					++value;
				}
				*end = 0; /* name=value */
				define2(name, (*value ? value : "1"), &maininclist);

			} else {
				warning("profile: ignoring <%s>\n", buf);
			}
		}
	}
	fclose(fdin);
}

struct filepointer *
getfile(const char *file)
{
	int	fd;
	struct filepointer	*content;
	struct stat	st;

	content = (struct filepointer *)malloc(sizeof(struct filepointer));
	content->f_name = file;
	if ((fd = open(file, O_RDONLY)) < 0) {
		warning("cannot open \"%s\"\n", file);
		content->f_p = content->f_base = content->f_end = (char *)malloc(1);
		*content->f_p = '\0';
		return(content);
	}
	fstat(fd, &st);
	content->f_base = (char *)malloc(st.st_size+1);
	if (content->f_base == NULL)
		fatalerr("cannot allocate mem\n");
	if ((st.st_size = read(fd, content->f_base, st.st_size)) < 0)
		fatalerr("failed to read %s\n", file);
	close(fd);
	content->f_len = st.st_size+1;
	content->f_p = content->f_base;
	content->f_end = content->f_base + st.st_size;
	*content->f_end = '\0';
	content->f_line = 0;
	content->cmdinc_count = 0;
	content->cmdinc_list = NULL;
	content->cmdinc_line = 0;
	return(content);
}

void
setfile_cmdinc(struct filepointer* filep, long count, char** list)
{
	filep->cmdinc_count = count;
	filep->cmdinc_list = list;
	filep->cmdinc_line = 0;
}

void
freefile(struct filepointer *fp)
{
	free(fp->f_base);
	free(fp);
}

int
match(const char *str, const char * const *list)
{
	int	i;

	for (i=0; *list; i++, list++)
		if (strcmp(str, *list) == 0)
			return(i);
	return(-1);
}

/*
 * Get the next line.  We only return lines beginning with '#' since that
 * is all this program is ever interested in.
 */
char *getnextline(struct filepointer *filep)
{
	char	*p,	/* walking pointer */
		*eof,	/* end of file pointer */
		*bol;	/* beginning of line pointer */
	int	lineno;	/* line number */
//	boolean whitespace = FALSE;

	/*
	 * Fake the "-include" line files in form of #include to the
	 * start of each file.
	 */
	if (filep->cmdinc_line < filep->cmdinc_count) {
		char *inc = filep->cmdinc_list[2 * filep->cmdinc_line + 0];
		char *buf = filep->cmdinc_list[2 * filep->cmdinc_line + 1];
		filep->cmdinc_line++;
		sprintf(buf,"%s%s%s",DASH_INC_PRE,inc,DASH_INC_POST);
		DBG_PRINT(stderr,"%s\n",buf);
		return(buf);
	}

	p = filep->f_p;
	eof = filep->f_end;
	if (p >= eof)
		return((char *)NULL);
	lineno = filep->f_line;

	for (bol = p--; ++p < eof; ) {
		if ((bol == p) && ((*p == ' ') || (*p == '\t')))
		{
			/* Consume leading white-spaces for this line */
			while (((p+1) < eof) && ((*p == ' ') || (*p == '\t')))
			{
				p++;
				bol++;
			}
//			whitespace = TRUE;
		}

		if (*p == '/' && (p+1) < eof && *(p+1) == '*') {
			/* Consume C comments */
			*(p++) = ' ';
			*(p++) = ' ';
			while (p < eof && *p) {
				if (*p == '*' && (p+1) < eof && *(p+1) == '/') {
					*(p++) = ' ';
					*(p++) = ' ';
					break;
				}
				if (*p == '\n')
					lineno++;
				*(p++) = ' ';
			}
			--p;
		}
		else if (*p == '/' && (p+1) < eof && *(p+1) == '/') {
			/* Consume C++ comments */
			*(p++) = ' ';
			*(p++) = ' ';
			while (p < eof && *p) {
				if (*p == '\\' && (p+1) < eof &&
				    *(p+1) == '\n') {
					*(p++) = ' ';
					lineno++;
				}
				else if (*p == '?' && (p+3) < eof &&
					 *(p+1) == '?' &&
					 *(p+2) == '/' &&
					 *(p+3) == '\n') {
					*(p++) = ' ';
					*(p++) = ' ';
					*(p++) = ' ';
					lineno++;
				}
				else if (*p == '\n')
					break;	/* to process end of line */
				*(p++) = ' ';
			}
			--p;
		}
		else if (*p == '\\' && (p+1) < eof && *(p+1) == '\n') {
			/* Consume backslash line terminations */
			*(p++) = ' ';
			*p = ' ';
			lineno++;
		}
		else if (*p == '?' && (p+3) < eof &&
			 *(p+1) == '?' && *(p+2) == '/' && *(p+3) == '\n') {
			/* Consume trigraph'ed backslash line terminations */
			*(p++) = ' ';
			*(p++) = ' ';
			*(p++) = ' ';
			*p = ' ';
			lineno++;
		}
		else if (*p == '\n') {
			lineno++;
			if (*bol == '#') {
				char *cp;

				*(p++) = '\0';
				/* punt lines with just # (yacc generated) */
				for (cp = bol+1;
				     *cp && (*cp == ' ' || *cp == '\t'); cp++);
				if (*cp) goto done;
				--p;
			}
			bol = p+1;
//			whitespace = FALSE;
		}
	}
	if (*bol != '#')
		bol = NULL;
done:
	filep->f_p = p;
	filep->f_line = lineno;
#ifdef DEBUG_DUMP
	if (bol)
		DBG_PRINT(stderr,"%s\n",bol);
#endif
	return(bol);
}

/*
 * Strip the file name down to what we want to see in the Makefile.
 * It will have objprefix and objsuffix around it.
 */
char *base_name(const char *in_file)
{
	char	*p;
	char	*file = strdup(in_file);
	for(p=file+strlen(file); p>file && *p != '.'; p--) ;

	if (*p == '.')
		*p = '\0';
	return(file);
}

#ifdef USING_AUTOCONF
# ifndef HAVE_RENAME
#  define NEED_RENAME
# endif
#else /* Imake configured, check known OS'es without rename() */
# if defined(USG) && !defined(CRAY) && !defined(SVR4) && !defined(__UNIXOS2__) && !defined(clipper) && !defined(__clipper__)
#  define NEED_RENAME
# endif
#endif

#ifdef NEED_RENAME
int rename (char *from, char *to)
{
	(void) unlink (to);
	if (link (from, to) == 0) {
		unlink (from);
		return 0;
	} else {
		return -1;
	}
}
#endif /* NEED_RENAME */

static void
redirect(const char *line, const char *makefile)
{
	struct stat st;
	FILE *fdin = NULL, *fdout;
	char backup[ BUFSIZ ], buf[ BUFSIZ ];
	boolean	found = FALSE;
	size_t	len;

	/*
	 * if makefile is "-" then let it pour onto stdout.
	 */
	if (makefile && *makefile == '-' && *(makefile+1) == '\0') {
		puts(line);
		return;
	}

	/*
	 * use a default if makefile is not specified.
	 */
	if (!makefile) {
		if (stat("Makefile", &st) == 0)
			makefile = "Makefile";
		else if (stat("makefile", &st) == 0)
			makefile = "makefile";
		else
			fatalerr("[mM]akefile is not present\n");
	} else {
		if (stat(makefile, &st) != 0) {
			if (MFLAG_GENERATED & mflags) {
				if (verbose)
					warning("Creating dependencies file \"%s\"", makefile);
				st.st_mode = 0666;
				goto create; /* APY, autogenerate */
			}
			fatalerr("\"%s\" is not present\n", makefile);
		}
	}

	snprintf(backup, sizeof(backup), "%s.bak", makefile);
	chmod(backup, 0666);
	/* rename() won't work on WIN32, CYGWIN, or CIFS if src file is open (or exists) */
#if defined(WIN32)
	unlink(backup);
#endif
	if (rename(makefile, backup) < 0)
		fatalerr("cannot rename %s to %s\n", makefile, backup);
	if ((fdin = fopen(backup, "r")) == NULL) {
		if (rename(backup, makefile) < 0)
			warning("renamed %s to %s, but can't move it back\n", makefile, backup);
		fatalerr("cannot open \"%s\"\n", backup);
	}
create:;
	if ((fdout = freopen(makefile, "w", stdout)) == NULL)
		fatalerr("cannot open \"%s\"\n", backup);
	len = strlen(line);
	if (fdin) {
		while (!found && fgets(buf, BUFSIZ, fdin)) {
			if (*buf == '#' && strncmp(line, buf, len) == 0)
				found = TRUE;
			fputs(buf, fdout);
		}
	}
	if (!found) {
		if (verbose)
			warning("Adding new delimiting line \"%s\" and dependencies...\n",line);
		puts(line); /* same as fputs(fdout); but with newline */
	} else if (append) {
		while (fgets(buf, BUFSIZ, fdin)) {
			fputs(buf, fdout);
		}
	}
	fflush(fdout);
#ifndef HAVE_FCHMOD
	chmod(makefile, st.st_mode);
#else
	fchmod(fileno(fdout), st.st_mode);
#endif /* HAVE_FCHMOD */
}

void
fatalerr(const char *msg, ...)
{
	va_list args;
	fprintf(stderr, "%s: error:  ", ProgramName);
	va_start(args, msg);
	vfprintf(stderr, msg, args);
	va_end(args);
	exit (1);
}

void
warning(const char *msg, ...)
{
	va_list args;
	fprintf(stderr, "%s: warning:  ", ProgramName);
	va_start(args, msg);
	vfprintf(stderr, msg, args);
	va_end(args);
}

void
warning1(const char *msg, ...)
{
	va_list args;
	va_start(args, msg);
	vfprintf(stderr, msg, args);
	va_end(args);
}

static void /*APY*/
help(void)
{
	static const char *helptext[] = {
		"makedepend-1.0.5 (modified)",
	    "",
	    "NAME",
	    "       makedepend - create dependencies in makefiles",
	    "",
	    "SYNOPSIS",
	    "       makedepend [ --copyright ] [ --help ]",
	    "",
	    "       makedepend  [ -Dname=def ] [ -Dname ] [ -Iincludedir ] [ -Yincludedir ]",
	    "       [ -a ] [ -fmakefile ] [ -include file ] [ -oobjsuffix ] [ -pobjprefix ]",
	    "       [ -sstring  ] [ -wwidth ] [ -v ] [ -m ] [ -M[M] ] [ -M[G] ]",
	    "       [ -- otheroptions -- ] source-file ...",
	    "",
	    "DESCRIPTION",
	    "",
	    "       The makedepend program reads each sourcefile in sequence and parses  it",
	    "       like  a  C-preprocessor,  processing  all  #include,  #define,  #undef,",
	    "       #ifdef, #ifndef, #endif, #if, #elif and #else directives so that it can",
	    "       correctly  tell  which #include, directives would be used in a compila-",
	    "       tion.  Any  #include,  directives  can  reference  files  having  other",
	    "       #include directives, and parsing will occur in these files as well.",
	    "",
	    "       Every  file that a sourcefile includes, directly or indirectly, is what",
	    "       makedepend calls a dependency.  These dependencies are then written  to",
	    "       a makefile in such a way that make(1) will know which object files must",
	    "       be recompiled when a dependency has changed.",
	    "",
	    "       By default, makedepend places its output in the file named makefile  if",
	    "       it  exists, otherwise Makefile.  An alternate makefile may be specified",
	    "       with the -f option.  It first searches the makefile for the line",
	    "",
	    "           # DO NOT DELETE THIS LINE -- make depend depends on it.",
	    "",
	    "       or one provided with the -s option, as a delimiter for  the  dependency",
	    "       output.   If  it  finds it, it will delete everything following this to",
	    "       the end of the makefile and put the output  after  this  line.   If  it",
	    "       doesn't  find  it, the program will append the string to the end of the",
	    "       makefile and place the output  following  that.   For  each  sourcefile",
	    "       appearing on the command line, makedepend puts lines in the makefile of",
	    "       the form",
	    "",
	    "            sourcefile.o: dfile ...",
	    "",
	    "       Where sourcefile.o is the name from the command line  with  its  suffix",
	    "       replaced  with  ``.o'',  and  dfile  is  a  dependency  discovered in a",
	    "       #include directive while parsing sourcefile or  one  of  the  files  it",
	    "       included.",
	    "",
	    "",
	    "EXAMPLE",
	    "",
	    "       Normally,  makedepend  will be used in a makefile target so that typing",
	    "       ``make depend'' will bring the dependencies up to date  for  the  make-",
	    "       file.  For example,",
	    "           SRCS = file1.c file2.c ...",
	    "           CFLAGS = -O -DHACK -I../foobar -xyz",
	    "           depend:",
	    "                   makedepend -- $(CFLAGS) -- $(SRCS)",
	    "",
	    "",
	    "OPTIONS",
	    "",
	    "       The  program will ignore any option that it does not understand so that",
	    "       you may use the same arguments that you would for cc(1).",
	    "",
	    "       -Dname=def or -Dname",
	    "            Define.  This places a definition for name in makedepend's  symbol",
	    "            table.  Without =def the symbol becomes defined as ``1''.",
	    "",
	    "       -Iincludedir",
	    "            Include  directory.   This  option  tells  makedepend  to  prepend",
	    "            includedir to its list of directories to search when it encounters",
	    "            a  #include  directive.   By default, makedepend only searches the",
	    "            standard include directories (usually /usr/include and possibly  a",
	    "            compiler-dependent directory).",
	    "",
	    "       -Yincludedir",
	    "            Replace  all  of  the standard include directories with the single",
	    "            specified include directory; you can omit the includedir to simply",
	    "            prevent searching the standard include directories.",
	    "",
	    "            (extension) If includedir is prefixed with @@, the value shall be",
	    "            taken to be the name of an environment variable, of the form",
	    "            '@@name', which will be imported and treated as a ';' seperated",
	    "            list of directories.",
	    "",
	    "       -a   Append  the dependencies to the end of the file instead of replac-",
	    "            ing them.",
	    "",
	    "       -fmakefile",
	    "            Filename.  This allows you to specify  an  alternate  makefile  in",
	    "            which  makedepend  can  place its output.  Specifying ``-'' as the",
	    "            file name (i.e., -f-) sends the output to standard output  instead",
	    "            of modifying an existing file.",
	    "",
	    "       -include file",
	    "            Process file as input, and include all the resulting output before",
	    "            processing the regular input file. This has the same affect as  if",
	    "            the specified file is an include statement that appears before the",
	    "            very first line of the regular input file.",
	    "",
	    "       -oobjsuffix",
	    "            Object file suffix.  Some systems may have object files whose suf-",
	    "            fix  is  something  other  than ``.o''.  This option allows you to",
	    "            specify another suffix, such as ``.b'' with -o.b or ``:obj''  with",
	    "            -o:obj and so forth.",
	    "",
	    "       -pobjprefix",
	    "            Object  file  prefix.   The prefix is prepended to the name of the",
	    "            object file. This is usually used to designate a different  direc-",
	    "            tory for the object file.  The default is the empty string.",
	    "",
	    "       -sstring",
	    "            Starting  string  delimiter.  This option permits you to specify a",
	    "            different string for makedepend to look for in the makefile.",
	    "",
	    "       -wwidth",
	    "            Line width.  Normally, makedepend will ensure  that  every  output",
	    "            line  that  it  writes will be no wider than 78 characters for the",
	    "            sake of readability.  This  option  enables  you  to  change  this",
	    "            width.",
	    "",
	    "       -v   Verbose operation.  This option causes makedepend to emit the list",
	    "            of files included by each input file.",
	    "",
	    "       -m   Warn about multiple inclusion.  This option causes  makedepend  to",
	    "            produce  a  warning  if  any input file includes another file more",
	    "            than once.  In  previous  versions  of  makedepend  this  was  the",
	    "            default behavior; the default has been changed to better match the",
	    "            behavior of the C  compiler,  which  does  not  consider  multiple",
	    "            inclusion  to  be  an error.  This option is provided for backward",
	    "            compatibility, and to aid in debugging problems related to  multi-",
	    "            ple inclusion.",
	    "",
	    "       -M [-MG]        (extension)",
	    "",
	    "            -M   tell the preprocessor to output dependencies.",
	    "",
	    "            -MG  says to treat  missing  header  files  as  generated files and",
	    "                 assume they live  in the  same  directory  as  the source file.",
	    "                 It must be specified in addition to -M.",
	    "",
	    "       -MM [-MG]       (extension)",
	    "",
	    "            Like -M  but  the  output  mentions  only  the  user  header files",
	    "            included with '#include \"file\"'. System header files included  with",
	    "            '#include <file>' are  omitted, either the system defaults or ones",
	    "            specified by '-Y'.",
	    "",
	    "       -MT <target>    (extension)",
	    "",
	    "            Change the target of the rule emitted by dependency generation. By",
	    "            default 'makedepend' takes the name of the input file, including any",
	    "            path, deletes any file suffix such as '.c', and appends the",
	    "            platform's usual object suffix (-o) and optional (-p); the result",
	    "            is the target.",
	    "",
	    "            An -MT option will set the target to be exactly the string you",
	    "            specify. If you want multiple targets, you can specify them as a",
	    "            single argument to -MT, or use multiple -MT options.",
	    "",
	    "            For example, -MT '$(objpfx)foo.o' might give",
	    "",
	    "                $(objpfx)foo.o: foo.c",
	    "",
	    "       -MC <profile>   (extension)",
	    "",
	    "            Specifies the configuration profile to read.",
	    "",
	    "       -- options --",
	    "            If  makedepend  encounters  a  double  hyphen (--) in the argument",
	    "            list, then any unrecognized argument following it will be silently",
	    "            ignored; a second double hyphen terminates this special treatment.",
	    "            In this way, makedepend can be made to safely ignore esoteric com-",
	    "            piler  arguments  that  might  normally  be found in a CFLAGS make",
	    "            macro (see the EXAMPLE section above).  All options  that  makede-",
	    "            pend  recognizes and appear between the pair of double hyphens are",
	    "            processed normally.",
	    "",
	    "",
	    "ALGORITHM",
	    "",
	    "       The approach used in this program enables it to run an order of  magni-",
	    "       tude  faster  than any other ``dependency generator'' I have ever seen.",
	    "       Central to this performance are two assumptions: that  all  files  com-",
	    "       piled  by  a  single makefile will be compiled with roughly the same -I",
	    "       and -D options; and that most files in a single directory will  include",
	    "       largely the same files.",
	    "",
	    "       Given  these assumptions, makedepend expects to be called once for each",
	    "       makefile, with all source files that are  maintained  by  the  makefile",
	    "       appearing  on the command line.  It parses each source and include file",
	    "       exactly once, maintaining an internal symbol table for each.  Thus, the",
	    "       first file on the command line will take an amount of time proportional",
	    "       to the amount of time that a normal C preprocessor takes.  But on  sub-",
	    "       sequent  files,  if  it  encounters an include file that it has already",
	    "       parsed, it does not parse it again.",
	    "",
	    "       For example, imagine you are compiling two files, file1.c and  file2.c,",
	    "       they  each  include  the header file header.h, and the file header.h in",
	    "       turn includes the files def1.h and def2.h.  When you run the command",
	    "",
	    "           makedepend file1.c file2.c",
	    "",
	    "       makedepend will parse  file1.c  and  consequently,  header.h  and  then",
	    "       def1.h and def2.h.  It then decides that the dependencies for this file",
	    "       are",
	    "",
	    "           file1.o: header.h def1.h def2.h",
	    "",
	    "       But when the  program  parses  file2.c  and  discovers  that  it,  too,",
	    "       includes  header.h,  it  does  not  parse  the  file,  but  simply adds",
	    "       header.h, def1.h and def2.h to the list of dependencies for file2.o.",
	    "",
	    "",
	    "BUGS",
	    "",
	    "       makedepend parses, but does not currently evaluate,  the  SVR4  #predi-",
	    "       cate(token-list)  preprocessor  expression; such expressions are simply",
	    "       assumed to be true.  This may cause the wrong #include directives to be",
	    "       evaluated.",
	    "",
	    "       Imagine  you  are  parsing  two  files,  say  file1.c and file2.c, each",
	    "       includes the file def.h.  The list of files that def.h  includes  might",
	    "       truly  be  different  when def.h is included by file1.c than when it is",
	    "       included by file2.c.  But once makedepend arrives at a list  of  depen-",
	    "       dencies for a file, it is cast in concrete.",
	    "",
	    "",
	    "AUTHOR",
	    "",
	    "       Todd Brunhoff, Tektronix, Inc. and MIT Project Athena",
	    NULL};
	const char *text;
	unsigned i;

	for (i = 0; NULL != (text = helptext[i]); ++i) {
		printf("%s\n", text);
	}
	exit(3);
}


static void /*APY*/
copyright(void)
{
	static const char *copytext[] = {
	    "makedepend-1.0.5 (modified)",
	    "",
	    "",
	    "       Copyright (c) 1993, 1994, 1998 The Open Group",
	    "       Copyright (c) 2012 - 2018, Adam Young.",
	    "",
	    "       Permission to use, copy, modify, distribute, and sell this software and its",
	    "       documentation for any purpose is hereby granted without fee, provided that",
	    "       the above copyright notice appear in all copies and that both that",
	    "       copyright notice and this permission notice appear in supporting",
	    "       documentation.",
	    "",
	    "       The above copyright notice and this permission notice shall be included in",
	    "       all copies or substantial portions of the Software.",
	    "",
	    "       THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR",
	    "       IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,",
	    "       FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE",
	    "       THE OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN",
	    "       AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN",
	    "       CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.",
	    "",
	    "       Except as contained in this notice, the name of The Open Group shall not be",
	    "       used in advertising or otherwise to promote the sale, use or other dealings",
	    "       in this Software without prior written authorization from The Open Group.",
	    "",
	    NULL};
	const char *text;
	unsigned i;

	for (i = 0; NULL != (text = copytext[i]); ++i) {
		printf("%s\n", text);
	}
	exit(3);
}

