/* -*- mode: c; indent-width: 4; -*- */
/* $Id: gm.c,v 1.25 2014/10/03 01:03:34 ayoung Exp $
 * Grief LISP macro compiler.
 *
 *
 *
 */

#include <editor.h>
#include <edgetopt.h>
#include <edcm.h>

#include <stdarg.h>
#include "keywd.h"
#include "language.h"
#include "word.h"

static char const *     x_progname = "gm";

static MACRO            macro_tbl[MAX_MACROS];

static uint32_t         m_offsets[MAX_MACROS];

static unsigned         macro_cnt;

static int              nglobals;               /* Number of global statements found so far */

static uint32_t         globals[CM_GLOBALS_MAX];   /* Table of indexes to global statements */

static FILE *           fp;                     /* File pointer for output file */

static const char *     output_file;            /* Name of output file */

static CM_t             cm_header = {CM_MAGIC}; /* Header for output file */

static unsigned         string_count;           /* Count of literals in list */

static char **          string_table;           /* String table */

static char *           str_table;              /* Pointer to string table for disassembly */

static LIST *           atom_start;             /* Pointer to base of compiled macro */

static int              atom_count;             /* Count of atoms in buffer */

static int              o_aflg = FALSE;

static int              o_lflg = FALSE;

static int              o_sflg = FALSE;         /* TRUE if size info. only */

static int              o_Lflg = FALSE;         /* TRUE if we want more disassembly info */

static const char *     o_includes[32];         /* Include paths */


/*
 *   Count of each atom type
 */
static int              num_atoms[(int)F_MAX];  /* MAGIC */


/*
 *  Prototype definitions.
 */
static void             write_output(const char *fname);
static const char *     put_string(LIST *lp);
static void             list_macro(int level, const LIST *lp);
static void             disassemble(const char *file);
static void             print_perc(void);
static void             patom(const LIST *first_atom, const LIST *mptr, const char *str);
static void             loadmacro(const LIST *lp, int size);
static void             usage(void);
static int              do_switches(int ac, char **av);
static void             do_include(const char *path);


/*
 *  Stubs required to link cm/language.
 */
int                     sys_read(int fd, char *b, int size);
void                    trace_list(const LIST *lp);
void                    execute_xmacro(const LIST *lp, const LIST *lp_argv);
void                    ewprintf(const char *msg, ...);
void                    errorf(const char *msg, ...);
void                    panic(const char *msg, ...);
void                    sys_abort(void);


int
main(int argc, char **argv)
{
    MACRO *mp;
    int exit_status = 0;
    int print_msg;
    char obuf[BUFSIZ];
    struct stat sbuf;
    int arg_index = do_switches(argc, argv);

#if defined(__EMX__)
    _response(&argc, &argv);
    _wildcard(&argc, &argv);
#endif

    cm_init(TRUE);

    if (arg_index >= argc)
        usage();

    builtin_init();

    print_msg = (arg_index < argc - 1);

    for (; arg_index < argc; ++arg_index) {
        const char *file = argv[arg_index];
        int len = strlen(file);

        atom_count = 0;

        if (strlen(file) > 3 && 0 == strcmp(file + len - 3, CM_EXTENSION)) {
            disassemble(file);
            continue;
        }

        if (print_msg)
            printf("Compiling %s...\n", file);

        if (cm_push(file) < 0) {
            perror(file);
            continue;
        }

        macro_cnt = 0;
        nglobals = 0;

        if (cm_parse(loadmacro, o_includes) != 0) {
            printf("Compilation of %s unsuccessful.\n", file);
            exit_status = -1;
            continue;
        }

        /*
         *  Open output file. If output file is a directory, then put
         *  the compiled file in the specified directory
         */
        if (output_file && stat(output_file, &sbuf) >= 0 &&
                (sbuf.st_mode & S_IFMT) == S_IFDIR) {
            if (len <= 2 || strcmp(file + len - 2, ".m") != 0) {
                sprintf(obuf, "%s/%s%s", output_file, file, CM_EXTENSION);
            } else {
                sprintf(obuf, "%s/%.*s%s", output_file, len - 2, file, CM_EXTENSION);
            }
        } else if (NULL == output_file) {
            if (len <= 2 || strcmp(file + len - 2, ".m") != 0) {
                sprintf(obuf, "%s%s", file, CM_EXTENSION);
            } else {
                sprintf(obuf, "%.*s%s", len - 2, file, CM_EXTENSION);
            }
        } else {
            strcpy(obuf, output_file);
        }

        write_output(obuf);
        if (o_aflg)
            print_perc();

        /* Dump internal form of macro if asked for */
        for (mp = macro_tbl; mp < &macro_tbl[macro_cnt]; mp++) {
            if (o_lflg) {
                list_macro(0, mp->m_list);
                printf("\n");
            }
        }

        output_file = NULL;
    }
    return exit_status;
}


