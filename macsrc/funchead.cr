/* -*- indent-width: 4; -*- */
/* $Id: funchead.cr,v 1.12 2014/10/27 23:28:21 ayoung Exp $
 * Create a function header based upon a template
 *
 * Examples

    Synopsis:
        %[1%FTYPE% %FNAME%%FOPEN% %FARGS% %FCLOSE%      // single arg]%

        %[2%FTYPE% %FNAME%%FOPEN% %FARGS(13)% %FCLOSE%  // multi arg]%

        %FTYPE% %FNAME%
            %FOPEN%
                %FARGS(,72)%
            %FCLOSE%

    Purpose:
        %FDESC%

    Parameters:
FINPUTS(9)
%FINPUTS(9)%

FINPUTS(0,+10)
        %FINPUTS(0,+10)%

FINPUTS(0,+10,-2)
        %FINPUTS(0,+10,-2)%

FINPUTS(0,32,,80,1,,1)
        %FINPUTS(0,32,,80,1,,1)%

FINPUTS(0,32,-4,60,1,1)
        %FINPUTS(0,32,-4,60,1,1)%

FINPUTS(0,-1)
        %FINPUTS(0,-1)%

FINPUTS(0,20,,70,1,1,1)
        %FINPUTS(0,20,,70,1,1,1)%

    Returns:
        %FTYPE% -

 *
 *
 *
 */

#include "grief.h"
#include "mode.h"

#define STDP            4                       /* standard parameters (type,name,open & close) */
#define ARGSP           3                       /* argument parameters (def, arg & desc) */
#define ARG_DEF         0
#define ARG_NAME        1
#define ARG_DESC        2

void                    funchead(string desc);

static void             _fh_arguments(void);
static void             _fh_inputs(void);
static declare          _fh_parse(void);

static string           _fh_wordleft(void);
static list             _c_fh_parse2(void);
static declare          _c_fh_parse3(list argv);
static string           _c_fh_argname(string arg);


void
fh(string desc)
{
    funchead(desc);
}


string
_completion_Funchead(string arg)
{
    return compl_readfile(arg);
}


void
funchead(string desc)
{
    string  fmode, ftemplate, fheader;
    int     curbuf, tempbuf;
    declare argv;

    /*
     *  Retrieve template
     */
    fmode = _mode_pkg_get();
    ftemplate = getenv("GRTEMPLATE");           // GRIEF macro templete search path
    if (ftemplate != "") {
        if (!exist( ftemplate )) {
            ftemplate = "";
        }
    }
    if (ftemplate == "") {
        ftemplate = inq_home() + "/template";
        if (!exist( ftemplate )) {
            ftemplate = "";
        }
    }
    if (ftemplate == "") {
        ftemplate = getenv("GRPATH") + "/../template";
        if (!exist( ftemplate )) {              // FIXME -- search path
            ftemplate = "";
        }
    }
    if (ftemplate != "") {
        ftemplate += "/";
    }

    fheader = ftemplate + "func*." + fmode;
    if (!get_parm(NULL, fheader, "Funchead: ", NULL, fheader)) {
        beep();
        return;
    }
    while (!exist(fheader)) {
        if (!get_parm(NULL, fheader, "Funchead: ", NULL, fheader)) {
            beep();
            return;
        }
    }

    /*  Parse, its assumed the parser returns a list of the following syntax:
     *
     *      argv[0]         Function type.
     *      argv[1]         Function name.
     *      argv[2]         Opening bracket.
     *      argv[3]         Closing bracket.
     *  foreach argument (4 ... a+3)
     *      argv[a]         Definition.
     *      argv[a+1]       Argument name, normally a striped definition.
     *      argv[a+2]       Description, normally taken from any trailing
     *                      comment blocks.
     */
    save_position();
    argv = _fh_parse();
    restore_position();
    if (!is_list(argv)) {
        return;
    }

    /*
     *  Build header
     */
    if ((tempbuf = create_buffer( "-funchead-", fheader, 1 )) >= 0) {
        int defrmargin, row, col, argc;
        string k;

        curbuf = inq_buffer();
        argc = length_of_list(argv);            /* argument/comment count */

        set_buffer( tempbuf );
        top_of_buffer();

        /* Determine rmargin before the template is modified */
        if ((defrmargin = inq_line_length()) < 78) {
            defrmargin = 78;
        }

        /*
         *  Handle single or multi-argument groupings
         *
         *      %[1 text processed if 0 or 1 arguments ]%
         *      %[2 text processed of 2 or more arguments ]%
         *
         *  The selected text may cross line boundaries
         */
        k = (argc <= (STDP+ARGSP) ? "2":"1");  /* remove text */
        while (re_search( SF_NOT_REGEXP, "%["+k ) > 0) {
            drop_anchor( MK_NORMAL );
            if ( re_search( SF_NOT_REGEXP, "]%" ) > 0) {
                next_char( 2 );
                delete_block();
            } else {
                next_char();
            }
            raise_anchor();
        }
        top_of_buffer();

        k = (argc <= (STDP+ARGSP) ? "1":"2");  /* remove alt markers */
        while (re_search( SF_NOT_REGEXP, "%["+k ) > 0) {
            delete_char( 3 );
            if ( re_search( SF_NOT_REGEXP, "]%" ) > 0) {
                delete_char( 2 );
            }
        }
        top_of_buffer();

        /* FTYPE/FNAME - Function type and name */
        translate( "%FTYPE%", argv[0], 1, 0 );
        translate( "%FNAME%", argv[1], 1, 0 );

        /* FOPEN/FCLOSE - Opening and closing brackets,
         *   normally used to enclose the argument list (below).
         */
        translate( "%FOPEN%", argv[2], 1, 0 );
        translate( "%FCLOSE%", argv[3], 1, 0 );

        /* FARGS/FINPUTS - Argument list */
        _fh_arguments(/*argv*/);
        _fh_inputs(/*argv*/);

        /* FDESC - Description */
        translate("%FDESC%", desc, 1, 0); top_of_buffer();

        end_of_buffer();
        inq_position( row, col );
        set_buffer( curbuf );
        transfer( tempbuf, 1, 1, row, col );
        delete_buffer( tempbuf );
    }
}


