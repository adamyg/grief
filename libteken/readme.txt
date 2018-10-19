        libteken: terminal emulator library
        source: http://80386.nl/projects/libteken

        Traditionally the FreeBSD console driver (syscons) uses the cons25 terminal
        type. This terminal type is basically a very compact subset of later VT-devices
        or graphical terminals like xterm. There are two problems with this approach:

                o Many other operating systems do not ship cons25 termcap entries.

                o Many embedded devices with serial or telnet interfaces only support VT100-style escape sequences.

        Because I noticed it isn't easy to emulate a proper terminal correctly, I
        decided to write a small library that converts a stream of bytes to drawing
        instructions, called libteken. Right now it implements a sane subset of escape
        sequences. Running regular ncurses applications using xterm-style termcap
        entries should work properly. This library is currently being used by the
        FreeBSD console driver.

        libteken supports UTF-8. This is currently disabled in FreeBSD, because we
        still need to port other subsystems to handle UTF-8 (input layer, font renderer,
        etc). When UTF-8 is enabled, it converts a stream of UTF-8 encoded bytes to
        Unicode code points.

        The source code comes with two applications. teken_demo emulates a terminal and
        displays it using ncurses. This application can be used to easily test the
        implementation. teken_stress floods a terminal emulator with random data. This
        can be used to test the implementation's robustness.

        Because the code is now part of the FreeBSD operating system, the source code
        can be obtained at FreeBSD's SVN repository:

                svn checkout svn://svn.FreeBSD.org/base/head/sys/teken/




