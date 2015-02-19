/* -*- mode: cr; indent-width: 4; -*-
 * $Id: tags.cr,v 1.22 2014/10/27 23:28:29 ayoung Exp $
 * Tag database interface.
 *
 *
 */

#include "grief.h"

#define DIR_DEPTH           10                  /* directory depth search */

static string           _tag_grinit;
static string           _tag_dir;
static string           _tag_source;
/*static*/ list         _tag_results;
static int              _tag_count;
static int              _tag_current;
static int              _tag_db = -1;

#define F_PARTIALMATCH      0x0001
#define F_IGNORECASE        0x0002
#define F_FILE              0x0004

#if defined(__PROTOTYPES__)
static void             _tag_execute(string function, int flags);
static int              _tag_search(string function, int flags);
static int              _tag_old_search(string function, int flags);
static string           _tags_sname(string name, int len, int maxlen);
static void             _tag_list(int flags);
static void             _tag_next();
static void             _tag_prev();
static int              _tag_editfile(int idx);
static int              _tag_scheck(string dir);
static string           _tag_file();
#endif


string
_griget_tags()
{
    return _tag_grinit;                         /* return current 'grinit' setting */
}


void
_griset_tags(string arg)
{
    _tag_grinit = trim(arg);
    if (-1 == _tag_db) {
        string tags;

        if ((tags = _tag_file()) != "") {       /* background load */
            _tag_db = tagdb_open(tags, 0, TRUE);
        }
    }
}


void
tags()
{
    string function;
    int idx, flags, param;

    idx = 0;
    param = 0;
    flags = 0;

    while (1) {
        if (! get_parm(idx++, function, "Function: ")) {
            return;
        }

        if (function == "/c") {
            flags &= F_IGNORECASE;
        } else if (function == "/i") {
            flags |= F_IGNORECASE;
        } else if (function == "/p") {
            flags |= F_PARTIALMATCH;
        } else {
            break;                              /* assume a 'filename' */
        }
    }

    idx = strlen(function);
    if (index("*?", substr(function, idx, 1))) {
        function = substr(function, 1, idx-1);
        flags |= F_PARTIALMATCH;                /* trailing wide card */
        flags |= F_FILE;
    }

    if (function == "") {
        error( "tags: must specify at least one character." );
        return;
    }

    _tag_execute(function, flags);
}


void
tag_function()
{
    string function;
    int i;

    save_position();
    re_search(SF_BACKWARDS, "<|{[^_A-Za-z0-9]\\c}");
    function = trim(read());
    i = re_search(NULL, "[^_A-Za-z0-9]", function);
    if (i > 0) {
        function = trim(substr(function, 1, i - 1));
    }
    restore_position();
    if (function == "") {
        beep();
        return;
    }
    _tag_execute(function, 0);
}


static void
_tag_execute(string function, int flags)
{
    UNUSED(flags);

    if (0 == _tag_search(function, flags)) {
        if (0 == _tag_count) {                  /* no match */
            error("tags: function %s not found.", function);

        } else if (1 == _tag_count) {           /* single match */
            _tag_editfile(0);

        } else {
            assign_to_key("<Ctrl-N>", "::_tag_next");
            assign_to_key("<Ctrl-P>", "::_tag_prev");
            assign_to_key("<Ctrl-B>", "::_tag_list -1");
            _tag_list(flags);
        }
    }
}

#define TITEMS              4
#define TNAME               0
#define TFILE               1
#define TFLAGS              2
#define TPATTERN            3

static int
_tag_search(string function, int flags)
{
    if (-1 == _tag_db) {
        string tags;

        if ((tags = _tag_file()) == "" ||
                (_tag_db = tagdb_open(tags)) == -1) {
         /* _tag_old_search(function, flags); */
            return -1;
        }
    }

    /*
     *  WARNING:
     *      the previous tagdb_search() results are destroyed on the next call.
     */
    _tag_results = tagdb_search(_tag_db, function, flags);
    _tag_count = length_of_list(_tag_results) / TITEMS;
    return 0;
}


static int
_tag_old_search(string function, int flags)
{
    string sstring;
    list results;
    int tag_buf, old_buf;
    int count = 0;

    UNUSED(flags);

    /* Open tags file */
    old_buf = inq_buffer();
    if ((sstring = _tag_file()) == "" ||
            (tag_buf = create_buffer("Tags", sstring, 1)) == -1) {
        return -1;
    }

    set_buffer(tag_buf);
    top_of_buffer();
    sstring = "^" + function;

    /* Scan for matches */
    while (re_search(NULL, sstring) > 0) {
        string line, file, pattern;
        int i, j, t;

        /* Bust line */
        line = read();
        i = index(line, "\t") + 1;

        /* ... file */
        if ((j = re_search(NULL, "[ \t][?/]", line)) > 0) {
            t = 2;                              /* regex */
        } else if ((j = re_search(NULL, "[ \t][0-9]", line)) > 0) {
            t = 1;                              /* line number */
        } else {
            continue;                           /* unknown format */
        }
        file = substr(line, i, j - i);
        trim(file);

        /* ... pattern */
        j += t;
        i = strlen(line) - j - 1;
        pattern = substr(line, j, i);

        /* ... exuberant Ctags support must remove trailing fields */
        if (t == 1) {
            if ((i = index(pattern, ";\"")) > 0) {
                pattern = substr(pattern, 1, i-1);
            }
        } else {
            if ((i = rindex(pattern, "/;\"")) > 0) {
                pattern = substr(pattern, 1, i-1);
            }
        }

        /* Enqueue */
        if (t == 1) {
            results += file + " (" + pattern + ")";
        } else {
            results += file;
        }
        results += file;
        results += 0;

        if (t == 1) {
            results += atoi(pattern);
        } else {
            results += pattern;
        }
        ++count;

        beginning_of_line();                    /* next */
        down();
    }

    set_buffer(old_buf);
    delete_buffer(tag_buf);

    _tag_results = results;
    _tag_count = count;
    return 0;
}