/*  FARGS [(lmargin[,rmargin])] - Argument list

    PARAMETERS:
        lmargin,    Left margin of arguments, defaulting to starting
                    location of the token.

                    Note that the first argument is also placed at the TOKEN location, with
                    lmargin being used to place arguments when a newline is required.

        rmargin,    Right margin, forcing a newline if exceeded. Default is 78 or width of
                    template which ever is the greater.

                    Specifying a non-zero value <= lmargin shall force a new-line for each
                    argument.

-*/
int
inq_column()
{                   /*TODO - add to standard library*/
    int line, column;
    inq_position(line, column);
    return column;
}


static void
_fh_arguments(void)
{
    extern  list argv;
    extern  int defrmargin, argc;
    string  opts;
    int     lmargin, rmargin;
    int     olen, i;

    while ((olen = re_search( NULL, "\\%FARGS*\\%" )-1) > 0) {
        /* Defaults */
        lmargin = inq_column();
        rmargin = defrmargin;

        /* Read and remove token */
        opts = read( olen );
        delete_char( olen );

        /* Parse arguments (if any) */
        if ((i = index(opts, "(")) >= 1) {
            opts = substr(opts, i+1);           /* lmargin */
            if ((i = atoi(opts)) > 0) {
                lmargin = i;
            }
            if ((i = index(opts, ",")) >= 1) {  /* rmargin */
                opts = substr(opts, i+1);
                if ((i = atoi(opts)) > 0){
                    rmargin = i;
                }
            }
        }

        /* Dump argument list */
        if (argc > STDP)  {
            i = STDP;                           /* arg1 */
            insert( argv[i+ARG_DEF] );

            for (i += ARGSP; i < argc; i += ARGSP) {
                                                /* arg2+ */
                if (inq_column() + strlen( argv[i+ARG_DEF] ) + 2 > rmargin) {
                    insert( ",\n" );
                    insert( " ", lmargin-1 );
                } else {
                    insert( ", " );
                }
                insert( argv[i+ARG_DEF] );
            }
        }
        top_of_buffer();
    }
}


