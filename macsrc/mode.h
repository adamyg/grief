#ifndef MACSRC_MODE_H_INCLUDED
#define MACSRC_MODE_H_INCLUDED
/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: mode.h,v 1.5 2014/10/27 23:28:24 ayoung Exp $
 * Mode related support functionality
 *
 *
 */

extern string           gri_package;

#if defined(__PROTOTYPES__)
extern void             mode(~string);
extern void             cmode(void);

extern void             hier(void);
extern void             chier(void);
extern void             hier_show(string what);

extern void             abbrev(~string);
extern void             abbrev_load(string arg);
extern void             _abbrev_enable(void);
extern void             _abbrev_set(int mode);
extern int              _abbrev_get(void);
extern void             _abbrev_check(void);

extern string           _mode_alias(string alias, string def);
extern void             _mode_extension(string ext);
string                  _mode_package(string ext, string cmd);

extern string           _mode_pkg_set(string mode);
extern string           _mode_pkg_get(void);
extern string           _mode_attr_set(string attr, string val);
extern string           _mode_attr_get(string attr);

extern void             routines_search(string sstr, int sflags, string name, string trim_func);

extern void             _package_call(string event_name, string mode);
extern void             _package_dump(void);

extern string           _c_mode(void);
extern void             _c_modeattach(void);
#endif

#endif  //MACSRC_MODE_H_INCLUDED
