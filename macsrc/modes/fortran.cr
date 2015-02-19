/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: fortran.cr,v 1.9 2014/10/22 02:34:33 ayoung Exp $
 * FORTRAN mode.
 *
 *
 */

#include "../grief.h"
#include "../mode.h"

#define ALIGN_CODE              0               /* Where to indent comments. */
#define ALIGN_CMT               1
#define ALIGN_COL               2

#define KW_LOWER                0
#define KW_UPPER                1
#define KW_MIXED                2

                                                /* Skip label text */
#define FOR_SKIP_LABEL          "{<[ ]@[a-zA-Z0-9]+:[ \t]@\\c[~ \t]}|{<[0-9 \t]@\\c[~ \t]}"
#define FOR_LABEL               "<[ ]@[0-9]\\c[ \t]}"
#define FOR_CMT_CHAR            "C"             /* Preferred comment character is C. */
#define FOR_CMT_LIST            "!Cc*$"         /* Legal comment chars are listed. */
#define FOR_CONT_CHAR           "+"             /* Continuation character is +. */
                                                /* Skip cont */
#define FOR_SKIP_CONT           "<[ \t]+\\[1-9+*][ \t]+@\\c[~ ]}"
#define FOR_CONT_LIST           "123456789+*"   /* Legal continuation chars. */

#define FOR_CMT_COL             1               /* Comment char is in col 1.*/
#define FOR_LABEL_COL           1               /* Labels may start in col 2. */
#define FOR_LABEL_WIDTH         4
#define FOR_CONT_COL            6               /* Continuation char is in col 6. */
#define FOR_TEXT_COL            7               /* Text starts in col 7. */
#define FOR_CMT_CONT_COL        7               /* Continue comments at col 7. */

#define FOR_RIGHT_MARGIN        73              /* Right margin is at col 72. */
#define FOR_CMT_MARGIN          81              /* Comment margin is at col 80. */

#define FOR90_LABEL             "<[ ]@[a-zA-Z0-9]+\\c:}"
#define FOR90_RIGHT_MARGIN      132
#define FOR90_CMT_CHAR          "!"
#define FOR90_CONT_CHAR         "&"

#define MIN_ABBREV              2

int                 _f_token(string code, ~string);
string              _f_indent_prev(int what);
int                 _f_indent_next(string part1, string part2);
int                 _f_indent_level(/*[column] */);
int                 _f_indent_pos(int level);
string              _f_reindent_label (int level, string text);
void                _f_reindent(int level);
int                 _f_iscomment(void);
int                 _f_iscont(void);
void                _f_comment(void);
void                _f_wrap(void);
int                 _f_split(int max_split_col);
void                _f_abbrev(void);
void                _f_expand_end(string end);
static void         _f_insert(string cmd);
static string       _f_case(string keyword);

static int          _f_smart,                   /* keyboards */
                    _f_template;

static int          _f_comment_align,
                    _f_cont_align = 2,
                    _f_min_abbrev = 2,
                    _f_keyword_case = KW_LOWER,
                    _f_freestyle = 1;

static string       _f_comment_char;

#define K_IF                    1               /* see _f_token() */
#define K_THEN                  2
#define K_ELSE                  3
#define K_ELSEIF                4
#define K_DO                    5
#define K_CONTINUE              6
#define K_SELECT                7
#define K_CASE                  8
#define K_END                   99

static list         _f_keywords = {
#define KWL_FIELDS              4
#define KWL_ABBREV              0
#define KWL_COMPL               1
#define KWL_FLAGS               2
#define KWL_POSC                3

#define DO_END                  0x01
#define DO_DO                   0x02
#define DO_INDENT               0x04
#define DO_INDENT0              0x08

        "BLOCK DATA",   "",                 DO_END,     0,
        "CASE",         "CASE ()",          0,          -1,
        "CHARACTER",    "",                 0,          0,
        "COMMON",       "",                 0,          0,
        "CONTINUE",     "",                 DO_INDENT,  0,
        "DATA",         "",                 0,          0,
        "DO",           "",                 DO_DO,      0,
        "ELSE",         "",                 DO_INDENT,  0,      /*order*/
        "ELI",          "ELSE IF () THEN",  DO_INDENT0, -6,
        "ELSEIF",       "ELSEIF () THEN",   DO_INDENT0, -6,
        "FORMAT",       "",                 0,          0,
        "FUNCTION",     "",                 DO_END,     0,
        "IF ",          "IF () THEN",       DO_END,     -6,
        "INCLUDE",      "INCLUDE ''",       0,          -1,
        "INTEGER",      "",                 0,          0,
        "LOGICAL",      "",                 0,          0,
        "PROGRAM",      "",                 DO_END,     0,
        "READ",         "",                 0,          0,
        "RETURN",       "",                 0,          0,
        "SUBROUTINE",   "",                 DO_END,     0,
        "WRITE",        "",                 0,          0,

        "FORALL",       "FORALL ()",        DO_END,     -1,     /*f95*/
        "SELECT",       "SELECT CASE ()",   DO_END,     -1,     /*f90*/
        "TYPE",         "",                 DO_END,     0,      /*f90*/
        "WHERE",        "WHERE ()",         DO_END,     -1,     /*f95*/
        };


