/* -*- mode: cr; tabs: 4; -*- */
/* $Id: tetris.cr,v 1.13 2014/10/27 23:28:32 ayoung Exp $
 * Tetris game.
 *
 *  This implementation is based on the X11 version by Rob Mayoff
 *  and the various HP-48SX versions.
 *
 *
 */

#include "../grief.h"

#define DEBUG           0

#define GRINITSECTION   "GRIEF-TETRIS"
#define TETRISMODE      "Tetris"

#define HEIGHT          20
#define WIDTH           10
#define PWIDTH          2                       // width of a square.

#define NUM_PIECES      7
#define NUM_MOVES       4                       // number of moves before piece moves down a line.

#define LEVEL_LINE      2
#define LEVEL_COL       PWIDTH * WIDTH + 8
#define SCORE_LINE      4
#define SCORE_COL       PWIDTH * WIDTH + 8
#define HISCORE_LINE    6
#define HISCORE_COL     PWIDTH * WIDTH + 8

#define STARTING_DELAY  350                     // block delay

#define MIN_DELAY       (STARTING_DELAY / 10)   // minimum time delay at the highest game level.

#if defined(DEBUG) && (DEBUG)
#undef  NUM_MOVES
#define NUM_MOVES       99
#undef  STARTING_DELAY
#define STARTING_DELAY  20000
#endif

static int              tetris_hiscore(void);
static void             tetris_play(void);
static int              piece_play(void);
static void             piece_draw(int x, int y, list piece);
static void             piece_erase(int x, int y, list piece);
static int              piece_blocked(int x, int y, list piece);
static int              check_for_completed_lines(void);
static void             draw_score(int level, int score);


/*
 *  buffer view
 */
static int              tetris_cmap = -1;
static list             tetris_view =
    {
    ".",  ".",  ".",  ".",  ".",  ".",  ".",  ".",  ".",  ".",  ".",  ".",  ".",  ".",  ".",  ".",
    ".",  ".",  ".",  ".",  ".",  ".",  ".",  ".",  ".",  ".",  ".",  ".",  ".",  ".",  ".",  ".",
    " ",  "!",  "\"", "#",  "$",  "%",  "&",  "'",  "(",  ")",  "*",  "+",  ",",  "-",  ".",  "/",
    "0",  "1",  "2",  "3",  "4",  "5",  "6",  "7",  "8",  "9",  ":",  ";",  "<",  "=",  ">",  "?",
    "@",  "A",  "B",  "C",  "D",  "E",  "F",  "G",  "H",  "I",  "J",  "K",  "L",  "M",  "N",  "O",
    "P",  "Q",  "R",  "S",  "T",  "U",  "V",  "W",  "X",  "Y",  "Z",  "[",  "\\", "]",  "^",  "_",
    "`",  "a",  "b",  "c",  "d",  "e",  "f",  "g",  "h",  "i",  "j",  "k",  "l",  "m",  "n",  "o",
    "p",  "q",  "r",  "s",  "t",  "u",  "v",  "w",  "x",  "y",  "z",  "{",  "|",  "}",  "~",  ".",
    ".",  ".",  ".",  ".",  ".",  ".",  ".",  ".",  ".",  ".",  ".",  ".",  ".",  ".",  ".",  ".",
    ".",  ".",  ".",  ".",  ".",  ".",  ".",  ".",  ".",  ".",  ".",  ".",  ".",  ".",  ".",  ".",
    ".",  ".",  ".",  ".",  ".",  ".",  ".",  ".",  ".",  ".",  ".",  ".",  ".",  ".",  ".",  ".",
    ".",  ".",  ".",  ".",  ".",  ".",  ".",  ".",  ".",  ".",  ".",  ".",  ".",  ".",  ".",  ".",
    ".",  ".",  ".",  ".",  ".",  ".",  ".",  ".",  ".",  ".",  ".",  ".",  ".",  ".",  ".",  ".",
    ".",  ".",  ".",  ".",  ".",  ".",  ".",  ".",  ".",  ".",  ".",  ".",  ".",  ".",  ".",  ".",
    ".",  ".",  ".",  ".",  ".",  ".",  ".",  ".",  ".",  ".",  ".",  ".",  ".",  ".",  ".",  ".",
    ".",  ".",  ".",  ".",  ".",  ".",  ".",  ".",  ".",  ".",  ".",  ".",  ".",  ".",  ".",  ".",
    };


