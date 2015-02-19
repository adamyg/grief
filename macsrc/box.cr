/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: box.cr,v 1.8 2014/10/27 23:28:17 ayoung Exp $
 * Macros to help in drawing diagrams
 *
 *
 */

static int  box_tl,
            box_tr,
            box_bl,
            box_br,
            box_hz,
            box_vt;

void
main()
{
    single_box();
}


void
single_box()
{
#if defined(MSDOS) || defined(OS2)
    box_tl = 0xda;                              /* MCHAR??? */
    box_tr = 0xbf;
    box_bl = 0xc0;
    box_br = 0xd9;
    box_hz = 0xc4;
    box_vt = 0xb3;
#else
    box_tl = ',';
    box_tr = '.';
    box_bl = '`';
    box_br = '\'';
    box_hz = '-';
    box_vt = '|';
#endif
}


void
double_box()
{
#if defined(MSDOS) || defined(OS2)
    box_tl = 0xc9;                              /* MCHAR??? */
    box_tr = 0xbb;
    box_bl = 0xc8;
    box_br = 0xbc;
    box_hz = 0xcd;
    box_vt = 0xba;
#else
    box_tl = ',';
    box_tr = '.';
    box_bl = '`';
    box_br = '\'';
    box_hz = '=';
    box_vt = '|';
#endif
}


void
dobox(int length, int height)
{
    int i, line, col, old_insert_mode;

    old_insert_mode = insert_mode(1);
    if (length > 1 && height > 1) {
        inq_position(line, col);
        self_insert(box_tl);

        for (i = 2; i < length; ++i) {
            self_insert(box_hz);
        }

        self_insert(box_tr);

        move_abs(line + 1, col);

        for (i = 2; i < height; ++i) {
            self_insert(box_vt);
            down();
            left();
        }

        move_abs(line, col + length);

        for (i = 2; i < height; ++i) {
            down();
            left();
            self_insert(box_vt);
        }

        move_abs(line + height - 1, col);

        self_insert(box_bl);

        for (i = 2; i < length; ++i) {
            self_insert(box_hz);
        }

        self_insert(box_br);

        move_abs(line, col);
    }

    insert_mode(old_insert_mode);
}



void
box()
{
    int start_line, start_col, end_line, end_col;

    if (! inq_marked(start_line, start_col, end_line, end_col)) {
        message("No marked region");

    } else {
        int line, col;

        inq_position(line, col);
        move_abs(start_line, start_col);
        dobox(end_col - start_col + 1, end_line - start_line + 1);
        move_abs(line, col);
    }
}

/*end*/
