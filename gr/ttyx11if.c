#include <edidentifier.h>
__CIDENT_RCSID(gr_ttyx11if_c,"$Id: ttyx11if.c,v 1.5 2014/10/22 02:33:24 ayoung Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: ttyx11if.c,v 1.5 2014/10/22 02:33:24 ayoung Exp $
 * X11 dynamic interface.
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

#include <editor.h>

#if defined(HAVE_LIBX11) && defined(HAVE_X11_XLIB_H)

#include "ttyx11if.h"                           /* Xlib dynamic binding */

#if !defined(X11_DYNAMIC)
void
XLibInitialise(void)
{
}

#else /*X11_DYNAMIC*/
#if defined(HAVE_DLFCN_H)
#include <dlfcn.h>
#endif
#include <errno.h>

#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xresource.h>
#include <X11/keysym.h>

typedef void (* XFunction)();

static XClassHint *   (*xifAllocClassHint)(void);
static Status         (*xifAllocColor)(Display*, Colormap, XColor*);
static Status         (*xifAllocNamedColor)(Display*, Colormap, _Xconst char*, XColor*, XColor*);
static XSizeHints *   (*xifAllocSizeHints)(void);
static XWMHints *     (*xifAllocWMHints)(void);
static int            (*xifBell)(Display *display, int percent);
static int            (*xifChangeGC)(Display*, GC, unsigned long, XGCValues*);
static int            (*xifChangeProperty)(Display *, Window, Atom, Atom, int, int, _Xconst unsigned char*, int);
static int            (*xifChangeWindowAttributes)(Display *, Window, unsigned long, XSetWindowAttributes*);
static int            (*xifClearWindow)(Display*, Window);
static Window         (*xifCreateWindow)(Display*, Window, int, int, unsigned int, unsigned int, unsigned int,
                                int, unsigned int, Visual *, unsigned long, XSetWindowAttributes*);
static int            (*xifConvertSelection)(Display*, Atom, Atom, Atom, Window requestor, Time time);
static Colormap       (*xifCreateColormap)(Display*, Window, Visual*, int);
static GC             (*xifCreateGC)(Display*, Drawable, unsigned long, XGCValues *);
static Region         (*xifCreateRegion)(void);
static int            (*xifDestroyRegion)(Region r);
static Window         (*xifCreateSimpleWindow)(Display *, Window, int, int,
                                unsigned int, unsigned int, unsigned int, unsigned long, unsigned long);
static int            (*xifDrawImageString16)(Display*, Drawable, GC, int, int, _Xconst XChar2b *, int);
static int            (*xifDrawLine)(Display*, Drawable, GC, int, int, int, int);
static int            (*xifDrawString16)(Display*, Drawable, GC, int, int, _Xconst XChar2b*, int);
static int            (*xifEventsQueued)(Display*, int);
static int            (*xifFillRectangle)(Display*, Drawable, GC, int, int, unsigned int, unsigned int);
static int            (*xifFlush)(Display*);
static int            (*xifFree)(void *);
static int            (*xifFreeFontInfo)(char **, XFontStruct*, int);
static int            (*xifGeometry)(Display* display, int screen, _Xconst char* position, _Xconst char* default_position,
                                unsigned int, unsigned int, unsigned int, int, int, int*, int*, int*, int*);
static Bool           (*xifGetFontProperty)(XFontStruct*, Atom, unsigned long*);
static Status         (*xifGetGeometry)(Display*, Drawable, Window*, int*, int*,
                                unsigned int*, unsigned int*, unsigned int*, unsigned int*);
static int            (*xifGetWindowProperty)(Display*, Window, Atom, long, long, Bool,
                                Atom, Atom*, int*, unsigned long*, unsigned long*, unsigned char**);
static Window         (*xifGetSelectionOwner)(Display* display, Atom selection);
static int            (*xifIfEvent)(Display*, XEvent*,Bool (*) (Display*, XEvent*, XPointer), XPointer);
static Atom           (*xifInternAtom)(Display *display, _Xconst char* atom_name, Bool only_if_exists);
static char **        (*xifListFontsWithInfo)(Display*, _Xconst char*, int, int*, XFontStruct**);
static XFontStruct *  (*xifLoadQueryFont)(Display*, _Xconst char*);
static int            (*xifLookupString)(XKeyEvent*, char*, int, KeySym*, XComposeStatus*);
static int            (*xifMapWindow)(Display*, Window);
static Status         (*xifMatchVisualInfo)(Display*, int, int, int, XVisualInfo*);
static int            (*xifNextEvent)(Display*, XEvent*);
static Display *      (*xifOpenDisplay)(_Xconst char*);
static Status         (*xifParseColor)(Display*, Colormap, _Xconst char*, XColor*);
static int            (*xifRectInRegion)(Region, int, int, unsigned int, unsigned int);
static int            (*xifRefreshKeyboardMapping)(XMappingEvent*);
static Status         (*xifSendEvent)(Display*, Window, Bool, long, XEvent*);
static int            (*xifSetFont)(Display*, GC, Font);
static int            (*xifSetSelectionOwner)(Display*, Atom, Window, Time);
static void           (*xifSetWMProperties)(Display*, Window, XTextProperty*, XTextProperty*, char**,
                                int, XSizeHints*, XWMHints*, XClassHint*);
static int            (*xifSetWindowColormap)(Display*, Window, Colormap);
static Status         (*xifStringListToTextProperty)(char**, int, XTextProperty*);
static int            (*xifUnionRectWithRegion)(XRectangle*, Region, Region);
static int            (*xifUnloadFont)(Display*, Font);
static XrmDatabase    (*xifrmGetFileDatabase)(_Xconst char*);
static Bool           (*xifrmGetResource)(XrmDatabase, _Xconst char*, _Xconst char*, char**, XrmValue*);
static void           (*xifrmInitialize)(void);

static struct XFunctionEntry {
    const char *        name;
    XFunction *         func;

} XFunctions[] = {
    { "_XAllocClassHint",           (XFunction *) &xifAllocClassHint           },
    { "_XAllocColor",               (XFunction *) &xifAllocColor               },
    { "_XAllocNamedColor",          (XFunction *) &xifAllocNamedColor          },
    { "_XAllocSizeHints",           (XFunction *) &xifAllocSizeHints           },
    { "_XAllocWMHints",             (XFunction *) &xifAllocWMHints             },
    { "_XBell",                     (XFunction *) &xifBell                     },
    { "_XChangeGC",                 (XFunction *) &xifChangeGC                 },
    { "_XChangeProperty",           (XFunction *) &xifChangeProperty           },
    { "_XChangeWindowAttributes",   (XFunction *) &xifChangeWindowAttributes   },
    { "_XClearWindow",              (XFunction *) &xifClearWindow              },
    { "_XConvertSelection",         (XFunction *) &xifConvertSelection         },
    { "_XCreateColormap",           (XFunction *) &xifCreateColormap           },
    { "_XCreateGC",                 (XFunction *) &xifCreateGC                 },
    { "_XCreateRegion",             (XFunction *) &xifCreateRegion             },
    { "_XCreateSimpleWindow",       (XFunction *) &xifCreateSimpleWindow       },
    { "_XCreateWindow",             (XFunction *) &xifCreateWindow             },
    { "_XDestroyRegion",            (XFunction *) &xifDestroyRegion            },
    { "_XDrawImageString16",        (XFunction *) &xifDrawImageString16        },
    { "_XDrawLine",                 (XFunction *) &xifDrawLine                 },
    { "_XDrawString16",             (XFunction *) &xifDrawString16             },
    { "_XEventsQueued",             (XFunction *) &xifEventsQueued             },
    { "_XFillRectangle",            (XFunction *) &xifFillRectangle            },
    { "_XFlush",                    (XFunction *) &xifFlush                    },
    { "_XFree",                     (XFunction *) &xifFree                     },
    { "_XFreeFontInfo",             (XFunction *) &xifFreeFontInfo             },
    { "_XGeometry",                 (XFunction *) &xifGeometry                 },
    { "_XGetFontProperty",          (XFunction *) &xifGetFontProperty          },
    { "_XGetGeometry",              (XFunction *) &xifGetGeometry              },
    { "_XGetSelectionOwner",        (XFunction *) &xifGetSelectionOwner        },
    { "_XGetWindowProperty",        (XFunction *) &xifGetWindowProperty        },
    { "_XIfEvent",                  (XFunction *) &xifIfEvent                  },
    { "_XInternAtom",               (XFunction *) &xifInternAtom               },
    { "_XListFontsWithInfo",        (XFunction *) &xifListFontsWithInfo        },
    { "_XLoadQueryFont",            (XFunction *) &xifLoadQueryFont            },
    { "_XLookupString",             (XFunction *) &xifLookupString             },
    { "_XMapWindow",                (XFunction *) &xifMapWindow                },
    { "_XMatchVisualInfo",          (XFunction *) &xifMatchVisualInfo          },
    { "_XNextEvent",                (XFunction *) &xifNextEvent                },
    { "_XOpenDisplay",              (XFunction *) &xifOpenDisplay              },
    { "_XParseColor",               (XFunction *) &xifParseColor               },
    { "_XRectInRegion",             (XFunction *) &xifRectInRegion             },
    { "_XRefreshKeyboardMapping",   (XFunction *) &xifRefreshKeyboardMapping   },
    { "_XSendEvent",                (XFunction *) &xifSendEvent                },
    { "_XSetFont",                  (XFunction *) &xifSetFont                  },
    { "_XSetSelectionOwner",        (XFunction *) &xifSetSelectionOwner        },
    { "_XSetWMProperties",          (XFunction *) &xifSetWMProperties          },
    { "_XSetWindowColormap",        (XFunction *) &xifSetWindowColormap        },
    { "_XStringListToTextProperty", (XFunction *) &xifStringListToTextProperty },
    { "_XUnionRectWithRegion",      (XFunction *) &xifUnionRectWithRegion      },
    { "_XUnloadFont",               (XFunction *) &xifUnloadFont               },
    { "_XrmGetFileDatabase",        (XFunction *) &xifrmGetFileDatabase        },
    { "_XrmGetResource",            (XFunction *) &xifrmGetResource            },
    { "_XrmInitialize",             (XFunction *) &xifrmInitialize             }
    };


XClassHint *    XAllocClassHint(void)
    { return (*xifAllocClassHint)(); }

Status          XAllocColor(Display* display, Colormap colormap, XColor* screen_in_out)
    { return (*xifAllocColor)(display, colormap, screen_in_out); }

Status          XAllocNamedColor(Display* display, Colormap colormap, _Xconst char* color_name,
                            XColor* screen_def_return, XColor* exact_def_return)
    { return (*xifAllocNamedColor)(display, colormap, color_name, screen_def_return, exact_def_return); }

XSizeHints *    XAllocSizeHints(void)
    { return (*xifAllocSizeHints)(); }

XWMHints *      XAllocWMHints(void)
    { return (*xifAllocWMHints)(); }

int             XBell(Display *display, int percent)
    { return (*xifBell)(display, percent); }

int             XChangeGC(Display* display, GC gc, unsigned long valuemask, XGCValues* values)
    { return (*xifChangeGC)(display, gc, valuemask, values); }

int             XChangeProperty(Display *display, Window W, Atom property, Atom type,
                            int format, int mode, _Xconst unsigned char* data, int nelements)
    { return (*xifChangeProperty)(display, W, property, type, format, mode, data, nelements); }

int             XChangeWindowAttributes(Display *display, Window w, unsigned long valuemask, XSetWindowAttributes* attributes)
    { return (*xifChangeWindowAttributes)(display, w, valuemask, attributes); }

int             XClearWindow(Display* display, Window w)
    { return (*xifClearWindow)(display, w); }

int             XConvertSelection(Display* display, Atom selection, Atom target, Atom property, Window requestor, Time time)
    { return (*xifConvertSelection)(display, selection, target, property, requestor, time); }

GC              XCreateGC(Display* display, Drawable d, unsigned long x, XGCValues *value)
    { return (*xifCreateGC)(display, d, x, value); }

Colormap        XCreateColormap(Display* display, Window w, Visual* visual, int alloc)
    { return (*xifCreateColormap)(display, w, visual, alloc); }

Region          XCreateRegion(void)
    { return (*xifCreateRegion)(); }

Window          XCreateSimpleWindow(Display *display, Window parent, int x, int y, unsigned int width, unsigned int height,
                            unsigned int border_width, unsigned long border, unsigned long background)
    { return (*xifCreateSimpleWindow)(display, parent, x, y, width, height, border_width, border, background); }

Window          XCreateWindow(Display* display, Window parent, int x, int y, unsigned int width, unsigned int height,
                                unsigned int border_width, int depth, unsigned int class, Visual* visual,
                                unsigned long valuemask, XSetWindowAttributes* attributes)
    { return (*xifCreateWindow)(display, parent, x, y, width, height, border_width, depth, class, visual, valuemask, attributes); }

int             XDestroyRegion(Region r)
    { return (*xifDestroyRegion)(r); }

int             XDrawLine(Display* display, Drawable d, GC gc, int x1, int y1, int x2, int y2)
    { return (*xifDrawLine)(display, d, gc, x1, y1, x2, y2); }

int             XDrawString16(Display* display, Drawable d, GC gc, int x, int y, _Xconst XChar2b* string, int length)
    { return (*xifDrawString16)(display, d, gc, x, y, string, length); }

int             XDrawImageString16(Display* display, Drawable d, GC gc, int x, int y, _Xconst XChar2b* string, int length)
    { return (*xifDrawImageString16)(display, d, gc, x, y, string, length); }

int             XEventsQueued(Display* display, int mode)
    { return (*xifEventsQueued)(display, mode); }

int             XFillRectangle(Display* display, Drawable d, GC gc, int x, int y, unsigned int width, unsigned int height)
    { return (*xifFillRectangle)(display, d, gc, x, y, width, height); }

int             XFlush(Display* display)
    { return (*xifFlush)(display); }

int             XFree(void *data)
    { return (*xifFree)(data); }

int             XFreeFontInfo(char **names, XFontStruct* free_info, int actual_count)
    { return (*xifFreeFontInfo)(names, free_info, actual_count); }

int             XGeometry(Display* display, int screen, _Xconst char* position, _Xconst char* default_position,
                            unsigned int bwidth, unsigned int fwidth, unsigned int fheight, int xadder, int yadder,
                                int* x_return, int* y_return, int* width_return, int* height_return)
    { return (*xifGeometry)(display, screen, position, default_position, bwidth, fwidth,
                                    fheight, xadder, yadder, x_return, y_return, width_return, height_return); }

Bool            XGetFontProperty(XFontStruct* font_struct, Atom atom, unsigned long* value_return)
    { return (*xifGetFontProperty)(font_struct, atom, value_return); }

Status          XGetGeometry(Display* display, Drawable d, Window* root_return,
                                int* x_return, int* y_return, unsigned int* width_return, unsigned int* height_return,
                                unsigned int* border_width_return, unsigned int* depth_return)
    { return (*xifGetGeometry)(display, d, root_return, x_return, y_return, width_return, height_return,
                                    border_width_return, depth_return); }

Window          XGetSelectionOwner(Display* display, Atom selection)
    { return (*xifGetSelectionOwner)(display, selection); }

int             XGetWindowProperty(Display* display, Window w, Atom property, long long_offset, long long_length,
                            Bool delete, Atom req_type, Atom* actual_type_return, int* actual_format_return,
                                unsigned long* nitems_return, unsigned long* bytes_after_return, unsigned char** prop_return)
    { return (*xifGetWindowProperty)(display, w, property, long_offset, long_length, delete, req_type,
                    actual_type_return, actual_format_return, nitems_return, bytes_after_return, prop_return); }

int             XIfEvent(Display* display, XEvent* event_return,
                            Bool (*predicate) (Display* display, XEvent* event, XPointer arg), XPointer arg)
    { return (*xifIfEvent)(display, event_return, predicate, arg); }

Atom            XInternAtom(Display* display, _Xconst char* atom_name, Bool only_if_exists)
    { return (*xifInternAtom)(display, atom_name, only_if_exists); }

char **         XListFontsWithInfo(Display* display, _Xconst char* pattern, int maxnames,
                            int* count_return, XFontStruct** info_return)
    { return (*xifListFontsWithInfo)(display, pattern, maxnames, count_return, info_return); }

XFontStruct*    XLoadQueryFont(Display* display, _Xconst char* name)
    { return (*xifLoadQueryFont)(display, name); }

int             XLookupString(XKeyEvent* event_struct, char* buffer_return, int bytes_buffer,
                            KeySym* keysym_return, XComposeStatus* status_in_out)
    { return (*xifLookupString)(event_struct, buffer_return, bytes_buffer, keysym_return, status_in_out); }

int             XMapWindow(Display* display, Window w)
    { return (*xifMapWindow)(display, w); }

Status          XMatchVisualInfo(Display* display, int screen, int depth, int class, XVisualInfo* vinfo_return)
    { return (*xifMatchVisualInfo)(display, screen, depth, class, vinfo_return); }

int             XNextEvent(Display* display, XEvent* event_return)
    { return (*xifNextEvent)(display, event_return); }

Display *       XOpenDisplay(_Xconst char* display_name)
    { return (*xifOpenDisplay)(display_name); }

Status          XParseColor(Display* display, Colormap colormap, _Xconst char* spec, XColor* exact_def_return)
    { return (*xifParseColor)(display, colormap, spec, exact_def_return); }

int             XRectInRegion(Region r, int x, int y, unsigned int width, unsigned int height)
    { return (*xifRectInRegion)(r, x, y, width, height); }

int             XRefreshKeyboardMapping(XMappingEvent* event_map)
    { return (*xifRefreshKeyboardMapping)(event_map); }

Status          XSendEvent(Display* display, Window w, Bool propagate, long event_mask, XEvent* event_send)
    { return (*xifSendEvent)(display, w, propagate, event_mask, event_send); }

int             XSetFont(Display* display, GC gc, Font font)
    { return (*xifSetFont)(display, gc, font); }

int             XSetSelectionOwner(Display* display, Atom selection, Window owner, Time time)
    { return (*xifSetSelectionOwner)(display, selection, owner, time); }

void            XSetWMProperties(Display* display, Window w, XTextProperty* window_name, XTextProperty* icon_name,
                            char** argv, int argc, XSizeHints* normal_hints, XWMHints* wm_hints, XClassHint* class_hints)
    { return (*xifSetWMProperties)(display, w, window_name, icon_name, argv, argc, normal_hints, wm_hints, class_hints); }

int             XSetWindowColormap(Display* display, Window w, Colormap colormap)
    { return (*xifSetWindowColormap)(display, w, colormap); }

Status          XStringListToTextProperty(char** list, int count, XTextProperty* text_prop_return)
    { return (*xifStringListToTextProperty)(list, count, text_prop_return); }

int             XUnionRectWithRegion(XRectangle* rectangle, Region src_region, Region dest_region_return)
    { return (*xifUnionRectWithRegion)(rectangle, src_region, dest_region_return); }

int             XUnloadFont(Display* display, Font font)
    { return (*xifUnloadFont)(display, font); }

XrmDatabase     XrmGetFileDatabase(_Xconst char* filename)
    { return (*xifrmGetFileDatabase)(filename); }

Bool            XrmGetResource(XrmDatabase database, _Xconst char* str_name, _Xconst char* str_class,
                            char** str_type_return, XrmValue* value_return)
    { return (*xifrmGetResource)(database, str_name, str_class, str_type_return, value_return); }

void            XrmInitialize(void)
    { return (*xifrmInitialize)(); }


void
XLibInitialise(void)
{
#if defined(linux) || defined(unix) || defined(_AIX) || defined(__APPLE__) ||\
        defined(HAVE_DLFCN_H)

    const char *libX11 = "/usr/lib/libX11.so";
    struct XFunctionEntry *ft = XFunctions;
    void *data, *handle;
    unsigned errors = 0;

#ifndef RTLD_LOCAL
#define RTLD_LOCAL 0
#endif

    if (NULL == (handle = dlopen(libX11, RTLD_LOCAL|RTLD_LAZY))) {
        const char *err = dlerror();
        printf("xgr: <%s> load error : %s\n", libX11, (err ? err : "unknown error"));
        errors = 1;

    } else {
        unsigned f;

        for (f = 0; f < VSIZEOF(XFunctions); ++f) {
            if (NULL == (data = dlsym(handle, ft->name))) {
                printf("xgr: unable to restore '%s'\n", ft->name);
                ++errors;
            }
            *ft->func = (XFunction)data;
        }
    }

    if (errors) {
        exit(3);
    }

#else
    printf("xgr: libX11 loader not available");
    exit(3);

#endif
}

#endif /*X11_DYNAMIC*/
#endif /*HAVE_LIBX11 && HAVE_X11_XLIB_H*/
//end
