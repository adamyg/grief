/* -*- mode: cr; indent-width: 4; -*-
 * $Id: c.cr,v 1.30 2022/08/10 16:11:47 cvsuser Exp $
 * C/C++ Language support mode.
 *
 *
 *
 */

#include "../grief.h"
#include "../mode.h"

static list c_hier_list = {
        "Arity       Operator                                        Assoc",
        "-------------------------------------------------------------------",
        "binary   ()  []  ->  .                                      l -> r",
        "unary    !   ~   ++  --  -  (type)  *  &  sizeof            r -> l",
        "binary   *   /   %                                          l -> r",
        "binary   +   -                                              l -> r",
        "binary   <<  >>                                             l -> r",
        "binary   <   <=  >   >=                                     l -> r",
        "binary   ==  !=                                             l -> r",
        "binary   &                                                  l -> r",
        "binary   ^                                                  l -> r",
        "binary   |                                                  l -> r",
        "binary   &&                                                 l -> r",
        "binary   ||                                                 l -> r",
        "ternary  ?:                                                 r -> l",
        "binary   = += -= *= /= %= >>= <<= &= ^= |=                  r -> l",
        "binary   ,                                                  l -> r",
        "-------------------------------------------------------------------",
        "                                                     From K&R, p 49",
        "Notes:",
        "  I'm a programmer. My attitude starts with arrogance, holds steady",
        "  at condescension, and ends with hostility. Get used to it."
        };


static list cpp_hier_list = {
        "Operator  Description              Assoc   Example of Use",
        "----------------------------------------------------------------------",
        "::        scope resolution         l -> r  class_name::member",
        "::        global                           ::name",
        "----------------------------------------------------------------------",
        ".         member selection         l -> r  object.member",
        "->        member selection                 pointer->member",
        "[]        subscripting                     pointer->member",
        "()        function call                    expr(expr_list)",
        "()        value construction               type(expr_list)",
        "sizeof    size of object                   sizeof(expr)",
        "sizeof    size of type                     sizeof(type)",
        "----------------------------------------------------------------------",
        "++        post increment           r -> l  lvalue ++",
        "++        pre increment                    ++ lvalue",
        "--        post decrement                   lvalue --",
        "--        pre decrement                    -- lvalue",
        "~         complement                       ~ expr",
        "!         not                              ! expr",
        "-         unary minus                      - expr",
        "+         unary plus                       + expr",
        "&         address of                       & lvalue",
        "*         dereference                      * expr",
        "new       create (allocate)                new type",
        "delete    destroy (de-allocate)            delete pointer",
        "delete[]  destroy (de-allocate)            delete[] pointer",
        "()        cast (type conversion)           (type) expr",
        "----------------------------------------------------------------------",
        ".*        member section           l -> r  object.*pointer-to-member",
        "->*       member section                   pointer->*pointer-to-member",
        "----------------------------------------------------------------------",
        "*         multiply                 l -> r  expr * expr",
        "/         divide                           expr / expr",
        "%         modulo (remainder)               expr % expr",
        "----------------------------------------------------------------------",
        "+         add (plus)               l -> r  expr + expr",
        "-         subtract(minus)                  expr - expr",
        "----------------------------------------------------------------------",
        "<<        shift left               l -> r  expr << expr",
        ">>        shift right                      expr >> expr",
        "----------------------------------------------------------------------",
        "<         less than                l -> r  expr > expr",
        "<=        less than or equal               expr <= expr",
        ">         greater than                     expr > expr",
        ">=        greater than or equal            expr >= expr",
        "----------------------------------------------------------------------",
        "==        equal                    l -> r  expr == expr",
        "!=        not equal                        expr != expr",
        "----------------------------------------------------------------------",
        "&         bitwise AND              l -> r  expr & expr",
        "----------------------------------------------------------------------",
        "^         bitwise exclusive OR     l -> r  expr ^ expr",
        "----------------------------------------------------------------------",
        "|         bitwise inclusive OR     l -> r  expr | expr",
        "----------------------------------------------------------------------",
        "&&        logical AND              l -> r  expr && expr",
        "----------------------------------------------------------------------",
        "||        logical inclusive OR     l -> r  expr || expr",
        "----------------------------------------------------------------------",
        "? :       conditional expression   r -> l  expr ? expr : expr",
        "----------------------------------------------------------------------",
        "=         simple assignment        r -> l  lvalue = expr",
        "*=        multiply and assign              lvalue *= expr",
        "/=        divide and assign                lvalue /= expr",
        "%=        modulo and assign                lvalue %= expr",
        "+=        add and assign                   lvalue += expr",
        "-=        subtract and assign              lvalue -= expr",
        "<<=       shift left and assign            lvalue <<= expr",
        ">>=       shift right and assign           lvalue >>= expr",
        "&=        AND and assign                   lvalue &= expr",
        "|=        inclusive OR and assign          lvalue |= expr",
        "^=        exclusive OR and assign          lvalue ^= expr",
        "----------------------------------------------------------------------",
        "throw     throw exception          l -> r  throw expr",
        "----------------------------------------------------------------------",
        ",         comma (sequencing)       l -> r  expr, expr",
        "----------------------------------------------------------------------",
        "From The C++ Progamming Language 2nd Edition pg 89-90",
        ""
        };


#define MIN_ABBREV      2                       /* Increase this to specify longer
                                                 * default minimum abbreviations */

#if defined(__PROTOTYPES__)
string          _c_prevline(void);
int             _c_indent(void);
void            _c_abbrev(void);
void            _c_brace_expand(void);
void            _c_open_brace(void);
void            _c_close_brace(void);
void            _c_align_comments(void);
void            _c_aligncomment(void);
void            _c_preprocessor(void);

static int      indexn(string s, int start, string subs);

static void     keywords_kr(void);
static void     keywords_gnu(void);
static void     keywords_c90(void);
static void     keywords_c99(void);
static void     keywords_c11(void);
static void     keywords_posix1_2001(void);
static void     keywords_nonposix(void);
#endif

static int      _c_min_abbrev,
                _c_smart,                       /* C smart keyboard handle */
                _c_template,                    /* C template keyboard handle */
                _c_indent_open,
                _c_indent_close,
                _c_indent_first;

static int      _c_align_column = 78;           /* column to right align on */

#define MODENAME "C"

extern void     doxygen_keyword();

