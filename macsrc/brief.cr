/* $Id: brief.cr,v 1.10 2014/10/27 23:28:18 ayoung Exp $
 * Brief macro emulation primitives.
 *
 *
 */

#include "grief.h"


#undef inq_brief_level
replacement int
inq_brief_level(void)
{
   return atoi(getenv("GRLEVEL"));
}


//TODO
// replacement int
// save_keystroke_macro(string filename)
// {
//    string = inq_remember_macro();
//    return -1;
// }


/*replacement*/
int
del(string name)
{
    return (remove(name) >= 0 ? 1 : -1);
}


/*replacement*/
int
dos(string cmd, int use_shell, string callback)
{
    return shell(cmd, !use_shell, callback);
}


/*replacement*/
#undef inq_environment
string
inq_environment(string name)
{
   return getenv(name);
}

/*eof*/
