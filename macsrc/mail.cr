/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: mail.cr,v 1.10 2014/10/27 23:28:24 ayoung Exp $
 * Macro to send and receive mail
 *
 *
 */

#include "grief.h"
#include "mode.h"

#if defined(MSDOS) || defined(OS2)
#define MAILPROG        "rmail"
#else
#define MAILPROG        "mail"
#endif

#if defined(__PROTOTYPES__)
static void             mail_watch(void);
static int              mail_changed(void);
static void             mail_send(string whom);
static void             mail_read(void);
static list             mail_keys(void);
static void             mail_save_unread(void);
static void             mail_quit(void);
static void             mail_delete(void);
static void             mail_write(void);
static void             mail_save(void);
static void             mail_write_msg(int save_headers);
static string           get_header(string name);
static list             mail_select_keys(void);
#endif

/*
 *  Following used to ensure that all mail response buffers have
 *  unique names.
 */
static int              mr_index = 1;

static int              mail_mtime;
static string           mailfile;
static int              is_cshell;


void
main(void)
{
    string maildir;
    string s;

    /* Remember whether we are using a cshell or not.  */
    s = getenv("SHELL");
    if (substr(s, rindex(s, "/") + 1) == "csh") {
        is_cshell = TRUE;

    } else if (substr(s, rindex(s, "/") + 1) == "tcsh") {
        is_cshell = TRUE;

    } else {
        is_cshell = FALSE;
    }

#if defined(MSDOS) || defined(OS2)
    mailfile = getenv("MAIL");
    if (mailfile == "") {
        maildir = inq_home();
        mailfile = maildir + "/" + getenv("LOGNAME") + ".f";
    }
    mail_mtime = -1;
    register_macro(REG_IDLE, "::mail_watch");
    mail_watch();

#else
    /* Work out where mail for this user is stored.  */
    mailfile = getenv("MAIL");
    if (mailfile == "") {
        /*
         *  Figure out whether we've got SysV or BSD mail system.
         */
        if (exist("/usr/spool/mail")) {
            maildir = "/usr/spool/mail";

        } else if (exist("/usr/mail")) {
            maildir = "/usr/mail";

        } else if (exist("/var/spool/mail")) {
            maildir = "/var/spool/mail";

        } else if (exist("/var/mail")) {
            maildir = "/var/mail";

        } else {
            maildir = "";
        }

        if (maildir != "") {
            mailfile = maildir + "/" + getenv("USER");
        }
    }
    mail_mtime = -1;
    register_macro(REG_IDLE, "::mail_watch");
    mail_watch();
#endif
}


static void
mail_watch(void)
{
    if (mail_changed()) {
        message("There is new mail.");
    }
}


/*
 *  Function returning TRUE if mail file has been modified.
 */
static int
mail_changed(void)
{
    int mtime;
    int fsize;

    file_pattern(mailfile);
    if (find_file(NULL, fsize, mtime) == 0) {
        mail_mtime = -1;
        return FALSE;
    }

    if (mtime != mail_mtime && fsize != 0) {
        mail_mtime = mtime;
        return TRUE;
    }
    return FALSE;
}


/*
 *  Shorthand to make it easier for user to type name of function.
 */
void
mail(void)
{
    string whom, old_mailfile;
    int old_mtime;

    if (mailfile = "") {
        message("Can't locate your mail folder directory.");
        return;
    }

    /* If no parameter specified then read the mail. */
    if (get_parm(0, whom) == 0) {
        mail_read();
        return;
    }

    /* If first character is '=' then read the specified folder. */
    if (substr(whom, 1, 1) == "=") {
        old_mailfile = mailfile;
        old_mtime = mail_mtime;
        mailfile = inq_home() + "/Mail/" +
            substr(whom, 2);
        file_pattern(mailfile);
        find_file(NULL, NULL, mail_mtime);
        message("Reading folder: %s", mailfile);
        mail_read();
        mailfile = old_mailfile;
        mail_mtime = old_mtime;
        return;
    }

    /* If we have a '%' then it's an absolute folder name.  */
    if (substr(whom, 1, 1) == "%") {
        old_mailfile = mailfile;
        old_mtime = mail_mtime;
        mailfile = substr(whom, 2);
        file_pattern(mailfile);
        find_file(NULL, NULL, mail_mtime);
        message("Reading file: %s", mailfile);
        mail_read();
        mailfile = old_mailfile;
        mail_mtime = old_mtime;
        return;
    }

    mail_send(whom);
}


