/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: georgec.cr,v 1.16 2014/10/27 23:28:33 ayoung Exp $
 * User configuration
 *
 *
 */

#include "../grief.h"
#include "../help.h"

static void ExecuteCmdAndReloadFile(string cmd, string okmsg);

   
void
georgec(void)
{
    save_state();                               /* turn on full state saving */
    load_macro("scrollfixed");                  /* scroll without cursor movement */
    load_macro("short");                        /* short command set */
}


void
_highlight_colors(void)                         /* default syntax colors */
{
    set_color_pair("string",        "light-green", "blue");
    set_color_pair("operator",      "light-cyan",  "blue");
    set_color_pair("number",        "light-green", "blue");
    set_color_pair("comment",       "cyan",        "blue");
    set_color_pair("delimiter",     "light-cyan",  "blue");

    set_color_pair("preprocessor",  "light-red",   "blue");
    set_color_pair("preprocessor1", "light-red",   "blue");
    set_color_pair("preprocessor2", "light-red",   "blue");

    set_color_pair("keyword",       "yellow",      "blue");
    set_color_pair("keyword1",      "yellow",      "blue");
    set_color_pair("keyword2",      "yellow",      "blue");
}


void
sccs(string arg)
{
    if ( arg == "edit" )
        ExecuteCmdAndReloadFile("sccs edit", "File Checked out & Buffer now writable.");
    else if ( arg == "unedit" )
        ExecuteCmdAndReloadFile("sccs unedit", "File UnChecked out & Buffer now readonly.");
    else
        message("Unknown Arguement for sccs: " + arg );
}


void
co(string arg)
{
    if ( arg == "l" )
        ExecuteCmdAndReloadFile("co -l", "File locked & Buffer now writable.");
    else if ( arg == "u" )
        ExecuteCmdAndReloadFile("co -u", "File Unlocked & Buffer now readonly.");
    else
        message("Unknown Arguement for co: " + arg );
}


//  Executes cmd as sh -c \"cmd + file + >/dev/null 2>&1 \" and reloads file
static void
ExecuteCmdAndReloadFile(string cmd, string okmsg)
{
    string file;
    int  r;

    if ( cmd == "" )
    {
        message("Invalid Command passed");
        return;
    }

    if ( inq_views() == 1 )
    {
        r = inq_buffer_flags();

        if ( !(r & BF_CHANGED) )
        {
            inq_names(file);

            r = shell("sh -c \"" + cmd + " " + file + "> /dev/null 2>&1\"", 0);
            if ( r == 0 )
            {
                int topLine, topCol, cursorLine, cursorCol;
                int orgbuf, newbuf;

                inq_top_left(topLine, topCol, cursorLine, cursorCol);
                orgbuf = inq_buffer();
                newbuf = create_buffer("Showbuf");
                set_buffer(newbuf);
                delete_buffer(orgbuf);
                edit_file(file);
                delete_buffer(newbuf);
                set_top_left(topLine, topCol, cursorLine, cursorCol);
                if ( okmsg == "" )
                {
                    message(cmd + " completed successfully & file reloaded.");
                }
                else
                {
                    message(okmsg);
                }
            }
            else
            {
                message(cmd + " Failed");
            }
        }
        else
        {
            message("File has been modified, cannot run " + cmd );
        }
    }
    else
    {
        message("File is displayed in more than one window, cannot run " + cmd);
    }
}


void
lintlookup(~string)
{
    string htmlfile = "/opt/SUNWspro/WS6U2/lib/locale/C/LC_MESSAGES/SUNW_SPRO_SC_acomp.error_help.html";
    string errTag;
    string cmd;
    int line;
    int err_buf, old_buf = inq_buffer();

    if (!get_parm(0, errTag, "Lint Error Tag: ", NULL, NULL))
        return;
        
    if ( index(errTag, "E_") != 1 )
    {
        message("Selected Tag \" " + errTag + "\" does seem like a vaild error Tag");
        return;
    }

    cmd = "lynx -dump -force_html " + htmlfile;

    err_buf = perform_command(cmd, "Error Descriptions");
    set_buffer(err_buf);
    if ( search_fwd(errTag, 0) > 0 )            /* find tag */
    {
        /* get position and search back to line separator & if found get line */
        inq_position(line);
        if ( search_back("^ +_+ @$" ) > 0 )
        {
            inq_position(line);
        }
        /* else leave it where we found the error */
    }
    else
    {
        message("Cannot Find " + errTag + " in lint error message file");
        return;
    }

    set_buffer(old_buf);
    help_window( HELP_STANDARD, err_buf, 0, 0, line );
    delete_buffer(err_buf);                     /* destroy local image */
}


void
ll(void)
{
    string errTag;
    int line1, pos1, line2, pos2;

    inq_position(line1, pos1);
    word_right();
    inq_position(line2, pos2);
    move_abs(line1, pos1);
    if ( line1 == line2 )
    {
        errTag = read(pos2-pos1);
        if ( index(errTag, "E_") == 1 )
        {
            message("Looking up lint Error Tag " + errTag);
            lintlookup(errTag);
        }
        else
        {
            message("Selected Tag \" " + errTag + "\" does seem like a vaild error Tag");
        }
    }
    else
    {
        message("No word at end of line");
    }
}


void
makefile(string post, string pre)
{
    string fullFile, file, object, ext;
    int i;

    inq_names(fullFile);
    i = rindex(fullFile, "/");
    if ( i != 0 )
    {
        file = substr(fullFile, i + 1);
        i = rindex(file, ".");
        if ( i != 0 )
        {
            ext = substr(file, i + 1);
            object = substr(file, 1, i - 1) + ".o";

            if ( ext == "cc" )
            {
                extern void make(~string);

                message("Running: make bootstrap " + pre + " xx_var/obj.sparc/" + object + " " + post);
                remove("xx_var/obj.sparc/" + object );
                make("make bootstrap " + pre + " xx_var/obj.sparc/" + object + " " + post);
            }
            else
            {
                message("Not a .cc file");
            }
        }
    }
}


void
mf(string post, string pre)
{
    makefile(post, pre);
}


void
com(string comStart)
{
    if ( comStart == "" )
        comStart = "//";

    insert(comStart + "\n");
    insert(comStart + " [Date]   [User]  DEVL   [Ver] ...\n");
    insert(comStart + "\n");
}


void
rwstd
{
    insert("makelib::SetStandardLibs( 'S:rwtools7_std' );\n");
}


void
rose
{
    insert("Unrosed, untabbed & reformatted.");
}
/*eof*/
