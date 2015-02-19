/* -*- mode: cr; indent-width: 4; -*-
 * $Id: lua.cr,v 1.7 2014/10/22 02:34:35 ayoung Exp $
 * LUA syntax support mode
 *
 *
 *
 */

#include "../grief.h"
#include "../mode.h"

#define MODENAME "LUA"


void
main()
{
    create_syntax(MODENAME);

    /*
     *  Standard syntax engine/
     *      used for basic line processing.
     */
    syntax_token(SYNT_COMMENT,   "--");
    syntax_token(SYNT_QUOTE,     "\\");
    syntax_token(SYNT_CHARACTER, "\'");
    syntax_token(SYNT_STRING,    "\"");
    syntax_token(SYNT_BRACKET,   "([{", ")]}");
    syntax_token(SYNT_OPERATOR,  "-%+/&*=<>|!~^");
    syntax_token(SYNT_DELIMITER, ",;.?:");
    syntax_token(SYNT_WORD,      "0-9a-zA-Z_");
    syntax_token(SYNT_NUMERIC,   "-+0-9a-fA-F.xXL");

    /*
     *  Advanced syntax engine/
     *      used to built a DFA based lexer/parser, which are generally faster.
     */
    syntax_rule("--.*$", "spell,todo,quick:comment");

    syntax_rule("[A-Za-z_][A-Za-z_0-9.]*", "keyword:normal");

    syntax_rule("[0-9]+(\\.[0-9]*)?([Ee][-+]?[0-9]*)?", "number");
    syntax_rule("0[xX][0-9A-Fa-f]*", "number");
    syntax_rule("[0-9]+", "number");

    syntax_rule("\"(\\\\\"|[^\"])*\"", "string");
    syntax_rule("\"(\\\\\"|[^\"])*\\$", "string");

    syntax_rule("\'(\\\\\'|[^\'])*\'", "string");
    syntax_rule("\'(\\\\\'|[^\'])*\\$", "string");

    syntax_rule("[()\\[\\]{},;.?:]", "delimiter");
    syntax_rule("[-%+/&*=<>|!~^]", "operator");

    syntax_build(__COMPILETIME__);              /* build and auto-cache */

    /*
     *  keywords
     */
    define_keywords(SYNK_PRIMARY,
        "and,"+
        "break,"+
        "do,"+
        "else,"+
        "elsif,"+
        "end,"+
        "for,"+
        "function,"+
        "if,"+
        "in,"+
        "local,"+
        "nil,"+
        "not,"+
        "or,"+
        "repeat,"+
        "return,"+
        "then,"+
        "until,"+
        "while");

    define_keywords(SYNK_CONSTANT,
        "LUA_PATH,"+
        "_REQUIREDNAME,"+
        "_ALERT,"+
        "_ERRORMESSAGE,"+
        "_LOADED,"+
        "_VERSION");

    define_keywords(SYNK_FUNCTION,
        "abs,"+
        "acos,"+
        "appendto,"+
        "ascii,"+
        "asin,"+
        "assert,"+
        "atan,"+
        "atan2,"+
        "call,"+
        "ceil,"+
        "clock,"+
        "closefile,"+
        "collectgarbage,"+
        "copytagmethods,"+
        "cos,"+
        "date,"+
        "debug.gethook,"+
        "debug.getinfo,"+
        "debug.getlocal,"+
        "debug.sethook,"+
        "debug.setlocal,"+
        "deg,"+
        "dofile,"+
        "dostring,"+
        "error,"+
        "execute,"+
        "exit,"+
        "exp,"+
        "floor,"+
        "flush,"+
        "foreach,"+
        "foreachi,"+
        "format,"+
        "frexp,"+
        "gcinfo,"+
        "getenv,"+
        "getglobal,"+
        "getglobals,"+
        "getinfo,"+
        "getlocal,"+
        "getmetatable,"+
        "getn,"+
        "gettagmethod,"+
        "globals,"+
        "gsub,"+
        "io.close,"+
        "io.flush,"+
        "io.input,"+
        "io.lines,"+
        "io.open,"+
        "io.output,"+
        "io.read,"+
        "io.stderr,"+
        "io.stdin,"+
        "io.stdout,"+
        "io.tmpfile,"+
        "io.write,"+
        "ipairs,"+
        "ldexp,"+
        "loadfile,"+
        "loadstring,"+
        "log,"+
        "log10,"+
        "math.abs,"+
        "math.acos,"+
        "math.asin,"+
        "math.atan,"+
        "math.atan2,"+
        "math.ceil,"+
        "math.cos,"+
        "math.deg,"+
        "math.exp,"+
        "math.floor,"+
        "math.frexp,"+
        "math.ldexp,"+
        "math.log,"+
        "math.log10,"+
        "math.max,"+
        "math.min,"+
        "math.mod,"+
        "math.rad,"+
        "math.random,"+
        "math.randomseed,"+
        "math.sin,"+
        "math.squrt,"+
        "math.tan,"+
        "max,"+
        "min,"+
        "mod,"+
        "newtag,"+
        "next,"+
        "openfile,"+
        "os.clock,"+
        "os.date,"+
        "os.difftime,"+
        "os.execute,"+
        "os.exit,"+
        "os.getenv,"+
        "os.remove,"+
        "os.rename,"+
        "os.setlocale,"+
        "os.time,"+
        "os.tmpname,"+
        "pairs,"+
        "pcall,"+
        "print,"+
        "rad,"+
        "random,"+
        "randomseed,"+
        "rawget,"+
        "rawset,"+
        "read,"+
        "readfrom,"+
        "remove,"+
        "rename,"+
        "require,"+
        "seek,"+
        "setcallhook,"+
        "setglobal,"+
        "setglobals,"+
        "setlinehook,"+
        "setlocal,"+
        "setlocale,"+
        "setmetatable,"+
        "settag,"+
        "settagmethod,"+
        "sin,"+
        "sort,"+
        "squrt,"+
        "strbyte,"+
        "strchar,"+
        "strfind,"+
        "string.byte,"+
        "string.char,"+
        "string.find,"+
        "string.format,"+
        "string.gfind,"+
        "string.gsub,"+
        "string.len,"+
        "string.lower,"+
        "string.rep,"+
        "string.sub,"+
        "string.upper,"+
        "strlen,"+
        "strlower,"+
        "strrep,"+
        "strsub,"+
        "strupper,"+
        "table.concat,"+
        "table.foreach,"+
        "table.foreachi,"+
        "table.getn,"+
        "table.insert,"+
        "table.remove,"+
        "table.setn,"+
        "table.sort,"+
        "tag,"+
        "tan,"+
        "tinsert,"+
        "tmpname,"+
        "tonumber,"+
        "tostring,"+
        "tremove,"+
        "type,"+
        "unpack,"+
        "write,"+
        "writet"
        );
}


/*
 *  Modeline/package support
 */
string
_lua_mode(void)
{
    return "lua";                               /* return package extension */
}


void
_lua_modeattach(void)
{
    attach_syntax(MODENAME);
}


string
_lua_highlight_first(void)
{
    _lua_modeattach();
    return "";
}
