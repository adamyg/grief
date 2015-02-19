/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: invaders.cr,v 1.11 2014/10/27 23:28:31 ayoung Exp $
 * ASCII Invaders game.
 *
 *  Utilised to test many of the "new" list features and screen update/display
 *  functionality.
 *
 *  This implementation is based loosly on the idea of a curses based version
 *  by Thomas Munro.
 *
 *
 */

#include "../grief.h"

#define GRINITSECTION       "GRIEF-INVADERS"

#define FPS                 25                  /* frames per second */

#define TM_UNIT             (1000 / FPS)

#define ALIEN_NONE          0
#define ALIEN_EX3           1
#define ALIEN_EX2           2
#define ALIEN_EX1           3
#define ALIEN_10            10
#define ALIEN_20            20
#define ALIEN_30            30

#define ALIEN_BASEY         3
#define ALIEN_WIDTH         6
#define ALIEN_HEIGHT        3
#define ALIEN_IHEIGHT       2
#define ALIEN_COLS(cols)    ((cols / ALIEN_WIDTH) * 4 / 5)
#define ALIEN_ROWS(rows)    ((rows / ALIEN_HEIGHT) * 3 / 5)

#define BOMB_MAX            5

#define GUNNER_WIDTH        7
#define GUNNER_MISSILE      3
#define GUNNER_HEIGHT       2
#define GUNNER_STATES       3

#define SHELTER_WIDTH       7
#define SHELTER_HEIGHT      3
#define SHELTER_SPACING     10
#define SHELTER_CHAR        'M'

#define BOMB_CHAR           '*'

#define MA_HEIGHT           2
#define MA_WIDTH            6

static int  iv_cmap = -1;

static list iv_alien30 =
    {   {   " {@@} ",
            " /\"\"\\ "
        },
        {   " {@@} ",
            "  \\/  "
        }
    };

static list iv_alien20 =
    {   {   " dOOb ",
            " ^/\\^ "
        },
        {   " dOOb ",
            " ~||~ "
        }
    };

static list iv_alien10 =
    {   {   " /^^\\ ",
            " |~~| "
        },
        {   " /^^\\ ",
            " \\~~/ "
        }
    };

static list iv_alienexp =
    {   {   " \\||/ ",
            " /||\\ "
        },
        {   "  \\/  ",
            "  /\\  "
        }
    };

static list iv_aliennone =
    {   "      ",
        "      ",
        "      "
    };

static list iv_alienMa =
    {   "_/XX\\_",
        "qWAAWp"
    };

static list iv_gunner =
    {   {   "  [^]  ",
            " /IHI\\ "
        },
        {   " ,' %  ",
            " ;&+,! "
        },
        {   " -,+$! ",
            " +  ^~ "
        }
    };

static list iv_shelter =
    {   " MMMMM ",
        "MMMMMMM",
        "MMM MMM"
    };

static list iv_help =
    {
        "Welcome to the Game of ASCII Invaders.",
        "",
        "<Left-Arrow> or <      Move Left",
        "<Right-Arrow> or >     Move Right",
        "<space>                Fire",
        "<Alt-X> or q           Terminate",
        "p                      Pause game"
    };

#define IVMODE              "Invaders"

#if defined(__PROTOTYPES__)
static int                  invaders_hiscore(void);
static int                  invaders_play(void);
static void                 shelter_paint(void);
static void                 gunner_paint(int x, int state);
static int                  missile_mov(int mx, int my);
static void                 missile_paint(int gx, int mx, int my);
static void                 alien_init(void);
static void                 alien_mov(void);
static void                 alien_paint(int state);
static int                  bomb_drop(void);
static void                 bomb_paint(void);
static int                  bomb_move(void);
#endif

