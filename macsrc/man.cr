/* $Id: man.cr,v 1.38 2014/10/27 23:28:24 ayoung Exp $
 * 'man' command implementation.
 *
 *
 */

#include "grief.h"
#include "help.h"

#if defined(unix)
#define MANPATH         "/usr/man:/usr/share/man:/usr/X11/man:/usr/openwin/man:/usr/dt/man:/usr/local/man"
#else
#define MANPATH         "/usr/man:/usr/share/man:/usr/local/man"
#endif
#define MANCONFIG       "/etc/man.config"       /* standard name */
#define ALTMANCONFIG    "/tmp/grman%d.config"   /* alternative name */

void                    unman(void);
void                    apropos(string entry);

list                    man_glob2(string section, string entry);
list                    man_globit(string type, string section, string entry);
int                     man_search(string section, string entry);
static void             man_clean(int buf);
#if defined(linux)
void                    man_config(string filename);
string                  man_config_columns(string filename, int cols);
#endif

string                  man_path;
list                    man_extensions;


void
main(void)
{
    man_path = getenv("MANPATH");               /* user spec */
#if defined(linux)
    man_config( MANCONFIG );                    /* linux 'man' configuration */
#endif
    if (man_path == "") {                       /* MANPATH default */
        man_path = MANPATH;
    }
}


void
man()
{
    string arg;
    string entry = "";
    string section = "";
    int found = 0;                              /* result count */
    int argc = 0;
    int ret;

    for (argc = 0;;) {
        /*
         *  Get next argument. If argument not specified, and its the first argument,
         *  then prompt user. Otherwise we've finished scanning the argument list.
         */
        if (get_parm(argc, arg) <= 0) {
            if (entry != "") {
                break;
            }
            if (get_parm(argc, arg, "Manual entry: ", NULL, "") <= 0 || arg == "") {
                return;
            }
        }
        argc++;

        if (strlen(arg) == 1 &&                 /* single character section */
                index( "123456789abcdefghijklmnopqrstuvwxyz", arg )) {
            section = arg;
            continue;
        }
        if (strlen(arg) >= 1 &&                 /* multi character section */
                index("123456789", substr(arg,1,1))) {
            section = arg;
            continue;
        }

        if (substr(arg, 1, 2) == "/s") {        /* -s breaks things!, also 3c */
            if (arg == "/s") {
                if (get_parm(argc++, section, "Section: ", NULL, "") <= 0 || section == "") {
                    return;
                }
            } else {
                section = substr(arg, 3);
            }
            continue;
        }

        if (substr(arg, 1, 1) == "!") {         /* multi-character section */
            section = substr(arg, 2);
            continue;
        }

        if (substr(arg, 1, 1) == "?") {         /* keywork search (apropos) */
            apropos(substr(arg, 2));
            found++;                            /* suppress warning */
            continue;
        }

        entry = arg;
        if ((ret = man_search(section, entry)) <0) {
            found += -(ret+1);
            break;                              /* error/Esc */
        }
        found += ret;
    }

    if (found) {
        message("");
    } else if (strlen(section)) {
        error("Manual entry '%s(%s)' not found.", entry, section);
    } else {
        error("Manual entry '%s' not found.", entry);
    }
}


void
unman(void)
{
    save_position();
    top_of_buffer();
    re_translate(SF_GLOBAL, "?\b", "");         /* remove man style hilights */
    restore_position();
}


/*
 *  man_command ---
 *      execute the specified manual command 'cmd' using the arguments 'args'.
 *
 *  Notes:
 *      a) Under linux must force LANG under redhat, as the default is en_US.
 *      b) groff ignores MANWIDTH, as such must directly involve the man command.
 */
int
man_command( string cmd, string args )
{
#if !defined(unix)
    return perform_command( cmd + " " + args, "" );

#else
    string manwidth, altconfig;                 /* MANWIDTH and alternative config */
    int cols, ret;

    inq_screen_size(NULL, cols);                /* current width */
    if (cols > 128) {                           /* default */
        cols = 128;
    }
    if ((cols -= 8) < 60) {
        cols = 60;
    }
    manwidth = "MANWIDTH="+cols+"; ";           /* width env setting */

#if defined(linux)
    manwidth += "LANG=C; ";                     /* force LANG */
    if (cmd == "man") {                         /* groff specific width kludge */
        altconfig = man_config_columns(MANCONFIG, cols);
        args = "-t -E ascii " + args;           /* encoding */
        if (altconfig != "") {
            args = "-C "+altconfig+" "+args;    /* alternative config */
        }
    }
#endif  /*linux*/

#if defined(sun)
    if (cmd != "man") {
        args += " | tbl | col -x";              // 19/12/12
    }
#endif  /*sun*/

    if (cmd == "man") {
        cmd = manwidth + "man " + args;
    } else {
        cmd = manwidth + "nroff -man " + args;  /* linux, sun */
    }
    message(cmd);
    ret = perform_command("sh -c \"(" + cmd + ")\"");
    if (altconfig != "")
        remove(altconfig);                      /* remove temp config */
    redraw();
    return (ret);
#endif
}


