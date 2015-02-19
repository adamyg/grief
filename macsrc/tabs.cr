/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: tabs.cr,v 1.12 2014/10/27 23:28:29 ayoung Exp $
 * tab handling
 *
 *
 */

#include "grief.h"

#if defined(__PROTOTYPES__)
static void             tab_process_lines(string macroname, int startline, int endline);
#endif

static string           spacefill;


void
main(void)
{
    int width = 132;

    spacefill = " ";
    while (width--) {
        spacefill += " ";
    }
}


void
show_tabs(void)
{
    if (inq_buffer_flags(NULL, "hiwhitespace")) {
        set_buffer_flags(NULL, NULL, "hiwhitespace");
    } else {
        set_buffer_flags(NULL, "hiwhitespace");
    }
    message("Toggled Tab Highlighting");
}


/*
 *  detab ----
 *      detab the current line or marked region.
 */
void
detab(void)
{
    int start_line, start_col, end_line, end_col, type;

    type = inq_marked(start_line, start_col, end_line, end_col);
    if (type == MK_NONE) {
        inq_position(start_line);
        end_line = start_line;
    }
    tab_process_lines("detab", start_line, end_line);
}


/*
 *  entab ----
 *      entab the current line or marked region.
 */
void
entab(void)
{
    int start_line, start_col, end_line, end_col, type;

    type = inq_marked(start_line, start_col, end_line, end_col);
    if (type == MK_NONE) {
        inq_position(start_line);
        end_line = start_line;
    }
    tab_process_lines("entab", start_line, end_line);
}


/*
 *  detab_str ---
 *      detab a single line Fills tabs with spaces at current tab setting.
 */
string
detab_str(string line)
{
    int pos;

    while ((pos = index(line, "\t")) > 0) {
        int expand = distance_to_tab(pos);
        line = substr(line, 1, pos-1) +substr(spacefill, 1, expand) + substr(line, pos+1);
    }
    return line;
}


/*
 *  backfill ---
 *      Backfill all whitespace
 */
static string
backfill(int atcol, int fillto)
{
    int tabinc;
    string result = "";

    if (atcol != fillto) {
        tabinc = distance_to_tab(atcol);
        while ((atcol + tabinc) <= fillto) {
            result += (tabinc == 1) ? " " : "\t";
            atcol += tabinc;
            tabinc = distance_to_tab(atcol);
        }
        result += substr(spacefill, 1, fillto - atcol);
        atcol = fillto;
    }
    return result;
}


/*
 *  entab_str ---
 *      entab a single line. Fills > 1 consecutive spaces with tabs at current tab
 *      setting.
 */
string
entab_str(string line)
{
    int atcol, col, pos, tabinc;
    string result = line;

    result = "";
    atcol = col = 1;
    while ((pos = re_search( NULL, "[ \t]", line )) > 0) {
        /* Find next whitespace */
        if (pos > 1) {
            result += backfill(atcol, col) + substr(line, 1, pos - 1);
            col += pos - 1;
            atcol = col;
        }

        tabinc = (substr(line, pos, 1) == " ") ? 1 : distance_to_tab(col);
        col += tabinc;
        line = substr(line, pos + 1);
    }
    return result + backfill(atcol, col) + line;
}


/*
 *  tab_process ---
 *      Process a group of lines in the current buffer with a given macro
 */
static void
tab_process_lines(string macroname, int startline, int endline)
{
    int line, lines, save_line, save_col, top_line, top_col;
    string temp;

    inq_top_left(top_line, top_col);
    inq_position(save_line, save_col);

    lines = endline - startline + 1;
    for (line = 0; line++ < lines;) {
        int percent = line * 100 / lines;

        if ((percent % 10) == 0)
            message("%s %d%% done", macroname, percent);
        move_abs(line + startline - 1, 1);
        temp = read();
        delete_line();
        temp = execute_macro(macroname + "_str", temp);
        insert(rtrim(temp)+"\n");               /* APY, was trim, hmmmmm */
        down();
    }

    set_top_left(top_line, top_col);
    move_abs(save_line, save_col);
}


/*
 *  tab_process_marked ---
 *      Process all marked lines within a marked region
 */
static void
tab_process_marked(string macroname)
{
    int start_line, start_col, end_line, end_col;

    if (inq_marked(start_line, start_col, end_line, end_col) == 0) {
        error("No marked block.");
        return;
    }
    raise_anchor();
    tab_process_lines(macroname, start_line, end_line);
}


/*
 *  tab_process_all ---
 *      Process all lines in the current buffer with a given macro
 */
static void
tab_process_all(string macroname)
{
    tab_process_lines(macroname, 1, inq_lines());
}


/*
 *  detab_buffer ---
 *      Detab all lines in current buffer
 */
void
detab_buffer(void)
{
    tab_process_all("detab");
}


/*
 *  detab_region ---
 *      Detab marked lines in current buffer
 */
void
detab_region(void)
{
    tab_process_marked("detab");
}


/*
 *  entab_buffer ---
 *      Entab all lines in current buffer
 */
void
entab_buffer(void)
{
    tab_process_all("entab");
}


/*
 *  entab_region ---
 *      Entab marked lines in current buffer
 */
void
entab_region(void)
{
    tab_process_marked("entab");
}


/*
 *  print_tabs ---
 *      This macro was written for testing the new form of distance_to_tab()
 */
void
print_tabs(void)
{
    int i = 0;

    save_position();
    for (i = 0; i < 3; ++i) {
        insert("\n");
        beginning_of_line();
        up();
    }

    beginning_of_line();
    for (i = 0; i++ < 80;) {
        int t = distance_to_tab(i);
        int x = 0;

        while (x++ < 3) {
            string yy;

            switch (x)
            {
            case 1:
                yy = (i / 10);
                break;
            case 2:
                yy = (i % 10);
                break;
            case 3:
                yy = t;
                break;
            }
            insert(yy);
            left();
            down();
        }
        up(5);
        right();
    }
    restore_position();
}

/*eof*/
