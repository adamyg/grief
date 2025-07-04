//  $Id: grupdater.cpp,v 1.12 2025/07/04 05:09:13 cvsuser Exp $
//
//  GriefEdit AutoUpdater command line.
//

#include "update/updatetoolshim.h"

#include "hosturls.inc"
#include "public_version1.inc"

#include "../include/edbuildinfo.h"

//  Function: Main
//      Application entry.
//
//  Returns:
//      0  - No check performed.
//      1  - Up-to-date.
//      2  - Installed.
//      3  - Update available.
//      99 - Usage
//

int
main(int argc, char *argv[])
{
    struct UpdateToolArgs args = {0};

    args.progname = "grupdater";
    args.progtitle = "GriefEdit, updater (" GR_VERSION "." GR_BUILD_NUMBER ")";

    args.appname = "GriefEdit";
    args.version = GR_VERSION "." GR_BUILD_NUMBER;

    args.hosturl = hosturl1;
    args.hosturlalt = hosturl2;
    args.publickey = public_key_base64;
    args.keyversion = key_version;

    return UpdateToolShim(argc, argv, &args);
}

//end
