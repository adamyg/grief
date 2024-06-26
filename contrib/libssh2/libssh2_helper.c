//$Id: libssh2_helper.c,v 1.1 2020/06/18 19:57:47 cvsuser Exp $
//
//  libssh2 support
//

#define  LIBSSH2_LIBRARY
#include <libssh2_helper.h>


LIBSSH2_API int
libssh2_helper_trace(void)
{
#if defined(LIBSSH2DEBUG)
    return 1;
#else
    return 0;
#endif
}


LIBSSH2_API const char *
libssh2_helper_engine(void)
{
#if defined(LIBSSH2_OPENSSL)
    return "openssl";
    
#elif defined(LIBSSH2_MBEDTLS)
    return "mbedtls";
    
#elif defined(LIBSSH2_WINCNG)
    return "wincng";

#else
#error Unknown engine configuration
#endif
}
 
//end

