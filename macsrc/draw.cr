/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: draw.cr,v 1.9 2014/10/27 23:28:21 ayoung Exp $
 * Box draw
 *
 *
 *
 */

#include "grief.h"

#if defined(MSDOS)
#define SVL         179             /* Single Vertical Line             */
#define SHL         196             /* Single Horizontal Line           */
#define ULSC        218             /* Upper Left Single Corner         */
#define URSC        191             /* Upper Right Single Corner        */
#define LLSC        192             /* Lower Left Single Corner         */
#define LRSC        217             /* Lower Right Single Corner        */
#define LST         195             /* Left Single Tee                  */
#define RST         180             /* Right Single Tee                 */
#define UST         194             /* Up Single Tee                    */
#define DST         193             /* Down Single Tee                  */
#define SX          197             /* Single Cross                     */

#define DVL         186             /* Double Vertical Line             */
#define DHL         205             /* Double Horizontal Line           */
#define ULDC        201             /* Upper Left Double Corner         */
#define URDC        187             /* Upper Right Double Corner        */
#define LLDC        200             /* Lower Left Double Corner         */
#define LRDC        188             /* Lower Right Double Corner        */
#define LDT         204             /* Left Double Tee                  */
#define RDT         185             /* Right Double Tee                 */
#define UDT         203             /* Up Double Tee                    */
#define DDT         202             /* Down Double Tee                  */
#define DX          206             /* Double Cross                     */

#define ULDSC       214             /* Upper Left Double Single Corner  */
#define URDSC       183             /* Upper Right Double Single Corner */
#define LLDSC       211             /* Lower Left Double Single Corner  */
#define LRDSC       189             /* Lower Right Double Single Corner */
#define LDST        199             /* Left Double Single Tee           */
#define RDST        182             /* Right Double Single Tee          */
#define UDST        210             /* Up Double Single Tee             */
#define DDST        208             /* Down Double Single Tee           */
#define DSX         215             /* Double Single Single Cross       */

#define ULSDC       213             /* Upper Left Single Double Corner  */
#define URSDC       184             /* Upper Right Single Double Corner */
#define LLSDC       212             /* Lower Left Single Double Corner  */
#define LRSDC       190             /* Lower Right Single Double Corner */
#define LSDT        198             /* Left Single Double Tee           */
#define RSDT        181             /* Right Single Double Tee          */
#define USDT        209             /* Up Single Double Tee             */
#define DSDT        207             /* Down Single Double Tee           */
#define SDX         216             /* Single Double Cross              */
#endif

#define BLANK       ' '             /* Blank character                  */
#define NONE        0               /* Neither single nor double        */
#define SINGLE      1               /* Single connection                */
#define DOUBLE      2               /* Double connection                */

#if defined(__PROTOTYPES__)
static int          chk_left(void);
static int          chk_right(void);
static int          chk_up(void);
static int          chk_down(void);
static void         draw_msg(void);
static void         draw_getenv(void);
#if defined(MSDOS)
static void         draw_ins(void);
#endif
static void         draw_del(void);
static void         draw_doit(string move_cmd, string ins_cmd);
static int          draw_up(void);
static int          draw_down(void);
static int          draw_left(void);
static int          draw_right(void);
#endif

static int          draw_mode;
static int          draw_style;
#if defined(MSDOS)
static int          draw_state1;
static int          draw_state2;
#endif
static int          _uc, _dc, _lc, _rc;


void
draw(void)
{
    int old_insert, old_tabs;

    draw_style = 0;
    get_parm(0, draw_style);

    keyboard_push();
    keyboard_typeables();
    assign_to_key("<Esc>",           "exit");
    assign_to_key("<Up>",            "::draw_doit up ::draw_up");
    assign_to_key("<Down>",          "::draw_doit down ::draw_down");
    assign_to_key("<Left>",          "::draw_doit left ::draw_left");
    assign_to_key("<Right>",         "::draw_doit right ::draw_right");
#if defined(MSDOS)
    assign_to_key("<Ins>",           "::draw_ins");
#endif
    assign_to_key("<Del>",           "::draw_del");
    assign_to_key("<F7>",            "remember");
    assign_to_key("<F8>",            "playback");
    assign_to_key("<Keypad-Star>",   "undo");

    old_tabs = use_tab_char(0);                 /* disable use of tabs */
    old_insert = insert_mode(0);                /* overtype mode */
    draw_mode = 0;
#if defined(MSDOS)
    draw_state1 = SINGLE;
    draw_state2 = DOUBLE;
#endif
    keyboard_flush();
    draw_msg();
    process();
    keyboard_pop();
    use_tab_char(old_tabs);                     /* restore tab setting */
    insert_mode(old_insert);                    /* restore insert setting */
    keyboard_flush();

    message("");
}