#if defined(__PROTOTYPES__)
static int              _f_indent(~ int);
static void             _f_insert(string);
static string           _f_case(string);
static void             _f_wrap(void);
static void             _f_expand_end(string end);
static void             _f_first_nonwhite(void);
#endif

#define MODENAME        "FORTRAN"


/*
 *  main ---
 */
void
main()
{
    create_syntax(MODENAME);

    /*
     *  Standard syntax engine
     */
    syntax_token(SYNT_COMMENT,      "!");
    syntax_token(SYNT_BRACKET,      "([", ")]");
    syntax_token(SYNT_CHARACTER,    '\'');
    syntax_token(SYNT_STRING,       '"');
    syntax_token(SYNT_WORD,         "0-9A-Z_a-z");
    syntax_token(SYNT_NUMERIC,      "-+0-9eEdD");
    syntax_token(SYNT_DELIMITER,    ",.");
    syntax_token(SYNT_OPERATOR,     "-+/*=<>");

    /*
     *  Options/
     *      case insensitive keywords
     *      enable fortran specials
     */
    set_syntax_flags(SYNF_CASEINSENSITIVE|SYNF_FORTRAN);

    /*
     *  Keywords
     *
     *  Fortran 77 keywords + include, record, structure, while:
     */
    define_keywords(SYNK_PRIMARY,   "do,go,if,to", -2);
    define_keywords(SYNK_PRIMARY,   "end", -3);
    define_keywords(SYNK_PRIMARY,   "call,data,else,exit,goto,open,read,real,save,stop,then", -4);
    define_keywords(SYNK_PRIMARY,   "block,close,enddo,endif,entry,pause,print,while,write", -5);
    define_keywords(SYNK_PRIMARY,   "common,double,elseif,format,record,return,rewind", -6);
    define_keywords(SYNK_PRIMARY,   "complex,endfile,include,inquire,integer,logical,program", -7);
    define_keywords(SYNK_PRIMARY,   "continue,external,function,implicit", -8);
    define_keywords(SYNK_PRIMARY,   "backspace,character,dimension,intrinsic,parameter,precision,structure", -9);
    define_keywords(SYNK_PRIMARY,   "subroutine", -10);
    define_keywords(SYNK_PRIMARY,   "equivalence", -11);
    define_keywords(SYNK_OPERATOR,  "eq,ge,gt,le,lt,ne,or,and,not");
    define_keywords(SYNK_BOOLEAN,   "true,false");

    /*
     *  Extensions for Fortran 90.
     */
    define_keywords(SYNK_EXTENSION, "use", -3);
    define_keywords(SYNK_EXTENSION, "case,kind,type", -4);
    define_keywords(SYNK_EXTENSION, "cycle,where", -5);
    define_keywords(SYNK_EXTENSION, "intent,module,public,select,target", -6);
    define_keywords(SYNK_EXTENSION, "endtype,nullify,pointer,private", -7);
    define_keywords(SYNK_EXTENSION, "allocate,contains,endwhere,namelist,optional,sequence", -8);
    define_keywords(SYNK_EXTENSION, "elsewhere,endmodule,endselect,interface,recursive", -9);
    define_keywords(SYNK_EXTENSION, "deallocate,endprogram,selectcase", -10);
    define_keywords(SYNK_EXTENSION, "allocatable,endfunction", -11);
    define_keywords(SYNK_EXTENSION, "endblockdata,endinterface", -12);
    define_keywords(SYNK_EXTENSION, "endsubroutine", -13);
    define_keywords(SYNK_EXTENSION, "moduleprocedure", -15);

    //  I/O functions
    define_keywords(SYNK_FUNCTION,
        "access," +
        "backspace," +
        "blank," +
        "close," +
        "direct," +
        "endfile," +
        "err," +
        "exist," +
        "file," +
        "fmt," +
        "form," +
        "formatted," +
        "inquire," +
        "iostat," +
        "name," +
        "named," +
        "nextrec," +
        "number," +
        "open," +
        "opened," +
        "print," +
        "read," +
        "rec," +
        "recl," +
        "rewind," +
        "sequential," +
        "status," +
        "unformatted," +
        "unit," +
        "write"
        );

    //  Mathematical functions
    define_keywords(SYNK_FUNCTION,
        "abs," +
        "acos," +
        "aimag," +
        "aint," +
        "alog," +
        "alog10," +
        "amax0," +
        "amax1," +
        "amin0," +
        "amin1," +
        "amod," +
        "anint," +
        "aprime," +
        "asin," +
        "atan," +
        "atan2," +
        "acos," +
        "cabs," +
        "cexp," +
        "char," +
        "clog," +
        "cmplx," +
        "conjg," +
        "cos ," +
        "cosh," +
        "ccos," +
        "csin," +
        "csqrt," +
        "dabs," +
        "dacos," +
        "dasin," +
        "datan," +
        "datan2," +
        "dble," +
        "dcos," +
        "dcosh," +
        "dfloat," +
        "ddmim," +
        "dexp," +
        "dim," +
        "dint," +
        "dlog," +
        "dlog10," +
        "dmax1," +
        "dmin1," +
        "dmod," +
        "dnint," +
        "dsign," +
        "dsin," +
        "dsinh," +
        "dsqrt," +
        "dtan," +
        "dtanh," +
        "equivalence," +
        "exp," +
        "float," +
        "iabs," +
        "ichar," +
        "idim," +
        "idint," +
        "ifix," +
        "index," +
        "int," +
        "isign," +
        "len," +
        "log," +
        "log10," +
        "max," +
        "max0," +
        "max1," +
        "min," +
        "min0," +
        "min1," +
        "mod," +
        "rand," +
        "sign," +
        "sin," +
        "sinh," +
        "sngl," +
        "sqrt," +
        "tan," +
        "tanh"
        );

    /*
     *  Extensions for Fortran 95.
     */

    /*
     *  Keyboard
     */
    load_indent();
    keyboard_push();
    assign_to_key("<Enter>",        "_f_indent");
    assign_to_key("<Tab>",          "_slide_in");
    assign_to_key("<Shift-Tab>",    "_slide_out");
    assign_to_key("<Space>",        "_f_abbrev");
    _f_template = inq_keyboard();
    keyboard_pop(1);
}


