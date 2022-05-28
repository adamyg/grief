#ifndef GR_WIN32WSA_H_INCLUDED
#define GR_WIN32WSA_H_INCLUDED
/* -*- mode: c; indent-width: 4; -*- */
/* $Id: win32wsa.h,v 1.10 2022/05/25 15:45:31 cvsuser Exp $
 * WSA Error Codes
 * Reproduced from a number of sources.
 *
 *
 * Copyright (c) 1998 - 2017, Adam Young.
 * All rights reserved.
 *
 * This file is part of the GRIEF Editor.
 *
 * The GRIEF Editor is free software: you can redistribute it
 * and/or modify it under the terms of the GRIEF Editor License.
 *
 * Redistributions of source code must retain the above copyright
 * notice, and must be distributed with the license document above.
 *
 * Redistributions in binary form must reproduce the above copyright
 * notice, and must include the license document above in
 * the documentation and/or other materials provided with the
 * distribution.
 *
 * The GRIEF Editor is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * License for more details.
 * ==end==
 */

    { WSA_INVALID_HANDLE,           // 6
        "Specified event object handle is invalid"
        //
        //  An application attempts to use an event object, but the specified handle is not valid.
        //  Note that this error is returned by the operating system, so the error number may
        //  change in future releases of Windows.
        //
    },
    { WSA_NOT_ENOUGH_MEMORY,        // 8
        "Insufficient memory available"
        //  An application used a Windows Sockets function that directly maps to a Windows
        //  function. The Windows function is indicating a lack of required memory resources. Note
        //  that this error is returned by the operating system, so the error number may change in
        //  future releases of Windows.
    },
    { WSA_INVALID_PARAMETER,        // 87
        "One or more parameters are invalid"
        //  An application used a Windows Sockets function which directly maps to a Windows
        //  function. The Windows function is indicating a problem with one or more parameters.
        //  Note that this error is returned by the operating system, so the error number may
        //  change in future releases of Windows.
    },

#if defined(_MSC_VER) && defined(STRUNCATE)
    { STRUNCATE,                    // 80
        "A string operation resulted in a truncated string (Secure CRT)"
    },
#endif

#if defined(EADDRINUSE)
    { EADDRINUSE,                   // 100
        "Address already in use (POSIX.1)"
        },
    { EADDRNOTAVAIL,                // 101
        "Address not available (POSIX.1)"
        },
    { EAFNOSUPPORT,                 // 102
        "Address family not supported (POSIX.1)"
        },
    { EALREADY,                     // 103
        "Connection already in progress (POSIX.1)"
        },
    { EBADMSG,                      // 104
        "Bad message (POSIX.1)"
        },
    { ECANCELED,                    // 105
        "Operation canceled (POSIX.1)"
        },
    { ECONNABORTED,                 // 106
        "Connection aborted by network (POSIX.1)"
        },
    { ECONNREFUSED,                 // 107
        "Connection refused (POSIX.1)"
        },
    { ECONNRESET,                   // 108
        "Connection reset (POSIX.1)"
        },
    { EDESTADDRREQ,                 // 109
        "Destination address required (POSIX.1)"
        },
    { EHOSTUNREACH,                 // 110
        "Host unreachable (POSIX.1)"
        },
    { EIDRM,                        // 111
        "Identifier removed (POSIX.1)"
        },
    { EINPROGRESS,                  // 112
        "Operation in progress (POSIX.1)"
        },
    { EISCONN,                      // 113
        "Socket is connected (POSIX.1)"
        },
    { ELOOP,                        // 114
        "Too many levels of symbolic links (POSIX.1)"
        },
    { EMSGSIZE,                     // 115
        "Message too long (POSIX.1)"
        },
    { ENETDOWN,                     // 116
        "Network is down (POSIX.1)"
        },
    { ENETRESET,                    // 117
        "Connection aborted by network (POSIX.1)"
        },
    { ENETUNREACH,                  // 118
        "Network unreachable (POSIX.1)"
        },
    { ENOBUFS,                      // 119
        "No buffer space available (POSIX.1)"
        },
    { ENODATA,                      // 120
        "No message is available on the STREAM head read queue (POSIX.1)"
        },
    { ENOLINK,                      // 121
        "Link has been severed (POSIX.1)"
        },
    { ENOMSG,                       // 122
        "No message of the desired type (POSIX.1)"
        },
    { ENOPROTOOPT,                  // 123
        "Protocol not available (POSIX.1)"
        },
    { ENOSR,                        // 124
        "No STREAM resources (POSIX.1)"
         },
    { ENOSTR,                       // 125
        "Not a STREAM (POSIX.1)"
        },
    { ENOTCONN,                     // 126
        "The socket is not connected (POSIX.1)"
        },
    { ENOTRECOVERABLE,              // 127
        "State not recoverable (POSIX)"
        },
    { ENOTSOCK,                     // 128
        "Not a socket (POSIX.1)"
        },
    { ENOTSUP,                      // 129
        "Operation not supported (POSIX.1)"
        },
    { EOPNOTSUPP,                   // 130
        "Operation not supported on socket (POSIX.1)"
        },
    { EOTHER,                       // 131
        "Other (POSIX.1)"
        },
    { EOVERFLOW,                    // 132
        "Value too large to be stored in data type (POSIX.1)"
        },
    { EOWNERDEAD,                   // 133
        "Owner died (POSIX.1)"
        },
    { EPROTO,                       // 134
        "Protocol error (POSIX.1)"
        },
    { EPROTONOSUPPORT,              // 135
        "Protocol not supported (POSIX.1)"
        },
    { EPROTOTYPE,                   // 136
        "Protocol wrong type for socket (POSIX.1)"
        },
    { ETIME,                        // 137
        "Timer expired (POSIX.1)"
        },
    { ETIMEDOUT,                    // 138
        "Connection timed out (POSIX.1)"
        },