void
main()
{
    /*
     *  Syntax lexer/
     *      utilised during basic line pre-processing.
     */
    create_syntax(MODENAME);
    syntax_token(SYNT_COMMENT,      "/*", "*/");
    syntax_token(SYNT_COMMENT,      "//");
    syntax_token(SYNT_STRING,       "\"");
    syntax_token(SYNT_CHARACTER,    "'");
    syntax_token(SYNT_QUOTE,        "\\");
    syntax_token(SYNT_LINECONT,     "\\");
    syntax_token(SYNT_PREPROCESSOR, "#");
    syntax_token(SYNT_BRACKET,      "([{", ")]}");

    /*
     *  Options/
     *      SYNF_COMMENTS_CSTYLE
     *          Ignore leading white-space on comments.
     *
     *      SYNF_COMMENTS_QUOTE
     *          Allow quoting of comment terminator.
     *
     *      SYNF_LINECONT_WS
     *          Continuation, allows trailing white-space.
     */
    set_syntax_flags(SYNF_COMMENTS_CSTYLE|SYNF_COMMENTS_QUOTE|SYNF_LINECONT_WS);

    /*
     *  DFA based syntax engine/
     *      used to built a DFA based lexer/parser, which are generally faster.
     */
    syntax_rule("^[ \t]*#", "pp:preprocessor");

                                                // comments (open, block, block-unmatched and eol)
    syntax_rule("/\\*.*$", "spell,todo:comment");
    syntax_rule("/\\*.*\\*/", "spell,todo,quick:comment");
    syntax_rule("//.*$", "spell,todo:comment");

  //syntax_rule("/[^*/].*$", "alert");          // invalid eol comment; div op's cause confusion.
    syntax_rule("\\*/", "quick:alert");         // unmatched block comment.

                                                // keywords and preprocessor directives
    syntax_rule("[A-Za-z_][A-Za-z_0-9]*", "keyword,directive:normal");

                                                // numeric constants (hex, oct, dec and errors)
        //  suffixes:
        //      l -             long
        //      u -             unsigned
        //      ul -            unsigned long
        //      ll -            long long
        //      ull -           unsigned long long
        //
    syntax_rule("[1-9][0-9]*[lLuU]?[lLuU]?[lLuU]?", "number");
    syntax_rule("0[xX][0-9A-Fa-f]+[lLuU]?[lLuU]?[lLuU]?", "number");
    syntax_rule("0[0-7]+[lLuU]?[lLuU]?[lLuU]?", "number");
    syntax_rule("0[8-9][0-9]*", "alert");
    syntax_rule("0[xX]", "alert");
    syntax_rule("0", "number");

    // floating point
        //
        //  [<integral>][.<decimal>][<exponent>][<suffix>][<imaginary>]
        //
        //      suffix -        float (f or F) and long double (l or L).
        //      imaginary -     i,I,j or J.
        //
    syntax_rule("[0-9]+\\.[0-9]*([Ee][-+]?[0-9]*)?[fFlL]?[iIjJ]?", "float");
    syntax_rule("[0-9]+[Ee][-+]?[0-9]*[fFlL]?[iIjJ]?", "float");
    syntax_rule("\\.[0-9]+([Ee][-+]?[0-9]*)?[fFlL]?[iIjJ]?", "alert");

                                                // strings (block, open/continued)
    syntax_rule("\"(\\\\.|[^\\\"])*\"", "string");
    syntax_rule("\"(\\\\.|[^\\\\\"])*[^\\\\ \\\"\t\n]+", "string");

    syntax_rule("\'\\\\[\\\']\'", "character"); // '\\', '\''
    syntax_rule("\'[^\']+\'", "character");     // 'x[xxxx]'

    syntax_rule("[()\\[\\]{},;.?:]", "delimiter");
    syntax_rule("[-%" + "+/&*=<>|!~^]+", "operator");

    syntax_rule("\\\\[ \t]+$", "whitespace");   // trailing white-space after continuation.

    // comment elements
    syntax_rule("htt[ps]:[^ ]+",
        "group=comment:link");

    syntax_rule("ftp:[^ ]+",
        "group=comment:link");

    syntax_rule("file:[^ ]+",
        "group=comment:link");

    doxygen_keyword();

    // string elements
    syntax_rule("\\\\x\\x+",                    // hexadecimal escapes.
        "group=string:constant_standout");

    syntax_rule("\\\\x[^A-Za-z0-9]",            // illegal hexadecimal escapes.
        "group=string:alert");

    syntax_rule("\\\\x{\\x+}",                  // extended hexdecimal escapes.
        "group=string:constant_standout");

    syntax_rule("\\\\u\\x\\x\\x\\x",            // universal character name, escapes.
        "group=string,quick:constant_standout");

    syntax_rule("\\\\U\\x\\x\\x\\x\\x\\x\\x\\x",
        "group=string,quick:constant_standout");

    syntax_rule("\\\\[uU][A-Za-z0-9]+",         // illegal unicode character escapes.
        "group=string:alert");

    syntax_rule("\\\\[0-7]+",                   // octal escapes.
        "group=string:constant_standout");

    syntax_rule("\\\\o{[0-7]+}",                // extended octal escapes.
        "group=string:constant_standout");

    syntax_rule("\\\\c[\\[a-zA_Z?]",            // control character escapes.
        "group=string:constant_standout");

    syntax_rule("\\\\['\"?\\\\abefnrtv]",       // character escapes (includes known extensions).
        "group=string:constant_standout");

    syntax_rule("\\\\[^'\"?\\\\abefnrtvux0]",   // illegal character escapes.
        "group=string:alert");

    syntax_rule("\\\\",                         // omitted character escapes.
        "group=string:alert");

    syntax_rule("%%",                           // escaped format.
        "group=string,quick:constant_standout");

                                                // format specifications (includes c99).
    syntax_rule("%" + "[-+ #0']?[0-9]*\\.[0-9]*[hlL]*[bdiuoxXDOUfeEgGcCsSpn]",
        "group=string,quick:constant_standout");

    syntax_rule("%" + "[-+ #0']?[0-9]*[hlL]*[bdiuoxXDOUfeEgGcCsSpn]",
        "group=string,quick:constant_standout");

    syntax_rule("%" + "[-+ #0']?[*]?\\.[*]?[hlL]*[bdiuoxXDOUfeEgGcCsSpn]",
        "group=string,quick:constant_standout");

    syntax_rule("%" + "[-+ #0']?[*]?[hlL]*[bdiuoxXDOUfeEgGcCsSpn]",
        "group=string,quick:constant_standout");

    syntax_rule("%" + "[^ \"]+",                // non-standard character escapes.
        "group=string:alert");

    // build and auto-cache
    syntax_build(__COMPILETIME__);

    /*
     *  keywords
     */

    // reserved - language primitives
    define_keywords(SYNK_PRIMARY,
         "new,delete,this,true,false,"+
         "and,and_eq,asm,bitand,bitor,compl,not,not_eq,"+
         "or,or_eq,return,sizeof,typeid,using,xor,xor_eq");

    define_keywords(SYNK_PRIMARY,
         "const_cast,static_cast,dynamic_cast,reinterpret_cast");

    define_keywords(SYNK_TYPE,
         "auto,bool,char,double,enum,float,int,"+
         "long,short,signed,unsigned,void,wchar_t");

    define_keywords(SYNK_STORAGECLASS,
         "auto,const,export,extern,friend,inline,mutable,"+
         "private,protected,public,register,static,volatile");

    define_keywords(SYNK_DEFINITION,
         "class,explicit,namespace,operator,struct,"+
         "template,typedef,typename,union,virtual");

    define_keywords(SYNK_CONDITIONAL,
         "case,default,else,goto,if,switch");

    define_keywords(SYNK_REPEAT,
         "break,continue,do,for,while");

    define_keywords(SYNK_EXCEPTION,
         "catch,throw,try");

    // functions - commonly used libc functions.
    define_keywords(SYNK_FUNCTION,
        "EOFabscosdivexplogpowsintan", 3);

    define_keywords(SYNK_FUNCTION,
        "FILENULLacosasinatanatofatoiatolceilcosh"+
        "exitfabsfeoffmodfreegetcgetslabsldivmodf"+
        "putcputsrandsinhsqrttanhtime", 4);

    define_keywords(SYNK_FUNCTION,
        "abortatan2clockctimediv_terrnofgetcfgets"+
        "floorfopenfputcfputsfreadfrexpfseekftell"+
        "qsort", 5);

    define_keywords(SYNK_FUNCTION,
        "assertatexitcallocfcloseferrorfflushfscanf"+
        "fwritegetenvgmtimemallocmemchrmemcmpmemcpy"+
        "memsetmktimeperrorprintfremoverenamerewind"+
        "setbufsetjmpsignalsize_tsscanfstderrstdout"+
        "strcatstrchrstrcmpstrcpystrdupstrlenstrspn"+
        "strstrstrtodstrtokstrtolsystemtime_ttmpnam"+
        "ungetcva_argva_end", 6);

    define_keywords(SYNK_FUNCTION,
        "asctimebsearchclock_tfgetposfprintffreopen"+
        "fsetposgetcharisalnumisalphaiscntrlisdigit"+
        "isgraphislowerisprintispunctisspaceisupper"+
        "jmp_buflongjmpmemmoveputcharreallocsetvbuf"+
        "sprintfstrcspnstrncatstrncmpstrncpystrpbrk"+
        "strrchrstrtoultmpfiletolowertoupperva_list"+
        "vprintf", 7);

    define_keywords(SYNK_FUNCTION,
        "clearerrdifftimeisxdigitstrerror"+
        "strftimeva_startvfprintfvsprintf", 8);

    define_keywords(SYNK_FUNCTION,
        "localtime", 9);

    define_keywords(SYNK_TODO,
        "XXX,TODO,FIXME,DEPRECATED,MAGIC,HACK");

    // Type2
    keywords_gnu();
    keywords_c90();
    keywords_c99();
    keywords_c11();
    keywords_posix1_2001();
    keywords_nonposix();

    // pre-preprocessor primitives
    define_keywords(SYNK_PREPROCESSOR, "line", -4);
    define_keywords(SYNK_PREPROCESSOR, "error,undef", -5);
    define_keywords(SYNK_PREPROCESSOR, "pragma", -6);
    define_keywords(SYNK_PREPROCESSOR, "defined", -7);
    define_keywords(SYNK_PREPROCESSOR, "__DATE__,__FILE__,__FUNCTION__,__LINE__,__TIME__");
    define_keywords(SYNK_PREPROCESSOR, "__bool_true_false_are_defined");

    define_keywords(SYNK_PREPROCESSOR_DEFINE, "define", -6);
    define_keywords(SYNK_PREPROCESSOR_INCLUDE, "include", -7);

    define_keywords(SYNK_PREPROCESSOR_CONDITIONAL, "if", -2);
    define_keywords(SYNK_PREPROCESSOR_CONDITIONAL, "elif,else", -4);
    define_keywords(SYNK_PREPROCESSOR_CONDITIONAL, "endif,ifdef,undef", -5);
    define_keywords(SYNK_PREPROCESSOR_CONDITIONAL, "ifndef", -6);
    define_keywords(SYNK_PREPROCESSOR_CONDITIONAL, "defined", -7);

    /* prereq's */
    load_indent();
    load_compile();

    /*
     *  keyboard
     */

    // set up smart indenting for C
    keyboard_push();
    assign_to_key("<Enter>",            "_c_indent");
    assign_to_key("<Tab>",              "_slide_in");
    assign_to_key("<Shift-Tab>",        "_slide_out");
    assign_to_key("<{>",                "_c_open_brace");
    assign_to_key("<}>",                "_c_close_brace");
    assign_to_key("<Alt-=>",            "_c_align_comments");
    assign_to_key("<#>",                "_c_preprocessor");
//  assign_to_key("<Ctrl-{>",           "_just_obrace");
//  assign_to_key("<Ctrl-}>",           "_just_cbrace");
//  assign_to_key("<Ctrl-S>",           "_just_space");
    _c_smart = inq_keyboard();
    keyboard_pop(1);

    /* Set up template indenting for C */
    keyboard_push();
    assign_to_key("<Enter>",            "_c_indent");
    assign_to_key("<Tab>",              "_slide_in");
    assign_to_key("<Shift-Tab>",        "_slide_out");
    assign_to_key("<Space>",            "_c_abbrev");
    assign_to_key("<{>",                "_c_brace_expand");
    assign_to_key("<}>",                "_c_close_brace");
    assign_to_key("<Alt-=>",            "_c_align_comments");
    assign_to_key("<#>",                "_c_preprocessor");
//  assign_to_key("<Ctrl-{>",           "_just_obrace");
//  assign_to_key("<Ctrl-}>",           "_just_cbrace");
//  assign_to_key("<Ctrl-S>",           "_just_space");
    _c_template = inq_keyboard();
    keyboard_pop(1);
}


static void
keywords_kr(void)
{
}


static void
keywords_gnu(void)
{
    define_keywords(SYNK_PRIMARY,
        "__asm__");

    define_keywords(SYNK_OPERATOR,
        "typeof,__real__,__imag__");

    define_keywords(SYNK_TYPE,
        "__label__,__complex__,__volatile__,"+
        "__int28,"+
        "_float80,__float128,"+
        "__fp16,"+
        "_Decimal32,_Decimal64,_Decimal128");

    define_keywords(SYNK_CONSTANT,
        "inline,__attribute__,__alignof__");

    //  Fixed-Point Types
    //
    //  As an extension, the GNU C compiler supports fixed-point types as defined in the N1169
    //  draft of ISO/IEC DTR 18037. Support for fixed-point types in GCC will evolve as the draft
    //  technical report changes. Calling conventions for any target might also change. Not all
    //  targets support fixed-point types.
    //
    //        The fixed-point types are
    //
    //      short _Fract, _Fract, long _Fract, long long _Fract, unsigned short _Fract, unsigned
    //      _Fract, unsigned long _Fract, unsigned long long _Fract, _Sat short _Fract, _Sat _Fract,
    //      _Sat long _Fract, _Sat long long _Fract, _Sat unsigned short _Fract, _Sat unsigned
    //      _Fract, _Sat unsigned long _Fract, _Sat unsigned long long _Fract, short _Accum, _Accum,
    //      long _Accum, long long _Accum, unsigned short _Accum, unsigned _Accum, unsigned long
    //      _Accum, unsigned long long _Accum, _Sat short _Accum, _Sat _Accum, _Sat long _Accum, _Sat
    //      long long _Accum, _Sat unsigned short _Accum, _Sat unsigned _Accum, _Sat unsigned long
    //      _Accum, _Sat unsigned long long _Accum.
    //
    define_keywords(SYNK_TYPE,
        "_Fract,_Sat,_Accum");

    define_keywords(SYNK_CONSTANT,
        "__FUNCTION__,__PRETTY_FUNCTION__,__func__");

    define_keywords(SYNK_CONSTANT,
        "__GNUC__,__GNUC_MINOR__,__GNUC_PATCHLEVEL__,"+
        "__clang__,__clang_major__,__clang_minor__,__clang_patchlevel__,__clang_version__,"+
        "WIN32,_WIN32,_WIN32_WINNT,WIN32_MEAN_AND_LEAN,"+
        "__MSDOS__,"+
        "__CYGWIN__,"+
        "__MINGW32__,__MINGW32_VERSION_MAJOR,__MINGW32_VERSION_MINOR,"+
        "__MINGW64__,__MINGW64_VERSION_MAJOR,__MINGW64_VERSION_MINOR,"+
        "__WATCOMC__,"+
        "_MSC_VER,"+
        "__BORLANDC__,"+
        "__SUNPRO_C,__SUNPRO_CC,"+
        "unix,"+
        "hpux,"+
        "linux,__linux__,"+
        "__APPLE__,"+
        "_AIX");

    define_keywords(SYNK_FUNCTION,
        "__builtin_apply_args,"+
        "__builtin_apply,"+
        "__builtin_return,"+
        "__builtin_return_address,"+
        "__builtin_frame_address,"+
        "__builtin_constant_p,"+
        "__builtin_types_compatible_p,"+
        "__builtin_choose_expr,"+
        "__builtin_expect,"+
        "__builtin_prefetch");

    //  GNU extensions
    //
    define_keywords(SYNK_FUNCTION,
        "strverscmp");
}


