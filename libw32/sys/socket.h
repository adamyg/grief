#ifndef GR_SOCKET_H_INCLUDED
#define GR_SOCKET_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_libw32_sys_socket_h,"$Id: socket.h,v 1.8 2015/02/19 00:17:39 ayoung Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 <sys/socket.h>
 *
 * Copyright (c) 1998 - 2015, Adam Young.
 * All rights reserved.
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

#include <sys/cdefs.h>
#include <win32_include.h>                      /* winsock and windows.h guard */

__BEGIN_DECLS

extern int              w32_h_errno;

struct pollfd;

int                     w32_sockinit(void);
int                     w32_getaddrinfo(const char *nodename, const char *servname,
                                const struct addrinfo *hints, struct addrinfo **res);
struct hostent *        w32_gethostbyname(const char *host);
const char *            w32_hstrerror(int herrno);
void                    w32_herror(const char *msg);
int                     w32_socket(int af, int type, int protocol);
int                     w32_connect(int fd, const struct sockaddr *name, int namelen);
int                     w32_getsockopt(int fd, int level, int optname, void *optval, int *optlen);
int                     w32_setsockopt(int fd, int level, int optname, const void *optval, int optlen);
int                     w32_getpeername(int fd, struct sockaddr *name, int *namelen);
int                     w32_bind(int fd, const struct sockaddr *name, int namelen);
int                     w32_listen(int fd, int num);
int                     w32_accept(int fd, struct sockaddr * addr, int * addrlen);
#if !defined(CR_POLL_H_INCLUDED)
int                     w32_poll(struct pollfd *fds, int cnt, int timeout);
#endif
int                     w32_recv(int fd, char *buf, int len, int flags);
int                     w32_shutdown(int fd, int flags);

#ifndef WIN32_SOCKET_H_CLEAN
#define HOST_NOT_FOUND          WSAHOST_NOT_FOUND
#define NO_AGAIN                WSANO_AGAIN
#define NO_RECOVERY             WSANO_RECOVERY
#define NO_DATA                 WSANO_DATA
#define NO_ADDRESSS             -1

#undef  h_errno
#define h_errno                 w32_h_errno

#if !defined(_MSC_VER) || (_MSC_VER < 1400)
#define getaddrinfo(a,b,c,d)    w32_getaddrinfo(a,b,c,d)
#endif
#define gethostbyname(a)        w32_gethostbyname(a)
#define hstrerror(a)            w32_hstrerror(a)
#define herror(a)               w32_herror(a)

#define socket(a,b,c)           w32_socket(a,b,c)
#define connect(a,b,c)          w32_connect(a,b,c)
#define setsockopt(a,b,c,d,e)   w32_setsockopt(a,b,c,d,e)
#define getsockopt(a,b,c,d,e)   w32_getsockopt(a,b,c,d,e)
#define getpeername(a,b,c)      w32_getpeername(a,b,c)
#define bind(a,b,c)             w32_bind(a,b,c)
#define listen(a,b)             w32_listen(a,b)
#define accept(a,b,c)           w32_accept(a,b,c)
#define poll(a,b,c)             w32_poll(a,b,c)
#define recv(a,b,c,d)           w32_recv(a,b,c,d)
#define shutdown(a,b)           w32_shutdown(a,b)

#endif /*SOCKET_MAPCALLS*/

__END_DECLS

#endif /*GR_SOCKET_H_INCLUDED*/
