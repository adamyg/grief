#include <edidentifier.h>
__CIDENT_RCSID(gr_crdebug_c,"$Id: crdebug.c,v 1.17 2022/05/31 16:18:22 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: crdebug.c,v 1.17 2022/05/31 16:18:22 cvsuser Exp $
 * Debug/diagnostics support.
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

#define GRUNCH_NODEENUM_MAP

#include "grunch.h"                             /* local definitions */

static int              listprint(const Head_p hd, int size, FILE *out, int depth);

static int              nt1(const node_t *np, int, FILE *out, int depth);
static int              ntp(const node_t *np, int, FILE *out, int depth);

static const char *     map_word(const struct crmap *tbl, int, char *buf /*[32]*/);


static const char *
map_word(const struct crmap *tbl, int val, char *buf)
{
    const struct crmap *mp;

    for (mp = tbl; mp->name; ++mp) {
        if (mp->val == val) {
            return mp->name;
        }
    }
    sprintf(buf, "<%x>", val);
    return buf;
}


int
node_print(const node_t *np, int size)
{
    if (np) {
        D_NODE_MAGIC(assert(np->magic == NODE_MAGIC);)
        size = nt1(np, size, stdout, 0);
        if (xf_flush) {
            fflush(stdout);
        }
    }
    return size;
}


int
node_dprint(const node_t *np, int size)
{
    if (np && xf_debug) {
        D_NODE_MAGIC(assert(np->magic == NODE_MAGIC);)
        size = nt1(np, (size <= 0 ? 0 : size), x_errfp, (xf_debug >= 2 ? 0 : -1));
        if (xf_flush) {
            fflush(x_errfp);
        }
    }
    return size;
}


int
list_print(const Head_p hd, int __CUNUSEDARGUMENT(size))
{
    return listprint(hd, 0, stdout, 0);
}


int
list_dprint(const Head_p hd, int size)
{
    if (xf_debug) {
        size = listprint(hd, (size <= 0 ? 0 : size), x_errfp, 0);
    }
    return size;
}


static int
listprint(const Head_p hd, int size, FILE *out, int depth)
{
    if (hd && xf_debug) {
        register List_p lp;
        const node_t *np;

        if (NULL != (lp = ll_first(hd))) {
            ++depth;
            do {
                if (NULL != (np = (node_t *)ll_elem(lp))) {
                    D_NODE_MAGIC(assert(np->magic == NODE_MAGIC);)
                    size = nt1(np, size, out, depth);
                } else {
                    nt1(np, size, out, depth);
                }
            } while(NULL != (lp = ll_next(lp)));
            if (depth > 0) {
                fprintf(out, "\n");
            }
        }
        if (xf_flush) {
            fflush(out);
        }
    }
    return size;
}


static int
nt1(const node_t *np, int size, FILE *out, int depth)
{
    if (NULL == np) {
        if (depth >= 0) {
            fprintf(out, "\n%*sX():", (++depth) * IDENT_LEVEL, "");
        }
        fprintf(out, " <void>");

    } else {
        const node_t *left = np->left, *right = np->right;
        const int is_function =
                (left && node_keywd == left->type && TO_FUNC == left->atom.ival ? 1 : 0);

        if (depth >= 0) {
            fprintf(out, "\n%*sX(%s):", (++depth) * IDENT_LEVEL, "",
                (left && right ? "LR" : (left ? "L" : (right ? "R" : "" ))));
        }

        if (is_function) {
            size = ntp(np, size, out, depth);

            if (depth < 0)
                fprintf(out, "(");

            if (left && left->left)             /* types */
                size = nt1(left->left, size, out, depth);

            if (right)                          /* symbol */
                size = nt1(right, size, out, depth);

            if (depth < 0)
                fprintf(out, ")");

            if (left && left->right)            /* arguments */
                size = nt1(left->right, size, out, depth);

        } else {
            /*
             *  Specials/
             *      in a number of cases the branches aren't nodes but lists.
             */
            if (node_keywd == np->type) {
                switch (np->atom.ival) {
                case K_CONSTRUCTORS:
                    ntp(np, size, out, depth);
                    ntp(right, size, out, depth);
                    listprint((Head_p) left, size, out, depth);
                    return size;

                case K_TRY:
                    ntp(np, size, out, depth);
                    if (left) {
                        listprint((Head_p) left, size, out, depth);
                    }
                    ntp(right, size, out, depth);
                    return size;

                case K_BLOCK:
                case K_LEXICALBLOCK:
                    ntp(np, size, out, depth);
                    if (right) {
                        listprint((Head_p) right, size, out, depth);
                    } else {
                        nt1(right, size, out, depth);
                    }
                    return size;

                case K_IF:
                case K_ELSE:
                case K_WHILE:
                case K_COND:
                case K_FOR:
                case K_FOREACH:
                    ntp(np, size, out, depth);
                    return size;

                case K_CASE:
                    ntp(np, size, out, depth);
                    ntp(left, size, out, depth);
                    if (right) {
                        listprint((Head_p) right, size, out, depth);
                    } else {
                        nt1(right, size, out, depth);
                    }
                    return size;
                }
            }

            /*
             *  Generic
             */
            size = ntp(np, size, out, depth);
            if (left) {
                size = nt1(left, size, out, depth);
            }
            if (right) {
                size = nt1(right, size, out, depth);
            }
        }
    }
    return size;
}


static int
ntp(const node_t *np, int size, FILE *out, int depth)
{
    char buf[32];

    if (NULL == np) {
        return size;
    }
    D_NODE_MAGIC(assert(np->magic == NODE_MAGIC);)

    switch (np->type) {
    case node_integer:
        if (! xf_struct) {
            fprintf(out, " %" ACCINT_FMT, np->atom.ival);
        }
        return size;

    case node_float:
        if (! xf_struct) {
            fprintf(out, " %f", np->atom.fval);
        }
        return size;

    case node_string:
        if (! xf_struct) {
            fprintf(out, " \"%s\"", np->atom.sval);
        }
        return size;

    case node_symbol:
        if (! xf_struct) {
            fprintf(out, " %s", np->atom.sval);
        }
        return size;

    case node_type:
        if (! xf_struct) {
            fprintf(out, " %s", symtype_to_defn((symtype_t)np->atom.ival));
        }
        return size;

    case node_arglist:
        if (! xf_struct) {
            Head_p arglist = np->atom.arglist;

            if (depth < 0) {
                fprintf(out, "(");
            } else {
                fprintf(out, "<arglist %p>", arglist);
            }

            if (arglist) {
                List_p lp;

                assert(ll_hcheck(arglist));
                if (NULL != (lp = ll_first(arglist))) {
                    while (1) {
                        node_t *t_np = ll_elem(lp);

                        if (t_np) {
                            D_NODE_MAGIC(assert(t_np->magic == NODE_MAGIC);)
                            size = nt1(t_np, size, out, (depth < 0 ? -1 : depth+1));
                        } else {
                            fprintf(out, " ...");
                        }

                        if (NULL == (lp = ll_next(lp))) {
                            break;
                        }
                        if (depth < 0) {
                            fprintf(out, ",");
                        }
                    }
                }
            }
            if (depth < 0) {
                fprintf(out, ")");
            }
        }
        return 4;

    default:
        assert(node_keywd == np->type);

        switch (np->atom.ival) {
        case TO_ARRAY:
            if (! np->right) {
                if (! xf_struct) {
                    fprintf(out, "[]");
                }
            } else if (np->right->type == node_integer) {
                if (! xf_struct) {
                    fprintf(out, "[%" ACCINT_FMT "]", np->right->atom.ival);
                }
                return size * np->right->atom.ival;
            }
            return size;

        case TO_PTR:
            if (! xf_struct) {
                fprintf(out, "*");
            }
            return 4;

        case TO_REF:
            if (! xf_struct) {
                fprintf(out, "&");
            }
            return 4;

        case TO_FUNC:
            if (! xf_struct) {
                if (depth >= 0) {
                    fprintf(out, " function");
                }
            }
            return 4;
        }

        if (! xf_struct) {
            if (K_NOOP != np->atom.ival) {
                fprintf(out, " %s", map_word(crenum_keywdtbl, np->atom.ival, buf));
            }
        }
    }
    return size;
}

/*end*/
