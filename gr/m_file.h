#ifndef GR_M_FILE_H_INCLUDED
#define GR_M_FILE_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_m_file_h,"$Id: m_file.h,v 1.17 2014/10/26 22:13:11 ayoung Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: m_file.h,v 1.17 2014/10/26 22:13:11 ayoung Exp $
 * File primitives.
 *
 *
 * This file is part of the GRIEF Editor.
 *
 * The GRIEF Editor is free software: you can redistribute it
 * and/or modify it under the terms of the GRIEF Editor License.
 *
 * The GRIEF Editor is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * License for more details.
 * ==end==
 */

#include <edsym.h>

__CBEGIN_DECLS

struct stat;

extern void                 acc_assign_stat(struct stat *sb, int rc);

extern void                 do_access(void);
extern void                 do_cd(void);
extern void                 do_chdir(void);
extern void                 do_chmod(void);
extern void                 do_chown(void);
extern void                 do_compare_files(void);
extern void                 do_copy_ea_info(void);
extern void                 do_exist(void);
extern void                 do_expandpath(void);
extern void                 do_searchpath(void);
extern void                 do_file_canon(void);
extern void                 do_file_glob(void);
extern void                 do_file_match(void);
extern void                 do_file_pattern(void);
extern void                 do_filename(int dirname);   /*basename and dirname*/
extern void                 do_filename_match(void);
extern void                 do_filename_realpath(void);
extern void                 do_find_file(int mode);
extern void                 do_fstype(void);
extern void                 do_ftest(void);
extern void                 do_getwd(void);
extern void                 do_glob(void);
extern void                 do_lstat(void);
extern void                 do_mkdir(void);
extern void                 do_mktemp(void);
extern void                 do_mode_string(void);
extern void                 do_parse_filename(void);
extern void                 do_read_ea(void);
extern void                 do_readlink(void);
extern void                 do_realpath(void);
extern void                 do_link(void);
extern void                 do_unlink(void);
extern void                 do_remove(void);
extern void                 do_rename(void);
extern void                 do_rmdir(void);
extern void                 do_set_ea(void);
extern void                 do_stat(void);
extern void                 do_strfilecmp(void);
extern void                 do_symlink(void);
extern void                 do_umask(void);

__CEND_DECLS

#endif /*GR_M_FILE_H_INCLUDED*/
