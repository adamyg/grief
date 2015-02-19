/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: snake.cr,v 1.4 2014/10/27 23:28:32 ayoung Exp $
 * Game of Snake.
 *
 *
 */

#include "../grief.h"

#define GRINITSECTION   "GRIEF-SNAKE"

#define BOARD_WIDTH     40
#define BOARD_HEIGHT    20

#define SIZEMIN         6
#define SIZEMAX         100

#define FOODMIN         5
#define FOODMAX         10

#define TMUNIT          800                     // milliseconds
#define TMFRAME         50                      // frame per speed increment
#define SPEEDMAX        15                      // (TMFRAME * SPEEDMAX) < TMUNIT

enum {
    LEFT,
    UP,
    RIGHT,
    DOWN
};

enum {
    PIECE,
    FOOD,
    VOID
};

static const list snake_help = {
    "       Game of Snake.",
    "",
    " <Left-Arrow>      Move Left.  ",
    " <Right-Arrow>     Move Right. ",
    " <Up-Arrow>        Move Left.  ",
    " <Down-Arrow>      Move Right. ",
    " <Alt-X> or <q>    Terminate.  ",
    " <Alt-B> or <b>    Boss mode.  ",
    " <x>               Restart.    ",
    "",
    };

static int              snake_hiscore(void);
static void             snake_play(void);
static int              snake_time(void);
static void             snake_init(void);
static int              snake_move(void);
static int              snake_hit(int x, int y);
static void             snake_head(int x, int y);
static void             snake_tail(void);
static int              snake_draw(int x, int y, int state);

static void             food_generate(~ int count);
static int              food_hit(int x, int y, int rm);