/*
 *  Macro to send mail to someone.
 */
static void
mail_send(string whom)
{
    string subject, answer, filename, signfile;
    string month_name, day_name, date_string;
    string mail_cmd;
    int hours, mins, secs, year, mon, day;
    int mail_buf, mail_win;
    int old_win, curbuf;

    curbuf = inq_buffer();
    old_win = inq_window();

    get_parm(NULL, subject, "Subject: ");
    message("");

    sprintf(filename, "%s/CR-mail.%d", inq_tmpdir(), getpid());

    mail_buf = create_buffer("Mail-Buffer", filename, 0);
    mail_win = sized_window(24, 76, "Type <Esc> to terminate entry.");
    set_window(mail_win);
    attach_buffer(mail_buf);
    date(year, mon, day, month_name, day_name);
    time(hours, mins, secs);

    sprintf(date_string, "From %s %s %s %d %02d:%02d:%02d %d\n",
            getenv("LOGNAME"), substr(day_name, 1, 3), substr(month_name, 1, 3), day, hours, mins, secs, year);
    insert(date_string);
    insert("To: " + whom + "\n");
    insert("Subject: " + subject + "\n");
    insert("X-Mailer: CRISP-Mail v2.1\n");
    sprintf(date_string, "Date: %s %s %d %02d:%02d:%02d %d\n\n",
            substr(day_name, 1, 3), substr(month_name, 1, 3), day, hours, mins, secs, year);
    insert(date_string);

    select_editable();

    /* Insert users signature if he's got one. */
    end_of_buffer();
    insert("\n--\n");
    sprintf(signfile, "%s/.signature", inq_home());
    read_file(signfile);
    refresh();
    delete_window();

    /* Now ask user if he wants to send the mail response. */
    answer = "";
    while (answer != "y" && answer != "n") {
        get_parm(NULL, answer, "Really send (y/n) ? ", 1);
        answer = lower(answer);
    }

    write_buffer();

    if (answer == "y") {
        if (is_cshell) {
            sprintf(mail_cmd, "%s %s <%s >& %s/gr.mail.err", MAILPROG,
                    whom, filename, inq_tmpdir());
        } else {
            sprintf(mail_cmd, "%s %s <%s 2> %s/gr.mail.err", MAILPROG,
                    whom, filename, inq_tmpdir());
        }
        message(mail_cmd);
        shell(mail_cmd, 0);
    }
    set_window(old_win);
    set_buffer(curbuf);
    attach_buffer(curbuf);
    delete_buffer(mail_buf);
    remove(filename);
}