/*-

    FINPUTS [([lmargin],[tab1],[tab2],[rmargin],[spacing],[nll,[fill])] ---
        Input list one per-line with optional description.

    PARAMETERS:
        lmargin,    Left margin of arguments, defaulting to starting location of the token.

        tab1,       Left margin of descriptions, which are normally derived from trailing
                    comments after each definition. If omitted or zero, descriptions are not
                    reported (default).

                    A value of the form +x results in there being a floating left margin, with
                    description text directly following the argument with spacing of 'x-1'.

                    A value of the form -x, sets the column using longest length argument plus a
                    spacing of 'x-1'.

                    Any other non-zero value specifies the absolute column where text shall start.

                    Any argument text which causes an overruns of the specified column places the
                    description on the next line.

        tab2,       Left margin used on secondary lines within the description, if omitted or
                    zero same value as tab1 (default).

                    A value of the form {+/-}x results in a secondary left margin being either a
                    positive or negative offset of the 'tab1' value. Any other non-zero specifies
                    the absolute column.

        rmargin,    Right margin of descriptions, forcing a newline if exceeded. Default is 78 or
                    width of template which ever is the greater.

        spacing,    Line spacing between each arguments, defaulting to 0.

        newline,    If TRUE, description text is always positioned on a new line at the tab1
                    position.

        fill,       If non-zero, description text is space padding to the right margin.

-*/

