#include <edidentifier.h>
__CIDENT_RCSID(gr_crmain_c,"$Id: crmain.c,v 1.48 2014/10/22 02:33:28 ayoung Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: crmain.c,v 1.48 2014/10/22 02:33:28 ayoung Exp $
 * grunch command line.
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
#include <edcm.h>

#include "grunch.h"                             /* local definitions */
#include <libstr.h>                             /* str_...()/sxprintf() */
#include <libstr.h>

#if defined(WIN32)
#define  WINDOWS_MEAN_AND_LEAN
#undef   u_char
#include <windows.h>
#endif
#if defined(__APPLE__)
#include <libproc.h>                            /* proc_..() */
#include <mach-o/dyld.h>                        /* _NSGetExecutablePath() */
#endif

/*
 *  Basic system dependent definitions.
 */
#if defined(DJGPP)
#define CONVERTSLASHES
#define DIRCHR              '/'
#define EXEEXT              ".exe"
#define PATHCH              ";"

#elif defined(__OS2__) || \
	defined(__MSDOS__) || defined(MSDOS) || defined(WIN32)
#define CONVERTSLASHES
#define DIRCHR              '\\'
#define EXEEXT              ".exe"
#define PATHCH              ";"
#else
#define DIRCHR              '/'
#define PATHCH              ":"
#endif
#if defined(_MSC_VER) && (_MSC_VER >= 1400)
#define UNLINK(__x)         _unlink(__x)
#else
#define UNLINK(__x)         unlink(__x)
#endif

#define PATHLENGTH          1024
#define TMPFILE             "/tmp/chXXXXXX"

/*
 *  PATH to C preprocessor.
 */
#if !defined(CC_PATH)
    #if defined(MSDOS) || defined(__MSDOS__) || defined(__OS2__) || defined(__EMX__)
        #define CC_PATH     "$PATH";
    #elif defined(sun) || defined(__sun__)
        #define CC_PATH     "/usr/ccs/lib:/usr/libexec:/usr/local/lib:/lib:/usr/lib:$PATH"
    #else
        #define CC_PATH     "/usr/libexec:/usr/local/lib:/lib:/usr/lib:$PATH"
    #endif
#endif

#define CC_GRUNCH           "-DGRUNCH -DGRUNCH_VERSION=0x0311 -DGRIEF"
#define CC_COMPILETIME      "-D__COMPILETIME__"
#define CC_PROTOTYPES       "-D__PROTOTYPES__"

/*
 *  Preprocessor application.
 */
#if !defined(CC_PROG)
    #if defined(__EMX__)
        #define CC_NATIVE   "cpp.exe"
    #elif defined(DJGPP)
        #define CC_NATIVE   "gcc.exe -E -x c"
    #elif defined(MSDOS) || defined(__MSDOS__) || defined(WIN32)
        #define CC_NATIVE   "cl.exe -nologo -E"
        #define CC_NATIVENOECHO
    #else
        #define CC_NATIVE   "cpp"
    #endif
    #define CC_PROG         "grcpp"
#endif

/*
 *  Preprocessor arguments.
 */
#if !defined(CC_OSARG)
    #if defined(MSDOS) || defined(__MSDOS__)
      #if defined(__MINGW32__)
        #define CC_OSARG    "-DMSDOS -DMINGW32"
      #elif defined(WIN32)
        #define CC_OSARG    "-DMSDOS -DWIN32"
      #else
        #define CC_OSARG    "-DMSDOS"
      #endif

    #elif defined(DJGPP)
        #define CC_OSARG    "-DMSDOS -DDJGPP"

    #elif defined(__aix__) || defined(AIX) || defined(_AIX)
        #define CC_OSARG    "-DUNIX -DAIX -D_AIX"

    #elif defined(sun) || defined(__sun__)
        #if defined(solaris) || defined(__SOLARIS__)
          #define CC_OSARG  "-DUNIX -DSUN -DSOLARIS -DSYSV"
        #else
          #define CC_OSARG  "-DUNIX -DSUN -DSUNOS"
        #endif

    #elif defined(__hpux)
        /*
         *  __hpux and __unix are defined on all HP/UX implementations (but not VMS)
         *      ref: HP C/HP-UX Programmer's Guide
         */
        #define CC_OSARG    "-DUNIX -DHPUX"

    #elif defined(linux) || defined(__linux__)
        #define CC_OSARG    "-DUNIX -DLINUX"

    #elif defined(__CYGWIN__)
        #if defined(__CYGWIN32__)
          #define CC_OSARG  "-DUNIX -DCYGWIN"
        #else
          #error crmain: Unknown CYGWIN environment ...
        #endif

    #elif defined(__FreeBSD__)
        #define CC_OSARG    "-DUNIX -DFREEBSD"

    #elif defined(BSD)
        #define CC_OSARG    "-DUNIX -DBSD"

    #elif defined(vms) || defined(VMS)
        #define CC_OSARG    "-DVMS"

    #elif defined(__EMX__) || defined(__OS2__)
        #define CC_OSARG    "-DOS2"

    #else
        #error crmain: Unknown target environment ...
    #endif
#endif


/*
 *  Prototypes
 */
static void             arg_push(const char *args[], unsigned count, const char *prefix, const char *val);
static unsigned         arg_sizeof(const char *args[], unsigned count);
static unsigned         arg_export(const char *args[], unsigned count, char *buffer, unsigned offset);
static void             arg_release(const char *args[], unsigned count);

static int              switch_process(int argc, char *argv[]);
static void             switch_map(const char *argc);

static void             resolve_self(const char *name);
static int              resolve_path(char *dst, int dstlen, const char *pp, const char *name, const char *label);
static int              compile_file(const char *file);
static int              verify_object(const char *srcname, struct stat *srcsb, const char *objname, struct stat *objsb);

static const char *     expand_var(const char *var, char *buf, int buflen);

static void             usage(void);


/*
 *  Globals
 */
static const char *     myname = "GRUNCH";
const char *            x_progname = "grunch";

int                     xf_grunch = 1;          /* Crunch level (1=extended, 2=compat) */
int                     xf_warnings = 0;        /* TRUE enable warnings */
int                     xf_warn_errors = 0;     /* TRUE force warnings to be treated as errors */
int                     xf_verbose = 0;         /* Verbose output */
int                     xf_debug = 0;           /* Debug level */
int                     xf_echoname = 0;        /* Echo filename */

int                     xf_abort = FALSE;       /* cause core dump on exit */
int                     xf_build = FALSE;       /* like 'make -n' */
int                     xf_debugger = FALSE;    /* TRUE compile with debug trace */
int                     xf_flush = FALSE;       /* TRUE flush output for crash debugging */
int                     xf_leave = FALSE;       /* TRUE leaves .m files intact */
int                     xf_make = 0;            /* Non zero only compile if objects are out of date */
int                     xf_prototype = TRUE;    /* Enable prototypes */
int                     xf_quiet = FALSE;       /* TRUE print filenames during compilation */
int                     xf_struct = FALSE;      /* TRUE dump structs */
int                     xf_symdump = FALSE;     /* dump symbol table */
int                     xf_unused = TRUE;       /* TRUE enable UNUSED() builtin */
int                     xf_watch = FALSE;       /* TRUE if we wanted to see passed command line */
int                     xf_output = FALSE;      /* TRUE retain prexisting output on failure */

const char *            x_filename = NULL;      /* Name of current source file */
static const char *     x_outputfile = NULL;    /* Output image name */

FILE *                  x_errfp = NULL;         /* Error file */
FILE *                  x_ofp = NULL;           /* Output file pointer */
extern FILE *           x_lexfp;                /* Input stream */

static char             gr_path[PATHLENGTH];    /* Directory where executable came from */

static time_t           x_grunch_mtime;         /* Modification time of ourselves. If grunch
                                                 * is newer than then the object file we need to make */

static const char *     cc_path  = CC_PATH;
static const char *     cc_prog  = CC_PROG;
static char *           cc_args  = NULL;
static const char *     cc_space = " > ";

static const char *     cc_defines[64];
static const char *     cc_includes[64];
static char             cc_timestamp[64];

#define CC_DEFINES      (sizeof(cc_defines)/sizeof(cc_defines[0]))
#define CC_INCLUDES     (sizeof(cc_includes)/sizeof(cc_includes[0]))

static char             cc_buf[PATHLENGTH];

static char *           cc_cmdfile;
static char             cc_tempfile[PATHLENGTH];

stat_t                  x_stats;

static struct longsw {
    const char *        name;
    int *               flag;
    const char *        help;
} longsw[] = {
    { "struct",     &xf_struct,     "Pretty print structure offsets." },
    { NULL,         NULL,           NULL                              },
    };

#if defined(CC_SLASHCONVERT) || \
        (!defined(DJGPP) && (defined(MSDOS) || defined(__MSDOS__) \
            || defined(__EMX__) || defined(__OS2__)))

#if !defined(CC_SLASHCONVERT)
#define CC_SLASHCONVERT
#endif
static void             backslash(char *fname);
static void             forwardslash(char *fname);
#else
#define backslash(x)
#define forwardslash(x)
#endif


int
main(int argc, char *argv[])
{
    const char *cc_params[8] = {NULL};          /* additional arguments */
#define CC_PARMS        (sizeof(cc_params)/sizeof(cc_params[0]))

    int exit_status = 0;
    const char *ev, *sp;
    int argi, len, x;

#if defined(__EMX__)
    _response(&argc, &argv);
    _wildcard(&argc, &argv);
#endif

    x_errfp = stderr;
    if (NULL != (x_progname = strrchr(argv[0], '/')) ||
            NULL != (x_progname = strrchr(argv[0], '\\'))) {
        ++x_progname;
    } else {
        x_progname = argv[0];
    }
    sprintf(cc_timestamp, "%s=%ld", CC_COMPILETIME, time(NULL));
    resolve_self(argv[0]);

    if (NULL != (ev = getenv("GRCPP"))) {       /* cpp override builtin defaults */
        if (*ev) {
            cc_prog = ev;
            if ('/' == *cc_prog
#if defined(DOSISH)
                || '\\' == *cc_prog || (isalpha((unsigned char)*cc_prog) && ':' == cc_prog[1])
#endif
                ) {
                cc_path = NULL;                 /* abs path, ignore CRPATH */
            }
        }
    }
    if (cc_path) {
        if (NULL != (ev = getenv("GRPATH"))) {
            cc_path = ev;                       /* path override */
        }
    }

    /*  Process switchs
     *      eg. '-I.. -DMYDEFINE'
     */
    autoload_init();
    argi = switch_process(argc, argv);

    if (xf_struct) {
        xf_symdump = TRUE;
    }

    if (! xf_leave) {
        x_generate_ascii = FALSE;
    }

    /*  Arguments
     *   eg. '-DCRUNCH -DGRIEF -D__PROTOTYPES__ -D__COMPILETIME__=12345668 -DMSDOS'
     */
    cc_params[0] = CC_GRUNCH;
    if (xf_prototype) {
        cc_params[1] = CC_PROTOTYPES;
    }
    cc_params[2] = cc_timestamp;
    if ((ev = getenv("GRARG")) != NULL) {       /* preprocessor arg override */
        cc_params[3] = ev;
    } else {
        cc_params[3] = CC_OSARG;
    }
    len = strlen(cc_prog);
    len += arg_sizeof(cc_params, CC_PARMS);
    len += arg_sizeof(cc_defines, CC_DEFINES);
    len += arg_sizeof(cc_includes, CC_INCLUDES);

    cc_args = chk_alloc(len);
    len = 0;
    if (NULL != (sp = strchr(cc_prog, ' '))) {
        len = strlen(cc_prog) - (++sp - cc_prog);
        memcpy(cc_args, sp, len);
        cc_args[len] = 0;
    }
    len += arg_export(cc_params, CC_PARMS, cc_args, len);
    len += arg_export(cc_defines, CC_DEFINES, cc_args, len);
    len += arg_export(cc_includes, CC_INCLUDES, cc_args, len);

    if (! resolve_path(cc_buf, sizeof(cc_buf), cc_path, cc_prog, (xf_watch ? "CPP" : NULL))) {
        fprintf(x_errfp, "%s: unable to locate C preprocessor '%s'\n" \
	             "using '%s'\n", x_progname, cc_prog, cc_path);
        exit(1);
    }

    arg_release(cc_defines, CC_DEFINES);
    arg_release(cc_includes, CC_INCLUDES);

    x = strlen(cc_buf);
    sprintf(cc_buf + x, " %s ", cc_args);
    cc_cmdfile = cc_buf + strlen(cc_buf);

    if (NULL == x_outputfile || 0 == x_outputfile[0]) {
        x_outputfile = ".";                     /* no output, default to current directory */
    }

    builtin_init();

    if (argc - argi > 1 && !xf_quiet) {         /* multiple source files */
        if (0 == xf_echoname) {
            ++xf_echoname;
        }
    }

    for (; argi < argc; ++argi) {
        if ((exit_status = compile_file(argv[argi])) != 0) {
            break;
        }
    }

    autoload_close();
    if (xf_abort) {                             /* core dump */
        abort();
    }
    exit(exit_status);
    return 0;
}


#if defined(CC_SLASHCONVERT)
static void
backslash(char *fname)
{
    while (*fname) {
        if (*fname == '\\') {
            *fname = '/';
        }
        ++fname;
    }
}


static void
forwardslash(char* fname)
{
    while (*fname) {
        if (*fname == '/') {
            *fname = '\\';
        }
        ++fname;
    }
}
#endif  /*CC_SLASHCONVERT*/


#if defined(MSDOS) || defined(__EMX__) || defined(WIN32)
static const char *
get_tmpdir(const char *env)
{
    const char *ev;

    if ((ev = getenv(env)) != NULL
#if defined(_MSC_VER) && (_MSC_VER >= 1400)
	    && _access(ev, 0) == 0) {
#else
	    && access(ev, 0) == 0) {
#endif
        return ev;
    }
    return NULL;
}
#endif  /*MSDOS || __EMX__ || WIN32*/


/*
 *  compile_file ---
 *      Process a source file.
 */
static int
compile_file(const char *srcname)
{
    char t_outname[BUFSIZ], outname[BUFSIZ];
    struct stat objsb = {0}, srcsb = {0};
    int len, exit_status = 0, ext_len = 0;
    int ok = FALSE;
#if defined(HAVE_MKSTEMP)
    int tmpfd = -1;
#endif

    x_filename = srcname;
    backslash((char *)x_filename);
    len = strlen(x_filename);

    strcpy(cc_cmdfile, x_filename);
    strcat(cc_cmdfile, cc_space);
    strcpy(outname, x_filename);

    if (len > 2 &&
            0 == strcmp(x_filename + len - 2, ".c")) {
        xf_grunch = 0;
        ext_len = 2;
        ok = TRUE;

    } else if (len > 3) {
        const char *ext = (x_filename + len - 3);

        if (0 == strcmp(ext, ".gr")  ||         /* grief/crisp/brief */
                0 == strcmp(ext, ".cr") ||
                0 == strcmp(ext, ".cb")) {
            xf_grunch = 1;                      /* TODO, xf_compat */
            ext_len = 3;
            ok = TRUE;
        }
    }

    if (ok) {
        srcsb.st_mtime = objsb.st_mtime = 0;

        if (xf_leave) {
            strcpy(outname + len - ext_len, ".m");

        } else if (x_outputfile && x_filename[0] != '/'
#if defined(DOSISH)
                        && x_filename[1] != ':'
#endif
            ) {
            strcpy(outname, x_outputfile);
            if (-1 != stat(outname, &objsb) &&
                    (objsb.st_mode & S_IFMT) == S_IFDIR) {
                strcat(outname, "/");
                strcat(outname, x_filename);
                strcpy(outname + strlen(outname) - ext_len, CM_EXTENSION);
                objsb.st_mtime = 0;
            }
        } else {
            strcpy(outname + len - ext_len, CM_EXTENSION);
        }

        if (verify_object(srcname, &srcsb, outname, &objsb)) {
            if (xf_build) {                     /* make file, only compile if out-of-date */
                printf("%s upto date.\n", srcname);
            }
            return 0;
        }

        if (xf_build) {
            printf("%s out of date.\n", srcname);
            return 0;
        }

        if (xf_echoname) {
            printf("%s\n", srcname);
        }

#if defined(MSDOS) || defined(__EMX__) || defined(WIN32)
        {
            const char *tmp = get_tmpdir("TMP");
            int l;

            if (tmp == NULL || NULL == (tmp = get_tmpdir("TEMP")) ||
                    NULL == (tmp = get_tmpdir("TMPDIR"))) {
                tmp = ".";                      /* current working directory */
            }
            strcpy(cc_tempfile, tmp);
            l = strlen(cc_tempfile);
            if (l && (cc_tempfile[l - 1] != '\\' || cc_tempfile[l - 1] != '/')) {
                cc_tempfile[l++] = DIRCHR;
            }
            strcpy(cc_tempfile + l, "chXXXXXX");
        }
#else
        strcpy(cc_tempfile, TMPFILE);
#endif

#if defined(HAVE_MKSTEMP)
        if ((tmpfd = mkstemp(cc_tempfile)) < 0) {
            printf("%s: error builting temporary image '%s' : %s (%d)\n",
                x_progname, cc_tempfile, strerror(errno), errno);
            exit(1);
        }
        strcat(cc_buf, cc_tempfile);
        forwardslash(cc_buf);
#elif defined(__WATCOMC__) || \
            defined(_MSC_VER) && (_MSC_VER >= 1400)
	strcat(cc_buf, _mktemp(cc_tempfile));
#else
        strcat(cc_buf, mktemp(cc_tempfile));
#endif
        forwardslash(cc_buf);

        if (xf_watch) {
            if (xf_watch >= 2) {
                printf("gr_path: %s\n", gr_path);
                printf("cc_prog: %s\n", cc_prog);
                printf("cc_args: %s\n", cc_args);
                printf("cc_temp: %s\n", cc_tempfile);
            }
            printf("%s\n", cc_buf);
        }

        if (0 != system(cc_buf)) {
	    UNLINK(cc_tempfile);
            exit(1);
        }

        if (xf_output) {                        /* local working copy */
            strcpy(t_outname, outname);
            strcat(t_outname, "$");
        }

        if (NULL == (x_ofp =
                fopen((xf_output ? t_outname : outname), FOPEN_W_BINARY))) {
            printf("%s: error opening pass2 '%s' : %s (%d)\n",
                x_progname, (xf_output ? t_outname : outname), strerror(errno), errno);
            exit(1);
        }

#if defined(HAVE_MKSTEMP)
        if (NULL == (x_lexfp = fdopen(tmpfd, "r"))) {
#else
        if (NULL == (x_lexfp = fopen(cc_tempfile, "r"))) {
#endif
            printf("%s: error opening temporary '%s' : %s (%d)\n",
                x_progname, cc_tempfile, strerror(errno), errno);
            exit(1);
        }

        parser_init();                          /* global initialisation */
        autoload_module(srcname, srcsb.st_mtime);
        yyparse();

        if (yyerrors() || (xf_warn_errors && yywarnings())) {
            exit_status = 1;
        }

        if (xf_symdump) {
            symtab_dump();
        }
        fclose(x_lexfp), x_lexfp = NULL;
        if (xf_leave < 2) {
            UNLINK(cc_tempfile);
        }

        compile_main(x_maintree);               /* main() code generation */
        autoload_export(0 == exit_status ? TRUE : FALSE);

        parser_close();                         /* release any storage etc */

        fclose(x_ofp), x_ofp = NULL;
        if (exit_status) {
            if (!xf_output) {
	        UNLINK(outname);
            }
        } else {
            if (xf_output) {
                unlink(outname);
                if (rename(t_outname, outname) < 0) {
                    printf("%s: error updating'%s' : %s (%d)\n",
                        x_progname, outname, strerror(errno), errno);
                    UNLINK(t_outname);
                    exit(1);
                }
            }
        }
        fflush(stdout);
        fflush(stderr);
    }
    return exit_status;
}


/*
 *  verify_object ---
 *      Determine whether the object is up-to-date against the associated source.
 */
static int
verify_object(const char *srcname, struct stat *srcsb, const char *objname, struct stat *objsb)
{
    int ret = FALSE;

    if ((srcsb->st_mtime > 0 || -1 != stat(srcname, srcsb)) &&
            (objsb->st_mtime > 0 || -1 != stat(objname, objsb))) {

        if (xf_make) {                          /* verify enabled */

                                                /* verify timestamps */
            if (objsb->st_mtime >= srcsb->st_mtime &&
                    (xf_make < 2 || objsb->st_mtime >= x_grunch_mtime)) {

                CM_t cm = {0};
                FILE *fp;
                                                /* verify object structure */
                if (NULL != (fp = fopen(objname, FOPEN_R_BINARY))) {
                    if (fread(&cm, 1, sizeof(cm), fp) == sizeof(cm)) {
                        cm_xdr_import(&cm);
                        if (cm.cm_version == cm_version &&
                                cm.cm_builtin == builtin_count &&
                                cm.cm_signature == builtin_signature) {
                            ret = TRUE;
                        }
                    }
                    fclose(fp);
                }
            }
        }
    }

    if (xf_watch) {
        printf("%s/%s: verify (%ld = %ld, %ld) : %d\n", objname, srcname,
                (unsigned long)objsb->st_mtime, (unsigned long)srcsb->st_mtime, (unsigned long)x_grunch_mtime, ret);
    }
    return ret;
}


static void
usage(void)
{
    static const char *help[] = {
        "",
        "Usage: %s [-acfgmnpqSWw] [-Dvar[=value]] [-Uvar] [-Ipath]",
        "           [-o output_file] [+struct] file-name ...",
        "",
        "Option:",
        "   -a              Create core file on exit (for debugging).",
        "   -c              Leave temporary files (.m etc).",
        "   -Dvar           Define var as in #define.",
        "   -d[d]           Enable internal debugging features.",
        "   -f              Flush output for debugging.",
        "   -Ipath          Add 'path' to the #include search path.",
        "   -g              Compile with debug on.",
        "   -m[m]           Compile only if out of date (make option).",
        "   -n              Don't do anything but tell us what you would do.",
        "   -o file         Name of compiled output file.",
        "   -e file         Error output file.",
        "   -A file         Autoload macro file.",
        "   -p cpp          Specify name of C pre-processor to use.",
        "   -q              Dont print filenames during compilation.",
        "   -S              Dump symbol table.",
        "   -Uvar           Undefine var as in #undef.",
        "   -UUNUSED        Remove internal UNUSED definition.",
        "   -V              Version information.",
        "   -w              Enable warnings.",
        "   -v              Verbose.",
        "   -warn_errors    Treat warnings as errors.",
        "   -wproto         Disable prototype checks.",
        "   -stages         Watch compiler passes.",
        "   +struct         Pretty print structure offsets for easy parsing.",
        "   -h              Command line help.",
        "",
        "Variables:",
        "   GRCPP=<cpp>     Preprocessor override (grcpp).",
        "   GRARG=<args>    Preprocessor argument override.",
        "   GRPATH=<path>   Grief include path.",
        "", NULL};
    int idx;

    fprintf(stderr, "\n"
        "" ApplicationName " Macro Compiler.\n"
        "%s %s compiled %s\n", x_progname, x_version, x_compiled);
    if (xf_debug) {
        fprintf(stderr,
            "\n  <%s %s %s %s",
            CC_PROG, CC_GRUNCH, CC_OSARG, cc_timestamp);
        if (xf_prototype) {
            fprintf(stderr, " %s", CC_PROTOTYPES);
        }
        fputs(">\n", stderr);
    }
    for (idx = 0; help[idx]; ++idx) {
        fprintf(stderr, help[idx], x_progname);
        fputc('\n', stderr);
    }
    exit(1);
}


void
sys_abort(void)
{
    abort();
}


void
panic(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    fputs("panic --- ", stderr);
    vfprintf(stderr, fmt, ap);
    fflush(stderr);
    abort();
}


static void
arg_push(const char *args[], unsigned count, const char *val, const char *prefix)
{
    unsigned idx;

    if (prefix) {
        char *t_val = chk_alloc(strlen(prefix) + strlen(val) + 1);

        strcpy(t_val, prefix);
        strcat(t_val, val);
        val = t_val;
    } else {
        val = chk_salloc(val);
    }

    for (idx = 0; idx < count; ++idx) {
        if (NULL == args[idx]) {
            args[idx] = val;
            return;
        }
    }
    fprintf(x_errfp, "%s -- argument '%s' exceed system limits\n", x_progname, val);
    exit(1);
}


static unsigned
arg_sizeof(const char *args[], unsigned count)
{
    unsigned length, idx;

    length = 0;
    for (idx = 0; idx < count; ++idx) {
        if (args[idx]) {
            length += strlen(args[idx]) + 1;
        }
    }
    return length;
}


static unsigned
arg_export(const char *args[], unsigned count, char *buffer, unsigned offset)
{
    char *cursor = buffer + offset;
    unsigned length, idx;

    length = 0;
    for (idx = 0; idx < count; ++idx) {
        if (args[idx]) {
            unsigned ilen = strlen(args[idx]);

            if (offset || length) {
                *cursor++ = ' ', ++length;
            }
            memcpy(cursor, args[idx], ilen);
            cursor += ilen, length += ilen;
        }
    }
    return length;
}


static void
arg_release(const char *args[], unsigned count)
{
    unsigned idx;

    for (idx = 0; idx < count; ++idx) {
        if (args[idx]) {
            chk_free((char *)args[idx]);
            args[idx] = NULL;
        }
    }
}


static int
switch_process(int argc, char *argv[])
{
    extern int yydebug;
    const char *cp;
    int argi;

    for (argi = 1; argi < argc; ++argi) {
        if (argv[argi][0] == '+') {
            switch_map(argv[argi] + 1);
            continue;
        }

        if (argv[argi][0] != '-') {
            break;
        }

        cp = argv[argi] + 1;
        while (cp && *cp) {
            const char *optv = NULL;
            int optc = *cp++;

            /* non-optional arguments */
            switch (optc) {
            case 'U':
                if (0 == strcmp(cp+1, "UNUSED")) {
                    xf_unused = 0;
                    optc = 0;
                    break;
                }
            case 'D':
            case 'I':
            case 'e':
            case 'o':
            case 'p':
            case 'A':
                optv = cp;
                if (0 == *optv) {
                    if (NULL == (optv = argv[++argi])) {
                        fprintf(x_errfp, "%s: option expects an argument -- %c.\n",
                            x_progname, optc);
                        usage();
                    }
                }
                cp = NULL;
                break;
            }

            /* options */
            switch (optc) {
            case 'a':       /* abort/core */
                xf_abort = TRUE;
                break;

            case 'U':       /* undef */
                if (optv) {
                    arg_push(cc_defines, CC_DEFINES, optv, "-U");
                }
                break;

            case 'D':       /* defines */
                arg_push(cc_defines, CC_DEFINES, optv, "-D");
                break;

            case 'I':       /* includes */
                arg_push(cc_includes, CC_INCLUDES, optv, "-I");
                break;

            case 'c':       /* leave temporary */
                ++xf_leave;
                break;

            case 'd':       /* debug level */
                if (3 == ++xf_debug) {
                    yydebug = TRUE;
                }
                break;

            case 'e': {     /* error output */
                    FILE *ox_errfp = x_errfp;

                    if (NULL == (x_errfp = fopen(optv, "w"))) {
                        fprintf(ox_errfp, "%s: cannot open error file '%s' : %s(%d)\n",
                            x_progname, optv, strerror(errno), errno);
                        exit(1);
                    }
                }
                break;

            case 'A':       /* autoload */
                if (-1 == autoload_open(optv)) {
                    fprintf(x_errfp, "%s: cannot open autoload file '%s' : %s(%d)\n",
                        x_progname, optv, strerror(errno), errno);
                    exit(1);
                }
                break;

            case 'f':       /* flush */
                xf_flush = TRUE;
                break;

            case 'g':       /* enable debug */
                xf_debugger = TRUE;
                break;

            case 'p':       /* preprocessor */
                if (0 == strcmp(optv, "native")) {
                    cc_prog = CC_NATIVE;
#if defined(CC_NATIVENOECHO)
                    xf_echoname = -1;
#endif
                } else {
                    cc_prog = optv;
                }
                break;

            case 'm':       /* make mode */
                ++xf_make;
                break;

            case 'n':       /* make -n mode */
                xf_build = TRUE;
                break;

            case 'o':       /* output image */
                x_outputfile = optv;
                cp = "";
                break;

            case 'O':       /* retain output image on failure */
                xf_output = TRUE;
                break;

            case 's':
                if (0 == strcmp(cp, "tages")) {
                    ++xf_watch;                 /* stages */
                    cp = NULL;
                    break;
                }
                fprintf(x_errfp, "%s: unknown option --- %c\n", x_progname, optc);
                usage();
                break;

            case 'S':
                if (xf_symdump) {
                    xf_struct = TRUE;
                }
                xf_symdump = TRUE;
                break;

            case 'q':       /* quiet */
                xf_quiet = TRUE;
                break;

            case 'w':       /* warnings */
                if (*cp >= '1' && *cp <= '9') {
                    xf_warnings = (*cp - '0');  /* warning level */
                    ++cp;

                } else if (0 == strcmp(cp, "arn_errors")) {
                    ++xf_warn_errors;           /* warn_errors */
                    cp = NULL;

                } else if (0 == strcmp(cp, "proto")) {
                    xf_prototype = FALSE;       /* wproto (disable) */
                    cp = NULL;

                } else {
                    xf_warnings = 1;
                }
                break;

            case 'v':       /* verbose */
                ++xf_verbose;
                break;

            case 'V':       /* version */
                printf("%s version %s, compiled %s\n", myname, x_version, x_compiled);
                exit(0);

            case 0:
                break;

            default:
                fprintf(x_errfp, "%s: unknown option --- %c\n", x_progname, optc);
            case 'h':
                usage();
                break;
            }
        }
    }
    return argi;
}


static void
switch_map(const char *str)
{
    struct longsw *sp = longsw;

    while (sp->name) {
        if (strcmp(sp->name, str) == 0) {
            *sp->flag = TRUE;
            return;
        }
        sp++;
    }
    usage();
}


/*  Function:           resolve_self
 *      Determine the canonical path to an current executable.
 *
 *  Parameters:
 *      name -              Application name (argv0).
 *
 *  Returns:
 *      nothing.
 */
static void
resolve_self(const char *name)
{
    struct stat sb = {0};

#if defined(WIN32)
    DWORD len;

    len = GetModuleFileName(NULL, gr_path, sizeof(gr_path));
    if (len && len < sizeof(gr_path)) {
        gr_path[len] = 0;
        if (-1 != stat(gr_path, &sb)) {         /* env */
            x_grunch_mtime = sb.st_mtime;
        }
        return;
    }

#else
    const char *cmpp = "$PATH" PATHCH "$GRPATH";
    char t_self[64], t_name[PATHLENGTH];
    int namelen = -1;
    char *cp;

#if defined(unix) || defined(linux)
    if ('/' != *name) {
#if defined(HAVE_GETEXECNAME)                   /* solaris */
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
            }

#elif defined(sun) || defined(__sun__)
            if ((namelen = readlink("/proc/self/paths/a.out", t_name, sizeof(t_name))) > 0) {
                t_name[namelen] =  0;
                name = t_name;
            }

#elif defined(__APPLE__)
            if (proc_pidpath(getpid(), t_name, sizeof(t_name)) != 0 {
                name = t_name;
            } else if (_NSGetExecutablePath(t_name, sizeof(t_name)) != -1) {
                name = t_name;                  /* Mac 10.2+ */
            }

#elif defined(BSD)
            if ((namelen = readlink("/proc/curproc/file", t_name, sizeof(t_name))) > 0) {
                t_name[namelen] =  0;
                name = t_name;                  /* procfs */
            } else {
                int mib[4] = {CTL_KERN, KERN_PROC, KERN_PROC_PATHNAME, -1};
                size cb = sizeof(t_name);
                if (-1 != sysctl(mib, 4, t_name, &cb, NULL, 0)) {
                  name = t_name;                /* alt method */
                }
            }
#endif
        }
    }

#else
#if defined(EXEEXT)
    namelen = strlen(name);
    if (namelen < sizeof(EXEEXT) ||		/* add .exe if we need to */
            0 != str_icmp((name + namelen) - (sizeof(EXEEXT) - 1), EXEEXT)) {
        memcpy(t_name, name, namelen);
        memcpy(t_name + namelen, EXEEXT, sizeof(EXEEXT)));
        name = t_name;
    }
#endif
#endif

    strxcpy(gr_path, name, sizeof(gr_path));
    if (NULL != (cp = strrchr(gr_path, DIRCHR))) {
        if (-1 != stat(gr_path, &sb)) {         /* abs */
            x_grunch_mtime = sb.st_mtime;
            return;
        }
        name = cp + 1;
    }

    if (TRUE == resolve_path(gr_path, sizeof(gr_path), cmpp, name, "NAME")) {
        if (-1 != stat(gr_path, &sb)) {         /* env */
            x_grunch_mtime = sb.st_mtime;
            return;
        }
    }
#endif  /*!WIN32*/

    fprintf(x_errfp, "%s: unable to locate application path.\n", x_progname);
    exit(1);
}


