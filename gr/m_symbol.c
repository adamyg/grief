#include <edidentifier.h>
__CIDENT_RCSID(gr_m_symbol_c,"$Id: m_symbol.c,v 1.39 2019/01/27 13:51:24 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: m_symbol.c,v 1.39 2019/01/27 13:51:24 cvsuser Exp $
 * Symbol primitives.
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

#include <sys/stat.h>

#include <editor.h>
#include <edconfig.h>
#include <edenv.h>                              /* ggetenv */
#include <edcm.h>
#include <libstr.h>                             /* str_...()/sxprintf() */

#include "m_symbol.h"                           /* public interface */

#include "accum.h"                              /* acc_...() */
#include "builtin.h"                            /* margv */
#include "debug.h"
#include "echo.h"
#include "eval.h"
#include "keywd.h"
#include "lisp.h"
#include "macros.h"
#include "main.h"
#include "math.h"
#include "symbol.h"
#include "word.h"

static void             i_currentline(SYMBOL *sp);
static void             i_currentcol(SYMBOL *sp);
static void             i_currentbuffer(SYMBOL *sp);
static void             i_currentwindow(SYMBOL *sp);
static void             s_getenv(SYMBOL *sp);

static const struct {   /* string values */
    const char *        tag;
    const char **       value;
} svalues[] = {
        { "CRISP_OPSYS",        &x_machtype     },
        { "GRPROGNAME",         &x_progname     },
    /*- { "CRISP_DELIM",        NULL            }, -*/
    /*- { "CRISP_SLASH",        NULL            }, -*/
    /*- { "CRISP_DIRSEP",       NULL            }, -*/
        };


/*
 *<<GRIEF>> [macro]
    Constant: GRRESTORE_FILE - GRIEF restore file name.

        extern const string
        GRRESTORE_FILE;

    Constant Description:
        The 'GRRESTORE_FILE' global constant is assigned to the
        system dependent default file name, for example '.grief'.

    Constant See Also:
        GRRESTORE_FILE, GRSTATE_FILE, GRSTATE_DB, GRINIT_FILE

 *<<GRIEF>> [macro]
    Constant: GRSTATE_FILE - GRIEF state file name.

        extern const string
        GRSTATE_FILE;

    Constant Description:
        The 'GRSTATE_FILE' global constant is assigned to the system
        dependent default file name, for example '.grstate'.

    Constant See Also:
        GRRESTORE_FILE, GRSTATE_FILE, GRSTATE_DB, GRINIT_FILE

 *<<GRIEF>> [macro]
    Constant: GRSTATE_DB - GRIEF state database name.

        extern const string
        GRSTATE_DB;

    Constant Description:
        The 'GRSTATE_DB' global constant is assigned to the system
        dependent default file name, for example 'grstatedb'.

    Constant See Also:
        GRRESTORE_FILE, GRSTATE_FILE, GRSTATE_DB, GRINIT_FILE

 *<<GRIEF>> [macro]
    Constant: GRINIT_FILE - GRIEF initialisation name.

        extern const string
        GRINIT_FILE;

    Constant Description:
        The 'GRINIT_FILE' global constant is assigned to the system
        dependent initialisation file name.

        The GRINIT_FILE file contains optional runtime configuration
        settings to initialise GRIEF when it starts.

            o On Unix systems, ".grinit".

            o On Windows systems, "_grinit".

     Location::

        The standard location of the configuration is within the home
        directory.

            o On Unix style systems, the home directory of the
                current user is specified by the $HOME environment
                variable; this is also represented by the '~'
                directory.

>                   ~/.grinit

            o On Windows systems, the home directory is normally a
                derived from the $HOMEDRIVE and $HOMEPATH environment
                variables or system calls.

>                   $HOMEDRIVE\$HOMEPATH\_grinit

        Within all environments the <GRPROFILE> override can be used
        to state an alternative location.

        The 'sysinfo' macro can be used to see the current home
        and/or profile specifications.

    Constant See Also:
        GRRESTORE_FILE, GRSTATE_FILE, GRSTATE_DB, GRINIT_FILE

 *<<GRIEF>> [macro]
    Constant: GRLOG_FILE - GRIEF diagnostics log file name.

        extern const string
        GRLOG_FILE;

    Constant Description:
        The 'GRLOG_FILE' global constant is assigned to the system
        dependent default file name:

        o On Unix systems, ".grief.log".

        o On Windows systems, "grief.log".

 */

static const struct {   /* string constant's */
    const char *        tag;
    const char *        value;
} sconsts[] = {
        { "GRRESTORE_FILE",     GRRESTORE_FILE  },
        { "GRSTATE_FILE",       GRSTATE_FILE    },
        { "GRSTATE_DB",         GRSTATE_DB      },
        { "GRINIT_FILE",        GRINIT_FILE     },
        { "GRLOG_FILE",         GRLOG_FILE      },
        { "GREXTENSION",        CM_EXTENSION    }
        };

static const struct {   /* integer constant's */
    const char *        tag;
    accint_t            value;
} iconsts[] = {
        /* open modes */
#if !defined(O_TEXT)
#define O_TEXT                  0
#endif
#if !defined(O_BINARY)
#define O_BINARY                0
#endif
#if !defined(O_SYNC)
#define O_SYNC                  0
#endif
#if !defined(O_NOINHERIT)
#define O_NOINHERIT             0
#endif
        { "O_CREAT",            O_CREAT         },
        { "O_EXCL",             O_EXCL          },
        { "O_RDONLY",           O_RDONLY        },
        { "O_RDWR",             O_RDWR          },
        { "O_TRUNC",            O_TRUNC         },
        { "O_WRONLY",           O_WRONLY        },
        { "O_TEXT",             O_TEXT          },
        { "O_BINARY",           O_BINARY        },
        { "O_SYNC",             O_SYNC          },
        { "O_NOINHERIT",        O_NOINHERIT     },

        /* file types */
#if !defined(S_IFREG)
#define S_IFREG                 0
#endif
#if !defined(S_IFLNK)
#define S_IFLNK                 0
#endif
#if !defined(S_IFSOCK)
#define S_ISOCK                 0
#endif

        { "S_IFMT",             S_IFMT          },
        { "S_IFDIR",            S_IFDIR         },
        { "S_IFCHR",            S_IFCHR         },
        { "S_IFIFO",            S_IFIFO         },
        { "S_IFREG",            S_IFREG         },
        { "S_IFLNK",            S_IFLNK         },
        { "S_IFSOCK",           S_IFSOCK        },

        /* permissions */
#if !defined(S_ISUID)
#define S_ISUID                 0
#endif
#if !defined(S_ISGID)
#define S_ISGID                 0
#endif
#if !defined(S_ISVTX)
#define S_ISVTX                 0
#endif
        { "S_IRUSR",            S_IRUSR         },
        { "S_IWUSR",            S_IWUSR         },
        { "S_IXUSR",            S_IXUSR         },
        { "S_IRGRP",            S_IRGRP         },
        { "S_IWGRP",            S_IWGRP         },
        { "S_IXGRP",            S_IXGRP         },
        { "S_IROTH",            S_IROTH         },
        { "S_IWOTH",            S_IWOTH         },
        { "S_IXOTH",            S_IXOTH         },

        { "S_ISUID",            S_ISUID         },
        { "S_ISGID",            S_ISGID         },
        { "S_ISVTX",            S_ISVTX         },

        /*access() modes*/
        { "F_OK",               F_OK            },
        { "R_OK",               R_OK            },
        { "W_OK",               W_OK            },
        { "X_OK",               X_OK            },

        /*seek*/
        { "SEEK_SET",           SEEK_SET        },  /* beginning of file */
        { "SEEK_CUR",           SEEK_CUR        },  /* current position */
        { "SEEK_END",           SEEK_END        },  /* end of file */

        /*limits*/
        { "PATH_MAX",           MAX_PATH        },
        { "NAME_MAX",           MAX_NAME        },
        { "INT_MIN",            ACCINT_MIN      },
        { "INT_MAX",            ACCINT_MAX      },
        { "MAXPROMPT",          MAX_CMDLINE-1   },  /* see get_parm() */
        { "RAND_MAX",           RAND_MAX        }   /* see rand() */
        };


static const struct {    /* float constant's */
    const char *        tag;
    accfloat_t          value;
} fconsts[] = {
        { "FLT_MIN",            ACCFLOAT_MIN    },
        { "FLT_MAX",            ACCFLOAT_MAX    },
        };

/*
 *<<GRIEF>> [macro]
    Constant: GRVERSIONMAJOR - GRIEF major version

        extern const int
            GRVERSIONMAJOR;

    Constant Description:
        The 'GRVERSIONMAJOR' global constant is set to the minor
        number of the running GRIEF implementation version.

    Constant See Also:
        GRVERSIONMAJOR, GRVERSIONMINOR

 *<<GRIEF>> [macro]
    Constant: GRVERSIONMINOR - GRIEF minor version.

        extern const int
            GRVERSIONMINOR;

    Constant Description:
        The 'GRVERSIONMINOR' global constant is set to the minor
        number of the running GRIEF implementation version.

    Constant See Also:
        GRVERSIONMAJOR, GRVERSIONMINOR

 *<<GRIEF>> [macro]
    Constant: GRLEVEL - GRIEF Nesting level.

        extern const int
            GRLEVEL;

    Constant Description:
        The 'GRLEVEL' global constant is set by GRIEF to the current
        nesting level, stating the number of current invocations of
        GRIEF within the same session. This value allows the user to
        determine that there are nested invocations of GRIEF running;
        see <inq_brief_level>.

    Constant Compatible:
        GRLEVEL is equivalent to the BRIEF 'BLEVEL' environment
        variable.

        It is a hangover from BRIEF which displays the level number
        in the bottom right corner. Unlike BRIEF the value only
        represents the number of instances running within the current
        terminal session; with each terminal maintaining an
        independent nesting level.

    Constant See Also:
        GRPATH, GRHELP

 */
static const struct {   /* integer values */
    const char *        tag;
    const int *         value;
} ivalues[] = {
        { "GRVERSIONMAJOR",     &x_major_version },
        { "GRVERSIONMINOR",     &x_minor_version },
        { "GRLEVEL",            &x_applevel },
        };


static const struct {   /* dynamic integer values */
    const char *        tag;
    void              (*value)(SYMBOL *);
} idynamic[] = {
        { "current_buffer",     i_currentbuffer },
        { "current_window",     i_currentwindow },
        { "current_line",       i_currentline },
        { "current_col",        i_currentcol },
        };


/*
 *<<GRIEF>> [macro]
    Constant: GRPATH - Macro object search path.

        extern const string
            GRPATH;

    Constant Description:
        The 'GRPATH' global string is used to specify one or more
        directory names stating the search path which GRIEF shall
        utilise to locate macro objects during <autoload> and
        <require> operations. The initial value of GRPATH is either
        imported from the environment or if not available is derived
        from the location of the running application.

        Same as the system 'PATH' environment variable, the
        directories contained within GRPATH should use the operating
        system dependent delimiter generally either a semi-colon (;)
        for DOS/Windows or a colon (:) for Unix style systems.

        Directly setting this value within the session environment
        permits an alternative GRIEF installation.

      Example::

        Possible Unix definition, allowing access to both user
        localised and global macros.

>           $HOME/grief:/usr/local/grief/macro

        The value of GRPATH can be dumped by running GRIEF with the
        '--config' command line option.

>           gr --config

    Constant Compatible:
        GRPATH is equivalent to the BRIEF 'BPATH' environment variable.

    Constant See Also:
        GRPATH, GRHELP, GRBACKUP, GRDICTIONARIES

 *<<GRIEF>> [macro]
    Constant: GRHELP - Help search path.

        extern const string
            GRHELP;

    Constant Description:
        The 'GRHELP' global string is used to specify the directory
        name stating the search path which GRIEF shall utilise to
        locate the help database. The initial value of GRHELP is
        either imported from the environment or if not available is
        derived from the location of the running application.

        Directly setting this value within the session environment
        permits an alternative GRIEF installation.

      Example::

        Default Unix definition.

>           /usr/local/grief/help

        The value of GRHELP can be dumped by running GRIEF with the
        '--config' command line option.

>           gr --config

    Constant Compatible:
        GRHELP is equivalent to the BRIEF 'BHELP' environment variable.

    Constant See Also:
        GRPATH, GRHELP, GRBACKUP, GRDICTIONARIES

 *<<GRIEF>> [macro]
    Constant: GRDICTIONARIES - Dictionary locales

        extern const string
            GRDICTIONARIES;

    Constant Description:
        The 'GRDICTIONARIES' global string is used to specify the
        dictionary locales to be applied during spell check
        operations, stated as a list of comma separated dictionary
        names.

     Example::

>           GRDICTIONARIES=en_GB,en_US

        The default value assigned when no value is available.

>           GRDICTIONARIES=en_US,en_GB,default

        The value of GRDICTIONARIES can be dumped by running GRIEF
        with the '--config' command line option.

>           gr --config

    Constant See Also:
        GRPATH, GRDICTIONARY, GRDICTIONARIES

 *<<GRIEF>> [macro]
    Constant: GRDICTIONARY - Dictionary search path.

        extern const string
            GRDICTIONARY;

    Constant Description:
        The 'GRDICTIONARY' global specifies the directory from which
        dictionaries are to be sourced during spell check operations.

    Constant See Also:
        GRPATH, GRDICTIONARY, GRDICTIONARIES

 *<<GRIEF>> [macro]
    Constant: GRTERM - Terminal override.

        extern const string
            GRTERM;

    Constant Description:
        The 'GRTERM' global string is used to specify the identifier
        for the current terminal type as defined by the UNIX terminfo
        database, similar to the system 'TERM' environment variable.
        GRTERM should be considered as an override to the standard
        system TERM specification; it may be used in preference to
        'TERM' allowing a GRIEF specialisation without affecting
        other console applications.

        The leading component of the GRTERM specification consists of
        the terminal base-name. Following the base-name, you may add
        any reasonable number of hyphen-separated feature suffixes,
        for example:

>           xterm-256color

        Some of the standard features include,

(start table,format=nd)
            [Feature        [Description                    ]

          ! m, mono         Mono terminal.

          ! 256, 256color   Terminal supports 256 colours.

          ! 16, 16color     Terminal supports 16 colours.

          ! 88              Terminal support 88 colours.
(end table)

        During initialisation GRIEF firstly references 'GRTERM' and
        then secondary the system 'TERM' environment variable values.
        The derived terminal base-name causes the associated tty
        macro is be loaded; this macro has the responsible for
        configuring GRIEF's keyboard and screen layout using
        <set_term_feature> and <set_term_keyboard> plus related
        primitives.

    Constant See Also:
        GRTERM, GRTERMCAP

 *<<GRIEF>> [macro]
    Constant: GRTERMCAP - Terminal capability database.

        extern const string
            GRTERMCAP;

    Constant Description:
        The 'GRTERMCAP' global string is used to specify the path to
        the terminal capability database, similar to the system
        'TERMCAP' environment variable. GRTERMCAP should be
        considered as an override to the standard system TERM
        specification; it may be used in preference to 'TERMCAP'
        allowing a GRIEF specialisation without affecting other
        console applications.

    Constant See Also:
        GRTERM, GRTERMCAP

 *<<GRIEF>> [macro]
    Constant: GRTMP - Temporary dictionary.

        extern const string
            GRTMP;

    Constant Description:
        The 'GRTMP' global string is used to specify the directory
        within which temporary files are to be created. GRTMP should
        be considered as an override to the standard system temporary
        directory specification.

        GRIEF firstly references GRTMP; if this is not defined it
        tries 'BTMP' secondary and then system dependent alternatives,
        for example 'TEMP' and 'TEMPDIR'.

        If no temporary is not defined, or if it is set to the name
        of a directory that does not exist, temporary files shall be
        created within a system dependent default.

    Constant Compatible:
        GRTMP is equivalent to the BRIEF 'BTMP' environment variable.

    Constant See Also:
        GRPATH

 *<<GRIEF>> [macro]
    Constant: GRFLAGS - Default command line arguments.

        extern const string
            GRFLAGS;

    Constant Description:
        The 'GRFLAGS' global string is used to specify the default
        implied command line arguments. GRIEF processes the switches
        stated in GRFLAGS after processing the explicit flags on the
        command line.

        The GRFLAGS variable provides a means to set up private user
        options to be automatically applied on each execution. For
        example, your GRIEF profile macro can be automatic loaded on
        start using:

>           GRFLAGS="-mmyprofile"

        The initial value of GRFLAGS is either imported from the
        environment or if not available then a system dependent
        default is used.

    Constant Compatible:
        GRFLAGS is equivalent to the BRIEF 'BFLAGS' environment
        variable.

    Constant See Also:
        GRFILE

 *<<GRIEF>> [macro]
    Constant: GRFILE - Default empty file name.

        extern const string
            GRFILE;

    Constant Description:
        The 'GRFILE' global string is used to specify the file name
        used on start up as the default buffer name to be created,
        when no explicit files are stated on the command line.

        The initial value of GRFILE is either imported from the
        environment or if not available then a value of 'newfile' is
        used.

>           GRFILE=newfile

    Constant See Also:
        GRFLAGS

 *<<GRIEF>> [macro]
    Constant: GRBACKUP - Backup path.

        extern const string
            GRBACKUP;

    Constant Description:
        The 'GRBACKUP' global string is used to specify the default
        backup directory, when buffer backups are enabled. In the
        absence of any buffer or global <set_backup_option> *BK_DIR*
        configuration nor the GRBACKUP environment variable, GRIEF
        renames the original file with a '.bak' extension.

    Constant Compatible:
        GRBACKUP is equivalent to the BRIEF 'BBACKUP' environment
        variable.

    Constant See Also:
        GRBACKUP, GRVERSIONS

 *<<GRIEF>> [macro]
    Constant: GRVERSIONS - Backup versions.

        extern const int
            GRVERSIONS;

    Constant Description:
        The 'GRVERSIONS' global string is used to specify the number
        of backup versions to be maintained for each modified file,
        when buffer backups are enabled. When stated GRIEF shall keep
        multiple backup copies of each edited file. In the absence of
        any buffer or global <set_backup_option> *BK_VERSIONS*
        configuration nor the GRVERSIONS environment variable, GRIEF
        simple keeps the previous image.

        When enabled, GRIEF creates a set of *n* directories given by
        GRVERSION named '1' through to 'n'; the directory name being
        number of previous edit sessions the files contained within
        represent.

        For example, given *GRBACKUP=~/.backups* and *GRVERSIONS=3*
        the following directory structure shall be maintained.

>           ~/.backups
>           ~/.backups/1
>           ~/.backups/2
>           ~/.backups/3

        During backup operations, starting from 'n-1' in reverse
        order the previous images are rolled into the higher
        sequenced directory; with any images within the last
        directory being removed. Finally the most recent is saved
        into the root of the tree. On completion the oldest version
        of a file can be found in the 'nth' directory; the next
        oldest is in 'n - 1'. The most recent backup is the file in
        the backup directory.

        Given the edited file 'myfile.txt' against the above
        configuration the following backup images may exist:

>           ~/.backups/myfile.txt       [latest]
>           ~/.backups/1/myfile.txt
>           ~/.backups/2/myfile.txt
>           ~/.backups/3/myfile.txt     [oldest]

    Constant See Also:
        GRBACKUP, GRVERSIONS

 *<<GRIEF>> [macro]
    Constant: GRPROFILE - Profile directory override.

        extern const string
            GRPROFILE;

    Constant Description:
        The 'GRPROFILE' global string is used to specify an override
        to the standard users home directory; it is utilised by
        several macros to source runtime configuration details.
        GRPROFILE provides the user a means of stating an alternative
        location.

    Constant See Also:
        GRPATH

 *<<GRIEF>> [macro]
    Constant: BPACKAGES - BRIEF packages default.

        extern const string
            BPACKAGES;

    Constant Description:
        The 'BPACKAGES' global string is used to specify the BRIEF
        language sensitive editing modes. This is a hangover from the
        PC version of BRIEF, it is used to configure old-style
        package support; see the macro source 'language.cr'.

        BPACKAGES shall be referenced on the first running of GRIEF
        and then exported into the GRIEF runtime configuration file.
        The initial value of BPACKAGES is either imported from the
        environment or if not available then the following default is
        used.

>       .c.cc.CC.cpp.h.H.hpp-c:hilite,t;.default:hilite,template,regular;

    Package Specification:
        You can attach these packages so that they are automatically
        used with program source files that have special file
        extensions. The packages configuration is used by all file
        extension dependent packages.

        The format of the 'package' specification is,

>       package: extension[,extension]...[-equivalent]:\
>                   package [args],package...;extension...

    extension::
        The *extension* element is the file extension that will
        invoke a specific style. One or more comma separated
        extension can be specified for each style.

        Note: the special extension "default" is used to specify all
        extensions not specifically included in the packages string.

    equivalent::
        The *equivalent* element is an optional value which specifies
        that the preceding extensions should be treated the same as
        the "equivalent extension" by the language sensitive features,
        an extension override.

        All extensions preceding the equivalent extension back to
        preceding semicolon, another equivalent extension, or the
        beginning of the packages definition are affected by the
        command. Hence given:

>           package: .asm:.r;.cpp.hpp.h-c:smart

        GRIEF shall consider extensions '.cpp', '.hpp' and '.h'
        equivalent to the extension '.c', and would use .c the smart
        editing package for all three. Note also that since '.c' has
        not been explicitly specified as an extension, no packages
        are assigned to it. The following is required to complete the
        recipe:

>           package: .asm:r;.cpp.hpp.h.c-c:smart

        GRIEF shall first check for the package using the actual
        extension before for one with the equivalent extension. If in
        the previous example, 'smart' indentation was available for
        '.cpp' files, GRIEF would use the '.cpp' support over the
        '.c' support.

        This feature is useful when the extension is not
        automatically recognised and is unable to provide 'smart'
        indenting. Using extension equivalence, informs GRIEF that an
        unsupported extension should be treated like an equivalent
        supported one.

>           package: .default:hilite;.c:hilite,t 1 1 0 1

        The extension '.default' is used as a wild card (or default),
        to any match an extension not explicitly listed. The above
        example enables the 'hilite' package on implicitly supported
        file types, plus an explicit configuration for the '.c'
        extension.

    package::
        Is the macro package attached to the extension. Each
        extension can have multiple packages associated with it, as
        long as the package don't conflict.

    Constant See Also:
        GRTEMPLATE

 *<<GRIEF>> [macro]
    Constant: GRTEMPLATE - Source template search path.

        extern const string
            GRTEMPLATE;

    Constant Description:
        The 'GRTEMPLATE' global string is used to specify the
        language sensitive file and function header generator. The
        initial value of GRTEMPLATE is either imported from the
        environment or if not available is derived from the location
        of the running application.

        Template syntax examples; see the macro source macro
        'funchead.cr' for details.

>       Synopsis:
>           %[1%FTYPE% %FNAME%%FOPEN% %FARGS% %FCLOSE%      // single arg]%
>
>           %[2%FTYPE% %FNAME%%FOPEN% %FARGS(13)% %FCLOSE%  // multi arg]%
>
>           %FTYPE% %FNAME%
>               %FOPEN%
>                   %FARGS(,72)%
>               %FCLOSE%
>
>       Purpose:
>            %FDESC%
>
>       Parameters:
>           FINPUTS(9)
>           %FINPUTS(9)%
>
>           FINPUTS(0,+10)
>                   %FINPUTS(0,+10)%
>
>           FINPUTS(0,+10,-2)
>                   %FINPUTS(0,+10,-2)%
>
>           FINPUTS(0,32,,80,1,,1)
>                   %FINPUTS(0,32,,80,1,,1)%
>
>           FINPUTS(0,32,-4,60,1,1)
>                   %FINPUTS(0,32,-4,60,1,1)%
>
>           FINPUTS(0,-1)
>                   %FINPUTS(0,-1)%
>
>           FINPUTS(0,20,,70,1,1,1)
>                   %FINPUTS(0,20,,70,1,1,1)%
>
>       Returns:
>           %FTYPE% -

    Constant See Also:
        BPACKAGES

 *<<GRIEF>> [macro]
    Constant: GRKBDPATH - Keyboard library search path.

        extern const string
            GRKBDPATH;

    Constant Description:
        The 'GRKBDPATH' global string is used to specify the
        directory name stating the location shall utilise to keyboard
        library definitions. The initial value of GRKBDPATH is either
        imported from the environment or if not available is derived
        from 'GRTEMPLATE'.

        These macros build the global list variable, 'kbd_labels',
        that is then is used by the <int_to_key> to derive the
        displayable label.

        The keyboard library macro is only relevant within several
        help functions, so that meaningful keyboard labels can be
        given. For example, you might have a META key rather than an
        ALT key.

>           list
>           kbd_labels = {
>               AF(1),          "<Meta-F1>",
>               AF(2),          "<Meta-F2>",
>               AF(3),          "<Meta-F3>",
>               AF(4),          "<Meta-F4>",
>               AF(5),          "<Meta-F5>",
>               AF(6),          "<Meta-F6>",
>               AF(7),          "<Meta-F7>",
>               AF(8),          "<Meta-F8>",
>               AF(9),          "<Meta-F9>",
>               AF(10),         "<Meta-F10>",
>               AF(11),         "<Meta-F11>",
>               AF(12),         "<Meta-F12>"
>           };

    Constant See Also:
        GRTEMPLATE

 */
static const struct {   /* dynamic string values */
    const char *        tag;
    void              (*value)(SYMBOL *);
} sdynamic[] = {
        { "GRPATH",             s_getenv },
        { "GRHELP",             s_getenv },
        { "GRFLAGS",            s_getenv },
        { "GRFILE",             s_getenv },
        { "GRTERM",             s_getenv },
        { "GRTERMCAP",          s_getenv },
    //  { "GRTERM_LOCALE",      s_getenv },     // XTERM_LOCALE override
    //  { "GRTEXT_LANG",        s_getenv },     // Text locale override.
        { "GRTMP",              s_getenv },
        { "GRBACKUP",           s_getenv },
        { "GRVERSIONS",         s_getenv },
        { "GRPROFILE",          s_getenv },
        { "GRDICTIONARIES",     s_getenv },
        { "GRDICTIONARY",       s_getenv },
    //  { "GRPERSONAL",         s_getenv },     // "${GRPERSONAL}/.grspell" or "${HOME}/.grspell"
    //  { "GRUNDO",             s_getenv },     // "${GRUNDO}/grunXXXXXXXX" or "${TMP}/grunXXXXXXXX"
    //  { "GRIPC",              s_getenv },
    //  { "BPACKAGES",          s_getenv },
        { "GRKBDLIB",           s_getenv },
        { "GRTEMPLATE",         s_getenv }
        };


/*  Function:           sym_global
 *      Create and initialise the well-known global symbols.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 */
void
sym_globals(void)
{
    SYMBOL *sp;
    unsigned i;

    /*strings*/
    for (i = 0; i < (sizeof(svalues)/sizeof(svalues[0])); ++i) {
        sp = sym_push(1, svalues[i].tag, F_STR, SF_CONSTANT|SF_SYSTEM);
        sym_assign_str(sp, *svalues[i].value);
    }

    for (i = 0; i < (sizeof(sconsts)/sizeof(sconsts[0])); ++i) {
        sp = sym_push(1, sconsts[i].tag, F_STR, SF_CONSTANT|SF_SYSTEM);
        sym_assign_str(sp, sconsts[i].value);
    }

    /*integer*/
    for (i = 0; i < (sizeof(ivalues)/sizeof(ivalues[0])); ++i) {
        sp = sym_push(1, ivalues[i].tag, F_INT, SF_CONSTANT|SF_SYSTEM);
        sym_assign_int(sp, *ivalues[i].value);
    }

    for (i = 0; i < (sizeof(iconsts)/sizeof(iconsts[0])); ++i) {
        sp = sym_push(1, iconsts[i].tag, F_INT, SF_CONSTANT|SF_SYSTEM);
        sym_assign_int(sp, iconsts[i].value);
    }

    for (i = 0; i < (sizeof(fconsts)/sizeof(fconsts[0])); ++i) {
        sp = sym_push(1, fconsts[i].tag, F_FLOAT, SF_CONSTANT|SF_SYSTEM);
        sym_assign_float(sp, fconsts[i].value);
    }

    for (i = 0; i < (sizeof(idynamic)/sizeof(idynamic[0])); ++i) {
        sp = sym_push(1, idynamic[i].tag, F_INT, SF_CONSTANT|SF_DYNAMIC|SF_SYSTEM);
        sp->s_dynamic = idynamic[i].value;
    }

    for (i = 0; i < (sizeof(sdynamic)/sizeof(sdynamic[0])); ++i) {
        sp = sym_push(1, sdynamic[i].tag, F_STR, SF_CONSTANT|SF_DYNAMIC|SF_SYSTEM);
        sp->s_dynamic = sdynamic[i].value;
    }
}


/*  Function:           i_currentline
 *      Callback during "currentline" dereferences.
 *
 *  Parameters:
 *      sp - Symbol object.
 *
 *  Returns:
 *      nothing.
 */
static void
i_currentline(SYMBOL *sp)
{
    sp->s_int = *cur_line;
}


/*  Function:           i_currentcol
 *      Callback during "currentcol" dereferences.
 *
 *  Parameters:
 *      sp - Symbol object.
 *
 *  Returns:
 *      nothing.
 */
static void
i_currentcol(SYMBOL *sp)
{
    sp->s_int = *cur_col;
}


/*  Function:           i_currentbuffer
 *      Callback during "currentbuffer" dereferences.
 *
 *  Parameters:
 *      sp - Symbol object.
 *
 *  Returns:
 *      nothing.
 */
static void
i_currentbuffer(SYMBOL *sp)
{
    sp->s_int = (curbp ? curbp->b_bufnum : -1);
}


/*  Function:           i_currentwindow
 *      Callback during "currentwindow" dereferences.
 *
 *  Parameters:
 *      sp - Symbol object.
 *
 *  Returns:
 *      nothing.
 */
static void
i_currentwindow(SYMBOL *sp)
{
    sp->s_int = (curwp ? curwp->w_num : -1);
}


/*  Function:           s_getenv
 *      Callback during "environment variable" dereferences.
 *
 *  Parameters:
 *      sp - Symbol object.
 *
 *  Returns:
 *      nothing.
 */
static void
s_getenv(SYMBOL *sp)
{
    const char *env = ggetenv(sp->s_name);
    sym_assign_str(sp, env ? env : "");
}


/*  Function:           do_extern
 *      extern primitive.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 *
 *<<GRIEF>>
    Macro: extern - Declare an external variable.

        extern <type> sym1, sym2, ..;

    Macro Description:
        The 'extern' storage class specifier extends the visibility
        of variables and functions, allowing objects and functions to
        be accessed across several source files.

        An 'extern' variable, function definition, or declaration
        makes the described variable or function usable by the
        succeeding part of the current source file.

        This declaration does not replace the definition. The
        declaration is used to describe the variable that is
        externally defined. Essentially the extern keyword creates a
        place holder in the symbol table to avoid undefined symbol
        reference errors.

        Note!:
        You should note that we are using the words definition and
        declaration carefully when we refer to external variables in
        this section. Definition refers to the place where the
        variable is created or assigned storage; declaration refers
        to places where the nature of the variable is stated but no
        storage is allocated.

(end!)

    Variables::

        A variable must be defined once in one of the modules of the
        program; this sets aside storage. If there is no definition,
        a runtime error shall result since the storage of the
        variable would not have been created,

        There may be none or more variable declarations. These can be
        declared 'extern' in many modules, including the module where
        it was defined, and even many times in the same module. All
        the declarations must match, which is normally by the use of
        a common header file shared between all source files.

        Any 'extern' declaration of the same identifier found within
        a block refers to that same object. If no other declaration
        for the identifier exists at file scope, the identifier has
        external linkage.

        Unlike C external linkage of variables occurs during macro
        execution, see <Scope Rules>. If a declaration for an
        identifier already exists in one of the visible namespaces
        they reference to the same image.

        When searching for a variable definition, GRIEF searches the
        symbol tables in the following order:

            o static variable definition in the current function.

            o buffer local variable.

            o local variables of a current block.

            o nested stack frames to the outermost function
                call 'dynamic scope'.

            o global variable.

        Within the following example the variable 'x' reference by
        the function 'foo' is resolved against the global image of
        'x', whereas 'b' shall be resolved against the image within
        the caller 'bar'.

(start code)
        static int x = 0;

        void
        foo()
        {
            extern int b, x;

            b = 0;              // resolved by 'i' within bar()
            x = 0;              // global 'x'
        }

        void
        bar()
        {
            int b;
            foo();
        }
(end)

    Functions::

        The 'extern' statement applied to a function prototype does
        nothing; as 'extern' is the default linkage. A function
        prototype is always a declaration and never a definition.

        All functions across loaded macros which refer to the same
        external identifier refer to the same object, so care must be
        taken that the type and extent specified in the definition
        are compatible with those specified by each function which
        references the data. This is generally achieved by the use of
        a common header file shared between all source files (e.g.
        "grief.h").

        It is an error to include a declaration for the same function
        with the storage class specifier static before the
        declaration with no storage class specifier because of the
        incompatible declarations. Including the 'extern' storage
        class specifier on the original declaration is valid and the
        function has internal linkage.

        Builtin macros do not have explicit prototypes as the Macro
        compiler has internal knowledge of all visible primitives.

    Macro Returns:
        nothing

    Macro Portability:
        n/a

    Macro See Also:
        int, string, list, float, declare, static, global
 */
void
do_extern(void)                 /* (name ...) */
{
    /*NOTHING: placeholder only, all validations are performed with the Macro Compiler*/
}


/*  Function:           do_make_local_variable
 *      make_local_variable primitive.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 *
 *<<GRIEF>>
    Macro: make_local_variable - Make a buffer local variable.

        void
        make_local_variable(declare &sym, ...)

    Macro Description:
        The 'make_local_variable()' primitive associates the
        specified variable with the current buffer, becoming a buffer
        variable..

        Unlike local variables are destroyed when the block within
        which they are defines terminates, buffer variables maintain
        their value across macro invocation and occupy permanent
        storage until the buffer is deleted (see scope).

    Macro Parameters:
        sym - Symbol reference.
        ... - Optional additional symbol references.

    Macro Returns:
        The 'make_local_variable()' primitive returns nothing directly.

        On error conditions the following diagnostics message shall
        be echoed on the command prompt, with 'xxx' representing the
        symbol name.

>           missing symbol.

>           'xxx' not found at current scope.

>           cannot promote reference 'xxx'.

>           system variable 'xxx'.

    Macro Portability:
        n/a

    Macro See Also:
        inq_symbol
 */
void
do_make_local_variable(void)    /* void (name ...) */
{
    sym_move(curbp->b_syms, "make_local_variable");
}


/*  Function:           do_global
 *      global primitive
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 *
 *<<GRIEF>>
    Macro: global - Declare a global variable.

        global sym1, sym2, ..;

    Macro Description:
        The 'global()' primitive promotes a local declaration and
        making symbol global to all macros.

        Global variables maintain their value across macro invocation
        and occupy permanent storage (See Scope), whereas local
        variables are destroyed upon the macro they are contained
        within is terminated.

        A variable must have been specified in a previous 'int',
        'string', 'list', 'float' or 'declare' statement before it
        can be made into a global.

        The 'global' is a managed primitive and shall be automaticlly
        invoked on global variable declarations as follows:

(start code)
        // global declarations

        int global_int1 = 1234;
        static int global_int2;
        const static int global_int3 = 125;

        string global_string2 = "Hello world";
        static string global_string2;

        float global_float1 = "Hello world";
        static float global_float2;

        list global_list1;
        static list global_list2;

        void
        mymacro()
        {
        }
(end code)

        Each macro object containing global declarations shall
        contain an internal '_init' macro, which is utilised by the
        Macro Compiler to initialise file level variables.

    Macro Note!:
        Both 'global' and '_init' are considered as internal
        primitives, reserved for exclusive use by the GRIEF Macro
        Compiler and may change without notice.

        Management of variable scoping and argument binding shall be
        handling automatically by the compiler.

    Macro Returns:
        nothing

    Macro Portability:
        n/a

    Macro See Also:
        Types, global, extern, static, const
 */
void
do_global(void)                 /* (name ...) */
{
    sym_move(x_gsym_tbl, "global");
}


/*  Function:           do_static
 *      static primitive.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 *
 *<<GRIEF>>
    Macro: static - Define a function or module scope.

        static var1, var2, ..;

    Macro Description:
        The 'static' statement can be used to change the storage
        scope of a variable or function. It is one of the major
        mechanism to enforce information hiding. static denotes that
        a function or data element is only known within the scope of
        the current module. This provides a form of object-hiding and
        can avoid name clashes with other macros (See Scope).

        In addition, if you use the static statement with a variable
        that is local to a function, it allows the last value of the
        variable to be preserved between successive calls to that
        function, including during recursions.

    Variables::

        A static variable is initialized only once. Globals are
        performed within the body of the '_init' function, whereas a
        function static variable that has an initializer is
        initialized the first time it comes into existence.

    Function::*

        A static function is hidden from usage outside their own
        macro file (or module), this can present a problem with
        functionality which involves the usage of callbacks (e.g.
        assign_to_key). In this case, the :: (scope resolution)
        operator is used to qualify hidden names so that they can
        still be used.

    Macro Example:

(start code)
        void
        main()
        {
            assign_to_key("<Alt-A>", "my_alt_a");
        }

        static void
        my_alt_a()
        {
            //function body
        }
(end code)

        The static declaration of my_alt_a() referenced by the
        assign_to_key() within main() wont be visible when the
        "Alt-A" key is processed as it shall be out of scope. The
        usage of "::my_alt_a" forces the my_alt_a reference to become
        fully qualified at the time of the key assignment. The
        following examples have the equivalent result:

        Implicit current module, where if a null module name is
        specified (e.g. "::function") then the symbol shall be bound
        to current module.

>           assign_to_key("<Alt-A>", "::my_alt_a");

        or, explicit current module, where a named namespaces is
        specified (e.g. "module::function"):

>           module("my_module"");
>           assign_to_key("<Alt-A>", "my_module::my_alt_a");

        or

>           module("my_module"");
>           assign_to_key("<Alt-A>", inq_module() + "::my_alt_a");

    Macro Returns:
        nothing

    Macro Portability:
        Module static declarations are a GRIEF extension.

    Macro See Also:
        Types, global, extern, static, const
 */
void
do_static(void)                 /* (name ...) */
{
    const char *function;
    SPTREE *mp;

    /*
     *  Pull arguments, unless from command line then just exit
     */
    if (ms_cnt <= 0) {
        return;
    }
    function = mac_stack[ms_cnt - 1].name;
    mp = macro_symbols(mac_stack[ms_cnt - 1].module);

    /*
     *  If within _init, module global
     */
    if (0 == strcmp(function, "_init")) {
        sym_move(mp, "static");                 /* move to module */

    /*
     *  else, function static
     */
    } else {
        char symname[SYM_FUNCNAME_LEN + 2];
        SPBLK *sp;

        sxprintf(symname, sizeof(symname), "$%s", function);
        if ((sp = splookup(symname, mp)) == NULL) {
            /*
             *  build new node
             */
            const size_t len = strlen(symname) + 1;

            sp = spblk(len);
            sp->key = sp->data;
            memcpy(sp->key, (const char *)symname, len);
            sp->data = spinit();
            spenq(sp, mp);
        }
        sym_move((SPTREE *)sp->data, "static");
    }
}


/*  Function:           do_const
 *      const primitive.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 *
 *<<GRIEF>>
    Macro: const - Define a variable as being constant.

        const <type> sym1, sym2 ...;

    Macro Description:
        The 'const' qualifier explicitly declares a data object as
        something that cannot be changed. Its value can only be set
        during initialization. You cannot use const data objects in
        expressions requiring a modifiable lvalue. For example, a
        const data object cannot appear on the lefthand side of an
        assignment statement.

    Macro Returns:
        nothing

    Macro Portability:
        An experimental GRIEF extension; functionality may change.

    Macro See Also:
        Types, global, extern, static, const
 */
void
do_const(void)                  /* (name ...) */
{
    const LIST *nextlp, *lp = get_xlist(1);

    if (NULL == lp || ms_cnt <= 0) {
        return;
    }

    for (;(nextlp = atom_next(lp)) != lp; lp = nextlp) {
        accint_t state = 1;                     /* default, enable */
        const char *symname;
        SYMBOL *lsp;

        /*
         *  retrieve "[state] symbol-name" defines.
         */
        if (atom_xint(lp, &state)) {
            lp = nextlp;
            if ((nextlp = atom_next(lp)) == lp) {
                break;
            }
        }
        if (NULL == (symname = atom_xstr(lp))) {
            continue;
        }

        /*
         *  locate the symbol and make constant
         */
        if (NULL == (lsp = sym_local_lookup(symname))) {
            ewprintf("const: '%s' not found at current scope.", symname);
            continue;
        }

        if (state) {
            SYMSET(lsp, SF_CONSTANT);
        } else {
            if (SYMTST(lsp, SF_SYSTEM) && SYMTST(lsp, SF_CONSTANT)) {
                ewprintf("const: system variable '%s'", symname);
            } else {
                SYMCLR(lsp, SF_CONSTANT);
            }
        }
    }
}


/*  Function:           do_declare
 *      declare, int, string, float and list primitives.
 *
 *  Parameters:
 *      flag - Base type.
 *
 *  Returns:
 *      nothing.
 *
 *<<GRIEF>>
    Macro: declare - Declare a polymorphic symbol

        declare sym1, sym2 ...;

    Macro Description:
        The 'declare' statement declares a polymorphic data type.

    Macro Returns:
        nothing

    Macro Portability:
        n/a

    Macro See Also:
        Types, int, string, list, float, double, array, declare

 *<<GRIEF>>
    Macro: int - Declare an integer symbol

        int sym1, sym2 ...;

    Macro Description:
        The 'int' statement declares an integral type that stores
        values in the range;

>           -2,147,483,648 to 2,147,483,647

        You can declare and initialize a variable of the type int
        like this example:

>           int i = 1234;

    Macro Returns:
        nothing

    Macro Portability:
        n/a

    Macro See Also:
        Types, int, string, list, float, double, array

 *<<GRIEF>> [var]
     Macro: float - Declare a float symbol

        float sym1, sym2 ...;

    Macro Description:
        The 'float' statement declares a simple type that stores
        64-bit floating-point values that stores values in the
        approximate range;

>           1.7E308 to 1.7E+308

        Note!:
        Unlike C and C++, both float and double are internally
        representing using a 64-bit double precision floating-point

    Macro Returns:
        nothing

    Macro Portability:
        n/a

    Macro See Also:
        Types, int, string, list, float, double, array

 *<<GRIEF>>
    Macro: double - Declare a double float symbol

        double sym1, sym2 ...;

    Macro Description:
        The 'double' statement is an alias for the <float> type.

        Note!:
        Unlike C and C++, both float and double are internally
        representing using a 64-bit double precision floating-point

    Macro Returns:
        nothing

    Macro Portability:
        n/a

    Macro See Also:
        Types, int, string, list, float, double, array

 *<<GRIEF>>
    Macro: string - Declare a string symbol

        string sym1, sym2 ...;

    Macro Description:
        The 'string' statement declares a containers which may
        contain zero or more characters.

    Macro Returns:
        nothing

    Macro Portability:
        n/a

    Macro See Also:
        Types, int, string, list, float, double, array

 *<<GRIEF>>
    Macro: list - Declare a list symbol

        list sym1, sym2 ...;

    Macro Description:
        The 'list' statement declares a list being a container of
        atoms. More precisely, a list is either an empty container
        NULL, or a sequential list of values.

        A list can store any other data type, including lists,
        allowing the creation of complex types. A list may generally
        only be extended at its end, by appending data; elements can
        be updated using <put_nth> and replaced using <splice>.

        Any element may be referenced by specifying its ordinal
        position in the list using <get_th> and array subscripts.
        Lists are not the most efficient data types, since subscript
        element access involves iterating from the head until the
        associated value is referenced, whereas <list_each> primitive
        allows effective iteration.

    Macro Returns:
        nothing

    Macro Portability:
        n/a

    Macro See Also:
        Types, int, string, list, float, double, array

 *<<GRIEF>> [var]
    Macro: array - Declare a array symbol

        array sym1, sym2 ...;

    Macro Description:
        The 'array' statement is reserved for future use.

        An array is a variable array of fixed sized arbitrary
        values. Unlike a list, a array supports constant-time
        element access and updates.

    Macro Returns:
        nothing

    Macro Portability:
        n/a

    Macro See Also:
        Types, int, string, list, float, double, array

 *<<GRIEF>> [var]
    Macro: bool - Declare a boolean symbol

        bool sym1, sym2 ...;

    Macro Description:
        The 'bool' statement as an alias for <int>.

    Macro Returns:
        nothing

    Macro Portability:
        n/a

    Macro See Also:
        Types, int, string, list, float, double, array
 */
void
do_declare(int flag)            /* (type name ...) */
{
    const LIST *nextlp, *lp = get_xlist(1);

    if (NULL == lp || ms_cnt <= 0) {
        return;
    }

    while ((nextlp = atom_next(lp)) != lp) {
        const char *symname = atom_xstr(lp);
        SYMBOL *lsp;

        lp = nextlp;
        if (NULL == symname) {
            ewprintf("declare: illegal variable name");
            continue;
        }

        /* create or recreate */
        if ((lsp = sym_local_lookup(symname)) != NULL) {
            if (SYMTST(lsp, SF_SYSTEM)) {
                ewprintf("declare: system variable '%s'", symname);
                continue;
            }
            sym_destroy(lsp);

        } else {
            SPBLK *spb = (SPBLK *) spblk(sizeof(SYMBOL));

            lsp = (SYMBOL *)spb->data;
            if (strlen(symname) >= sizeof(lsp->s_name)) {
                ewprintf("declare: variable name '%s' truncated", symname);
            }
            strxcpy(lsp->s_name, symname, sizeof(lsp->s_name));
            spb->key = lsp->s_name;
            spenq(spb, x_lsym_tbl[x_nest_level]);
            lsp->s_flags = 0;
            lsp->s_obj = NULL;
        }

        /* assign type */
        if (F_POLY == flag) {
            sym_create(lsp, NULL, F_INT);       /* defaults to 'int' */
            lsp->s_flags = SF_POLY;
        } else {
            sym_create(lsp, NULL, flag);
        }
    }
}


/*  Function:           do_inq_symbol
 *      inq_symbol primitive.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 *
 *<<GRIEF>>
    Macro: inq_symbol - Determine if the symbol exists.

        int
        inq_symbol(string symbol)

    Macro Description:
        The 'inq_symbol()' primitive determines whether the variable
        'Iname' exists at the current scope (See Scope). One main use
        of this primitive is to determine if a specific local buffer
        symbol has been defined.

    Macro Parameters:
        symbol - Name of the symbol.

    Macro Example:

        An example usage:

>       // required to resolve symbol at compile time
>       extern string my_buffer_var;
>
>       string
>       _set_buffer_var(string val)
>       {
>           if (inq_symbol("my_buffer_var")) {
>               my_buffer_var = mode;       // buffer-scope
>           } else
>           {   // otherwise we must create
>               string my_buffer_var = mode;
>               make_local_variable( my_buffer_var );
>           }
>       }
>
>       string
>       _get_buffer_var()
>       {
>           if (inq_symbol("my_buffer_var")) {
>               return my_buffer_var;
>           }
>           return "";
>       }

    Macro Returns:
        The 'inq_symbol()' primitive returns a positive value indicates
        that the symbol exists, otherwise 0 if not found within the
        current scope.

    Macro Portability:
        A Grief extension.

    Macro See Also:
        make_local_variable
 */
void
inq_symbol(void)                /* int (string symbol) */
{
    const char *sym = get_str(1);
    SYMBOL *sp;

    if (NULL != (sp = sym_lookup(sym))) {
        acc_assign_int(1);
    } else {
        acc_assign_int(0);
    }
}


/*  Function:           do_get_parm
 *      get_parm primitive.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 *
 *<<GRIEF>>
    Macro: get_parm - Retrieve the value of a macro parameter.

        int
        get_parm([int argument], declare &symbol,
                [string prompt], [int length = MAXPROMPT],
                    [declare default], [int one = FALSE])

    Macro Description:
        The 'get_parm()' primitive shall retrieve the value of the
        specified macro parameter 'argument' optionally prompting the
        user if the referenced parameter was not given during the
        macro execution. When prompted the question within 'prompt'
        is presented using the optional 'default' which the user can
        then edit.

        This function can also be used to always prompt the user for
        a reply by invoking with the argument parameter being omitted
        and specified as *NULL*.

        Generally the user must complete the input using an 'enter'
        unless the prompt is in 'single character mode', whereby the
        first key is taken as the input. The mode is either implied
        by the 'length' parameter or an explicit 'one' parameter, see
        below.

    Navigation/actions Keys:

        The following keys bindings are active during parameter
        prompts.

(start table,format=nd)
        [Key                        [Function                                           ]

     !  Right, Left                 Move cursor the back/forward one character.

     !  Ctrl+Right, Ctrl+Left       Move cursor the start/end of the current word.

     !  Home, End                   Move to first/last character within the edit field.

     !  Alt+I, Ctrl-O               Toggle insert/overstrike mode.

     !  DEL                         Delete character under the cursor.

     !  Backspace, Ctrl+H           Delete character prior to the cursor.

     !  Alt+K                       Delete from cursor to the end of line.

     !  Insert                      Paste from scape.

     !  Backspace, Ctrl+H           Delete character prior to the cursor.

     !  ESC                         Abort current edit, restoring original content.

     !  Alt-D                       Delete current line.

     !  Alt-K                       Delete from cursor to the end of line.

     !  Alt+Q, Ctrl+Q               Quote next character.

     !  Enter                       Process change.

     !  ALT+H                       Help.

     !  Ctrl+A (*)                  Move cursor to beginning of line.

     !  Ctrl+D (*)                  Delete character under cursor.

     !  Ctrl+K (*)                  Delete from cursor to the end of line.

     !  Ctrl+X, Ctrl+U (*)          Delete current line.

     !  Ctrl+V, Ctrl+Y (*)          Paste from clipboard.
(end table)

        (*) Emacs style key mappings.

    Note!:
        Arguments passed to macros are passed as call by name, that
        is every time a 'get_parm' is issued against a particular
        parameter, that parameter is re-evaluated.

        This lazy evaluation has a number of implications.

            o The order of parameter evaluation is dependent on the
               'get_parm' execution order within the called macro,
               not the arguments position.

            o Each parameter may be evaluated several times.

            o Parameters may never be evaluated, which is again
               dependent on the logic placed around get_parm usage.

        This feature can be very useful sometimes, and at other times
        it can cause anomalous side-effects (see Lazy Evaluation).

    Macro Parameters:
        argument - An integer stating the associated macro
            argument index to be retrieved, starting at an offset of
            zero for the first parameter.

        symbol - Specifies the symbol reference into which the
            resulting parameter shall be stored.

        prompt - Option string which specifies the prompt which
            is represented to the user. If the prompt is omitted, the
            user is not prompted.

        length - Optional integer parameter that specifies the
            upper limit of the string length that is to be retrieved.
            When given as '1' then single character mode is implied
            unless overriden using the 'one' parameter. If omitted
            the upper length is assumed to be *MAXPROMPT*.

        default - An optional value, whos type should match the type of
            'symbol', if specified contains the value which shall
            initially be placed on the command line if a prompt is
            required.

        one - Optional integer flag, if specified as non-zero than the
            user shall be prompted for a single character. Generally
            the user must complete the input using an 'enter',
            whereas in single character mode the first key is taken
            as the input. In addition, single character mode disables
            the execution of the <_prompt_begin> and <_prompt_end>
            callbacks plus any associated prompt history.

            If omitted the character mode shall be implied from the
            specified 'length'; a length of 1 being 'true' otherwise
            'false'.

    Macro Returns:
        The 'get_parm()' primitive returns greater than zero on
        success, otherwise zero if either the user aborted or an
        error was encountered.

(start table,format=nd)
        [Return     [Description                                    ]
      ! 0           Abort, invalid argument or conversion error.
      ! 1           Success.
      ! 2           Default was assigned (extension).
(end table)

    Macro Portability:
        Unlike BRIEF the default parameter is always the fifth,
        whereas with BRIEF the default value is either the fourth or
        fifth dependent on whether an integer 'length' is stated
        since the 'default' was only permitted to be a string.

        The 'one' option is a Grief extension.

    Macro See Also:
        inq_prompt, _prompt_begin, _prompt_end
 */
void
do_get_parm(void)               /* (int argument, lval symbol, [string prompt],
                                        [int length = MAXPROMPT], [declare default], [int one = FALSE]) */
{
    char reply[MAX_CMDLINE];
#define MAXPROMPT        (MAX_CMDLINE-1)        /* reply max, minus NUL */
    SYMBOL *sp = get_symbol(2);
    int one = 0, length = 0;
    const LIST *lp;
    int argi, ret;
    const char *def;
    char numbuf[32];
    LISTV lv;

    acc_assign_int(0);                          /* failure */
    if (ms_cnt <= 0) {
        return;
    }

    if ((length = get_xinteger(4, -1)) < 0 || length > MAXPROMPT) {
        length = MAXPROMPT;                     /* undef or out-of-range */
    } else if (length <= 1) {
        one = length = 1;                       /* length (<= 1)  implies 'one' mode */
    }

    one = get_xinteger(6, one);                 /* extension */

    if (F_STR == sp->s_type) {
        def = get_xstr(5);                      /* 29/07/09, optional parameter */
    } else {
        sprintf(numbuf, "%d", (int)get_xinteger(5, 0)); /*ACCINT*/
        def = numbuf;
    }

    if (isa_undef(1)) {                         /* argument number */
        /*
         *  Prompt ?
         */
reprompt:;
        if (isa_undef(3)) {
            if (!isa_undef(5)) {                /* 20/10/08, assign default */
                if (0 == (ret = com_equ(MOP_NOOP, sp, margv + 5))) {
                    acc_assign_int(2);          /* default assigned */
                }
            }
            return;
        }

        /*
         *  Command line prompt, using default if any
         */
        ret = 1;                                /* success */
        if (egetparm(get_str(3), def, reply, length + 1, one) != TRUE) {
            ret = 0;
        } else {
            if (F_STR == sp->s_type) {
                sym_assign_str(sp, reply);
            } else {
                assert(F_INT == sp->s_type);
                if (1 != sscanf(reply, "%" ACCINT_FMT, &sp->s_int)) {
                    sp->s_int = 0;
                    ret = 0;
                }
            }
        }
        acc_assign_int(ret);
        return;
    }

    if ((argi = get_xinteger(1, -1)) < 0) {
        ewprintf("get_parm: argument index '%d' out of range", argi);
        return;
    }

    if (NULL == (lp = atom_nth(mac_stack[ms_cnt - 1].argv, argi))) {
        goto reprompt;
    }

    /*
     *  Remove local frame and reinitialise,
     *      this is required in the event eval() invokes a function.
     */
    {   struct mac_stack lmacstack;
        SPTREE *lsym;

        assert(x_nest_level >= 2);

        lsym = x_lsym_tbl[x_nest_level];
        x_lsym_tbl[x_nest_level--] = spinit();
        lmacstack = mac_stack[--ms_cnt];

        ret = eval(lp, &lv);                   /* eval the parameter */

        mac_stack[ms_cnt++] = lmacstack;
        spfree(x_lsym_tbl[++x_nest_level]);
        x_lsym_tbl[x_nest_level] = lsym;
    }

    /*
     *  Process return
     */
    if (F_ERROR != ret) {                       /* 17/01/07, conversion errors */
        if (F_NULL != lv.l_flags) {             /* assign result */
            if (0 == (ret = com_equ(MOP_NOOP, sp, &lv))) {
                acc_assign_int(1);              /* success */
            }
        }
    }
}


/*  Function:           do_put_parm
 *      put_parm primitive.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 *
 *<<GRIEF>>
    Macro: put_parm - Assign an argument value.

        int
        put_parm(int argidx,
            declare val, [int optional = TRUE])

    Macro Description:
        The 'put_parm()' primitive assigns a value 'val' to a
        parameter positioned at 'argidx' that was passed to the
        current macro.

    Macro Parameters:
        argidx - Integer argument index of the parameter passed
            to the current macro which is to be assigned a value;
            parameter indexs start at zero.

        val - Value to be assigned to the parameter; the value
            type should match the type of the parameter.

        optional - Optional boolean value, if *true* missing
            parameters shall not generate an error, otherwose if
            *false* or omitted an error will be echoed.

    Macro Returns:
        The 'put_parm()' primitive returns 1 or greater on sucesss,
        otherwise 0 or less on error.

        On the detection of error conditions the following
        diagnostics messages shall be echoed on the command prompt
        where 'x' is the associated argument number;

>           put_parm: argument index 'x' out of range

>           put_parm: argument 'x' incorrect type

>           put_parm: argument 'x' type conversion error

    Macro Example:

        Assign the value '100' to the first parameter.

>           if (put_parm(0, 100)) {
>               message("assigned");
>           }

    Macro Portability:
        n/a

    Macro See Also:
        get_parm
 */
void
do_put_parm(void)               /* int (int argidx, declare value, [int optional = TRUE]) */
{
    const int argi = get_xinteger(1, -1);       /* non-optional */
    const int optional = get_xinteger(3, TRUE);
    const LIST *lp;
    int ret = 0;

    if (ms_cnt <= 0) {
        acc_assign_int(0);
        return;
    }

    if (argi < 0) {
        ewprintf("put_parm: argument index '%d' out of range", argi);

    } else if (/*argi >= mac_stack[ms_cnt - 1].argc ||*/
                    NULL == (lp = atom_nth(mac_stack[ms_cnt - 1].argv, argi))) {
        if (xf_warnings) {
            infof("put_parm: argument index '%d' out of range", argi);
        }

    } else if (F_STR != *lp) {
        if (!optional || F_NULL != *lp) {
            ewprintf("put_parm: argument '%d' incorrect type", argi);
        }

    } else {
        SYMBOL *sp;

        --x_nest_level; --ms_cnt;
        sp = sym_elookup(LGET_PTR2(const char, lp));
        ++ms_cnt; ++x_nest_level;

        if (NULL != sp) {
            if (! sym_isconstant(sp, "put_parm")) {
                switch(sp->s_type) {
                case F_INT:
                    if (isa_integer(2)) {
                        sym_assign_int(sp, get_integer(2));
                        ret = 1;
                    } else if (isa_float(2)) {
                        sym_assign_int(sp, (int)get_accfloat(2));
                        ret = 1;
                    }
                    break;
                case F_STR:
                    if (isa_string(2)) {
                        sym_assign_nstr(sp, get_str(2), get_strlen(2));
                        ret = 1;
                    }
                    break;
                case F_FLOAT:
                    if (isa_float(2)) {
                        sym_assign_float(sp, get_accfloat(2));
                        ret = 1;
                    } else if (isa_integer(2)) {
                        sym_assign_float(sp, get_integer(2));
                        ret = 1;
                    }
                    break;
                case F_LIST:
                    if (isa_list(2)) {
                        sym_assign_list(sp, get_list(2));
                        ret = 1;
                    }
                    break;
                default:
                    break;
                }

                if (0 == ret) {
                    ewprintf("put_parm: argument '%d' type conversion error", argi);
                    ret = -1;
                }
            }
        }
    }
    acc_assign_int(ret);
}


/*  Function:           do_arg_list
 *      arg_list primitive.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 *
 *<<GRIEF>>
    Macro: arg_list - Argument list.

        list
        arg_list([int eval = FALSE],
            [int start = 0], [int end = -1])

    Macro Description:
        The 'arg_list()' primitive retrieves a list of the values
        representing the arguments which were passed to the current
        macro; allows for arbitrary argument processing.

    Macro Parameters:
        eval - Optional boolean flag, if *true* each argument shall be
            evaluated (e.g. variables are referenced) with the result
            being retrieved, otherwise the raw value (e.g. variable
            name) shall be retrieved.

        start - Optional integer starting the index of the first
            argument to be including within the list. If omitted
            defaults to 1, being the first argument.

        end - Optional integer starting the index of the last
            argument to be including within the list. If omitted
            defaults to -1, being the last argument.

    Macro Returns:
        The 'arg_list()' primitive returns the arguments passed to
        the current macro as a list.

    Macro Examples:

>           void
>           func()
>           {
>               message("%s", arg_list());
>           }

        In the example above, the list of arguments will be shown. But
        in a call like

>           int val = 99;
>           func(val);

        In this case, the message will show "val" and not 99. To get the
        values replaced, use

>           message("%s", arg_list(1));
>           Return value

        List of arguments passed to calling macro.

    Macro Portability:
        The 'start' and 'end' parameters are Grief extensions.

    Macro See Also:
        put_parm, get_parm
*/
void
do_arg_list(void)               /* ([int eval], [int start = 0], [int end = -1]) */
{                                               /* 01/04/10 */
    const int doeval = get_xinteger(1, 0);
    const int start = get_xinteger(2, 0);
    const int end = get_xinteger(3, LIST_MAXLEN);
    const LIST *nextlp, *lp;
    int argi, argc;
    LISTV *argv;
    LIST *new_lp;
    int len;

    acc_assign_null();
    if (ms_cnt <= 0) {
        return;
    }

    /* count active parameters */
    argc = 0;
    for (lp = mac_stack[ms_cnt - 1].argv, argi = 0;
            (nextlp = atom_next(lp)) != lp; lp = nextlp, ++argi) {
        if (argi >= start && argi <= end) {
            ++argc;
        }
    }

    /* build argument list */
    if (argc <= 0 ||
            NULL == (argv = (LISTV *)chk_alloc(argc * sizeof(LISTV)))) {
        return;
    }

    argc = 0;
    for (lp = mac_stack[ms_cnt - 1].argv, argi = 0;
            (nextlp = atom_next(lp)) != lp; lp = nextlp, ++argi) {

        if (argi >= start && argi <= end) {
            if (doeval) {
                /*
                 *  evaluate the parameter,
                 *    must remove the local frame and reinitialise, this is
                 *    required in the event eval() invokes a function.
                 */
                struct mac_stack lmacstack;
                SPTREE *lsym;

                lsym = x_lsym_tbl[x_nest_level];
                x_lsym_tbl[x_nest_level--] = spinit();
                lmacstack = mac_stack[--ms_cnt];
                (void)eval(lp, argv + argc);    /* evaluate the argument */
                mac_stack[ms_cnt++] = lmacstack;
                spfree(x_lsym_tbl[++x_nest_level]);
                x_lsym_tbl[x_nest_level] = lsym;

            } else {
                /*
                 *  otherwise just assign
                 */
                argv_make(argv + argc, lp);
            }
            ++argc;
        }
    }

    /* return results */
    if (NULL != (new_lp = argv_list(argv, argc, &len))) {
        acc_donate_list(new_lp, len);
    }
    chk_free((void *)argv);
}


/*  Function:           do_ref_parm
 *      ref_parm primitive.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 *
 *<<GRIEF>>
    Macro: ref_parm - Create a reference parameter.

        void
        ref_parm(int argument,
            string local_symbol, [int optional = FALSE])

    Macro Description:
        The 'ref_parm()' primitive creates a local reference to one
        of the current macro arguments, this primitive is the
        underlying implementation of macro reference arguments.

>           int mymacro(int &iparm, string &sparm)

        is equivalent to the following

>           int mymacro()
>           {
>               ref_parm(0, "iparm");
>               ref_parm(1, "sparm");

    Macro Note!:
        This interface should be considered as an internal primitive,
        reserved for exclusive use by the GRIEF Macro Compiler and
        may change without notice. Management of variable scoping and
        argument binding shall be handling automatically by the
        compiler.

    Macro Parameters:
        argument - Argument index.

        local_symbol - Name of the local symbol used as the local
            alias to the referenced argument.

        optional - Optional integer flag determining whether the
            argument is optional, if omitted is assumed *FALSE*.

    Macro Returns:
        nothing

    Macro Portability:
        A Grief extension

    Macro See Also:
        get_parm, put_parm
 */
void
do_ref_parm(void)               /* (int argument, string local_symbol, [int optional = FALSE]) */
{
    const int argi = get_xinteger(1, 1);
    const char *symname = get_str(2);
    const int optional = get_xinteger(3, FALSE);
    const LIST *lp;
    SYMBOL *lsp, *sp;

    acc_assign_int(-1);
    if (ms_cnt <= 0) {
        return;
    }

    if (NULL == (lp = atom_nth(mac_stack[ms_cnt - 1].argv, argi))) {
        ewprintf("ref_parm: argument index '%d' out of range", argi);
        return;
    }

    if (F_STR != *lp) {
        if (!optional || F_NULL != *lp) {       /* 11/01/11. optional reference */
            ewprintf("ref_parm: target incorrect type");
        }
        return;
    }

    if (NULL == (lsp = sym_local_lookup(symname))) {
        ewprintf("ref_parm: '%s' not found at current scope.", symname);
        return;
    }

    assert(x_nest_level >= 2);
    --x_nest_level; --ms_cnt;
    sp = sym_elookup(LGET_PTR2(const char, lp));
    ++ms_cnt; ++x_nest_level;

    if (NULL != sp) {
        if (! sym_isconstant(sp, "ref_parm")) {
            sym_destroy(lsp);

            lsp->s_type  = F_REFERENCE;
            lsp->s_flags = SF_REFERENCE;
            lsp->s_sym   = sp;
            acc_assign_int(1);                  /* success */
        }
    }
}
/*end*/
