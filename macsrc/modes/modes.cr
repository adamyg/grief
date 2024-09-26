/* -*- mode: cr; indent-width: 4; -*- */
/* $id: crisp.cr,v 1.11 2002/02/27 18:25:42 adamy Exp $
 * Mode configuration
 *
 *
 */

#include "../grief.h"               /* global header */
#include "../mode.h"

static list alist = {       // mode alias map
    /*
     *  Mode                Alias list, note the leading/trailing dot as delimiter.
     */
    "cr",                   ".brief.crisp.grief.",
    "chlog",                ".changelog.",
    "pymode",               ".python.",
    "mak",                  ".makefile.",
    "sh",                   ".bash.",
    "txt",                  ".text.",
    };


static list xlist =  {      // mode extension map
     /*
      * Mode handlers       Extension list, note the leading/trailing dot as delimiter.
      */
    "awk",                  ".awk.",
    "c",                    ".c.C.h.",
    "cmake",                ".cmake.",
    "cplusplus",            ".cpp.cc.c++.cxx.hpp.h++.hxx.",
    "cr",                   ".cr.",
    "csharp",               ".cs.",
    "dosbatch",             ".bat.BAT.cmd.CMD.",
    "fortran",              ".f.f77.f90.",
    "gas",                  ".s.",
    "html",                 ".html.htm.docb.sgml.sgm.",
    "json",                 ".json.",
    "mak",                  ".mk.mak.",
    "masm",                 ".asm.",
    "perl",                 ".pl.pm.",
    "python",               ".py.",
    "rust",                 ".rs.",
    "sh",                   ".sh.csh.tcsh.zsh.bash.ash.rsh.",
    "slang",                ".sl.",
    "txt",                  ".txt.",
    "vim",                  ".vim.",
    "xml",                  ".xml.",
    "yaml",                 ".yaml.yml.",
    };


void
main(void)
{
    autoload("modes/ansi",          "_ansi_mode");

    autoload("modes/awk",           "_awk_mode");

    autoload("modes/dosbatch",      "_dosbatch_mode");

    autoload("modes/c",             "_c_mode", "_c_modeattach", "_cplusplus_mode",
                                            "_c_hier_list", "_cpp_hier_list");

    autoload("modes/chlog",         "_chlog_mode", "_changelog_mode");

    autoload("modes/cmake",         "_cmake_mode");

    autoload("modes/cr",            "_cr_mode", "_cr_hier_list");

    autoload("modes/csharp",        "_csharp_mode", "_csharp_hier_list");

    autoload("modes/gas",           "_gas_mode");

    autoload("modes/html",          "_html_mode");

    autoload("modes/java",          "_java_mode", "_java_hier_list");

    autoload("modes/json",          "_json_mode");

    autoload("modes/lisp",          "_lisp_mode");

    autoload("modes/lua",           "_lua_mode");

    autoload("modes/mak",           "_mak_mode");

    autoload("modes/masm",          "_masm_mode");

    autoload("modes/nasm",          "_nasm_mode");

    autoload("modes/nroff",         "_nroff_mode");

    autoload("modes/perl",          "_perl_mode", "_perl_hier_list");

    autoload("modes/protobuf",      "_protobuf_mode");

    autoload("modes/python",        "_python_mode");

    autoload("modes/rust",          "_rust_mode");

    autoload("modes/sh",            "_sh_mode");

    autoload("modes/slang",         "_slang_mode");

    autoload("modes/fortran",       "_fortran_mode", "_f_mode", "_f77_mode", "_f90_mode");

    autoload("modes/autoconf",      "_autoconf_mode");

    autoload("modes/txt",           "_txt_mode");

    autoload("modes/vim",           "_vim_mode");

    autoload("modes/xml",           "_xml_mode");

    autoload("modes/yaml",          "_yaml_mode");

    autoload("modes/doxygen",       "doxygen_keyword");
}


/*
 *  _none_mode ---
 *      NULL mode, disabling any syntax colorisation.
 */
string
_none_mode(void)
{
    detach_syntax();
}


/*
 *  _modes_alias ---
 *      Alias lookup and map to the base package.
 */
string
_mode_alias(string alias, string def)
{
    int idx;

    if ("" != alias) {                          // locate alias and remap.
        if ((idx = re_search(SF_NOT_REGEXP|SF_IGNORE_CASE, "."+alias+".", alist)) >= 0) {
            return alist[idx - 1];
        }
                                                // locate extension and remap.
        if ((idx = re_search(SF_NOT_REGEXP|SF_IGNORE_CASE, "."+alias+".", xlist)) >= 0) {
            return xlist[idx - 1];
        }
    }
    return def;
}


/*
 *  _mode_extension ---
 *      Extension handler calling during file loads from _extension().
 *
 *  Descriptions:
 *      Invoke the mode interface, forcing the mode module be to autoloaded which in
 *      turn loads (and defines) any of the default highlight, smart, template, regular
 *      or hier BPACKAGE functionality associated with the extension.
 */
void
_mode_extension(string ext)
{
    if ("" != ext) {
        if ((ext = _mode_alias(ext, "")) != "") {
            execute_macro("_" + ext + "_mode");
        }
    }
}


/*
 *  _mode_package ---
 *      Default handlers calling during bpackage processing.
 *
 *      Invokes the mode interface (if any) and retrieves the package name
 *      and redirects the bpackage initialisation.
 */
string
_mode_package(string ext, string cmd)
{
    string mac, pkg;
    int idx;

    if ("" != ext) {
        if ((idx = re_search(SF_NOT_REGEXP, "."+ext+".", xlist)) >= 0) {
            /*
             *  locate handler and invoke
             */
            pkg = execute_macro("_" + xlist[idx -1] + "_mode");
            if ("" != pkg) {
                if (cmd != "") {                // retrieve package prefix
                    mac = "_" + pkg + cmd;
                    if (inq_macro(mac) > 0) {
                        return mac;
                    }
                } else {
                    return pkg;
                }
            }
        }
    }
    return "";
}

/*end*/
