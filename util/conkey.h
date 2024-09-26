/* -*- mode: c; indent-width: 4; -*- */
/* $Id: conkey.h,v 1.3 2024/09/16 16:20:36 cvsuser Exp $
 * console key support
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

#define ALT_PRESSED (LEFT_ALT_PRESSED|RIGHT_ALT_PRESSED)
#define CTRL_PRESSED (LEFT_CTRL_PRESSED|RIGHT_CTRL_PRESSED)

#define MOUSE_PRESSED 0x1000 // extension
#define MOUSE_RELEASED 0x2000 // extension
#define MOUSE_RELEASE_ALL 0x4000 // extension

#if !defined(MOUSE_HWHEELED)
#define MOUSE_HWHEELED 0x0008
#endif

const char *mouse_description(const MOUSE_EVENT_RECORD* mer);
const char *key_description(const KEY_EVENT_RECORD *ke);

const char *DecodeCygwinKey(INPUT_RECORD *ir, const char *spec, const char *end);
const char *DecodeMSTerminalKey(INPUT_RECORD *ir, const char *spec, const char *end);
const char *DecodeXTermKey(INPUT_RECORD *ir, const char *spec, const char *end);

const void *DecodeKeyArguments(unsigned arguments[], unsigned maxargs, char terminator, const void *buffer, const void *end);

const void *DecodeXTermMouse(INPUT_RECORD *ir, const void *spec, const void *end);
const void *DecodeSGRMouse(INPUT_RECORD *ir, const void *spec, const void *end);

//end