static void
keywords_c90(void)
{
    define_keywords(SYNK_CONSTANT,
        "__STDC_VERSION__");
}


static void
keywords_c99(void)
{
    int idx, base;

    define_keywords(SYNK_CONSTANT,
        "__func__");

    define_keywords(SYNK_PREPROCESSOR,
        "_Pragma,__STDC_IEC_559__,__STDC_IEC_559__COMPLEX__,__STDC_ISO_10646__,__VA_ARGS__");

    define_keywords(SYNK_STORAGECLASS,
        "restrict,__restrict,"+
        "inline,__inline");

    // B.1 Diagnostics <assert.h>
    define_keywords(SYNK_CONSTANT,
        "NDEBUG");

    define_keywords(SYNK_FUNCTION,
        "assert");

    // B.2 Complex <complex.h>
    define_keywords(SYNK_TYPE,
        "_Bool,"+
        "complex,"+
        "_Complex,"+
        "_Complex_I,"+
        "imaginary,"+
        "_Imaginary,"+
        "_Imaginary_I");

    define_keywords(SYNK_FUNCTION,
        "cabs,"+
        "cabsf,"+
        "cabsl,"+
        "cacos,"+
        "cacosf,"+
        "cacosh,"+
        "cacoshf,"+
        "cacoshl,"+
        "cacosl,"+
        "carg,"+
        "cargf,"+
        "cargl,"+
        "casin,"+
        "casinf,"+
        "casinh,"+
        "casinhf,"+
        "casinhl,"+
        "casinl,"+
        "catan,"+
        "catanf,"+
        "catanh,"+
        "catanhf,"+
        "catanhl,"+
        "catanl,"+
        "ccos,"+
        "ccosf,"+
        "ccosh,"+
        "ccoshf,"+
        "ccoshl,"+
        "ccosl,"+
        "cexp,"+
        "cexpf,"+
        "cexpl,"+
        "cimag,"+
        "cimagf,"+
        "cimagl,"+
        "clog,"+
        "clogf,"+
        "clogl,"+
        "conj,"+
        "conjf,"+
        "conjl,"+
        "cpow,"+
        "cpowf,"+
        "cproj,"+
        "cprojf,"+
        "cprojl,"+
        "creal,"+
        "crealf,"+
        "creall,"+
        "csin,"+
        "csinf,"+
        "csinh,"+
        "csinhf,"+
        "csinhl,"+
        "csinl,"+
        "csqrt,"+
        "csqrtf,"+
        "csqrtl,"+
        "ctan,"+
        "ctanf,"+
        "ctanh,"+
        "ctanhf,"+
        "ctanhl,"+
        "ctanl");

    // B.3 Character handling <ctype.h>
    define_keywords(SYNK_FUNCTION,
        "isalnum,"+
        "isalpha,"+
        "isblank,"+
        "iscntrl,"+
        "isdigit,"+
        "isgraph,"+
        "islower,"+
        "isprint,"+
        "ispunct,"+
        "isspace,"+
        "isupper,"+
        "isxdigit,"+
        "tolower,"+
        "toupper");

    // B.4 Errors <errno.h>
    define_keywords(SYNK_CONSTANT,
        "EDOM,"+
        "EILSEQ,"+
        "ERANGE");

    define_keywords(SYNK_FUNCTION,
        "errno");

    // B.5 Floating-point environment <fenv.h>
    define_keywords(SYNK_CONSTANT,
        "FE_DIVBYZERO,"+
        "FE_INEXACT,"+
        "FE_INVALID,"+
        "FE_OVERFLOW,"+
        "FE_UNDERFLOW,"+
        "FE_ALL_EXCEPT,"+
        "FE_DOWNWARD,"+
        "FE_TONEAREST,"+
        "FE_TOWARDZERO,"+
        "FE_UPWARD,"+
        "FE_DFL_ENV");

    define_keywords(SYNK_FUNCTION,
        "fenv_t,"+
        "fexcept_t,"+
        "feclearexcept,"+
        "fegetexceptflag,"+
        "feraiseexcept,"+
        "fesetexceptflag,"+
        "fetestexcept,"+
        "fegetround,"+
        "fesetround,"+
        "fegetenv,"+
        "feholdexcept,"+
        "fesetenv,"+
        "feupdateenv");

    // B.6 Characteristics of floating types <float.h>
    define_keywords(SYNK_CONSTANT,
        "FLT_ROUNDS,"+
        "FLT_EVAL_METHOD,"+
        "FLT_RADIX,"+
        "FLT_MANT_DIG,"+
        "DBL_MANT_DIG,"+
        "LDBL_MANT_DIG,"+
        "DECIMAL_DIG,"+
        "FLT_DIG,"+
        "DBL_DIG,"+
        "LDBL_DIG,"+
        "FLT_MIN_EXP,"+
        "DBL_MIN_EXP,"+
        "LDBL_MIN_EXP,"+
        "FLT_MIN_10_EXP,"+
        "DBL_MIN_10_EXP,"+
        "LDBL_MIN_10_EXP,"+
        "FLT_MAX_EXP,"+
        "DBL_MAX_EXP,"+
        "LDBL_MAX_EXP,"+
        "FLT_MAX_10_EXP,"+
        "DBL_MAX_10_EXP,"+
        "LDBL_MAX_10_EXP,"+
        "FLT_MAX,"+
        "DBL_MAX,"+
        "LDBL_MAX,"+
        "FLT_EPSILON,"+
        "DBL_EPSILON,"+
        "LDBL_EPSILON,"+
        "FLT_MIN,"+
        "DBL_MIN,"+
        "LDBL_MIN");

    // B.7 Format conversion of integer types <inttypes.h>
    define_keywords(SYNK_FUNCTION,
        "imaxdiv_t,"+
        "imaxabs,"+
        "imaxdiv,"+
        "strtoimax,"+
        "strtoumax,"+
        "wcstoimax,"+
        "wcstoumax");

    define_keywords(SYNK_CONSTANT,
        "PRIdN,PRIdLEASTN,PRIdFASTN,PRIdMAX,PRIdPTR,"+
        "PRIiN,PRIiLEASTN,PRIiFASTN,PRIiMAX,PRIiPTR,"+
        "PRIoN,PRIoLEASTN,PRIoFASTN,PRIoMAX,PRIoPTR,"+
        "PRIuN,PRIuLEASTN,PRIuFASTN,PRIuMAX,PRIuPTR,"+
        "PRIxN,PRIxLEASTN,PRIxFASTN,PRIxMAX,PRIxPTR,"+
        "PRIXN,PRIXLEASTN,PRIXFASTN,PRIXMAX,PRIXPTR,"+
        "SCNdN,SCNdLEASTN,SCNdFASTN,SCNdMAX,SCNdPTR,"+
        "SCNiN,SCNiLEASTN,SCNiFASTN,SCNiMAX,SCNiPTR,"+
        "SCNoN,SCNoLEASTN,SCNoFASTN,SCNoMAX,SCNoPTR,"+
        "SCNuN,SCNuLEASTN,SCNuFASTN,SCNuMAX,SCNuPTR,"+
        "SCNxN,SCNxLEASTN,SCNxFASTN,SCNxMAX,SCNxPTR");

    // B.8 Alternative spellings <iso646.h>
    define_keywords(SYNK_OPERATOR,
        "and,"+
        "and_eq,"+
        "bitand,"+
        "bitor,"+
        "compl,"+
        "not,"+
        "not_eq,"+
        "or,"+
        "or_eq,"+
        "xor,"+
        "xor_eq");

    // B.9 Sizes of integer types <limits.h>
    define_keywords(SYNK_CONSTANT,
        "CHAR_BIT,"+
        "SCHAR_MIN,"+
        "SCHAR_MAX,"+
        "UCHAR_MAX,"+
        "CHAR_MIN,"+
        "CHAR_MAX,"+
        "MB_LEN_MAX,"+
        "SHRT_MIN,"+
        "SHRT_MAX,"+
        "USHRT_MAX,"+
        "INT_MIN,"+
        "INT_MAX,"+
        "UINT_MAX,"+
        "LONG_MIN,"+
        "LONG_MAX,"+
        "ULONG_MAX,"+
        "LLONG_MIN,"+
        "LLONG_MAX,"+
        "ULLONG_MAX");

    // B.10 Localization <locale.h>
    define_keywords(SYNK_CONSTANT,
        "NULL,"+
        "LC_ALL,"+
        "LC_COLLATE,"+
        "LC_CTYPE,"+
        "LC_MONETARY,"+
        "LC_NUMERIC,"+
        "LC_TIME");

    define_keywords(SYNK_FUNCTION,
        "setlocale,"+
        "localeconv");

    // B.11 Mathematics <math.h>
    define_keywords(SYNK_TYPE,
        "float_t,"+
        "double_t");

    define_keywords(SYNK_CONSTANT,
        "HUGE_VAL,"+
        "HUGE_VALF,"+
        "HUGE_VALL,"+
        "INFINITY,"+
        "NAN,"+
        "FP_INFINITE,"+
        "FP_NAN,"+
        "FP_NORMAL,"+
        "FP_SUBNORMAL,"+
        "FP_ZERO,"+
        "FP_FAST_FMA,"+
        "FP_FAST_FMAF,"+
        "FP_FAST_FMAL,"+
        "FP_ILOGB0,"+
        "FP_ILOGBNAN,"+
        "MATH_ERRNO,"+
        "MATH_ERREXCEPT");

    define_keywords(SYNK_FUNCTION,
        "math_errhandling,"+
        "fpclassify,"+
        "isfinite,"+
        "isinf,"+
        "isnan,"+
        "isnormal,"+
        "signbit,"+
        "acos,"+
        "acosf,"+
        "acosl,"+
        "asin,"+
        "asinf,"+
        "asinl,"+
        "atan,"+
        "atanf,"+
        "atanl,"+
        "atan2,"+
        "atan2f,"+
        "atan2l,"+
        "cos,"+
        "cosf,"+
        "cosl,"+
        "sin,"+
        "sinf,"+
        "sinl,"+
        "tan,"+
        "tanf,"+
        "tanl,"+
        "acosh,"+
        "acoshf,"+
        "acoshl,"+
        "asinh,"+
        "asinhf,"+
        "asinhl,"+
        "atanh,"+
        "atanhf,"+
        "atanhl,"+
        "cosh,"+
        "coshf,"+
        "coshl,"+
        "sinh,"+
        "sinhf,"+
        "sinhl,"+
        "tanh,"+
        "tanhf,"+
        "tanhl,"+
        "exp,"+
        "expf,"+
        "expl,"+
        "exp2,"+
        "exp2f,"+
        "exp2l,"+
        "expm1,"+
        "expm1f,"+
        "expm1l,"+
        "frexp,"+
        "frexpf,"+
        "frexpl,"+
        "ilogb,"+
        "ilogbf,"+
        "ilogbl,"+
        "ldexp,"+
        "ldexpf,"+
        "ldexpl,"+
        "log,"+
        "logf,"+
        "logl,"+
        "log10,"+
        "log10f,"+
        "log10l,"+
        "log1p,"+
        "log1pf,"+
        "log1pl,"+
        "log2,"+
        "log2f,"+
        "log2l,"+
        "logb,"+
        "logbf,"+
        "logbl,"+
        "modf,"+
        "modff,"+
        "modfl,"+
        "scalbn,"+
        "scalbnf,"+
        "scalbnl,"+
        "scalbln,"+
        "scalblnf,"+
        "scalblnl,"+
        "cbrt,"+
        "cbrtf,"+
        "cbrtl,"+
        "fabs,"+
        "fabsf,"+
        "fabsl,"+
        "hypot,"+
        "hypotf,"+
        "hypotl,"+
        "pow,"+
        "powf,"+
        "powl,"+
        "sqrt,"+
        "sqrtf,"+
        "sqrtl,"+
        "erf,"+
        "erff,"+
        "erfl,"+
        "erfc,"+
        "erfcf,"+
        "erfcl,"+
        "lgamma,"+
        "lgammaf,"+
        "lgammal,"+
        "tgamma,"+
        "tgammaf,"+
        "tgammal,"+
        "ceil,"+
        "ceilf,"+
        "ceill,"+
        "floor,"+
        "floorf,"+
        "floorl,"+
        "nearbyint,"+
        "nearbyintf,"+
        "nearbyintl,"+
        "rint,"+
        "rintf,"+
        "rintl,"+
        "lrint,"+
        "lrintf,"+
        "lrintl,"+
        "llrint,"+
        "llrintf,"+
        "llrintl,"+
        "round,"+
        "roundf,"+
        "roundl,"+
        "lround,"+
        "lroundf,"+
        "lroundl,"+
        "llround,"+
        "llroundf,"+
        "llroundl,"+
        "trunc,"+
        "truncf,"+
        "truncl,"+
        "fmod,"+
        "fmodf,"+
        "fmodl,"+
        "remainder,"+
        "remainderf,"+
        "remainderl,"+
        "remquo,"+
        "remquof,"+
        "remquol,"+
        "copysign,"+
        "copysignf,"+
        "copysignl,"+
        "nan,"+
        "nanf,"+
        "nanl,"+
        "nextafter,"+
        "nextafterf,"+
        "nextafterl,"+
        "nexttoward,"+
        "nexttowardf,"+
        "nexttowardl,"+
        "fdim,"+
        "fdimf,"+
        "fdiml,"+
        "fmax,"+
        "fmaxf,"+
        "fmaxl,"+
        "fmin,"+
        "fminf,"+
        "fminl,"+
        "fma,"+
        "fmaf,"+
        "fmal,"+
        "isgreater,"+
        "isgreaterequal,"+
        "isless,"+
        "islessequal,"+
        "islessgreater,"+
        "isunordered");

    // B.12 Nonlocal jumps <setjmp.h>
    define_keywords(SYNK_FUNCTION,
        "jmp_buf,"+
        "setjmp,"+
        "longjmp");

    // B.13 Signal handling <signal.h>
    define_keywords(SYNK_FUNCTION,
        "sig_atomic_t,"+
        "raise");

    define_keywords(SYNK_CONSTANT,
        "SIG_DFL,"+
        "SIG_ERR,"+
        "SIG_IGN,"+
        "SIGABRT,"+
        "SIGFPE,"+
        "SIGILL,"+
        "SIGINT,"+
        "SIGSEGV,"+
        "SIGTERM");

        //XXX - other 'unistd.h' SIGxxx values
        //

    // B.14 Variable arguments <stdarg.h>
    define_keywords(SYNK_FUNCTION,
        "va_list,"+
        "va_arg,"+
        "va_copy,"+
        "va_end,"+
        "va_start");

    // B.15 Boolean type and values <stdbool.h>
    define_keywords(SYNK_FUNCTION,
        "bool,"+
        "true,"+
        "false");

    // B.16 Common definitions <stddef.h>
    define_keywords(SYNK_FUNCTION,
        "ptrdiff_t,"+
        "size_t,"+
        "wchar_t,"+
        "NULL,"+
        "offsetof");

    // B.17 Integer types <stdint.h>
    list stdint_types = {
        "int%u_t,",                             // e.g. int32_t
        "uint%u_t,",
        "int_least%u_t,",
        "uint_least%u_t,",
        "int_fast%u_t,",
        "uint_fast%u_t"};

    list stdint_constants = {
        "INT%u_MIN,",
        "INT%u_MAX,",
        "UINT%u_MAX,",
        "INT_LEAST%u_MIN,",
        "INT_LEAST%u_MAX,",
        "UINT_LEAST%u_MAX,",
        "INT_FAST%u_MIN,",
        "INT_FAST%u_MAX,",
        "UINT_FAST%u_MAX,",
        "INT%u_C,",
        "UINT%u_C"};

    for (base = 8; base <= 64; base <<= 1) {
        declare name;                           // 8, 16, 32 and 64
        string defs;

        defs = "";
        while ((idx = list_each(stdint_types, name)) >= 0) {
            defs += format(name, base);
        }
        define_keywords(SYNK_TYPE, defs);

        defs = "";
        while ((idx = list_each(stdint_constants, name)) >= 0) {
            defs += format(name, base);
        }
        define_keywords(SYNK_CONSTANT, defs);
    }

    define_keywords(SYNK_CONSTANT,
        "intptr_t,"+
        "uintptr_t,"+
        "intmax_t,"+
        "uintmax_t,"+
        "INTPTR_MIN,"+
        "INTPTR_MAX,"+
        "UINTPTR_MAX,"+
        "INTMAX_MIN,"+
        "INTMAX_MAX,"+
        "UINTMAX_MAX,"+
        "PTRDIFF_MIN,"+
        "PTRDIFF_MAX,"+
        "SIG_ATOMIC_MIN,"+
        "SIG_ATOMIC_MAX,"+
        "SIZE_MAX,"+
        "WCHAR_MIN,"+
        "WCHAR_MAX,"+
        "WINT_MIN,"+
        "WINT_MAX,"+
        "INTMAX_C,"+
        "UINTMAX_C");

    // B.18 Input/output <stdio.h>
    define_keywords(SYNK_CONSTANT,
        "size_t,"+
        "FILE,"+
        "fpos_t,"+
        "NULL,"+
        "_IOFBF,"+
        "_IOLBF,"+
        "_IONBF,"+
        "BUFSIZ,"+
        "EOF,"+
        "FOPEN_MAX,"+
        "FILENAME_MAX,"+
        "L_tmpnam,"+
        "SEEK_CUR,"+
        "SEEK_END,"+
        "SEEK_SET,"+
        "TMP_MAX,"+
        "stdin,"+
        "stderr,"+
        "stdout");

    define_keywords(SYNK_FUNCTION,
        "remove,"+
        "rename,"+
        "tmpfile,"+
        "tmpnam,"+
        "fclose,"+
        "fflush,"+
        "fopen,"+
        "freopen,"+
        "setbuf,"+
        "setvbuf,"+
        "fprintf,"+
        "fscanf,"+
        "printf,"+
        "scanf,"+
        "snprintf,"+
        "sprintf,"+
        "sscanf,"+
        "vfprintf,"+
        "vfscanf,"+
        "vprintf,"+
        "vscanf,"+
        "vsnprintf,"+
        "vsprintf,"+
        "vsscanf,"+
        "fgetc,"+
        "fgets,"+
        "fputc,"+
        "fputs,"+
        "getc,"+
        "getchar,"+
        "gets,"+
        "putc,"+
        "putchar,"+
        "puts,"+
        "ungetc,"+
        "fread,"+
        "fwrite,"+
        "fgetpos,"+
        "fseek,"+
        "fsetpos,"+
        "ftell,"+
        "rewind,"+
        "clearerr,"+
        "feof,"+
        "ferror,"+
        "perror");

    // B.19 General utilities <stdlib.h>
    define_keywords(SYNK_CONSTANT,
        "size_t,"+
        "wchar_t,"+
        "div_t,"+
        "ldiv_t,"+
        "lldiv_t,"+
        "NULL,"+
        "EXIT_FAILURE,"+
        "EXIT_SUCCESS,"+
        "RAND_MAX,"+
        "MB_CUR_MAX");

    define_keywords(SYNK_FUNCTION,
        "atof,"+
        "atoi,"+
        "atol,"+
        "atoll,"+
        "strtod,"+
        "strtof,"+
        "strtold,"+
        "strtol,"+
        "strtoll,"+
        "strtoul,"+
        "strtoull,"+
        "rand,"+
        "srand,"+
        "calloc,"+
        "free,"+
        "malloc,"+
        "realloc,"+
        "abort,"+
        "atexit,"+
        "exit,"+
        "_Exit,"+
        "getenv,"+
        "system,"+
        "bsearch,"+
        "qsort,"+
        "abs,"+
        "labs,"+
        "llabs,"+
        "div,"+
        "ldiv,"+
        "lldiv,"+
        "mblen,"+
        "mbtowc,"+
        "wctomb,"+
        "wcstombs");

    // B.20 String handling <string.h>
    define_keywords(SYNK_CONSTANT,
        "size_t,"+
        "NULL");

    define_keywords(SYNK_CONSTANT,
        "memcpy,"+
        "memmove,"+
        "strcpy,"+
        "strncpy,"+
        "strcat,"+
        "strncat,"+
        "memcmp,"+
        "strcmp,"+
        "strcoll,"+
        "strncmp,"+
        "strxfrm,"+
        "memchr,"+
        "strchr,"+
        "strcspn,"+
        "strpbrk,"+
        "strrchr,"+
        "strspn,"+
        "strstr,"+
        "strtok,"+
        "memset,"+
        "strerror,"+
        "strlen");

    // B.21 Type-generic math <tgmath.h>
    define_keywords(SYNK_FUNCTION,
        "acos,"+
        "asin,"+
        "atan,"+
        "acosh,"+
        "asinh,"+
        "atanh,"+
        "cos,"+
        "sin,"+
        "tan,"+
        "cosh,"+
        "sinh,"+
        "tanh,"+
        "exp,"+
        "log,"+
        "pow,"+
        "sqrt,"+
        "fabs,"+
        "atan2,"+
        "cbrt,"+
        "ceil,"+
        "copysign,"+
        "erf,"+
        "erfc,"+
        "exp2,"+
        "expm1,"+
        "fdim,"+
        "floor,"+
        "fma,"+
        "fmax,"+
        "fmin,"+
        "fmod,"+
        "frexp,"+
        "hypot,"+
        "ilogb,"+
        "ldexp,"+
        "lgamma,"+
        "llrint,"+
        "llround,"+
        "log10,"+
        "log1p,"+
        "log2,"+
        "logb,"+
        "lrint,"+
        "lround,"+
        "nearbyint,"+
        "nextafter,"+
        "nexttoward,"+
        "remainder,"+
        "remquo,"+
        "rint,"+
        "round,"+
        "scalbn,"+
        "scalbln,"+
        "tgamma,"+
        "trunc,"+
        "carg,"+
        "cimag,"+
        "conj,"+
        "cproj,"+
        "creal");

    // B.22 Date and time <time.h>
    define_keywords(SYNK_CONSTANT,
        "NULL,"+
        "CLOCKS_PER_SEC,"+
        "size_t,"+
        "clock_t,"+
        "time_t");

    define_keywords(SYNK_FUNCTION,
        "clock,"+
        "difftime,"+
        "mktime,"+
        "time,"+
        "asctime,"+
        "ctime,"+
        "gmtime,"+
        "localtime,"+
        "strftime");

    // B.23 Extended multibyte/wide character utilities <wchar.h>
    define_keywords(SYNK_CONSTANT,
        "wchar_t,"+
        "size_t,"+
        "mbstate_t,"+
        "wint_t,"+
        "NULL,"+
        "WCHAR_MAX,"+
        "WCHAR_MIN,"+
        "WEOF");

    define_keywords(SYNK_FUNCTION,
        "fwprintf,"+
        "fwscanf,"+
        "swprintf,"+
        "swscanf,"+
        "vfwprintf,"+
        "vfwscanf,"+
        "vswprintf,"+
        "vswscanf,"+
        "vwprintf,"+
        "vwscanf,"+
        "wprintf,"+
        "wscanf,"+
        "fgetwc,"+
        "fgetws,"+
        "fputwc,"+
        "fputws,"+
        "fwide,"+
        "getwc,"+
        "getwchar,"+
        "putwc,"+
        "putwchar,"+
        "ungetwc");

    define_keywords(SYNK_CONSTANT,
        "wcstod,"+
        "wcstof,"+
        "wcstold,"+
        "wcstol,"+
        "wcstoll,"+
        "wcstoul,"+
        "wcstoull,"+
        "wcscpy,"+
        "wcsncpy,"+
        "wmemcpy,"+
        "wmemmove,"+
        "wcscat,"+
        "wcsncat,"+
        "wcscmp,"+
        "wcscoll,"+
        "wcsncmp,"+
        "wcsxfrm,"+
        "wmemcmp,"+
        "wcschr,"+
        "wcscspn,"+
        "wcspbrk,"+
        "wcsrchr,"+
        "csspn,"+
        "wcsstr,"+
        "wcstok,"+
        "wmemchr,"+
        "wcslen,"+
        "wmemset,"+
        "wcsftime,"+
        "btowc,"+
        "wctob,"+
        "mbsinit,"+
        "mbrlen,"+
        "mbrtowc,"+
        "wcrtomb,"+
        "mbsrtowcs,"+
        "wcsrtombs");

    // B.24 Wide character classification and mapping utilities <wctype.h>
    define_keywords(SYNK_CONSTANT,
        "wctrans_t,"+
        "wctype_t,"+
        "WEOF");

    define_keywords(SYNK_FUNCTION,
        "iswalnum,"+
        "iswalpha,"+
        "iswblank,"+
        "iswcntrl,"+
        "iswdigit,"+
        "iswgraph,"+
        "iswlower,"+
        "iswprint,"+
        "iswpunct,"+
        "iswspace,"+
        "iswupper,"+
        "iswxdigit,"+
        "iswctype,"+
        "wctype,"+
        "towlower,"+
        "towupper,"+
        "towctrans,"+
        "wctrans");
}


