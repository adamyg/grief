/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: telnet.cr,v 1.9 2014/10/27 23:28:29 ayoung Exp $
 *
 *
 *
 */

#include "grief.h"

static void netlogin(string prog, string host, int flags);


void
telnet(~string)
{
    string host;

    if (get_parm(0, host) == 0)
        host = get_host_entry();
    if (host == "")
        return;
    netlogin("telnet", host, PF_NOINSERT);
}


void
rlogin(~string)
{
    string host;

    if (get_parm(0, host) == 0)
        host = get_host_entry();
    if (host == "")
        return;
    netlogin("rlogin", host, PF_NOINSERT);
}


void
ftp(~string)
{
    string host;

    if (get_parm(0, host) == 0)
        host = get_host_entry();
    if (host == "")
        return;
    netlogin("ftp", host, PF_ECHO);
}


void
ncftp(~string)
{
    string host;

    if (get_parm(0, host) == 0)
        host = get_host_entry();
    if (host == "")
        return;
    netlogin("ncftp -L", host, PF_ECHO);
}


static void
netlogin(string prog, string host, int flags)
{
    string cmd;

#if defined(OS2)
    create_shell(getenv("COMSPEC"), host + "-Buffer", flags, "\r");
    cmd = prog + " " + host + "\n";

#else
    create_shell("/bin/sh", host + "-Buffer", flags, "\r");
    cmd = "exec " + prog + " " + host + "\n";
#endif
    insert_process(cmd);
    if (flags & PF_ECHO)
        sh_line_mode();
    else
        sh_char_mode();
    refresh();
}


/*
 *  get_host_entry ---
 *      Macro to select an entry from the /etc/hosts file
 */
string
get_host_entry(void)
{
    int curbuf, buf, win, line_no;
    string host, hosts;

    message("Press <Enter> to select a host.");
    curbuf = inq_buffer();
    hosts = getenv("ETC");
    if (hosts == "")
        hosts = "/etc";
    hosts = hosts + "/hosts";

    if ((buf = create_buffer("-Remote Hosts-", hosts, 1)) == -1)
        return "";
    set_buffer(buf);
    set_buffer_flags(NULL, NULL, ~BF_READONLY);

    /* Delete comment lines. */
    top_of_buffer();
    while (re_search(NULL, "^#|$") > 0)
        delete_line();
    top_of_buffer();
    re_translate(SF_GLOBAL, "^[0-9A-Fa-fxX.]+[ \t]+{[-a-zA-Z0-9.]+}$|{[ \t#]*$}", "\\0");
    sort_buffer();
    win = sized_window(inq_lines(), 20, "");
    line_no = select_buffer(buf, win, NULL, NULL, NULL, "");
    if (line_no >= 0)
        host = trim(read());

    /* Close buffer (NOT SAVED) */
    delete_buffer(buf);
    set_buffer(curbuf);
    attach_buffer(curbuf);
    message("");
    return host;
}

/*eof*/