/*
 *  Modeline/package support
 */
string
_f_mode()
{
    return "f";                             /* return primary package extension */
}


string
_fortran_mode()
{
    return _f_mode();
}


string
_f77_mode()
{
    return _f_mode();
}


string
_f90_mode()
{
    return _f_mode();
}


string
_f95_mode()
{
    return _f_mode();
}


string
_f_highlight_first()
{
    attach_syntax(MODENAME);
    return "";
}


/*  Function:   _f_smart_first ---
 *      Smart indenting initialisation.
 *
 *  Description:
 *      This macro is called by the BPACKAGES parser in language.cr
 *
 *      Turn on smart indenting for FORTRAN. This macro is designed to be run the first
 *      time a file is edited, but may also be run from the command line.
 *
 *      They initialize the local keymaps for the various indenting functions, set the
 *      abbreviation length and adjust the indenting style
 *
 *   Parameters:
 *      0 -     Controls where the cursor is left when we are
 *              continuing an existing comment.
 *
 *              0 - Leave the cursor at the current indent
 *                  level (flush with the last line of code).
 *
 *              1 - Leave it flush with the indent level of
 *                  the last comment.
 *
 *              2 - Means to leave it at FOR_CMT_CONT_COL.
 *
 *      1 -     The number of levels continuation lines should be indented
 *              relative to the start of the statement.
 */
string
_f_smart_first()
{
    use_tab_char("n");                          /* disable hard tabs */
    use_local_keyboard(_f_smart);
    return "";
}


/*  Function:   _f_smart_on
 *      Smart indenting enable.
 *
 *  Description:
 *      Turn on smart indenting for FORTRAN. This macro is designed to be run every
 *      time a FORTRAN file is edited.
 *
 *      It turns on statement wrap for FORTRAN, and forces empty areas to be filled
 *      with spaces.
 */
string
_f_smart_on()
{
    register_macro(REG_TYPED, "_f_wrap");
    returns ("_f_smart_off");
}


/*  Function:   _f_smart_off
 *       Smart indenting disable.
 *
 *  Description:
 *      Turn off smart indenting for FORTRAN. This macro is designed to be run every
 *      time a FORTRAN file is left.
 *
 *      Some functions must be deactivated every time we leave a FORTRAN file.
 *
 *      It turns off statement wrap for FORTRAN, and restores the tab fill setting.
 */
string
_f_smart_off()
{
    unregister_macro(REG_TYPED, "_f_wrap");
}


/*  Function:   _f_template_first
 *      Template indenting initialisation.
 *
 *  Descrition:
 *      This macro ise called by the BPACKAGES parser in language.cr
 *
 *      Turn on template indenting for FORTRAN. This macro is designed to be run the
 *      first time a file is edited, but may also be run from the command line.
 *
 *      They initialize the local keymaps for the various indenting functions, set the
 *      abbreviation length and adjust the indenting style
 *
 *  Parameters:
 *      0 -     See smart.
 *
 *      1 -     See smart.
 *
 *      2 -     The minimum prefix length required for template expansion.
 *              Set this parameter to 0 if you want to selectively expand
 *              templates by pressing <Tab>.
 *
 *      3 -     Controls case of expanded keywords.
 *
 *                  If this is 0, r<Space> expands to "return";
 *                  if 1, it expands to "RETURN";
 *                  and if 2, it expands to "Return".
 *
 *      4 -     END expansioNULL, either 'ENDIF' or 'END IF' (TODO).
 *
 **/
string
_f_template_first()
{
    _f_min_abbrev = 0;
    _f_keyword_case = KW_LOWER;
    get_parm(0, _f_min_abbrev);
    _f_min_abbrev = MIN_ABBREV;
    use_local_keyboard(_f_template);

    get_parm(0, _f_comment_align);
    get_parm(1, _f_cont_align);
    get_parm(2, _f_min_abbrev);
    get_parm(3, _f_keyword_case);

    return "";
}