static int
chk_left(void)
{
   int l1, l2, c1, c2;

   inq_position(l1, c1); left(); inq_position(l2, c2);
   if (l1 == l2 && c1 == c2+1)
      return (1);
   move_abs(l1, c1);
   return (0);
}


static int
chk_right(void)
{
    int l1, l2, c1, c2;

    inq_position(l1, c1); right(); inq_position(l2, c2);
    if (l1 == l2 && c1+1 == c2)
       return (1);
    move_abs(l1, c1);
    return (0);
}


static int
chk_up(void)
{
    int l1, l2, c1, c2;

    inq_position(l1, c1); up(); inq_position(l2, c2);
    if (l1 == l2+1 && c1 == c2)
        return (1);
    move_abs(l1, c1);
    return (0);
}


static int
chk_down(void)
{
    int l1, l2, c1, c2;

    inq_position(l1, c1); down(); inq_position(l2, c2);
    if (l1+1 == l2 && c1 == c2)
        return (1);
    move_abs(l1, c1);
    return (0);
}


static void
draw_msg(void)
{
#if defined(MSDOS)
    message("<Up/Down> <Left/Right> <Ins> <Del> or <Esc> to quit" +
                 (draw_state1 == SINGLE ? "" : " (DOUBLE)"));
#else
    message("<Up/Down> <Left/Right> <Del> mode or <Esc> to quit");
#endif
}



/*
 * draw_getenv ---
 *    Determine the characters required to extend the line
 *    in all directions (up, down, left anf right) from the
 *    the current character.
 */
static void
draw_getenv(void)
{
    _uc = NONE, _dc = NONE, _lc = NONE, _rc = NONE;

#if defined(MSDOS)
    if (draw_style == 0) {
        string ch;

        if (chk_up()) {
            _uc = index("≥¥µ∏ø¬√≈∆—’ÿ⁄", ch = read (1)) ?
                    SINGLE : index("∂∑π∫ª«…ÀÃŒ“÷◊", ch) ? DOUBLE : NONE;
            down();
        }

        if (chk_down()) {
            _dc = index("≥¥µæ¿¡√≈∆œ‘ÿŸ", ch = read (1)) ?
                    SINGLE : index("∂π∫ºΩ«» ÃŒ–”◊", ch) ? DOUBLE : NONE;
            up();
        }

        if (chk_left()) {
            _lc = index("¿¡¬√ƒ≈«–“”÷◊⁄", ch = read (1)) ?
                    SINGLE : index("∆»… ÀÃÕŒœ—‘’ÿ", ch) ? DOUBLE : NONE;
            right();
        }

        if (chk_right()) {
            _rc = index("¥∂∑Ωø¡¬ƒ≈–“◊Ÿ", ch = read(1)) ?
                     SINGLE : index("µ∏πªºæ ÀÕŒœ—ÿ", ch) ? DOUBLE : NONE;
            left();
        }
        return;
    }
#endif

    if (chk_up()) {
        _uc = index("|-+", read(1)) ? SINGLE : NONE;
        down();
    }

    if (chk_down()) {
        _dc = index("|-+", read(1)) ? SINGLE : NONE;
        up();
    }

    if (chk_left()) {
        _lc = index("|-+", read(1)) ? SINGLE : NONE;
        right();
    }

    if (chk_right()) {
        _rc = index("|-+", read(1)) ? SINGLE : NONE;
        left();
    }
}


#if defined(MSDOS)
void
draw_ins(void)
{
    int new_mode;

    new_mode = draw_state2;
    draw_state2 = draw_state1;
    draw_state1 = new_mode;
    if (! draw_mode)
        draw_msg();
}
#endif


