#ifndef COLORS_RGBMAP_H_INCLUDED
#define COLORS_RGBMAP_H_INCLUDED
/* $Id: rgbmap.h,v 1.3 2024/06/30 17:21:20 cvsuser Exp $
 * RGB color map utilises.
 *
 */

extern string           RGBPaletteIndex(int r, int g, int b, int colordepth);
extern string           RGBMap(string rgb, int colordepth);
extern string           RGBMap256(string rgb);

#endif  //COLORS_RGBMAP_H_INCLUDED