static void
mail_read(void)
{
    string from_line, subject_line, line;
    list who_list;
    int who_len, space, line_no, last_line_no,      /* Used to calculate message size.  */
        curbuf, hdrbuf,                             /* Buffer containing header summary */
        mailbuf,                                    /* Buffer containing mail file.     */
        hdrwin,                                     /* Window for pop-up                */
        quit_flag = FALSE;                          /* Set if user aborts mail reading. */

    curbuf = inq_buffer();

    /* See if there is any mail. */
    if (! exist(mailfile)) {
        error("There is no mail for you.");
        return;
    }

    /* Build up a list of who the mail is from */
    if (edit_file(mailfile) == -1) {
        return;
    }

    top_of_buffer();
    mailbuf = inq_buffer();
    hdrbuf = create_buffer("Mail Messages", NULL, 1);
    last_line_no = 0;

    while (re_search(NULL, "<From ") > 0) {
        right(5);
        inq_position(line_no);
        from_line = rtrim(read());
        space = index(from_line, " ");
        from_line = substr(from_line, 1, space - 1);

        /* Get subject line. */
        subject_line = "";
        if (re_search(NULL, "^{Subject: }|{From }") > 0) {
            if (read(1) == "S")
                subject_line = trim(substr(read(), 10));
        }

        if (strlen(subject_line) < 30)
            subject_line += "                                       ";
        subject_line = substr(subject_line, 1, 30);

        if (re_search(NULL, "<From ") <= 0)
            end_of_buffer();
        last_line_no = line_no;
        inq_position(line_no);

        set_buffer(hdrbuf);
        if (who_len != 0)
            insert("\n");

        sprintf(line, "%d. %s [%4d] %s", who_len + 1, subject_line, line_no - last_line_no, from_line);
        insert(line);
        set_buffer(mailbuf);
        who_list[2 * who_len] = last_line_no;
        who_list[2 * who_len + 1] = line_no;
        ++who_len;
    }

    if (who_len != 0) {
        message("Use <Esc> to exit and save changes; Q to exit.");

        hdrwin = sized_window(who_len + 1, 76, "<Alt-K> key bindings. <Alt-H> for help.");
        set_buffer(curbuf);
        attach_buffer(curbuf);
        select_buffer(hdrbuf, hdrwin, 0, mail_keys(),
                NULL, "help_display \"features/mail.hlp\" \"Mail\"");

        /* Save the unread mail back in the mailbox */
        if (quit_flag) {
            message("Quit - %s not updated.", mailfile);
        } else {
            mail_save_unread();
        }
    } else {
        message("There is no mail for you.");
    }

    /* Take user back to original buffer */
    set_buffer(curbuf);
    delete_buffer(mailbuf);
    attach_buffer(curbuf);
}


static list
mail_keys(void)
{
    assign_to_key("d",          "::mail_delete");
    assign_to_key("D",          "::mail_delete");
    assign_to_key("s",          "::mail_save");
    assign_to_key("S",          "::mail_save");
    assign_to_key("w",          "::mail_write");
    assign_to_key("W",          "::mail_write");
    assign_to_key("q",          "::mail_quit");
    assign_to_key("Q",          "::mail_quit");
    assign_to_key("<Enter>",    "::mail_select");

    return quote_list(
                "d          Delete message",
                "s          Save message (without headers)",
                "w          Save message (with headers)",
                "q          Abort mail reading.",
                "<Enter>    Read mail message.");
}


void
mail_save_unread(void)
{
    string line;
    int n, pos, start_line, end_line, num_saved, msg_no;
    extern list who_list;
    extern int who_len, mailbuf, hdrbuf;

    message("Updating %s...", mailfile);

    /* Delete read articles backwards. This is necessary otherwise we have
     * to cater for the fact that the line indexes in the who_list become
     * incorrect as we delete each read article.
     */
    for (n = who_len - 1; n >= 0; --n) {
        set_buffer(hdrbuf);
        goto_line(n + 1);
        line = read();
        pos = re_search(NULL, "[.*]", line);

        if (pos > 0 && substr(line, pos, 1) == "*") {
            msg_no = atoi(line) - 1;
            start_line = who_list[msg_no * 2];
            end_line = who_list[msg_no * 2 + 1] - 1;
            set_buffer(mailbuf);
            goto_line(start_line);
            drop_anchor(MK_LINE);
            goto_line(end_line);
            delete_block();
        } else {
            ++num_saved;
        }
    }

    set_buffer(mailbuf);
    if (mail_changed()) {
        error("Sorry - new mail arrived.");
        return;
    }

    if (num_saved == 0)
        clear_buffer();
    write_buffer();

    /* Force internal mod time to be updated. */
    mail_changed();
    message("%d message%s saved in %s",
        num_saved, num_saved == 1 ? "" : "s", mailfile);
}


static void
mail_quit(void)
{
    extern int quit_flag;

    quit_flag = TRUE;
    exit();
}


static void
mail_delete(void)
{
    re_translate(NULL, "[*.]*$", "* <Message Deleted>");
    beginning_of_line();
    push_back(key_to_int("<Down>"));
}


static void
mail_write(void)
{
    mail_write_msg(TRUE);
}


static void
mail_save(void)
{
    mail_write_msg(FALSE);
}


