// -*- mode: cr; indent-width: 4; -*-
// $Id: helloworld.cr,v 1.1 2014/06/27 19:22:19 ayoung Exp $
// Source: helloworld.cr
//    Grief Macro Tutorial
//

#include <grief.h>

void
helloworld()
{
   message("Hello, world!");
}

void
main()
{
   message("Welcome");
   sleep(2);
}