#if defined(ETXTBSY)
    { ETXTBSY,                      // 139
        "Text file busy (POSIX.1)"
        },
#endif
    { EWOULDBLOCK,                  // 140
        "Operation would block (POSIX.1)"
        },
#endif

    { WSA_OPERATION_ABORTED,        // 995
        "Overlapped operation aborted"
        //  An overlapped operation was canceled due to the closure of the socket, or the execution
        //  of the SIO_FLUSH command in WSAIoctl. Note that this error is returned by the operating
        //  system, so the error number may change in future releases of Windows.
    },
    { WSA_IO_INCOMPLETE,            // 996
        "Overlapped I/O event object not in signaled state"
        //  The application has tried to determine the status of an overlapped operation which is
        //  not yet completed. Applications that use WSAGetOverlappedResult (with the fWait flag
        //  set to FALSE) in a polling mode to determine when an overlapped operation has completed,
        //  get this error code until the operation is complete. Note that this error is returned
        //  by the operating system, so the error number may change in future releases of Windows.
    },
    { WSA_IO_PENDING,               // 997
        "Overlapped operations will complete later"
        //  The application has initiated an overlapped operation that cannot be completed
        //  immediately. A completion indication will be given later when the operation has been
        //  completed. Note that this error is returned by the operating system, so the error
        //  number may change in future releases of Windows.
    },
    { WSAEINTR,                     // 10004
        "Interrupted function call"
        // A blocking operation was interrupted by a call to WSACancelBlockingCall.
    },
    { WSAEBADF,                     // 10009
        "File handle is not valid"
        // The file handle supplied is not valid.
    },
    { WSAEACCES,                    // 10013
        "Permission denied"
        //  An attempt was made to access a socket in a way forbidden by its access permissions. An
        //  example is using a broadcast address for sendto without broadcast permission being set
        //  using setsockopt(SO_BROADCAST).
        //
        //  Another possible reason for the WSAEACCES error is that when the bind function is
        //  called (on Windows NT 4.0 with SP4 and later), another application, service, or kernel
        //  mode driver is bound to the same address with exclusive access. Such exclusive access
        //  is a new feature of Windows NT 4.0 with SP4 and later, and is implemented by using the
        //  SO_EXCLUSIVEADDRUSE option.
    },
    { WSAEFAULT,                    // 10014
        "Bad address"
        //  The system detected an invalid pointer address in attempting to use a pointer argument
        //  of a call. This error occurs if an application passes an invalid pointer value, or if
        //  the length of the buffer is too small. For instance, if the length of an argument,
        //  which is a sockaddr structure, is smaller than the sizeof(sockaddr).
    },
    { WSAEINVAL,                    // 10022
        "Invalid argument"
        // Some invalid argument was supplied (for example, specifying an invalid level to the
        // setsockopt function). In some instances, it also refers to the current state of the
        // socketfor instance, calling accept on a socket that is not listening.
    },
    { WSAEMFILE,                    // 10024
        "Too many open files"
        //  Too many open sockets. Each implementation may have a maximum number of socket handles
        //  available, either globally, per process, or per thread.
    },
    { WSAEWOULDBLOCK,               // 10035
        "Resource temporarily unavailable"
        //  This error is returned from operations on nonblocking sockets that cannot be completed
        //  immediately, for example recv when no data is queued to be read from the socket. It is
        //  a nonfatal error, and the operation should be retried later. It is normal for
        //  WSAEWOULDBLOCK to be reported as the result from calling connect on a nonblocking
        //  SOCK_STREAM socket, since some time must elapse for the connection to be established.
    },
    { WSAEINPROGRESS,               // 10036
        "Operation now in progress"
        //  A blocking operation is currently executing. Windows Sockets only allows a single
        //  blocking operationper- task or threadto be outstanding, and if any other function call
        //  is made (whether or not it references that or any other socket) the function fails with
        //  the WSAEINPROGRESS error.
    },
    { WSAEALREADY,                  // 10037
        "Operation already in progress"
        //  An operation was attempted on a nonblocking socket with an operation already in
        //  progressthat is, calling connect a second time on a nonblocking socket that is already
        //  connecting, or canceling an asynchronous request (WSAAsyncGetXbyY) that has already
        //  been canceled or completed.
    },
    { WSAENOTSOCK,                  // 10038
        "Socket operation on nonsocket"
        //  An operation was attempted on something that is not a socket. Either the socket handle
        //  parameter did not reference a valid socket, or for select, a member of an fd_set was
        //  not valid.
    },
    { WSAEDESTADDRREQ,              // 10039
        "Destination address required"
        //  A required address was omitted from an operation on a socket. For example, this error
        //  is returned if sendto is called with the remote address of ADDR_ANY.
    },
    { WSAEMSGSIZE,                  // 10040
        "Message too long"
        //  A message sent on a datagram socket was larger than the internal message buffer or some
        //  other network limit, or the buffer used to receive a datagram was smaller than the
        //  datagram itself.
    },
    { WSAEPROTOTYPE,                // 10041
        "Protocol wrong type for socket"
        //  A protocol was specified in the socket function call that does not support the
        //  semantics of the socket type requested. For example, the ARPA Internet UDP protocol
        //  cannot be specified with a socket type of SOCK_STREAM.
    },
    { WSAENOPROTOOPT,               // 10042
        "Bad protocol option"
        //  An unknown, invalid or unsupported option or level was specified in a getsockopt or
        //  setsockopt call.
    },
    { WSAEPROTONOSUPPORT,           // 10043
        "Protocol not supported"
        //  The requested protocol has not been configured into the system, or no implementation
        //  for it exists. For example, a socket call requests a SOCK_DGRAM socket, but specifies a
        //  stream protocol.
    },
    { WSAESOCKTNOSUPPORT,           // 10044
        "Socket type not supported"
        //  The support for the specified socket type does not exist in this address family. For
        //  example, the optional type SOCK_RAW might be selected in a socket call, and the
        //  implementation does not support SOCK_RAW sockets at all.
    },
    { WSAEOPNOTSUPP,                // 10045
        "Operation not supported"
        //  The attempted operation is not supported for the type of object referenced. Usually
        //  this occurs when a socket descriptor to a socket that cannot support this operation is
        //  trying to accept a connection on a datagram socket.
    },
    { WSAEPFNOSUPPORT,              // 10046
        "Protocol family not supported"
        // The protocol family has not been configured into the system or no implementation for it
        // exists. This message has a slightly different meaning from WSAEAFNOSUPPORT. However, it
        // is interchangeable in most cases, and all Windows Sockets functions that return one of
        // these messages also specify WSAEAFNOSUPPORT.
    },
    { WSAEAFNOSUPPORT,              // 10047
        "Address family not supported by protocol family"
        //  An address incompatible with the requested protocol was used. All sockets are created
        //  with an associated address family (that is, AF_INET for Internet Protocols) and a
        //  generic protocol type (that is, SOCK_STREAM). This error is returned if an incorrect
        //  protocol is explicitly requested in the socket call, or if an address of the wrong
        //  family is used for a socket, for example, in sendto.
    },
    { WSAEADDRINUSE,                // 10048
        "Address already in use"
        //  Typically, only one usage of each socket address (protocol/IP address/port) is
        //  permitted. This error occurs if an application attempts to bind a socket to an IP
        //  address/port that has already been used for an existing socket, or a socket that was
        //  not closed properly, or one that is still in the process of closing. For server
        //  applications that need to bind multiple sockets to the same port number, consider using
        //  setsockopt (SO_REUSEADDR). Client applications usually need not call bind at allconnect
        //  chooses an unused port automatically. When bind is called with a wildcard address
        //  (involving ADDR_ANY), a WSAEADDRINUSE error could be delayed until the specific address
        //  is committed. This could happen with a call to another function later, including
        //  connect, listen, WSAConnect, or WSAJoinLeaf.
    },
    { WSAEADDRNOTAVAIL,             // 10049
        "Cannot assign requested address"
        //  The requested address is not valid in its context. This normally results from an
        //  attempt to bind to an address that is not valid for the local computer. This can also
        //  result from connect, sendto, WSAConnect, WSAJoinLeaf, or WSASendTo when the remote
        //  address or port is not valid for a remote computer (for example, address or port 0).
    },
    { WSAENETDOWN,                  // 10050
        "Network is down"
        //  A socket operation encountered a dead network. This could indicate a serious failure of
        //  the network system (that is, the protocol stack that the Windows Sockets DLL runs over),
        //  the network interface, or the local network itself.
    },
    { WSAENETUNREACH,               // 10051
        "Network is unreachable"
        //  A socket operation was attempted to an unreachable network. This usually means the
        //  local software knows no route to reach the remote host.
    },
    { WSAENETRESET,                 // 10052
        "Network dropped connection on reset"
        //  The connection has been broken due to keep-alive activity detecting a failure while the
        //  operation was in progress. It can also be returned by setsockopt if an attempt is made
        // to set SO_KEEPALIVE on a connection that has already failed.
    },
    { WSAECONNABORTED,              // 10053
        "Software caused connection abort"
        //  An established connection was aborted by the software in your host computer, possibly
        //  due to a data transmission time-out or protocol error.
    },
    { WSAECONNRESET,                // 10054
        "Connection reset by peer"
        //  An existing connection was forcibly closed by the remote host. This normally results if
        //  the peer application on the remote host is suddenly stopped, the host is rebooted, the
        //  host or remote network interface is disabled, or the remote host uses a hard close (see
        //  setsockopt for more information on the SO_LINGER option on the remote socket). This
        //  error may also result if a connection was broken due to keep-alive activity detecting a
        //  failure while one or more operations are in progress. Operations that were in progress
        //  fail with WSAENETRESET. Subsequent operations fail with WSAECONNRESET.
    },
    { WSAENOBUFS,                   // 10055
        "No buffer space available"
        //  An operation on a socket could not be performed because the system lacked sufficient
        //  buffer space or because a queue was full.
    },
    { WSAEISCONN,                   // 10056
        "Socket is already connected"
        //  A connect request was made on an already-connected socket. Some implementations also
        //  return this error if sendto is called on a connected SOCK_DGRAM socket (for SOCK_STREAM
        //  sockets, the to parameter in sendto is ignored) although other implementations treat
        //  this as a legal occurrence.
    },
    { WSAENOTCONN,                  // 10057
        "Socket is not connected"
        //  A request to send or receive data was disallowed because the socket is not connected
        //  and (when sending on a datagram socket using sendto) no address was supplied. Any other
        //  type of operation might also return this errorfor example, setsockopt setting
        //  SO_KEEPALIVE if the connection has been reset.
    },
    { WSAESHUTDOWN,                 // 10058
        "Cannot send after socket shutdown"
        //  A request to send or receive data was disallowed because the socket had already been
        //  shut down in that direction with a previous shutdown call. By calling shutdown a
        //  partial close of a socket is requested, which is a signal that sending or receiving, or
        //  both have been discontinued.
    },
    { WSAETOOMANYREFS,              // 10059
        "Too many references"
        //  Too many references to some kernel object.
    },
    { WSAETIMEDOUT,                 // 10060
        "Connection timed out"
        //  A connection attempt failed because the connected party did not properly respond after
        //  a period of time, or the established connection failed because the connected host has
        //  failed to respond.
    },
    { WSAECONNREFUSED,              // 10061
        "Connection refused"
        //  No connection could be made because the target computer actively refused it. This
        //  usually results from trying to connect to a service that is inactive on the foreign
        //  hostthat is, one with no server application running.
    },
    { WSAELOOP,                     // 10062
        "Cannot translate name"
        //  Cannot translate a name.
    },
    { WSAENAMETOOLONG,              // 10063
        "Name too long"
        //  A name component or a name was too long.
    },
    { WSAEHOSTDOWN,                 // 10064
        "Host is down"
        //  A socket operation failed because the destination host is down. A socket operation
        //  encountered a dead host. Networking activity on the local host has not been initiated.
        //  These conditions are more likely to be indicated by the error WSAETIMEDOUT"
    },
    { WSAEHOSTUNREACH,              // 10065
        "No route to host"
        //  A socket operation was attempted to an unreachable host. See WSAENETUNREACH.
    },
    { WSAENOTEMPTY,                 // 10066
        "Directory not empty"
        //  Cannot remove a directory that is not empty.
    },
    { WSAEPROCLIM,                  // 10067
        "Too many processes"
        //  A Windows Sockets implementation may have a limit on the number of applications that
        //  can use it simultaneously. WSAStartup may fail with this error if the limit has been
        //  reached.
    },
    { WSAEUSERS,                    // 10068
        "User quota exceeded"
        //  Ran out of user quota.
    },
    { WSAEDQUOT,                    // 10069
        "Disk quota exceeded"
        //  Ran out of disk quota.
    },
    { WSAESTALE,                    // 10070
        "Stale file handle reference"
        //  The file handle reference is no longer available.
    },
    { WSAEREMOTE,                   // 10071
        "Item is remote"
        //  The item is not available locally.
    },
    { WSASYSNOTREADY,               // 10091
        "Network subsystem is unavailable"
        //  This error is returned by WSAStartup if the Windows Sockets implementation cannot
        //  function at this time because the underlying system it uses to provide network services
        //  is currently unavailable. Users should check:
        //
        //  That the appropriate Windows Sockets DLL file is in the current path. That they are not
        //  trying to use more than one Windows Sockets implementation simultaneously. If there is
        //  more than one Winsock DLL on your system, be sure the first one in the path is
        //  appropriate for the network subsystem currently loaded. The Windows Sockets
        //  implementation documentation to be sure all necessary components are currently
        //  installed and configured correctly.
    },
    { WSAVERNOTSUPPORTED,           // 10092
        "Winsock.dll version out of range"
        //  The current Windows Sockets implementation does not support the Windows Sockets
        //  specification version requested by the application. Check that no old Windows Sockets
        //  DLL files are being accessed.
    },
    { WSANOTINITIALISED,            // 10093
        "Successful WSAStartup not yet performed"
        //  Either the application has not called WSAStartup or WSAStartup failed. The application
        //  may be accessing a socket that the current active task does not own (that is, trying to
        //  share a socket between tasks), or WSACleanup has been called too many times.
    },
    { WSAEDISCON,                   // 10101
        "Graceful shutdown in progress"
        //  Returned by WSARecv and WSARecvFrom to indicate that the remote party has initiated a
        //  graceful shutdown sequence.
    },
    { WSAENOMORE,                   // 10102
        "No more results"
        //  No more results can be returned by the WSALookupServiceNext function.
    },
    { WSAECANCELLED,                // 10103
        "Call has been canceled"
        //  A call to the WSALookupServiceEnd function was made while this call was still
        //  processing. The call has been canceled.
    },
    { WSAEINVALIDPROCTABLE,         // 10104
        "Procedure call table is invalid"
        //  The service provider procedure call table is invalid. A service provider returned a
        //  bogus procedure table to Ws2_32.dll. This is usually caused by one or more of the
        //  function pointers being NULL.
    },
    { WSAEINVALIDPROVIDER,          // 10105
        "Service provider is invalid"
        //  The requested service provider is invalid. This error is returned by the
        //  WSCGetProviderInfo and WSCGetProviderInfo32 functions if the protocol entry specified
        //  could not be found. This error is also returned if the service provider returned a
        //  version number other than 2.0.
    },
    { WSAEPROVIDERFAILEDINIT,       // 10106
        "Service provider failed to initialize"
        //  The requested service provider could not be loaded or initialized. This error is
        //  returned if either a service provider's DLL could not be loaded (LoadLibrary failed) or
        //  the provider's WSPStartup or NSPStartup function failed.
    },
    { WSASYSCALLFAILURE,            // 10107
        "System call failure"
        //  A system call that should never fail has failed. This is a generic error code, returned
        //  under various conditions.
        //
        //  Returned when a system call that should never fail does fail. For example, if a call to
        //  WaitForMultipleEvents fails or one of the registry functions fails trying to manipulate
        //  the protocol/namespace catalogs.
        //
        //  Returned when a provider does not return SUCCESS and does not provide an extended error
        //  code. Can indicate a service provider implementation error.
    },
    { WSASERVICE_NOT_FOUND,         // 10108
        "Service not found"
        //  No such service is known. The service cannot be found in the specified name space.
    },
    { WSATYPE_NOT_FOUND,            // 10109
        "Class type not found"
        //  The specified class was not found.
    },
    { WSA_E_NO_MORE,                // 10110
        "No more results"
        //  No more results can be returned by the WSALookupServiceNext function.
    },
    { WSA_E_CANCELLED,              // 10111
        "Call was canceled"
        //  A call to the WSALookupServiceEnd function was made while this call was still
        //  processing. The call has been canceled.
    },
    { WSAEREFUSED,                  // 10112
        "Database query was refused"
        //  A database query failed because it was actively refused.
    },
    { WSAHOST_NOT_FOUND,            // 11001
        "Host not found"
        //  No such host is known. The name is not an official host name or alias, or it cannot be
        //  found in the database(s) being queried. This error may also be returned for protocol
        //  and service queries, and means that the specified name could not be found in the
        //  relevant database.
    },
    { WSATRY_AGAIN,                 // 11002
        "Nonauthoritative host not found"
        //  This is usually a temporary error during host name resolution and means that the local
        //  server did not receive a response from an authoritative server. A retry at some time
        //  later may be successful.
    },
    { WSANO_RECOVERY,               // 11003
        "This is a nonrecoverable error"
        //  This indicates that some sort of nonrecoverable error occurred during a database
        //  lookup. This may be because the database files (for example, BSD-compatible HOSTS,
        //  SERVICES, or PROTOCOLS files) could not be found, or a DNS request was returned by the
        //  server with a severe error.
    },
    { WSANO_DATA,                   // 11004
        "Valid name, no data record of requested type"
        //  The requested name is valid and was found in the database, but it does not have the
        //  correct associated data being resolved for. The usual example for this is a host
        //  name-to-address translation attempt (using gethostbyname or WSAAsyncGetHostByName)
        //  which uses the DNS (Domain Name Server). An MX record is returned but no A
        //  recordindicating the host itself exists, but is not directly reachable.
    },
    { WSA_QOS_RECEIVERS,            // 11005
        "QoS receivers"
        //  At least one QoS reserve has arrived.
    },
    { WSA_QOS_SENDERS,              // 11006
        "QoS senders"
        //  At least one QoS send path has arrived.
    },
    { WSA_QOS_NO_SENDERS,           // 11007
        "No QoS senders"
        //  There are no QoS senders.
    },
    { WSA_QOS_NO_RECEIVERS,         // 11008
        "QoS no receivers"
        //  There are no QoS receivers.
    },
    { WSA_QOS_REQUEST_CONFIRMED,    // 11009
        "QoS request confirmed"
        //  The QoS reserve request has been confirmed.
    },
    { WSA_QOS_ADMISSION_FAILURE,    // 11010
        "QoS admission error"
        //  A QoS error occurred due to lack of resources.
    },
    { WSA_QOS_POLICY_FAILURE,       // 11011
        "QoS policy failure"
        //  The QoS request was rejected because the policy system couldnt allocate the requested
        //  resource within the existing policy.
    },
    { WSA_QOS_BAD_STYLE,            // 11012
        "QoS bad style"
        //  An unknown or conflicting QoS style was encountered.
    },
    { WSA_QOS_BAD_OBJECT,           // 11013
        "QoS bad object"
        //  A problem was encountered with some part of the filterspec or the provider-specific
        //  buffer in general.
    },
    { WSA_QOS_TRAFFIC_CTRL_ERROR,   // 11014
        "QoS traffic control error"
        //  An error with the underlying traffic control (TC) API as the generic QoS request was
        //  converted for local enforcement by the TC API. This could be due to an out of memory
        //  error or to an internal QoS provider error.
    },
    { WSA_QOS_GENERIC_ERROR,        // 11015
        "QoS generic error"
        //  A general QoS error.
    },
    { WSA_QOS_ESERVICETYPE,         // 11016
        "QoS service type error"
        //  An invalid or unrecognized service type was found in the QoS flowspec.
    },
    { WSA_QOS_EFLOWSPEC,            // 11017
        "QoS flowspec error"
        //  An invalid or inconsistent flowspec was found in the QOS structure.
    },
    { WSA_QOS_EPROVSPECBUF,         // 11018
        "Invalid QoS provider buffer"
        //  An invalid QoS provider-specific buffer.
    },
    { WSA_QOS_EFILTERSTYLE,         // 11019
        "Invalid QoS filter style"
        //  An invalid QoS filter style was used.
    },
    { WSA_QOS_EFILTERTYPE,          // 11020
        "Invalid QoS filter type"
        //  An invalid QoS filter type was used.
    },
    { WSA_QOS_EFILTERCOUNT,         // 11021
        "Incorrect QoS filter count"
        //  An incorrect number of QoS FILTERSPECs were specified in the FLOWDESCRIPTOR.
    },
    { WSA_QOS_EOBJLENGTH,           // 11022
        "Invalid QoS object length"
        //  An object with an invalid ObjectLength field was specified in the QoS provider-specific buffer.
    },
    { WSA_QOS_EFLOWCOUNT,           // 11023
        "Incorrect QoS flow count"
        //  An incorrect number of flow descriptors was specified in the QoS structure.
    },
