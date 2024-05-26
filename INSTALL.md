
# GriefEdit

This is _GRIEF 3.2.3_, a derivative of the Crisp editor for various UNIX platforms written and released by Paul Fox some 22 years ago and made available on the Internet. This editor is in no way, shape or form derived from the CrispEdit published by the folks who subsequently purchased the copyright to the Fox code.

Compilation under some variety of UNIX should be easiest since the build is supported by a GNU *autoconf* generate `configure` script, which figures out most of what it needs to know from your system.

## Installation instructions

The project can be built from source, the method dependent on the target host. 
The following environments and toolchains are supported.

  * Actively tested operating systems.

      * Linux (gcc)
      * Cygwin (gcc)
      * Win32 (MSVC 2008-2022, Open-Watcom 1.9+, Mingw64/32 latest - beta)

  * Not recently tested, yet builds.

      * OS/X 10.4+

  * Not recently tested, yet *should* build; with minimal effort.

      * HP/UX (gcc)
      * Solaris (gcc)
      * BSD Net/Free/Open (gcc)
      * AIX (gcc)

  * Defunct yet previously supported; development effort assumed.

      * OS/2
      * DOS (djgpp)
      * VMS

The build status of these packages is below.

[![Build status](https://ci.appveyor.com/api/projects/status/3tx1vwwclydfp1t6?svg=true&passingText=Ubuntu%20Passing&failingText=Ubuntu%20Failing&pendingText=Ubuntu%20Pending)](https://ci.appveyor.com/project/adamyg/grief-ubuntu)
[![Build status](https://ci.appveyor.com/api/projects/status/k63ggto1v8t1c28d?svg=true&passingText=MacOS%20Passing&failingText=MacOS%20Failing&pendingText=MacOS%20Pending)](https://ci.appveyor.com/project/adamyg/grief-macos)
[![Build status](https://ci.appveyor.com/api/projects/status/77myicx6ab5d6g1a?svg=true&passingText=Win32%20Passing&failingText=Win32%20Failing&pendingText=Win32%20Pending)](https://ci.appveyor.com/project/adamyg/grief-win32)
[![Build status](https://ci.appveyor.com/api/projects/status/3h8sweuo36r8q28t?svg=true&passingText=Cygwin32%20Passing&failingText=Cygwin32%20Failing&pendingText=Cygwin32%20Pending)](https://ci.appveyor.com/project/adamyg/grief-cygwin32)
[![Build status](https://ci.appveyor.com/api/projects/status/8jk4qx55d4bql3l1?svg=true&passingText=MinGW32%20Passing&failingText=MinGW32%20Failing&pendingText=MinGW32%20Pending)](https://ci.appveyor.com/project/adamyg/grief-mingw)

This document describes installation on all supported operating systems: the Unix/Linux family (including macOS), and Windows (Native and Cygwin).

## Prerequisites

To build and install GriefEdit, you shall need:

  * Clone of the source repository.
  * A supported operating system.
  * Perl 5 with core modules, see [NOTES-PERL.md](doc\NOTES-PERL.md).
  * CoreUtils
  * Make
  * An ANSI C/C++ compiler.
  * A development environment in the form of development libraries and C header files.

For additional platform specific requirements, solutions to specific issues and other details, please read one of these:

  * [Notes for UNIX-like platforms](doc\INSTALL-UNIX.md)
  * [Notes for Windows platforms](doc\INSTALL-WINDOWS.md)
  * [Notes on Perl](doc\NOTES-PERL.md)
  
Plus additional information is available within the _GitHub_ [workflows](.github/workflows/build.yml).   

Quick Installation Guide
========================

If you just want to get going without bothering too much about the details, here is the short version of how to build and install GriefEdit. 

Building GriefEdit
------------------

Use the following commands to configure and build.

### Unix / Linux / macOS / Cygwin

To setup the build environment execute the bundled `configure_new` script, which should select a suitable set of compiler and library options:

    $ ./configure_new

On completion the resulting configuration shall be presented, including installation and details on basic options; for example on the Linux host.

````
-
-  Configuration:
-
-         Compiler: gcc / g++
-           Depend: gcc -MM
-           CFLAGS:   --param max-inline-insns-single=1200 -pthread
-                     Debug:-g -Og, Release:-O1 -DNDEBUG
-         CXXFLAGS: -g -O2 -pthread
-                     Debug:-g -Og, Release:-O1 -DNDEBUG
-          LDFLAGS:   -pthread
-     Preprocessor:
-    Mouse support: none
-   Termap support: ncurses
-    Spell Support: -lenchant
-       Conversion:
-        Detection:    -lmagic
-         Security: -lssl -lcrypto
-      Compression: -lbz2 -llzma -lz
-   Other Features: -lcurl -larchive
-        Libraries:  -lX11 -lXft -lXext -lpthread
-            Extra: -lclang -lm -ldl -lltdl -liberty
-        Allocator:
-             Yacc: -ly
-
-  Installation:
-
-           prefix: /usr/local
-          datadir: /usr/local/share
-
-              BINPATH=/usr/local/bin
-              GRPATH=/usr/local/share/gr/macro
-              GRHELP=/usr/local/share/gr/help
-

 Review the options above for accuracy, to modify see --help for details.
 For example, a user local installation can set using the --prefix option.

      'configure --prefix=/home/adamy/grief'

 Once they look okay, execute to build:

      'make release'              - build software.

 Installation, one of the follow dependent on location:

      'sudo make release install' - install for common/system usage; or
   or 'make release install'      - local installation.

 Optionally after installation:

      'make release clean'        - remove build tree.
````

Once reviewed execute the following:

    $ make release
   

### Windows

If you are using Visual Studio, for example Visual Studio C/C++ 2022, open a "Developer Command Prompt" and issue the following commands to build and package.

    $ .\support\vc2022config
    $ .\win32\make-42 release contrib
    $ .\win32\make-42 release
    $ .\win32\make-42 release package


Installing GriefEdit
--------------------

The following commands will install GriefEdit to a default system location.

For security reasons the default system location is by default not writable for unprivileged users.  So for the final installation step administrative privileges are required.  The default system location and the procedure to obtain administrative privileges depends on the operating system. Alternatively you can run directly from the build tree as a normal user privileges.

### Unix / Linux / macOS / Cygwin

Depending on your distribution, you need to run the following command as root user or prepend `sudo` to the command:   

    $ sudo make release install

The default installation location is

    /usr/local/bin

More precisely, the files will be installed into the  subdirectories

    /usr/local/bin
    /usr/local/share/gr/macros
    /usr/local/share/gr/help


### Windows

To install run the created installer located under the sub-directory `win32'.  Note: The full path of the installer shall be displayed at the end on the make package command above.

    $ .\win32\grwin32-build##-setup.exe

The default installation location is

    C:\Program Files\Grief

for native binaries, or

    C:\Program Files (x86)\Grief

for 32bit binaries on 64bit Windows (WOW64).


Build Failures
--------------

If the build fails, inspect the output and locate any meaning full error messages. The compiler may state exactly what is needed to correct.  For example it is reporting a missing standard or third party component.

If the build succeeded previously, but fails after a source or configuration change, it might be helpful to clean the build tree before attempting another build.  Use this command:

    $ make release clean

# Notes
                            
## Terminals

See the [Quick Start and Programmers Guide](doc/griefprogguide.pdf) for information on setting up GriefEdit completely for your environment, in particular setting environment variables and setting up terminal features.

  * xterm

    GRIEF works well in an xterm, but requires a little more work. The default xterm setup rarely offers enough key/modifier combinations to cope with GRIEF's rather full keyboard mapping; for example, `Shift/Ctrl` function keys rarely work by default. 
    
    Fortunately, X provides a solution via the ability to remap the keyboard to suit the application. To provide functionality equivalent to a console GRIEF, paste contents of the file into your .Xresources.

        EXTRA/xterm.dot.Xresources

    Depending on your hardware and the version of X you're running under, you may need to modify some of the keysym names to reflect your keyboard. This should, however, work fine for an xterm running under XFree86. Ensure that this file is merged via xrdb in your .xinitrc and/or .xsession scripts.

  * dtterm

    See the followinw resources to gain full access to the keyboard.

        EXTRA/dtterm.Xresources   
   
  * rxvt

    **rxvt** is one of the better supported terminal environments which works without any specific configuration.

## Compiling

Notes compiling on specific operating systems and configuring for optimal performance and ease of use on the console:

Note: *Some* of this information is obsolete yet should be hints to resolve terminal issues.

  * Linux

    The Linux console has a fairly complete/full keyboard implementation and the supplied linux.cr terminal features/support macro should work without modification, except if you load alternative keymaps at boot. If you do, then I would suggest overriding keybindings in a separate function within linux.cr, then setting GRTERM to linux-(fnname) where "(fnname)" is the name of the function that does the additional (re)mappings.

  * AIX

    The compile for AIX will be pretty much "out of the box".

    If the termcap/terminfo libraries are installed, build using:

        --with-curses

  * HP/UX

    Use either _curses_, _ncurses_ or _wncurses_ are functional.

  * FreeBSD

    This version of GRIEF has only been tested with the syscons console driver, and not pcvt. The notes below apply only to use with syscons.

    The FreeBSD console is pretty much SCO compatible, so $GRTERM should be set to "sco" (or sco-vga50 if you run in 50 line mode on a VGA or better adaptor). However, unlike the latter SCO operating systems, FreeBSD does not set the `Alt-<keys>` to reasonable defaults - in fact, they are set to their ascii equivalents. Fortunately, a solution is provided in the form of (sco-compatible) loadable keymaps, and a keymap that will make GRIEF usable on
    the console can be found in `EXTRA/us.sco.kbd`. Move this into your keymaps directory normally `/usr/share/syscons/keymaps` and load this by setting the

        keymap name in /etc/sysconfig.

    One other step may be helpful. syscons maps the keypad grey minus, plus and asterisk keys to their ascii keys. These keys need to be remapped to escape-functions kbdcontrol. To do this, simply add the following to an appropriate place in /etc/sysconfig:

        keychange="52 \e[- 56 \e[+"

    This change should not cause any conflicts with other keys. If you wish to use some other values, then modify the sco.cr macro to suit.

    Finally, since `Alt-<Fn>` keys are used for switching between virtual consoles, the syscons configuration for crisp instead maps `Alt-<Fn>` key functions to `Ctrl-Shift-<Fn>.`

    Compiling

    The compile for FreeBSD will be pretty much "out of the box". If you run an older version of FreeBSD than 2.2-RELEASE, you may wish to include the supplied malloc package (configure --with-malloc : see configure --help for details) for improved performance. With 2.2-RELEASE or later, this no longer applies as the system supplied malloc and related routines as they will be the superior. If you supply a package name instead: eg. --with-malloc=malloc_db, for example, then this library is linked in instead. Note that the routines used here are        the malloc() etc. functions described in the standard c headers.

Last updated: _May/24_

-end-