static string
_tags_sname(string name, int len, int maxlen)
{
    const int i = maxlen/2;

    if (len < maxlen) {
        return name;
    }
    return substr(name, 1, i) + "~" + substr(name, len - (maxlen - (i + 2)));
}


static void
_tag_list(int flags)
{
    UNUSED(flags);

    if (_tag_count > 1) {                       /* multiple matches */
        int idx;

        if (0 == flags || (flags & F_FILE)) {
            int cur, curbuf, win, buf;

            curbuf = inq_buffer();
            buf = create_buffer("Tags", NULL, TRUE);
            set_buffer(buf);
            for (idx = 0; idx < _tag_count; ++idx) {
                insertf("%-24s - %s\n", _tag_results[cur + TNAME], _tag_results[cur + TFILE]);
                cur += TITEMS;
            }
            delete_line();
            set_buffer(curbuf);
            win = sized_window(inq_lines(buf) + 1, -1, "<Esc> to quit.");
            idx = select_buffer(buf, win);
            delete_buffer(buf);
        } else {
            idx = select_slim_list("Tags", "", _tag_results, SEL_NORMAL, NULL, NULL, TITEMS);
        }

        if (idx > 0) {
            _tag_current = idx - 1;
            _tag_editfile(_tag_current);
        }
    }
}


static void
_tag_next()
{
    if (_tag_count <= 1) {
        beep();
    } else {
        if (++_tag_current >= _tag_count) {
            _tag_current = 0;
        }
        _tag_editfile(_tag_current);
    }
}


static void
_tag_prev()
{
    if (_tag_count <= 1) {
        beep();
    } else {
        if (--_tag_current < 0) {
            _tag_current = _tag_count - 1;
        }
        _tag_editfile(_tag_current);
    }
}


static int
_tag_editfile(int idx)
{
    int old_msg_level = set_msg_level(3);
    string file;
    declare ref;

    idx  *= TITEMS;
    file = _tag_results[idx + TFILE];
    ref  = _tag_results[idx + TPATTERN];

    if (edit_file(file) >= 0 ||                 /* absolute */
            edit_file(_tag_dir + file) >= 0) {  /* and relative */
        set_msg_level(old_msg_level);

        if (is_integer(ref)) {
            move_abs(ref);
        } else {
            string pattern, prefix, suffix;

            /* trim leading grep */
            pattern = ref;
            if (index("/?<", substr(pattern, 1, 1))) {
                pattern = substr(pattern, 2, strlen(pattern)-2);
            }

            /* quote any regex magic characters */
            if (substr(pattern, 1, 1) == "^") {
                prefix = "^";                   /* start of line */
                pattern = substr(pattern, 2);
                if (re_search(NULL, "\\$>", pattern) > 0) {
                    suffix = "$";               /* end of line */
                    pattern = substr(pattern, 1, strlen(pattern) - 1);
                }
            }
            pattern = prefix + quote_regexp(pattern) + suffix;

            /* search */
            save_position();
            top_of_buffer();
            if (re_search(SF_UNIX, pattern) > 0) {
                restore_position(0);
            } else {
                restore_position();
                error("tags: (%s) not found.", pattern);
            }
        }

    } else {
        set_msg_level(old_msg_level);
        error("tags: source not found. (%s)", file);
    }
}


static int
_tag_scheck(string dir)
{
    if (exist(dir + "tags")) {
        _tag_source = "tags";
        return 1;
    } else if (exist(dir + "ctags")) {
        _tag_source = "ctags";
        return 1;
    }
    return 0;
}


static string
_tag_file()
{
    if ("" == _tag_source) {                    /* search specified path */
        if ("" != _tag_grinit && "yes" != _tag_grinit) {
            string result = search_path(_tag_grinit, "tags");

            if ("" == result) {
                result = search_path(_tag_grinit, "ctags");
            }

            if ("" != result) {
                _tag_dir = dirname(result);
                return result;
            }
        }
    }

    if ("" == _tag_source) {                    /* search path */
        string dir;
        int depth;

        for (depth = 0; depth < DIR_DEPTH; ++depth) {
            if (_tag_scheck(dir)) {
                break;
            }
            if (depth >= DIR_DEPTH) {
                error("tags: cannot locate a tag file.");
                return "";
            }
            dir = "../" + dir;                  /* up a directory level */
        }
        _tag_dir = dir;
    }

    return _tag_dir + _tag_source;
}


/*
 *  mtags ---
 *      create a tags file for *.m files in current directory.
 */
void
mtags()
{
    int curbuf, buf;

    curbuf = inq_buffer();
    message("Creating .m tags file.");
    buf = perform_command("grep '^(macro' *.m | awk '{printf \"%s\t%s\\n\",$2,$0}' | sort", "");
    set_buffer(buf);
    top_of_buffer();
    re_translate(SF_GLOBAL, ":", "        /^");
    top_of_buffer();
    re_translate(SF_GLOBAL, "$", "$/");
    write_buffer("tags");
    set_buffer(curbuf);
    message("'tags' file created.");
}

/*eof*/
