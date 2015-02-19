-*- mode: ansi; -*-

Topic:          libfetch

        libfetch provides a high-level interface for retreiving and
        uploading files using Uniform Resource Locators (URLs).

        The library implements:

           * local file access (file://)
           * FTP
           * HTTP
           * HTTPS (optional, using OpenSSL)

        FTP and HTTP proxies can be used.

   Source:

        The version is a merge of the current NetBSD and new features
        from the leading FreeBSD (http/md5) implementation.

        Additional enhancments include 
        
           * SYST
           * FEAT
           * MLST
           * MLSD
           * LIST
           * url_stat mode.

        http://www.freebsd.org/cgi/cvsweb.cgi/src/lib/libfetch/
        http://ftp.netbsd.org/pub/pkgsrc/current/pkgsrc/net/libfetch/

 Copyright:

>   Copyright (c) 1998-2011 Dag-Erling Smørgrav
>   Copyright (c) 2008, 2010 Joerg Sonnenberger <joerg@NetBSD.org>
>   All rights reserved.
>       
>   Redistribution and use in source and binary forms, with or without
>   modification, are permitted provided that the following conditions
>   are met:
>   1. Redistributions of source code must retain the above copyright
>      notice, this list of conditions and the following disclaimer
>      in this position and unchanged.
>   2. Redistributions in binary form must reproduce the above copyright
>      notice, this list of conditions and the following disclaimer in the
>      documentation and/or other materials provided with the distribution.
>   3. The name of the author may not be used to endorse or promote products
>      derived from this software without specific prior written permission
>       
>   THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
>   IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
>   OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
>   IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
>   INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
>   NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
>   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
>   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
>   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
>   THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

documentation:

   FETCH(3)                 BSD Library Functions Manual                 FETCH(3)

   [1mNAME[0m
        [1mfetchMakeURL[22m, [1mfetchParseURL[22m, [1mfetchFreeURL[22m, [1mfetchXGetURL[22m, [1mfetchGetURL[22m,
        [1mfetchPutURL[22m, [1mfetchStatURL[22m, [1mfetchListURL[22m, [1mfetchXGet[22m, [1mfetchGet[22m, [1mfetchPut[22m,
        [1mfetchStat[22m, [1mfetchList[22m, [1mfetchXGetFile[22m, [1mfetchGetFile[22m, [1mfetchPutFile[22m,
        [1mfetchStatFile[22m, [1mfetchListFile[22m, [1mfetchXGetHTTP[22m, [1mfetchGetHTTP[22m, [1mfetchPutHTTP[22m,
        [1mfetchStatHTTP[22m, [1mfetchListHTTP[22m, [1mfetchXGetFTP[22m, [1mfetchGetFTP[22m, [1mfetchPutFTP[22m,
        [1mfetchStatFTP[22m, [1mfetchListFTP [22m— file transfer functions

   [1mLIBRARY[0m
        library “libfetch”

   [1mSYNOPSIS[0m
        [1m#include <sys/param.h>[0m
        [1m#include <stdio.h>[0m
        [1m#include <fetch.h>[0m

        [4mstruct[24m [4murl[24m [4m*[0m
        [1mfetchMakeURL[22m([4mconst[24m [4mchar[24m [4m*scheme[24m, [4mconst[24m [4mchar[24m [4m*host[24m, [4mint[24m [4mport[24m,
            [4mconst[24m [4mchar[24m [4m*doc[24m, [4mconst[24m [4mchar[24m [4m*user[24m, [4mconst[24m [4mchar[24m [4m*pwd[24m);

        [4mstruct[24m [4murl[24m [4m*[0m
        [1mfetchParseURL[22m([4mconst[24m [4mchar[24m [4m*URL[24m);

        [4mvoid[0m
        [1mfetchFreeURL[22m([4mstruct[24m [4murl[24m [4m*u[24m);

        [4mFILE[24m [4m*[0m
        [1mfetchXGetURL[22m([4mconst[24m [4mchar[24m [4m*URL[24m, [4mstruct[24m [4murl_stat[24m [4m*us[24m, [4mconst[24m [4mchar[24m [4m*flags[24m);

        [4mFILE[24m [4m*[0m
        [1mfetchGetURL[22m([4mconst[24m [4mchar[24m [4m*URL[24m, [4mconst[24m [4mchar[24m [4m*flags[24m);

        [4mFILE[24m [4m*[0m
        [1mfetchPutURL[22m([4mconst[24m [4mchar[24m [4m*URL[24m, [4mconst[24m [4mchar[24m [4m*flags[24m);

        [4mint[0m
        [1mfetchStatURL[22m([4mconst[24m [4mchar[24m [4m*URL[24m, [4mstruct[24m [4murl_stat[24m [4m*us[24m, [4mconst[24m [4mchar[24m [4m*flags[24m);

        [4mstruct[24m [4murl_ent[24m [4m*[0m
        [1mfetchListURL[22m([4mconst[24m [4mchar[24m [4m*URL[24m, [4mconst[24m [4mchar[24m [4m*flags[24m);

        [4mFILE[24m [4m*[0m
        [1mfetchXGet[22m([4mstruct[24m [4murl[24m [4m*u[24m, [4mstruct[24m [4murl_stat[24m [4m*us[24m, [4mconst[24m [4mchar[24m [4m*flags[24m);

        [4mFILE[24m [4m*[0m
        [1mfetchGet[22m([4mstruct[24m [4murl[24m [4m*u[24m, [4mconst[24m [4mchar[24m [4m*flags[24m);

        [4mFILE[24m [4m*[0m
        [1mfetchPut[22m([4mstruct[24m [4murl[24m [4m*u[24m, [4mconst[24m [4mchar[24m [4m*flags[24m);

        [4mint[0m
        [1mfetchStat[22m([4mstruct[24m [4murl[24m [4m*u[24m, [4mstruct[24m [4murl_stat[24m [4m*us[24m, [4mconst[24m [4mchar[24m [4m*flags[24m);

        [4mstruct[24m [4murl_ent[24m [4m*[0m
        [1mfetchList[22m([4mstruct[24m [4murl[24m [4m*u[24m, [4mconst[24m [4mchar[24m [4m*flags[24m);

        [4mFILE[24m [4m*[0m
        [1mfetchXGetFile[22m([4mstruct[24m [4murl[24m [4m*u[24m, [4mstruct[24m [4murl_stat[24m [4m*us[24m, [4mconst[24m [4mchar[24m [4m*flags[24m);

        [4mFILE[24m [4m*[0m
        [1mfetchGetFile[22m([4mstruct[24m [4murl[24m [4m*u[24m, [4mconst[24m [4mchar[24m [4m*flags[24m);

        [4mFILE[24m [4m*[0m
        [1mfetchPutFile[22m([4mstruct[24m [4murl[24m [4m*u[24m, [4mconst[24m [4mchar[24m [4m*flags[24m);

        [4mint[0m
        [1mfetchStatFile[22m([4mstruct[24m [4murl[24m [4m*u[24m, [4mstruct[24m [4murl_stat[24m [4m*us[24m, [4mconst[24m [4mchar[24m [4m*flags[24m);

        [4mstruct[24m [4murl_ent[24m [4m*[0m
        [1mfetchListFile[22m([4mstruct[24m [4murl[24m [4m*u[24m, [4mconst[24m [4mchar[24m [4m*flags[24m);

        [4mFILE[24m [4m*[0m
        [1mfetchXGetHTTP[22m([4mstruct[24m [4murl[24m [4m*u[24m, [4mstruct[24m [4murl_stat[24m [4m*us[24m, [4mconst[24m [4mchar[24m [4m*flags[24m);

        [4mFILE[24m [4m*[0m
        [1mfetchGetHTTP[22m([4mstruct[24m [4murl[24m [4m*u[24m, [4mconst[24m [4mchar[24m [4m*flags[24m);

        [4mFILE[24m [4m*[0m
        [1mfetchPutHTTP[22m([4mstruct[24m [4murl[24m [4m*u[24m, [4mconst[24m [4mchar[24m [4m*flags[24m);

        [4mint[0m
        [1mfetchStatHTTP[22m([4mstruct[24m [4murl[24m [4m*u[24m, [4mstruct[24m [4murl_stat[24m [4m*us[24m, [4mconst[24m [4mchar[24m [4m*flags[24m);

        [4mstruct[24m [4murl_ent[24m [4m*[0m
        [1mfetchListHTTP[22m([4mstruct[24m [4murl[24m [4m*u[24m, [4mconst[24m [4mchar[24m [4m*flags[24m);

        [4mFILE[24m [4m*[0m
        [1mfetchXGetFTP[22m([4mstruct[24m [4murl[24m [4m*u[24m, [4mstruct[24m [4murl_stat[24m [4m*us[24m, [4mconst[24m [4mchar[24m [4m*flags[24m);

        [4mFILE[24m [4m*[0m
        [1mfetchGetFTP[22m([4mstruct[24m [4murl[24m [4m*u[24m, [4mconst[24m [4mchar[24m [4m*flags[24m);

        [4mFILE[24m [4m*[0m
        [1mfetchPutFTP[22m([4mstruct[24m [4murl[24m [4m*u[24m, [4mconst[24m [4mchar[24m [4m*flags[24m);

        [4mint[0m
        [1mfetchStatFTP[22m([4mstruct[24m [4murl[24m [4m*u[24m, [4mstruct[24m [4murl_stat[24m [4m*us[24m, [4mconst[24m [4mchar[24m [4m*flags[24m);

        [4mstruct[24m [4murl_ent[24m [4m*[0m
        [1mfetchListFTP[22m([4mstruct[24m [4murl[24m [4m*u[24m, [4mconst[24m [4mchar[24m [4m*flags[24m);

   [1mDESCRIPTION[0m
        These functions implement a high-level library for retrieving and upload‐
        ing files using Uniform Resource Locators (URLs).

        [1mfetchParseURL[22m() takes a URL in the form of a null-terminated string and
        splits it into its components function according to the Common Internet
        Scheme Syntax detailed in RFC1738.  A regular expression which produces
        this syntax is:

            <scheme>:(//(<user>(:<pwd>)?@)?<host>(:<port>)?)?/(<document>)?

        If the URL does not seem to begin with a scheme name, the following syn‐
        tax is assumed:

            ((<user>(:<pwd>)?@)?<host>(:<port>)?)?/(<document>)?

        Note that some components of the URL are not necessarily relevant to all
        URL schemes.  For instance, the file scheme only needs the <scheme> and
        <document> components.

        [1mfetchMakeURL[22m() and [1mfetchParseURL[22m() return a pointer to a [4murl[24m structure,
        which is defined as follows in <[4mfetch.h[24m>:

            #define URL_SCHEMELEN 16
            #define URL_USERLEN 256
            #define URL_PWDLEN 256

            struct url {
                char         scheme[URL_SCHEMELEN+1];
                char         user[URL_USERLEN+1];
                char         pwd[URL_PWDLEN+1];
                char         host[MAXHOSTNAMELEN+1];
                int          port;
                char        *doc;
                off_t        offset;
                size_t       length;
                time_t       ims_time;
            };

        The [4mims_time[24m field stores the time value for If-Modified-Since HTTP
        requests.

        The pointer returned by [1mfetchMakeURL[22m() or [1mfetchParseURL[22m() should be freed
        using [1mfetchFreeURL[22m().

        [1mfetchXGetURL[22m(), [1mfetchGetURL[22m(), and [1mfetchPutURL[22m() constitute the recom‐
        mended interface to the [1mfetch [22mlibrary.  They examine the URL passed to
        them to determine the transfer method, and call the appropriate lower-
        level functions to perform the actual transfer.  [1mfetchXGetURL[22m() also
        returns the remote document's metadata in the [4murl_stat[24m structure pointed
        to by the [4mus[24m argument.

        The [4mflags[24m argument is a string of characters which specify transfer
        options.  The meaning of the individual flags is scheme-dependent, and is
        detailed in the appropriate section below.

        [1mfetchStatURL[22m() attempts to obtain the requested document's metadata and
        fill in the structure pointed to by its second argument.  The [4murl_stat[0m
        structure is defined as follows in <[4mfetch.h[24m>:

            struct url_stat {
                off_t        size;
                time_t       atime;
                time_t       mtime;
            };

        If the size could not be obtained from the server, the [4msize[24m field is set
        to -1.  If the modification time could not be obtained from the server,
        the [4mmtime[24m field is set to the epoch.  If the access time could not be
        obtained from the server, the [4matime[24m field is set to the modification
        time.

        [1mfetchListURL[22m() attempts to list the contents of the directory pointed to
        by the URL provided.  If successful, it returns a malloced array of
        [4murl_ent[24m structures.  The [4murl_ent[24m structure is defined as follows in
        <[4mfetch.h[24m>:

            struct url_ent {
                char         name[PATH_MAX];
                struct url_stat stat;
            };

        The list is terminated by an entry with an empty name.

        The pointer returned by [1mfetchListURL[22m() should be freed using [1mfree[22m().

        [1mfetchXGet[22m(), [1mfetchGet[22m(), [1mfetchPut[22m() and [1mfetchStat[22m() are similar to
        [1mfetchXGetURL[22m(), [1mfetchGetURL[22m(), [1mfetchPutURL[22m() and [1mfetchStatURL[22m(), except
        that they expect a pre-parsed URL in the form of a pointer to a [4mstruct[0m
        [4murl[24m rather than a string.

        All of the [1mfetchXGetXXX[22m(), [1mfetchGetXXX[22m() and [1mfetchPutXXX[22m() functions
        return a pointer to a stream which can be used to read or write data from
        or to the requested document, respectively.  Note that although the
        implementation details of the individual access methods vary, it can gen‐
        erally be assumed that a stream returned by one of the [1mfetchXGetXXX[22m() or
        [1mfetchGetXXX[22m() functions is read-only, and that a stream returned by one
        of the [1mfetchPutXXX[22m() functions is write-only.

   [1mFILE SCHEME[0m
        [1mfetchXGetFile[22m(), [1mfetchGetFile[22m() and [1mfetchPutFile[22m() provide access to doc‐
        uments which are files in a locally mounted file system.  Only the <docu‐
        ment> component of the URL is used.

        [1mfetchXGetFile[22m() and [1mfetchGetFile[22m() do not accept any flags.

        [1mfetchPutFile[22m() accepts the ‘a’ (append to file) flag.  If that flag is
        specified, the data written to the stream returned by [1mfetchPutFile[22m() will
        be appended to the previous contents of the file, instead of replacing
        them.

   [1mFTP SCHEME[0m
        [1mfetchXGetFTP[22m(), [1mfetchGetFTP[22m() and [1mfetchPutFTP[22m() implement the FTP proto‐
        col as described in RFC959.

        If the ‘p’ (passive) flag is specified, a passive (rather than active)
        connection will be attempted.

        If the ‘l’ (low) flag is specified, data sockets will be allocated in the
        low (or default) port range instead of the high port range (see ip(4)).

        If the ‘d’ (direct) flag is specified, [1mfetchXGetFTP[22m(), [1mfetchGetFTP[22m() and
        [1mfetchPutFTP[22m() will use a direct connection even if a proxy server is
        defined.

        If no user name or password is given, the [1mfetch [22mlibrary will attempt an
        anonymous login, with user name "anonymous" and password "anony‐
        mous@<hostname>".

   [1mHTTP SCHEME[0m
        The [1mfetchXGetHTTP[22m(), [1mfetchGetHTTP[22m() and [1mfetchPutHTTP[22m() functions imple‐
        ment the HTTP/1.1 protocol.  With a little luck, there is even a chance
        that they comply with RFC2616 and RFC2617.

        If the ‘d’ (direct) flag is specified, [1mfetchXGetHTTP[22m(), [1mfetchGetHTTP[22m()
        and [1mfetchPutHTTP[22m() will use a direct connection even if a proxy server is
        defined.

        If the ‘i’ (if-modified-since) flag is specified, and the [4mims_time[24m field
        is set in [4mstruct[24m [4murl[24m, then [1mfetchXGetHTTP[22m() and [1mfetchGetHTTP[22m() will send a
        conditional If-Modified-Since HTTP header to only fetch the content if it
        is newer than [4mims_time[24m.

        Since there seems to be no good way of implementing the HTTP PUT method
        in a manner consistent with the rest of the [1mfetch [22mlibrary, [1mfetchPutHTTP[22m()
        is currently unimplemented.

   [1mAUTHENTICATION[0m
        Apart from setting the appropriate environment variables and specifying
        the user name and password in the URL or the [4mstruct[24m [4murl[24m, the calling pro‐
        gram has the option of defining an authentication function with the fol‐
        lowing prototype:

        [4mint[24m [1mmyAuthMethod[22m([4mstruct[24m [4murl[24m [4m*u[24m)

        The callback function should fill in the [4muser[24m and [4mpwd[24m fields in the pro‐
        vided [4mstruct[24m [4murl[24m and return 0 on success, or any other value to indicate
        failure.

        To register the authentication callback, simply set [4mfetchAuthMethod[24m to
        point at it.  The callback will be used whenever a site requires authen‐
        tication and the appropriate environment variables are not set.

        This interface is experimental and may be subject to change.

   [1mRETURN VALUES[0m
        [1mfetchParseURL[22m() returns a pointer to a [4mstruct[24m [4murl[24m containing the individ‐
        ual components of the URL.  If it is unable to allocate memory, or the
        URL is syntactically incorrect, [1mfetchParseURL[22m() returns a NULL pointer.

        The [1mfetchStat[22m() functions return 0 on success and -1 on failure.

        All other functions return a stream pointer which may be used to access
        the requested document, or NULL if an error occurred.

        The following error codes are defined in <[4mfetch.h[24m>:

        [FETCH_ABORT]       Operation aborted

        [FETCH_AUTH]        Authentication failed

        [FETCH_DOWN]        Service unavailable

        [FETCH_EXISTS]      File exists

        [FETCH_FULL]        File system full

        [FETCH_INFO]        Informational response

        [FETCH_MEMORY]      Insufficient memory

        [FETCH_MOVED]       File has moved

        [FETCH_NETWORK]     Network error

        [FETCH_OK]          No error

        [FETCH_PROTO]       Protocol error

        [FETCH_RESOLV]      Resolver error

        [FETCH_SERVER]      Server error

        [FETCH_TEMP]        Temporary error

        [FETCH_TIMEOUT]     Operation timed out

        [FETCH_UNAVAIL]     File is not available

        [FETCH_UNKNOWN]     Unknown error

        [FETCH_URL]         Invalid URL

        The accompanying error message includes a protocol-specific error code
        and message, e.g. "File is not available (404 Not Found)"

   [1mENVIRONMENT[0m
        FETCH_BIND_ADDRESS  Specifies a hostname or IP address to which sockets
                            used for outgoing connections will be bound.

        FTP_LOGIN           Default FTP login if none was provided in the URL.

        FTP_PASSIVE_MODE    If set to anything but ‘no’, forces the FTP code to
                            use passive mode.

        FTP_PASSWORD        Default FTP password if the remote server requests
                            one and none was provided in the URL.

        FTP_PROXY           URL of the proxy to use for FTP requests.  The docu‐
                            ment part is ignored.  FTP and HTTP proxies are sup‐
                            ported; if no scheme is specified, FTP is assumed.
                            If the proxy is an FTP proxy, [1mlibfetch [22mwill send
                            ‘user@host’ as user name to the proxy, where ‘user’
                            is the real user name, and ‘host’ is the name of the
                            FTP server.

                            If this variable is set to an empty string, no proxy
                            will be used for FTP requests, even if the HTTP_PROXY
                            variable is set.

        ftp_proxy           Same as FTP_PROXY, for compatibility.

        HTTP_AUTH           Specifies HTTP authorization parameters as a colon-
                            separated list of items.  The first and second item
                            are the authorization scheme and realm respectively;
                            further items are scheme-dependent.  Currently, the
                            “basic” and “digest” authorization methods are sup‐
                            ported.

                            Both methods require two parameters: the user name
                            and password, in that order.

                            This variable is only used if the server requires
                            authorization and no user name or password was speci‐
                            fied in the URL.

        HTTP_PROXY          URL of the proxy to use for HTTP requests.  The docu‐
                            ment part is ignored.  Only HTTP proxies are sup‐
                            ported for HTTP requests.  If no port number is spec‐
                            ified, the default is 3128.

                            Note that this proxy will also be used for FTP docu‐
                            ments, unless the FTP_PROXY variable is set.

        http_proxy          Same as HTTP_PROXY, for compatibility.

        HTTP_PROXY_AUTH     Specifies authorization parameters for the HTTP proxy
                            in the same format as the HTTP_AUTH variable.

                            This variable is used if and only if connected to an
                            HTTP proxy, and is ignored if a user and/or a pass‐
                            word were specified in the proxy URL.

        HTTP_REFERER        Specifies the referrer URL to use for HTTP requests.
                            If set to “auto”, the document URL will be used as
                            referrer URL.

        HTTP_USER_AGENT     Specifies the User-Agent string to use for HTTP
                            requests.  This can be useful when working with HTTP
                            origin or proxy servers that differentiate between
                            user agents.

        NETRC               Specifies a file to use instead of [4m~/.netrc[24m to look
                            up login names and passwords for FTP sites.  See
                            ftp(1) for a description of the file format.  This
                            feature is experimental.

        NO_PROXY            Either a single asterisk, which disables the use of
                            proxies altogether, or a comma- or whitespace-sepa‐
                            rated list of hosts for which proxies should not be
                            used.

        no_proxy            Same as NO_PROXY, for compatibility.

   [1mEXAMPLES[0m
        To access a proxy server on [4mproxy.example.com[24m port 8080, set the
        HTTP_PROXY environment variable in a manner similar to this:

              HTTP_PROXY=http://proxy.example.com:8080

        If the proxy server requires authentication, there are two options avail‐
        able for passing the authentication data.  The first method is by using
        the proxy URL:

              HTTP_PROXY=http://<user>:<pwd>@proxy.example.com:8080

        The second method is by using the HTTP_PROXY_AUTH environment variable:

              HTTP_PROXY=http://proxy.example.com:8080
              HTTP_PROXY_AUTH=basic:*:<user>:<pwd>

        To disable the use of a proxy for an HTTP server running on the local
        host, define NO_PROXY as follows:

              NO_PROXY=localhost,127.0.0.1

   [1mSEE ALSO[0m
        fetch(1), ftpio(3), ip(4)

        J. Postel and J. K. Reynolds, [4mFile[24m [4mTransfer[24m [4mProtocol[24m, October 1985,
        RFC959.

        P. Deutsch, A. Emtage, and A. Marine., [4mHow[24m [4mto[24m [4mUse[24m [4mAnonymous[24m [4mFTP[24m, May
        1994, RFC1635.

        T. Berners-Lee, L. Masinter, and M. McCahill, [4mUniform[24m [4mResource[24m [4mLocators[0m
        [4m(URL)[24m, December 1994, RFC1738.

        R. Fielding, J. Gettys, J. Mogul, H. Frystyk, L. Masinter, P. Leach, and
        T. Berners-Lee, [4mHypertext[24m [4mTransfer[24m [4mProtocol[24m [4m--[24m [4mHTTP/1.1[24m, January 1999,
        RFC2616.

        J. Franks, P. Hallam-Baker, J. Hostetler, S. Lawrence, P. Leach, A.
        Luotonen, and L. Stewart, [4mHTTP[24m [4mAuthentication:[24m [4mBasic[24m [4mand[24m [4mDigest[24m [4mAccess[0m
        [4mAuthentication[24m, June 1999, RFC2617.

   [1mHISTORY[0m
        The [1mfetch [22mlibrary first appeared in FreeBSD 3.0.

   [1mAUTHORS[0m
        The [1mfetch [22mlibrary was mostly written by Dag-Erling Smørgrav
        ⟨des@FreeBSD.org⟩ with numerous suggestions and contributions from Jordan
        K. Hubbard jkh@FreeBSD.org, Eugene Skepner eu@qub.com, Hajimu Umemoto
        ume@FreeBSD.org, Henry Whincup ⟨henry@techiebod.com⟩, Jukka A. Ukkonen
        jau@iki.fi⟩, Jean-François Dockes ⟨jf@dockes.org⟩ and others.  It
        replaces the older [1mftpio [22mlibrary written by Poul-Henning Kamp
        ⟨phk@FreeBSD.org⟩ and Jordan K. Hubbard jkh@FreeBSD.org.

        This manual page was written by Dag-Erling Smørgrav des@FreeBSD.org.

   [1mBUGS[0m
        Some parts of the library are not yet implemented.  The most notable
        examples of this are [1mfetchPutHTTP[22m() and [1mFTP proxy support.

        There is no way to select a proxy at run-time other than setting the
        HTTP_PROXY or FTP_PROXY environment variables as appropriate.

        [1mlibfetch [22mdoes not understand or obey 305 (Use Proxy) replies.

        Error numbers are unique only within a certain context; the error codes
        used for FTP and HTTP overlap, as do those used for resolver and system
        errors.  For instance, error code 202 means "Command not implemented,
        superfluous at this site" in an FTP context and "Accepted" in an HTTP
        context.

        [1mfetchStatFTP[22m() does not check that the result of an MDTM command is a
        valid date.

        The man page is incomplete, poorly written and produces badly formatted
        text.

        The error reporting mechanism is unsatisfactory.

        Some parts of the code are not fully reentrant.

   BSD                           September 27, 2011                           BSD

#end