static void
draw_del(void)
{
    draw_mode = !draw_mode;
    if (draw_mode)
        message("Delete mode on");
    else draw_msg();
}


static void
draw_doit(string move_cmd, string ins_cmd)
{
    draw_getenv();
    if (draw_mode) {
        execute_macro(move_cmd);
        self_insert(BLANK);
        left();
    } else {
        self_insert(execute_macro(ins_cmd));
        execute_macro(move_cmd);
        left();
    }

    draw_getenv();
    message( "u=%d d=%d l=%d r=%d", _uc, _dc, _lc, _rc );
}


static int
draw_up(void)
{
#if defined(MSDOS)
    if (draw_style == 0) {
        /* crosses */
        if (draw_state1 == _dc && (draw_state1 == _lc && draw_state1 == _rc))
            return draw_state1 == SINGLE ? SX  : DX;
        if (draw_state1 == _dc && (draw_state2 == _lc && draw_state2 == _rc))
            return draw_state1 == SINGLE ? SDX : DSX;
        if (draw_state2 == _dc && (draw_state1 == _lc && draw_state1 == _rc))
            return draw_state1 == SINGLE ? DSX : SDX;

        /* tees */
        if (draw_state1 == _lc && draw_state1 == _rc)
            return draw_state1 == SINGLE ? DST  : DDT;
        if (draw_state1 == _lc && draw_state1 == _dc)
            return draw_state1 == SINGLE ? RST  : RDT;
        if (draw_state1 == _rc && draw_state1 == _dc)
            return draw_state1 == SINGLE ? LST  : LDT;
        if (draw_state2 == _lc && draw_state2 == _rc)
            return draw_state1 == SINGLE ? DSDT : DDST;
        if (draw_state2 == _lc && draw_state1 == _dc)
            return draw_state1 == SINGLE ? RSDT : RDST;
        if (draw_state2 == _rc && draw_state1 == _dc)
            return draw_state1 == SINGLE ? LSDT : LDST;

        /* corners */
        if (draw_state1 == _lc)
            return draw_state1 == SINGLE ? LRSC : LRDC;
        if (draw_state1 == _rc)
            return draw_state1 == SINGLE ? LLSC : LLDC;
        if (draw_state2 == _lc)
            return draw_state1 == SINGLE ? LRSDC : LRDSC;
        if (draw_state2 == _rc)
            return draw_state1 == SINGLE ? LLSDC : LLDSC;

        /* line */
        return draw_state1 == SINGLE ? SVL : DVL;
    }
#endif

    if (_rc || _lc)
        return '+';
    return '|';
}


static int
draw_down(void)
{
#if defined(MSDOS)
    if (draw_style == 0) {
        /* crosses */
        if (draw_state1 == _uc && (draw_state1 == _lc && draw_state1 == _rc))
            return draw_state1 == SINGLE ? SX : DX;
        if (draw_state1 == _uc && (draw_state2 == _lc && draw_state2 == _rc))
            return draw_state1 == SINGLE ? SDX : DSX;
        if (draw_state2 == _uc && (draw_state1 == _lc && draw_state1 == _rc))
            return draw_state1 == SINGLE ? DSX : SDX;

        /* tees */
        if (draw_state1 == _lc && draw_state1 == _rc)
            return draw_state1 == SINGLE ? UST : UDT;
        if (draw_state1 == _lc && draw_state1 == _uc)
            return draw_state1 == SINGLE ? RST : RDT;
        if (draw_state1 == _rc && draw_state1 == _uc)
            return draw_state1 == SINGLE ? LST : LDT;
        if (draw_state2 == _lc && draw_state2 == _rc)
            return draw_state1 == SINGLE ? USDT : UDST;
        if (draw_state2 == _lc && draw_state1 == _uc)
            return draw_state1 == SINGLE ? RSDT : RDST;
        if (draw_state2 == _rc && draw_state1 == _uc)
            return draw_state1 == SINGLE ? LSDT : LDST;

        /* corners */
        if (draw_state1 == _lc)
            return draw_state1 == SINGLE ? URSC : URDC;
        if (draw_state1 == _rc)
            return draw_state1 == SINGLE ? ULSC : ULDC;
        if (draw_state2 == _lc)
            return draw_state1 == SINGLE ? URSDC : URDSC;
        if (draw_state2 == _rc)
            return draw_state1 == SINGLE ? ULSDC : ULDSC;

        /* line */
        return draw_state1 == SINGLE ? SVL : DVL;
    }
#endif

    if (_rc || _lc)
        return '+';
    return '|';
}