/*
 *  Score table, used to give the user a score depending on the piece and the orientation
 *  of the object at the point it lands. This table is in the same order as the pieces
 *  description following.
 */
static list             score_tbl =
    {
        {10, 5, 5, 8},
        {10, 8, 5, 5},
        {10, 5},
        {10, 5},
        {10, 5},
        {10},
        {10, 5, 8, 5}
    };

/*
 *  piece definitions
 */
#define BLOCK           "H"

static list             pieces =
    {
        /*******************************/
        {{"",
            "HHHHHH",
            "HH"},

        { "  HH",
            "  HH",
            "  HHHH"},

        { "    HH",
            "HHHHHH"},

        { "HHHH",
            "  HH",
            "  HH"}

        },

        /*******************************/
        {{"",
          "HHHHHH",
          "    HH"},

        { "  HHHH",
          "  HH",
          "  HH"},

        { "HH",
          "HHHHHH"},

        { "  HH",
          "  HH",
          "HHHH"}
        },

        /*******************************/
        {{"",
          "HHHH",
          "  HHHH"},

        { "  HH",
          "HHHH",
          "HH"}
        },

        /*******************************/
        {{"",
          "  HHHH",
          "HHHH"},

        { "HH",
          "HHHH",
          "  HH"}
        },

        /*******************************/
        {{"HHHHHHHH"},

        { "    HH",
          "    HH",
          "    HH",
          "    HH"}
        },

        /*******************************/
        {{"HHHH",
          "HHHH"}
        },

        /*******************************/
       {{"      ",
          "HHHHHH",
          "  HH"},

        { "  HH",
          "  HHHH",
          "  HH"},

        { "  HH",
          "HHHHHH"},

        { "  HH",
          "HHHH",
          "  HH"}
        }
    };


/*
 *  popup help.
 */
static list             tetris_help = {
    "Welcome to the Game of Tetris. The following keys",
    "are used to manipulate the blocks as they fall:",
    "",
    "<Left-Arrow> or j            Move Left",
    "<Right-Arrow> or l           Move Right",
    "<Up-Arrow> or i              Rotate block",
    "<Keypad-5>                   Drop block",
    "<Down-Arrow>                 Move down",
    "<Alt-X> or q                 Terminate Tetris",
    "<Space>                      Pause game"
    };


