#ifndef GR_M_FTP_H_INCLUDED
#define GR_M_FTP_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_m_ftp_h,"$Id: m_ftp.h,v 1.6 2014/11/16 17:28:40 ayoung Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: m_ftp.h,v 1.6 2014/11/16 17:28:40 ayoung Exp $
 * FTP primitives -- beta/undocumented.
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

/*--export--defines--*/
/*
 *  ftp primitives
 */
#define DEFAULT_FTP_PORT        21
#define DEFAULT_HTTP_PORT       80
#define DEFAULT_SSH_PORT        22

#define PROTOCOL_FTP            0
#define PROTOCOL_HTTP           1
#define PROTOCOL_HTTPS          2
#define PROTOCOL_FILE	        3
#define PROTOCOL_SFTP           10          /* FTP SSL/TLS */
#define PROTOCOL_FTPS           11          /* FTP SSH */
#define PROTOCOL_RSH            12
#define PROTOCOL_SSH            13
#define PROTOCOL_RSYNC          14
/*--end--*/

/*TODO*/
#define SFTP_OPT_COMPRESSION    0x01
#define SFTP_OPT_NONSTANDARDDES 0x02
#define SFTP_OPT_SSH1           0x04
#define SFTP_OPT_SSH2_MAC_BUG   0x08
#define SFTP_OPT_LOGGING        0x10
#define SFTP_DEFAULT_ALG        "blowfish", "aes", "3des", "WARN", "des"

__CBEGIN_DECLS

void                        do_ftp_chdir(void);
void                        do_ftp_close(void);
void                        do_ftp_connect(void);
void                        do_ftp_connection_list(void);
void                        do_ftp_create(void);
void                        do_ftp_directory(void);
void                        do_ftp_error(void);
void                        do_ftp_find_connection(void);
void                        do_ftp_get_file(void);
void                        do_ftp_getcwd(void);
void                        do_ftp_mkdir(void);
void                        do_ftp_protocol(void);
void                        do_ftp_put_file(void);
void                        do_ftp_register(void);
void                        do_ftp_remove(void);
void                        do_ftp_rename(void);
void                        do_ftp_set_options(void);
void                        do_ftp_sitename(void);
void                        do_ftp_stat(void);
void                        do_ftp_timeout(void);

__CEND_DECLS

#endif /*GR_M_FTP_H_INCLUDED*/