list
man_glob(string section, string entry)
{
    list glist;

    glist = man_glob2(section, entry);
    if (length_of_list(glist) == 0 && section != "") {
        glist = man_glob2("", entry);
        if (length_of_list(glist)) {            /* alternative found */
            error( "Manual '%s(%s)' not found ... loading alternative", entry, section );
            sleep(1);
        }
    }
    return (glist);
}


list
man_glob2(string section, string entry)
{
    list glist;

    glist = man_globit("man", section, entry);  /* unformatted */
#if defined(sun)                                /* sgml pages */
    glist += man_globit("sman", section, entry);
#endif
    if (length_of_list(glist) == 0) {           /* preformatted */
        glist = man_globit( "cat", section, entry);
    }
    return (glist);
}


list
man_globit(string type, string section, string entry)
{
    string lsection;                            /* lower case section name */
    list glist, tlist;                          /* glob and temp file list */
    list paths;                                 /* paths */
    int i;

    /* bust man path, remove dups (if any) */
    paths = split(man_path, ":");
    for (i = 0; i < length_of_list(paths); i++) {
        int i2;

        for (i2 = i+1; i2 < length_of_list(paths); i2++)
            if (paths[i] == paths[i2]) {
                delete_nth(paths, i2);
                i2--;
            }
    }

    /* search */
    if (section == "")
        section = "*";
    lsection = lower(section);                  /* lower case version */

    for (i = 0; i < length_of_list(paths); i++) {
#if defined(sun)
        //  TODO
        //      paths/man.cf                    <= defines search order
        //          MANSECT=xxx[,xxx]
        //
#endif

        tlist = file_glob(paths[i] + "/" + type + section + "/" + entry + ".*");

        if (length_of_list(tlist) == 0 && section != lsection) {
            tlist = file_glob(paths[i] + "/" + type + lsection + "/" + entry + ".*");
        }
        if (length_of_list(tlist) == 0) {
            tlist = file_glob(paths[i] + "/*/"+ type + section + "/" + entry + ".*");
        }
        if (length_of_list(tlist) == 0 && section != lsection) {
            tlist = file_glob(paths[i] + "/*/" + type + lsection + "/" + entry + ".*");
        }
        glist += tlist;                         /* concat result */
    }
    return (glist);
}


