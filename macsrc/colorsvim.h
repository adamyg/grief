/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: colorsvim.h,v 1.1 2024/07/05 18:41:55 cvsuser Exp $
 * Enhanced colour/colorscheme support.
 *
 *
 */

#define SCHEME_CTERMONLY    0x0001
#define SCHEME_GUIONLY      0x0002

void vim_colorscript(string scheme, string base, int flags, string file);
int vim_colorscheme(string label, int colors, ~string base, list spec, int asgui);

/*eof*/
