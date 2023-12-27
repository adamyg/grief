//
//  MSVC +2010/
//
//      Work around for the following condition,
//
//              <system_error> is incompatible with _CRT_NO_POSIX_ERROR_CODES.
//

#include <msvcversions.h>

#if defined(__cplusplus)
#if defined(_MSC_VER) && (_MSC_VER > _MSC_VER_2010) && \
        defined(_CRT_NO_POSIX_ERROR_CODES)
    //
    //  Problem:
    //      <system_error> fails to compile with _CRT_NO_POSIX_ERROR_CODES defined as they are referenced
    //      within the 'enum class errc' definitions. yet the associations are irrelevant in many ways.
    //
    //  Description:
    //      _CRT_NO_POSIX_ERROR_CODES and the values defined within <errno.h> do not reflect either
    //      WinSock's nor WinSystem error codes, for example:
    //
    //      EINVAL is assigned to 22, which neither maps to
    //
    //          o ERROR_INVALID_PARAMETER (87), or
    //          o WSAEINVAL (10004)
    //
    //      In addition, the available error codes do not reflect the full scope of reported either 
    //      WinSock's or OS API's.
    //
    //      Hence we disable globally and define our own set against WinSock2 returns allowing them
    //      to be functionality compatiable with API calls; see <win32_errno.h>.
    //
    //  Notes:
    //      Only become an issue from MSVC 2010+ and recently addressed in MSVC 2019+.
    //
    //      Ref: https://github.com/microsoft/STL/blob/master/stl/src/syserror.cpp
    //
#undef   _CRT_NO_POSIX_ERROR_CODES
#include <system_error>
#define  _CRT_NO_POSIX_ERROR_CODES

#else
#include <system_error>
#endif

#endif  //__cplusplus

//end