int
man_search( string section, string entry )
{
    list    preventry, prevsection, prevfidx;   /* previous stack */
    int     previdx = 0;                        /* previous index */
    int     found;                              /* total match count */
    string  dir, current_dir;
    string  refer;
    list    flist;                              /* file list */
    int     fidx;                               /* file list index */
    list    l;                                  /* file component list */
    int     k;                                  /* file component index */
    string  f, s, n, x;                         /* file = subdir, name, ext */
    int     ret = 0;                            /* subprocess return value */
    int     curbuf, buf = -1;                   /* current buffer */

    UNUSED(ret);

    curbuf = inq_buffer();                      /* save callers working buffer */

    flist = man_glob( section, entry );         /* section/entry search */
    fidx = 0;

    while (fidx < length_of_list(flist)) {      /* for each matching file */
        /*
         *  Break the filename up into components because we want the
         *  filename part and the last directoryname part.
         */
        f = flist[fidx];                        /* current file */
        l = split(f, "/");                      /* delimiter */
        k = length_of_list(l);
        s = l[k - 2];                           /* subdir */
        n = l[k - 1];                           /* filename (includes extension) */
        x = substr(n, rindex(n, "."));          /* file extension */

        /*
         *  Compressed images
         */
        if (".Z" == x || ".gz" == x ||
                re_search(SF_NOT_REGEXP, x, man_extensions) > 0) {

            if (substr(s, 1, 3) == "cat") {     /* 'cat' page */
                message( "Uncompressing " + n );
                buf = perform_command("zcat '" + f + "'", f);

            } else {                            /* 'man' page */
                n = substr(n, 1, rindex(n, ".")-1);     /* remove extension */
                s = substr(n, rindex(n, ".")+1);        /* section */
                f = substr(n, 1, rindex(n, ".")-1);     /* man page */

                message("Man " + s + " " + f);
                buf = man_command("man", s +" '" + f +"'");
            }

        } else if (".z" == x) {                 /* solaris/bsd specific */
            message("Unpacking " + n);
            buf = perform_command("pcat '" + f + "'", f);

        /*
         *  Non-compressed images
         */
        } else if (substr(s, 1, 3) == "cat") {
            message("Viewing " + f );
            buf = perform_command("cat '" + f + "'", f);

        /*
         *  Unformatted images
         */
        } else {
            string type = "nroff";              /* default decoder */

            /*
             *  cd in order for the includes to work.
             */
            message("Formatting " + n);
            k = rindex(f, "/");
            getwd(NULL, current_dir);
            if (k > 0) {
                dir = substr(f, 1, k - 1);
                k = rindex(dir, "/");
                if (k > 0) {
                    dir = substr(dir, 1, k - 1);
                    {
                        string ss = dir;
                        cd(ss);
                    }
                    f = substr(f, strlen(dir) + 2);
                }
            }

            /*  Determine the encoding style
             *
             *  Processing SGML Manual Pages (Sun)
             *      Manual pages are identified as being marked up in SGML by the
             *      presence of the string <!DOCTYPE. If the file also contains
             *      the string SHADOW_PAGE the file refers to another manual page
             *      for the content. The reference is made with a file entity
             *      reference to the manual page that contains the text. This is
             *      similar to the .so mechanism used in the nroff formatted man
             *      pages.
             *
             *  eg.  <!DOCTYPE REFENTRY PUBLIC ...
             *           <!-- SHADOW_PAGE -->
             *           <!ENTITY DmiDeleteComponent-3x SYSTEM "./DmiAddComponent.3x">
             */
#if defined(sun)
            if ((buf = create_buffer("-man-", f, TRUE)) != -1) {
                string line;

                set_buffer(buf);
                line = read();
                if (re_search(SF_NOT_REGEXP, "<!DOCTYPE", line) > 0) {
                                                /* SGML mark up lanaguage */
                    type = "SGML";

                    if (re_search(SF_NOT_REGEXP, "SHADOW_PAGE") > 0) {
                                                /* shadow page */
                        if (re_search(0, "ENTITY * SYSTEM \"") > 0) {
                            int ei, xi;         /* end/extension index */

                            line = trim(read());
                            while ((ei = index(line, "/")) > 0) {
                                line = substr(line, ei+1);      /* strip path details */
                            }

                            ei = rindex(line, "\"");
                            if ((xi = rindex(line, ".")) <= 0) {
                                s = "", f = substr(line, 1, ei - 1);

                            } else {            /* section,entry */
                                s = substr(line, xi + 1, ei - (xi+1));
                                f = substr(line, 1, xi - 1);
                            }
                            type = "redirect";
                        }
                    }
                }
                set_buffer(curbuf);             /* restore buffer */
                delete_buffer(buf);
                buf = -1;
            }
#endif  /*for each special environment */

            /*
             *  Decode,
             *      '\" X,  where 'X' is separated from the `"' by a single SPACE and
             *      consists  of  any combination of characters in the following list,
             *      man pipes its input to nroff(1) through the corresponding
             *      preprocessors.
             *
             *          e  eqn(1), or neqn for nroff.
             *          r  refer(1)
             *          t  tbl(1)
             *          v  vgrind(1)
             *
             *      If eqn or neqn is invoked, it will automatically read the
             *      file /usr/pub/eqnchar (see eqnchar(5)). col(1) is automatically
             *      used.
             *
             *      Note: preferred order is, refer, tbl. eqn, nroff and col.
             */
            if (type == "redirect") {
                cd(current_dir);
                return man_search( s, f );      /* section, entry */
            }

            if (type == "SGML") {               /* solaris specific (2.6+) */
#if defined(sun)
                buf = perform_command("/usr/lib/sgml/sgml2roff '" + f + "' " +
                            "| tbl | eqn | nroff -man | col -x", f);
#endif
            } else if (type == "nroff") {
                buf = man_command( "nroff", "'" + f + "'" );
            }

            cd(current_dir);                    /* restore directory */
        }

        set_buffer(curbuf);                     /* restore buffer */

        /* man failure */
        if (buf < 0) {
            if (++fidx < length_of_list(flist)) /* for each matching file */
                continue;
            if (--previdx < 0)                  /* pop */
                break;
            entry = preventry[ previdx ];       /* reload previous */
            section = prevsection[ previdx ];
            fidx = prevfidx[ previdx ];

        } else {                                /* success, display image */
            found++;                            /* total matches */

            man_clean(buf);
            set_buffer(curbuf);

            refer = help_window(HELP_MAN, buf, 0, 0, 0, previdx,
                        (previdx == 0 ?                 /* top ? */
                        "<F10/Esc> exit" : "<F10/Esc> exit, <Back> prev" ) +
                            ((fidx+1) < length_of_list(flist) ? ", <Space> next" : "" ));

            set_buffer(curbuf);                 /* restore buffer */
            delete_buffer(buf);                 /* destroy local image */
            buf = -1;

            /* terminate */
            if (refer == "--exit--" || refer == "--esc--") {
                return -(found + 1);            /* exit (F10) */
            }

            /* previous/next */
            if (refer == "") {
                beep();                         /* no selection, reload */

            } else if (refer == "--apropos--") {
                continue;

            } else if (refer == "--prev--" || refer == "--space--") {
                if (refer == "--space--" &&
                        (fidx+1) < length_of_list(flist)) {
                    fidx++;
                    continue;                   /* next match */
                }

                if (previdx <= 0) {
                    if (refer == "--space--") {
                        break;                  /* end of list */
                    }
                    beep();                     /* bottom of stack, reload */

                } else {
                    previdx--;                  /* pop */
                    entry = preventry[ previdx ];       /* reload previous */
                    section = prevsection[ previdx ];
                    fidx = prevfidx[ previdx ];
                }

            } else {
                preventry[ previdx ] = entry;   /* push */
                prevsection[ previdx ] = section;
                prevfidx[ previdx ] = fidx;
                previdx++;

                if (substr(refer, 1, 3) == "(g)")
                    refer = substr(refer, 4);   /* remove GNU prefix */

                if ((k = index(refer, "(")) <= 0) {
                    entry = refer;              /* no section */
                    section = "";
                } else {
                    entry = trim(substr(refer, 1, k-1));
                    section = substr(refer, k+1);
                    if ((k = index(section, ")")) > 0)  /* locate closing ')' */
                        section = trim(substr(section, 1, k-1));
                }
                fidx = 0;                       /* reset file index */
            }
        } /* if (buf) */

        while (1) {                             /* glob directory */
            flist = man_glob( section, entry ); /* section/entry search */

            if (length_of_list(flist) != 0) {
                break;                          /* match */
            }
            beep();

            if (strlen(section)) {
                error("Manual entry '%s(%s)' not found.", entry, section);
            } else {
                error("Manual entry '%s' not found.", entry);
            }
            sleep(2);

            if (--previdx < 0) {
                break;                          /* shouldn't happen ! */
            }
            entry = preventry[ previdx ];
            section = prevsection[ previdx ];
            fidx = prevfidx[ previdx ];
        }
    }
    return (found);
}