void
snake(void)
{
    int curbuf = inq_buffer();
    int curwin = inq_window();
    int score = 0, hiscore = snake_hiscore();
    int bx, by;
    int buf, win;
    int echo;

    bx = BOARD_WIDTH;
    by = BOARD_HEIGHT;

    if (get_parm(0, bx) > 0) {
        get_parm(1, by);
    }

    if (bx < 5) bx = BOARD_WIDTH;
    if (by < 5) by = BOARD_HEIGHT;

    if ((buf = create_buffer("Snake", NULL, TRUE)) < 0 ||
            (win = sized_window(by + 1, bx + 1, "")) < 0) {
        if (buf >= 0) {
            delete_buffer(buf);
        }
        return;
    }

    set_window(win);
    set_buffer(buf);
    attach_buffer(buf);
    insert_mode(FALSE, buf);                    // disable insert
    set_buffer_flags(NULL, BF_NO_UNDO);
    set_ctrl_state(WCTRLO_VERT_SCROLL, WCTRLS_HIDE, win);
    set_ctrl_state(WCTRLO_HORZ_SCROLL, WCTRLS_HIDE, win);

    message("Press <Alt-H> for help.");
    echo = echo_line(0);
    snake_play();
    echo_line(echo);

    message("%sScore: %d", (score > hiscore ? "Hi" : ""), score);
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
snake_hiscore(void)
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
snake_play(void)
{
    extern int score, bx, by;

    list body, food;                            // body/food elements.
    int  size, speed, moves;                    // statistics.
    int  sx, sy, sd;                            // snake position and direction.
    int  ftm, ctm;                              // time references.
    int  ch;

    snake_init();
    food_generate(FOODMIN);
    srand(time());

    UNUSED(body, food);
    UNUSED(size, speed, moves);
    UNUSED(bx, by);
    UNUSED(sx, sy, sd);

    moves = 0;
    while (1) {
        move_abs(sy, sx);
     // message("Score: %d, Moves: %d, Speed: %d, Size: %d", score, moves, speed, size);
        ch = read_char(TMUNIT / TMFRAME);
        keyboard_flush();

        if (ch > 0) {
            switch (ch) {
            case key_to_int("<Alt-X>"):
            case key_to_int("<Esc>"):
            case 'q': case 'Q':
                return;

            case key_to_int("<Left-Arrow>"):
            case 'j': case 'J':
            case '<':
                sd = LEFT;
                break;

            case key_to_int("<Right-Arrow>"):
            case 'l': case 'L':
            case '>':
                sd = RIGHT;
                break;

            case key_to_int("<Up-Arrow>"):
            case 'i': case 'I':
            case '^':
                sd = UP;
                break;

            case key_to_int("<Down-Arrow>"):
            case 'm': case 'M':
            case 'v':
                sd = DOWN;
                break;

            case 'x': case 'X':
                snake_init();
                food_generate(5);
                clear_buffer();
                break;

            case key_to_int("<Alt-B>"):
            case 'b': case 'B':
                set_window_flags(NULL, WF_HIDDEN);
                message("Boss: ");
                read_char();
                set_window_flags(NULL, 0, ~WF_HIDDEN);
                break;

            case key_to_int("<Alt-H>"):
                select_list("Snake Help", "Press <Esc> to resume play.",
                    1, snake_help, SEL_NORMAL);
                break;
            }
        }

        if ((ctm = snake_time()) >= ftm) {
            if (! snake_move()) {
                return;                         // done.
            }
            ftm = ctm + (TMUNIT - (TMFRAME * speed));
        }
    }
}


static void
snake_init(void)
{
    extern int score, size, speed, moves;       // statistics
    extern int sx, sy, sd;                      // snake position and direction
    extern int bx, by;                          // bounds
    extern int ftm;                             // time references

    ftm   = snake_time() + 1000;
    score = 0;
    size  = SIZEMIN;
    speed = 1;
    moves = 0;
    sx    = bx/2;
    sy    = by/2;
    sd    = UP;
}


static int
snake_time(void)
{
    int msecs;
    return (time(NULL, NULL, NULL, msecs) * 1000) + msecs;
}


static int
snake_move(void)
{
    extern int score, size, speed, moves;       // statistics.
    extern int sx, sy, sd;                      // snake position and direction.
    extern int bx, by;                          // bounds.

    /* move head (if possible) */
    switch (sd) {
    case UP:
        if (sy > 1)  sy--;
        break;
    case DOWN:
        if (sy < by) sy++;
        break;
    case LEFT:
        if (sx > 1)  sx--;
        break;
    case RIGHT:
        if (sx < bx) sx++;
        break;
    }

    /* check new position
     *  note: failure to relocate the head results in a match, as such
     *    includes hitting a boundary plus doubling back on the body.
     */
    if (snake_hit(sx, sy)) {
        return FALSE;                           // snake has been hit
    }
    ++moves;

    /* move head */
    snake_head(sx, sy);

    /* food */
    if (food_hit(sx, sy, TRUE)) {               // grow new food

        food_generate(1);

        if (speed < SPEEDMAX) {                  
            ++speed;                            // increase speed
        }

        if (size < SIZEMAX) {
            size += 2;                          // increase size
        }

        message("Score: %d", ++score);

    } else if (speed < SPEEDMAX) {
        if (0 == (moves % (SPEEDMAX - speed))) {
            if (size < SIZEMAX) {
                ++size;                         // increase size */
            }
        }
    }

    /* erase trailing image */
    snake_tail();

    return TRUE;
}


static int
snake_hit(int x, int y)
{
    extern list body;                           // body elements
    list pt;

    while (list_each(body, pt) >= 0) {
        if (pt[0] == x && pt[1] == y) {
            return TRUE;
        }
    }
    return FALSE;
}


static void
snake_head(int x, int y)
{
    extern list body;                           // body elements

    snake_draw(x, y, PIECE);
    push(body, make_list(x, y));
}


static void
snake_tail(void)
{
    extern list body;                           // body elements
    extern int size;                            // size limit

    while (length_of_list(body) > size) {
        list pt = shift(body);
        snake_draw(pt[0], pt[1], VOID);
    }
}


static int
snake_draw(int x, int y, int state)
{
    extern int moves;

    move_abs(y, x);
    switch (state) {
    case PIECE:
        self_insert(moves % 2 ? 'o' : '0');
        break;
    case FOOD:
        self_insert('@');
        break;
    default:
        self_insert(' ');
        break;
    }
}


static void
food_generate(int count = 1)
{
    extern list body, food;                     // body/food elements
    extern int bx, by;                          // bounds
    int xu = bx, yu = by;
    int x, y;

    while (--count >= 0) {
        do {
            x = rand(xu) + 1;
            y = rand(yu) + 1;

        } while (food_hit(x, y, FALSE) || snake_hit(x, y));

        snake_draw(x, y, FOOD);
        push(food, make_list(x, y));
    }
}


static int
food_hit(int x, int y, int rm)
{
    extern list food;                           // food elements
    list pt;
    int idx;

    while ((idx = list_each(food, pt)) >= 0) {
        if (pt[0] == x && pt[1] == y) {
            if (rm) {
                splice(food, idx, 1);
            }
            return TRUE;
        }
    }
    return FALSE;
}
/*end*/





