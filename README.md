[![Website](https://img.shields.io/badge/View-Website-blue)](https://sourceforge.net/projects/grief/)
[![GitHub release](https://img.shields.io/github/release/Naereen/StrapDown.js.svg)](https://GitHub.com/adamyg/grief/releases/)
[![Workflow](https://github.com/adamyg/grief/actions/workflows/build.yml/badge.svg)](https://github.com/adamyg/grief/actions)

[![Build status](https://ci.appveyor.com/api/projects/status/3tx1vwwclydfp1t6?svg=true&passingText=Ubuntu%20Passing&failingText=Ubuntu%20Failing&pendingText=Ubuntu%20Pending)](https://ci.appveyor.com/project/adamyg/grief-ubuntu)
[![Build status](https://ci.appveyor.com/api/projects/status/k63ggto1v8t1c28d?svg=true&passingText=MacOS%20Passing&failingText=MacOS%20Failing&pendingText=MacOS%20Pending)](https://ci.appveyor.com/project/adamyg/grief-macos)
[![Build status](https://ci.appveyor.com/api/projects/status/77myicx6ab5d6g1a?svg=true&passingText=Win32%20Passing&failingText=Win32%20Failing&pendingText=Win32%20Pending)](https://ci.appveyor.com/project/adamyg/grief-win32)
[![Build status](https://ci.appveyor.com/api/projects/status/3h8sweuo36r8q28t?svg=true&passingText=Cygwin32%20Passing&failingText=Cygwin32%20Failing&pendingText=Cygwin32%20Pending)](https://ci.appveyor.com/project/adamyg/grief-cygwin32)
[![Build status](https://ci.appveyor.com/api/projects/status/8jk4qx55d4bql3l1?svg=true&passingText=MinGW32%20Passing&failingText=MinGW32%20Failing&pendingText=MinGW32%20Pending)](https://ci.appveyor.com/project/adamyg/grief-mingw)

# GRIEF - BRIEF clone
=======================================================

## Introduction:
--------------------

GRIEF is a full-featured console based editor offering a wealth of facilities on multiple Unix, Windows and Mac platforms. 
It edits plain text files and has numerous options depending on the type of work you are doing.

Based on a long standing interface, Grief is an intuitive and easy editor to both novice and seasoned developers, inheriting its clean user interface from the BRIEF family of programmers editors.

Brief, BRIEF, or B.R.I.E.F., an acronym for Basic Reconfigurable Interactive Editing Facility, was a popular programmer's text editor in the 1980s and early 1990s.

![Example1](https://github.com/adamyg/grief/blob/master/hlpdoc/examples/Example1.png?raw=true)

For a more in-depth look at the setup and running of GRIEF, see ![GRIEF Quick Start and Programmers Guide](https://github.com/adamyg/grief/blob/master/griefprogguide.pdf)

## GRIEF 3.2.x build Tree:
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

## Installation:

See INSTALL for details.

## Distributions:

   * WIN32 binaries

        https://github.com/adamyg/grief/releases
        and https://sourceforge.net/projects/grief

   * Source

        https://github.com/adamyg/grief/releases

        To build from source, review the github workflows:
        https://github.com/adamyg/grief/blob/master/.github/workflows/build.yml


## Status:

Please feel free to raise tickets on Github when issues are encountered.


# QuickStart
----------------------------

This introduction is an outline on how to use GRIEF, based on the default keyboard layout made popular by BRIEF.  The fundamental GRIEF commands you need to know are shown below.

GRIEF’s local configuration may be viewed using the *--config* option.    

````
$gr --config
````

Windows profile:

````
PROGNAME=C:/Program Files (x86)/Grief/bin/gr.exe
MACHTYPE=Win32
GRPATH=C:/Program Files (x86)/Grief/macros;C:/Program Filesx86)/Grief/src;
GRHELP=C:/Program Files (x86)/Grief/help;
GRPROFILE=
GRLEVEL=1
GRFILE=newfile
GRFLAGS=-i60
GRBACKUP=
GRVERSIONS=
GRDICTIONARIES=
GRDICTIONARY=
GRTERM=win32
````

Linux profile:

````
PROGNAME=/usr/local/bin/gr
MACHTYPE=UNIX
GRPATH=/usr/local/share/gr/macros:/usr/local/share/gr/src:
GRHELP=/usr/local/share/gr/help
GRPROFILE= GRLEVEL=1 GRFILE=newfile
GRFLAGS=-i60
GRBACKUP=
GRVERSIONS=
GRDICTIONARIES=
GRDICTIONARY=
GRTERM=linux
````

The more significant configuration elements which are required for correct operation are:

  * The *GRPATH* global string is used to specify one or more directory names stating the search path which GRIEF shall utilise to locate macro objects during ⟨autoload⟩ and ⟨require⟩ operations.
    The initial value of GRPATH is either imported from the environment or if not available is derived from the location of the running application.

  * The *GRHELP* global string is used to specify the directory name stating the search path which GRIEF shall utilise to locate the help database. 
    The initial value of GRHELP is either imported from the environment or if not available is derived from the location of the running application.

  * The *GRPROFILE* global string is used to specify an override to the standard users home directory; it is utilised by several macros to source runtime configuration details.
    GRPROFILE provides the user a means of stating an alternative location.

## Editing and exiting

Editing any file is a simple as running GRIEF against the file image.

````
gr m_ruler.c
````

Starts the editor, loading the specified file.

![mainscreen](https://github.com/adamyg/grief/blob/master/hlpdoc/src/images/mainscreen.png?raw=true)

The file can be reviewed by navigating using _Movement_ commands.

At any time during the file session online _Help_ is available, plus the ``F10:key_map`` macro shall present the current keyboard binding.

The file content may be directly manipulated using _Text Editing_ commands, components can be relocated using _Cut and Paste_ or translated _Search and Replace_. 
Any unwanted edits maybe corrected using ⟨Undo and Redo⟩ commands.

Once complete the edit session is complete a number of options are available.

| Key              | Description                                                  
|:-----------------|:-------------------------------------------------------------------------------
| Alt-W            | Write buffer                                                 
| Alt-X            | Exit.            
| Alt-H            | Help.
| F10:key_map      | Display key binds.
| F10:explain      | Feature explanation.
| Alt-Z            | Spawn a sub-shell
| F10              | Run additional commands against the file using the ``Command Prompt``

If you execute the ``Alt-X`` or ``F10:exit`` after modifying any of the open files, the prompt below is presented.

````
1 buffer has not been saved. Exit [ynw]?
````

You will be given a number of choices.

  * *w* - Writes the file back to the file-system and exit to the operating system.
  * *y* - The buffer is not saved, and you return to the operating system; **note** your local changes shall be lost.
  * *n* - The command is cancelled and you return to the editor.
   
The main features of the screen are the window arena, command line and status area.

![griefscreen](https://github.com/adamyg/grief/blob/master/hlpdoc/src/images/griefscreen.png?raw=true)

The window arena is the area bounded by single and double lines, or borders, that displays the file content.  If the file is a new image, this space shall be empty and the window is blank.

The top border of the window contains the file name associated with the visible buffer, plus an optional modification indicator. 
On right and bottom borders scroll bars represent the vertical and horizontal cursor position within the buffer when the buffer length or maximum line width are larger then the window arena.

The command line, positioned below the window area, is dual purpose, used to display both messages and to prompt for information. 
The command line is also referred to as the _Command Prompt_ when acting to request user input.

The status area, also referred to as the echo line, displays information about the current active buffer and general editor status.

## Buffers

Files are always accessed by loading them into a buffer.
GRIEF is able to keep multiple files in memory at once, which allows the user to move among them easily.
Once a user is in GRIEF they can call up as many files as they need to use by loading them into a buffer within GRIEF.
Pop-up menus are often used when moving among files.
How to chooseoptions on the pop-up menu is self-evident. 
To exit a pop-up menu use the ``Esc`` key

| Key              | Description
|:-----------------|:-------------------------------------------------------------------------------
| Alt-E            | File and Buffer Manipulation in GRIEF Call up a file to be edited.  The user is prompted for the name of the file. Hitting \<Tab> pops up a menu of file-names the user can select from.
| Alt-B            | Pops up a window containing a list of the files currently in memory.
| Alt-N            | Next buffer.
| Alt-P            | Previous buffer.
| Alt-1 to Alt-9   | Drop a bookmark.
| Alt-J            | Goto a bookmark.

GRIEF windows can be *tiled* and used to look at more than one file at the same time, or different parts of the same file at the same time.

By tiling, we mean the window can be split horizontally or vertically.
This can be done as many times as you like, i.e. you can split your window into as many sections as you like.
Different buffers (files) can be viewed in different sections of the tiled window, or even different parts of the same buffer.

Each section of the tiled window can be treated as an individual GRIEF session.
Any changes to a buffer made in one part of a tiled window are displayed in the other parts of the tiled window displaying the same part of the same buffer

| Key              | Description
|:-----------------|:-------------------------------------------------------------------------------
| F1               | Select a different section of a tiled window.  User is prompted to point to the section desired, using one of the four arrow keys
| F2               | Move the boundary between two windows on the screen.  User is prompted to point the boundary which is to be moved, and then use the arrow keys to move the boundary.  The screen is redrawn as the boundary is moved  
| F3               | Split the current window into two equal sizes, either horizontally or vertically
| F4               | Delete a boundary between two windows and merge them together
| Ctrl-Z           | Subwindow zoom toggle: makes a forward zoom on the current subwindow.  This means that the current subwindow will occupy all possible place in the total window.  Use Ctrl-Z again to unzoom, i.e. to see again all sub-windows.
| F10:wininfo      | Window information dialog.

Tiled windows are created by splitting the current window in half either horizontally or vertically, by default using ⟨F3⟩, providing two views of the current buffer.

On request the user is prompted and the current window is split based on he direction of the selected arrow key.
On completion the newly created window is made current, with the cursor located at the same coordinates as the parent window.

## Navigation

Buffer navigation is available using a rich set of the cursor commands.

| Key              | Description
|:-----------------|:-------------------------------------------------------------------------------
| Right,Space      | Move cursor right one position.              
| Left,Backspace   | Move cursor left one position. 
| Down             | Move cursor down one line, maintaining same column  position.
| Up               | Move cursor up one line, maintaining same column position.
| PgUp,Wheel-Up    | Move cursor up visible display page.
| PgDn,Wheel-Up    | Move cursor down visible display page.
| Home             | Move cursor to beginning of current line.
| Home+Home        | Move cursor to top of current window.
| Home+Home+Home   | Move cursor to beginning of the buffer.
| End              | Move cursor to last character of current line.
| End+End          | Move cursor to end of current window.
| End+End+End      | Move cursor to end of the buffer.
| Ctrl-PgUp        | Move cursor to top of the buffer.
| Ctrl-PgDn        | Move cursor to end of the buffer.
| Ctrl-Up          | Scroll lock window movement, moving the text view up one line retaining the cursor display within the window.
| Ctrl-Down        | Scroll lock window movement, moving the text view down one line retaining the cursor display within the window.
| Ctrl-Left        | Move cursor to start of previous word.
| Ctrl-Right       | Move cursor to start of next word.
| Ctrl-Left        | Move cursor to start of previous word.
| Alt-G            | Goto line.
| Ctrl-G           | For supported source types list the local function definitions.

Scroll-locking is a facility for keeping the cursor in a fixed position inside a window, accessed via the ⟨ScrollLock⟩ key toggling alternately between enabled and disabled.

The behaviour of scroll-locking is dependent on the number of visible windows.

When a single window is active, the cursor is locked into position. 
Upon movement the cursor shall retain the same screen location, instead the buffer content is panned within the window where possible.

If multiple windows are active, scroll-locking allows two windows to be scrolled together. 
This feature permits two files to be compared with one another, without the user needing to switch between windows in order to check the two file views in sync.
When enabled the user shall be prompted to select which other window the current window shall be synced with.

## Editing

GRIEF is a *modeless* editor, compared to vi, and it’s successor Vim, which are *modal* editors.  
Modeless meaning that text is entered directly into the buffer as typed and commands are generally not context specific, behaving the same most of the time. 
Modal editing, on the other hand, means that the editor switches between the state of inserting text and taking commands.

The normal editing environment is referred to as the editing mode, and all of GRIEF’s commands and editing capabilities are available from it.

There are two variations on the editing mode: insert mode and overstrike mode. In insert mode, typed text is inserted at the cursor.
In overstrike, existing text is overwritten as you type.  The insert mode is reflected in state of the cursor plus its status shall be visible within the status area.

Deletion and relocation of text is possible using the scrap buffer, see
⟨Cut and Paste⟩.

| Key              | Description                                                
|:-----------------|:-------------------------------------------------------------------------------
| Backspace        | Delete the last character typed.
| Delete           | Delete the character under the cursor.
| Alt-D            | Delete the current line.
| Alt-K            | Delete characters to right of cursor on current line.
| Alt-I            | Toggle insert and overwrite modes.
| Ctrl-K           | Delete word to left of the cursor.
| Ctrl-L           | Delete word to right of the cursor.
| Ctrl-F           | Format block.
| F10:set spell    | Enable spell checking
| F10:center       | Center the current line.

## Command Line

```
Command:
```

The command prompt shall also become active when user input is requiredby an interactive command, for example Goto Line.

```
Go to Line:
```

The command line has full editing features.
The *Left* and *Right* cursor keys allows navigation within the prompt, permitting text to be inserted and deleted.
Whilst at the prompt context specific help is available using ⟨Alt-H⟩ display help related to the current command.

Up to the last sixteen responses are saved for each prompt displayed; single character responses are ignored and not saved.  Use the cursor
*Up* and *Down* keys to cycle through the list of remembered responses. 
The *Esc*, which cancels the command is not stored.

You can also recall the last response entered at any prompt using ``Alt-L``. 
This is useful when you find you entered the correct responseto the wrong prompt.
Press ⟨Esc⟩ to cancel the first command, issue the ew command, and press ⟨Alt-L⟩ to recall the last response.

Certain commands for example Edit File can take advantage of the file name completion feature. 
Type the first letters of the name and press ⟨Tab⟩.
If only one file in the directory matches, it is completed.
If more than one file is found, a selection dialog is displayed allowing selection of the desired file.

During any command line interaction the available key binding are the following.

| Key              | Description
|:-----------------|:-------------------------------------------------------------------------------
| Enter            | Execute the current command.
| Esc              | Cancel the input.
| Home, End        | Moves the cursor to the head or tail of the text.
| Left, Right      | Moves the cursor within the input text.
| Del              | Delete the current character.
| Backspace        | Delete previous character
| Ctrl-Left        | Move left one word at a time.
| Ctrl-Right       | Move right one word at a time.
| Up, Down         | Navigate history; up/down scrolls through previous responses to the prompt.
| Ins              | Paste scape buffer.
| Tab              | File completion.
| Alt-B            | Current buffer name.
| Alt-F            | Current path and file name.
| Alt-L            | Recalls the last response that was typed by the user.
| Alt-P            | Paste text under cursor.
| Alt-W            | History buffer.
| Alt-I            | Toggle between insert and over-type mode.
| Alt-Q            | Quote next input character.  This allows user to type things like ``Tab`` and ``Esc`` as part of the input text.

## Searching and replace

Text are be manipulated by searching and/or translating selected text by the use of Regular expression search patterns.

| Key              | Description                                                
|:-----------------|:-------------------------------------------------------------------------------
| F5, Alt-S        | Search in a forward direction.  The user is prompted for the search item.
| Alt-F5, Alt-Y    | Search in a backwards direction.  The user is prompted for the search item.
| KP-5, Shift-F5   | Search for the next occurrence of an item in either the  forward or backwards direction, depending on the last search.
| F6, Alt-T        | Performs a replace in the forwards direction.  Prompts the  user for a item to search for _translate_ and an item to replace it with.  For each matched occurrence of the item, the user is prompted for whether to change or not.
| Alt-F6           | Performs a replace in the backwards direction.
| Shift-F6         | Repeats last replace in the same direction.
| Ctrl-F5          | Toggles the case sensitivity.  The default is for case sensitivity to be turned on.  When turned off, lower case characters match against upper case and vice versa.
| Alt-O            | Options menu, permits access to the global word processing options.
| F10:bufinfo      | Buffer information dialog, configures the buffer options.

Search patterns are expressed in terms of a Regular expression.
Regular expressions are special characters in search or translate strings that let you specify character patterns to match, instead of just sequences of literal characters.

Regular expression characters are similar to shell wild-cards, yet are far more powerful.  There are several supported expression syntaxes, with the original BRIEF syntax being the default.

These are the BRIEF regular expressions.

| Expression       | Matches
|:-----------------|:-------------------------------------------------------------------------------
| ?                | Any character except a newline.
| \*               | Zero or more characters (except newlines).
| \\t              | Tab character.
| \\n              | Newline character.
| \\c              | Position cursor after matching.
| \\\\             | Literal backslash.
| ⟨ or %           | Beginning of line.
| ⟩ or $           | End of line.
| @                | Zero or more of last expression.
| \+               | One or more of last expression.
| \|               | Either last or next expression.
| {}               | Define a group of expressions.
| \[ \]            | Any one of the characters inside \[ \].
| \[\~ \]          | Any character except those in \[\~ \].
| \[a-z\]          | Any character between a and z, inclusive.

## Cut and Paste

If we want to copy a block of text from one part of a buffer (think of a buffer as a file) to another, or from one buffer to another, we use the *scrap*.
The scrap is a temporary storage area for text which has been cut or copied from a buffer and can then be inserted as many times as necessary to some other buffer.
Cutting text is different from deleting text.
Cutting text deletes the original text but saves it so it can be inserted elsewhere. 
The deleted text is gone.

In order for a piece of text to be cut/copied and then pasted it must first be highlighted.
To highlight a region of text, the user first drops an anchor.
As the cursor is moved away from the anchor, the text between where the anchor was dropped and the current cursor position is highlighted, showing the text which can be cut or copied.

There are three types of regions, block, column and line.
Block and line type is used to cut/copy and paste whole lines. 
A column type isused to cut/copy and paste columns of text.

| Key              | Description                                                      
|:-----------------|:-------------------------------------------------------------------------------
| Alt-M            | Drops a normal block.  Text falling within the current cursor  position and from where the original anchor was dropped shall be highlighted
| Alt-C            | Drops a column marker.  Text falling within a rectangular region from where the anchor was dropped to the current cursor will be highlighted
| Alt-L            | Drop a line marker.  Text falling within a rectangular region  from where the anchor was dropped to the current cursor will be highlighted
| Alt-A            | Drops an inclusive block, similar to a normal block.
| KP-Plus          | If no region is currently highlighted, then the current line is coped to the scrap buffer.  If there is a highlighted region, then that region is copied to the scrap without being deleted
| KP-Minus         | If no region is currently highlighted, then the current line is cut to the scrap buffer.  If there is a highlighted region, then that region is copied to the scrap and deleted
| Ins              | Paste the contents of the scrap buffer into the current buffer at the current cursor position.  For line-types regions, the lines are inserted before the current line rather than inserting where the cursor is.
| Ctrl-O           | Search options.

## Undo/Redo

The ⟨undo⟩ command can be used to undo any commands in the current buffer.

The undo facility can be compared to an edit audit trail, which tracks all modifications to the buffer, including edits, marked regions and cursor movement.
Each buffer records changes within the scope of the current editor session independent of other buffers, with an infinite level of undo information for each buffer.

The undo command reverses recent changes in the buffer’s text, and the undo command always applies to the current buffer. 
Commands can be undone sequentially all the way back to the point where the buffer was pened or created. 
If you undo too much you can use ⟨redo⟩ to cancel the last undo.

| Key              | Description 
|:-----------------|:-------------------------------------------------------------------------------
| Alt-U,KP-star    | Undoes previously executed command.
| Ctrl-U           | Redoes the previous undo.

## Additional

There are numerous additional features available, many are directly available via the *Feature Menus* ⟨Alt-F⟩ and/or general menu, these include.

| Key              | Description
|:-----------------|:-------------------------------------------------------------------------------
| Alt-F            | Features menu.
| Alt-G            | Function list.
| Alt-V            | Display build information at the prompt; see \<version>
| Ctrl-O           | Options menu.
| Ctrl-A           | Extra menu
| F10:bufinfo      | Buffer information; see below.
| F10:coloriser    | Load a new content coloriser.
| F10:colorscheme  | View coloriser definition.
| F10:about        | About dialog.
| F12              | Menu; enable the pull-down command menu.  Can also be access using the *menu* command and disabled running the *menuoff* command.

![Example2](https://github.com/adamyg/grief/blob/master/hlpdoc/examples/Example2.png?raw=true)

Other Resources:
--------------------------------

Externally reviewed information regarding the development of BRIEF, can be found at the following

    * http://en.wikipedia.org/wiki/Brief_(text_editor)

Details about the commercially supported BRIEF clone CRisPEdit(tm), can be found at the following.

    * <http://www.crispeditor.co.uk> previously <http://www.crisp.demon.co.uk>

    * <http://www.crisp.com>


last update: May/24
-end-