void
tetris()
{
    int curbuf = inq_buffer();
    int curwin = inq_window();
    int hiscore, score = 0, level = 0;
    int buf, win;

#if DEBUG
    srand(1);                                   // seed random
#endif

    hiscore = tetris_hiscore();
    buf = create_buffer("Tetris", NULL, TRUE);
#define LEFT_X  20
    win = create_window(LEFT_X, HEIGHT + 1, LEFT_X + PWIDTH * WIDTH + 20, 1);
    set_buffer(buf);
    if (tetris_cmap < 0) {
        create_syntax(TETRISMODE);
        syntax_token(SYNT_OPERATOR, BLOCK);
        tetris_cmap = create_char_map(NULL, NULL, tetris_view);
    }
    set_ctrl_state(WCTRLO_VERT_SCROLL, WCTRLS_HIDE, win);
    set_ctrl_state(WCTRLO_HORZ_SCROLL, WCTRLS_HIDE, win);
    attach_syntax(TETRISMODE);
    attach_buffer(buf);

    message("Press <Alt-H> for help.");

    tetris_play();

    message("Press any key to clear window.");
    read_char();

    message("%sScore: %d  Level: %d", score > hiscore ? "Hi-" : "", score, level);
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
tetris_hiscore(void)
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


static void
tetris_play(void)
{
    extern int score, level;

    string blank_line;
    string block, complete_line;
    int lines_completed = 0;
    int piece_num;
    int delay = STARTING_DELAY;
    int dir;
    int i;

    UNUSED(lines_completed);

    /* Make sure first four characters on each line are non-blank, so we can't move 
     *  left into them. These characters are not displayed in the window.
     */
    blank_line = "   |";
    for (i = PWIDTH * WIDTH; i-- > 0;) {
        blank_line += " ";
    }
    blank_line += "|\n";

    /* string used to test for a complete line. */
    for (i = 0; i++ < PWIDTH;) {
        block += BLOCK;
    }
    for (i = 0, complete_line = ""; i++ < WIDTH;) {
        complete_line += block;
    }

    insert(blank_line, HEIGHT);
    top_of_buffer();

    while (1) {

        if ((i = piece_play()) < 0) {
            break;
        }

        score += score_tbl[piece_num][dir] + i;
        i = check_for_completed_lines();
        if (i) {
            score += i * i * 40;
            if (delay > MIN_DELAY) {
                delay = STARTING_DELAY - (level * STARTING_DELAY) / 10;
            }
        }
    }
}


static int
piece_play(void)
{
    extern int piece_num, dir;
    extern int score, level, delay;

    int num = 0;
    int incr = 0;
    int drop_piece = 0;
    int num_moves;
    int num_dropped = 0;                        // number of levels we drop
    int new_dir;
    int x, y;
    int ch;
    list ps, p;

    x = WIDTH / 2 + 4; y = 1;
    piece_num = rand(length_of_list(pieces));
    ps = pieces[piece_num];
    dir = 0;

    num_moves = NUM_MOVES;

    while (1) {

        if (y + length_of_list(p) >= HEIGHT) {
            piece_draw(x, y, p);
            break;
        }

        p = ps[dir];
        draw_score(level, score);
        if (piece_blocked(x, y, p)) {
            break;
        }

        piece_draw(x, y, p);
        if (drop_piece == 0) {
            ch = read_char(delay);
            keyboard_flush();
            num_moves--;
            switch (ch) {
            case key_to_int("<Alt-X>"):
            case key_to_int("<Esc>"):
            case 'q':
                return -1;
            case key_to_int("<Left-Arrow>"):
            case 'j':
                incr = -PWIDTH;
                break;
            case key_to_int("<Right-Arrow>"):
            case 'l':
                incr = PWIDTH;
                break;
            case key_to_int("<Up-Arrow>"):
            case 'i':
                piece_erase(x, y, p);
                new_dir = (dir + 1) % length_of_list(ps);
                if (!piece_blocked(x, y, ps[new_dir]))
                dir = new_dir;
                break;
            case ' ':
                message("Game paused - any key to continue:");
                read_char();
                break;
            case 'b':           /* boss mode :) */
            case 'P':
                set_window_flags(NULL, WF_HIDDEN);
                message("Boss: ");
                read_char();
                set_window_flags(NULL, 0, ~WF_HIDDEN);
                break;
            case key_to_int("<Keypad-5>"):
                drop_piece = TRUE;
                break;
            case key_to_int("<Down-Arrow>"):
            case -1:
                num_moves = 0;
                break;
            case key_to_int("<Alt-H>"):
                select_list("Tetris Help", "Press <Esc> to resume play.",
                            1, tetris_help, SEL_NORMAL);
                break;
            }
        }
        piece_erase(x, y, p);

        if (num_moves <= 0 || drop_piece) {
            if (piece_blocked(x, y + 1, p)) {
                piece_draw(x, y, p);
                break;
            }
            y++;
            if (drop_piece) {
                num_dropped++;
            }
            num_moves = NUM_MOVES;
        }

        if (incr < 0 && x + incr >= 1 && !piece_blocked(x + incr, y, p)) {
            x += incr;
        } else if (incr > 0 && !piece_blocked(x + incr, y, p)) {
            x += incr;
        }
        incr = 0;
        num++;
    }

    if (y >= HEIGHT) {
        piece_draw(x, y - 1, p);
    }
    return (num == 0 ? -1 : num_dropped);
}


static void
piece_draw(int x, int y, list piece)
{
    int i, j, llen = length_of_list(piece);
    string s;

    for (i = 0; i < llen; i++) {
        s = piece[i];

        if ((j = index(s, BLOCK)) != 0) {
            move_abs(y + i, x + j - 1);
            s = trim(substr(s, j));
            insert(s);
            delete_char(strlen(s));
        }
    }

    set_top_left(1, 4);
    move_abs(1, 5);
    refresh();
}



static void
piece_erase(int x, int y, list piece)
{
    int i, j, len, llen = length_of_list(piece);
    string s;

    move_abs(y, x);
    for (i = 0; i < llen; i++) {
        s = piece[i];
        len = strlen(s);

        for (j = 1; j <= len; j++) {
            if (substr(s, j, 1) != " ") {
                delete_char();
                insert(" ");
            } else {
                move_rel(NULL, 1);
            }
        }
        move_rel(1, -len);
    }
    move_abs(1, 1);
}


/*
 *  piece_blocked ---
 *      Check to see if we can put the piece at (x,y) in the grid.
 */
static int
piece_blocked(int x, int y, list piece)
{
    int i, j, llen = length_of_list(piece);
    string s;

    move_abs(y, x);
    for (i = 0; i < llen;) {
        s = piece[i++];

        /* If we've got a non-blank string, then check that for all
         * character positions containing a BLOCK that we have a space
         * in the buffer.
         */
        if (j = index(s, BLOCK)) {
            /* The following statement is as follows: grab number of
             * strings from buffer corresponding to number of characters
             * in this line. Ignore leading space characters. Reduce all
             * spaces to a single space. We should be left with one space
             * only if this line of the block will fit.
             */
            if (compress(substr(read(strlen(s)), j)) != " ") {
                return TRUE;
            }
        }
        down();
    }
    return FALSE;
}


static int
check_for_completed_lines(void)
{
    extern int lines_completed;
    extern string complete_line;
    extern string blank_line;
    extern int level, score;

    int i, num_completed = 0;

    for (i = 1; i <= HEIGHT;) {
        move_abs(i, 5);

        if (read(WIDTH * PWIDTH) != complete_line) {
            i++;
            continue;
        }

        /* now animate the disappearance of the line. */
        move_abs(i, 4 + PWIDTH * WIDTH);
        drop_anchor(MK_NORMAL);
        move_abs(i, 5);
        while (read(1) == BLOCK) {
            delete_char();
            insert(" ");
            refresh();
        }
        raise_anchor();

        /* erase score and level before trying to delete a line 
         * because we may end up scrolling the buffer. 
         */
        if (num_completed++ == 0) {
            move_abs(LEVEL_LINE, LEVEL_COL);
            delete_to_eol();
            move_abs(SCORE_LINE, SCORE_COL);
            delete_to_eol();
            move_abs(HISCORE_LINE, HISCORE_COL);
            delete_to_eol();
        }
        move_abs(i, 1);
        delete_line();
        move_abs(1, 1);
        insert(blank_line);
        move_abs(1, 5);

        if ((lines_completed++ & 0x0f) == 0) {
            level++;
        }
    }

    /* restore score display */
    if (lines_completed) {
        draw_score(level, score);
        refresh();
    }

    return num_completed;
}


static void
draw_score(int level, int score)
{
    extern int hiscore;

    move_abs(LEVEL_LINE, LEVEL_COL);
    delete_to_eol();
    insert("Level: " + level);
    move_abs(SCORE_LINE, SCORE_COL);
    delete_to_eol();
    insert("Score: " + score);
    move_abs(HISCORE_LINE, HISCORE_COL);
    delete_to_eol();
    insert("Hi-Score: " + hiscore);
}
/*end*/
