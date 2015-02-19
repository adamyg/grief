/* -*- mode: cr; indent-width: 4; -*-
 * $Id: mak.cr,v 1.11 2014/10/22 02:34:35 ayoung Exp $
 * 'make' syntax.
 *
 *
 */

#include "../grief.h"

/*
    Standard usage:
        -*- mode: mak; indent-tabs-mode: t; tab-width: 8 -*-

    Keywords:
        AR                ARFLAGS           AS
        ASFLAGS           CC                CFLAGS
        CO                COFLAGS           COMSPEC
        CPP               CPPFLAGS          CTANGLE
        CWEAVE            CXX               CXXFLAGS
        DEFAULT           DELETE_ON_ERROR   EXPORT_ALL_VARIABLES
        FC                FFLAGS            GET
        GFLAGS            GPATH
        IGNORE            INTERMEDIATE      LDFLAGS
        LEX               LFLAGS            LIBPATTERNS
        MAKE              MAKECMDGOALS      MAKEFILES
        MAKEFLAGS         MAKEINFO          MAKELEVEL
        MAKEOVERRIDES     MFLAGS            Makefile
        NOTPARALLEL       OUTPUT_OPTION     PC
        PFLAGS            PHONY             POSIX
        PRECIOUS          RFLAGS            RM
        SECONDARY         SHELL             SILENT
        SUFFIXES          SUFFIXES          TANGLE
        TEX               TEXI2DVI          VPATH
        WEAVE             YACC              YACCR
        YFLAGS

        addprefix         addsuffix         basename
        call              define            dir
        else              endef             endif
        error             export            filter
        filter-out        findstring        firstword
        foreach           if                ifdef
        ifeq              ifndef            ifneq
        include           join              makefile
        notdir            origin            override
        patsubst          shell             sort
        strip             subst             suffix
        unexport          vpath             warning
        wildcard          word              wordlist
        words
**/

#define MODENAME "MAKE"


void
main()
{
    create_syntax(MODENAME);

    syntax_token(SYNT_COMMENT,      "#");
    syntax_token(SYNT_BRACKET,      "([{", ")]}");
    syntax_token(SYNT_STRING,       "\"");
    syntax_token(SYNT_LITERAL,      "\'");
    syntax_token(SYNT_OPERATOR,     ",;:$");
    syntax_token(SYNT_DELIMITER,    "-%+/&*=<>|!~^");
    syntax_token(SYNT_WORD,         "-0-9a-zA-Z_.");
    syntax_token(SYNT_NUMERIC,      "-+0-9");

    /*
     *  Options,
     *      Hilite whitespace
     *      Hilite (trailing) line-cont whitespace.
     */
    set_syntax_flags(SYNF_HILITE_WS|SYNF_HILITE_LINECONT);

    /*
     *  Keywords
     */
    define_keywords(SYNK_PRIMARY,   "ARASCCCOFCPCRMif", 2);
    define_keywords(SYNK_PRIMARY,   "CPPCXXGETLEXTEXdir", 3);
    define_keywords(SYNK_PRIMARY,   "MAKEYACCcallelseifeqjoinsortword", 4);
    define_keywords(SYNK_PRIMARY,   "GPATHPHONYPOSIXSHELLVPATHWEAVEYACCR" +
                                    "endefendiferrorifdefifneqshellstrip" +
                                    "substvpathwords", 5);
    define_keywords(SYNK_PRIMARY,   "CFLAGSCWEAVEFFLAGSGFLAGSIGNORELFLAGS" +
                                    "MFLAGSPFLAGSRFLAGSSILENTTANGLEYFLAGS" +
                                    "defineexportfilterifndefnotdiroriginsuffix", 6);
    define_keywords(SYNK_PRIMARY,   "ARFLAGSASFLAGSCOFLAGSCOMSPECCTANGLEDEFAULT" +
                                    "LDFLAGSforeachincludewarning", 7);
    define_keywords(SYNK_PRIMARY,   "CPPFLAGSCXXFLAGSMAKEINFOMakefilePRECIOUS" +
                                    "SUFFIXESSUFFIXESTEXI2DV" +
                                    "Ibasenamemakefileoverride" +
                                    "patsubstunexportwildcardwordlist", 8);
    define_keywords(SYNK_PRIMARY,   "MAKEFILESMAKEFLAGSMAKELEVELSECONDARY" +
                                    "addprefixaddsuffixfirstword", 9);
    define_keywords(SYNK_PRIMARY,   "filter-outfindstring", 10);
    define_keywords(SYNK_PRIMARY,   "LIBPATTERNSNOTPARALLEL", 11);
    define_keywords(SYNK_PRIMARY,   "INTERMEDIATEMAKECMDGOALS", 12);
    define_keywords(SYNK_PRIMARY,   "MAKEOVERRIDESOUTPUT_OPTION", 13);
    define_keywords(SYNK_PRIMARY,   "DELETE_ON_ERROR", 15);
    define_keywords(SYNK_PRIMARY,   "EXPORT_ALL_VARIABLES", 20);
}


/*
 *  Modeline/package support
 */
string
_mak_mode()
{
    return "mak";
}


string
_mak_highlight_first()
{
    attach_syntax(MODENAME);
    return "";
}