static int
draw_left(void)
{
#if defined(MSDOS)
    if (draw_style == 0) {
        /* crosses  */
        if (draw_state1 == _rc && (draw_state1 == _uc && draw_state1 == _dc))
            return draw_state1 == SINGLE ? SX : DX;
        if (draw_state1 == _rc && (draw_state2 == _uc && draw_state2 == _dc))
            return draw_state1 == SINGLE ? DSX : SDX;
        if (draw_state2 == _rc && (draw_state1 == _uc && draw_state1 == _dc))
            return draw_state1 == SINGLE ? SDX : DSX;

        /* tees */
        if (draw_state1 == _uc && draw_state1 == _dc)
            return draw_state1 == SINGLE ? RST : RDT;
        if (draw_state1 == _uc && draw_state1 == _rc)
            return draw_state1 == SINGLE ? DST : DDT;
        if (draw_state1 == _dc && draw_state1 == _rc)
            return draw_state1 == SINGLE ? UST : UDT;
        if (draw_state2 == _uc && draw_state2 == _dc)
            return draw_state1 == SINGLE ? RDST : RSDT;
        if (draw_state2 == _uc && draw_state1 == _rc)
            return draw_state1 == SINGLE ? DDST : DSDT;
        if (draw_state2 == _dc && draw_state1 == _rc)
            return draw_state1 == SINGLE ? UDST : USDT;

        /* corners */
        if (draw_state1 == _uc)
            return draw_state1 == SINGLE ? LRSC : LRDC;
        if (draw_state1 == _dc)
            return draw_state1 == SINGLE ? URSC : URDC;
        if (draw_state2 == _uc)
            return draw_state1 == SINGLE ? LRDSC : LRSDC;
        if (draw_state2 == _dc)
            return draw_state1 == SINGLE ? URDSC : URSDC;

        /* line */
        return draw_state1 == SINGLE ? SHL : DHL;
    }
#endif

   if (_uc || _dc)
      return '+';
   return '-';
}


static int
draw_right(void)
{
#if defined(MSDOS)
    if (draw_style == 0) {
        /* crosses */
        if (draw_state1 == _lc && (draw_state1 == _uc && draw_state1 == _dc))
            return draw_state1 == SINGLE ? SX : DX;
        if (draw_state1 == _lc && (draw_state2 == _uc && draw_state2 == _dc))
            return draw_state1 == SINGLE ? DSX : SDX;
        if (draw_state2 == _lc && (draw_state1 == _uc && draw_state1 == _dc))
            return draw_state1 == SINGLE ? SDX : DSX;

        /* tees */
        if (draw_state1 == _uc && draw_state1 == _dc)
            return draw_state1 == SINGLE ? LST : LDT;
        if (draw_state1 == _uc && draw_state1 == _lc)
            return draw_state1 == SINGLE ? DST : DDT;
        if (draw_state1 == _dc && draw_state1 == _lc)
            return draw_state1 == SINGLE ? UST : UDT;
        if (draw_state2 == _uc && draw_state2 == _dc)
            return draw_state1 == SINGLE ? LDST : LSDT;
        if (draw_state2 == _uc && draw_state1 == _lc)
            return draw_state1 == SINGLE ? DDST : DSDT;
        if (draw_state2 == _dc && draw_state1 == _lc)
            return draw_state1 == SINGLE ? UDST : USDT;

        /* corners */
        if (draw_state1 == _uc)
            return draw_state1 == SINGLE ? LLSC : LLDC;
        if (draw_state1 == _dc)
            return draw_state1 == SINGLE ? ULSC : ULDC;
        if (draw_state2 == _uc)
            return draw_state1 == SINGLE ? LLDSC : LLSDC;
        if (draw_state2 == _dc)
            return draw_state1 == SINGLE ? ULDSC : ULSDC;

        /* line */
        return draw_state1 == SINGLE ? SHL : DHL;
    }
#endif

    if (_uc || _dc)
        return '+';
    return '-';
}

/*eof*/
