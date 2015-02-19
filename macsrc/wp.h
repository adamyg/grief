#ifndef MACSRC_WP_H_INCLUDED
#define MACSRC_WP_H_INCLUDED
/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: wp.h,v 1.2 2014/10/27 23:28:30 ayoung Exp $
 * Word processing features/options.
 *
 */

/* Style. */
#define JUSTIFIED       0
#define RAGGED          1

/* Interface. */
#if defined(__PROTOTYPES__)
extern void             wp_options(void);
extern int              autowrap(~string yesno);
extern int              autoindent(~string yesno);
extern void             margin(~string);

extern void             format_paragraph(void);
extern void             format_block(void);
extern int              format_region(int start_line, int end_line);
extern void             center(~int);
#endif //__PROTOTYPES__

#endif //MACSRC_WP_H_INCLUDED

