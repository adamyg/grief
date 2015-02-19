/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: sweeper.cr,v 1.8 2014/10/27 23:28:32 ayoung Exp $
 * Minesweeper game.
 *
 *  Utilised to test the "new" list push/pop features.
 *
 *
 */

#include "../grief.h"

#define GRINITSECTION   "GRIEF-SWEEPER"

#define BOARD_WIDTH     20
#define BOARD_HEIGHT    20
#define BOARD_TOTAL     50

#define MQUESTIONED     -2
#define MMARKED         -1
#define MHIDDEN         0
#define MOPEN           1

static const list sweeper_help =
    {
        "Welcome to the Game of Minesweeper.",
        "",
        "<Left-Arrow> or <      Move Left",
        "<Right-Arrow> or >     Move Right",
        "<space>                Fire",
        "<Alt-X> or q           Terminate",
        "p                      Pause game"
    };


#if defined(__PROTOTYPES__)
static int              sweeper_hiscore(void);
static void             sweeper_play(void);
static void             sweeper_init(void);
static int              sweeper_count(int y, int x);
static void             sweeper_win(void);
static void             sweeper_click(int y, int x, int domark);
static void             sweeper_expose(int y, int x);
static void             sweeper_score(void);
static void             sweeper_draw(int y, int x);
#endif

/*
 *  minesweeper
 */
void
sweeper(void)
{
    int curbuf = inq_buffer();
    int curwin = inq_window();
    int score, hiscore;
    int mwidth, mheight, mtotal;
    int lines, cols;
    int buf, win;
    int echo;

    score = 0;
    hiscore = sweeper_hiscore();

    inq_screen_size(lines, cols);

    mwidth  = BOARD_WIDTH;
    mheight = BOARD_HEIGHT;
    mtotal  = BOARD_TOTAL;

    if (get_parm(0, mwidth) > 0) {
        mheight = mwidth;
        if (get_parm(1, mheight) > 0) {
            mtotal = (mwidth * mheight)/4;      // XXX - need a better method
            get_parm(2, mtotal);
        }
    }

    if (mwidth  < 5) mwidth  = BOARD_WIDTH;
    if (mheight < 5) mheight = BOARD_HEIGHT;

    if (mtotal  > (mwidth * mheight)/4)
        mtotal  = (mwidth * mheight)/4;         // XXX - need a better limit
    if (mtotal  < 5) mtotal  = 5;

    cols = mwidth + 20;
    lines = mheight + 3;

    if ((buf = create_buffer("Sweeper", NULL, TRUE)) < 0 ||
            (win = sized_window(lines, cols, "")) < 0) {
        if (buf >= 0) {
            delete_buffer(buf);
        }
        return;
    }

    set_window(win);
    set_buffer(buf);
    attach_buffer(buf);
    insert_mode(FALSE, buf);                    // disable insert.
    set_buffer_flags(NULL, BF_NO_UNDO);
    set_ctrl_state(WCTRLO_VERT_SCROLL, WCTRLS_HIDE, win);
    set_ctrl_state(WCTRLO_HORZ_SCROLL, WCTRLS_HIDE, win);

    message("Press <Alt-H> for help.");
    echo = echo_line(0);
    sweeper_play();
    echo_line(echo);

    message("%sScore: %d", score > hiscore ? "Hi-" : "", score);
    if (score > hiscore) {
        grinit_update(GRINITSECTION, "hiscore", format("%d", score));
    }

    delete_window(win);
    delete_buffer(buf);
    set_window(curwin);
    set_buffer(curbuf);
    attach_buffer(curbuf);
}


static int              
sweeper_hiscore(void)
{
    const string value =
            grinit_query(GRINITSECTION, "hiscore");
    int hiscore = 0;

    if (strlen(value)) {
        if ((hiscore = atoi(value)) < 0) {
            hiscore = 0;
        }
    }
    return hiscore;
}


/*
 *  sweeper_play ---
 *      Play the game.
 */