#if defined(WSA_QOS_EUNKNOWNPSOBJ)
    { WSA_QOS_EUNKNOWNPSOBJ,        // 11024
        "Unrecognized QoS object"
        //  An unrecognized object was found in the QoS provider-specific buffer.
    },
#elif defined(WSA_QOS_EUNKOWNPSOBJ)
    { WSA_QOS_EUNKOWNPSOBJ,
        "Unrecognized QoS object"
        //  An unrecognized object was found in the QoS provider-specific buffer.
    },
#endif
    { WSA_QOS_EPOLICYOBJ,           // 11025
        "Invalid QoS policy object"
        // An invalid policy object was found in the QoS provider-specific buffer.
    },
    { WSA_QOS_EFLOWDESC,            // 11026
        "Invalid QoS flow descriptor"
        //  An invalid QoS flow descriptor was found in the flow descriptor list.
    },
    { WSA_QOS_EPSFLOWSPEC,          // 11027
        "Invalid QoS provider-specific flowspec"
        //  An invalid or inconsistent flowspec was found in the QoS provider-specific buffer.
    },
    { WSA_QOS_EPSFILTERSPEC,        // 11028
        "Invalid QoS provider-specific filterspec"
        //  An invalid FILTERSPEC was found in the QoS provider-specific buffer.
    },
    { WSA_QOS_ESDMODEOBJ,           // 11029
        "Invalid QoS shape discard mode object"
        //  An invalid shape discard mode object was found in the QoS provider-specific buffer.
    },
    { WSA_QOS_ESHAPERATEOBJ,        // 11030
        "Invalid QoS shaping rate object"
        //  An invalid shaping rate object was found in the QoS provider-specific buffer.
    },
    { WSA_QOS_RESERVED_PETYPE,      // 11031
        "Reserved policy QoS element type"
        //  A reserved policy element was found in the QoS provider-specific buffer.
    }

#endif  /*GR_WIN32WSA_H_INCLUDED*/
/*end*/