static void
keywords_c11(void)
{
    define_keywords(SYNK_STORAGECLASS,
        "_Thread_local");

    define_keywords(SYNK_PRIMARY,
        "_Alignas,"+
        "_Bool,"+
        "_Complex,"+
        "_Generic,"+
        "_Noreturn,"+
        "aligned_alloc,"+
        "alignof,"+
        "restrict");

    define_keywords(SYNK_PREPROCESSOR,
        "_STDC_ANALYZABLE_,"+
        "_STDC_IEC_559_,"+
        "_STDC_IEC_559_COMPLEX_,"+
        "_STDC_LIB_EXT1_,"+
        "_STDC_NO_ATOMICS_,"+
        "_STDC_NO_COMPLEX_,"+
        "_STDC_NO_THREADS_,"+
        "_STDC_NO_VLA_,"+
        "ONCE_FLAG_INIT,"+
        "TSS_DTOR_ITERATIONS");

    define_keywords(SYNK_CONSTANT,
        "ONCE_FLAG_INIT,"+
        "ATOMIC_CHAR16_T_LOCK_FREE,"+
        "ATOMIC_CHAR32_T_LOCK_FREE,"+
        "ATOMIC_CHAR_LOCK_FREE,"+
        "ATOMIC_CHAR_LOCK_FREE,"+
        "ATOMIC_FLAG_INIT,"+
        "ATOMIC_INT_LOCK_FREE,"+
        "ATOMIC_LLONG_LOCK_FREE,"+
        "ATOMIC_LLONG_LOCK_FREE,"+
        "ATOMIC_LONG_LOCK_FREE,"+
        "ATOMIC_SHORT_LOCK_FREE,"+
        "ATOMIC_VAR_INIT,"+
        "ATOMIC_WCHAR_T_LOCK_FREE,"+
        "mtx_plain,"+
        "mtx_recursive,"+
        "mtx_timed,"+
        "thrd_timedout,"+
        "thrd_success,"+
        "thrd_busy,"+
        "thrd_error,"+
        "thrd_nomem");

    define_keywords(SYNK_TYPE,
        "thread_local,"+
        "char16_t,"+
        "char32_t,"+
        "cnd_t,"+
        "thrd_t,"+
        "tss_t,"+
        "mtx_t,"+
        "tss_dtor_t,"+
        "thrd_start_t,"+
        "once_flag");

    define_keywords(SYNK_FUNCTION,
        "fopen_s,"+
        "tmpnam_s,"+

        "atomic_compare_exchange,"+
        "atomic_exchange,"+
        "atomic_fetch,"+
        "atomic_flag,"+
        "atomic_flag_clear,"+
        "atomic_flag_test_and_set,"+
        "atomic_init,"+
        "atomic_is_lock_free,"+
        "atomic_load,"+
        "atomic_signal_fence,"+
        "atomic_store,"+
        "atomic_thread_fence,"+

        "mtx_destroy,"+
        "mtx_init,"+
        "mtx_lock,"+
        "mtx_timedlock,"+
        "mtx_trylock,"+
        "mtx_unlock,"+

        "cnd_broadcast,"+
        "cnd_destroy,"+
        "cnd_init,"+
        "cnd_signal,"+
        "cnd_timedwait,"+
        "cnd_wait,"+

        "call_once,"+

        "aligned_alloc,"+
        "quick_exit,"+

        "thrd_create,"+
        "thrd_current,"+
        "thrd_detach,"+
        "thrd_equal,"+
        "thrd_exit,"+
        "thrd_join,"+
        "thrd_sleep,"+
        "thrd_yield,"+

        "tss_create,"+
        "tss_delete,"+
        "tss_get,"+
        "tss_set,"+

        "llround,"+
        "llrint,"+

        "fprintf_s,"+
        "printf_s,"+
        "snprintf_s,"+
        "sprintf_s,"+
        "vfprintf_s,"+
        "vprintf_s,"+
        "vsnprintf_s,"+
        "vsprintf_s,"+
        "fwprintf_s,"+
        "snwprintf_s,"+
        "swprintf_s,"+
        "vfwprintf_s,"+
        "vsnwprintf_s,"+
        "vswprintf_s,"+
        "vwprintf_s,"+
        "wprintf_s,"+

        "fscanf_s,"+
        "scanf_s,"+
        "sscanf_s,"+
        "vfscanf_s,"+
        "vscanf_s,"+
        "vsscanf_s,"+
        "fwscanf_s,"+
        "swscanf_s,"+
        "vfwscanf_s,"+
        "vswscanf_s,"+
        "vwscanf_s,"+
        "wscanf_s,"+
        "asctime_s,"+
        "ctime_s,"+
        "gmtime_s,"+
        "localtime_s,"+
        "bsearch_s,"+
        "qsort_s,"+
        "strtok_s,"+
        "wcstok_s,"+

        "memcpy_s,"+
        "memmove_s,"+
        "memset_s,"+

        "gets_s,"+
        "getenv_s,"+
        "wctomb_s,"+
        "mbstowcs_s,"+
        "wcstombs_s,"+
        "strcpy_s,"+
        "strncpy_s,"+
        "strcat_s,"+
        "strncat_s,"+
        "strerror_s,"+
        "strnlen_s,"+
        "wcscpy_s,"+
        "wcsncpy_s,"+
        "wmemcpy_s,"+
        "wmemmove_s,"+
        "wcscat_s,"+
        "wcsncat_s,"+
        "wcsnlen_s,"+
        "wcrtomb_s,"+
        "mbsrtowcs_s,"+
        "wcsrtombs_s");
}