static void
sweeper_play(void)
{
    extern  int  mheight, mwidth, mtotal;
    int     mremaining, mmarked, mopened;
    list    mcell, mstatus;
    int     my, mx;

    UNUSED(mremaining, mmarked, mopened);
    UNUSED(mcell, mstatus);

    /* initialise board */
    sweeper_init();

    /* loop */
    while (1) {
        int ch;

        move_abs(my+2, mx+2);
        ch = read_char();

        keyboard_flush();
        switch (ch) {
        case key_to_int("<Alt-X>"):
        case key_to_int("<Esc>"):
        case 'q':
            return;

        case key_to_int("<Left-Arrow>"):
        case 'j':
            if (mx)
                --mx;
            break;

        case key_to_int("<Right-Arrow>"):
        case 'l':
            if (mx < mwidth-1)
                ++mx;
            break;

        case key_to_int("<Up-Arrow>"):
        case 'i':
            if (my)
                --my;
            break;

        case key_to_int("<Down-Arrow>"):
        case 'm':
            if (my < mheight-1)
                ++my;
            break;

        case key_to_int("<Keypad-5>"):
        case key_to_int("<Enter>"):
            sweeper_click(my, mx, TRUE);
            break;

        case ' ':
            sweeper_click(my, mx, FALSE);
            break;

        case 'x':
            sweeper_init();
            break;

        case 'b':
        case 'P':
            set_window_flags(NULL, WF_HIDDEN);
            message("Boss: ");
            read_char();
            set_window_flags(NULL, 0, ~WF_HIDDEN);
            break;

        case key_to_int("<Alt-H>"):
            select_list("Minesweeper Help", "Press <Esc> to resume play.",
                1, sweeper_help, SEL_NORMAL);
            break;
        }
    }
}


/*
 *  sweeper_init ---
 *      Initialise the board.
 */
static void
sweeper_init(void)
{
    extern list mcell, mstatus;
    extern int  mheight, mwidth, mtotal;
    extern int  mremaining, mmarked, mopened;
    extern int  my, mx;
    int x, y;
    list l;

    srand(time());

    // clear board
    for (x = 0; x <= mwidth; x++)
        l[x] = 0;

    for (y = 0; y <= mheight; y++) {
        mcell[y] = make_list(l);
        mstatus[y] = make_list(l);
    }

    // assign mines
    for (mremaining = 0; mremaining < mtotal;) {
        y = rand() % mheight; x = rand() % mwidth;
        if (mcell[y][x] == 0) {
            mcell[y][x] = -1;
            ++mremaining;
        }
    }

    // assign mine counts
    for (y = 0; y < mheight; ++y)
        for (x = 0; x < mwidth; ++x)
            if (mcell[y][x] == 0)
                mcell[y][x] = sweeper_count(y, x);

    for (y = 0; y < mheight; ++y)
        for (x = 0; x < mwidth; ++x)
            sweeper_draw(y, x);

    // open first cell
    do {
        y = rand() % mheight; x = rand() % mwidth;

    } while (mcell[y][x] != 0);

//  sweeper_expose(y, x);

    mmarked = mopened = 0;
    my = y; mx = x;

    sweeper_score();
}


/*
 *  sweeper_count ---
 *      Initialise the board.
 */
static int
sweeper_count(int y, int x)
{
    extern int mheight, mwidth, mtotal;
    extern int mremaining, mmarked, mopened;
    extern list mcell, mstatus;
    int notx = (x < (mwidth-1));
    int c = 0;

    // top line
    if (y > 0) {
        if (x > 0 && mcell[y-1][x-1] == -1)
            ++c;

        if (mcell[y-1][x] == -1)
            ++c;

        if (notx && mcell[y-1][x+1] == -1)
            ++c;
    }

    // left
    if (x > 0 && mcell[y][x-1] == -1)
        ++c;

    // right
    if (notx && mcell[y][x+1] == -1)
        ++c;

    // bottom column
    if (y < (mheight-1)) {
        if (x > 0 && mcell[y+1][x-1] == -1)
            ++c;

        if (mcell[y+1][x] == -1)
            ++c;

        if (notx && mcell[y+1][x+1] == -1)
            ++c;
    }

    return c;
}


/*
 *  sweeper_win ---
 *      Check if the board has been completed.
 */
static void
sweeper_win(void)
{
    extern int mheight, mwidth, mtotal;
    extern int mremaining, mmarked, mopened;

    if (mremaining == 0 && mmarked == mtotal &&
            (mmarked + mopened >= mwidth * mheight)) {
        message("You win!");
        refresh();
        sleep(4);
        sweeper_init();
    }
}


/*
 *  sweeper_click ---
 *      Process cell clicks.
 */
