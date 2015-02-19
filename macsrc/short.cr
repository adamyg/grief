/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: short.cr,v 1.4 2014/10/27 23:28:27 ayoung Exp $
 * Short command set.
 *
 *
 */

#include "grief.h"


void
ht(void)
{
    extern void show_tabs(void);

    show_tabs();
}


void
ut(void)
{
    ht();
    use_tab_char("y");
}


void
r(void)
{
    set_calling_name("");
    read_file();
}


void
e(void)
{
    set_calling_name("");
    edit_file();
}


void
w(void)
{
    set_calling_name("");
    write_buffer();
}


void
x(void)
{
    set_calling_name("");
    exit();
}


void
b(void)
{
    set_calling_name("");
    buffer_list();
}


void
m(void)
{
    set_calling_name("");
    mark();
}


void
c(void)
{
    set_calling_name("");
    cut();
}


void
p(void)
{
    set_calling_name("");
    paste();
}

/*eof*/
