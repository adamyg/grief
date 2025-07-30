// -*- mode: c; -*-
//
// GriefEdit, hosturl's
// June 2025
//

static const char
#if defined(_WIN64) || defined(_M_AMD64) // x64; or as channel?
        *hosturl2 = "https://sourceforge.net/projects/grief/files/griefx64.manifest/download", // legacy
        *hosturl1 = "https://api.github.com/repos/adamyg/grief~griefx64.manifest"; // primary
#else
        *hosturl2 = "https://sourceforge.net/projects/grief/files/grief.manifest/download", // legacy
        *hosturl1 = "https://api.github.com/repos/adamyg/grief~grief.manifest"; // primary
#endif

//end
