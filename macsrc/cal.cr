/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: cal.cr,v 1.15 2014/10/27 23:28:18 ayoung Exp $
 * Calendar
 *
 * Format:
 *
 *          Jun 1993               Jul 1993              Aug 1993
 *     S  M Tu  W Th  F  S   S  M Tu  W Th  F  S   S  M Tu  W Th  F  S
 *           1  2  3  4  5               1  *  3   1  2  3  4  5  6  7
 *     6  7  8  9 10 11 12   4  5  6  7  8  9 10   8  9 10 11 12 13 14
 *    13 14 15 16 17 18 19  11 12 13 14 15 16 17  15 16 17 18 19 20 21
 *    20 21 22 23 24 25 26  18 19 20 21 22 23 24  22 23 24 25 26 27 28
 *    27 28 29 30           25 26 27 28 29 30 31  29 30 31
 *
 */

#include "grief.h"

#define SYNTAX          "cal_coloriser"

/* day references 0..6 */
#define SATURDAY        6                       /* 1 Jan 1 was a Saturday */
#define THURSDAY        4                       /* 14th Sept 1752 */

/* change over details */
#define FIRST_MISSING_DAY 639798                /* 3 Sep 1752 */
#define NUMBER_MISSING_DAYS 11                  /* 11 day correction */

/* number of centuries since 1700, not inclusive */
#define centuries_since_1700(yr) \
                ((yr) > 1700 ? (yr) / 100 - 17 : 0)

/* number of centuries since 1700 whose modulo of 400 is 0 */
#define quad_centuries_since_1700(yr) \
               ((yr) > 1600 ? ((yr) - 1600) / 400 : 0)

/* number of leap years between year 1 and this year, not inclusive */
#define leap_years_since_year_1(yr) \
               ((yr) / 4 - centuries_since_1700(yr) + quad_centuries_since_1700(yr))

static void             draw(int op);

static string           title = " S  M Tu  W Th  F  S";

static list             months = {
            "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
            };


/*  Function:           main
 *      Runtime initialisation, creates the cal coloriser.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 */
void
main()
{
    create_syntax(SYNTAX);
    syntax_rule("\\[.*\\]",         "operator");
    syntax_rule("[0-9]+",           "number");
    syntax_build(__COMPILETIME__);
}


/*  Function:           leap_year
 *    Determine whether the specified year is a leap-year.
 *
 *  Parameters:
 *      year -              Year.
 *
 *  Returns:
 *      Returns nonzero if yearnum is a leap year
 *
 *  Notes:
 *      leap years are not always every 4 years. There are exceptions. Years that end in
 *      00 are not leap years unless it is a multiple of 400. Therefore 1700, 1800, 1900,
 *      and 2100 are not leap years, but 2000 is.
 */
static int
leap_year(int year)
{
    if (year <= 1752) {
        return !(year % 4);
    }
    return (!(year % 4) && (year % 100)) || (! (year % 400));
}


/*  Function:           nth_day
 *      Calculate nth day of year for given date
 *
 *  Parameters:
 *      day -               Day of the month.
 *      month -             Month of the year.
 *      year -              Year.
 *
 *  Returns:
 *      day
 */
static int
nth_day(int day, int month, int year)
{
    day = 31 * (month - 1) + day;
    if (month > 2) {
        day = day - (month * 4 + 23) / 10;
        if (leap_year (year)) {
            ++day;
        }
    }
    return day;
}


/*  Function:           day_of_week
 *      calculate day of week for given date
 *
 *  Parameters:
 *      day -               Day of the month.
 *      month -             Month of the year.
 *      year -              Year.
 *
 *  Returns:
 *      nothing
 */
static int
day_in_week(int day, int month, int year, ~int)
{
    int temp;

    temp = (year - 1) * 365 + leap_years_since_year_1(year - 1)
                + nth_day(day, month, year);

    put_parm(3, temp);
    if (temp < FIRST_MISSING_DAY) {
        return ((temp - 1 + SATURDAY) % 7);
    }
    if (temp >= FIRST_MISSING_DAY + NUMBER_MISSING_DAYS) {
        return (((temp - 1 + SATURDAY) - NUMBER_MISSING_DAYS) % 7);
    }
    return (THURSDAY);
}


/*  Function:           print_month
 *      output given month to buffer.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 */
