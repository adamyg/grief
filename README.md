[![Website](https://img.shields.io/badge/View-Website-blue)](https://sourceforge.net/projects/grief/)
[![GitHub release](https://img.shields.io/github/release/Naereen/StrapDown.js.svg)](https://GitHub.com/adamyg/grief/releases/)
[![Workflow](https://github.com/adamyg/grief/actions/workflows/build.yml/badge.svg)](https://github.com/adamyg/grief/actions)

[![Build status](https://ci.appveyor.com/api/projects/status/3tx1vwwclydfp1t6?svg=true&passingText=Ubuntu%20Passing&failingText=Ubuntu%20Failing&pendingText=Ubuntu%20Pending)](https://ci.appveyor.com/project/adamyg/grief-ubuntu)
[![Build status](https://ci.appveyor.com/api/projects/status/k63ggto1v8t1c28d?svg=true&passingText=MacOS%20Passing&failingText=MacOS%20Failing&pendingText=MacOS%20Pending)](https://ci.appveyor.com/project/adamyg/grief-macos)
[![Build status](https://ci.appveyor.com/api/projects/status/77myicx6ab5d6g1a?svg=true&passingText=Win32%20Passing&failingText=Win32%20Failing&pendingText=Win32%20Pending)](https://ci.appveyor.com/project/adamyg/grief-win32)
[![Build status](https://ci.appveyor.com/api/projects/status/3h8sweuo36r8q28t?svg=true&passingText=Cygwin32%20Passing&failingText=Cygwin32%20Failing&pendingText=Cygwin32%20Pending)](https://ci.appveyor.com/project/adamyg/grief-cygwin32)
[![Build status](https://ci.appveyor.com/api/projects/status/8jk4qx55d4bql3l1?svg=true&passingText=MinGW32%20Passing&failingText=MinGW32%20Failing&pendingText=MinGW32%20Pending)](https://ci.appveyor.com/project/adamyg/grief-mingw)

GRIEF - BRIEF clone
=======================================================

Introduction:
--------------------

GRIEF is a full-featured console based editor offering a wealth of facilities on
multiple Unix, Windows and Mac platforms. It edits plain text files and has numerous
options depending on the type of work you are doing.

Based on a long standing interface, Grief is an intuitive and easy editor to both
novice and seasoned developers, inheriting its clean user interface from the BRIEF
family of programmers editors.

![GRIEF Quick Start and Programmers Guide](https://github.com/adamyg/grief/blob/master/griefprogguide.pdf)

Brief, BRIEF, or B.R.I.E.F., an acronym for Basic Reconfigurable Interactive Editing
Facility, was a popular programmer's text editor in the 1980s and early 1990s.

GRIEF 3.2.x build Tree:
----------------------------

The following environments and toolchains are supported.

  * Actively tested operating systems.

      * Linux (gcc)
      * Cygwin (gcc)
      * Win32 (MSVC 2008-2022, Open-Watcom 1.9+, Mingw64/32 latest)

  * Not recently tested, yet builds.

      * OS/X 10.4+

  * Not recently tested, yet *should* build; with minimal effort.

      * HP/UX (gcc)
      * Solaris (gcc)
      * BSD Net/Free/Open
      * AIX (gcc)

  * Defunct yet previously supported; effort assumed

      * OS/2
      * DOS (djgpp)
      * VMS

Examples
--------------------

![Example1](https://github.com/adamyg/grief/blob/master/hlpdoc/examples/Example1.png?raw=true)

![Example2](https://github.com/adamyg/grief/blob/master/hlpdoc/examples/Example2.png?raw=true)


Installation:
--------------------

Recent distributions are now bundled with an installer, the following shall prompt and then check for available upgrades.
Note: Automatic checks wont occur at this time; functionality staged for a later release.

```
grupdater force
```

Distributions:
--------------------

   * WIN32 binaries

        https://github.com/adamyg/grief/releases
        and https://sourceforge.net/projects/grief

   * Source

        https://github.com/adamyg/grief/releases

        To build from source, review the github workflows:
        https://github.com/adamyg/grief/blob/master/.github/workflows/build.yml


Status:
--------------------

Please feel free to raise tickets on SourceForge when issues are encountered.


Other Resources:
--------------------------------

Externally reviewed information regarding the development of BRIEF, can be found at the following

    * http://en.wikipedia.org/wiki/Brief_(text_editor)

Details about the commercially supported BRIEF clone CRisPEdit(tm), can be found at the following.

    * <http://www.crispeditor.co.uk> previously <http://www.crisp.demon.co.uk>

    * <http://www.crisp.com>

last update: June/2022

-end-