static void
mail_write_msg(int save_headers)
{
    string filename, msg;
    int curbuf, line_no, start_line, end_line;
    extern list who_list;
    extern int mailbuf;

    curbuf = inq_buffer();
    filename = inq_home() + "/mbox";
    if (save_headers) {
        msg = "Write to file: ";
    } else {
        msg = "Save to file: ";
    }
    get_parm(NULL, filename, msg, NULL, filename);

    /* Check for a folder name. */
    if (substr(filename, 1, 1) == "=") {
        filename = inq_home() + "/Mail/" + substr(filename, 2);
    }

    line_no = atoi(read()) - 1;

    set_buffer(mailbuf);
    start_line = who_list[line_no * 2];
    end_line = who_list[line_no * 2 + 1] - 1;
    goto_line(start_line);
    if (!save_headers) {
        re_search(NULL, "^$");
        down();
    }
    drop_anchor(MK_LINE);

    goto_line(end_line);
    if (write_block(filename, 1) <= 0) {
        message("Error whilst trying to write %s", filename);
    } else {
        message("Message saved in %s", filename);
    }

    set_buffer(curbuf);
    mail_delete();
}


/*
 *  Macro to send a mail reply
 */
static void
mail_reply(void)
{
    int mail_rep_buf;
    string filename, answer, mail_cmd, tmpfile, title;
    string from, subject, f;
    int curbuf, buf, rep_win, old_win, line_no, start_line, end_line, i;
    extern int top_line, mailbuf, hdrbuf;
    extern list who_list;

    curbuf = inq_buffer();
    old_win = inq_window();

    set_buffer(hdrbuf);
    line_no = atoi(read(4));
    title = trim(substr(read(), 4, 30));
    set_buffer(curbuf);
    if (title == "<Message Deleted>")
        return;

    --line_no;
    start_line = who_list[2 * line_no];
    end_line = who_list[2 * line_no + 1] - 1;

    set_buffer(mailbuf);
    goto_line(start_line);
    drop_anchor(MK_LINE);
    goto_line(end_line);
    sprintf(tmpfile, "%s/CRmail.%d", inq_tmpdir(), getpid());
    write_block(tmpfile);
    buf = create_buffer(title, tmpfile, 1);
    set_buffer(buf);

    sprintf(filename, "%s/gr.rep.%d", inq_tmpdir(), getpid());
    save_position();

    top_of_buffer();
    drop_anchor(MK_LINE);
    end_of_buffer();
    copy();
    restore_position();

    mail_rep_buf = create_buffer("Reply-Buffer-" + mr_index++, filename, 0);
    top_line++;
    rep_win = sized_window(24, 76, "Type <ESC> to terminate reply");
    set_window(rep_win);
    attach_buffer(mail_rep_buf);

    set_buffer(mail_rep_buf);
    top_of_buffer();
    paste();
    drop_anchor(MK_LINE);
    end_of_buffer();
    delete_block();

    set_buffer(buf);
    from = get_header("From:");
    subject = get_header("Subject:");
    set_buffer(mail_rep_buf);

    top_of_buffer();
    drop_anchor(MK_LINE);
    if (re_search(NULL, "^$") > 0) {
        delete_block();
        re_translate(SF_GLOBAL, "^", "> ");
        insert("\n");
        top_of_buffer();
        insert("To: ");
        insert(from);
        insert("\n");
        if (substr(upper(subject), 1, 3) != "RE:") {
            subject = "Re: " + subject;
        }
        insert("Subject:    " + subject + "\n");
        insert("X-Mailer: CRISP-Mail v2.1\n");
        insert("\n");
        end_of_buffer();
        drop_anchor(MK_LINE);
    }
    raise_anchor();
    end_of_buffer();

    unregister_macro(REG_TYPED, "sel_alpha");
    select_editable();

    register_macro(REG_TYPED, "sel_alpha", TRUE);
    --top_line;

    /* Insert users .signature if he's got one */
    end_of_buffer();
    insert("\n");
    read_file(inq_home() + "/.signature");
    refresh();
    delete_window();

    /* Now ask user if he wants to send the mail response */
    answer = "x";
    while (index("yn", answer) == 0) {
        get_parm(NULL, answer, "Really send (y/n) ? ", 1);
        answer = lower(answer);
    }

    if (answer == "y") {
        /* Escape characters which may cause the shell to blow up on the from name. */
        f = "";
        while ((i = index(from, "!")) > 0) {
            f += substr(from, 1, i - 1) + "\\!";
            from = substr(from, i + 1);
        }
        f += from;
        from = f;
        write_buffer();

        if (is_cshell) {
            sprintf(mail_cmd, "%s %s < %s >& %s/gr.mail.errors", MAILPROG,
                    from, filename, inq_tmpdir());
        } else {
            sprintf(mail_cmd, "%s %s < %s 2> %s/gr.mail.errors", MAILPROG,
                    from, filename, inq_tmpdir());
        }
        message(mail_cmd);
        shell(mail_cmd, 0);
    }

    set_window(old_win);
    set_buffer(curbuf);
    attach_buffer(curbuf);
}


