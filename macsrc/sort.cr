/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: sort.cr,v 1.10 2014/10/27 23:28:28 ayoung Exp $
 * Sort marked block/file.
 *
 *
 */

#include "grief.h"

#if defined(__PROTOTYPES__)
extern void             sort(~int, ~int);
#endif


/*
 *  sort_keywords ---
 *      Sort the file (or marker area) in an order suitable for use as keyword
 *      list, being accending by length then ascii.
 */
void
sort_keywords(void)
{
    sort(0, 1);                                 /* by length */
}


void
sort_numeric(void)
{
    sort(0, 2);                                 /* by numeric */
}


/*
 *  sort ---
 *      Sort the file (or marked area).
 */
void
sort(~int, ~int)
{
    int    lc, rc, tl, bl, mt, cb, ib, co, cl_len;
    int    option_rv, option_type = 0;
    string cl, cl_cmp;                          /* current line and its compare image */
    string line;                                /* temp line */

    get_parm(0, option_rv);                     /* reverse */
    get_parm(1, option_type);                   /* ASCII, length(1) or numeric(2) */
    save_position();
    mt = inq_marked(tl, lc, bl, rc);
    raise_anchor();

    if (! mt) {
        tl = 1;
        end_of_buffer();
        inq_position(bl);
    } else {
        if (mt != 2) {
            lc = rc = 0;
        } else {
            rc -= lc - 1;
        }
    }

    cb = inq_buffer();
    if ((ib = create_buffer("-sort_buffer-", NULL, TRUE)) == -1) {
        return;
    }
    mt = (bl - tl) + 2;

    goto_line(tl);
    while (bl >= tl) {
        --bl;
        cl = read();
        if (! lc) {
            cl_len = strlen(cl);
        } else {
            cl_cmp = substr(cl, lc, rc);
        }

        cl_len = strlen(cl_cmp);                /* marked region, subtext */
        delete_line();
        set_buffer(ib);
        end_of_buffer();

        co = up();
        while (co){
            line = read();
            if (lc) {
                line = substr(line, lc, rc);
            }

            switch(option_type) {
            case 2:     /* sort by numeric value */
                co = atoi(cl_cmp) < atoi(line);
                break;

            case 1:     /* sort by length */
                co = cl_len < strlen(line);
                if (co == 0) {
                    co = cl < line;             /* .. subsort by, ascii */
                }
                break;

            case 0:     /* ascii (dafault) */
                co = cl < line;
                break;
            }

            if (option_rv < 0)                  /* reverse order */
                co = !co;
            if (co) {
                co = up();
            } else {
                down();
            }
        }

        insert(cl);
        set_buffer(cb);

        if (read_char(-1) == key_to_int("<ESC>")) {
            while (read_char(-1) != -1)
                /*void*/;
            delete_buffer(ib);
            push_back(key_to_int(inq_assignment("undo", 1)));
            return;
        }
    }

    transfer(ib, 1, 1, mt, 1);
    delete_buffer(ib);
    up();
    delete_line();
    restore_position();
}

/*end*/
