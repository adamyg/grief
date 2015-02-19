/* $Id: cpp.cr,v 1.8 2014/10/27 23:28:19 ayoung Exp $
 *
 *
 */

#include "grief.h"


void
main()
{
}


void
declcplex(string kwd, ~string)
{
   int line_no, col_no;                         /* Save old position */
   string rest;
   int old_autoindent = autoindent("n");

   insert(kwd);                                 /* Put in keyword */
   inq_position(line_no, col_no);
   insert("\n{\n");
   if (get_parm(1, rest)) {                     /* Another argument was given */
      insert(rest);
   }
   insert("};\n");
   move_abs(line_no, col_no);                   /* Move to saved position */
   if (old_autoindent) autoindent("y");
}


void
doclass()
{
   declcplex("class ", "  public:\n  protected:\n  private:\n");
}


void
dostruct()
{
   declcplex("struct ");
}


void
doenum()
{
   declcplex("enum ");
}

/*end*/
