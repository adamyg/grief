/* -*- mode: cr; indent-width: 4; -*-
 * $Id: perl.cr,v 1.17 2022/07/10 13:08:02 cvsuser Exp $
 * Perl support mode.
 *
 *
 */

#include "../grief.h"
#include "../mode.h"

#define MODENAME "PERL"

#define MIN_ABBREV      2       /* Increase this to specify longer default minimum abbreviations */

static list         perl_hier_list =
    {
        "Arity          Operator",
        "--------------------------------------------------",
        "left           terms and list operators (leftward)",
        "left           ->",
        "nonassoc       ++ --",
        "right          **",
        "right          ! ~ \\ and unary + and -",
        "left           =~ !~",
        "left           * / % x",
        "left           + - .",
        "left           << >>",
        "nonassoc       named unary operators",
        "nonassoc       < > <= >= lt gt le ge",
        "nonassoc       == != <=> eq ne cmp",
        "left           &",
        "left           | ^",
        "left           &&",
        "left           ||",
        "nonassoc       ..  ...",
        "right          ?:",
        "right          = += -= *= etc.",
        "left           , =>",
        "nonassoc       list operators (rightward)",
        "right          not",
        "left           and",
        "left           or xor",
    };

static int          _perl_min_abbrev;
static int          _perl_keyboard;


/*
 *  main ---
 *      Define 'Perl' mode syntax
 */
void
main()
{
    create_syntax(MODENAME);

    /*
     *  Standard syntax engine
     */
    syntax_token(SYNT_COMMENT,      "#");
    syntax_token(SYNT_COMMENT,      "=pod", "=cut");
    syntax_token(SYNT_QUOTE,        "\\");
    syntax_token(SYNT_STRING,       "\"");
    syntax_token(SYNT_LITERAL,      "\'");
    syntax_token(SYNT_BRACKET,      "([{", ")]}");
    syntax_token(SYNT_DELIMITER,    ",;.?:");
    syntax_token(SYNT_OPERATOR,     "-+/&*=<>|!~^%");
    syntax_token(SYNT_WORD,         "$0-9A-Z_a-z", "0-9A-Z_a-z");
    syntax_token(SYNT_NUMERIC,      "-+.0-9_xa-fA-F");

    /*
     *  Advanced syntax engine/
     *      used to built a DFA based lexer/parser, which is generally faster.
     */
    syntax_rule("^#.*$", "spell,todo:comment");
    syntax_rule("[ \t;]#.*$", "spell,todo:comment");

    syntax_rule("([$%&@*]|\\$#)[A-Za-z_0-9]+", "normal");
    syntax_rule("\\$([_./,\"#*?\\[\\];!@:$<>()%=-~^|&`'+]|\\^[A-Z])", "normal");

    syntax_rule("[A-Za-z_][A-Za-z0-9_]*", "keyword:normal");

    syntax_rule("[0-9]+(\\.[0-9]+)?([Ee][\\-\\+]?[0-9]+)?", "number");
    syntax_rule("0x[0-9A-Fa-f]+", "number");

    syntax_rule("\"(\\\\\"|[^\"])*\"", "string");
    syntax_rule("\'(\\\\\'|[^\'])*\'", "string");

    syntax_rule("[()\\[\\]{}<>,.:;?]", "delimiter");
    syntax_rule("[-+!%&*/=<>|~^]", "operator");

    syntax_build(__COMPILETIME__);              /* build and auto-cache */

    /*
     *  Keywords
     */
    define_keywords(SYNK_PRIMARY,
        "if", -2);
    define_keywords(SYNK_PRIMARY,
        "for,cmp", -3);
    define_keywords(SYNK_PRIMARY,
        "carp,else", -4);
    define_keywords(SYNK_PRIMARY,
        "croak,elsif,until,while", -5);
    define_keywords(SYNK_PRIMARY,
        "unless", -6);
    define_keywords(SYNK_PRIMARY,
        "foreach", -7);

    define_keywords(SYNK_OPERATOR,
        "eq,ge,gt,le,lt,ne,or", -2);

    define_keywords(SYNK_TODO,
        "XXX,TODO,FIXME,DEPRECATED,MAGIC,HACK");

    /*
     *  Builtin functions in Perl 4x
     */
    define_keywords(SYNK_FUNCTION,
        "do,uc,qq,qr,tr,or", -2);

    define_keywords(SYNK_FUNCTION,
        "cos,die,eof,exp,hex,int,log,oct,oct,ord,pop,pos,sin,tie,use,"+
        "vec,not,and,xor", -3);

    define_keywords(SYNK_FUNCTION,
        "bind,chop,dump,each,eval,exec,exit,fork,getc,goto,grep,join,"+
        "keys,kill,last,link,next,open,pack,pipe,push,rand,read,recv,"+
        "redo,seek,send,sort,sqrt,stat,tell,time,wait,warn", -4);

    define_keywords(SYNK_FUNCTION,
        "alarm,atan2,chdir,chmod,chown,close,crypt,fcntl,flock,index,"+
        "ioctl,local,lstat,mkdir,print,reset,rmdir,semop,shift,sleep,"+
        "split,srand,study,times,umask,undef,untie,utime,write", -5);

    define_keywords(SYNK_FUNCTION,
        "accept,caller,chroot,delete,fileno,format,gmtime,length,listen,"+
        "msgctl,msgget,msgrcv,msgsnd,printf,rename,return,rindex,scalar,"+
        "select,semctl,semget,shmctl,shmget,socket,splice,substr,system,"+
        "unlink,unpack,values", -6);

    define_keywords(SYNK_FUNCTION,
        "binmode,connect,dbmopen,defined,getpgrp,getppid,opendir,package,"+
        "readdir,require,reverse,seekdir,setpgrp,shmread,sprintf,symlink,"+
        "syscall,sysread,sysseek,telldir,ucfirst,unshift,waitpid", -7);

    define_keywords(SYNK_FUNCTION,
        "closedir,continue,dbmclose,endgrent,endpwent,getgrent,getgrgid,"+
        "getgrnam,getlogin,getpwent,getpwnam,getpwuid,readlink,setgrent,"+
        "setpwent,shmwrite,shutdown,syswrite,truncate", -8);

    define_keywords(SYNK_FUNCTION,
        "endnetent,getnetent,localtime,quotemeta,rewinddir,setnetent,"+
        "wantarray", -9);

    define_keywords(SYNK_FUNCTION,
        "endhostent,endservent,gethostent,getservent,getsockopt,sethostent,"+
        "setservent,setsockopt,socketpair", -10);

    define_keywords(SYNK_FUNCTION,
        "endprotoent,getpeername,getpriority,getprotoent,getsockname,"+
        "setpriority,setprotoent", -11);

    define_keywords(SYNK_FUNCTION,
        "getnetbyaddr,getnetbyname", -12);

    define_keywords(SYNK_FUNCTION,
        "gethostbyaddr,gethostbyname,getservbyname,getservbyport", -13);

    define_keywords(SYNK_FUNCTION,
        "getprotobyname", -14);

    define_keywords(SYNK_FUNCTION,
        "getprotobynumber", -16);

    /*
     *  Builtin functions new to Perl 5x
     */
    define_keywords(SYNK_FUNCTION,
        "lc,my,no,qw,qx", -2);

    define_keywords(SYNK_FUNCTION,
        "abs,chr,map,our,ref,sub", -3);

    define_keywords(SYNK_FUNCTION,
        "glob,tied", -4);

    define_keywords(SYNK_FUNCTION,
        "bless,chomp", -5);

    define_keywords(SYNK_FUNCTION,
        "exists,import", -6);

    define_keywords(SYNK_FUNCTION,
        "lcfirst,sysopen", -7);

    define_keywords(SYNK_FUNCTION,
        "formline,readline,readpipe", -8);

    define_keywords(SYNK_FUNCTION,
        "prototype", -9);

    /*
     *  Keyboard
     */
    load_indent();
    keyboard_push();
    assign_to_key("<Enter>",        "_perl_indent");
    assign_to_key("<Tab>",          "_slide_in");
    assign_to_key("<Shift-Tab>",    "_slide_out");
//  assign_to_key("<Space>",        "_perl_abbrev");
    assign_to_key("<{>",            "_c_open_brace");
    assign_to_key("<}>",            "_c_close_brace");
    _perl_keyboard = inq_keyboard();
    keyboard_pop(1);
}