static void
man_clean(int buf)
{
    string header, footer;

    set_buffer(buf);

    /* determine footer, last non-blank line */
    end_of_buffer();
    beginning_of_line();
    while ((footer = read()) == "\n") {
        up();
    }
    if (re_search(SF_IGNORE_CASE | SF_LENGTH,
                "<*{last}|{change}|{updated}[^0-9]+", footer) > 0) {
        footer = substr(footer, 1, 40);         /* use first 40 characters */
    } else {
        footer = "";
    }

    /* determine header, first non-blank line */
    top_of_buffer();
    while ((header = read()) == "\n") {
        down();
    }
    if (re_search(NULL, "\\([a-zA-Z0-9]+\\)[ \t]@>", header) > 0) {
        header = substr(header, 1, 40);         /* use first 40 characters */
    } else {
        header = "";
    }
    down();

    /* delete page breaks
     *
     *           [ blank-lines ]
     *       footer
     *           [ blank-lines ]
     *       header
     *           [ blank-lines ]
     *
     *  Note:
     *      Match only on first 40 character to deal with trailing page numbers.
     *
     */
    if (strlen(footer) && strlen(header)) {
        while (re_search(SF_NOT_REGEXP, footer) > 0) {
            if (move_rel(0, -1)) {              /* ! column 1 */
                continue;
            }
            up();
            while (read(1) == "\n") {
                up();
            }
            down();
            drop_anchor(MK_LINE);
            if (re_search(SF_NOT_REGEXP, header) <= 0) {
                break;
            }
            down();
            while (read(1) == "\n") {
                down();
            }
            up();
            delete_block();
        }
        raise_anchor();
    }
}