/*  Function: _f_indent
 *      Smart editing macros.
 *
 *  Description;
 *      These macro performs simple smart editing for programs.
 *
 *      This automatically inserts a matching brace at the proper indentation level
 *      when an opening brace is typed in. To insert a brace without a matching brace
 *      (it attempts to be smart about matching braces, but you never can make this
 *      type of thing quite smart enough), type either Ctrl-{ or quote the brace with
 *      Alt-q.
 *
 *   Notes:
 *      _f_indent has two modes.
 *
 *      Both modes will reindent the current line if necessary and position the cursor
 *      correctly on the following line.
 *
 *      When GRIEF is in insert mode, or this macro is called from open_line, _f_indent
 *      will add a new line to the buffer. The contents depend on the cursor position
 *      when _f_indent is called: if the cursor is at the end of the line, the new line
 *      will be blank, but if not, the old line will be split in two. If the old line
 *      is a comment, the new line will be a comment, and if the old line is code, the
 *      new line will be either a new code line or a continuation line.
 *
 *      When GRIEF is in overstrike mode and the macro was not called by open_line,
 *      GRIEF does not add a new line. Note that open_line doesn't call the macro
 *      directly, but calls it via key assignment, which makes life difficult for us.
 */
int
_f_indent(~ int)
{
    int     curr_line,                          /* Line cursor is on when called. */
            curr_col,                           /* Current column.  */
            code_indent_level,                  /* Current indent column. */
            iscomment,                          /* Is this line a comment?. */
            iscont,                             /* Is this line a continuation?. */
            insmode,                            /* Should we insert a newline?. */
            insert_prefix = FALSE,              /* flag if insert in prefix area. */
            scratch;                            /* Scratch integer. */

    string  split_text,                         /* Remainder of line being split. */
            code_text,                          /* Trimmed text of code line. */
            prev_text,                          /* Trimmed text of previous line. */
            code_part,                          /* Just the code on the line. */
            end;

    inq_position (curr_line, curr_col);
    if (!get_parm (0, insmode)) {
        insmode = inq_mode();
    }

    /*
     *  If we're splitting, and not at the end of the line, we save the end
     *  of the line (minus its newline) and delete it.
     *
     *  Make sure we handle splits in the prefix area correctly.
     *
     *  If split in prefix area, yet not beginning of prefix area, split
     *  starting from the text column.
     * 
     *  If split at beginning of prefix area, set flag to later reinsert
     *  code in prefix area.
     */
    if (insmode && read (1) != "\n") {          /* eol? */
        if ( ! _f_freestyle ) {
            if ((curr_col < FOR_TEXT_COL) && (curr_col > 1)) {
                move_abs (0, FOR_TEXT_COL);

            } else if (curr_col == FOR_LABEL_COL) {
                insert_prefix = TRUE;
            }
        }
        split_text = rtrim(read(), "\n");       /* chomp */
        delete_to_eol();

    } else {
        end_of_line();
    }

    /* 
     *  Determine if the current line is a comment. Blank lines are commented
     *  because they are illegal in FORTRAN. A negative value for iscomment
     *  means we made the line into a comment.
     */
    beginning_of_line ();
    if ((trim (read ()) == "") || (curr_col == 1)) {
        delete_to_eol();
        _f_comment();
        beginning_of_line();
        iscomment = -1;
    } else {
        iscomment = _f_iscomment();
    }

    /*  
     *  Find the beginning of the last code statement, and remember the text
     *  of the line and its indent level.
     *
     *  Note: This should match the current line, if not the statement is
     *   syntactically invalid and the indent request shall be basicly ignored.
     */
    scratch = curr_line;
    end_of_line();

    if ((iscont = _f_iscont()) > 0) {
        if (re_search(SF_BACKWARDS|SF_MAXIMAL, FOR_SKIP_CONT) > 0 ||
                re_search(SF_BACKWARDS|SF_MAXIMAL, "<[ \t]+\\c[~ \t]}") > 0) {
            inq_position (scratch);
            code_indent_level = _f_indent_level();
            code_part = trim(read());
        }

    } else {
        if (re_search(SF_BACKWARDS|SF_MAXIMAL, FOR_SKIP_LABEL) > 0) {
            inq_position (scratch);
            code_indent_level = _f_indent_level();
            code_part = trim(read());
        }
    }
    move_abs(curr_line, 1);
    code_text = rtrim(read(), "\n");            /* chomp */

//  refresh();
//  message("text=%s,code=%s", code_text, code_part);

    /*
     *  If the line is not a comment, see if it needs to be reindented.
     */
    if (!iscomment) {
        /*
         *  If the line is a continuatioNULL, read the continuation text into
         *  code_part, and reindent _for_cont_align levels right of
         *  code_indent_level.
         */
        if (iscont) {
            prev_text = code_part;
            code_part = ltrim (substr (code_text, FOR_TEXT_COL));
            _f_reindent (code_indent_level + _f_cont_align);

        /*  
         *  If scratch is equal to curr_line, the line is a valid statement;
         *  if not, it's syntactically invalid.
         */
        } else if (scratch == curr_line) {
            /* 
             *  If the line lacks a label, but the text part of the line
             *  begins with a number, convert that to a label. This lets us
             *  enter labeled statements quickly.
             */
            if (atoi(code_text) &&
                    atoi(substr(code_text, FOR_LABEL_COL, FOR_TEXT_COL-1)) == 0) {
                code_text = _f_reindent_label (code_indent_level, code_text);
                code_part = trim (substr (code_text, FOR_TEXT_COL));
            }

            /*
             *  The following switch statement is the body of the indent
             *  macro indenting rules.
             *
             *  Match the statement against any previous statement at the
             *  same level.
             *
             */
            switch (_f_token(code_part, end)) {
            case K_ELSE:
            case K_ELSEIF:
                if ((scratch = _f_indent_prev(K_IF)) >= 0 ||
                            (scratch = _f_indent_prev(K_ELSE)) >= 0) {
                    _f_reindent (code_indent_level = scratch);
                }
                break;

            case K_CASE:
                if ((scratch = _f_indent_prev(K_SELECT)) >= 0 ||
                            (scratch = _f_indent_prev(K_CASE)) >= 0) {
                    _f_reindent (code_indent_level = scratch);
                }
                break;

            case K_END:
                if ((scratch = _f_indent_prev(K_END)) >= 0) {
                    _f_reindent (code_indent_level = scratch);
                }
                break;

            case K_CONTINUE:
                if (atoi(code_text)) {          /* labeled */
                    string temp;

                    /*  Search back for a DO statement with a matching label,
                     *  but stop at the start of a PROGRAM, FUNCTION or SUBROUTINE
                     *  unit.
                     */
                    sprintf(temp, "<[0-9 \t]@{\\cDO @%d}|{{PROGRAM}|{FUNCTION}|{SUBROUTINE}|{BLOCK DATA}}",
                        atoi (code_text));

                    if (re_search( SF_IGNORE_CASE|SF_BACKWARDS, temp ) &&
                            upper(read(3)) == "DO ") {
                        code_indent_level = _f_indent_level();
                    } else {
                        beep();                 /* warning, unknown label */
                    }
                    move_abs(curr_line, 1);

                } else {                        /* unlabeled ??? */
                    if ((scratch = _f_indent_prev(K_DO)) >= 0)
                        code_indent_level = scratch;
                }
                _f_reindent(code_indent_level);
                break;
            }
        }
    }

    //  Move to the next line, splitting if necessary using either
    //  code or comment continuation.
    //
    if (insmode) {
        end_of_line();

        if (_f_freestyle) {
            if (!iscomment && split_text != "") {
                int col;

                inq_position(NULL, col);
                if (col < FOR_CMT_COL)
                    move_abs (0, FOR_CMT_COL);  /* XXX - config ??? */
                insert(FOR90_CONT_CHAR);
            }
        }

        insert ("\n");

        if (iscomment) {
            if (iscomment > 0) {
                _f_comment();
            }
        } else if (split_text != "") {
            if (! _f_freestyle) {
                move_abs (0, FOR_CONT_COL);
                insert (FOR_CONT_CHAR);
            }
        }
    } else {
        beginning_of_line(); down();
    }

    //  Position the cursor on the next line. If the current line was a comment, the
    //  cursor position is controlled by _f_comment_align.
    //
    //  If the current line was code, we start with the same level, but indent if the
    //  statement contained certain keywords. However, if the new line is a
    //  continuatioNULL, the cursor goes at the beginning of the continued text.
    //
    if (iscomment) {
        switch (_f_comment_align) {
        case ALIGN_CODE: {                      /* code */
                code_indent_level += _f_indent_next (code_part, code_part);
                move_abs (0, _f_indent_pos(code_indent_level));
            }
            break;

        case ALIGN_CMT: {                       /* comment */
                save_position ();
                if (search_back("<[Cc*$] +\\c[~ ]", 1)) {
                    inq_position(NULL, code_indent_level);
                } else {
                    code_indent_level = FOR_CMT_CONT_COL;
                }
                restore_position ();
                move_abs(0, code_indent_level);
            }
            break;


        case ALIGN_COL:                         /* left */
            move_abs(0, FOR_CMT_CONT_COL);
            break;
        }

    } else {
        if (_f_iscont())
            code_indent_level += _f_cont_align;
        else
            code_indent_level += _f_indent_next (code_part, iscont ? prev_text : code_part);

        move_abs (0, _f_indent_pos(code_indent_level));
    }

    //  Reinsert any characters that were split.
    //
    scratch = 0;
    if (split_text != "") {
        /* Reinsert in the prefix area. */
        if (insert_prefix) {
            move_abs (0, FOR_LABEL_COL);
            save_position ();
            scratch = strlen (split_text);
            insert (split_text);
            restore_position ();

        /* Reinsert in the text area. */
        } else {
            save_position ();
            scratch = strlen (split_text);
            insert (split_text = ltrim (split_text));
            restore_position ();
            scratch -= strlen (split_text);
        }
    }
    returns (scratch);
}