/*
 *  Modeline/package support
 */
string
_perl_mode()
{
    _c_mode();
    return "perl";                              /* return primary package extension */
}


list
_perl_hier_list()
{
    return perl_hier_list;
}


string
_perl_highlight_first()
{
    attach_syntax(MODENAME);
    return "";
}


string
_perl_smart_first()
{
    _perl_min_abbrev = 0;
    get_parm(0, _perl_min_abbrev);
    _perl_min_abbrev = MIN_ABBREV;
    use_local_keyboard( _perl_keyboard );
    return "";
}


string
_perl_template_first()
{
    return _perl_smart_first();
}


string
_perl_regular_first()
{
    return _perl_smart_first();
}


#define IGNORE_LIST_SIZE        1               /* Set to number of elements in list */

static list perl_ignore_list =
    {
        "[ \\t]@#*$",                           /* Perl comment lines */
    };


/*
 *  _perl_prevline ---
 *      Finds the previous syntactically significant line
 */
string
_perl_prevline(void)
{
    int curr_line, at_line, patt_no, done;
    string line;

    inq_position(curr_line, NULL);

    /* Move to the line we wish to start at */
    at_line = curr_line;
    for (done = FALSE; !done;) {
        /* Process each line, and find one with text that cannot be eliminated */
        beginning_of_line();
        line = read();

        for (patt_no = 0; patt_no < IGNORE_LIST_SIZE; ++patt_no) {
            int patt_len, gotcha;

            /* Keep removing discardable text */
            while ((gotcha = re_search( NULL, perl_ignore_list[patt_no], line, NULL, patt_len )) > 0)
                line = substr(line, 1, gotcha - 1) + substr(line, gotcha + patt_len);
        }

        /* We've now eliminated all comment/discardable text. Trim
         * the string, and if it's empty move back one more line. If
         * we've reached the beginning of the buffer, simply return an
         * empty string. If there's anything left, return that
         */
        line = trim(line);
        if (line != "") {
            ++done;
        } else {
            if (!--at_line)
                ++done;
            else
                up();
        }
    }
    return line;
}