static void
sweeper_click(int y, int x, int domark)
{
    extern int mheight, mwidth, mtotal;
    extern int mremaining, mmarked, mopened;
    extern list mcell, mstatus;
    int status = mstatus[y][x];
    int cell = mcell[y][x];

    // mark cell (toggle)
    if (domark) {
        if (status == MHIDDEN) {
            if (mremaining <= 0)
                return;
            --mremaining;

            if (cell == -1)
                ++mmarked;

            mstatus[y][x] = MMARKED;
            sweeper_draw(y, x);
            sweeper_score();
            sweeper_win();                      // check status

        } else if (status == MMARKED) {
            if (cell == -1)
                --mmarked;
            ++mremaining;

            mstatus[y][x] = MQUESTIONED;
            sweeper_draw(y, x);
            sweeper_score();

        } else if (status == MQUESTIONED) {
            mstatus[y][x] = MHIDDEN;
            sweeper_draw(y, x);
        }
        return;
    }

    // can't opena marked square!
    if (status < MHIDDEN)
        return;

    // hit a mine?
    if (cell == -1) {
        for (y = 0; y < mheight; ++y)
            for (x = 0; x < mwidth; ++x)
                if (mcell[y][x] == -1) {        // expose all mines
                    mstatus[y][x] = MOPEN;
                    sweeper_draw(y, x);
                }
        message("You lose!");
        refresh();
        sleep(5);
        sweeper_init();
        return;

    // nop ... expose
    } else {
        if (0 == cell) {
            sweeper_expose(y, x);               // expose neighbours

        } else {    /*cell > 0*/
            if (mstatus[y][x] == MHIDDEN) {     // expose hidden
                mstatus[y][x] = MOPEN;
                sweeper_draw(y, x);
                ++mopened;
            }
        }
    }

    sweeper_score();
    sweeper_win();
}


/*
 *  sweeper_expose ---
 *      Expose a cell (and neighbors if possible).
 */
static void
sweeper_expose(int y, int x)
{
    extern int mheight, mwidth, mopened;
    extern list mcell, mstatus;

    list stack;
    int  ym = mheight-1;
    int  xm = mwidth-1;
    int  cell;

    push(stack, x, y);

    while (length_of_list(stack) > 1) {
        y = pop(stack);
        x = pop(stack);

        // hidden and not a mine
        if (mstatus[y][x] != MHIDDEN)
            continue;

        if ((cell = mcell[y][x]) < 0)
            continue;

        // open cell
        mstatus[y][x] = MOPEN;
        sweeper_draw(y, x);
        ++mopened;

        // fill neighbours
        if (cell > 0)
            continue;

        if (x > 0) {                            // left
            if (y > 0)
                push(stack, x-1, y-1);
            push(stack, x-1, y);
            if (y < ym)
                push(stack, x-1, y+1);
        }

        if (y > 0)                              // top
            push(stack, x, y-1);

        if (y < ym)                             // bottom
            push(stack, x, y+1);

        if (x < xm) {                           // right
            if (y > 0)
                push(stack, x+1, y-1);
            push(stack, x+1, y);
            if (y < ym)
                push(stack, x+1, y+1);
        }
    }
}


/*
 *  Update the score
 */
static void
sweeper_score(void)
{
    extern int mheight, mwidth, mtotal;
    extern int mremaining, mmarked, mopened;

    move_abs(2, mwidth+4);
    delete_to_eol();
    insert("Total:     " + mtotal);

    move_abs(3, mwidth+4);
    delete_to_eol();
    insert("Remaining: " + mremaining);

//  move_abs(4, mwidth+4);
//  delete_to_eol();
//  insert("Time:      " + mtime);

//  move_abs(5, mwidth+4);
//  delete_to_eol();
//  insert("Marked:    " + mmarked);

//  move_abs(6, mwidth+4);
//  delete_to_eol();
//  insert("Open:      " + mopened);
}


/*
 *  sweeper_draw ---
 *      Update the given cell.
 */
static void
sweeper_draw(int y, int x)
{
    extern list mcell, mstatus;
    int status = mstatus[y][x];

    move_abs(y+2, x+2);

    switch (status) {
    case MMARKED:           // user marked
        self_insert('@');
        break;

    case MQUESTIONED:       // user questioned
        self_insert('?');
        break;

    case MHIDDEN:           // unknown
        self_insert('.');
        break;

    case MOPEN: {           // open
            int cell = mcell[y][x];

            if (cell == -1) {
                self_insert('*');               // mine

            } else if (cell == 0) {
                self_insert(' ');               // blank

            } else {
                insertf("%d", cell);            // count
                delete_char();
            }
        }
        break;
    }
}

/*end*/