static int
do_switches(int ac, char **av)
{
    int c;

    while ((c = getopt(ac, av, "hacLqldo:I:s")) != EOF) {
        switch (c) {
        case 'a':
            o_aflg = TRUE;
            break;

        case 'l':
            o_lflg = 1;
            break;

        case 'q':
            break;

        case 'L':
            o_Lflg = 1;
            break;

        case 'o':
            output_file = optarg;
            break;

        case 'I':
            do_include(optarg);
            break;

        case 's':
            o_sflg = TRUE;
            break;

        default:
        case 'h':
            usage();
            break;
        }
    }
    return optind;
}


static void
do_include(const char *path)
{
    unsigned idx;

    path = chk_salloc(path);
    for (idx = 0; idx < ((sizeof(o_includes)/sizeof(o_includes[0])) - 1); ++idx) {
        if (NULL == o_includes[idx]) {
            o_includes[idx] = path;
            return;
        }
    }
    ewprintf("argument '-I' exceed system limits");
    exit(1);
}


static void
usage(void)
{
    static const char *help[] = {
        "",
        "Usage: gm [options] <file> ...",
        "",
        "options:",
        "   -a              Print atom percentages.",
        "   -l              List macro expansions.",
        "   -L              Print detailed disassembly info.",
        "   -q              Quiet error messages.",
        "   -s              Print size of " CM_EXTENSION " file only.",
        "   -o file         Name of compiled output file.",
        "   -I path         Include path.",
        "   -h              Command line help.",
        "",
        NULL
        };
    unsigned idx;

    fprintf(stderr,
        "\n"
        "" ApplicationName " macro decompiler.\n"
        "%s %s compiled %s (engine version %u.%u)\n", x_progname, x_version, x_compiled,
                (unsigned)(cm_version / 10), (unsigned)(cm_version % 10));
    for (idx = 0; help[idx]; ++idx) {
        fputs(help[idx], stderr);
        fputc('\n', stderr);
    }
    exit(1);
}