static void
keywords_posix1_2001(void)
{
    define_keywords(SYNK_FUNCTION,
        "strcasecmp,"+
        "strncasecmp,"+
        "wcscasecmp,"+
        "wcsncasecmp");

    define_keywords(SYNK_CONSTANT,
        "E2BIG,"+
        "EACCES,"+
        "EADDRINUSE,"+
        "EADDRNOTAVAIL,"+
        "EAFNOSUPPORT,"+
        "EAGAIN,"+
        "EALREADY,"+
        "EBADF,"+
        "EBADMSG,"+
        "EBUSY,"+
        "ECHILD,"+
        "ECONNABORTED,"+
        "ECONNREFUSED,"+
        "ECONNRESET,"+
        "EDEADLK,"+
        "EDESTADDRREQ,"+
        "EDOM,"+
        "EFAULT,"+
        "EDQUOT,"+
        "EEXIST,"+
        "EFBIG,"+
        "EIDRM,"+
        "EILSEQ,"+
        "EINPROGRESS,"+
        "EINTR,"+
        "EINVAL,"+
        "EIO,"+
        "EISCONN,"+
        "EISDIR,"+
        "ELOOP,"+
        "EMFILE,"+
        "EMLINK,"+
        "EMSGSIZE,"+
        "EMULTIHOP,"+
        "ENAMETOOLONG,"+
        "ENETDOWN,"+
        "ENETUNREACH,"+
        "ENFILE,"+
        "ENOBUFS,"+
        "ENODATA,"+
        "ENODEV,"+
        "ENOENT,"+
        "ENOEXEC,"+
        "ENOLCK,"+
        "ENOLINK,"+
        "ENOMEM,"+
        "ENOMSG,"+
        "ENOPROTOOPT,"+
        "ENOSPC,"+
        "ENOSR,"+
        "ENOSTR,"+
        "ENOSYS,"+
        "ENOTCONN,"+
        "ENOTDIR,"+
        "ENOTEMPTY,"+
        "ENOTSOCK,"+
        "ENOTTY,"+
        "ENXIO,"+
        "EOPNOTSUPP,"+
        "EOVERFLOW,"+
        "EPERM,"+
        "EPIPE,"+
        "EPROTO,"+
        "EPROTONOSUPPORT,"+
        "EPROTOTYPE,"+
        "ERANGE,"+
        "EROFS,"+
        "ESPIPE,"+
        "ESRCH,"+
        "ESTALE,"+
        "ETIME,"+
        "ETIMEDOUT,"+
        "ETXTBSY,"+
        "EWOULDBLOCK,"+
        "EWOULDBLOCK,"+
        "EAGAIN,"+
        "EXDEV");

    define_keywords(SYNK_CONSTANT,
        "SIGABRT,"+         // 6    Process aborted
        "SIGALRM,"+         // 14   Signal raised by alarm
        "SIGBUS,"+          // 7    Bus error: "access to undefined portion of memory object"
        "SIGCHLD,"+         // 17   Child process terminated, stopped (or continued*)
        "SIGCONT,"+         // 18   Continue if stopped
        "SIGFPE,"+          // 8    Floating point exception: "erroneous arithmetic operation"
        "SIGHUP,"+          // 1    Hangup
        "SIGILL,"+          // 4    Illegal instruction
        "SIGINT,"+          // 2    Interrupt
        "SIGIO,"+           // 29   I/O now possible
        "SIGIOT,"+          // 6    IOT trap
        "SIGKILL,"+         // 9    Kill (terminate immediately)
        "SIGPIPE,"+         // 13   Write to pipe with no one reading
        "SIGPOLL,"+         // 29   Pollable event
        "SIGPROF,"+         // 27   Profiling timer expired
        "SIGPWR,"+          // 30   Power failure restart
        "SIGQUIT,"+         // 3    Quit and dump core
        "SIGSEGV,"+         // 11   Segmentation violation
        "SIGSTOP,"+         // 19   Stop executing temporarily
        "SIGSYS,"+          // 31   Bad syscall
        "SIGTERM,"+         // 15   Termination (request to terminate)
        "SIGTRAP,"+         // 5    Trace/breakpoint trap
        "SIGTSTP,"+         // 20   Terminal stop signal
        "SIGTTIN,"+         // 21   Background process attempting to read from tty ("in")
        "SIGTTOU,"+         // 22   Background process attempting to write to tty ("out")
        "SIGUNUSED,"+       // 31   SIGSYS
        "SIGURG,"+          // 23   Urgent data available on socket
        "SIGUSR1,"+         // 10   User-defined 1
        "SIGUSR2,"+         // 12   User-defined 2
        "SIGVTALRM,"+       // 26   Signal raised by timer counting virtual time: "virtual timer expired"
        "SIGWINCH,"+        // 28   Window size change
        "SIGXCPU,"+         // 24   CPU time limit exceeded
        "SIGXFSZ");         // 25   File size limit exceeded
}