#if defined(linux)
void
man_config( string filename )
{
    int curbuf, buf;
    string line;

    curbuf = inq_buffer();
    if ((buf = create_buffer("-man-config-", filename, TRUE)) == -1) {
        return;
    }

    set_buffer(buf);
    top_of_buffer();
                                                /* MANPATH specifications */
    while (re_search(SF_UNIX, "^MANPATH[\t ]")>0) {
        line = substr(trim(read()), 9);
        man_path = man_path + line + ":";
        down();
    }

    top_of_buffer();
    while (re_search(SF_UNIX, "^\\.") > 0) {    /* extensions */
        list l;

        l = split(trim(read()), "\t");
        man_extensions += l[0];                 /* extension */
        down();                                 /* next line */
    }

    set_buffer(curbuf);                         /* restore buffer */
    delete_buffer(buf);
}

string
man_config_columns( string filename, int cols )
{
    int curbuf, buf;
    string  args;

    curbuf = inq_buffer();
    if ((buf = create_buffer("-man-config-alt-", filename, TRUE)) == -1) {
        return "";
    }

    set_buffer(buf);
    set_buffer_flags(NULL, NULL, ~BF_READONLY);
    top_of_buffer();

                                                /* NROFF specifications */
    while (re_search(SF_UNIX, "^NROFF[\t ]") > 0) {
                                                /* groff specific */
        args = " -rLL=" + cols + "n -rLT=" + cols + "n";
        end_of_line();
        insert( args );
    }

    sprintf(filename, ALTMANCONFIG, getpid());  /* specialize alternative */
    write_buffer(filename);                     /* write alternativc image */
    set_buffer(curbuf);                         /* restore buffer */
    delete_buffer(buf);
    return filename;
}
#endif  /*linux*/


/*
 *  get_dirs ---
 *      return a list of directories contained within the directory passed as an argument
 */
list
get_dirs(string dir)
{
    list l = NULL;
    string name;
    int mode;

    file_pattern(dir + "/*");
    while (find_file(name, NULL, NULL, NULL, mode)) {
        if (mode & S_IFDIR)
        l += name;
    }
    return l;
}


/*
 *  apropos ---
 *      display output of unix 'apropos' command in a window
 */
void
apropos(string entry)
{
    string line;
    string section;
    int curbuf, win, buf, ret;
    int ent, sect;

    if (strlen(entry) == 0) {                   /* missing entry paramter */
        error("Usage: apropos entry");
        return;
    }

    curbuf = inq_buffer();
    message( "apropos running ... please wait" );
    if ((buf = perform_command( "apropos " + entry, "Apropos-Buffer" )) == -1) {
        return;
    }

    set_buffer(curbuf);                         /* restore buffer */
    win = sized_window(inq_lines(buf)+1, inq_line_length(buf)+2,
                "<Esc/Space> exit, <Enter> select." );
    ret = select_buffer(buf, win, SEL_NORMAL,
                "_apropos_keys", NULL, NULL );
    if (ret < 0) {
        delete_buffer(buf);
        return;
    }
    message("");
    set_buffer(buf);
    goto_line(ret);
    line = rtrim(read());

    ent = re_search(NULL, "[, (\t]", line);     /* find end of first world */
    if (ent) {
        entry = substr(line, 1, ent-1);         /* entry as first word */
        if ((sect = index(line, "(")) > 0) {    /* locate section */
            section = substr(line, sect+1, index(line, ")") - (sect+1));
            if ((ent = re_search(NULL, "[ \t]+",line)) < sect) {
                                                /* handle 'alias manpage(section)' */
                entry = trim(substr(line, ent+1, sect - (ent+1)));
            }
            man_search(section, entry);
        } else {
            man_search("", entry);
        }
    }
}


/*
 *  _help_mapkeys ---
 *    Additional select buffer keys, for keyboard summary window
 */
void
_apropos_keys()
{
    assign_to_key( "<Left>",    "sel_esc" );
    assign_to_key( "<Space>",   "sel_esc" );
}


/*
 *  Local Variables: ***
 *  mode: cr ***
 *  tabs: 4 ***
 *  End: ***
 */