static void
_fh_inputs(void)
{
    extern  list argv;
    extern  int defrmargin, argc;
    string  opts, tab2arg;
    int     lmargin, tab1, tab2, col, rmargin, spacing, newline, fill;
    int     olen, wlen, w, i;
    list    words;

    while ((olen = re_search( NULL, "\\%FINPUTS*\\%" )-1) > 0) {
        /* Defaults */
        lmargin = inq_column();
        tab1    = 0;
        tab2arg = "";
        rmargin = defrmargin;
        spacing = 0;
        newline = 0;
        fill = 0;

        /* Read and remove token */
        opts = read( olen );
        delete_char( olen );

        /* Parse arguments (if any) */
        if ((i = index(opts, "(")) >= 1) {

            opts = substr(opts, i+1);           /* lmargin */
            if ((i = atoi(opts)) > 0) {
                lmargin = i;
            }

            if ((i = index(opts, ",")) >= 1) {  /* tab1 */
                opts = substr(opts, i+1);
                if (substr(opts, 1, 1) == "+") {
                                                /* .. +spacing (dynamic) */
                    tab1 = -atoi(opts+1);

                } else {                        /* .. absolute */
                    tab1 = atoi(opts);
                    if (tab1 <= -1) {           /* .. arg-width, +spacing */
                        int tlen, alen;

                        for (tlen = 0, i = STDP; i < argc; i += ARGSP) {
                            if ((alen = strlen( argv[i+ARG_NAME] )) > tlen)
                                tlen = alen;
                        }
                        tab1 = lmargin + tlen - tab1;
                    }
                }
            }
            if ((i = index(opts, ",")) >= 1) {  /* tab2 */
                opts = substr(opts, i+1);
                tab2arg = opts;
            }
            if ((i = index(opts, ",")) >= 1) {  /* rmargin */
                opts = substr(opts, i+1);
                if ((i = atoi(opts)) > 0)
                    rmargin = i;
            }
            if ((i = index(opts, ",")) >= 1) {  /* spacing */
                opts = substr(opts, i+1);
                if ((i = atoi(opts)) > 0)
                    spacing = i;
            }
            if ((i = index(opts, ",")) >= 1) {  /* newline */
                opts = substr(opts, i+1);
                if ((i = atoi(opts)) > 0)
                    newline = i;
            }
            if ((i = index(opts, ",")) >= 1) {  /* fill */
                opts = substr(opts, i+1);
                if ((i = atoi(opts)) > 0)
                    fill = i;
            }
        }

        /* Dump argument list */
        for (i = STDP; i < argc; i += ARGSP) {
            /* Input name */
            if (i > STDP) {
                insert("\n");
                insert(" ", lmargin - 1);
            } else {
                move_abs(NULL, lmargin);
            }
            insert(argv[i + ARG_NAME]);
            insert(",");

            /* Descriptions */
            if (tab1) {                         /* desc */
                /* Move to left margin (tab1) */
                col = inq_column();
                if (newline) {
                    insert("\n");
                    move_abs( NULL, col );
                }
                if (tab1 < 0) {                 /* offset */
                    insert(" ", -tab1 - 1);
                } else {                        /* absolute */
                    if (newline || col <= tab1) {
                        move_abs(NULL, tab1);   /* .. same line */
                    } else {
                        insert("\n");           /* .. overrun, new line */
                        insert(" ", tab1 - 1);
                    }
                }
                col = inq_column();

                /* Determine 2nd left margin (tab2) */
                if (substr(tab2arg, 1, 1) == "+" ||
                        substr(tab2arg, 1, 1) == "-")
                {                               /* offset */
                    if ((tab2 = atoi(tab2arg)) >= 1)
                        tab2--;
                    if (tab1 < 0)
                        tab2 += col;
                    else tab2 += tab1;
                    if (tab2 <= 0)
                        tab2 = 1;
                } else {                        /* absolute */
                    tab2 = atoi(tab2arg);
                    if (tab2 == 0) {            /* default, tab1 */
                        if (tab1 < 0)
                            tab2 = col;
                        else tab2 = tab1;
                    }
                }

                /* Dump list --- formatting as required */
                words = split( compress(argv[i+ARG_DESC], 1), " " );
                if ((wlen = length_of_list(words)) > 0) {
                    int x1, x2;                 /* extra space requirements */

                    for (x1 = -1, w = 0; w < wlen; w++) {
                        if (w > 0) {            /* word 2+ seperator */
                            if (inq_column()+strlen(words[w]) > rmargin) {
                                insert( "\n" );
                                insert( " ", tab2 - 1 );
                                x1 = -1;        /* .. recalc padding */
                            } else {
                                insert( " ", 1 + x1 );
                                if (x2 > 0)
                                    insert( " " ), x2--;
                            }
                        }

                        if (x1 == -1) {         /* new line */
                            x1 = x2 = 0;        /* default, no padding */

                            if (fill) {         /* determine padding */
                                int w2, wl, ns, mk;

                                /* Determine the raw length of the next line */
                                col = inq_column()-1;
                                for (ns = -1, w2 = w; w2 < wlen; w2++, ns++) {
                                    wl = strlen(words[w2]);
                                    if (col + wl > rmargin)
                                        break;
                                    col += wl+1;/* accumlate length + delim */
                                }

                                /* Calculate if extra padding required, only if;
                                 * Not the last line (w2),
                                 * there are spaces which can be padded (ns)
                                 * and the line length is under (mk).
                                 *
                                 *  x1  Padding on each word
                                 *  x2  plus overflow, first word(s) only.
                                 */
                                if (w2 < wlen && ns > 0 &&
                                        (mk = rmargin - (col-1)) > 0)
                                {
                                    x1 = mk / ns;
                                    x2 = mk % ns;
                                }
                            }
                        }
                        insert( words[w] );     /* current word */
                    }
                }
            }

            /* Line spacing */
            for (w = 0; w < spacing; w++) {
                insert("\n");                   /* newline */
            }
        }
        top_of_buffer();
    }
}


/*
 *  _fh_parse ---
 *      Parse the current buffer.
 */
static declare
_fh_parse(void)
{
    list argv;

    argv = _c_fh_parse2();                      /* name and type */
    if (length_of_list(argv) == 0) {
        error( "Function definition not found" );
        return -1;
    }
    return _c_fh_parse3( argv );                /* arguments */
}


/*
 *  _fh_wordleft ---
 *      Return the word left of the current cursor position.
 */
static string
_fh_wordleft(void)
{
    int line, col, line1, col1;
    string buff;

    inq_position(line, col);
    objects( "word_left" );                     /* previous word */
    inq_position(line1, col1);
    if (line != line1)
        buff = read();
    else if (col > col1)
        buff = read(col - col1);
    return trim(compress(buff));
}


/*
 *  _c_fh_parse2 ---
 *      Routine name and type
 *
 *  Notes:
 *      Doesn't currently handle comments which cross a line boundary.
 */