static void
write_output(const char *fname)
{
    uint32_t base, offset;
    const LIST *lp;
    unsigned i;

    if (NULL == (fp = fopen(fname, FOPEN_W_BINARY))) {
        perror(fname);
        exit(1);
    }

    if (NULL == (atom_start = chk_alloc(CM_ATOMS_MAX)) ||
            NULL == (string_table = chk_alloc(sizeof(char *) * CM_STRINGS_MAX))) {
        fprintf(stderr, "Not enough memory to compile macros\n");
        exit(1);
    }

    cm_header.cm_version = cm_version;
    cm_header.cm_builtin = builtin_count;
    cm_header.cm_signature = builtin_signature;
    cm_header.cm_num_macros = macro_cnt;

    if (fwrite((char *) &cm_header, sizeof(cm_header), 1, fp) != 1) {
output_error:
        perror(output_file);
        exit (1);
    }

    if (fwrite(m_offsets, sizeof(uint32_t), macro_cnt + 2, fp) != macro_cnt + 2) {
        goto output_error;
    }

    base = ftell(fp);
    string_count = 0;

    for (i = 0; i < macro_cnt; ++i) {
        unsigned n = macro_tbl[i].m_size;
        const LIST *lpend;

        if (o_Lflg) {
            printf("\n*** Macro %d (%s%s%s):\n", i,
                (macro_tbl[i].m_flags & 0x01 ? "static " : ""),
                (macro_tbl[i].m_flags & 0x02 ? "replacement " : ""),
                macro_tbl[i].m_name);
        }

        lp = macro_tbl[i].m_list;
        lpend = lp + n;
        m_offsets[i] = (ftell(fp) - (long) base) / sizeof(LIST);

        while (lp < lpend) {
            const char *str = "";
            const int atom = *lp;

            if (F_STR == atom || F_LIT == atom) {
                str = put_string((LIST *)lp);

            } else if (F_ID == atom) {
                const int id = LGET_ID(lp);

                assert(id >= 0 && id < (int)builtin_count);
                str = builtin[id].b_name;
                if (0 == strcmp(str, "global")) {
                    globals[nglobals++] = m_offsets[i] + (lp - macro_tbl[i].m_list);
                }
            }

            if (o_Lflg) {
                patom(macro_tbl[i].m_list, lp, str);
            }

            lp += sizeof_atoms[*lp];
        }

        if (fwrite((char *) macro_tbl[i].m_list, sizeof(LIST), n, fp) != n) {
            goto output_error;
        }
    }

    if (ftell(fp) & 3) {                        /* pad object */
        fwrite("PAD", (int) (4 - (ftell(fp) & 3)), 1, fp);
    }

    m_offsets[macro_cnt] = ftell(fp) - (long) base;
    m_offsets[macro_cnt + 1] = string_count;

    /* Now write out table of string offsets from here */
    for (offset = 0, i = 0; i < string_count; ++i) {
        uint32_t o = WGET32(offset);

        if (fwrite((char *) &o, sizeof o, 1, fp) != 1) {
            goto output_error;
        }
        offset += strlen(string_table[i]) + 1;
    }

    /* Now write out string table */
    for (i = 0; i < string_count; ++i) {
        const int len = strlen(string_table[i]);

        if (fwrite(string_table[i], len + 1, 1, fp) != 1) {
            goto output_error;
        }
    }

    if (ftell(fp) & 3) {                        /* pad object */
        fwrite("PAD", (int) (4 - (ftell(fp) & 3)), 1, fp);
    }

    cm_header.cm_globals = ftell(fp);
    WGET32_block(globals, nglobals);

    if (nglobals && fwrite((char *) globals, sizeof globals[0] * nglobals, 1, fp) != 1) {
        goto output_error;
    }

    rewind(fp);

    cm_header.cm_num_atoms = atom_count;
    cm_header.cm_num_globals = nglobals;
    cm_header.cm_num_strings = string_count;
    cm_xdr_export(&cm_header);
    if (fwrite((char *) &cm_header, sizeof(cm_header), 1, fp) != 1) {
        goto output_error;
    }

    WGET32_block(m_offsets, macro_cnt + 2);
    if (fwrite((char *) m_offsets, sizeof(uint32_t), macro_cnt + 2, fp) != macro_cnt + 2) {
        goto output_error;
    }

    fclose(fp);

    chk_free(string_table);
    chk_free(atom_start);
}


static const char *
put_string(LIST *lp)
{
    const char *str = (const char *)LGET_PTR(lp);
    char **cpp, **cpend = string_table + string_count;

    for (cpp = string_table; cpp < cpend; ++cpp) {
        if (**cpp == *str && 0 == strcmp(*cpp, str)) {
            chk_free((void *)str);
            LPUT_INT(lp, (int)(cpp - string_table));
            return *cpp;
        }
    }

    *cpp = (void *)str;
    LPUT_INT(lp, (accint_t) string_count++);
    return str;
}