/*
 *  This macro grabs an RFC header line. The name of the line is
 *  given by the argument passed to this macro. The RFC label is
 *  stripped off and a compressed version of the string after the
 *  colon is returned.
 */
static string
get_header(string name)
{
    string value;

    top_of_buffer();
    if (re_search(NULL, "^" + name) > 0) {
        value = read();
        value = substr(value, index(value, ":") + 1);
        value = trim(compress(value));

        /* If its the From: field then we need to strip off any comments. */
        if (name == "From:")  {
            if (index(value, "(")) {
                value = substr(value, 1, index(value, "(") - 1);
            }

            if (index(value, "<")) {
                value = substr(value, index(value, "<") + 1);
                value = substr(value, 1, index(value, ">") - 1);
            }
        }
        return value;
    }
    return "DONT KNOW";
}


static void
mail_select(void)
{
    int line_no, start_line, end_line, buf, win, curbuf, curwin;
    string tmpfile, title;
    extern list who_list;
    extern int mailbuf, top_line;

    curbuf = inq_buffer();
    curwin = inq_window();
    line_no = atoi(read(4));
    title = trim(substr(read(), 4, 30));
    if (title == "<Message Deleted>")
        return;
    save_position();
    --line_no;
    start_line = who_list[line_no * 2];
    end_line = who_list[line_no * 2 + 1] - 1;

    set_buffer(mailbuf);
    goto_line(start_line);
    drop_anchor(MK_LINE);
    goto_line(end_line);
    sprintf(tmpfile, "%s/CRmail.%d", inq_tmpdir(), getpid());
    write_block(tmpfile);
    raise_anchor();
    buf = create_buffer(title, tmpfile, 1);
    set_buffer(buf);

    /* Make sure window appears below the top-level mail popup so its visible. */
    ++top_line;
    win = sized_window(inq_lines() + 1, inq_line_length(), "<Esc> to return to main menu.");
    select_buffer(buf, win, 0, mail_select_keys());

    /* Restore the top_line variable for the sized_window() macro otherwise
     * all subsequent windows will appear one line further down the screen.
     */
    --top_line;
    remove(tmpfile);
    set_window(curwin);
    set_buffer(curbuf);
    attach_buffer(curbuf);
    top_of_buffer();
    restore_position();
    re_translate(NULL, "[*.]", "*");
    beginning_of_line();
    push_back(key_to_int("<Down>"));
}


/*
 *  Function called to set up private key bindings when user opts
 *  to read a mail message.
 */
static list
mail_select_keys(void)
{
    assign_to_key("<Ctrl-G>", "::mail_routines");
    assign_to_key("r", "::mail_reply");
    assign_to_key("R", "::mail_reply");

    return quote_list("r          Reply to mail.", "<Ctrl-G>   List mail headers.");
}


/*
 *  Macro called when user hits <Ctrl-G> for use with
 *  things like mailing lists, where we have lots of small
 *  subjects in one file.
 */
static void
mail_routines(void)
{
    routines_search("<Subject: ", 0, "Mail Topics", "mail_routines_trim");
}


static string
mail_routines_trim(string routine_name)
{
    int spos;

    spos = re_search(NULL, ":", routine_name);
    if (spos > 0)
        routine_name = substr(routine_name, spos + 1);
    return trim(routine_name);
}

/*end*/