/*  
 *  _f_token ---
 *      Tokenise the specified text. Only indentation level syntactically significant are handled.
 */
int
_f_token(string code, ~string)
{
    int pos, ret = -1;

    /* first word */
    code = ltrim(compress(upper(code)))+"  ";   /* note 2 spaces */
    pos = index(code, " ");
    switch (substr(code, 1, pos-1)) {
    case "IF":
        ret = K_IF;
        break;

    case "THEN":
        ret = K_THEN;
        break;

    case "ELSE":
        ret = K_ELSE;
        if (substr(code, 1, 7) == "ELSE IF") {
            ret = K_ELSEIF;
        }
        break;

    case "ELSEIF":
        ret = K_ELSEIF;
        break;

    case "DO":
        ret = K_DO;
        break;

    case "CONTINUE":
        ret = K_CONTINUE;
        break;

    case "SELECT":
        ret = K_SELECT;
        break;

    case "CASE":
        ret = K_CASE;
        break;

    case "END":
        ret = K_END;
        pos++;
        break;

    default:                /*END DO etc*/
        if (substr(code, 1, 3) == "END") {
            ret = K_END;
            pos = 4;
        }
    }

    /* second (and third ?????) word */
    if (ret == K_END) {
        code = substr(code, pos);
        code = substr(code, 1, index(code, " ")-1);
        put_parm(2, code);
    }
    return (ret);
}


