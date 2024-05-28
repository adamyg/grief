
Windows platforms
=================

  - [Native builds using Open-Watcom](#native-builds-using-openwatcom-c-c)
  - [Native builds using Visual C++](#native-builds-using-visual-c-c)
  - [Native builds using MinGW](#native-builds-using-mingw)
  - [Hosted builds using Cygwin](#hosted-builds-using-cygwin)

Native builds using OpenWatcom C/C++
====================================

In addition to the requirements and instructions listed in ``INSTALL.md``, these following third-party tools are required:

### Perl

A Perl installation needs to be available plus the installation should be visible within the current PATH.

Strawberry Perl is recommended, available from <http://strawberryperl.com/>,
as an alternative is ActiveState Perl, <https://www.activestate.com/ActivePerl>.

### Watcom C/C++

Watcom C/C++ (currently Open Watcom C/C++) is an integrated development environment (IDE) product from Watcom International Corporation for the C, C++, and Fortran programming languages. Watcom C/C++ was a commercial product until it was discontinued, then released under the Sybase ``Open Watcom`` Public License as ``Open Watcom C/C++``.

Two versions of ``Open Watcom C/C++`` are freely available, the legacy _1.9_ version and the current Open _2.0_ development stream, both are actively supported.
These are available from <https://www.openwatcom.org> and <https://github.com/open-watcom/open-watcom-v2>.

Note: Watcom is the current tool-chain published builds utilise.

### Binary utilises

In addition to Perl and the selected compiler tool-chain, several utilises are required. Under Unix like environments these are referred to as the ``binutils`` package, yet are not generally installed on Windows host.

Minimal tools required are:

  * gmake - _GNU make utility_.

      make is a utility which can determine automatically which pieces of a large program need to be recompiled, and issue the commands to recompile them.

  * bison/yacc - _General purpose parser generators_.

  * busybox - _The Swiss Army Knife of Embedded Linux_

      BusyBox combines tiny versions of many common UNIX utilities into a single small executable. It provides replacements for most of the utilities you usually find in GNU fileutils, shellutils, etc. The utilities in BusyBox generally have fewer options than their full-featured GNU cousins; however, the options that are included provide the expected functionality and behave very much like their GNU counterparts. BusyBox provides a fairly complete environment for any small or embedded system

  * wget - _Command-line utility for retrieving files using HTTP, HTTPS and FTP protocols_.

  * coreutils - Collection of file and text manipulation utilities; including

      * cp - _copy files and directories_.

      * mv - _move (rename) files_.

      * rm - _remove files or directories_.

      * egrep - _print lines that match patterns_.

      * gzip, gunzip, zcat - _compress or expand files_.

      * tar - _an archiving utility_.

      Coreutils are bundled with [__GIT for Windows__](https://gitforwindows.org/),
      alternatively install [__MSYS2__](https://www.msys2.org/).

      Once installed the required commands should be visible within the path.

To support native Windows builds, the make tool ``gmake-42``, web tool ``wget`` and the shell support tool ``busybox`` are bundled within the source repository sub-directory ``win32/``.

  - ``gmake`` was built from its original source available from [GNU binutils](https://www.gnu.org/software/binutils/).

  - Whereas the pre-built binary for ``wget`` was sourced from <https://eternallybored.org/misc/wget> and ``busybox`` was sourced from a recent stable release [BusyBox for Windows](https://frippery.org/busybox).

  - Furthermore ``flex`` and ``byacc`` *Berkley yacc* are built as part of the third-party components under ``contrib/``.

The configure front-end shall attempt to select the most suitable tools available on the build host.

### InnoSetup

To package the build application as an installer, ``Inno Setup`` is utilised.
``Inno Setup`` is a free installer for Windows program's, available from <https://jrsoftware.org/>.

This package is optional and only required if an installer is being built.

``Inno Setup`` 5.6.x or greater is required. Note 6.x Inno and later shall only function on Vista or greater. 6.x and later versions of Inno no longer support Windows 2000, XP, and Server 2003.

Using Chocolatey, to install _Inno Setup_, run the following on a command line:

    $ choco install innosetup --version=5.6.1

Note: Inno-Setup 5.6.x (Unicode version) is used to package the current installers.


Quick start
-----------

  * Install _Perl_

  * Install _Inno-Setup_

    Install ``Inno Setup`` 5.6.x within its default installation path; if modified the arguments to the support scripts below shall need to be adjusted to match, set the environment variable ``INNO="<install-path>"`` prior to priming the tree.

  * Make sure _Perl_ is on your __\$PATH__.

  * Create a _Open Watcom_ command Prompt and confirm that ``wcl386`` is within your path.

    From the source root, a suitable environment can be setup using the one of the following dependent on the desired toolchain, were ``C:\Watcom`` is the toolchain installation directory.

        $ .\support\owcsetenv C:\Watcom         # Open Watcom 1.9
        $ .\support\owc20setenv C:\Watcom       # Open Watcom 2.0

  * From the root of the source directory perform the following:

      * Configure and prime the build system.

            $ .\support\owcconfig               # Open Watcom 1.9
            $ .\support\owc20config             # Open Watcom 2.0

        on completion the make system is ready, run only make shall present usage information:

            $  .\win32\gmake-42
            | make [VERBOSE=1] [release or debug] target
            |
            |       Build one or more of the following targets recursively within each sub-directory
            |       for the toolchain <toolchains>.
            |
            | Options:
            |       VERBOSE - increase run-time diagnostics; suppress stderr filtering.
            |
            | Targets:
            |
            |       contrib - build third-party components.
            |       build - build everything.
            |       package - build package.
            |       clean - delete everything which can be remade.
            |       vclean - delete all.
            |       help - command line usage.

      * Build the ``contrib/`` packages.

            $  .\win32\gmake-42 release contrib

      * Build the editor components.

            $ .\win32\gmake-42 release build

      * Optionally, build the installer.

            $ .\win32\gmake-42 release package

        Note: Alternatively zip and copy the ``bin``, ``macro`` and ``help`` trees to your desired install location.

Installation
=============

The default installation paths compiled into __GriefEdit__ are as follows. However the MACRO and HELP source directories may be overridden with either environment variables or command line are:

  * Executables -      _/Program Files/Grief/bin_

  * Help files -       _/Program Files/Grief/help_

  * Compiled macros -  _/Program Files/Grief/macros_

  * Macro source -     _/Program Files/Grief/src_

Native builds using Visual C/C++
================================

Visual C/C++ offers an alternative way to build native __GriefEdit__, similar to Open-Watcom C/C++ builds.

Note: Since these are proprietary and ever-changing we cannot test them all. Older versions may not work. Use a recent version wherever possible.

  * Install _Perl_

  * Install _Inno-Setup_

  * Make sure _Perl_ is on your __\$PATH__.

  * Use Visual Studio Developer Command Prompt with administrative privileges, choosing one of its variants depending on the intended architecture. Or run `cmd` and execute `vcvarsall.bat` with one of the options `x86`, `x86_amd64`, `x86_arm`, `x86_arm64`, `amd64`, `amd64_x86`, `amd64_arm`, or `amd64_arm64`. This all setup the environment variables needed for the compiler `cl.exe`.

    See also <https://docs.microsoft.com/cpp/build/building-on-the-command-line>

  * From the root of the source directory perform the following:

      * Configure and prime the build system.

            $ .\support\vc####config

        where #### representes the toolchain, for example 2019.

            $ .\support\vc2019config

      * Build the ``contrib/`` packages.

            $ .\win32\gmake-42 release contrib

      * Build the editor components.

            $ .\win32\gmake-42 release

      * Optionally, build the installer.

            $ .\win32\gmake-42 release package


Native builds using Mingw
=========================

Mingw32/64 offers another alternative way to build native __GriefEdit__, similar to Open-Watcom C/C++ builds.

MSYS2 provides GNU tools, a Unix-like command prompt, and a UNIX compatibility layer for applications. However, in this context it is only used for building GriefEdit. The resulting application does not rely on MSYS2 to run and is fully native.

  * _MSYS2_ shell, from https://www.msys2.org/

  * _Perl_, at least version 5.10.0, which usually comes pre-installed with MSYS2.

  * Install _Inno-Setup_

  * Create a MSYS/Mingw64 Command Prompt.

    To install the minimal tools required:

        $ pacman --noconfirm -S base-devel
        $ pacman --noconfirm -S bison
        $ pacman --noconfirm -S

    plus one of the following

        $ pacman --noconfirm -S mingw-w64-x86_64-gcc
        $ pacman --noconfirm -S mingw-w64-i686-gcc

    These compilers must be on your MSYS2 \$PATH, example below assuming the default installation path ``c:/msys64/``. A common error is to not have these on your \$PATH. The MSYS2 version of gcc will not work correctly here.

  * From the root of the source directory perform the following:

      * Configure and prime the build system.

          * x64 tool-chain

                  PATH=c:\msys64\mingw64\bin;c:\msys64\usr\bin
                  $ .\support\mingw64config

          * x86 tool-chain.

                  PATH=c:\msys64\mingw32\bin;c:\msys64\usr\bin
                  $ .\support\mingw32config

      * Build the ``contrib/`` packages.

            $  .\win32\gmake-42 release contrib

      * Build the editor components.

            $ .\win32\gmake-42 release

      * Optionally, build the installer.

            $ .\win32\gmake-42 release package


Native builds using Cygwin
==========================

Cygwin implements a POSIX/Unix runtime system (`cygwin1.dll`) on top of the Windows subsystem and provides a Bash shell and GNU tools environment. Consequently, a build of GriefEdit with Cygwin is virtually identical to the Unix procedure.

To build using Cygwin, you need to:

 * Install Cygwin, see <https://cygwin.com/>.

   During this process, install the minimal tools required:

    * Perl, at least version 5.10.0 and ensure it is in the \$PATH.

    * coreutils - _GNU core utilities (includes fileutils, sh-utils and textutils)_

    * make - _GNU (gmake) version of the 'make' utility_.

    * gcc-core - _GNU Compiler Collection_.

    * optionally, bison/flex and wget.

 * Run the Cygwin Bash shell.

Apart from this, follow the Unix / Linux instructions in [INSTALL-LINUX](INSTALL-UNIX.md).

Additional, optional, components can also be utilised.

Cygwin packages are be installed by a number of means.

  * [setup-x86_64](https://cygwin.com/install.html)

  * [apt-cpt](https://github.com/transcode-open/apt-cyg)


# Alternative tools

As an alternative to the bundled ``win32//`` tools, these can be sourced by a number of methods.

  * gmake (4.2 or greater)

      - Using Chocolatey. First you need to install this package manager. Once installed you simply need to install make (you may need to run it in an elevated/admin command prompt) :

            choco install make

      - Install as part of a MSYS installation:

            pacman -S make

      After installation confirm the tool is within the \$PATH.

  * flex/bison

      - Using Chocolatey. First you need to install this package manager. Once installed you simply need to install make (you may need to run it in an elevated/admin command prompt) :

            choco install winflexbison

      - Install as part of a MSYS installation:

            pacman -S bison

      - Legacy ``GnuWin32`` package, bison is one of the bundled applications.

            choco install gnuwin32-coreutils.install

      After installation confirm the tool is within the \$PATH, or add the installation details on the configure command line, for example

            $ .\support\vc2022config.bat --bison=c:/msys64/usr/bin/bison --flex=c:/msys64/usr/bin/flex

  * wget

      - Using Chocolatey. First you need to install this package manager. Once installed you simply need to install make (you may need to run it in an elevated/admin command prompt) :

            choco install wget

      - Install as part of a MSYS installation:

            pacman -S wget

      After installation confirm the tool is within the \$PATH, or add the installation details on the configure command line, for example

            $ .\support\vc2022config.bat --wget=c:/msys64/usr/bin/wget

  * busybox

    Directly import from either [BusyBox for Windows](https://frippery.org/busybox) or [busybox.net](https://busybox.net).

        $ wget https://frippery.org/files/busybox/busybox.exe

    Using Chocolatey, to install _BusyBox-w32_, run the following on a command line:

        choco install busybox

    On install confirm the tool is within the \$PATH, or add the installation details on the configure command line, for example

        $ .\support\vc2022config.bat --wget=<path-to-busybox>/busybox

Last Update: May/2024

=end=