static void
keywords_nonposix(void)
{
    // see: http://www.ioplex.com/~miallen/errcmp.html

    // Linux
    define_keywords(SYNK_CONSTANT,
        "EBADE,"+           // Invalid exchange
        "EBADFD,"+          // File descriptor in bad state
        "EBADR,"+           // Invalid request descriptor
        "EBADRQC,"+         // Invalid request code
        "EBADSLT,"+         // Invalid slot
        "ECHRNG,"+          // Channel number out of range
        "ECOMM,"+           // Communication error on send
        "ECONNABORTED,"+    // Connection aborted
        "ECONNRESET,"+      // Connection reset
        "EDEADLOCK,"+       // Synonym for EDEADLK
        "EHOSTDOWN,"+       // Host is down
        "EISNAM,"+          // Is a named type file
        "EKEYEXPIRED,"+     // Key has expired
        "EKEYREJECTED,"+    // Key was rejected by service
        "EKEYREVOKED,"+     // Key has been revoked
        "EL2HLT,"+          // Level 2 halted
        "EL2NSYNC,"+        // Level 2 not synchronized
        "EL3HLT,"+          // Level 3 halted
        "EL3RST,"+          // Level 3 halted
        "ELIBACC,"+         // Cannot access a needed shared library
        "ELIBBAD,"+         // Accessing a corrupted shared library
        "ELIBEXEC,"+        // Cannot exec a shared library directly
        "ELIBMAX,"+         // Attempting to link in too many shared libraries
        "ELIBSCN,"+         // lib section in a.out corrupted
        "EMEDIUMTYPE,"+     // Wrong medium type
        "ENETDOWN,"+        // Network is down
        "ENETRESET,"+       // Connection aborted by network.
        "ENETUNREACH,"+     // Network unreachable
        "ENOKEY,"+          // Required key not available
        "ENOMEDIUM,"+       // No medium found
        "ENONET,"+          // Machine is not on the network
        "ENOPKG,"+          // Package not installed
        "ENOTBLK,"+         // Block device required
        "ENOTUNIQ,"+        // Name not unique on network
        "EPFNOSUPPORT,"+    // Protocol family not supported
        "EREMCHG,"+         // Remote address changed
        "EREMOTE,"+         // Object is remote
        "EREMOTEIO,"+       // Remote I/O error
        "ERESTART,"+        // Interrupted system call should be restarted
        "ESHUTDOWN,"+       // Cannot send after transport endpoint shutdown
        "ESOCKTNOSUPPORT,"+ // Socket type not supported
        "ESTRPIPE,"+        // Streams pipe error
        "EUCLEAN,"+         // Structure needs cleaning
        "EUNATCH,"+         // Protocol driver not attached
        "EUSERS,"+          // Too many users
        "EXFULL"            // Exchange full
        );

    // BSD
    define_keywords(SYNK_CONSTANT,
        "EAUTH,"+           // Authentication error
        "EBADARCH,"+        // Bad CPU type in executable
        "EBADEXEC,"+        // Bad executable
        "EBADMACHO,"+       // Malformed Macho file
        "EBADRPC,"+         // RPC struct is bad
        "EDEVERR,"+         // Device error, e.g. paper out
        "EDOOFUS,"+         // Programming error
        "EFTYPE,"+          // Inappropriate file type or format
        "ELAST,"+           // Must be equal largest errno
        "ENEEDAUTH,"+       // Need authenticator
        "EPROCUNAVAIL,"+    // Bad procedure for program
        "EPROGMISMATCH,"+   // Program version wrong
        "EPROGUNAVAIL,"+    // RPC prog. not avail
        "EPWROFF,"+         // Device power is off
        "ERPCMISMATCH,"+    // RPC version wrong
        "ESHLIBVERS"        // Shared library version mismatch
        );

    // AIX
    define_keywords(SYNK_CONSTANT,
        "ENOTREADY,"+       // Device not ready
        "EWRPROTECT,"+      // Write-protected media
        "EFORMAT,"+         // Unformatted media
        "EDIST,"+           // old, currently unused AIX errno
        "EDESTADDREQ,"+     // Destination address required
        "ERESTART,"+        // restart the system call
        "ECLONEME,"+        // this is the way we clone a stream ...
        "EPROCLIM,"+        // Too many processes
        "ECORRUPT,"+        // Invalid file system control data
        "EMEDIA,"+          // media surface error
        "ESOFT,"+           // I/O completed, but needs relocation
        "ENOATTR,"+         // no attribute found
        "ESAD,"+            // security authentication denied
        "ENOTRUST"          // not a trusted program
        );

    // HP-UX
    define_keywords(SYNK_CONSTANT,
        "EBADVER,"+         // Version number mismatch for loadable kernel module
        "ECONFIG,"+         // Configured kernle resource exhausted
        "ENOLOAD,"+         // Cannot load required kernel module
        "ENOMATCH,"+        // Symbol matching given spec not found
        "ENOREG,"+          // Cannot register required kernel module
        "ENOSYM,"+          // symbol does not exist in executable
        "ENOUNLD,"+         // Cannot unload kernel module
        "ENOUNREG,"+        // Cannot unregister kernel module
        "EOPCOMPLETE,"+     // Operation completed at server
        "EPATHREMOTE,"+     // Pathname is remote
        "EREFUSED,"+        // Double define for NFS
        "ERELOC,"+          // Object file error in loading kernel module
        "EREMOTERELEASE"    // Remote peer released connection
        );
}


/*
 *  ansic ---
 *      Macro to convert forward prototype declarations to ANSI C prototype
 *      declarations. It is assumed that forward declarations are of the form:
 *
 *          type funcstring.
 *
 *      We simply remove the comments
 */
