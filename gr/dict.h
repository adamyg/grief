#ifndef GR_DICT_H_INCLUDED
#define GR_DICT_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_dict_h,"$Id: dict.h,v 1.9 2014/10/22 02:32:56 ayoung Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: dict.h,v 1.9 2014/10/22 02:32:56 ayoung Exp $
 * Dictionaries.
 *
 *
 * This file is part of the GRIEF Editor.
 *
 * The GRIEF Editor is free software: you can redistribute it
 * and/or modify it under the terms of the GRIEF Editor License.
 *
 * The GRIEF Editor is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * License for more details.
 * ==end==
 */

#include <edsym.h>

__CBEGIN_DECLS

enum {  /*dict_each() operand*/
    DICT_EACH_PAIR,
    DICT_EACH_KEY,
    DICT_EACH_VALUE
};

extern void                 do_create_dictionary(void);
extern void                 do_delete_dictionary(void);
extern void                 do_dict_clear(void);
extern void                 do_dict_delete(void);
extern void                 do_dict_each(int what);
extern void                 do_dict_exists(void);
extern void                 do_dict_list(void);
extern void                 do_dict_name(void);
extern void                 do_get_property(void);
extern void                 do_list_of_dictionaries(void);
extern void                 do_set_property(void);

__CEND_DECLS

#endif /*GR_DICT_H_INCLUDED*/