/*  _f_indent_prev ---
 *      Finds the indentation level of previously syntactically significant line match 'what'.
 *
 *      Search back for a matching statement, but stop at the start of a PROGRAM, FUNCTION 
 *      or SUBROUTINE unit, since we can't have one of those * in the middle of a DO loop.
 */
string
_f_indent_prev(int what)
{
    int     indent = -1;
    int     curr_line, done;
    int     nesting, pos, token;
    string  line;

    save_position();
    inq_position(curr_line, NULL);
    for (done = FALSE; !done;) {
        /*
         *  Process each line, and find one with text that cannot be eliminated
         */
        up();
        while (!done && _f_iscomment()) {
            if (--curr_line <= 0)
                break;
            up();
        }
        if (--curr_line <= 0)
            break;

        beginning_of_line();
        line = read();

        /*
         *  Stop at significant markers
         */
        if ((pos = re_search(SF_MAXIMAL, FOR_SKIP_LABEL, line)) > 0) {
            token = _f_token(substr(line, pos));
            if (what == K_END) {
                /*
                 *  END current block, check all markers.
                 */
                int idx, len = length_of_list(_f_keywords);

                if (token == K_END) {
                    nesting++;                  /* block nesting?? */
                } else {
                    for (idx = 0; !done && idx < len;) {
                        if (re_search(SF_NOT_REGEXP|SF_IGNORE_CASE, _f_keywords[ idx ], line)) {
                            int flags = _f_keywords[ idx+KWL_FLAGS ];

                            if (flags & (DO_END|DO_DO)) {
                                if (nesting <= 0)
                                    done++;     /* block end */
                                else nesting--;
                                break;
                            }
                        }
                        idx += KWL_FIELDS;
                    }
                }
            } else if (what == token || !what) {
                /*
                 *  Matching
                 */
                done++;
            }

            if (done) {
                indent = _f_indent_level(pos);  /* if locate, determine nesting */
            }
        }

        if (!done) {
            if (re_search( SF_IGNORE_CASE,      /* end of scope */
                    "<[ \t]@{{PROGRAM}|{FUNCTION}|{SUBROUTINE}|{BLOCK DATA}}", line )) {
                done++;
            }
        }
    }
    restore_position();
    return indent;
}



/*
 *  _f_indent_next ---
 *      Computes the indent level of the next line based on its parameters, which are
 *      the code part of the last line of the statement and the code part of the first 
 *      line of the statement. The lines may be the same (and usually are).
 *
 *      Returns 1 if the indent level of the next line should be increased, or 0 if 
 *      it should remain the same.
 */
int
_f_indent_next(string part1, string part2)
{
    switch (_f_token(part1)) {
    case K_IF:
    case K_THEN:
    case K_ELSE:
    case K_ELSEIF:
        return (1);
    default:
        return (K_DO == _f_token(part2));
    }
}


/*
 *  _f_indent_level ---
 *      Maps between column and indent levels (in tab stops).
 *
 *      The mapping accounts for the six reserved columns at the beginning of each
 *      line: indent level 0 extends from column FOR_TEXT_COL to just before the next
 *      tab stop, etc.
 */
int
_f_indent_level(/*[column] */)
{
    int curr_col, column;
    int level;

    curr_col = FOR_TEXT_COL;
    if (!get_parm (0, column)) {
        inq_position (NULL, column);
    }
    save_position();
    move_abs(0, curr_col);
    while ((curr_col += distance_to_indent()) <= column) {
        move_abs(0, curr_col);
        ++level;
    }
    restore_position();
    returns (level);
}


/*
 *  _f_indent_pos ---
 *      Maps between indent levels (in tab stops) and column positions.
 *
 *      The mapping accounts for the six reserved columns at the beginning of each
 *      line: indent level 0 extends from column FOR_TEXT_COL to just before the next
 *      tab stop, etc.
 */
int
_f_indent_pos(int level)
{
    int curr_col;

    curr_col = FOR_TEXT_COL;

    save_position();
    move_abs(0, curr_col);
    while (--level >= 0) {
        curr_col += distance_to_indent();
        move_abs(0, curr_col);
    }
    restore_position();
    return curr_col;
}


/*
 *  _f_reindent_label ---
 *      Given a line with a label in the text area, convert it to a "standard FORTRAN"
 *      line, with the label in its own area.
 *
 *      Simplifies the entry of labeled lines.
 */
string
_f_reindent_label(int level, string text)
{
    string temp, label;
    int t, l, c;

    text = trim(text);
    if ((t = re_search(SF_MAXIMAL, FOR_SKIP_LABEL, text)) <= 0) {
        return (text);
    }

    label = rtrim(substr(text, 1, t-1));

    if ((l = strlen(label)) < FOR_LABEL_WIDTH) {
        l = FOR_LABEL_WIDTH;
    }

    if ((c = _f_indent_pos(level) - l) <= 0) {
        c = 1;
    }

    sprintf(temp, " %*s%*s%s",
        FOR_LABEL_WIDTH, label, c, "", substr(text, t));

    beginning_of_line ();
    delete_to_eol ();
    insert (temp);
    return (temp);
}


