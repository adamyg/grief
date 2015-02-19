/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: gui.cr,v 1.5 2014/10/27 23:28:22 ayoung Exp $
 * GUI support.
 *
 *
 */

#include "grief.h"

//  Function:           create_notice
//      Create and display a modal message dialog box.
//
//  Parameters:
//      msg -               Message text, newline seperated lines.
//      button1 -           Button definition text.
//      ... -               Additional buffer definitions, upto 3 in total.
//
//  Returns:
//      Button number pressed (0...). Upon an error -1 shall be returned.
//
int
create_notice(string msg, string button1, ...)
{
    UNUSED(msg, button1);
    return -1;
}


//  Function:           dialog_box
//      Create and display the built-in dialog box of the specified 'type'.
//
//  Parameters:
//      type -              Dialog type.
//
//  Types:
//      font
//      open_file
//      color
//
//  Returns:
//      Returns 1 on success, 0 if the dialog was cancelled,
//      otherwise -1 if the type is not supported.
//
int
dialog_box(string type, string &retval, ~string title)
{
    UNUSED(type, retval, title);
    return -1;
}

/*eof*/
