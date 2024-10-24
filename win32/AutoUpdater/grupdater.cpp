// $Id: grupdater.cpp,v 1.11 2023/10/24 13:57:08 cvsuser Exp $
//
//  AutoUpdater command line.
//

#include <cstdlib>
#include <string>
#include <cstring>
#include <iostream>

#include <edbuildinfo.h>

#include "libautoupdater.h"
#include "sign/upgetopt.h"

static void                 Usage();
static const char *         Basename(const char *name);

static const char *         x_progname;


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
    const char *version = GR_VERSION "." GR_BUILD_NUMBER,
            *hosturl = "https://sourceforge.net/projects/grief/files/grief.manifest/download";
    int mode = 2, interactive = 0;
    int ch;

    x_progname = Basename(argv[0]);
    while (-1 != (ch = Updater::Getopt(argc, argv, "V:H:L:icvh"))) {
        switch (ch) {
        case 'V':   /* application version */
            version= Updater::optarg;
            break;
        case 'H':   /* host URL */
            hosturl = Updater::optarg;
            break;
        case 'L':   /* logpath */
            autoupdate_logger_path(Updater::optarg);
            break;
        case 'i':   /* interactive */
            ++interactive;
            break;
        case 'c':   /* console */
            autoupdate_set_console_mode(1);
            break;
        case 'v':   /* verbose */
            autoupdate_logger_stdout(1);
            break;
        case 'h':
        default:
            Usage();
            break;
        }
    }

    argv += Updater::optind;
    if ((argc -= Updater::optind) < 1) {
        std::cerr << "\n" <<
            x_progname << ": expected arguments <mode>" << std::endl;
        Usage();
    } else if (argc > 1) {
        std::cerr << "\n" <<
            x_progname << ": unexpected arguments '" << argv[1] << "' ..." << std::endl;
        Usage();
    }

    const char *arg = argv[0];

    if (0 == _stricmp("disable", arg)) {
        mode = 0;
    } else if (0 == _stricmp("enable", arg)) {
        mode = 1;
    } else if (0 == _stricmp("auto", arg)) {
        mode = 2;
    } else if (0 == _stricmp("prompt", arg)) {
        mode = 3;
    } else if (0 == _stricmp("force", arg)) {
        mode = 4;
    } else if (0 == _stricmp("reinstall", arg)) {
        mode = 5;
    } else if (0 == _stricmp("reset", arg)) {
        mode = -1;
    } else if (0 == _stricmp("dump", arg)) {
        mode = -2;
    } else if (0 == _stricmp("config", arg)) {
        std::cout
            << GR_PACKAGE_NAME << "\n"
            << "Built:   " << GR_BUILD_DATE << "\n"
            << "Version: " << version << "\n"
            << "Host:    " << hosturl << "\n";
        return 0;
    } else {
        std::cerr << "\n" <<
            x_progname << ": unknown mode '" << arg << "'" << std::endl;
        Usage();
    }

    if (mode >= 1) {
        autoupdate_appversion_set(version);
        autoupdate_hosturl_set(hosturl);
    }

    return autoupdate_execute(mode, interactive);
}


//  Function: Usage
//      Echo the command line usage and exit.
//
//  Parameters:
//      none
//
//  Returns:
//      n/a
//
static void
Usage()
{
    std::cerr <<
        "\n"\
        "GRIEF updater                                              version 1.01\n"\
        "\n"\
        "   grupdater [options] mode\n"\
        "\n"\
        "Modes:\n"\
        "   auto -              Periodically check for updates.\n"\
        "   prompt -            Re-prompt user when periodic updates are disabled.\n"\
        "   force -             Prompt ignoring skip status.\n"\
        "   reinstall -         Prompt unconditionally, even if up-to-date/skipped.\n"\
        "\n"\
        "   enable -            Enable periodic checks.\n"\
        "   disable -           Disable automatic periodic checks.\n"\
        "   reset -             Reset the updater status.\n"\
        "\n"\
        "   config -            Configuration.\n"\
        "\n"\
        "\n"\
        "Options:\n"\
        "   -V <version>        Version label.\n"\
        "   -H <host>           Host URL.\n"\
        "   -L <logpath>        Diagnostics log path.\n"\
        "   -c                  Console mode, otherwise GUI.\n"\
        "   -i                  Interactive ('auto' only).\n"\
        "   -v                  Verbose diagnostice.\n"\
        "\n" << std::endl;
    std::exit(99);
}


//  Function: Basename
//      Retrieve the file basename from the specified file path.
//
static const char *
Basename(const char *filename)
{
    const char *name;
    return (NULL != (name = std::strrchr(filename, '/')))
                || (NULL != (name = std::strrchr(filename, '\\'))) ? name + 1 : filename;
}

/*end*/