/*
 *  _f_reindent ---
 *      Reindents the code part of a FORTRAN line to a specified level. If the line is
 *      already indented correctly, does nothing.
 */
void
_f_reindent(int level)
{
    move_abs(0, FOR_TEXT_COL);
    re_search(NULL, "[~ \t]");                  // free-style

    if (_f_indent_level() != level) {
        string temp;

        temp = trim(read());
        move_abs(0, FOR_TEXT_COL);
        delete_to_eol();
        move_abs(0, _f_indent_pos(level));
        insert(temp);
    }
}


/*
 *  _f_iscomment ---
 *      Is a line a comment?
 */
int
_f_iscomment(void)
{
    int retval;

    save_position();
    move_abs(0, FOR_CMT_COL);
    retval = index(FOR_CMT_LIST, read(1));
    restore_position();
    return(retval);
}


/*
 *  _f_iscont ---
 *      Is a line a continuation?
 */
int
_f_iscont(void)
{
    string line;
    int retval;

    save_position();
    move_abs(0, FOR_CONT_COL);
    retval = index(FOR_CONT_LIST, read(1));     // old (fixed) style

    if (retval == 0 && _f_freestyle) {
        up(); beginning_of_line();
        line = read();                          // new (free) style
        if ((retval = rindex(line, "\n")) > 0) {
            retval = index(FOR90_CONT_CHAR, substr(line, retval-1, 1));
        }
    }
    restore_position();
    return (retval);
}


/*
 *  _f_comment ---
 *      Comment the current line.
 */
void
_f_comment(void)
{
    /* 
     *  Determine the comment character is use, otherwise default.
     */
    if (_f_comment_char == "") {
//      string lst;
        int lines, c;

        _f_comment_char =                       // default
            (_f_freestyle ? FOR90_CMT_CHAR : FOR_CMT_CHAR);

        save_position();
        top_of_buffer();
        lines = inq_lines();
        while (lines-- > 0) {
            move_abs(0, FOR_CMT_COL);
            if ((c = index(FOR_CMT_LIST, read(1))) > 0) {
                _f_comment_char = substr(FOR_CMT_LIST, c, 1);
                break;
            }
            down();
        }
        restore_position();
    }

    /* insert */
    save_position();
    move_abs(0, FOR_CMT_COL);
    insert(_f_comment_char);
    restore_position();
}


/*
 *  _f_wrap --
 *      Perform automatic wrapping at the current right margiNULL, enforcing f77 fixed style.
 */
void
_f_wrap(void)
{
    int rmargin;
    int col, split_col;

    rmargin = FOR_RIGHT_MARGIN;                 // f77
    if (_f_freestyle) {
        rmargin = FOR90_RIGHT_MARGIN;           // f90+, use wp setttings??
    }
    inq_position(NULL, col);

    /*
     *  Checks to see if we have gone past the right margin.
     */
    if (col <= rmargin) {
        return;
    }

    /*
     *  f77/
     *      If the line's a comment, it may extend to column 80, so we just warn 
     *      and return unless the line is even longer.
     */
    if (! _f_freestyle && _f_iscomment()) {
        if (col <= FOR_CMT_MARGIN) {
            beep();                             // warning
            return;
        }
        split_col = FOR_CMT_MARGIN;
    } else {
        split_col = rmargin;
    }

    /*
     *  Split, if the line is code, make it into a properly indented
     *  continuation line. If the line is a comment, it continues it.
     */
    col -= _f_split(split_col);
    col -= _f_indent(1);
    next_char(col);
}


/* 
 *  _f_split ---
 *      Find the best split point for a FORTRAN statement.
 */
int
_f_split(int max_split_col)
{
    int split_col;

    /*
     *  If we can't find a better splitting point, we will split the line at
     *  the last possible text character on the line.
     */
    move_abs (0, FOR_TEXT_COL);
    search_fwd ("[~ ]", 1);
    next_char ();
    drop_anchor ();
    move_abs (0, max_split_col);

    /*
     *  We prefer to split lines at the beginning of a run of spaces or
     *  immediately after a comma.
     *
     *  Failing that, we will split at the first of a run of opening
     *  parentheses, after a closing parenthesis, plus sigNULL, equals sign or
     *  hypheNULL, or after a logical operator like .GT.
     *
     *  Failing that, we will split after a quote, slash, or asterisk.
     */
#define SFFLAGS     (SF_BLOCK|SF_IGNORE_CASE|SF_BACKWARDS)

    if (re_search( SFFLAGS, "{ +}|{,\\c?}" ) ||
            re_search( SFFLAGS, "{(+}|{[)\\-+=]|{[A-Z].}\\c?}" ) ||
            re_search( SFFLAGS, "{['\"/*]\\c?}" )) {
        inq_position(NULL, split_col);
        move_abs(0, max_split_col);
    }
    raise_anchor();

    /*
     *  If we found no split columNULL, or if the split was past the margin,
     *  split at the margin. Beep, as a warning.
     */
    if (!split_col || split_col > max_split_col) {
        split_col = max_split_col;
        beep();                                 // warning
    }
    move_abs(0, split_col);

    return (split_col);
}