/*  Function:           resolve_path
 *      Searches a path for a specific executable/filename.
 *
 *  Parameters:
 *      dst -               Destination buffer.
 *      dstlen -            Buffer length, in bytes.
 *      pp -                Path specification.
 *      name -              Application name.
 *      label -             Trace label.
 *
 *  Returns:
 *      TRUE if successful, otherwise FALSE.
 */
static int
resolve_path(char *dst, int dstlen, const char *pp, const char *name, const char *label)
{
    struct stat sb;
    char t_name[PATHLENGTH];
    int namelen;
    char *p;

    namelen = strlen(name);
    if (NULL != (p = strchr(name, ' '))) {
        namelen = p - name;
    }
    if (namelen >= dstlen) {
        return FALSE;
    }

    memcpy(t_name, name, namelen);
    t_name[namelen] = 0;

#if defined(EXEEXT)
    if (namelen < sizeof(EXEEXT) ||		/* add .exe, if needed */
            0 != str_icmp((t_name + namelen) - (sizeof(EXEEXT)-1), EXEEXT)) {
        memcpy(t_name + namelen, EXEEXT, sizeof(EXEEXT));
	namelen += (sizeof(EXEEXT) - 1);
    }
#endif

    if (-1 != stat(t_name, &sb) && !S_ISDIR(sb.st_mode)) {
        strxcpy(dst, t_name, dstlen);           /* abs path */
        goto success;
    }

    if (gr_path[0]) {                           /* relative to self */
#if defined(WIN32)
        strxcpy(dst, ('"' == *gr_path ? gr_path + 1 : gr_path), dstlen);
#else
        strxcpy(dst, gr_path, dstlen);
#endif
        if (NULL != (p = strrchr(dst, DIRCHR))) {
            strcpy(++p, t_name);
            p[namelen] = 0;
            if (xf_watch) {
                printf("PATH: %s\n", dst);
            }
            if (-1 != stat(dst, &sb) && !S_ISDIR(sb.st_mode)) {
                goto success;
            }
        }
    }

    if (pp && *pp) {                            /* process path */
       char evbuf[PATHLENGTH * 2];

        if (NULL != (pp = expand_var(pp, evbuf, sizeof(evbuf)))) {
            for (p = strtok(evbuf, PATHCH); p; p = strtok(NULL, PATHCH)) {
                if (!*p) {
                    strxcpy(dst, t_name, dstlen);
                } else {
                    sxprintf(dst, dstlen, "%s%c%s", p, DIRCHR, t_name);
                }
                if (xf_watch) {
                    printf("PATH: %s\n", dst);
                }
                if (stat(dst, &sb) >= 0 && !S_ISDIR(sb.st_mode)) {
                    goto success;
                }
            }
        }    }
    return FALSE;

success:;
#if defined(CC_SLASHCONVERT)
    backslash(dst);
#endif
#if defined(WIN32)
    if (strchr(dst, ' ')) {                     /* quote arg0 */
        int len = (int)strlen(dst);

        if ((len + 2) < dstlen) {
            memmove(dst + 1, dst, len);
            dst[0]     = '\"';
            dst[++len] = '\"';
            dst[++len] = '\0';
        }
    }
#endif
    if (label) {
        printf("%s: %s\n", label, dst);
    }
    return TRUE;
}