void
ansic(void)
{
    int n;

    top_of_buffer();
    n = re_translate(SF_PROMPT, "{^[A-Za-z]*(}/\\*{*}\\*/);", "\\0\\1);");
    message("%d declarations changed to ANSI C.", n);
}


/*
 *  krc ---
 *      Macro to convert K&R forward prototype declarations to ANSI C
 *      prototype declarations. It is assumed that forward declarations are of
 *      the form: type func(xxx), where xxx is the parameter list inside a
 *      comment string. We simply remove the comments
 */
void
krc(void)
{
    int n;

    top_of_buffer();
    n = re_translate(SF_PROMPT,
            "{^[A-Za-z][^y][^p][^e][^d][^e][^f]*(}{*});",
            "\\0/*\\1*/);");
    message("%d declarations changed to K&R C.", n);
}


/*
 *  Modeline/package support.
 */
string
_c_mode(void)
{
    return "c";                     /* return package extension */
}


string
_cplusplus_mode(void)
{
    return _c_mode();
}


void
_c_modeattach(void)
{
    attach_syntax(MODENAME);
}


string
_c_highlight_first(void)
{
    _c_modeattach();
    return "";
}


/*
 *  Hier support.
 */
list
_c_hier_list(void)
{
    return c_hier_list;
}


list
_cpp_hier_list(void)
{
    return cpp_hier_list;
}


/*
 *  c_template_first, _cpp_template_first, _c_smart_first, _cpp_smart_first ---
 *
 *      These macros are called by the BPACKAGES parser in language.cr They
 *      initialize the local keymaps for the various indenting functions, set
 *      the abbreviation length and adjust the indenting style.
 */
string
_c_template_first(void)
{
    _c_min_abbrev = 0;
    get_parm(0, _c_min_abbrev);
    _c_min_abbrev = MIN_ABBREV;
    use_local_keyboard(_c_template);
    _c_indent_open = 1;
    _c_indent_close = 0;
    _c_indent_first = 0;
    get_parm(1, _c_indent_open);
    get_parm(2, _c_indent_close);
    get_parm(3, _c_indent_first);
    return "";
}


string
_h_template_first(void)
{
    return _c_template_first();
}


string
_cpp_template_first(void)
{
    return _c_template_first();
}


string
_c_smart_first(void)
{
    use_tab_char("n");
    use_local_keyboard(_c_smart);
    _c_indent_open  = 1;
    _c_indent_close = 0;
    _c_indent_first = 0;
    get_parm(0, _c_indent_open);
    get_parm(1, _c_indent_close);
    get_parm(2, _c_indent_first);
    return "";
}


string
_h_smart_first(void)
{
    return _c_smart_first();
}


string
_cpp_smart_first(void)
{
    return _c_smart_first();
}


/*
 *  c_indent ---
 *      This macro does syntax-sensitive indenting ("smart indenting")
 *      for C language files
 */
#define SEMI_COLON          1                   /* Code for semicolon */
#define OPEN_BRACE          2                   /* Code for open brace */
#define CLOSE_BRACE         3                   /* Code for close brace */
#define FULL_COLON          4                   /* Code for colon (case) */
#define COMMA               5                   /* Code for comma */
#define CLOSE_PAREN         6                   /* code for closing paren */
#define EQUALS              7                   /* code for equals (initialisation) */

string  C_TERM_CHR = ";{}:,)=";

#define IGNORE_LIST_SIZE    3                   /* Set to number of elements in list */

static list c_ignore_list = {
    "[ \\t]@{/\\*}+{[~\\*]|[~/]}*{\\*/}",       /* C comments */
    "[ \\t]@//*$",                              /* C++ comments */
    "[ \\t]@#*$",                               /* Pre-processor lines */
    };


/*
 *  _c_prevline ---
 *      Finds the previous syntactically significant line.
 */
string
_c_prevline(void)
{
    int curr_line, at_line, patt_no, done;
    string line;

    inq_position(curr_line, NULL);

    /* Move to the line we wish to start at */
    at_line = curr_line;
    for (done = FALSE; !done;) {
        /* Process each line, and find one with text that cannot be eliminated */
        beginning_of_line();
        line = read();
        for (patt_no = 0; patt_no < IGNORE_LIST_SIZE; ++patt_no) {
            int patt_len, gotcha;

            /* Keep removing discardable text */
            while ((gotcha = re_search( NULL, c_ignore_list[patt_no], line, NULL, patt_len )) > 0) {
                line = substr(line, 1, gotcha - 1) + substr(line, gotcha + patt_len);
            }
        }

        /*
         *  We've now eliminated all comment/discardable text. Trim the string, and if
         *  it's empty move back one more line. If we've reached the beginning of the
         *  buffer, simply return an empty string. If there's anything left, return that
         */
        line = trim(line);
        if (line != "") {
            ++done;
        } else {
            if (!--at_line) {
                ++done;
            } else {
                up();
            }
        }
    }
    return line;
}


int
_c_indent(void)
{
    int     curr_indent_col,                    /* Current unmodified indent col */
            following_position,                 /* Column of end of line following cursor */
            curr_col,                           /* Column cursor is on when called */
            curr_line,                          /* Column line is on when called */
            what_is_char_1,                     /* End of first line's character identifier */
            what_is_char_2,                     /* End of second line's indentifier */
            level,                              /* Current indenting level */
            tmp_level;

    string  following_string,                   /* All characters following the cursor */
            line_end;

    /*
     *  Gather information on the two previous non-blank lines
     */
    if (! inq_mode()) {
        end_of_line();
    }
    inq_position(curr_line, curr_col);
    end_of_line();
    inq_position(NULL, following_position);

    /*
     *  If there are characters following the cursor, save them in following_string.
     */
    if (following_position > curr_col) {
        drop_anchor(MK_NONINC);
        move_abs(0, curr_col);
        following_string = ltrim(read());
        delete_block();
    }

    line_end = _c_prevline();                   /* Retrieve previous line */
    inq_position(tmp_level, NULL);
    if (line_end != "") {
        what_is_char_2 = index(C_TERM_CHR, substr(line_end, strlen(line_end)));
        beginning_of_line();
        re_search(NULL, "[~ \t]");
        inq_position(NULL, curr_indent_col);
        up();
        line_end = _c_prevline();
        inq_position(level, NULL);
        if (line_end != "") {
            what_is_char_1 = index(C_TERM_CHR, substr(line_end, strlen(line_end)));
        } else {
            what_is_char_1 = SEMI_COLON;
        }

    } else {
        what_is_char_1 = what_is_char_2 = SEMI_COLON;
        curr_indent_col = 1;
    }
    move_abs(curr_line, curr_indent_col);

    /*
     *  We've determined the last two non-blank lines' last characters as well as the
     *  column position of the first non-blank character. Now we position the cursor on
     *  the new line's proper level.
     */
    if (curr_indent_col == 1 && !_c_indent_first && what_is_char_2 == OPEN_BRACE) {
        curr_indent_col += distance_to_indent();

    } else {
        /*
         *  The following switch statement is the body of the _c_indent macro indenting
         *  rules. what_is_char_2 is the terminator on the immediately preceeding line,
         *  what_is_char_1 is the terminator prior that. This allows us to make some
         *  guesses about whether we should indent or outdent from the current level
         */
        switch (what_is_char_2) {
        case SEMI_COLON:
            if (! what_is_char_1)
                curr_indent_col -= distance_to_indent();
            break;
        case CLOSE_BRACE:
            if (_c_indent_close)
                curr_indent_col -= distance_to_indent();
            break;
        case OPEN_BRACE:
            if (_c_indent_open)
                curr_indent_col += distance_to_indent();
            break;
        case COMMA:
            if (what_is_char_1 != COMMA)
                curr_indent_col += distance_to_indent();
            break;
        case EQUALS:
        case FULL_COLON:
        case CLOSE_PAREN:
            curr_indent_col += distance_to_indent();
            break;
        default:
            /* Nothing */
            break;
        }
    }

    move_abs(0, curr_col);

    if (get_parm(0, curr_col) == 0) {
        if (inq_assignment(inq_command(), 1) == "<Enter>") {
            self_insert(key_to_int("<Enter>"));
        } else {
            self_insert(key_to_int("<Ctrl-M>"));
        }
    }

    beginning_of_line();
    curr_col = distance_to_indent() + 1;

    level = 0;
    while (curr_col <= curr_indent_col) {
        move_abs(0, curr_col);
        ++level;
        curr_col += distance_to_indent();
    }

    if (following_string != "") {
        following_string = substr(following_string, 1, strlen(following_string) - 1);
        save_position();
        insert(following_string);
        restore_position();
    }
    return level;
}


/*  Template editing macros:
 *
 *  These macro performs simple template editing for C programs. Each time the space bar
 *  is pressed, the characters before the cursor are checked to see if they are "if",
 *  "else if", "while", "for", "do", switch" or "case" (or abbreviations for them).
 *  These keywords must only be preceded with spaces or tabs, and be typed at the end of
 *  a line. If a match is found, the remainder of the statement is filled in
 *  automatically.
 *
 *  In addition, a brace pairer is included -- this automatically inserts a matching
 *  brace at the proper indentation level when an opening brace is typed in. To insert a
 *  brace without a matching brace (it attempts to be smart about matching braces, but
 *  you never can make this type of thing quite smart enough), type either Ctrl-{ or
 *  quote the brace with Alt-q.
 *
 */
#define DO_BRACE            1
#define DO_CLOSE            2
#define DO_WHILE            4

#define ABBR_FIELDS         5
#define ABBR_COMPL          1
#define ABBR_FLAGS          2
#define ABBR_POSL           3
#define ABBR_POSC           4

static list     c_abbrev_list = {
  /* match,     completion,                             flags,      line/col */
    "return",   "return ;",                             0,          0, -1,
    "else",     "else",                                 0,          0, 0,
    "else if",  "else if ()",                           0,          0, -1,
    "if ",      "if ()",                                0,          0, -1,
    "while",    "while ()",                             DO_BRACE,   0, -1,
    "for",      "for (;;)",                             DO_BRACE,   0, -3,
    "do",       "do",                                   DO_WHILE,   2, 2,
    "switch",   "switch ()",                            DO_BRACE,   0, -1,
    "case",     "case :",                               0,          0, -1,
    "default",  "default:\n",                           0,          0, -1,
    "main",     "void\nmain(int argc, char *argv[])",   DO_BRACE,   2, -26,
    "#if",      "#if ",                                 0,          0, 0,
    "#ifdef",   "#if defined()",                        0,          0, -1,
    "#endif",   "#endif",                               0,          0, 0,
    "#include", "#include <>",                          0,          0, -1,
    "#define",  "#define ",                             0,          0, 0,
    };


