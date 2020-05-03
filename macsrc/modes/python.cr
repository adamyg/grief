/* -*- mode: cr; indent-width: 4; -*-
 * $Id: python.cr,v 1.1 2020/05/03 21:47:30 cvsuser Exp $
 * Python mode.
 *
 *
 */

#include "../grief.h"
#include "../mode.h"

static list         py_hier_list =
    {
        "Operator                Name                          Associativity",
        "---------------------------------------------------------------------",
        "()                      Parenthesis                   Left to right",
        "**                      Exponent                      Right to left",
        "~                       Bitwise NOT                   Left to right",
        "*, /, %, //             Multiplication, Division,     Left to right",
        "                        Modulo, Floor Division                     ",
        "+, -                    Addition, Subtraction         Left to right",
        ">>, <<                  Bitwise right and left shift  Left to right",
        "&                       Bitwise AND                   Left to right",
        "^                       Bitwise XOR                   Left to right",
        "|                       Bitwise OR                    Left to right",
        "==, !=, >, <, >=, <=    Comparison                    Left to right",
        "  =, +=, -=, *=,        Assignment                    Right to left",
        "/=, %=, **=, //=                                                   ",
        "is, is not              Identity                      Left to right",
        "In, not in              Membership                    Left to right",
        "and, or, not            Logical                       Left to right",
	"",
    };

#define MODENAME    "PYTHON"


void
main()
{
    create_syntax(MODENAME);

    /*
     *  Standard syntax engine
     */
    syntax_token(SYNT_COMMENT,      "#");
    syntax_token(SYNT_QUOTE,        "\\");
    syntax_token(SYNT_STRING,       "\"");
    syntax_token(SYNT_LITERAL,      "\'");
    syntax_token(SYNT_BRACKET,      "([{", ")]}");
    syntax_token(SYNT_DELIMITER,    ",;.?:");
    syntax_token(SYNT_OPERATOR,     "-%+/&*=<>|!~^");
    syntax_token(SYNT_WORD,         "0-9A-Z_a-z");
    syntax_token(SYNT_NUMERIC,      "-+.0-9_xa-fA-F");

    /*
     * DFA
     */
#if (TODO)
    syntax_build(__COMPILETIME__);              /* build and auto-cache */
#endif

    /*
     * Keywords
     */
    define_keywords(SYNK_PRIMARY,
         "as,"+                                 // module alias
         "assert,"+                             // debugging.
         "async,await,"+                        // asyncio library
         "del,"+                                // delete an object reference.
         "except,raise,try,"+                   // exceptions
         "finally,"+
         "from,import,"+                        // modules
         "pass,"+                               // noop
         "return,"+
         "yield,"+
         "with");

    define_keywords(SYNK_OPERATOR,
         "and,or,not,"+                         // logical operators
         "in,is");                              // list operators
	
    define_keywords(SYNK_TYPE,
         "int,float,complex");

    define_keywords(SYNK_STORAGECLASS,
         "global,"+                             // globally scoped variables.
         "nonlocal");                           // externally scoped.

    define_keywords(SYNK_DEFINITION,
         "class,"+                              // define a new user-defined class.
         "def,"+                                // define a user-defined function.
         "lambda");

    define_keywords(SYNK_CONDITIONAL,
         "if,elif,else");                       // conditional branching.

    define_keywords(SYNK_REPEAT,
         "break,continue,"+                     // for/while loop behaviour.
         "for,while");

    define_keywords(SYNK_CONSTANT,
         "True,False,None");
	
    /*
     * Standard functions
     * Source: https://docs.python.org/3/library/functions.html
     */
    define_keywords(SYNK_FUNCTION,
         "__import__,"+
         "abs,"+
         "all,"+
         "any,"+
         "ascii,"+
         "bin,"+
         "bool,"+
         "breakpoint,"+
         "bytearray,"+
         "bytes,"+
         "callable,"+
         "chr,"+
         "classmethod,"+
         "compile,"+
         "delattr,"+
         "dict,"+
         "dir,"+
         "divmod,"+
         "enumerate,"+
         "eval,"+
         "exec,"+
         "filter,"+
         "format,"+
         "frozenset,"+
         "getattr,"+
         "globals,"+
         "hasattr,"+
         "hash,"+
         "help,"+
         "hex,"+
         "id,"+
         "input,"+
         "isinstance,"+
         "issubclass,"+
         "iter,"+
         "len,"+
         "list,"+
         "locals,"+
         "map,"+
         "max,"+
         "memoryview,"+
         "min,"+
         "next,"+
         "object,"+
         "oct,"+
         "open,"+
         "ord,"+
         "pow,"+
         "print,"+
         "property,"+
         "range,"+
         "repr,"+
         "reversed,"+
         "round"+
         "set,"+
         "setattr,"+
         "slice,"+
         "sorted,"+
         "staticmethod,"+
         "str,"+
         "sum,"+
         "super,"+
         "tuple,"+
         "type,"+
         "vars,"+
         "zip");
	
   /*
    * Exceptions
    * Source: https://docs.python.org/3/library/exceptions.html
    */
    define_keywords(SYNK_CONSTANT,
         "BaseException,"+
         "SystemExit,"+
         "KeyboardInterrupt,"+
         "GeneratorExit,"+
         "Exception,"+
         "StopIteration,"+
         "StopAsyncIteration,"+
         "ArithmeticError,"+
         "FloatingPointError,"+
         "OverflowError,"+
         "ZeroDivisionError,"+
         "AssertionError,"+
         "AttributeError,"+
         "BufferError,"+
         "EOFError,"+
         "ImportError,"+
         "ModuleNotFoundError,"+
         "LookupError,"+
         "IndexError,"+
         "KeyError,"+
         "MemoryError,"+
         "NameError,"+
         "UnboundLocalError,"+
         "OSError,"+
         "BlockingIOError,"+
         "ChildProcessError,"+
         "ConnectionError,"+
         "BrokenPipeError,"+
         "ConnectionAbortedError,"+
         "ConnectionRefusedError,"+
         "ConnectionResetError,"+
         "FileExistsError,"+
         "FileNotFoundError,"+
         "InterruptedError,"+
         "IsADirectoryError,"+
         "NotADirectoryError,"+
         "PermissionError,"+
         "ProcessLookupError,"+
         "TimeoutError,"+
         "ReferenceError,"+
         "RuntimeError,"+
         "NotImplementedError,"+
         "RecursionError,"+
         "SyntaxError,"+
         "IndentationError,"+
         "TabError,"+
         "SystemError,"+
         "TypeError,"+
         "ValueError,"+
         "UnicodeError,"+
         "UnicodeDecodeError,"+
         "UnicodeEncodeError,"+
         "UnicodeTranslateError,"+
         "Warning,"+
         "DeprecationWarning,"+
         "PendingDeprecationWarning,"+
         "RuntimeWarning,"+
         "SyntaxWarning,"+
         "UserWarning,"+
         "FutureWarning,"+
         "ImportWarning,"+
         "UnicodeWarning,"+
         "BytesWarning,"+
         "ResourceWarning");
}


/*
 * Modeline/package support
 */
string
_python_mode()
{
    return "python";                            /* return package extension */
}


string
_python_highlight_first()
{
    attach_syntax(MODENAME);
    return "";
}


/*
 * Hier support
 */
list
_python_hier_list()
{
    return py_hier_list;
}

/*end*/