/*  Function:           expand_var
 *      Expand environment variables within a string.
 *
 *  Parameters:
 *      env -               Variable to be expanded.
 *      buf -               Result buffer address.
 *      buflen -            Size of buffer, in bytes.
 *
 *  Returns:
 *      Address of buffer containing result.
 */
static const char *
expand_var(const char *env, char *buf, int buflen)
{
    char *p, var[256];

    if (xf_watch) {
        printf("IN:  <%s>\n", env);
    }

    strxcpy(buf, env, buflen);
    while (NULL != (p = strchr(buf, '$'))) {
        const char *env, *q = p + 1;
        int len = 0;

        /*variable*/
        if (*q == '{') {                        /* '${var}' */
            while (*++q) {
                if ('}' == *q) {
                    ++q;
                    break;
                }
                if (len < (sizeof(var) - 2)) {
                    var[len++] = *q;
                }
            }
        } else if (*q == '(') {                 /* '$(var)' */
            while (*++q) {
                if (')' == *q) {
                    ++q;
                    break;
                }
                if (len < (sizeof(var) - 2)) {
                    var[len++] = *q;
                }
            }
        } else {                                /* '$var' */
            while (*q && ('_' == *q || isalnum(*q))) {
                if (len < (sizeof(var) - 2)) {
                    var[len++] = *q;
                }
                ++q;
            }
        }
        var[len] = '\0';

        /*expand*/
        if (NULL == (env = getenv(var))) {
            strcpy(p, q);                       /* just remove it */

        } else {
            const int l = p - buf,              /* leading length */
                r = strlen(q),                  /* remaining characters */
                n = strlen(env);                /* new characters */

            if ((l + n + r) >= buflen) {
                printf("%s: unable to expand environment, out of space.", x_progname);
                return NULL;
            }
            if (r) memmove(p + n, q, r);
            memcpy(p, env, n);
            p[n + r] = '\0';
        }
    }

    if (xf_watch) {
        printf("OUT: <%s>\n", buf);
    }
    return buf;
}
/*end*/
