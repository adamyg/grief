// -*- mode: cr; indent-width: 4; -*-
// $Id: listtest1.cr,v 1.2 2014/10/22 02:34:37 ayoung Exp $
// List tests
//

void
listtest1_pop()
{
   list l = {"one", "two", "three", "four"};

   while (length_of_list(l)) {
      message("%s", pop(l));
   }

}

void
listtest1_shift()
{
   list l = {"one", "two", "three", "four"};

   while (length_of_list(l)) {
      message("%s", shift(l));
   }
}
