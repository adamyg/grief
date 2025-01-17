#include <edidentifier.h>
__CIDENT_RCSID(gr_macros_c,"$Id: macros.c,v 1.58 2025/01/17 12:38:29 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: macros.c,v 1.58 2025/01/17 12:38:29 cvsuser Exp $
 * Manipulating macro definitions.
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
#include <edconfig.h>
#include <edenv.h>                              /* gputenvv(), ggetenv() */
#include <libstr.h>                             /* str_...()/sxprintf() */

#include "accum.h"                              /* acc_...() */
#include "builtin.h"
#include "debug.h"
#include "echo.h"
#include "eval.h"
#include "file.h"
#include "keywd.h"
#include "language.h"
#include "lisp.h"
#include "macrolib.h"
#include "macros.h"
#include "main.h"
#include "symbol.h"
#include "sysinfo.h"
#include "system.h"
#include "wild.h"
#include "word.h"

#define MAX_MACRODIRS       32                  /* MAGIC - system limit */
#define MAX_MACRONAME       128                 /* MAGIC - system limit */
#define MAX_NAMESPACES      8                   /* MAGIC - system limit */

typedef struct {                                /* macro objects */
    MAGIC_t             o_magic;
#define MOBJECT_MAGIC       MKMAGIC('M', 'O', 'b', 'j')
    uint16_t            o_ident;                /* unique identifier */
#define MOBJECT_MODULELEN   12                  /* $<ident> */
    char                o_module[MOBJECT_MODULELEN];
    SPTREE *            o_macros;               /* macro table */
    SPTREE *            o_symbols;              /* symbol table */
    const char *        o_macroname;            /* macro name */
    const char *        o_filepath;             /* filename */
    struct name_space * o_namespace;            /* assigned namespace, i.e. module(<namespace>) */
    char                o_buffer[1];            /* macro-name and filename storage */
} MOBJECT;


typedef struct name_space {                     /* module() definitions */
    MAGIC_t             n_magic;
#define MNAMESPACE_MAGIC    MKMAGIC('M', 'N', 'S', 'p')
    int                 n_ident;                /* unique identifier */
    int                 n_alloced;
    int                 n_count;
    MOBJECT *           n_objects[MAX_NAMESPACES];
    char                n_name[1];              /* namespace storage */
} MNAMESPACE;


struct mac_walk {
    const char **       w_list;
    int                 w_idx;
};


struct mod_walk {
    const char *        match;
    unsigned            count;
    const char *        module;
    uint16_t            ident;
};


static void             mod_walk(SPBLK *sp, void *arg);

static int              mac_validname(const char *name);
static void             mac_walk(SPBLK *sp, void *arg);
static int              mac_find(const char *name);
static const char **    mac_bpath(void);
static int              mac_read(char *fname, const char *mname);
static const MOBJECT *  mac_object(const char *filepath, const char *macroname);
static int              mac_parse(const char *filepath, const char *macroname);
static MOBJECT *        mac_lookup(const char *module);
static SPBLK *          mac_objsearch(const char *module, const char *name);

static const char *     mac_exts[] = { CM_EXTENSION, ".cr", ".m", NULL };
static const char *     mac_paths[MAX_MACRODIRS];

static int              mac_count = 0;
static SPTREE *         mac_namespace = NULL;   /* module to namespace mapping */

static SPTREE *         mac_objects = NULL;     /* object_id <> file_name mapping */
static SPTREE *         mac_global = NULL;      /* global macros */

static int              xf_findmacro;           /* TRUE whilst find_macro() primitive being executed */
static int              xf_initdef;             /* TRUE if "_init" was defined (used during loads) */

static int              x_modident;             /* macro unique identifier */
static int              x_macident;             /* macro unique identifier */


/*  Function:           macro_init
 *      Function called at startup time to initialise the macros splay trees.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 */
void
macro_init(void)
{
    mac_global    = spinit();
    mac_namespace = spinit();
    mac_objects   = spinit();
    macrolib_init();
}


/*  Function:           macro_shutdown
 *      Shutdown the macro subsystem.
 *
 *  Parameters:
 *      tree - Symbol tree.
 *      pfDelete - Destructor function.
 *
 *  Returns:
 *      nothing
 */
static void
zap_module(void *p, void *udata)
{
    MNAMESPACE *mp = (MNAMESPACE *)p;

    __CUNUSED(udata)
    assert(MNAMESPACE_MAGIC == mp->n_magic);
    chk_free(p);
}


static void
zap_macro(void *p, void *udata)
{
    MACRO *mptr = (MACRO *)p;

    __CUNUSED(udata)
    assert(mptr->m_magic == MACRO_MAGIC);
    if (M_OVERLOAD & mptr->m_flags) {
        return;                                 /* dont touch!! (need M_BUILTIN) */
    }
    chk_free((void *)mptr->m_name);
    mptr->m_name = NULL;
    if (M_AUTOLOAD & mptr->m_flags) {
        chk_free((void *)mptr->m_list);         /* release storage */
    }
    mptr->m_list = NULL;
    mptr->m_magic = (MAGIC_t)~MACRO_MAGIC;
    --mac_count;

    chk_free(p);
}


static void
zap_object(void *p, void *udata)
{
    MOBJECT *objp = (MOBJECT *)p;

    assert(MOBJECT_MAGIC == objp->o_magic);
    spzap(objp->o_macros, zap_macro, NULL);
    objp->o_macros = NULL;

    sym_macro_delete(objp->o_symbols);
    spfree(objp->o_symbols);
    objp->o_symbols = NULL;

    if (NULL == udata) {
        objp->o_magic = ~MOBJECT_MAGIC;
        chk_free(p);
    }
}


void
macro_shutdown(void)
{
    BUILTIN *bp = builtin, *endbp = builtin + builtin_count;

    /* objects */
    spzap(mac_objects, zap_object, NULL);
    mac_objects = NULL;

    /* modules */
    spzap(mac_namespace, zap_module, NULL);
    mac_namespace = NULL;

    /* globals */
    spzap(mac_global, zap_macro, NULL);
    mac_global = NULL;

    /* overloaded builtins */
    for (; bp < endbp; ++bp) {
        if (bp->b_flags & B_REDEFINE) {         /* consume overloaded builtins */
            MACRO *mptr = bp->b_first_macro;

            while (mptr) {                      /* last shall be the original/internal */
                MACRO *nextmptr = mptr->m_next;

                assert(M_OVERLOAD & mptr->m_flags);
                mptr->m_flags &= ~M_OVERLOAD;
                zap_macro(mptr, NULL);
                mptr = nextmptr;
            }
            bp->b_first_macro = bp->b_macro = NULL;

        } else {
            assert(NULL == bp->b_first_macro);
            assert(NULL == bp->b_macro);
        }
    }
    assert(mac_count == 0);                     /* detect leaks */
}


/*  Function;           module_identifier
 *      Returns the current module/macro name, i.e. module within the current scope.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      Module identifier otherwise NULL.
 */
const char *
module_identifier(void)
{
    return (mac_sd > 0 ? mac_sp->module : NULL);
}


/*  Function;           module_namespace
 *      Returns the current namespace.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      Namespace identifier otherwise NULL.
 */
const char *
module_namespace(const char *module)
{
    if (module && '$' == *module) {
        const MOBJECT *objp = mac_lookup(module);

        if (objp->o_namespace) {
            return objp->o_namespace->n_name;
        }
    }
    return NULL;
}


/*  Function:           module_symbols
 *      Retrieve the module specific symbol table SPTREE using the
 *      human readable version of the module name.
 *
 *  Parameters:
 *      module - Module name (e.g. 'restore').
 *
 *  Returns:
 *      Symbol table.
 */
SPTREE *
module_symbols(const char *module)
{
    const char *resolved = NULL;

    if (module) {
        const MOBJECT *objp;

        if ('$' == *module) {                   /* internal module name */
            resolved = module;
                                                /* search namespaces */
        } else if (NULL != (objp = mac_object(module, NULL))) {
            resolved = objp->o_module;

        } else {
            struct mod_walk mods;

            mods.match = module;
            mods.count = 0;
            spwalk(mac_objects, mod_walk, &mods);
            if (1 == mods.count) {
                resolved = mods.module;
            }
        }

        if (resolved) {
            return macro_symbols(resolved);
        }
    }
    return NULL;
}


static void
mod_walk(SPBLK *sp, void *arg)
{
    struct mod_walk *mods = (struct mod_walk *)arg;
    MOBJECT *objp = (MOBJECT *)sp->data;

    assert(MOBJECT_MAGIC == objp->o_magic);
    if (0 == file_cmp(mods->match, objp->o_macroname)) {
        if (1 == ++mods->count) {
            mods->module = objp->o_module;      /* first match */
            mods->ident  = objp->o_ident;
        }
    }
}


static const char *
mac_basename(const char *name)
{
    const char *base = name;

    while (*name) {
        const int ch = *name++;
        if (ch == '/' || ch == '\\') {
            base = name;
        }
    }
    return base;
}


/*  Function:           macro_find
 *      Search the macro table and return pointer to a macro definition.
 *
 *  Parameters:
 *      name - Macro name.
 *      module - Current scope or module.
 *
 *  Returns:
 *      Macro object address.
 */
MACRO *
macro_find(const char *name, const char *module)
{
    SPBLK *sp = NULL;
    const char *p;

    /*
     *  explicit scoping, using the "::" naming modifier ?
     *
     *  There are two forms:
     *      ::function                          implicit
     *  and
     *      module::function                    explicit
     */
    assert(name);
    name = mac_basename(name);
    if (':' == name[0] && ':' == name[1]) {
        if (module) {
            sp = mac_objsearch(module, name + 2);
        }

    } else if (NULL != (p = strchr(name, ':')) && ':' == p[1]) {
        char t_module[ MAX_MACRONAME ];
        int len = (p - name) + 1;

        if (len < (int)sizeof(t_module)) {      /* MAGIC */
            strxcpy(t_module, name, len);
            sp = mac_objsearch(t_module, p + 2);
        }

    /*
     *  implicit scoping, module then global.
     */
    } else {                                    /* current */
        if (module) {
            sp = mac_objsearch(module, name);
        }
        if (NULL == sp) {                       /* global */
            sp = splookup(name, mac_global);
        }
    }

    if (sp) {
        return (MACRO *) sp->data;
    }
    return NULL;
}


/*  Function:           macro_exist
 *      Checks for existence of the specified 'macro' taking into
 *      account the current module scope, generating an error if not.
 *
 *  Parameters:
 *      name - Macro name.
 *      [msg] - Error message prefix (ie caller).
 *
 *   Return:
 *      TRUE/FALSE -
 */
/*__CBOOL*/ int
macro_exist(const char *name, const char *msg)
{
    if (macro_lookup(name)) {
        return TRUE;
    }

    if (msg) {
        errorf("%s: undefined macro '%s' reference.", msg, name);
    } else {
        errorf("undefined macro '%s' reference.", name);
    }
    return FALSE;
}


/*  Function:           macro_lookup
 *      lookup the specified 'macro' taking into account the
 *      current module scope.
 *
 *  Parameters:
 *      name - Macro name.
 *
 *  Returns:
 *      Macro object address.
 */
MACRO *
macro_lookup(const char *name)
{
    return macro_find(name, module_identifier());
}


/*  Function:           macro_symbols
 *      Retrieve the macro/object specific symbol table SPTREE.
 *
 *  Parameters:
 *      name - Macro name (e.g '$1234').
 *
 *  Returns:
 *      Symbol tree address otherwise NULL.
 */
SPTREE *
macro_symbols(const char *module)
{
    MOBJECT *objp = mac_lookup(module);

    if (objp) {
        return objp->o_symbols;
    }
    return NULL;
}


/*  Function:           macro_resolve
 *       Performs a macro lookup and returns a full qualified macro name.
 *
 *  Parameters:
 *      name - Macro name (e.g '$1234').
 *
 *  Returns:
 *      Resolved macro-name otherwise NULL.
 */
const char *
macro_resolve(const char *cmd)
{
    const char *name, *module = module_identifier();
    char name_buf[ MAX_MACRONAME ], *p;
    const char *buf = cmd;
    MACRO *mptr;

    while (isspace(*cmd)) {
        ++cmd;                                  /* consume leading white */
    }

    if (NULL == (p = strchr(cmd, ' '))) {       /* easy */
        name = cmd;
    } else {                                    /* copy and trim */
        strxcpy(name_buf, cmd, sizeof(name_buf));
        if (NULL != (p = strchr(name_buf, ' '))) {
            *p = '\0';
        }
        name = name_buf;
    }

    /* locate macro */
    if (NULL != (mptr = macro_find(name, module))) {
        if (M_STATIC & mptr->m_flags) {
                                                /* no module supplied */
            if (NULL == (p = strchr(name, ':'))) {
                ewprintf("%s: unqualified reference.", name);

            } else if (':' != p[1]) {
                ewprintf("%s: invalid qualifier reference.", name);

            } else if (p == name) {             /* qualify exec command */
                const int len = strlen(mptr->m_module) + strlen(cmd) + 1;
                char *qbuf;
                                                /* note; use full 'cmd' */
                if (NULL != (qbuf = chk_calloc(len, 1))) {
                    sxprintf(qbuf, len, "%s%s", mptr->m_module, cmd);
                    buf = qbuf;
                }
            }
        }

    } else if (xf_warnings) {                   /* debugging only */
        if (NULL == builtin_lookup(name) && mac_find(name) < 0) {
            ewprintf("%s: undefined macro reference.", name);
        }
    }

    return buf;
}


/*  Function:           do_module
 *      Module primitive.
 *
 *  Macro Parameters:
 *      module - Module name (e.g 'mymodule').
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: module - Assign a module identifier.

        int
        module(string modulename)

    Macro Description:
        GRIEF provides a mechanism for alternative namespaces, being an
        abstract container providing context for the items, to protect
        modules from accessing on each other's variables.

        The purpose of this functionality is to provide data hiding
        within a set of common macro files (objects), by allowing separate
        function and variables namespaces. Each namespace creates a
        container for a set of symbols, providing a level of indirection
        to specific identifiers, thus making it possible to distinguish
        between identifiers with the same exact name, in effect reducing
        naming conflicts in unrelated objects (See Scope).

        The module statement declares the object as being in the given
        namespace. The scope of the module declaration is from the
        declaration itself and effects all current and future
        declarations within the associated object.

        The intended usage of the module() primitive is the within the
        main function in all of the related objects.

        The namespace specification should be a string containing a valid
        sequence of identifier symbols of the form

>           [A-Za-z_][A-Za-z_0-9]*

        Namespaces are in conjunction with static scoping of members (see
        static). If you are writing a set of macros some of which are
        internal and some for external use, then you can use the 'static'
        declaration specifier to restrict the visibility of a member
        variable or function.

        However as static functions are hidden from usage outside their
        own macro file (or module), this can present a problem with
        functionality which involves the usage of callbacks (e.g.
        assign_to_key). In this case, the :: (scope resolution) operator
        is used to qualify hidden names so that they can still be used.

        Multiple objects can contained within the same namespace. The
        module primitive allows you to refer to static functions defined
        in another macro file explicitly, using the "::" naming modifier,
        and also allows static macro functions to be accessed in
        callbacks. Upon a module being associated, it is possible to
        refer use the syntax "<module-name>::<function>" to refer to a
        function in callbacks.

    Example:

>       void
>       main()
>       {
>               module("my_module");
>               assign_to_key("<Alt-D>", "my_module::doit");
>
>               // which to has the identical behaviour as
>               assign_to_key("<Alt-D>", "::doit");
>
>               // and
>               assign_to_key("<Alt-D>", inq_module() + "::doit");
>       }
>
>       static void
>       doit()
>       {
>               :
>               :
>       }

    Macro Returns:
        The 'module' primitive returns 0 on success, otherwise 1 if
        module already existed or -1 if called outside the context of any
        macro file.

    Macro Portability:
        A Grief extension.

    Macro See Also:
        inq_module, scope, static, assign_to_key
 */
void
do_module(void)                 /* int (string namespace) */
{
    const char *module = module_identifier();
    const char *name_space = get_str(1);

    if (!mac_sd || !mac_validname(name_space)) {
        acc_assign_int((accint_t) -1);          /* invalid */

    } else {
        SPBLK *sp;
        MNAMESPACE *mp;
        MOBJECT *objp;

        if (NULL != (sp = splookup(name_space, mac_namespace))) {
            mp = (MNAMESPACE *) sp->data;        /* pre-existing */

            assert(MNAMESPACE_MAGIC == mp->n_magic);
            if (mp->n_count >= mp->n_alloced) {
                acc_assign_int((accint_t) -1);
                return;
            }
            acc_assign_int((accint_t) 1);

        } else {                                /* new module namespace */
            const int namelen = strlen(name_space);
            sp = (SPBLK *) spblk(sizeof(MNAMESPACE) + namelen + 1);

            mp = (MNAMESPACE *) sp->data;
            mp->n_magic   = MNAMESPACE_MAGIC;
            mp->n_ident   = ++x_modident;
            mp->n_alloced = MAX_NAMESPACES;
            mp->n_count   = 0;
            strcpy(mp->n_name, name_space);
            sp->key = (char *)mp->n_name;
            spenq(sp, mac_namespace);
            acc_assign_int((accint_t) 0);
        }
                                                /* object */
        if (NULL != (objp = mac_lookup(module))) {
            assert(MOBJECT_MAGIC == objp->o_magic);
            if (objp->o_namespace) {
                if (objp->o_namespace == mp) {
                    acc_assign_int((accint_t) 2);
                } else {
                    ewprintf("module already bound to '%s'.", objp->o_namespace->n_name);
                    acc_assign_int((accint_t) -1);
                }
            } else {                            /* push into object table */
                mp->n_objects[ mp->n_count++ ] = objp;
                objp->o_namespace = mp;
            }
        }
    }
}


/*  Function:           do_bless
 *      bless primitive.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 *
 *<<GRIEF>>
    Macro: bless - Associate an object with a class/module.

        int
        bless([int ref], [string classname])

    Macro Description:
        The 'bless' primitive is reserved for future use.

        This primitive informs the dictionary referenced by 'ref'
        that it is now an object in the 'classname' package. If
        'classname' is omitted, the current module is used. Because a
        bless is often the last thing in a constructor, it returns
        the reference for convenience.

        The 'bless' primitive enables Perl style objects.

    Note!:
        Consider always blessing objects in classname's that are
        mixed case, using a leading upper-case; Namespaces with all
        lowercase names are considered reserved for GRIEF pragmata.

    Macro Parameters:
        n/a

    Macro Returns:
        The 'bless' statement returns the dictionary identifier.

    Macro Portability:
        A Grief extension.

    Macro See Also:
        module
 */
void
do_bless(void)                  /* int (int, [string]) */
{
    //TODO
}


/*  Function:           mac_validname
 *      Validate a macro name.
 *
 *  Parameters:
 *      name - Macro name buffer.
 *
 *  Returns:
 *      TRUE on success otherwise FALSE.
 */
static int
mac_validname(const char *name)
{
    const unsigned char *s = (unsigned char *)name;

    for (;;) {
        const int c = *s;

        if (isalpha(c) || c == '_' ||           /* [A-Za-z_][A-Za-z_0-9]+ */
                (s != (unsigned char *)name && isdigit(c))) {
            if (*++s == '\0') {
                return TRUE;
            }
            continue;
        }
        break;
    }
    return FALSE;
}


/*  Function:           inq_module
 *      inq_module primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: inq_module - Retrieve the current module.

        string
        inq_module([int test = 1])

    Macro Description:
        The 'inq_module()' primitive retrieves the module name
        assigned with the calling macro. When loaded macros are
        automaticly assigned with a unique module name or can be
        specificly assigned using the <module> primitive.

        Module names are used to implement name spaces for static
        macro functions or to provide a way of creating a macro
        package split amongst source files.

    Example:

        The following example setups a static scoped callback function:

>       void
>       main(void)
>       {
>               register_callback( inq_module() + "::callback" );
>       }
>
>       static void
>       callback(void)
>       {
>       }

    Macro Returns:
        The 'inq_module()' primitive returns the name of the current
        module.

    Macro Portability:
        A Grief extension.

    Macro See Also:
        module, static, scope
 */
void
inq_module(void)                    /* ([int test = 1) */
{
    const char *module = module_identifier();
    const int test = get_xinteger(1, 0);

    if (module) {
        switch (test) {
        case 0: {       /* resolve namespace */
                const char *name_space = module_namespace(module);

                if (name_space) {
                    module = name_space;
                }
            }
            break;
        case 1:         /* module/object identifier */
            break;
        default:
            break;
        }
    }
    acc_assign_str(module ? module : "", -1);
}


/*  Function:           macro_list
 *      Retrieve a list of all current 'global' macros.
 *
 *  Parameters:
 *      void
 *
 *  Returns:
 *      Macro list address, NULL terminated list of const
 *      string pointers.
 */
const char **
macro_list(unsigned *count)
{
    int sz = (mac_count + 1) * sizeof(char *);  /* mac_count = upper bound */
    struct mac_walk macs = {0};

    macs.w_idx = 0;
    if (NULL == (macs.w_list = chk_calloc(sz, 1))) {
        return NULL;
    }
    spwalk(mac_global, mac_walk, (SPBLK **) &macs);
    if (count) {
        *count = macs.w_idx;
    }
    return macs.w_list;
}


/*
 *  Helper function 4 macro_list.
 */
static void
mac_walk(SPBLK *sp, void *arg)
{
    MACRO *mp = (MACRO *)sp->data;
    struct mac_walk *macs = (struct mac_walk *)arg;

    macs->w_list[ macs->w_idx++ ] = mp->m_name;
}


/*  Function:           do_macro
 *      Macro primitive.
 *
 *  List construct:
 *      [flags] - Optional flags.
 *      name - Macro name.
 *      list - List available reference.
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: macro - Define a macro body.

        declare
        macro(list declaration)

    Macro Description:
        The 'macro' internal primitive is the mechanism by which
        macros are imported during the loading of a macro object.

    Macro Parameters:
        decl - List containing the macro declaration, including
            scoping flags, macro name and the function body.

    Macro Note!:
        This interface should be considered as a *internal* primitive,
        reserved for exclusive use by the GRIEF Macro Compiler and
        may change without notice. Management of macro definitions
        plus any associated argument binding shall be handled
        automatically by the compiler.

        Registering an ill-formed macro declaration shall have
        undefined effects, most likely resulting in GRIEF crashes.

    Macro Returns:
        Result of any <main> and/or associated internal <_init>
        execution.

    Macro Portability:
        n/a

    Macro See Also:
        replacement

 *<<GRIEF>>
    Macro: replacement - Overload an existing macro definition.

        replacement <macro-definition>

    Macro Description:
        The 'replacement' macro modifier is used to explicity declare
        overloaded interfaces, which is a macro that supersedes (or
        complements) either another macro or directly executable
        function of the same name.

        Replacement macros are used to intercept calls to other
        macros or functions and are usually to enhance existing the
        functionality of thoses commands without rewriting them
        completely.

        For example the following macro replaces edit_file;

>           edit_file()
>           {
>               edit_file();
>               beep();
>           }

        Any reference to edit_file() in the replacement macro refers
        to the function which is replaces; hence in this example
        edit_file(), then calls the orginal edit_file() and then
        beeps() on its return.

        During a compile when a macro which overloads an built-in
        macro is encountered, the compiler shall generate a warning
        message. To suppress the warning the keyword replacement
        should be stated prior to the function name, as follows:

>           replacement
>           edit_file()
>           {
>                   edit_file();
>                   beep();
>           }

        Note that the only behaviour effected by the 'replacement'
        keyword is the warning suppression and defining a function
        the same as built-in shall always result in an implicit
        function overloading. As such the two examples are
        functionality equivalent.

    Macro Returns:
        n/a

    Macro Portability:
        n/a

    Macro See Also:
        macro, static, extern, global

 */
void
do_macro(int replacement)           /* (string name, [int flags], list def) */
{
    static LIST dummy_macro = F_HALT;           /* XXX - yuk/bad */
    const LIST *lp = get_list(1);
    accint_t options = 0;
    const char *name;
    const LIST *body;
    int flags = 0;

    __CUNUSED(replacement)

    /* options [optional] */
    if (atom_xint(lp, &options)) {
        lp = atom_next(lp);
        if (0x01 & options) {
            flags |= M_STATIC;                  /* .. module static macro */
        }

    /*
     *  TODO - overloading/replacement
     *
     *      builtins:
     *          Dont allow overloading unless stated.  Should always be the case
     *          as new-crunch implicity sets REPLACEMENT when a builtin name is used.
     *
     *      user:
     *          Allow overloading if stated (different module), otherwise override.
     *
     *  if (replacement || (0x02 & options)) {
     *      flags |= M_OVERLOAD;                // replacement/overload
     *  }
     */
    }

    /* macro name */
    if (NULL == (name = atom_xstr(lp))) {
        ewprintf("macro: missing name.");
        return;
    }
    lp = atom_next(lp);

    /* macro body */
    body = (atom_xlist(lp) ? lp : &dummy_macro);
    macro_define(name, flags, body);
}


/*  Function:           inq_macro
 *       inq_macro primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: inq_macro - Determine whether a macro or primitive exists.

        int
        inq_macro(string name, [int mode = 0])

    Macro Description:
        The 'inq_macro()' primitive tests whether the specified macro
        'name' exists within a stated namespace(s) defined by 'mode'.

    Macro Parameters:
        name - String containing the macro name to be tested.

        mode - Optional integer test type, if omitted defaults to 0,
            see table for mode behaviours.

    *Tests*

        The following test behaviours are available given by 'mode';

(start table,format=nd)
        [Value  [Description                                        ]
      ! 0x00    Default behaviour if 'mode' is omitted.

                inq_macro tests specified macro name against both
                runtime loaded macros and built-in primitive.

                It returns 1 if the macro exists (loaded, autoload or
                a primitive replacement), 0 if it is a built-in
                primitive, otherwise -1 if neither a built-in
                primitive nor macro.

                Note: The returns are incompatible with the original
                BRIEF implementation.

      ! 0x01    inq_macro only tests whether the specified macro
                'name' is runtime loaded macro, ignoring built-in
                primitives.

                It returns 1 if the macro is loaded, 2 if it exists
                as an autoload but has yet to be loaded otherwise 0
                if the macro does not exist.

      ! 0x02    inq_macro returns the assigned positive module number
                (see inq_module) if the macro has been loaded, 0 if
                it is an autoload, otherwise -1 if the macro does not
                exist.

      ! 0x03    inq_macro only tests whether the specified macro
                'name' is a primitive, ignoring runtime loaded macros.

                It returns 1 if the primitive exists, 2 if it has
                been replaced (see replacement) otherwise 0 if the
                primitive does not exist.
(end table)

    Macro Returns:
        The 'inq_macro()' primitive returns a mode specified value,
        otherwise -1 if the stated 'mode' was invalid.

    Macro Portability:
        The autoload visibility under mode 0x01 and mode 0x03 are
        Grief extensions.

    Macro See Also:
        load_macro
 */
void
inq_macro(void)                     /* (string name, [int test]) */
{
    const char *name = get_str(1);
    const int test = get_xinteger(2, -1);
    const MACRO *mptr = macro_lookup(name);
    int ret = -1;

    switch (test) {
    case 1:                 /* exists */
        if (mptr) {
            ret = (M_AUTOLOAD & mptr->m_flags ? 2 : 1);
        } else {
            ret = 0;
        }
        break;
    case 2:                 /* moduleid, common to all macros within an object */
        if (mptr) {
            const MOBJECT *objp = mac_lookup(mptr->m_module);

            assert(objp);
            ret = (objp ? objp->o_ident : -1);
        } else {
            ret = -1;
        }
        break;
    case 3: {               /* primitive */
            const BUILTIN *bp;

            if (NULL != (bp = builtin_lookup(name))) {
                ret = (bp->b_first_macro ? 2 : 1);
            } else {
                ret = 0;
            }
        }
        break;
    case 9:                 /* unique macro identifier (undocumented) */
        if (mptr) {
            ret = mptr->m_ident;
        } else {
            ret = -1;
        }
        break;
    default:                /* standard */
        if (mptr) {
            ret = 1;                            /* found */
            if (M_OVERLOAD & mptr->m_flags) {
                ret = 2;                        /* replacement */
            }
        } else {
            const BUILTIN *bp = builtin_lookup(name);

            if (bp) {
                ret = 0;                        /* builtin */
            } else {
                ret = -1;                       /* unknown */
            }
        }
        break;
    }
    acc_assign_int((accint_t) ret);
}


/*  Function:           macro_define
 *      Insert a new macro into the specified table.
 *
 *  Parameters:
 *      name - Macro name.
 *      lp - Definition.
 *      flags - M_AUTOLOAD or M_STATIC.
 *
 *  Returns:
 *      xxx
 */
int
macro_define(const char *name, int flags, const LIST *lp)
{
    static const char who[] = "load_macro";
    const char *module = module_identifier();
    BUILTIN *bp = builtin_lookup(name);
    MACRO *mptr = NULL;

    assert(mac_sd && mac_sp);
    assert(module);
    assert('$' == *module);
    assert(0 == flags || M_AUTOLOAD == flags || M_STATIC == flags);

    /*
     *  Basic tests
     */
    if (M_STATIC & flags) {
        if (NULL != bp) {
            infof("%s: overloaded builtin '%s' cannot be static, promoted.", who, name);
            flags &= ~M_STATIC;

        } else if (M_OVERLOAD & flags) {
            infof("%s: overloaded '%s' cannot be static, promoted.", who, name);
            flags &= ~M_STATIC;
        }
    }

    /*
     *  Test existing tables/
     *   a) If STATIC, test module specific
     *   b) global
     */
    mptr = macro_find(name, (M_STATIC & flags ? module : NULL));

    /*
     *  autoload or redefinition ...
     */
    if (NULL != mptr) {
        assert((M_AUTOLOAD & flags) == 0);

        if (M_AUTOLOAD & mptr->m_flags) {
            /*
             *  Release autoload storage and reuse node
             */
            if (NULL == bp && (flags & M_STATIC)) {
                infof("%s: autoload on the static macro '%s'.", who, name);
            }
            chk_free((void *)mptr->m_list);
            mptr->m_list = NULL;
            if (NULL != bp) {
                flags |= M_OVERLOAD;            /* builtin, implied OVERLOAD */
            }

        } else if ((flags & M_STATIC) && mptr->m_module != module) {
            /*
             *  Diff module/scope
             */
            if (0 == (mptr->m_flags & M_STATIC)) {
                infof("%s: '%s::%s' hides global.", who, module, name);
            }
            mptr = NULL;

        } else if (mptr->m_module != module &&
                        (NULL != bp || (M_OVERLOAD & flags))) {
            /*
             *  Overloading
             */
            infof("%s: overloading '%s'", who, name);
            flags |= M_OVERLOAD;
            mptr = NULL;                        /* create new load */
            assert(bp);                         /* XXX - non-built not as yet supported */

        } else {
            /*
             *  Redefinition, reuse
             */
            if (flags == mptr->m_flags) {
                infof("%s: '%s' redefined.", who, name);
            } else {
                ewprintf("%s: '%s' redefined with scope change, original retained.", who, name);
                flags |= (mptr->m_flags & (M_OVERLOAD|M_STATIC));
            }
            macro_delete(mptr->m_list);         /* remove image */
            mptr->m_list = NULL;
        }
    }

    /*
     *  new macro/overloading
     */
    if (NULL == mptr) {
        SPBLK *sp = (SPBLK *) spblk(sizeof(MACRO));

        mptr = (MACRO *) sp->data;
        mptr->m_magic   = MACRO_MAGIC;
        mptr->m_name    = chk_salloc(name);
        mptr->m_next    = NULL;

        sp->key = (char *)mptr->m_name;
        if (NULL != bp) {                       /* overload builtin */
            mptr->m_next = bp->b_first_macro;
            bp->b_first_macro = bp->b_macro = mptr;
            bp->b_flags |= B_REDEFINE;
            if (NULL == mptr->m_next) {
                spenq(sp, mac_global);
            } else {
                spfreeblk(sp);
                sp = NULL;
            }
            flags |= M_OVERLOAD;

        } else if (0 == (flags & M_STATIC)) {   /* global/autoload */
            spenq(sp, mac_global);

        } else {                                /* object specific */
            MOBJECT *objp = mac_lookup(module);

            assert(objp);
            spenq(sp, objp->o_macros);
        }
        ++mac_count;                            /* total macro count */
    }

    /*
     *  assign or (re)assign
     */
    mptr->m_module  = module;
    mptr->m_ident   = ++x_macident;
    mptr->m_ftime   = TRUE;
    mptr->m_list    = lp;
    mptr->m_flags   = (uint16_t)flags;
    xf_initdef |= (0 == strcmp(name, "_init"));
    return 0;
}


/*  Function:           macro_delete
 *      Delete the specified macro.
 *
 *  Parameters:
 *      list - Macro body.
 *
 *  Returns:
 *      nothing
 */
int
macro_delete(const LIST *list)
{
    (void) list;                                /* unimplemented */
    return 0;
}


/*  Function:           macro_startup
 *      Function called at startup to get the show going ...
 *      need to load the 'grief' macro and execute it.
 *
 *      If the 'GRINIT_MACRO' macro not defined then we must have had a
 *      problem with our environment variables, so let the caller know.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 */
int
macro_startup(const char *name)
{
    assert(0 == mac_sd && 0 == mac_sp);
    mac_sp = mac_stack - 1;

    if (NULL == name) name = GRINIT_MACRO;
    execute_str(name);
    return macro_lookup(name) == NULL ? -1 : 0;
}


/*  Function:           macro_load
 *      Search and load the specified macro file.
 *
 *  Parameters:
 *      filename - Macro object filename.
 *
 *  Returns:
 *      1   Parse/syntax error.
 *      0   Success.
 *      -1  File open error.
 */
int
macro_load(const char *filename)
{
    const char **bpaths = mac_bpath();          /* search paths */
    char filepath[MAX_PATH];
    int i;

    if (sys_isabspath(filename)) {
        static const char *abspath[] =  {       /* absolute path */
                "", NULL
                };
        bpaths = abspath;
    }

    for (i = 0; NULL != bpaths[i]; ++i) {
        int ret;

        if (*bpaths[i]) {
            sxprintf(filepath, sizeof(filepath), "%s%s%s", bpaths[i], sys_delim(), filename);
        } else {
            strxcpy(filepath, filename, sizeof(filepath));
        }

        if ((ret = mac_read(filepath, filename)) >= 0) {
            trace_log("macro_load=%s\n", filepath);
            return ret;
        }
    }

    if (FALSE == x_mflag && !xf_findmacro) {
        infof("load_macro: file %s not found", filename);
    }

    trace_log("macro_load=%s (-1)\n", filename);
    return -1;
}


/*  Function:           macro_autoload
 *      Autoload the macro (if required).
 *
 *  Parameters:
 *      mptr - Macro definition.
 *      resolve - Whether the macro must be resolved.
 *
 *  Notes:
 *      When autoloads have been assigned to symbols at the completion of the
 *      object load the symbol shall still have M_AUTOLOAD asserted, hence
 *      the need for the 'resolve' option.
 *
 *      The M_UNREOLVED is still set to stop additional load attempts from
 *      occurring.
 *
 *  Returns:
 *      1   Already loaded.
 *      0   Success.
 *      -1  File open error.
 *      -2  Parse/syntax error.
 *      -3  Loaded but unresolved.
 */
int
macro_autoload(MACRO *mptr, int resolve)
{
    int ret;

    if (mptr->m_flags & M_UNRESOLVED) {
        ret = -3;                               /* loaded, but unresolved */

    } else if (M_AUTOLOAD & mptr->m_flags) {
        /*
         *  note: m_list is overloaded, containing the module name.
         */
        ret = macro_load((const char *) mptr->m_list);
        if (1 == ret) {
            ret = -2;                           /* syntax error */

        } else if (0 == ret) {
            if (M_AUTOLOAD & mptr->m_flags) {
                mptr->m_flags |= M_UNRESOLVED;
                if (resolve) {                  /* symbols wont be resolved */
                    ewprintf("load_macro: macro '%s' not resolved during autoload.", mptr->m_name);
                    ret = -3;
                }
            }
        }

    } else {
        ret = 1;                                /* already loaded and defined */
    }

    return ret;
}


/*  Function:           macro_loaded
 *      Determine the given macro file/object has been loaded.
 *
 *  Parameters:
 *      filename - Macro path.
 *
 *  Returns:
 *      TRUE is loaded, otherwise FALSE.
 */
/*__CBOOL*/ int
macro_loaded(const char *filename)
{
    const char **bpaths = mac_bpath();          /* search paths */
    char filepath[MAX_PATH], *ext;
    int x, i;

    if (sys_isabspath(filename)) {
        static const char *abspath[] =  {       /* absolute path */
                "", NULL
                };
        bpaths = abspath;
    }

    for (i = 0; NULL != bpaths[i]; ++i) {
        if (bpaths[i][0]) {
            sxprintf(filepath, sizeof(filepath), "%s%s%s", bpaths[i], sys_delim(), filename);
        } else {
            strxcpy(filepath, filename, sizeof(filepath));
        }

        if (NULL != (ext = strrchr(filename, '.'))) {
            for (x = 0; mac_exts[x]; ++x) {
                if (0 == file_cmp(ext, mac_exts[x])) {
                    if (mac_object(filepath, NULL)) {
                        return TRUE;
                    }
                    return FALSE;
                }
            }
        } else {
            ext = filepath + strlen(filepath);
            for (x = 0; mac_exts[x]; ++x) {
                strcpy(ext, mac_exts[x]);
                if (mac_object(filepath, NULL)) {
                    return TRUE;
                }
            }
        }
    }
    return FALSE;
}


static int
mac_find(const char *filename)
{
    int ret;

    xf_findmacro = TRUE;
    ret = macro_load(filename);
    xf_findmacro = FALSE;
    return (ret);
}


/*  Function:           mac_bpath
 *      Busts the GRPATH environment variable and builds a list of absolute directories
 *      to be used during macro object searchs.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      Array of macro paths.
 */
static char const **
mac_bpath(void)
{
    char abspath[MAX_PATH+1];
    const char *delimiter = sys_pathdelimiter();
    const char *path, *bpath;
    char *cp, *buf;
    int i = 0;

    if (mac_paths[0]) {
        return mac_paths;
    }

    /* GRPATH env */
    if (NULL == (bpath = ggetenv("GRPATH"))) {
        bpath = "";
    }

    trace_log("GRPATH\n");
    buf = chk_salloc(bpath);
    for (cp = strtok(buf, delimiter);           /* system delimiter */
            cp != NULL && i < (MAX_MACRODIRS - 4); cp = strtok(NULL, delimiter)) {
        char t_path[MAX_PATH];

        path = t_path;
        if (NULL != file_expand(cp, t_path, sizeof(t_path))) {
            abspath[0] = 0;
            if (0 == sys_realpath((const char *)t_path, abspath, sizeof(abspath)) && abspath[0]) {
                path = abspath;
            }
            mac_paths[i++] = chk_salloc(path);
            trace_log("\tPATH: %s\n", path);
        }
    }

    if (0 == xf_restrict) {                     /* current and home */
        abspath[0] = 0;
        if (0 == sys_realpath(".", abspath, sizeof(abspath)) && strlen(abspath) > 0) {
            trace_log("\tCWD:  %s\n", abspath);
            mac_paths[i++] = chk_salloc(abspath);
        }

        if (NULL != (path = sysinfo_homedir(NULL, -1)) && strlen(path) > 0) {
            trace_log("\tHOME: %s\n", path);
            mac_paths[i++] = chk_salloc(path);
        }
    }

    mac_paths[i] = NULL;
    chk_free(buf);
    return mac_paths;
}


static int
mac_read(char *filepath, const char *macroname)
{
    char *ext;
    int x, ret = -1;

    trace_log("load_macro(%s): %s\n", macroname, filepath);

    /*
     *  If a known extension is specified, then go directly for that file
     */
    if (NULL != (ext = strrchr(filepath, '.'))) {
        for (x = 0; mac_exts[x]; ++x) {
            if (0 == file_cmp(ext, mac_exts[x])) {
                if ((ret = mac_parse(filepath, macroname)) >= 0) {
                    goto end_of_function;
                }
                return -1;                      /* error, about */
            }
        }
    }
    ext = filepath + strlen(filepath);

    /*
     *  We didn't have an extension, so now try known extensions.
     *
     *      Note: ".cr" files only whilst executing find_macro().
     */
    for (x = 0; mac_exts[x]; ++x) {
        if (! xf_findmacro && 0 == strcmp(mac_exts[x], ".cr")) {
            continue;
        }
        strcpy(ext, mac_exts[x]);               /* append extension */
        if ((ret = mac_parse(filepath, macroname)) >= 0) {
            break;
        }
    }

end_of_function:
    /*
     *  If we had a successful load, and we're executing the find_macro()
     *  primitive, then assign the file name to the accumulator.
     */
    if (xf_findmacro && ret >= 0) {
        acc_assign_str(filepath, -1);
    }
    return ret;
}


/*  Function:           mac_hash
 *      Hash the filename.
 *
 *  Parameters:
 *      str - File path.
 *      hash - Initial hash.
 *
 *  Returns:
 *      Resulting hash.
 */
static uint32_t
mac_hash(const char *filpath, uint32_t hash)
{                                               /* remove extension */
    const unsigned char *dot = (void *)strrchr(filpath, '.');
    const unsigned char *s = (const unsigned char *)filpath;
    int c;

    while (s != dot && (c = *s++) != '\0') {
        if ('\\' == c) {
            c = '/';
        } else {
            c = tolower(c);
        }
        hash = c + (hash << 6) + (hash << 16) - hash;
    }
    return hash;
}


/*  Function:           mac_object
 *      Generate a unique (default) module identifier for the given macro object.
 *
 *  Parameters:
 *      filepath - File path (e.g. "/usr/lib/grief/macros/restore.cm").
 *      macroname - Macro name (e.g. "restore").
 *
 *  Returns:
 *      Object identifier.
 */
static const MOBJECT *
mac_object(const char *filepath, const char *macroname)
{
    uint32_t hash = 0;                          /* seed hash */
    char module[MOBJECT_MODULELEN] = {0};
    MOBJECT *objp = NULL;

    do {
        hash = mac_hash(filepath, hash);        /* hash, $=denotes internal */
        sxprintf(module, sizeof(module), "$%04x", (unsigned)(hash & 0xffff));

        if (NULL != (objp = mac_lookup(module))) {
            if (file_cmp(objp->o_filepath, filepath) != 0) {
                objp = NULL;                    /* found, rehash */

            } else if (macroname) {
                infof("load_macro: object being reloaded.");
                zap_object((void *)objp, (void *)1);
             /* macro_delete(); FIXME, object memory leak */
                objp->o_macros  = spinit();
                objp->o_symbols = spinit();
            }

        } else {
            int filelen, macrolen;
            SPBLK *op;
            char *p;

            if (NULL == macroname) {
                return NULL;
            }

            filelen = (int)strlen(filepath);
            macrolen = (int)strlen(macroname);

            op = spblk(sizeof(MOBJECT) + macrolen + filelen + 2);
            objp = (MOBJECT *)op->data;
            objp->o_magic     = MOBJECT_MAGIC;
            strcpy(objp->o_module, module);
            objp->o_ident     = (uint16_t) (hash & 0xffff);
            objp->o_macros    = spinit();
            objp->o_symbols   = spinit();
            objp->o_macroname = objp->o_buffer;
            objp->o_filepath  = objp->o_buffer + macrolen + 2;
            objp->o_namespace = NULL;           /* module(<namespace>) */

            for (p = (char *)objp->o_macroname; *macroname; ++p) {
                if ('\\' == (*p = *macroname++)) {
                    *p = '/';
                }
            }
            *p = 0;

            for (p = (char *)objp->o_filepath; *filepath; ++p) {
                if ('\\' == (*p = *filepath++)) {
                    *p = '/';
                }
            }
            *p = 0;

            op->key = objp->o_module;
            spenq(op, mac_objects);
        }
    } while (NULL == objp);

    return objp;
}


/*  Function:           loadmacro
 *      cm_parse() callback (see below).
 *
 *  Parameters:
 *      lp - Macro definition.
 *      size - List size, in bytes.
 *
 *  Returns:
 *      nothing
 */
static void
loadmacro(const LIST *lp, int size)
{
    __CUNUSED(size)
    execute_macro(lp);                          /* ^macro_define shall be called!! */
}


/*  Function:           mac_parse
 *      Parse and initialise the module.
 *
 *  Paraemeters:
 *      filepath - Full path to macro.
 *      macroname - Macro name.
 *
 *  Returns:
 *      1               Parse/syntax error.
 *      0               Success.
 *      -1              File open error.
 */
static int
mac_parse(const char *filepath, const char *macroname)
{
    const MOBJECT *objp;
    int ret = -1;

    if (xf_findmacro) {                         /* if finding, dont load the macro object */
        return 1;
    }

    if (cm_push(filepath) < 0) {                /* initialise language subsystem */
        return -1;
    }

    if (NULL == (objp = mac_object(filepath, macroname))) {
        cm_pop();
        return -1;                              /* creation error */
    }

    if (mac_sd >= MAX_MACSTACK) {
        panic("Macro stack overflow (%d).", mac_sd);

    } else {                                    /* --- push */
        register struct mac_stack *stack = mac_stack + mac_sd++;
        mac_registers_t *regs;

        ++mac_sp; assert(stack == mac_sp);

        stack->module = objp->o_module;
        stack->name   = "_init";
        stack->argv   = NULL;
        stack->argc   = 0;
        stack->level  = x_nest_level;
        if (NULL != (regs = stack->registers)) {
            assert(REGISTERS_MAGIC == regs->magic);
            memset(&regs->symbols, 0, sizeof(SYMBOL *) * (regs->slots + 1));
        }

        /* parse/load the object */
        trace_ilog("object: fname:%s, mname:%s = %s\n", \
            filepath, macroname, stack->module);

        xf_initdef = FALSE;
        ret = cm_parse(loadmacro, NULL);        /* 0=success, otherwise -1 on error */
        if (xf_initdef) {
            execute_str("_init");               /* shall be module::_init */
        }

        assert(mac_sp == (mac_stack + (mac_sd - 1)));
        --mac_sd, --mac_sp;                     /* --- pop */
    }

    return (ret ? 1 : 0);
}


static MOBJECT *
mac_lookup(const char *module)
{
    if (module) {
        SPBLK *op;

        assert('$' == *module);
        if (NULL != (op = splookup(module, mac_objects))) {
            MOBJECT *objp = (MOBJECT *) op->data;

            assert(MOBJECT_MAGIC == objp->o_magic);
            return objp;
        }
    }
    return NULL;
}


/*
 *  Search the object specific symbol space.
 */
static SPBLK *
mac_objsearch(const char *module, const char *name)
{
    SPBLK *op, *sp = NULL;

    if ('$' == *module) {                       /* object identifier (e.g. "$1234") */
        const MOBJECT *objp = mac_lookup(module);

        if (objp) {
            assert(MOBJECT_MAGIC == objp->o_magic);
            sp = splookup(name, objp->o_macros);
        }

    } else {                                    /* namespace (e.g. module("mymodule")) */
        if (NULL != (op = splookup(module, mac_namespace))) {
            MNAMESPACE *mp = (MNAMESPACE *)op->data;
            const MOBJECT *objp;
            int i = 0;

            assert(MNAMESPACE_MAGIC == mp->n_magic);
            do {
                assert(i < MAX_NAMESPACES);
                if (NULL != (objp = mp->n_objects[i])) {
                    assert(MOBJECT_MAGIC == objp->o_magic);
                    assert('$' == objp->o_module[0]);
                    sp = splookup(name, objp->o_macros);
                }
            } while (NULL == sp && ++i < mp->n_count);
        }
    }
    return sp;
}


/*  Function:           do_autoload
 *      autoload primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: autoload - Register location of one or more macros.

        void
        autoload(string filename, string function, ...)

    Macro Description:
        The 'autoload()' primitive informs the macro loader about the
        existence of global macros. The primitive registers the
        specified macro 'function' against the macro object image
        'filename', permitting an arbitrary number of macro names
        after the initial function; as such one or more function
        references may be associated.

        Whenever the macro engine encounters an undefined macro
        reference, it searches the internal autoload list for a
        definition. If available, the associated macro object shall
        be loaded as normal, with the loader searching the <GRPATH>
        definition for the underlying object image.

        If during an autoload driven load the referenced function was
        not resolved the following diagnostic shall be generated.

>           load_macro: macro 'xxx' not resolved during autoload.

        A function reference may only be associated with one macro
        object, with any secondary references ignored generating the
        following diagnostic message.

>           autoload: 'xxx' already defined.

     *Example*

        The macros 'modeline' and 'mode' are registers against the
        macro object 'modeline'.

>           autoload("modeline", "modeline", "mode");

    Macro Parameters:
        filename - String that contains the name of the compiled
            macro file.

        function - String containing the name of the macro function.

        ... - Additional macro function names.

    Macro Returns:
        nothing

    Macro Portability:
        n/a

    Macro See Also:
        require
 */
void
do_autoload(void)               /* void (filename, function, ...) */
{
    const char *macroname = get_str(1);
    const LIST *nextlp, *lp;

    for (lp = get_list(2); (nextlp = atom_next(lp)) != lp; lp = nextlp) {
        LISTV result;
        const int type = eval(lp, &result);
        const char *name;

        if (F_LIT == type || F_STR == type) {
            name = result.l_str;

        } else if (F_RSTR == type) {
            name = r_ptr(result.l_ref);

        } else {
            ewprintf("autoload: invalid parameter");
            return;
        }

        if (NULL == macro_lookup(name)) {
            macro_define(name, M_AUTOLOAD, (LIST *)chk_salloc(macroname));
        } else {
            ewprintf("autoload: '%s' already defined.", name);
        }
    }
}


/*  Function:           do_load_macro
 *      do_load_macro primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: load_macro - Load a macro object.

        int
        load_macro(string filename, [int reload = 1])

    Macro Description:
        The 'load_macro()' primitive loads the specified macro object
        'filename', resolving all external references in the file.

        If 'filename' does not contain a path specification, then all
        directories contained within the <GRPATH> environment
        variable are searched.

        If 'reload' is either omitted or non-zero, the macro shall be
        reloaded regardless of whether the object image has already
        been loaded using load_macro or a <autoload> reference.
        Otherwise if zero the image shall not be reloaded if a image
        with the same name was loaded previously.

        Once a successful load the macro objects <main> entry point
        is executed.

        Note!:
        Upon loading pre-compiled macro objects (.cm), several
        interface checks occur confirming whether the object is
        compatible with the current GRIEF version, for example engine
        version number and primitive count. Check failure results in
        a load error and suitable diagnostics being echoed on the
        command prompt.

    Macro Parameters:
        filename - String containing the macro object to be loaded.
            If the specified file has an extension, then an exact
            match is located. Otherwise files matching one of the
            following extensions are '.cm', '.cr', and '.m' are
            located.

        reload - Optional boolean flag, if given as *false* and the
            same macro object was previously loaded, nothing shall be
            loaded.

    Macro Returns:
        The 'load_macro()' primitive returns 1 if macro file was
        successfully loaded; return 2 if 'reload' is non-zero an a
        loaded image already exists; otherwise 0 on failure.

    Macro Portability:
        Unlike BRIEF, macros in source form (.m files) can be loaded
        with this macro.

        The 'reload' option is an Grief extension.

    Macro See Also:
        autoload, inq_macro, main

 *<<GRIEF>>
    Macro: main - Macro entry point.

        void
        main()

    Macro Description:
        The 'main' macro is an optional callback which may be defined
        within each macro source module.

        Upon a macro load, the macro 'main' is executed if it is
        present within the macro object. This macro can be used to
        initialise and perform load-time specific variable
        initialisation and/or functions.

        Note!:
        The main function is indirectly executed on the successful
        loading of a macro object by the <_init> entry point.

    Macro Returns:
        nothing

    Macro Portability:
        n/a

    Macro See Also:
        _init, load_macro

 *<<GRIEF>> [callback]
    Macro: _init - Internal macro initialisation.

        void
        _init()

    Macro Description:
        The '_init' macro is utilised by the Compiler to set-up file
        level variables (see global).

    Macro Note!:
        This interface is an internal primitive, reserved for
        exclusive use by the GRIEF Macro Compiler and may change
        without notice. Management of variable scoping and argument
        binding shall be handled automatically by the compiler.

    Macro Returns:
        nothing

    Macro Portability:
        n/a

    Macro See Also:
        main, global, load_macro
 */
void
do_load_macro(void)             /* int (string filename, [int reload=1]) */
{
    const int reload = get_xinteger(2, 1);
    const char *filepath;
    char buf[MAX_PATH] = {0};
    int loaded;

    acc_assign_int((accint_t) 0);
    if (NULL == (filepath = get_arg1("Macro file: ", buf, sizeof(buf)))) {
        return;
    }

    loaded = macro_loaded(filepath);
    if (loaded && !reload) {
        if (xf_warnings) {
            infof("Macro file already loaded successfully.");
        }
        acc_assign_int((accint_t) 2);

    } else if (0 == macro_load(filepath)) {
        infof("Macro file %sloaded successfully.", (loaded ? "re" : ""));
        acc_assign_int((accint_t) 1);
    }
}


/*  Function:           do_delete_macro
 *      do_delete_macro primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: delete_macro - Delete a macro from memory.

        void
        delete_macro(string macro)

    Macro Description:
        The 'delete_macro' macro deletes a macro file, which contains
        one or more macros, from memory; the underlying macro object
        retains unchanged.

        Any keyboard references and variables, both global and buffer
        will be retained. If any of the macros within the deleted
        macro object are referenced at a later time, the normal macro
        load logical is applied which may in turn reload the object.

    Note!:
        This primitive is provided for BRIEF compatibility and is
        currently a no-op as macros are not directly deletable. The
        storage allocated to a macro shall be lost when a macro by
        the same name is loaded from another file.

    Macro Parameters:
        macro - String containing the name of a macro file to be
            deleted. If no extension is supplied, then '.cm' shall be
            appended. If omitted the user is prompted.

    Macro Returns:
        nothing

    Macro Portability:
        Not implemented.

    Macro See Also:
        load_macro
 */
void
do_delete_macro(void)           /* int (string macro) */
{
    /*unimplemented*/
}


/*  Function:           do_find_macro
 *      find_macro primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: find_macro - Determine path of a macro object.

        string
        find_macro(string filename)

    Macro Description:
        The 'find_module' resolves the full-path to the macro object
        'filename'. Using the same mechanism as <load_macro>, the
        macro object search path environment variable <GRPATH> is
        utilised to search for the specified macro object.

    Macro Parameters:
        filename - String containing the macro object to resolve. If
            the specified file has an extension, then an exact match
            is located. Otherwise files matching one of the following
            extensions are '.cm', '.cr', and '.m' are located.

    Macro Returns:
        The 'find_module' returns a string containing the full-path
        to the resolved macro object, otherwise an empty string if
        the object could not be found.

    Macro Portability:
        n/a

    Macro See Also:
        autoload, load_macro

 */
void
do_find_macro(void)             /* string (string filename) */
{
    if (mac_find(get_str(1)) < 0) {
        acc_assign_str("", 0);
    }
}


/*  Function:           do_require
 *      require primitive.
 *
 *  Parameters:
 *      none
 *
 *<<GRIEF>>
    Macro: require - Enforce the use of an external module.

        int
        require(string filename)

    Macro Description:
        The 'require()' primitive enforces that the specified macro
        object 'filename' be loaded if it has not already been loaded.

        This primitive is similar to using <load_macro> with a
        'reload' option of zero, but is provided as a more convenient
        and explicit form.

        The best practice of this primitive, use 'require' within the
        'main' function of any given macro object.

>           void main(void)
>           {
>               require("utils");   // our external dependency
>           }

    Note!:
        <autoload> and 'require' usage should generally be exclusive for
        any given macro object.

        Within CRiSPEdit(tm) documentation it is stated that the results
        are undefined due to the tables maintained for each are
        separate which can result in the macro being mistakenly
        loaded more then once. As such the side-effect of the reload
        shall be any global state the macro was maintained shall be
        lost.

        Under GRIEF their usage *may* be mixed yet for compatibility
        this style of usage should be avoided.

    Macro Returns:
        Returns 0 if already loaded; 1 if the macro as loaded. Otherwise
        -1 if the macro file could not be loaded.

    Macro Portability:
        n/a

    Macro See Also:
        load_macro, autoload
 */
void
do_require(void)                /* int (string filename) */
{
    const char *filepath = get_str(1);
    int ret;

    if (xf_warnings) {                          /* check for autoload() references */
        MACRO *mptr = macro_lookup(filepath);

        if (mptr && (M_AUTOLOAD & mptr->m_flags)) {
            ewprintf("require: '%s' is marked as autoload.", filepath);
        }
    }

    if (macro_loaded(filepath)) {
        ret = 0;                                /* already loaded */
    } else if (0 == macro_load(filepath)) {
        ret = 1;                                /* load success */
    } else {
        ret = -1;                               /* otherwise error */
    }
    acc_assign_int((accint_t) ret);
}
/*end*/