static list
_c_fh_parse2(void)
{
    list argv;

    beginning_of_line();
    if (search_fwd("(") > 0) {                  /* start of current */
        int curbuf, buf;

        save_position();
        drop_anchor(MK_NONINC);

        if (search_back(")") == 0)              /* end of previous (??) */
            top_of_buffer();

        curbuf = inq_buffer();
        if ((buf = create_buffer("-funchead-", NULL, 1)) >= 0) {
            int start_line, start_col, end_line, end_col;
            string word;

            inq_marked(start_line, start_col, end_line, end_col);
            set_buffer(buf);
            transfer(curbuf, start_line, start_col, end_line, end_col);

            top_of_buffer();
            translate("//\\*>", "", 1);         /* Trailing comments */
            translate("/\\**\\*/", "", 1);      /* Remove comments */
            end_of_buffer();

            if ((word = _fh_wordleft()) != "") {
                argv += _fh_wordleft();         /* type */
                argv += word;                   /* function name */
                argv += "(";                    /* open */
                argv += ")";                    /* close */
            }
            set_buffer(curbuf);
            delete_buffer(buf);
        }
        raise_anchor();
        restore_position();
    }
    return argv;
}


/*
 *  _c_fh_parse3 ---
 *      Arguments parser.
 */
static declare
_c_fh_parse3(list argv)
{
    string  argument, desc;
    int     lines, whitespace, brackets, argc;
    string  c;

    /*
     *  Parameters, with possible intermixed comments.
     */
    next_char();
    brackets = 1;

    while (1) {
        /* Next character */
        if ((c = read(1)) == "")                /* EOF ? */
            break;
        next_char();

        if (index(c, "\n") && lines++ > 20)     /* fuse search to lines */
            break;                              /* ... completion */
        c = rtrim(c);                           /* remove \n */

        /* Terminators */
        if (c == ";")                           /* prototype */
            break;                              /* ... ignore */
        if (c == "{")                           /* function body */
            break;                              /* ... completion */

        /* Comments */
        if (c == "/") {                         /* Possible comment */
            c = read(1);

            if (substr(c,1,1) == "/") {         /* EOL comment */
                next_char();
                if (index(c,"\n") == 0) {       /* Comment text */
                    desc += rtrim(read()) + " ";
                    beginning_of_line();        /* .. move down one line */
                    down();
                }
                continue;
            }

            if (substr(c,1,1) == "*") {         /* Block comment */
                next_char();
                while (1) {                     /* .. find end of comment */
                    c = read(1);
                    if (next_char() == 0 || c == "") {
                        error( "Mismatched comment block found." );
                        return -1;              /* .. end of file */
                    }

                    if (c == "*" && substr(read(1),1,1) == "/") {
                        next_char();            /* .. end of block */
                        desc += " ";
                        break;
                    }
                                                /* .. append to description */
                    re_translate( NULL, "\n", " ", c );
                    desc += c;
                }
                continue;
            }
        }

        /* Text */
        if (index("\r\n\t ", c)) {              /* Whitespace */
            whitespace++;
        } else {                                /* Append to routine */
            if (c == "(") {                     /* Maintain bracket nesting */
                brackets++;
            } else if (c == ")") {
                if (--brackets <= 0) {
                    if (argument != "") {
                        argv += argument;
                        argv += _c_fh_argname(argument);
                        argv += desc;
                        argc++;
                    }
                    break;                      /* Done */
                }
            }

            if (brackets == 1 && c == ",") {    /* Argument delimitor */
                argv += argument;
                argv += _c_fh_argname( argument );
                argument = "";
                argc++;
            } else {                            /* Argument text */
                if (argument == "") {           /* .. new argument */
                    if (argc) {
                        argv += desc;           /* .. close prev argument */
                    }
                    argument = c;
                    desc = "";
                } else if (!whitespace || index("(),", c)) {
                    argument += c;              /* .. appending */
                } else {
                    argument += " " + c;        /* .. new word, delimit */
                }
                whitespace = 0;
            }
        }
    }

    if (brackets != 0) {
        error("Mismatched braces found");
        return -1;
    }
    return argv;
}


static string
_c_fh_argname(string arg)
{
    int i;

    if (index(arg, "(") == 0) {                 /* not function */
        if ((i = rindex(arg, " ")) > 0)
            arg = substr(arg, i+1);             /* take last word */
    }
    return arg;
}

/*end*/
