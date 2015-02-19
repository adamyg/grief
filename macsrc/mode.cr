/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: mode.cr,v 1.5 2014/10/27 23:28:24 ayoung Exp $
 * mode support, common global functionality.
 *
 *
 */

#include "grief.h"
#include "mode.h"

extern string           _bvar_package;
extern string           _bvar_smart;
extern string           _bvar_template;
extern string           _bvar_regular;


/*
 *  cmode ---
 *      "c" syntax mode
 */
void
cmode(void)
{
    _c_modeattach();
}


/*
 *  _mode_pkg_set ---
 *      Set the current buffer package.
 */
string
_mode_pkg_set(string mode)
{
    if (inq_symbol("_bvar_package")) {          /* local mode */
        _bvar_package = mode;

    } else {
        local string _bvar_package = mode;

        make_local_variable(_bvar_package);     /* buffer local variable */
    }
}


/*
 *  _mode_pkg_get ---
 *      Get the current buffer package.
 */
string
_mode_pkg_get(void)
{
    string mode;

    if (inq_symbol("_bvar_package")) {          /* local mode */
        return _bvar_package;
    }
    inq_names(NULL, mode);                      /* default, file extension */
    return (mode);
}


/*
 *  _mode_attr_set ---
 *      Set the current buffer mode attribute.
 */
string
_mode_attr_set(string attr, string val)
{
    switch(attr) {
    case "template":
        if (inq_symbol("_bvar_template")) {
            _bvar_template = val;

        } else {
            local string _bvar_template = val;

            make_local_variable(_bvar_template);
        }
        break;

    case "smart":
        if (inq_symbol("_bvar_smart")) {
            _bvar_smart = val;

        } else {
            local string _bvar_smart = val;

            make_local_variable(_bvar_smart);
        }
        break;

    case "regular":
        if (inq_symbol("_bvar_regular")) {
            _bvar_regular = val;

        } else {
            local string _bvar_regular = val;

            make_local_variable(_bvar_regular);
        }
    }
}


/*
 *  _mode_attr_get ---
 *      Get the current buffer mode attribute.
 */
string
_mode_attr_get(string attr)
{
    switch(attr) {
    case "template":
        if (inq_symbol("_bvar_template")) {
            return _bvar_template;
        }
        break;

    case "smart":
        if (inq_symbol("_bvar_smart")) {
            return _bvar_smart;
        }
        break;

    case "regular":
        if (inq_symbol("_bvar_regular")) {
            return _bvar_regular;
        }
        break;
    }
    return "";
}


/*
 *  modeis ---
 *      Diag interface.
 */
void
modeis(void)
{
    string mode = _mode_pkg_get();

    message("mode=%s", mode);
}

/*end*/