static string PERL_TERM_CHR =  ";{}:,)=";

#define SEMI_COLON      1                       /* Code for semicolon */
#define OPEN_BRACE      2                       /* Code for open brace */
#define CLOSE_BRACE     3                       /* Code for close brace */
#define FULL_COLON      4                       /* Code for colon (case) */
#define COMMA           5                       /* Code for comma */
#define CLOSE_PAREN     6                       /* code for closing paren */
#define EQUALS          7                       /* code for equals (initialisation) */

int
_perl_indent(~ int)
{
    int     curr_indent_col,                    /* Current unmodified indent col */
            following_position,                 /* Column of end of line following cursor */
            curr_col,                           /* Column cursor is on when called */
            curr_line,                          /* Column line is on when called */
            what_is_char_1,                     /* End of first line's character identifier */
            what_is_char_2,                     /* End of second line's indentifier */
            level,                              /* Current indenting level */
            tmp_level;

    string  following_string,                   /* All characters following the cursor */
            line_end;

    /* Gather information on the two previous non-blank lines
     */
    if (! inq_mode())
        end_of_line();
    inq_position(curr_line, curr_col);
    end_of_line();
    inq_position(NULL, following_position);

    /* If there are characters following the cursor, save
     * them in following_string.
     */
    if (following_position > curr_col) {
        drop_anchor(MK_NONINC);
        move_abs(0, curr_col);
        following_string = ltrim(read());
        delete_block();
    }

    line_end = _perl_prevline();                /* Retrieve previous line */
    inq_position(tmp_level, NULL);

    if (line_end != "") {
        what_is_char_2 = index(PERL_TERM_CHR, substr(line_end, strlen(line_end)));
        beginning_of_line();
        re_search(NULL, "[~ \t]");
        inq_position(NULL, curr_indent_col);
        up();
        line_end = _perl_prevline();
        inq_position(level, NULL);
        if (line_end != "")
            what_is_char_1 = index(PERL_TERM_CHR, substr(line_end, strlen(line_end)));
        else
            what_is_char_1 = SEMI_COLON;

    } else {
        what_is_char_1 = what_is_char_2 = SEMI_COLON;
        curr_indent_col = 1;
    }
    move_abs(curr_line, curr_indent_col);

    /* We've determined the last two non-blank lines' last
     * characters as well as the column position of the first
     * non-blank character. Now we position the cursor on the new
     * line's proper level.
     */
    if (curr_indent_col == 1 && what_is_char_2 == OPEN_BRACE) {
        curr_indent_col += distance_to_indent();

    } else {
        /* The following switch statement is the body of the _perl_indent
         * macro indenting rules. what_is_char_2 is the terminator on
         * the immediately preceeding line, what_is_char_1 is the terminator
         * prior that. This allows us to make some guesses about whether we
         * should indent or outdent from the current level
         */
        switch (what_is_char_2) {
        case SEMI_COLON:
            if (!what_is_char_1)
                curr_indent_col -= distance_to_indent();
            break;
        case CLOSE_BRACE:
            curr_indent_col -= distance_to_indent();
            break;
        case OPEN_BRACE:
            curr_indent_col += distance_to_indent();
            break;
        case COMMA:
            if (what_is_char_1 != COMMA)
                curr_indent_col += distance_to_indent();
            break;
        case EQUALS:
        case FULL_COLON:
        case CLOSE_PAREN:
            curr_indent_col += distance_to_indent();
            break;
        default:
            /* Nothing */
            break;
        }
    }

    move_abs(0, curr_col);

    if (get_parm(0, curr_col) == 0) {
        if (inq_assignment(inq_command(), 1) == "<Enter>")
            self_insert(key_to_int("<Enter>"));
        else
            self_insert(key_to_int("<Ctrl-M>"));
    }

    beginning_of_line();
    curr_col = distance_to_indent() + 1;

    level = 0;
    while (curr_col <= curr_indent_col) {
        move_abs(0, curr_col);
        ++level;
        curr_col += distance_to_indent();
    }
    if (following_string != "") {
        following_string = substr(following_string, 1, strlen(following_string) - 1);
        save_position();
        insert(following_string);
        restore_position();
    }
    return level;
}


/*
 *  _perl_abbrev ---
 *      This function checks to see if the characters before the space
 *      just typed (and actually inserted by this function) are
 *      destined to be followed by the remainder of a construct.
 */
void
_perl_abbrev(void)
{
    self_insert();
}