static void
disassemble(const char *file)
{
    uint32_t *vm_offsets, *soffsets;
    const LIST *lp, *baselp;
    unsigned nm, i;
    const char *str;
    struct stat sb;
    FILE *infp;
    CM_t *cm;

    printf("\n*** File: %s\n\n", file);

    for (i = 0; i < (sizeof(num_atoms)/sizeof(num_atoms[0]));) {
        num_atoms[i++] = 0;
    }

    if (NULL == (infp = fopen(file, FOPEN_R_BINARY))) {
        fprintf(stderr, "error opening '%s' : %s", file, strerror(errno));
        exit(1);
    }

    if (stat(file, &sb) < 0) {
        perror(file);
        exit(1);
    }

    cm = chk_alloc((unsigned) sb.st_size);
    if (read(fileno(infp), cm, (int) sb.st_size) != (int) sb.st_size) {
        fprintf(stderr, "read error on %s file : %s", CM_EXTENSION, strerror(errno));
        exit(1);
    }
    fclose(infp);

    /* Make sure header is in the natural byte order */
    cm_xdr_import(cm);

    printf(" magic       : 0x%04x\n", (unsigned) cm->cm_magic);
    printf(" version     : %u.%u\n", (unsigned)(cm->cm_version / 10), (unsigned)(cm->cm_version % 10));
    printf(" builtin     : %lu\n", (unsigned long)cm->cm_builtin);
    printf(" signature   : 0x%08lx\n", (unsigned long)cm->cm_signature);
    printf(" num_macros  : %lu\n", (unsigned long)cm->cm_num_macros);
    printf(" num_atoms   : %lu\n", (unsigned long)cm->cm_num_atoms);
    printf(" globals     : %lu\n", (unsigned long)cm->cm_globals);
    printf(" num_strings : %lu\n", (unsigned long)cm->cm_num_strings);

    if (cm->cm_version != cm_version) {
        fprintf(stderr, "%s file has wrong version number\n", CM_EXTENSION);
        fprintf(stderr, "Current version is %d\n", cm_version);
        exit(1);
    }

    if (cm->cm_builtin != builtin_count) {
        fprintf(stderr, "%s file has incorrect builtin macro count\n", CM_EXTENSION);
        fprintf(stderr, "Current version is %d\n", builtin_count);
        exit(1);
    }

    if (cm->cm_signature != builtin_signature) {
        fprintf(stderr, "%s file has incorrect builtin signature\n", CM_EXTENSION);
        fprintf(stderr, "Current version is %08x\n", builtin_signature);
        exit(1);
    }

    vm_offsets  = (uint32_t *) (cm + 1);
    WGET32_block(vm_offsets, cm->cm_num_macros + 2);

    baselp      = (const LIST *) (vm_offsets + cm->cm_num_macros + 2);
    soffsets    = (uint32_t *) (((char *) baselp) + vm_offsets[cm->cm_num_macros]);
    str_table   = (char *) (soffsets + cm->cm_num_strings);

    WGET32_block(soffsets, cm->cm_num_strings);
    if (CM_MAGIC != cm->cm_magic) {
        fprintf(stderr, "%s: invalid magic number\n", file);
        exit(1);
    }

    if (o_sflg) {
        goto end_of_function;
    }

    printf("\n");
    for (i = 0; i < cm->cm_num_macros; ++i) {
        printf("Macro %d, offset = atom #%d\n", i, (int) vm_offsets[i]);
    }

    printf("\n");
    printf("String table : %08x\n", (int) vm_offsets[cm->cm_num_macros]);
    printf("\n");

    nm = 0;
    lp = baselp;

    while (lp < baselp + cm->cm_num_atoms) {
        const LIST atom = *lp;

        str = "";
        if (F_STR == atom || F_LIT == atom) {
            const uint32_t off = LGET_INT(lp);

            str = str_table + soffsets[off];

        } else if (F_ID == atom) {
            const int id = LGET_ID(lp);

            assert(id >= 0 && id < (int)builtin_count);
            str = builtin[id].b_name;
            if (0 == strcmp(str, "macro")) {
                if (o_lflg) {
                    printf("\n*** Macro %d:\n", nm);
                }
                ++nm;
            }
        }

        patom(baselp, lp, str);

        if (0 == sizeof_atoms[atom]) {
            ++lp;
        } else {
            lp += sizeof_atoms[atom];
        }
    }

    if (0 == o_lflg)
        goto end_of_function;

    printf("\n");
    printf("String Table:\n");
    for (i = 0; i < cm->cm_num_strings; ++i) {
        printf("  String %2d: Offset=%04x ", i, (int) soffsets[i]);
        printf("'%s'\n", str_table + soffsets[i]);
    }
    printf("\n");

end_of_function:
    if (o_aflg) {
        print_perc();
    }
    chk_free(cm);
}


static void
list_macro(int level, const LIST *lp)
{
    level=level;
    lp=lp;
}


/*
 *  print_perc ---
 *      Print out table of percentage of each atom type used. Used for optimisations.
 */
static void
print_perc(void)
{
    int natoms = 0, total_size = 0;
    unsigned i;

    for (i = 0; i < (sizeof(num_atoms)/sizeof(num_atoms[0])); ++i) {
        natoms += num_atoms[i];
        total_size += num_atoms[i] * sizeof_atoms[i];
    }

    if (0 == natoms) natoms = 1;
    if (0 == total_size) total_size = 1;

    printf("Type          Count    %%count      size     %%size\n");
    for (i = 0; i < sizeof num_atoms / sizeof num_atoms[0]; ++i) {
        printf("%-10s %8d  %8ld  %8ld  %8ld\n",
            nameof_atoms[i], num_atoms[i], ((long) num_atoms[i] * 100) / natoms,
            (long) num_atoms[i] * sizeof_atoms[i], ((long) num_atoms[i] * sizeof_atoms[i] * 100) / total_size);
    }
    printf("Total:     %8d            %8d\n", natoms, total_size);
}


