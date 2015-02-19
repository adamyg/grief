/* $Id: hier.cr,v 1.6 2014/10/22 02:34:18 ayoung Exp $
 * Language support macro.
 *
 *
 */

#include "grief.h"
#include "mode.h"

static list	        hierarchies = {
    "C",                "hier_show \"c\"",
	"C++",              "hier_show \"cpp\"",
	"GRIEF",            "hier_show \"grief\"",
    "c#",               "hier_show \"csharp\"",
	"Javascript",       "hier_show \"java\"",
	"Perl",             "hier_show \"perl\"",
    };

static list         grief_hier_list =  {
    "Arity       Operator                                   Assoc",
    "--------------------------------------------------------------",
    "binary   ()  []  ->  .                                 l -> r",
    "unary    !   ~   ++  --  -  (type)  *  &  sizeof       r -> l",
    "binary   *   /   %                                     l -> r",
    "binary   +   -                                         l -> r",
    "binary   <<  >>                                        l -> r",
    "binary   <   <=  >   >=                                l -> r",
    "binary   ==  !=  =~  !~                                l -> r",
    "binary   &                                             l -> r",
    "binary   ^                                             l -> r",
    "binary   |                                             l -> r",
    "binary   &&                                            l -> r",
    "binary   ||                                            l -> r",
    "ternary  ?:                                            r -> l",
    "binary   = += -= *= /= %= >>= <<= &= ^= |=             r -> l",
    "binary   ,                                             l -> r",
    "--------------------------------------------------------------"
    };


list
_grief_hier_list(void)
{
    return grief_hier_list;
}


void
hier(void)
{
    select_list("Language Operator Tables", "", 2, hierarchies, 0, "hier");
}


void
chier(void)
{
    hier_show("c");
}


void
cppchier(void)
{
    hier_show("cpp");
}


void
hier_show(string what)
{
    declare ret;

    ret = execute_macro( "_" + what + "_hier_list");
    if (is_list(ret)) {
        select_list(what + " operator chart", "", 1, ret);
    }
}

/*
 *  Local Variables: ***
 *  mode: cr ***
 *  tabs: 4 ***
 *  End: ***
 */
