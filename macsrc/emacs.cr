/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: emacs.cr,v 1.2 2018/10/11 22:21:31 cvsuser Exp $
 * EMACS emulation - alpha.
 *
 *
    Keyboard binding:

        C-SP        set-mark-command                C-q         quoted-insert
        C-a         beginning-of-line               C-r         isearch-backward
        C-b         backward-char                   C-s         isearch-forward
        C-c         exit-recursive-edit             C-t         transpose-chars
        C-d         delete-char                     C-u         universal-argument
        C-e         end-of-line                     C-v         scroll-up
        C-f         forward-char                    C-w         kill-region
        C-h         help-command                    C-x         Control-X-prefix
        TAB         indent-for-tab-command          C-y         yank
        LFD         newline-and-indent              C-z         suspend-emacs
        C-k         kill-line                       ESC         ESC-prefix
        C-l         recenter                        C-]         abort-recursive-edit
        RET         newline                         C-_         undo
        C-n         next-line                       SPC .. ~    self-insert-command
        C-o         open-line                       DEL         delete-backward-char
        C-p         previous-line

        C-x C-a     add-mode-abbrev                 C-x 5       split-window-horizontally
        C-x C-b     list-buffers                    C-x ;       set-comment-column
        C-x C-c     save-buffers-kill-emacs         C-x <       scroll-left
        C-x C-d     list-directory                  C-x =       what-cursor-position
        C-x C-e     eval-last-sexp                  C-x >       scroll-right
        C-x C-f     find-file                       C-x [       backward-page
        C-x C-h     inverse-add-mode-abbrev         C-x ]       forward-page
        C-x TAB     indent-rigidly                  C-x ^       enlarge-window
        C-x C-l     downcase-region                 C-x `       next-error
        C-x C-n     set-goal-column                 C-x a       append-to-buffer
        C-x C-o     delete-blank-lines              C-x b       switch-to-buffer
        C-x C-p     mark-page                       C-x d       dired
        C-x C-q     toggle-read-only                C-x e       call-last-kbd-macro
        C-x C-r     find-file-read-only             C-x f       set-fill-column
        C-x C-s     save-buffer                     C-x g       insert-register
        C-x C-t     transpose-lines                 C-x h       mark-whole-buffer
        C-x C-u     upcase-region                   C-x i       insert-file
        C-x C-v     find-alternate-file             C-x j       register-to-dot
        C-x C-w     write-file                      C-x k       kill-buffer
        C-x C-x     exchange-dot-and-mark           C-x l       count-lines-page
        C-x C-z     suspend-emacs                   C-x m       mail
        C-x ESC     repeat-complex-command          C-x n       narrow-to-region
        C-x $       set-selective-display           C-x o       other-window
        C-x (       start-kbd-macro                 C-x p       narrow-to-page
        C-x )       end-kbd-macro                   C-x q       kbd-macro-query
        C-x +       add-global-abbrev               C-x r       copy-rectangle-to-register
        C-x -       inverse-add-global-abbrev       C-x s       save-some-buffers
        C-x .       set-fill-prefix                 C-x u       advertised-undo
        C-x /       dot-to-register                 C-x w       widen
        C-x 0       delete-window                   C-x x       copy-to-register
        C-x 1       delete-other-windows            C-x {       shrink-window-horizontally
        C-x 2       split-window-vertically         C-x }       enlarge-window-horizontally
        C-x 4       ctl-x-4-prefix                  C-x DEL     backward-kill-sentence

        C-x 4 C-f   find-file-other-window          C-x 4 d     dired-other-window
        C-x 4 .     find-tag-other-window           C-x 4 f     find-file-other-window
        C-x 4 b     pop-to-buffer                   C-x 4 m     mail-other-window

        ESC C-SP    mark-sexp                       ESC =       count-lines-region
        ESC C-a     beginning-of-defun              ESC >       end-of-buffer
        ESC C-b     backward-sexp                   ESC @       mark-word
        ESC C-c     exit-recursive-edit             ESC O       ??
        ESC C-d     down-list                       ESC [       backward-paragraph
        ESC C-e     end-of-defun                    ESC \       delete-horizontal-space
        ESC C-f     forward-sexp                    ESC ]       forward-paragraph
        ESC C-h     mark-defun                      ESC ^       delete-indentation
        ESC LFD     indent-new-comment-line         ESC a       backward-sentence
        ESC C-k     kill-sexp                       ESC b       backward-word
        ESC C-n     forward-list                    ESC c       capitalize-word
        ESC C-o     split-line                      ESC d       kill-word
        ESC C-p     backward-list                   ESC e       forward-sentence
        ESC C-s     isearch-forward-regexp          ESC f       forward-word
        ESC C-t     transpose-sexps                 ESC g       fill-region
        ESC C-u     backward-up-list                ESC h       mark-paragraph
        ESC C-v     scroll-other-window             ESC i       tab-to-tab-stop
        ESC C-w     append-next-kill                ESC j       indent-new-comment-line
        ESC ESC     ??                              ESC k       kill-sentence
        ESC C-\     indent-region                   ESC l       downcase-word
        ESC SPC     just-one-space                  ESC m       back-to-indentation
        ESC !       shell-command                   ESC q       fill-paragraph
        ESC $       spell-word                      ESC r       move-to-window-line
        ESC %       query-replace                   ESC t       transpose-words
        ESC '       abbrev-prefix-mark              ESC u       upcase-word
        ESC (       insert-parentheses              ESC v       scroll-down
        ESC )       move-past-close-and-reindent    ESC w       copy-region-as-kill
        ESC ,       tags-loop-continue              ESC x       execute-extended-command
        ESC -       negative-argument               ESC y       yank-pop
        ESC .       find-tag                        ESC z       zap-to-char
        ESC 0 ..    ESC 9  digit-argument           ESC |       shell-command-on-region
        ESC ;       indent-for-comment              ESC ~       not-modified
        ESC <       beginning-of-buffer             ESC DEL     backward-kill-word

        C-h v       describe-variable               C-h d       describe-function
        C-h w       where-is                        C-h k       describe-key
        C-h t       help-with-tutorial              C-h c       describe-key-briefly
        C-h s       describe-syntax                 C-h b       describe-bindings
        C-h n       view-emacs-news                 C-h a       command-apropos
        C-h C-n     view-emacs-news                 C-h C-d     describe-distribution
        C-h m       describe-mode                   C-h C-c     describe-copying
        C-h l       view-lossage                    C-h ?       help-for-help
        C-h i       info                            C-h C-h     help-for-help
        C-h f       describe-function
 *
 */

#include "grief.h"

static void         emacs_ua();
static void         emacs_ck(string args);
static void         emacs_xk(string args);
static void         emacs_ek(string args);

static int          emacs_argument = -1;

void
main()
{
    return;
    assign_to_key("<Ctrl-Space>", "emacs_ck \" \"");
    assign_to_key("<Ctrl-A>", "emacs_ck \"a\"");
    assign_to_key("<Ctrl-B>", "emacs_ck \"b\"");
    assign_to_key("<Ctrl-C>", "emacs_ck \"c\"");
    assign_to_key("<Ctrl-D>", "emacs_ck \"d\"");
    assign_to_key("<Ctrl-E>", "emacs_ck \"e\"");
    assign_to_key("<Ctrl-F>", "emacs_ck \"f\"");
    assign_to_key("<Ctrl-H>", "emacs_ck \"h\"");
    assign_to_key("<Ctrl-K>", "emacs_ck \"k\"");
    assign_to_key("<Ctrl-L>", "redraw");
    assign_to_key("<Ctrl-N>", "emacs_ck \"n\"");
    assign_to_key("<Ctrl-O>", "emacs_ck \"o\"");
    assign_to_key("<Ctrl-P>", "emacs_ck \"p\"");
    assign_to_key("<Ctrl-Q>", "emacs_ck \"q\"");
    assign_to_key("<Ctrl-R>", "emacs_ck \"r\"");
    assign_to_key("<Ctrl-S>", "emacs_ck \"s\"");
    assign_to_key("<Ctrl-T>", "emacs_ck \"t\"");
 // assign_to_key("<Ctrl-U>", "emacs_ua");
    assign_to_key("<Ctrl-V>", "emacs_ck \"v\"");
    assign_to_key("<Ctrl-W>", "emacs_ck \"w\"");
    assign_to_key("<Ctrl-Y>", "emacs_ck \"y\"");
 // assign_to_key("<Ctrl-]>", "emacs_ck \"]\"");
    assign_to_key("<Ctrl-_>", "emacs_ck \"_\"");
 // assign_to_key("<Esc>",    "emacs_esc");
 // assign_to_key("<Ctrl-X>", "emacs_ck");
}


/*
 *  emacs_ua ---
 *      universal-argument

    In the terminology of mathematics and computing, argument means “data provided to a function
    or operation. You can give any Emacs command a numeric argument (also called a prefix
    argument). Some commands interpret the argument as a repetition count. For example, giving
    C-f an argument of ten causes it to move point forward by ten characters instead of one. With
    these commands, no argument is equivalent to an argument of one, and negative arguments cause
    them to move or act in the opposite direction.

    The easiest way to specify a numeric argument is to type a digit and/or a minus sign while
    holding down the <META> key. For example,

       M-5 C-n

    moves down five lines. The keys M-1, M-2, and so on, as well as M--, are bound to commands
    (digit-argument and negative-argument) that set up an argument for the next command. M--
    without digits normally means -1.

    If you enter more than one digit, you need not hold down the <META> key for the second and
    subsequent digits. Thus, to move down fifty lines, type

       M-5 0 C-n

    Note that this does not insert five copies of '0' and move down one line, as you might expect
    the '0' is treated as part of the prefix argument.

    (What if you do want to insert five copies of '0'? Type M-5 C-u 0. Here, C-u 'terminates' the
    prefix argument, so that the next keystroke begins the command that you want to execute. Note
    that this meaning of C-u applies only to this case. For the usual role of C-u, see below.)

    Instead of typing M-1, M-2, and so on, another way to specify a numeric argument is to type
    C-u (universal-argument) followed by some digits, or (for a negative argument) a minus sign
    followed by digits. A minus sign without digits normally means -1.

    C-u alone has the special meaning of 'four times': it multiplies the argument for the next
    command by four. C-u C-u multiplies it by sixteen. Thus, C-u C-u C-f moves forward sixteen
    characters. Other useful combinations are C-u C-n, C-u C-u C-n (move down a good fraction of
    a screen), C-u C-u C-o (make 'a lot' of blank lines), and C-u C-k (kill four lines).

    You can use a numeric argument before a self-inserting character to insert multiple copies of
    it. This is straightforward when the character is not a digit; for example, C-u 6 4 a inserts
    64 copies of the character ‘a’. But this does not work for inserting digits; C-u 6 4 1
    specifies an argument of 641. You can separate the argument from the digit to insert with
    another C-u; for example, C-u 6 4 C-u 1 does insert 64 copies of the character '1'.

    Some commands care whether there is an argument, but ignore its value. For example, the
    command M-q (fill-paragraph) fills text; with an argument, it justifies the text as well.
    (See Filling, for more information on M-q.) For these commands, it is enough to specify the
    argument with a single C-u.

    Some commands use the value of the argument as a repeat count, but do something special when
    there is no argument. For example, the command C-k (kill-line) with argument n kills n lines, 
    including their terminating newlines. But C-k with no argument is special: it kills the text
    up to the next newline, or, if point is right at the end of the line, it kills the newline
    itself. Thus, two C-k commands with no arguments can kill a nonblank line, just like C-k with
    an argument of one. (See Killing, for more information on C-k.)

    A few commands treat a plain C-u differently from an ordinary argument. A few others may
    treat an argument of just a minus sign differently from an argument of -1. These unusual
    cases are described when they come up; they exist to make an individual command more
    convenient, and they are documented in that command's documentation string.

    We use the term prefix argument to emphasize that you type such arguments before the command, 
    and to distinguish them from minibuffer arguments (see Minibuffer), which are entered after
    invoking the command. */

static int
argval(string val, int accum)
{
    accum = (accum > 0 || accum < -1 ? abs(accum) : 0);

    if (strlen(val)) {
        if (1 == strlen(val) && characterat(val, 1) == '-') {
            return -1;                          // single '-' == -1
        }
        return atoi(val) + accum;
    }
    return accum;
}


void
emacs_ua()
{
    int accum = 0, prompt = 0, ch;
    string value;

    emacs_argument = 0;
    while (1) {

        if (prompt) {
            if (accum) {
                if (accum < 0) {                // repeat self-insert
                    message("Ctrl-u %s Ctrl-u", value);
                } else {
                    message("Ctrl-u(%d) %s", accum, value);
                }
            } else {
                message("Ctrl-u %s", value);
            }
        }

        if ((ch = read_char(prompt ? 2500 : 1000)) < 0) {
            ++prompt;
            continue;
        }

        if (accum < 0) {                        // repeat self-insert
            message("");
            //
            //  A numeric argument before a self-inserting character to 
            //  insert multiple copies of it.
            //
            if (isprint(ch)) {
                int argument = argval(value, accum);
                while (argument-- > 0) {
                    self_insert(ch);
                }
            } else {
                beep();
            }
            return;
        }

        if (isdigit(ch) || (ch == '-' && 0 == strlen(value))) {
            //
            //  argument value
            //
            value += format("%c", ch);
            accum = 0;

        } else {
            string key = int_to_key(ch);

            switch (key) {
            case "<Ctrl-G>":
            case "<Esc>":
                message("Quit");
                return;

            case "<Ctrl-H>":
                explain("emacskeys");
                break;

            case "<Ctrl-U>":
                //
                //  C-u specials.
                //
                if (0 == strlen(value)) {       // multiplier
                    //  C-u alone has the special meaning of 'four times'; it multiplies the
                    //  argument for the next command by four. C-u C-u multiplies it by sixteen.
                    //
                    if (accum) {
                        accum *= 4;
                    } else {
                        accum = 4;
                    }
                } else {                        // repeat self-insert
                    //  You can separate the argument from the digit to insert with another C-u; for
                    //  example, C-u 64 C-u 1 does insert 64 copies of the character '1'.
                    //
                    if (accum) {
                        accum *= -1;
                    } else {
                        accum = -1;
                    }
                }
                break;

            case "<Backspace>":
            case "<Del>":
                //
                //  Simple edit controls.
                //
                value = substr(value, 1, strlen(value)-1);
                break;

            default: {
                    int argument = argval(value, accum);

                    message("");
                    if (isprint(ch)) {          // printable() self-insert
                        while (argument-- > 0) {
                            self_insert(ch);
                        }
                        return;
                    }
                    emacs_argument = argument;
                    push_back(ch);              // re-execute key
                }
                return;
            }
        }
    }
}


static void
emacs_ck(string arg)
{
    const int count = 
        (emacs_argument > 1 ? emacs_argument : 1);

    switch (arg) {
    case " ":   // set-mark-command
        break;
    case "a":   // beginning-of-line
        beginning_of_line();
        break;
    case "b":   // backward-char
        left(count);
        break;
    case "c":   // exit-recursive-edit
        break;
    case "d":   // delete-char
        break;
    case "e":   // end-of-line
        end_of_line();
        break;
    case "f":   // forward-char
        right(count);
        break;
    case "h":   // help-command
        help();
        break;
    case "k":   // kill-line
        break;
    case "l":   // recenter
        break;
    case "n":   // next-line
        down(count);
        break;
    case "o":   // open-line
        break;
    case "p":   // previous-line
        up(count);
        break;
    case "q":   // quoted-insert
        break;
    case "r":   // isearch-backward
        search__back();
        break;
    case "s":   // isearch-forward
        search__fwd();
        break;
    case "t":   // transpose-chars
        break;
    case "v":   // scroll-up
        break;
    case "w":   // kill-region
        cut();
        break;
    case "y":   // yank
        paste();
        break;
    case "z":   // suspend-emacs
        break;
        break;
    case "]":   // abort-recursive-edit
        break;
    case "_":   // undo
        undo();
        break;
    }
    emacs_argument = -1;
}


static void
emacs_xk(string args)
{
}


static void
emacs_ek(string args)
{
}
/*end*/