void
invaders(void)
{
    int curbuf = inq_buffer();
    int curwin = inq_window();
    int lines, cols;
    int score, remaining, level, hiscore;
    int buf, win;
    int echo;

    score = remaining = level = 0;
    hiscore = invaders_hiscore();

    if (iv_cmap < 0) {
        create_syntax(IVMODE);
        syntax_token(SYNT_OPERATOR, "*");
        syntax_token(SYNT_COMMENT,  "M");
        syntax_token(SYNT_NUMERIC,  "\\/X@dOb|");
        iv_cmap = 0;
    }

    inq_screen_size(lines, cols);

    if (cols > 80) cols = 80;
    cols = ((cols - 5) / SHELTER_SPACING) * SHELTER_SPACING;
    cols -= SHELTER_SPACING - SHELTER_WIDTH;

    lines -= 5;

    if ((buf = create_buffer("-*- ASCII Invaders -*-", NULL, TRUE)) < 0 ||
            (win = sized_window(lines, cols, "")) < 0) {
        return;
    }

    set_window(win);
    set_buffer(buf);
    attach_buffer(buf);
    insert_mode(FALSE, buf);                    /* disable insert */
    set_buffer_flags(NULL, BF_NO_UNDO);
    set_ctrl_state(WCTRLO_VERT_SCROLL, WCTRLS_HIDE, win);
    set_ctrl_state(WCTRLO_HORZ_SCROLL, WCTRLS_HIDE, win);
    attach_syntax(IVMODE);

    message("Press <Alt-H> for help.");
    srand(1);

    echo = echo_line(0);
    while (1) {
        ++level;
        if (invaders_play() == -1) {
            break;
        }
        clear_buffer();
    }
    echo_line(echo);

    message("%sScore: %d  Level: %d", (score > hiscore ? "Hi-" : ""), score, level);
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
invaders_hiscore(void)
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
 *  invaders ---
 *      Play the game of invaders.
 */
static int
invaders_play(void)
{
    extern int lines, cols, score, remaining, level;

    list aliens, bombs;

    int a_rows, a_cols, a_top, a_left, a_lidx;
    int a_state, a_steps, a_step;
    int b_steps, b_step;

    int g_x, g_xo, g_state;                     /* gunner details */

    int m_x, m_y;                               /* missile details */

    int btm, ctm, frametm;
    int msecs;
    int ch;

    int update;

    g_x = 1;
    a_steps = 12;
    b_steps = 6;
    btm = time();

    a_state = 0;
    a_top  = ALIEN_BASEY;
    a_cols = ALIEN_COLS(cols);
    a_rows = ALIEN_ROWS(lines);
    a_left = 0;
    a_lidx = 1;                                 /* col=0, active count */

    alien_init();
    shelter_paint();
    update = 1;

    UNUSED(aliens);
    UNUSED(bombs);

    remaining = a_cols * a_rows;                // initial count
    message("Score: %d  Level: %d, Remaining: %d", score, level, remaining);

    while (remaining > 0) {

        ctm = ((time(NULL, NULL, NULL, msecs) - btm) * 1000) + msecs;

        if (g_state) {
            if (ctm >= frametm + 500) {
                if (++g_state >= GUNNER_STATES) {
                    return -1;
                }
                gunner_paint(g_x, g_state);
                frametm = ctm;
            }

        } else {
            if (ctm >= frametm + TM_UNIT) {
                --a_step;
                --b_step;

                if (a_step <= 0) {
                    a_step = a_steps;
                    a_state++;
                    alien_mov();
                    update = 0x01;
                }

                update |= bomb_drop();

                if ((m_y = missile_mov(m_x, m_y)) == 0) {
                    update |= 0x1;              /* missile hit something */
                    m_x = 0;
                }

                frametm = ctm;
            }

            if (g_x != g_xo) {                  /* gunner movement */
                gunner_paint(g_x, g_state);
                g_xo = g_x;
            }

            if (b_step <= 0) {
                if (bomb_move()) {
                    g_state = 1;
                    gunner_paint(g_x, g_state);
                }
                b_step = b_steps;
            }

            if (update) {
                alien_paint(a_state % 2);
                bomb_paint();
                update = 0;
            }

            missile_paint(g_x, m_x, m_y);       /* position cursor */
        }

        ch = read_char(TM_UNIT);
        switch (ch) {
        case key_to_int("<Alt-X>"):
        case key_to_int("<Esc>"):
        case 'q': case 'Q':
            return -1;

        case key_to_int("<Left-Arrow>"):
        case '<': case ',':
        case '-':
            if (g_x > 1) --g_x;
            break;

        case key_to_int("<Right-Arrow>"):
        case '>': case '.':
        case '+':
            if (g_x < cols - (GUNNER_WIDTH+1)) ++g_x;
            break;

        case key_to_int("<Return>"):
        case ' ':
            if (m_x == 0)
                m_x = g_x + GUNNER_MISSILE;
            break;

        case 'b':           /* boss mode :) */
        case 'B':
            set_window_flags(NULL, WF_HIDDEN);
            message("Boss: ");
            read_char();
            set_window_flags(NULL, 0, ~WF_HIDDEN);
            break;

        case 'p':           /* pause */
        case 'P':
            message("Game paused - any key to continue:");
            read_char();
            break;

        case key_to_int("<Alt-H>"):
            select_list("Invaders", "Press <Esc> to resume play.", 1, iv_help, SEL_NORMAL);
            break;
        }
    }
    return 0;
}


/*
 *  shelter_paint ---
 *      Paint the shelters
 */
static void
shelter_paint(void)
{
    extern int lines, cols;
    int basey, x, y;

    basey = lines - 1 - SHELTER_HEIGHT - GUNNER_HEIGHT;
    for (y = 0; y < SHELTER_HEIGHT; y++) {
        for (x = SHELTER_SPACING/2; x <= cols - SHELTER_SPACING; x += SHELTER_SPACING) {
            move_abs(basey + y, x);
            insert(iv_shelter[y]);
        }
    }
}


/*
 *  gunner_paint ---
 *      Paint the gunner
 */
static void
gunner_paint(int x, int state)
{
    extern int lines, cols;
    list piece;
    int basey, y;

    basey = lines - 1 - GUNNER_HEIGHT;
    goto_line(basey);
    for (y = 0; y < GUNNER_HEIGHT; ++y) {
        delete_line();
    }
    piece = iv_gunner[state];
    for (y = 0; y < GUNNER_HEIGHT; ++y) {
        move_abs(basey + y, x);
        insert(piece[y]);
    }
}


/*
 *  missile_mov ---
 *      Reposition the gunners missile and process any alien hits.
 */
static int
missile_mov(int mx, int my)
{
    extern int lines, cols;
    extern int a_rows, a_cols, a_top, a_left, a_lidx;
    extern int score, remaining, level;
    extern list aliens, bombs;
    string c;

    if (mx) {
        if (my == 0) {                          /* first time */
            my = lines - 2 - GUNNER_HEIGHT;

        } else if ((my -= 1) == 0) {
            return (0);
        }

        move_abs(my, mx);
        c = trim(read(1));

        if (strlen(c)) {
            if (atoi(c,0) == SHELTER_CHAR) {
                insert(" ");                    /* hit a shelter */
                delete_char();
                return (0);

            } else if (atoi(c,0) == BOMB_CHAR) {
                int i;                          /* hit a bomb */

                for (i = 0; i < length_of_list(bombs); i++) {
                    if (bombs[i][0] == mx && bombs[i][1] == my) {
                        delete_nth(bombs, i);
                        break;
                    }
                }
                insert(" ");
                delete_char();
                return (0);

            } else {
                int x, y;

                y = my - a_top;

                if (y >= 0 && y < (ALIEN_HEIGHT * a_rows) &&
                        (y % ALIEN_HEIGHT) < ALIEN_IHEIGHT) {
                    y /= ALIEN_HEIGHT;          /* index 0..a_cols-1 */

                    if (aliens[y][0] > 0) {
                        x = mx - abs(a_left);

                        if (x >= 0 && x < (ALIEN_WIDTH * a_cols) &&
                                (x % ALIEN_WIDTH) < (ALIEN_WIDTH - 1)) {
                            x /= ALIEN_WIDTH;   /* index 0..a_rows-1 */
                            x += a_lidx;

                            if (aliens[y][x] >= ALIEN_10) {
                                score += aliens[y][x];
                                --remaining;
                                aliens[y][x] = ALIEN_EX1;
                                message("Score: %d  Level: %d, Remaining: %d", score, level, remaining);
                                return 0;
                            }
                        }
                    }
                }
            }
        }
        return my;
    }
    return 0;
}


/*
 *  missile_mov ---
 *      Reposition the gunners missile.
 */
static void
missile_paint(int gx, int mx, int my)
{
    extern int lines, cols;

    if (mx) {
        move_abs(my, mx);
    } else {
        my = lines - 1 - GUNNER_HEIGHT;
        move_abs(my, gx + GUNNER_MISSILE);
    }
}


/*
 *  alien_init ---
 *      Initialise aliens
 */
static void
alien_init(void)
{
    extern int a_rows, a_cols;
    extern list aliens;
    list al;
    int x, y;

    /* 1st row */
    y = 0;
    al[0] = a_cols;                             /* count */
    for (x = 1; x <= a_cols; x++)
        al[x] = ALIEN_30;
    aliens[y] = make_list(al);

    /* 1/2 of remaining  */
    while (++y < (a_rows / 2)) {
        al[0] = a_cols;
        for (x = 1; x <= a_cols; x++) {
            al[x] = ALIEN_20;
        }
        aliens[y] = make_list(al);
    }

    /* remaining */
    do {
        al[0] = a_cols;
        for (x = 1; x <= a_cols; x++) {
            al[x] = ALIEN_10;
        }
        aliens[y] = make_list(al);
    } while (++y < a_rows);
}


/*
 *  alien_mov ---
 *      Move the current alien position by one-step.
 */
static void
alien_mov(void)
{
    extern int lines, cols;
    extern int a_rows, a_cols, a_top, a_left, a_lidx;
    extern list aliens;
    int max, top;

    top = a_top;
    max = cols - (ALIEN_WIDTH * ((a_cols+1) - a_lidx));

    /* determine status */
    if (a_left >= max) {
        a_left = max * -1;
        top++;

    } else if (a_left == -1) {
        a_left = 1;
        top++;

    } else {
        a_left += 1;
    }

    /* top change, must delete top line */
    if (top != a_top) {
        goto_line(a_top);
        delete_line();
        insert("\n");
        a_top = top;
    }
}


/*
 *  alien_paint ---
 *      Paint the aliens.
 */
static void
alien_paint(int state)
{
    extern int a_rows, a_cols, a_top, a_left, a_lidx;
    extern list aliens;
    list al, piece;
    int scrnx, scrny, removed;
    int x, y, i;

    for (y = a_rows-1; y >= 0; --y) {
        if (aliens[y][0] > 0)
            break;                              /* first non-blank line */
    }

    for (; y >= 0; y--) {
        scrny = a_top + (y * ALIEN_HEIGHT);

        if (a_left == 0) {
            a_left = 1;                         /* day-one */

        } else {
            if (y == a_rows-1) {                /* others */
                i = ALIEN_IHEIGHT - 1;
            } else {
                i = ALIEN_HEIGHT - 1;
            }

            for (; i >= 0; i--) {
                goto_line(scrny + i);
                delete_line();
                insert("\n");
            }
        }

        al = aliens[y];

        if (al[0] > 0) {
            scrnx = abs(a_left);                /* sign notes direction */
            for (x = a_lidx; x <= a_cols; ++x) {
                if (al[x] == ALIEN_30) {
                    piece = iv_alien30[ state ];

                } else if (al[x] == ALIEN_20) {
                    piece = iv_alien20[ state ];

                } else if (al[x] == ALIEN_10) {
                    piece = iv_alien10[ state ];

                } else if (al[x] == ALIEN_EX1) {
                    aliens[y][x] = ALIEN_EX2;
                    piece = iv_alienexp[ 0 ];

                } else if (al[x] == ALIEN_EX2) {
                    aliens[y][x] = ALIEN_EX3;
                    piece = iv_alienexp[ 1 ];

                } else if (al[x] == ALIEN_EX3) {
                    aliens[y][0] = aliens[y][0] - 1;
                    aliens[y][x] = ALIEN_NONE;
                    piece = iv_aliennone;
                    if (x == a_lidx || x == a_cols)
                        removed++;

                } else {
                    piece = iv_aliennone;
                }

                for (i = 0; i < ALIEN_IHEIGHT; i++) {
                    move_abs(scrny + i, scrnx);
                    insert(piece[i]);
                }

                scrnx += ALIEN_WIDTH;
            }
        }
    }

    if (removed) {
        /*
         *  Left margin
         */
        x = a_lidx;

        for (y = a_rows-1; y >= 0; --y) {
            if (aliens[y][x] != ALIEN_NONE)
                break;                          /* first non-blank line */
        }
        if (y == -1) {
            a_left += ALIEN_WIDTH;
            a_lidx++;
        }

        /*
         *  Right margin
         */
        for (y = a_rows-1; y >= 0; --y) {
            if (aliens[y][a_cols] != ALIEN_NONE)
                break;                          /* first non-blank line */
        }
        if (y == -1) {
            a_cols--;
        }
    }
}


/*
 *  bomb_drop ---
 *      xxx
 */
static int
bomb_drop(void)
{
    extern int a_rows, a_cols, a_top, a_left, a_lidx;
    extern list aliens;
    extern list bombs;                          /* bomb details */
    int x, y;

    for (y = a_rows-1; y >= 0; --y) {
        if (aliens[y][0] > 0)
            break;                              /* first non-blank line */
    }

    if (y >= 0) {
        int scrnx, scrny;
        list al = aliens[y];

        scrny = a_top + (y * ALIEN_HEIGHT) + ALIEN_HEIGHT - 1;
        scrnx = abs(a_left) + (ALIEN_WIDTH/2);

        for (x = a_lidx; x <= a_cols; x++) {
            if (al[x] >= ALIEN_10 && 0 == rand(80))
            {                                   /* new bomb */
                list bomb = make_list( scrnx, scrny, 0 );

                splice(bombs, length_of_list(bombs), 0, bomb);
                return 2;
            }
            scrnx += ALIEN_WIDTH;
        }
    }
    return 0;
}


static void
bomb_paint(void)
{
    extern list bombs;                          /* bomb details */
    int i;

    for (i = 0; i < length_of_list(bombs); i++)  {
        move_abs(bombs[i][1], bombs[i][0]);
        self_insert(BOMB_CHAR);
    }
}


static int
bomb_move(void)
{
    extern int lines, cols;
    extern list bombs;                          /* bomb details */
    int gunner, i;

    gunner = lines - 1 - GUNNER_HEIGHT;

    for (i = 0; i < length_of_list(bombs); ++i) {
        int col, line, state;
        string c;

        col = bombs[i][0];
        line = bombs[i][1];
        state = bombs[i][2];

        if (state++ > 0) {
            move_abs(line, col);
            self_insert(' ');
            if (++line >= lines) {              /* ground */
                delete_nth(bombs, i);
                continue;
            }
        }

        move_abs(line, col);
        c = trim(read(1));

        if (strlen(c)) {
            if (atoi(c,0) == SHELTER_CHAR) {
                self_insert(' ');               /* shelter */
                delete_nth(bombs, i);
                continue;

            } else if (atoi(c,0) != BOMB_CHAR && line >= gunner) {
                return 1;                       /* hit */
            }
        }

        self_insert(BOMB_CHAR);

        bombs[i][1] = line;
        bombs[i][2] = state;
    }
    return 0;
}

