/* -*- mode: cr; indent-width: 4; -*-
 * $Id: dosbatch.cr,v 1.5 2014/10/22 02:34:33 ayoung Exp $
 * DOS/CMD batch syntax mode.
 *
 *
 */

#include "../grief.h"

#define MODENAME "DOSBATCH"


void
main()
{
    create_syntax(MODENAME);

    syntax_token(SYNT_STRING,    "\"");
    syntax_token(SYNT_BRACKET,   "([{", ")]}");
    syntax_token(SYNT_OPERATOR,  "=");
    syntax_token(SYNT_DELIMITER, "$@,;.?:");
    syntax_token(SYNT_WORD,      "0-9a-zA-Z_");
    syntax_token(SYNT_NUMERIC,   "-+0-9a-fA-F.xXL");

    set_syntax_flags(SYNF_CASEINSENSITIVE|SYNF_STRING_ONELINE);

    syntax_rule("^rem.*$", "spell,todo:comment");
    syntax_rule("^@rem.*$", "spell,todo:comment");
    syntax_rule("\"(\\\\\"|[^\"])*\"", "string");

    syntax_rule("[A-Za-z_][A-Za-z0-9_()]*", "keyword:normal");
    syntax_rule("[0-9]+(\\.[0-9]+)?([Ee][\\-\\+]?[0-9]+)?", "number");

    syntax_build(__COMPILETIME__);

    define_keywords(SYNK_PRIMARY,
	 "ASSOC,"+
	 "BREAK,"+
	 "CALL,"+
	 "CD,"+
	 "CHDIR,"+
	 "COPY,"+
	 "DEL,"+
	 "DIR,"+
	 "ECHO,"+
	 "ENDLOCAL,"+
	 "EXIT,"+
	 "FOR,"+
	 "FTYPE,"+
	 "GOTO,"+
	 "IF,"+
	 "MD,"+
	 "MKDIR,"+
	 "POPD,"+
	 "PROMPT,"+
	 "PUSHD,"+
	 "RD,"+
	 "REM,"+
	 "RENAME,"+
	 "RMDIR,"+
	 "SET,"+
	 "SETLOCAL,"+
	 "SHIFT,"+
	 "START,"+
	 "TYPE"
	 );

    define_keywords(SYNK_FUNCTION,
	 "ATTRIB,"+
	 "BCDEDIT,"+
	 "CACLS,"+
	 "CHCP,"+
	 "CHKDSK,"+
	 "CHKNTFS,"+
	 "CLS,"+
	 "CMD,"+
	 "COLOR,"+
	 "COMP,"+
	 "COMPACT,"+
	 "CONVERT,"+
	 "DATE,"+
	 "DISKCOMP,"+
	 "DISKCOPY,"+
	 "DISKPART,"+
	 "DOSKEY,"+
	 "DRIVERQUERY,"+
	 "ERASE,"+
	 "FC,"+
	 "FIND,"+
	 "FINDSTR,"+
	 "FORMAT,"+
	 "FSUTIL,"+
	 "GPRESULT,"+
	 "GRAFTABL,"+
	 "HELP,"+
	 "ICACLS,"+
	 "LABEL,"+
	 "MKLINK,"+
	 "MODE,"+
	 "MORE,"+
	 "MOVE,"+
	 "OPENFILES,"+
	 "PATH,"+
	 "PAUSE,"+
	 "PRINT,"+
	 "RECOVER,"+
	 "REN,"+
	 "REPLACE,"+
	 "ROBOCOPY,"+
	 "SC,"+
	 "SCHTASKS,"+
	 "SHUTDOWN,"+
	 "SORT,"+
	 "SUBST,"+
	 "SYSTEMINFO,"+
	 "TASKKILL,"+
	 "TASKLIST,"+
	 "TIME,"+
	 "TITLE,"+
	 "TREE,"+
	 "VER,"+
	 "VERIFY,"+
	 "VOL,"+
	 "WMIC,"+
	 "XCOPY"
	 );

   define_keywords(SYNK_OPERATOR,
	 "IN,DO,"+				// FOR %var IN (set) DO command
	 "NOT,EXISTS,"+				// IF [NOT] EXISTS var command
	 "DEFINED,"+				// IF [NOT] DEFINED var command
						// IF /I string op string command
	 "EQU,"+				//	 equal
	 "NEQ,"+				//	 not equal
	 "LSS,"+				//	 less than
	 "LEQ,"+				//	 less than or equal
	 "GTR,"+				//	 greater than
	 "GEQ"					//       greater than or equa
	 );

   define_keywords(SYNK_CONSTANT,
	 "APPDATA,"+
	 "COMPUTERNAME,"+
	 "COMSPEC,"+
	 "CommonProgramFiles,"+
	 "CommonProgramW6432,"+
	 "HOMEDRIVE,"+
	 "HOMEPATH,"+
	 "NUMBER_OF_PROCESSORS,"+
	 "OS,"+
	 "PATH,"+
	 "PATHEXT,"+
	 "PROCESSOR_ARCHITECTURE,"+
	 "PROCESSOR_IDENTIFIER,"+
	 "PROCESSOR_LEVEL,"+
	 "PROCESSOR_REVISION,"+
	 "PROMPT,"+
	 "ProgramData,"+
	 "ProgramFiles(x86),"+
	 "ProgramFiles,"+
	 "ProgramW6432,"+
	 "SystemDrive,"+
	 "SystemRoot,"+
	 "TEMP,TMP,"+
	 "USERDOMAIN,"+
	 "USERNAME,"+
	 "USERPROFILE,"+
	 "windir,"+
	 "windows_tracing_flags,"+
	 "windows_tracing_logfile,"+
	 "ERRORLEVEL,"+				// IF ERRORLEVEL value command
	 "CMDEXTVERSION,"+                      // IF CMDEXTVERSION value command
	 "ENABLEEXTENSIONS,"+			// SETLOCAL
	 "DISABLEEXTENSIONS,"+
	 "ENABLEDELAYEDEXPANSION,"+
	 "DISABLEDELAYEDEXPANSION"
	 );
}


string
_dosbatch_mode()
{
    return "dosbatch";				/* return package extension */
}


string
_dosbatch_highlight_first()
{
    attach_syntax(MODENAME);                    /* attach colorizer */
    return "";
}