/*  Template editing macros:

    These macro performs simple template editing for programs.

    Each time the space bar is pressed, the characters before the cursor are checked to
    see if they are "if", "else if", "while", "for", "do", switch" or "case" (or
    abbreviations for them).

    These keywords must only be preceded with spaces or tabs, and be typed at the end
    of a line. If a match is found, the remainder of the statement is filled in
    automatically.

    In additioNULL, a brace pairer is included -- this automatically inserts a matching
    brace at the proper indentation level when an opening brace is typed in. To insert
    a brace without a matching brace (it attempts to be smart about matching braces,
    but you never can make this type of thing quite smart enough), type either Ctrl-{
    or quote the brace with Alt-q.

**/

/*
 *  _f_abbrev ---
 *      This function checks to see if the characters before the space just typed (and
 *      actually inserted by this function) are destined to be followed by the
 *      remainder of a C construct
 */
void
_f_abbrev(void)
{
    int done = FALSE;
    string rd = read(1);

//  pause_on_error();

    if (rd == "\n") {
        string word;
        int length;

        save_position();
        beginning_of_line();
        word = trim(read());
        if ((length = rindex(word, " ")) > 0)
            word = substr(word, length+1);
        restore_position();

        if ((length = strlen(word)) >= _f_min_abbrev) {
            int len = length_of_list(_f_keywords);
            int at = 0;

            while (!done && at < len) {
                if (re_search(SF_NOT_REGEXP|SF_IGNORE_CASE, word, _f_keywords[at]) == 1) {
                    done = TRUE;
                    break;
                }
                at += KWL_FIELDS;
            }

            if (done) {
                string completion;
                int flags;

                /*
                 *  Delete the abbrev and replace it with the expanded version.
                 */
                message("%s->%s", word, _f_keywords[at]);
                prev_char(length);
                delete_char(length);
                completion = _f_keywords[at + KWL_COMPL];
                if (completion == "") {
                    completion = _f_keywords[at];
                    completion += " ";
                }
                _f_insert(completion);

                /*
                 *  Specialised handling
                 */
                flags = _f_keywords[at + KWL_FLAGS];
                if (flags & DO_DO)
                {   /*
                     *  Make up a unique label based on the current line number.
                     *
                     *           DO xxx
                     *      xxx  CONTINUE
                     *           END DO
                     */
                    int loc;

                    inq_position(loc);
                    sprintf(completion, "%d\n %-*d CONTINUE", loc, FOR_LABEL_WIDTH, loc);
                    _f_insert(completion);
                    _f_expand_end("DO");
                    end_of_line();

                } else if (flags & DO_END) {
                    string end;

                    end = _f_keywords[at] + " ";
                    end = (substr(end, 1, index(end, " ")-1));
                    _f_expand_end(end);

                } else if (flags & DO_INDENT) {
                    _f_indent(1);

                } else if (flags & DO_INDENT0) {
                    _f_indent(0);
                    up(); end_of_line();

                } else {
                    end_of_line();
                }
                move_rel(0, _f_keywords[at + KWL_POSC]);

                done = TRUE;
            }
        }
    }

    if (!done) {
        if (inq_symbol("_bvar_abbrev")) {       // standard abbrev support
            _abbrev_check();
        } else {
            self_insert();
        }
    }

//  pause_on_error();
}


/*
 *  _f_expand_end ---
 *      Expand the end of a block of type 'end'.
 */
void
_f_expand_end( string end )
{
    string line;
    int pos;

    save_position();
    _f_indent(1);

    /* determine parent column */
    up(); beginning_of_line();
    line = read();
    if ((pos = re_search(SF_MAXIMAL, FOR_SKIP_LABEL, line)) <= 0) {
        pos = FOR_TEXT_COL;                     /* default first text column */
    }
    down();
    move_abs(0, pos);

    /* labelled? */
    if ((pos = re_search(SF_MAXIMAL, FOR90_LABEL, line)) > 0) {
        end += " " + ltrim(substr(line, 1, pos-1));
    }

    /* insert */
    _f_insert("END" + end);                     /* option??? 'ENDIF' or 'END IF' */
    restore_position();
}


/*
 *  _f_insert ---
 *      Converts the command into the user-specified preferred case and inserts into
 *      the current buffer.
 */
static void
_f_insert(string cmd)
{
    insert( _f_case(cmd) );
}


/*
 *  _f_case ---
 *      Converts keywords into the user-specified preferred case. If the case is 
 *      MIXED, the first separate letter of each "word" in the keyword string is 
 *      converted to upper case, and the others are converted to lower case.
 */
static string
_f_case(string keyword)
{
    switch (_f_keyword_case) {
    case KW_UPPER:
        return (upper(keyword));

    case KW_LOWER:
        return (lower(keyword));

    case KW_MIXED: {
            int space;
            string retval;

            keyword = lower(keyword);
            while (space = index(keyword, " ")) {
                retval += upper(substr(keyword, 1, 1));
                retval += substr(keyword, 2, space - 1);
                keyword = substr(keyword, ++space);
            }
            retval += upper(substr(keyword, 1, 1));
            return (retval += substr(keyword, 2));
        }
    }
    return "";
}