static void
loadmacro(const LIST *lp, int size)
{
    const char *macro_keywd;
    const LIST *lpn;
    int flags = 0;
    char name[64];

    if (macro_cnt >= (MAX_MACROS - 1)) {
        printf("Macro table full\n");
        return;
    }
    lpn = lp + sizeof_atoms[*lp];

    if (F_INT == *lpn) {
        flags = LGET_INT(lpn);
        lpn += sizeof_atoms[F_INT];
    }

    if (F_STR != *lpn && F_ID != *lpn) {
        cm_error("Macro must start with a name\n", NULL);
        exit(1);
    }

    strcpy(name, F_ID == *lpn ? builtin[LGET_ID(lpn)].b_name : (const char *) LGET_PTR(lpn));

    macro_keywd = (F_ID == *lp ? builtin[LGET_ID(lp)].b_name : (const char *) LGET_PTR(lp));

    if (0 != strcmp(macro_keywd, "macro") &&
            0 != strcmp(macro_keywd, "replacement")) {
        return;
    }

    if (0 == strcmp(macro_keywd, "macro") && *lpn == F_ID) {
        printf("Warning: '%s' redefines a builtin.\n", name);
    }

    macro_tbl[macro_cnt].m_flags = flags;
    macro_tbl[macro_cnt].m_name  = chk_salloc(name);
    macro_tbl[macro_cnt].m_size  = size;
    atom_count += size;

    if (o_lflg) {
        printf("Entering macro '%s'\n", name);
        list_macro(0, lp);
    }

    macro_tbl[macro_cnt].m_list = lp;
    ++macro_cnt;
}


static void
patom(const LIST *first, const LIST *lp, const char *str)
{
    const int atom_no = (lp - first);
    LIST atom;
    char lbuf[10];

    if (0 == o_lflg || NULL == lp) {
        return;
    }

    atom = *lp;
    ++num_atoms[atom];

    if ((F_ID == atom && 0 == strcmp("macro", str)) ||
            (F_STR == atom && 0 == strcmp("macro", str))) {
        printf("\n");
    }
    sprintf(lbuf, "0x%04x", atom);
    printf("Atom %04x: ", atom_no);

    if (o_Lflg) {
        switch (atom) {
        case F_HALT:
        case F_END:
            printf("[%02x/........] ", atom);
            break;
        case F_LIST:
        case F_ID:
            printf("[%02x/....%04x] ", atom, LGET_ID(lp));
            break;
        default:
            printf("[%02x/%08x] ", atom, (int)LGET32(lp));
            break;
        }
    }

    switch (atom) {
    case F_INT:
        printf("F_INT   %" ACCINT_FMT "\n", (accint_t)LGET_INT(lp));
        break;
    case F_LIT:
        printf("F_LIT   \"%s\"\n", str);
        break;
    case F_STR:
        printf("F_STR   \"%s\"\n", str);
        break;
    case F_FLOAT:
        printf("F_FLOAT %" ACCFLOAT_FMT "\n", (accfloat_t)LGET_FLOAT(lp));
        break;
    case F_ID: {
            const int id = LGET_ID(lp);
            const char *name = builtin[id].b_name;

            assert(id >= 0 && id < (int)builtin_count);
            printf("F_ID    %s\n", name);
        }
        break;
    case F_LIST: {
            const int llen = LGET_LEN(lp);

            printf("F_LIST  ");
            if (0 == llen) {
                printf("======\n");
            } else {
                printf("--> %x\n", atom_no + llen);
            }
        }
        break;
    case F_NULL:
        printf("F_NULL\n");
        break;
    case F_HALT:
        printf("F_HALT\n");
        break;
    case F_END:
        printf("F_END   *** End ***\n");
        break;
    default:
        printf("F_WHAT? <%02x>\n", atom);
        break;
    }
}


int
sys_read(int fd, char *b, int size)
{
    int n, osize = size;

    do {
        n = read(fd, b, size);
        size -= n;
        b += n;
    } while (n > 0);
    return osize - size;
}


void
ewprintf(const char *msg, ...)
{
    va_list argp;

    va_start(argp, msg);
    vfprintf(stderr, msg, argp);
    va_end(argp);
    fprintf(stderr, "\n");
}


void
errorf(const char *msg, ...)
{
    va_list argp;

    va_start(argp, msg);
    vfprintf(stderr, msg, argp);
    va_end(argp);
    fprintf(stderr, "\n");
}


void
panic(const char *msg, ...)
{
    va_list argp;

    va_start(argp, msg);
    fprintf(stderr, "PANIC: ");
    vfprintf(stderr, msg, argp);
    va_end(argp);
    fprintf(stderr, "\n");
    fflush(stderr);
    sys_abort();
}


void
sys_abort(void)
{
    abort();
}
/*end*/