static int
print_month(
    int offset, int indent, int month, int year, int day, int highlight)
{
    string month_name;
    int first, magic, max, i;

    /* get days in month */
    first = day_in_week(1, month, year, magic);
    if (month >= 12) {
        max = 31;
    } else {
        max = nth_day(1, month+1, year) - nth_day(1, month, year);
    }
    ++indent;
    top_of_buffer();

    /* output month/year line */
    month_name = months[month - 1];
    move_abs((offset*9) + 1, indent + (strlen(title) - strlen(month_name) - 5) / 2);
    insertf("%s %d", month_name, year);
    down();

    /* output days line */
    move_abs(0, indent);
    insert(title);
    down();

    /* output day numbers in 7 columns */
    move_abs(0, first * 3 + indent);
    for (i = 1; i <= max; ++i) {
        if (first >= 7) {                       /* end of line */
            if (!down()) {
                end_of_line(); insert("\n");
            }
            move_abs(0, indent);
            first = 0;
        }
        if ((day == i) && highlight) {
            left();                             /* current day */
            insertf("[%2d]", i);
        } else {
            insertf("%2d ", i);                 /* others */
        }
        ++first;

        if (magic++ == FIRST_MISSING_DAY) {     /* handle 1752, Sept 3 - 13 */
            i += NUMBER_MISSING_DAYS;
        }
    }
}


/*  Function:           monthcvt
 *      Convert month number or localized name string into integer
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 */
static int
monthcvt(string month_name)
{
    int m;

    month_name = lower(month_name);
    for (m = 0; m < 12; ++m) {                  /* compare against names */
        if (month_name == lower(months[m])) {
            return m + 1;
        }
    }
    return atoi(month_name);                    /* presume it's an integer */
}


/*  Function:           cal
 *      Output multiple month calendar
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 */
void
cal()                   /* ([month], [year]) */
{
    string month_name;
    int cal_month, cal_year, cal_lines;
    int buf, win;
    int op = 0;

    /* ask user for month / year */
    if (get_parm(1, cal_year) > 0) {            /* month year */
        get_parm(0, month_name);
        cal_month = monthcvt( month_name );
    } else if (get_parm(0, cal_year) > 0) {     /* year */
        cal_month = 1;
    } else {
        op = 2;                                 /* home */
    }

    /* build buffer */
    save_position();
    inq_screen_size(cal_lines);
    cal_lines = (cal_lines - 4) / 9;
    if (cal_lines > 4) {
        cal_lines = 4;
    }
    if ((buf = create_buffer("calendar", NULL, 1)) >= 0) {
        set_buffer(buf);
        attach_syntax(SYNTAX);
        draw(op);
        win = sized_window(cal_lines * 9, 70, "<Esc> exit, <Up/Down/Home> change");
        select_buffer(buf, win, NULL, inq_module() + "::cal_keys");
        delete_buffer(buf);
    }
    restore_position(2);
}


/*  Function:           calx
 *      Output current calendar, using system 'cal' service.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 */
void
calx(string month, string year)
{
    int buf, win;
    string cmd;

    save_position();
    sprintf(cmd, "cal %s %s", month, year);
    buf = perform_command(cmd, "Calendar");
    win = sized_window(inq_lines(buf),
                inq_line_length(buf), "Type <Esc> to exit");
    select_buffer(buf, win);
    delete_buffer(buf);
    restore_position(2);
}


static void
cal_keys()
{
    assign_to_key("<Up>",     "::draw -1");
    assign_to_key("<left>",   "::draw -1");
    assign_to_key("<Down>",   "::draw 1");
    assign_to_key("<Right>",  "::draw 1");
    assign_to_key("<Home>",   "::draw 2");
}


static void
draw(int op)
{
    extern int cal_month, cal_year, cal_lines;
    int this_day, this_month, this_year;
    int line, col;

    localtime(time(), this_year, this_month, this_day);

    if (op == -1) {                             /* back */
        for (line = cal_lines*6; line > 0; line--)
            if (--cal_month == 0) {
                cal_month = 12;
                if (--cal_year <= 0) {
                    cal_year = 1, cal_month = 1;
                }
            }

    } else if (op == 2) {                       /* home */
        cal_year = this_year; cal_month = this_month;
        if (cal_year > 1 && --cal_month == 0) {
            cal_month = 12, --cal_year;         /* .. center on first line */
        }

    } else {  /* 0 or 1 */
        if (cal_year >= 9999) {
            cal_year = 9999;
        } else if (cal_year < 1) {
            cal_year = 1, cal_month = 1;
        } else if (cal_month < 1) {
            cal_month = 1;
        } else if (cal_month > 12) {
            cal_month = 12;
        }
    }

    clear_buffer();
    raise_anchor();

    for (line = 0; line < cal_lines; line++)
        for (col = 0; col < 3; col++) {
            print_month(line, (col * 23) + 1, cal_month, cal_year,
                this_day, ((cal_month == this_month) && (cal_year == this_year)));
            if (++cal_month == 13) {
                cal_month = 1;
                ++cal_year;
            }
        }
}

/*eof*/
