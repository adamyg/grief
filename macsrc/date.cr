/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: date.cr,v 1.10 2014/10/27 23:28:19 ayoung Exp $
 * Date/time insertion.
 *
 *
 */

#include "grief.h"


void
main()
{
    assign_to_key("<Ctrl-D>", "insert_date 1");
}


void
insert_date()
{
    string day, month, buf;
    int month_num, day_num, year;
    int hr, min;
    int type;

    date(year, month_num, day_num, month, day);
    if (!get_parm(0, type)) {
        type = 0;
    }

    switch (type) {
    case 4:         /* MM/DD/YY */
        sprintf(buf, "%02d/%02d/%02d", month_num, day_num, year % 100);
        break;

    case 3:         /* YY/DD/MM */
        sprintf(buf, "%02d/%02d/%02d", year % 100, day_num, month_num);
        break;

    case 2:         /* YY/MM/DD */
        sprintf(buf, "%02d/%02d/%02d", year % 100, month_num, day_num);
        break;

    case 1:         /* DD/MM/YY */
        sprintf(buf, "%02d/%02d/%02d", day_num, month_num, year % 100);
        break;

    default:        /* default, long form */
        time(hr, min, NULL);
        sprintf(buf, "%s %s %02d %02d:%02d %04d", substr(day, 1, 3),
        substr(month, 1, 3), day_num, hr, min, year);
    }
    insert(buf);
}

/*eof*/