/*
 *  _c_abbrev ---
 *      This function checks to see if the characters before the space just
 *      typed (and actually inserted by this function) are destined to be
 *      followed by the remainder of a C construct.
 */
void
_c_abbrev(void)
{
    int done = FALSE;
    string rd = read(1);

    if (rd == "\n") {
        int length;
        string line;

        save_position();
        beginning_of_line();
        line = trim(read());
        restore_position();
        length = strlen(line);

        if (length >= _c_min_abbrev) {
            int idx = length_of_list(c_abbrev_list) / ABBR_FIELDS;
            int loc = 0;

            while (!done && loc < idx) {
                int at = loc * 5;

                if (re_search(SF_NOT_REGEXP, line, c_abbrev_list[at]) == 1) {
                    string completion;

                    message("%s->%s", line, c_abbrev_list[at]);
                    prev_char(length);
                    delete_char(length);
                    completion = c_abbrev_list[at + ABBR_COMPL];
                    insert(completion);
                    save_position();

                    if (c_abbrev_list[at + ABBR_FLAGS] & DO_WHILE) {
                        _open_line();
                        _c_brace_expand();
                        move_rel(1, 0);
                        _open_line();
                        insert("while();");

                    } else if (c_abbrev_list[at + ABBR_FLAGS] & DO_BRACE) {
                        _open_line();
                        _c_brace_expand();
                    }

                    restore_position();
                    move_rel(c_abbrev_list[at + ABBR_POSL], c_abbrev_list[at + ABBR_POSC]);
                    done = TRUE;
                }
                loc++;
            }
        }
    }

    if (! done) {
        if (inq_symbol("_bvar_abbrev")) {       /* standard abbrev support */
            _abbrev_check();
        } else {
            self_insert();
        }
    }
}


/*
 *  _c_brace_expand --
 *      This function checks to see if the typed brace should be
 *      indented and matched to the current indenting level.
 */
void
_c_brace_expand(void)
{
    _c_open_brace();
    save_position();
    _c_indent();
    _c_close_brace();
    restore_position();
}


void
_c_open_brace(void)
{
    if (trim(read()) != "") {
        self_insert('{');

    } else {
        int curr_line, curr_col;

        inq_position(curr_line, curr_col);
        beginning_of_line();
        if (trim(read()) == "") {
            /* Align open brace at previous indent level */
            _c_prevline();
            beginning_of_line();
            re_search(NULL, "[~ \t]");
            inq_position(NULL, curr_col);
        }
        move_abs(curr_line, curr_col);
        self_insert('{');
        _c_indent();
    }
}


void
_c_close_brace(void)
{
    if (trim(read()) == "")  {
        int curr_line, curr_col, do_backtab = FALSE;

        inq_position(curr_line, curr_col);
        beginning_of_line();
        if (trim(read()) == "") {
            if (!_c_indent_first || !_c_indent_close)
                ++do_backtab;
        }
        move_abs(curr_line, curr_col);
        if (do_backtab && curr_col > 1)
            _back_tab();
    }
    self_insert('}');
}


/*
 *  _c_align_comments ---
 *      Align comments for a bounded region of lines.
 */
void
_c_align_comments(void)
{
    int startline, endline;

    if (!inq_marked(startline, NULL, endline, NULL)) {
        startline = 1;
        endline = inq_lines();
    }

    save_position();
    while (startline <= endline) {
        move_abs(startline++, 1);
        _c_aligncomment();
    }
    restore_position();
}


/*
 *  _c_aligncomment ---
 *      Aligns C/C++ comments on the right side for easy read'in
 */
void
_c_aligncomment(void)
{
    int cur_line, cur_col;
    string line;

    inq_position(cur_line, cur_col);
    beginning_of_line();
    line = detab_str(read());                   /* read all of current line */
    if (line != "") {
        int posc, posp, done, toadd;
        string work, comment;

        work = line;
        for (toadd = done = 0; !done;) {
            int pquote, nquote;

            /* Handle C style comments */
            posc = re_search(SF_LENGTH, "{/\\*}+{[~\\*]|[~/]}*{\\*/}$", work);

            /* Handle C++ style comments */
            posp = re_search(SF_LENGTH, "//*$", work);
            if (posp > posc) {                  /* Select last one in line */
                posc = posp;
            } else {
                posp = 0;                       /* posp is non-zero if c++ comment */
            }

            if (!posc) {                        /* Nothing found */
                ++done;

            } else {
                /* Make sure they aren't in quotes */
                while ((pquote = indexn(work, pquote + 1, "\"")) > 0 && pquote < posc) {
                    if (pquote > 1) {
                        string ch = substr(work, pquote - 1, 1);

                        if (ch == "\\" || ch == "'")
                            continue;           /* Ignore escaped quote */
                    }
                    ++nquote;
                }

                if (!(nquote & 1)) {            /* Even number is ok */
                    ++done;
                } else {
                    /* At this point we know comment is quoted */
                    if (!pquote) {
                        /* No quote after comment mark? Don't touch this line */
                        posc = 0;
                        ++done;
                    }
                    toadd = pquote;
                    work = substr(work, pquote + 1);
                }
            }
        }

        if (posc > 0) {
            /* Now, reposition the line by splitting the line and comment,
             * stripping spaces off the end of the code line, deleting the
             * line and reinserting it and positioning the comment at the
             * right position aligned on the right side
             */
            posc += toadd;
            comment = substr(line, posc);
            line = rtrim(substr(line, 1, posc - 1), " \t\r\n");

            /*
             * We have to be careful if the line + comment length exceeds
             * the column at the right
             */
            if ((strlen(line) + strlen(comment)) >= _c_align_column) {
                line += comment;
                comment = "";
            }

            /* Now, remove the line and insert a newline */
            delete_line();
            insert("\n");

            /* Move back up and reinsert the line  */
            prev_char();
            insert(line);
            if (comment != "") {
                /* Place the comment in virtual space and insert */
                move_abs(NULL, _c_align_column - strlen(comment));
                insert(comment);
            }
            beginning_of_line();

            /* Entab the line again */
            line = entab_str(read());           /* entab the line */
            delete_line();
            insert(line);
        }
    }
    move_abs(cur_line, cur_col);
}


static int
indexn(string s, int start, string subs)
{
    int i;

    if (start < 1) {
        start = 1;
    } else {
        s = substr(s, start);
    }
    i = index(s, subs);
    if (i)
        i += start - 1;
    return i;
}


void
_c_preprocessor(void)
{
    save_position();
    beginning_of_line();
    if (trim(read()) != "") {
        restore_position();
    } else {
        delete_line();
        insert("\n");
        prev_char();
    }
    insert("#");
}


/*
 *  new_c_comment_block ---
 *      This macro comments out a block of code. It tries to be intelligent about
 *      things like comments inside the block -- if it finds one, it escapes it and
 *      moves on.
 *
 *      Two different types of blocks are created, depending on the cursor start
 *      positions.
 *
 *      a)  If in column one, comment_block assumes a "block" type comment is desired.
 *
 *              -1  Use mode default.
 *              0   Simple.
 *              1   Box, leading and trailing.
 *              2   One leading '*'.
 *              3   Two leading '*'.
 *
 *      b)  If the start is not in column one, the area is simply bracketed
 *          by C comment delimiters.
 */
void
new_c_comment_block(int type)
{
    int sline, scol, eline, ecol;

    if (! inq_marked(sline, scol, eline, ecol)) {
        error("No marked block.");
        return;
    }

//  if (type < 0)
//      type = _c_comment_type;

    message("Commenting %d/%d to %d/%d", sline, scol, eline, ecol);

    raise_anchor();
    save_position();

    move_abs(sline, scol);
    if (scol == 1 && type > 0) {
        if (type == 1) {
            /*
             *  Create top line of box
             *
             *      (***************************************************)
             */
            insert("/*");
            insert("*", ecol);
            insert("*/\n");
            eline++,
            ecol += 3;

        } else if (type == 2 || type ==3 ) {
            insert("/*\n");

        } else if (type == 4) {
            insert("//\n");
        }
        ++sline;
    } else {
        insert(" /* ");
    }

    if (scol == 1 && type > 0) {
        while (sline <= eline) {
            move_abs(sline, 1);
            if (type == 1) {
                insert("/*");
                move_abs(0, ecol);
                insert("*/");

            } else if (type == 2) {
                insert(" *");
                insert(" ", distance_to_tab());

            } else if (type == 3) {
                insert("**");
                insert(" ", distance_to_tab());

            } else if (type == 4) {
                insert("//");
                insert(" ", distance_to_tab());
            }

            ++sline;
        }
    }

    if (scol == 1 && type > 0) {
        move_abs(sline, 1);
        if (type == 1) {
            /*
             *  Create top line of box
             *
             *      (***************************************************)
             */
            insert("/*");
            insert("*", ecol-3);
            insert("*/\n");

        } else if (type == 2) {
            insert(" */\n");

        } else if (type == 3) {
            insert("*/\n");

        } else if (type == 4) {
            insert("//\n");
        }

    } else  {
        move_abs(eline, ecol);
        insert(" */ ");
    }

    restore_position();
}


/*
 *  c_uncomment_block ---
 *      This routine uncomments a block of code.
 *
 *      It removed the comment and leading "**" characters (if appropriate),
 *      and restores the internal comments to their original, un-escaped state.
 */
void
new_c_uncomment_block(void)
{
    int sline, scol, eline, ecol;
    string s;

    if (! inq_marked(sline, scol, eline, ecol)) {
        return;
    }
    save_position();

    /*
     *  Left corners *\
     *  Right corners \*
     *  Comment opens
     *  Comment end and EOL *'s
     */
    re_translate(SF_GLOBAL | SF_BLOCK | SF_MAXIMAL, "\\*\\\\[ \t]@$", "");
    re_translate(SF_GLOBAL | SF_BLOCK | SF_MAXIMAL, "<[ \t]@\\\\\\*", "");
    re_translate(SF_GLOBAL | SF_BLOCK | SF_MAXIMAL, "<[ \t]@/[\\*]+", "");
    re_translate(SF_GLOBAL | SF_BLOCK | SF_MAXIMAL, "[\\*]+{/}@[ \t]@$", "");

    /*
     *  leading '**', ' *' and ' -' replaced with white-space.
     */
    while (sline < eline) {
        move_abs(sline, 1);
        if ((s = rtrim(read(2))) == "**" || s == " *" || s == " -") {
            delete_char(2);
            insert("  ");
        }
        sline++;
    }
    restore_position();
}

/*end*/
