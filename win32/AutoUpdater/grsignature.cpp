//  $Id: grsignature.cpp,v 1.4 2025/07/04 05:09:13 cvsuser Exp $
//
//  grsignature - manifest generation tool.
//

#include "sign/signtoolshim.h"

#include "hosturls.inc"

#include "../include/edbuildinfo.h"

int
main(int argc, char *argv[])
{
    const char *hosturl = hosturl1;

    struct SignToolArgs args = {0};

    args.progname = "grsignature";
    args.progtitle = "GriefEdit, manifest generator (" GR_VERSION "." GR_BUILD_NUMBER ")";

    args.hosturl = hosturl1;
    args.hosturlalt = hosturl2;

    return SignToolShim(argc, argv, &args);
}

//end
