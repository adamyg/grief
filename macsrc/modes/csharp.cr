/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: csharp.cr,v 1.11 2014/10/22 02:34:33 ayoung Exp $
 * C# Programming mode.
 *
 *
 */

#include "../grief.h"

static list cs_hier_list = {
    "Category         Operator  Description               Assoc   ",
    "-------------------------------------------------------------",
    "Primary          .         member selection                  ",
    "                 ->        member selection                  ",
    "                 []        subscripting                      ",
    "                 ()        function call                     ",
    "                 ()        value construction                ",
    "                 sizeof    size of object                    ",
    "                 sizeof    size of type                      ",
    "                 checked                                     ",
    "                 unchecked                                   ",
    "                 stackalloc                                  ",
    "                 ------------------------------------------- ",
    "                 +         unary plus                        ",
    "                 -         unary minus                       ",
    "                 !         not                               ",
    "                 ++        post increment                    ",
    "                 ++        pre increment                     ",
    "                 --        post decrement                    ",
    "                 --        pre decrement                     ",
    "                 ()        cast (type conversion)            ",
    "                 *         dereference                       ",
    "                 &         address of                        ",
    "                 ------------------------------------------- ",
    "Multiplicative   *         multiply                          ",
    "                 /         divide                            ",
    "                 %         modulo (remainder)                ",
    "                 ------------------------------------------- ",
    "Additive         +         add (plus)                        ",
    "                 -         subtract(minus)                   ",
    "                 ------------------------------------------- ",
    "Shift            <<        shift left                        ",
    "                 >>        shift right                       ",
    "                 ------------------------------------------- ",
    "Relational       <         less than                         ",
    "                 <=        less than or equal                ",
    "                 >         greater than                      ",
    "                 >=        greater than or equal             ",
    "                 is                                          ",
    "                 as                                          ",
    "                 ------------------------------------------- ",
    "Equality         ==        equal                             ",
    "                 !=        not equal                         ",
    "                 ------------------------------------------- ",
    "Logical          &         bitwise AND                       ",
    "                 ------------------------------------------- ",
    "                 ^         bitwise exclusive OR              ",
    "                 ------------------------------------------- ",
    "                 |         bitwise inclusive OR              ",
    "                 ------------------------------------------- ",
    "Conditional      &&        logical AND                       ",
    "                 ------------------------------------------- ",
    "                 ||        logical inclusive OR              ",
    "                 ------------------------------------------- ",
    "                 ? :       conditional expression            ",
    "                 ------------------------------------------- ",
    "Assignment       =         simple assignment                 ",
    "                 *=        multiply and assign               ",
    "                 /=        divide and assign                 ",
    "                 %=        modulo and assign                 ",
    "                 +=        add and assign                    ",
    "                 -=        subtract and assign               ",
    "                 <<=       shift left and assign             ",
    "                 >>=       shift right and assign            ",
    "                 &=        AND and assign                    ",
    "                 |=        inclusive OR and assign           ",
    "                 ^=        exclusive OR and assign           ",
    "                 ------------------------------------------- ",
    "",
    "The operators are listed in precedence order according to the",
    "category in which they fit. That is, the primary operators",
    "(e.g., ++) are evaluated before the unary operators",
    "(e.g., !). Multiplication is evaluated before addition"
    };


#define MODENAME "CSHARP"

void
main()
{
    create_syntax(MODENAME);

    /*
     *  Standard syntax engine
     */
    syntax_token(SYNT_COMMENT,      "/*", "*/");
    syntax_token(SYNT_COMMENT,      "//");
    syntax_token(SYNT_QUOTE,        "\\");
    syntax_token(SYNT_PREPROCESSOR, "#");
    syntax_token(SYNT_CHARACTER,    "\'");
    syntax_token(SYNT_STRING,       "\"");
    syntax_token(SYNT_BRACKET,      "([{", ")]}");
    syntax_token(SYNT_OPERATOR,     "-%+/&*=<>|!~^");
    syntax_token(SYNT_DELIMITER,    ",;.?:");
    syntax_token(SYNT_WORD,         "0-9a-zA-Z_");
    syntax_token(SYNT_NUMERIC,      "-+0-9a-fA-F.xXL");

    /*
     *  options
     */
    set_syntax_flags(SYNF_COMMENTS_CSTYLE|SYNF_PREPROCESSOR_WS);

    /*
     *  Keywords
     */
    define_keywords(SYNK_PRIMARY,   "as,do,if,in,is", -2);
    define_keywords(SYNK_PRIMARY,   "for,int,new,out,ref,try", -3);
    define_keywords(SYNK_PRIMARY,   "base,bool,byte,case,char,else,enum,goto,lock,long,"+
                                    "null,this,true,uint,void", -4);
    define_keywords(SYNK_PRIMARY,   "break,catch,class,const,event,false,fixed,float,"+
                                    "sbyte,short,throw,ulong,using,while", -5);
    define_keywords(SYNK_PRIMARY,   "double,extern,object,params,public,return,sealed,"+
                                    "sizeof,static,string,struct,switch,typeof,unsafe,"+
                                    "ushort", -6);
    define_keywords(SYNK_PRIMARY,   "checked,decimal,default,finally,foreach,private,virtual", -7);
    define_keywords(SYNK_PRIMARY,   "abstract,continue,delegate,explicit,implicit,internal,"+
                                    "operator,override,readonly,volatile", -8);
    define_keywords(SYNK_PRIMARY,   "interface,namespace,protected,unchecked", 9);
    define_keywords(SYNK_PRIMARY,   "stackalloc", -10);
}


/*
 * Modeline/package support
 */
string
_csharp_mode()
{
    return "csharp";                            /* return package extension */
}


string
_csharp_highlight_first()
{
    attach_syntax(MODENAME);
    return "";
}


/*
 * Hier support
 */
list
_csharp_hier_list()
{
    return cs_hier_list;
}
